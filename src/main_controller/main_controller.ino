#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Inicializa o LCD no endereço correto (0x27 ou 0x3F)
LiquidCrystal_I2C lcd(0x27, 16, 2); 

// Definição dos Pinos
const int pinoBotaoTempo = 2;
const int pinoBotaoIniciar = 3;
const int pinoReleSirene = 4;
const int pinoBuzzer = 11; 
const int pinosFios[8] = {5, 6, 7, 8, 9, 10, 12, A3}; // ID:01 a ID:08

// Tipos de funções para os fios
enum FuncaoFio { NEUTRO_1, NEUTRO_2, DESARME, DETONACAO, ACELERAR, DESACELERAR, DOBRAR, METADE };
FuncaoFio funcoesSorteadas[8];

// Variáveis de Controle do Cronômetro
long tempoRestanteSg = 300; 
bool bombaAtiva = false;
bool bombaDetonada = false;
bool bombaDesarmada = false;

// Variáveis de Modificadores (Efeito dos Fios)
float velocidadeContagem = 1.0; 
unsigned long ultimoMilissegundo = 0;
bool fioCortadoStatus[8] = {false, false, false, false, false, false, false, false};

void setup() {
  Serial.begin(9600);
  
  // Inicialização do LCD
  lcd.init();
  lcd.backlight();
  
  // Configuração dos Pinos
  pinMode(pinoBotaoTempo, INPUT_PULLUP);
  pinMode(pinoBotaoIniciar, INPUT_PULLUP);
  pinMode(pinoReleSirene, OUTPUT);
  pinMode(pinoBuzzer, OUTPUT);
  
  digitalWrite(pinoReleSirene, LOW); 
  digitalWrite(pinoBuzzer, LOW);

  for (int i = 0; i < 8; i++) {
    pinMode(pinosFios[i], INPUT_PULLUP);
  }

  // Sorteia e inicializa o painel
  sortearFios();
  mostrarMenuInicial();
}

void loop() {
  if (!bombaAtiva && !bombaDetonada && !bombaDesarmada) {
    configurarTempo();
  } 
  else if (bombaAtiva && !bombaDetonada && !bombaDesarmada) {
    atualizarCronometro();
    verificarCorteFios();
  }
}

void sortearFios() {
  // --- CURA DO VÍCIO DO SOTEIO (Semente de Ruído Amplificado) ---
  // Lemos o pino A7 (deve estar totalmente livre na placa) 16 vezes
  // acumulando variações elétricas para quebrar qualquer padrão no boot.
  unsigned long semente = 0;
  for (int k = 0; k < 16; k++) {
    semente = (semente << 2) + analogRead(A7); 
    delayMicroseconds(50);
  }
  randomSeed(semente);

  // Lista com as 8 funções distribuídas
  FuncaoFio listaBase[8] = {NEUTRO_1, NEUTRO_2, DESARME, DETONACAO, ACELERAR, DESACELERAR, DOBRAR, METADE};
  
  // Embaralha os 8 itens de forma justa
  for (int i = 7; i > 0; i--) {
    int j = random(0, i + 1);
    FuncaoFio temp = listaBase[i];
    listaBase[i] = listaBase[j];
    listaBase[j] = temp;
  }
  for (int i = 0; i < 8; i++) {
    funcoesSorteadas[i] = listaBase[i];
  }
  
  // Imprime o primeiro relatório para o celular do Chefe
  atualizarPainelChefe();
}

