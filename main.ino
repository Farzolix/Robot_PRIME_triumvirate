// -------- Пины --------

// ЛЕВЫЙ мотор
const int RELAY_LEFT_PIN = 22;
const int PWM_LEFT_PIN   = 4;
const int DIR_LEFT_PIN   = 2;

// ПРАВЫЙ мотор
const int RELAY_RIGHT_PIN = 18;
const int PWM_RIGHT_PIN   = 5;
// направление правого мотора посажено на GND проводом

// Индуктивный датчик 
const int SENSOR_METAL_PIN = 15;

// ИК-датчик стены
const int IR_WALL_PIN = 12;

// Помпы (реле)
const int PUMP1_PIN = 14;
const int PUMP2_PIN = 27;

// -------- Настройки --------

const int SPEED_LEFT  = 255;
const int SPEED_RIGHT = 255;

// Реле считаем active-LOW (IN = LOW -> включено).
const bool RELAY_ACTIVE_LOW = true;

// Металл: можно ли сейчас запускать последовательность с помпами
bool canTriggerMetal = true;

// -------- Вспомогательные функции для реле --------

void relayOn(int pin) {
  digitalWrite(pin, RELAY_ACTIVE_LOW ? LOW : HIGH);
}

void relayOff(int pin) {
  digitalWrite(pin, RELAY_ACTIVE_LOW ? HIGH : LOW);
}

// -------- Моторы --------

void stopMotors() {
  analogWrite(PWM_LEFT_PIN,  0);
  analogWrite(PWM_RIGHT_PIN, 0);

  relayOff(RELAY_LEFT_PIN);
  relayOff(RELAY_RIGHT_PIN);
}

void driveForwardBoth() {
  // левый: нужное направление
  pinMode(DIR_LEFT_PIN, OUTPUT);
  digitalWrite(DIR_LEFT_PIN, LOW);

  relayOn(RELAY_LEFT_PIN);
  relayOn(RELAY_RIGHT_PIN);

  analogWrite(PWM_LEFT_PIN,  SPEED_LEFT);
  analogWrite(PWM_RIGHT_PIN, SPEED_RIGHT);
}

// поворот: едет только левое колесо (правое стоит)
void driveLeftOnly() {
  pinMode(DIR_LEFT_PIN, OUTPUT);
  digitalWrite(DIR_LEFT_PIN, LOW);

  relayOn(RELAY_LEFT_PIN);
  analogWrite(PWM_LEFT_PIN, SPEED_LEFT);

  analogWrite(PWM_RIGHT_PIN, 0);
  relayOff(RELAY_RIGHT_PIN);
}

// -------- Помпы --------

void pump1On()  { relayOn(PUMP1_PIN); }
void pump1Off() { relayOff(PUMP1_PIN); }

void pump2On()  { relayOn(PUMP2_PIN); }
void pump2Off() { relayOff(PUMP2_PIN); }

// последовательность при металле: стоп -> помпа1 5с -> помпа2 5с
void runPumpSequence() {
  stopMotors();

  pump1On();
  delay(5000);
  pump1Off();

  pump2On();
  delay(5000);
  pump2Off();
}

// -------- Чтение датчиков --------

bool isMetalDetected() {
  int raw = digitalRead(SENSOR_METAL_PIN);
  // SN04-N: при металле тянет на GND
  return (raw == LOW);
}

// предполагаем, что ИК-датчик даёт LOW, когда "видит стену"
bool isWallSeen() {
  int raw = digitalRead(IR_WALL_PIN);
  bool wall = (raw == LOW);
  return wall;

  // если HIGH = стена, поменять на:
  // bool wall = (raw == HIGH);
}

// -------- Логика ИК-датчика (стены) --------

// Управляет режимами движения: прямой ход / поворот
void handleIRLogic() {
  static bool lastWall = true;   // прошлое состояние: стена была/нет
  static bool turning  = false;  // сейчас крутимся одним колесом?

  bool wall = isWallSeen();

  if (wall) {
    // стена есть: едем двумя колёсами вперёд
    if (!lastWall || turning) {
      driveForwardBoth();
      turning = false;
      Serial.println("IR: WALL -> DRIVE BOTH");
    }
  } else {
    // стены нет: один раз при переходе остановиться и начать поворот
    if (lastWall) {
      Serial.println("IR: NO WALL -> STOP 3s, THEN TURN");
      stopMotors();
      delay(3000);
      driveLeftOnly();
      turning = true;
    } else {
      // продолжаем поворот
      if (!turning) {
        driveLeftOnly();
        turning = true;
        Serial.println("IR: KEEP TURNING");
      }
    }
  }

  lastWall = wall;
}

// -------- setup / loop --------

void setup() {
  // моторы
  pinMode(RELAY_LEFT_PIN,  OUTPUT);
  pinMode(RELAY_RIGHT_PIN, OUTPUT);
  pinMode(PWM_LEFT_PIN,    OUTPUT);
  pinMode(PWM_RIGHT_PIN,   OUTPUT);

  // помпы
  pinMode(PUMP1_PIN, OUTPUT);
  pinMode(PUMP2_PIN, OUTPUT);
  relayOff(PUMP1_PIN);
  relayOff(PUMP2_PIN);

  // датчики
  pinMode(SENSOR_METAL_PIN, INPUT_PULLUP);  // SN04-N
  pinMode(IR_WALL_PIN,      INPUT);        // ИК-датчик

  Serial.begin(9600);

  // старт: считаем, что стену видим -> едем двумя
  driveForwardBoth();
}

void loop() {
  // --- металл + помпы ---
  bool metal = isMetalDetected();

  if (metal && canTriggerMetal) {
    Serial.println("METAL: START PUMP SEQUENCE");
    canTriggerMetal = false;

    runPumpSequence();   // блокирующие 10 секунд

    Serial.println("METAL: PUMP SEQUENCE DONE");
    // после помп дальше рулит ИК-логика
  }

  if (!metal) {
    // металл ушёл -> можно снова триггерить в будущем
    canTriggerMetal = true;
  }

  // --- логика стены/поворота ---
  handleIRLogic();

  delay(50);
}