void atualizarPainelChefe() {
  Serial.println(F("\n=================================================="));
  Serial.println(F("   SISTEMA OPERACIONAL DA BOMBA (EOD DECODER v2)  "));
  Serial.println(F("=================================================="));
  
  // 1. Relatório de Fios que já foram cortados
  Serial.println(F("[LOG DE COMPONENTES DANIFICADOS]:"));
  bool algumCortado = false;
  for(int i = 0; i < 8; i++) {
    if(fioCortadoStatus[i]) {
      algumCortado = true;
      Serial.print(F("  -> ID:0")); Serial.print(i + 1);
      Serial.print(F(" Status: CORTADO [Ação: "));
      Serial.print(obterNomeFuncao(funcoesSorteadas[i]));
      Serial.println(F("]"));
    }
  }
  if(!algumCortado) Serial.println(F("  -> Nenhum condutor rompido até o momento."));
  
  Serial.println(F("--------------------------------------------------"));
  Serial.println(F("[MAPEAMENTO DE LINHAS ATIVAS]:"));

  // Varre todos os 8 fios para exibir o painel do Chefe
  for(int i = 0; i < 8; i++) {
    if(fioCortadoStatus[i]) {
      continue; // Ignora os já cortados neste bloco
    }

    Serial.print(F("  [ID:0")); Serial.print(i + 1); Serial.print(F("] -> "));

    // --- NOVA LÓGICA CRÍTICA: SEGREDO DE ESTADO ---
    // Se o fio atual for o de DESARME ou o de DETONAÇÃO, o sistema bloqueia
    // e exibe erro para forçar o duelo final de 50% de chance!
    if(funcoesSorteadas[i] == DESARME || funcoesSorteadas[i] == DETONACAO) {
      Serial.println(F("#### [CRIPTOGRAFADO / ERRO DE LEITURA] ####"));
    } else {
      // Todos os outros efeitos colaterais e neutros são entregues de bandeja
      Serial.print(F("DESCRIPTOGRAFADO COM SUCESSO: ["));
      Serial.print(obterNomeFuncao(funcoesSorteadas[i]));
      Serial.println(F("]"));
    }
  }
  
  Serial.println(F("=================================================="));
  Serial.println(F("[STATUS]: AGUARDANDO NOVA ACAO.\n"));
}

// Função auxiliar para traduzir o Enum em texto legível no celular do Chefe
String obterNomeFuncao(FuncaoFio f) {
  switch(f) {
    case NEUTRO_1:     return "Fio Neutro 1";
    case NEUTRO_2:     return "Fio Neutro 2";
    case DESARME:      return "DESARME DA BOMBA (VITORIA)";
    case DETONACAO:    return "DETONACAO IMEDIATA (EXPLOSAO)";
    case ACELERAR:     return "Modificador de Tempo: ACELERAR +2x";
    case DESACELERAR:  return "Modificador de Tempo: REDUZIR RITMO";
    case DOBRAR:       return "Modificador de Tempo: DUPLICAR CRONOMETRO";
    case METADE:       return "Modificador de Tempo: CORTAR TEMPO EM 50%";
    default:           return "Desconhecido";
  }
}

void mostrarMenuInicial() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("BOMBA PRONTA!");
  lcd.setCursor(0, 1);
  lcd.print("Tempo: ");
  exibirTempoNoLCD(tempoRestanteSg);
}

void configurarTempo() {
  if (digitalRead(pinoBotaoTempo) == LOW) {
    tone(pinoBuzzer, 2000, 50); 
    delay(250); 
    
    tempoRestanteSg += 300; // +5 minutos
    if (tempoRestanteSg > 3600) { // Limite 1 hora
      tempoRestanteSg = 300; 
    }
    mostrarMenuInicial();
  }

  if (digitalRead(pinoBotaoIniciar) == LOW) {
    tone(pinoBuzzer, 1500, 100); delay(150);
    tone(pinoBuzzer, 1800, 100); delay(150);
    tone(pinoBuzzer, 2200, 300); 
    
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(" ARMANDO SISTEMA");
    for(int i = 0; i < 16; i++) {
      lcd.setCursor(i, 1);
      lcd.print(".");
      tone(pinoBuzzer, 1000, 20);
      delay(100);
    }
    
    bombaAtiva = true;
    ultimoMilissegundo = millis();
    lcd.clear();
    // Atualiza o painel serial no momento que o jogo efetivamente começa
    atualizarPainelChefe(); 
  }
}

void atualizarCronometro() {
  unsigned long milissegundoAtual = millis();
  unsigned long intervaloReal = 1000 * velocidadeContagem;

  if (milissegundoAtual - ultimoMilissegundo >= intervaloReal) {
    ultimoMilissegundo = milissegundoAtual;
    tempoRestanteSg--;

    // --- CORREÇÃO DO BUG VISUAL ---
    // Toda vez que o relógio atualiza, nós forçamos o LCD a limpar a linha 0
    // preenchendo os 16 caracteres com espaços vazios antes de reescrever.
    lcd.setCursor(0, 0);
    lcd.print("                "); // 16 espaços em branco para apagar fantasmas
    
    lcd.setCursor(0, 0);
    lcd.print("TEMPO RESTANTE: ");
    
    // Fazemos o mesmo na linha de baixo para o cronômetro ficar limpo
    lcd.setCursor(0, 1);
    lcd.print("                "); 
    
    lcd.setCursor(5, 1);
    exibirTempoNoLCD(tempoRestanteSg);

    if (velocidadeContagem < 1.0) {
      tone(pinoBuzzer, 1300, 70); 
    } else {
      tone(pinoBuzzer, 1000, 50); 
    }

    if (tempoRestanteSg <= 0) {
      detonarBomba();
    }
  }
}

void verificarCorteFios() {
  bool houveCorte = false;
  for (int i = 0; i < 8; i++) {
    if (digitalRead(pinosFios[i]) == HIGH && fioCortadoStatus[i] == false) {
      fioCortadoStatus[i] = true; 
      houveCorte = true;
      
      tone(pinoBuzzer, 1800, 80); delay(100); tone(pinoBuzzer, 1800, 80);

      switch (funcoesSorteadas[i]) {
        
        case DESARME:
          desarmarBomba();
          break;

        case DETONACAO:
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("FIO DETONADOR!! ");
          lcd.setCursor(0, 1);
          lcd.print("   ERRO FATAL   ");
          delay(1200);
          detonarBomba();
          break;

        case ACELERAR:
          velocidadeContagem = 0.4; 
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("RITMO ACELERADO!");
          lcd.setCursor(0, 1);
          lcd.print("ADRENALINA: +2x ");
          delay(1500);
          break;

        case DESACELERAR:
          velocidadeContagem = 1.8; 
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print(" RITMO REDUZIDO ");
          lcd.setCursor(0, 1);
          lcd.print(" GANHOU TEMPO!  ");
          delay(1500);
          break;

        case DOBRAR: 
          tempoRestanteSg = tempoRestanteSg * 2;
          if (tempoRestanteSg > 5400) tempoRestanteSg = 5400; 
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("TEMPO DUPLICADO!");
          lcd.setCursor(0, 1);
          lcd.print("   SISTEMA x2   ");
          delay(1500);
          break;

        case METADE: 
          tempoRestanteSg = tempoRestanteSg / 2;
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("TEMPO CORTADO!! ");
          lcd.setCursor(0, 1);
          lcd.print(" REDUZIDO A 50% ");
          delay(1500);
          break;

        case NEUTRO_1:
        case NEUTRO_2:
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("   FIO NEUTRO   ");
          lcd.setCursor(0, 1);
          lcd.print(" SEM ALTERACAO  ");
          delay(1200);
          break;
      }
      
      // Se houve corte e o jogo não acabou, atualiza o painel do hacker dinamicamente!
      if (bombaAtiva && !bombaDetonada && !bombaDesarmada) {
        atualizarPainelChefe();
      }
    }
  }
}

void desarmarBomba() {
  bombaAtiva = false;
  bombaDesarmada = true;
  digitalWrite(pinoReleSirene, LOW);
  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("BOMBA DESARMADA ");
  lcd.setCursor(0, 1);
  lcd.print("VITORIA DA EQUIPE");
  
  // Envia log definitivo de vitória para a tela do Chefe
  Serial.println("\n==================================================");
  Serial.println("  [ALERTA]: CIRCUITO DESATIVADO COM SUCESSO!     ");
  Serial.println("               --- VITORIA DA EQUIPE ---         ");
  Serial.println("==================================================");
  
  tone(pinoBuzzer, 1500, 150); delay(200);
  tone(pinoBuzzer, 1500, 150); delay(200);
  tone(pinoBuzzer, 2000, 400);
}

void detonarBomba() {
  bombaAtiva = false;
  bombaDetonada = true;
  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(" BOMBA DETONADA! ");
  lcd.setCursor(0, 1);
  lcd.print(" GAME OVER !!!  ");
  
  // Envia log definitivo de detonação para a tela do Chefe
  Serial.println("\n==================================================");
  Serial.println("  [CRÍTICO]: ARTEFATO DETONADO! ERRO FATAL!       ");
  Serial.println("               --- GAME OVER ---                  ");
  Serial.println("==================================================");
  
  digitalWrite(pinoReleSirene, HIGH); 
  tone(pinoBuzzer, 600, 3000); 
}

void exibirTempoNoLCD(long tempoSegundos) {
  long m = tempoSegundos / 60;
  long s = tempoSegundos % 60;
  if (m < 10) lcd.print("0");
  lcd.print(m);
  lcd.print(":");
  if (s < 10) lcd.print("0");
  lcd.print(s);
}