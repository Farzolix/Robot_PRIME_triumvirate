// ---------- Пины ----------

// ЛЕВЫЙ мотор
const int RELAY_LEFT_PIN = 22;  // реле левого мотора
const int PWM_LEFT_PIN   = 4;   // скорость левого (синий)
const int DIR_LEFT_PIN   = 2;   // направление левого (белый)

// ПРАВЫЙ мотор
const int RELAY_RIGHT_PIN = 18; // реле правого мотора
const int PWM_RIGHT_PIN   = 5;  // скорость правого (синий)
// направление правого мотора физически посажено на минус батарейки

// Датчик индуктивности
const int SENSOR_PIN = 15;      // чёрный провод датчика, INPUT_PULLUP

// ПОМПЫ
const int PUMP1_PIN = 14;       // реле помпы 1
const int PUMP2_PIN = 27;       // реле помпы 2

// ---------- Настройки ----------

const int SPEED_LEFT  = 255;
const int SPEED_RIGHT = 255;

// реле считаем active-LOW (IN = LOW -> включено)
const bool RELAY_ACTIVE_LOW = true;

// можно ли сейчас реагировать на новый металл
bool canTrigger = true;

// ---------- Вспомогательные функции ----------

void relayOn(int pin) {
  digitalWrite(pin, RELAY_ACTIVE_LOW ? LOW : HIGH);
}

void relayOff(int pin) {
  digitalWrite(pin, RELAY_ACTIVE_LOW ? HIGH : LOW);
}

// моторы вперёд
void driveForward() {
  // левый мотор: направление в нужную сторону (белый = GND)
  pinMode(DIR_LEFT_PIN, OUTPUT);
  digitalWrite(DIR_LEFT_PIN, LOW);

  // включаем реле моторов
  relayOn(RELAY_LEFT_PIN);
  relayOn(RELAY_RIGHT_PIN);

  analogWrite(PWM_LEFT_PIN,  SPEED_LEFT);
  analogWrite(PWM_RIGHT_PIN, SPEED_RIGHT);
}

// моторы стоп (через реле)
void stopMotors() {
  analogWrite(PWM_LEFT_PIN,  0);
  analogWrite(PWM_RIGHT_PIN, 0);

  relayOff(RELAY_LEFT_PIN);
  relayOff(RELAY_RIGHT_PIN);
}

// помпы
void pump1On()  { relayOn(PUMP1_PIN); }
void pump1Off() { relayOff(PUMP1_PIN); }

void pump2On()  { relayOn(PUMP2_PIN); }
void pump2Off() { relayOff(PUMP2_PIN); }

// последовательность: остановиться, помпа1 5с, помпа2 5с
void runPumpSequence() {
  stopMotors();

  pump1On();
  delay(5000);
  pump1Off();

  pump2On();
  delay(5000);
  pump2Off();
}

// ---------- setup / loop ----------

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

  // датчик SN04-N
  pinMode(SENSOR_PIN, INPUT_PULLUP);

  Serial.begin(9600);

  // стартуем: металла нет, едем
  driveForward();
}

void loop() {
  int raw    = digitalRead(SENSOR_PIN);
  bool metal = (raw == LOW);  // LOW = металл под датчиком

  if (metal && canTrigger) {
    // первый раз увидели металл -> запускаем последовательность
    Serial.println("METAL DETECTED -> STOP + PUMPS");
    canTrigger = false;          // пока металл не уйдёт, больше не триггерим

    runPumpSequence();           // стоп + помпы по очереди

    // после помп сразу снова едем, даже если металл ещё есть
    driveForward();
    Serial.println("SEQUENCE DONE -> DRIVE AGAIN");
  }

  // как только металл исчезнет, снова разрешаем триггер
  if (!metal) {
    canTrigger = true;
  }

  delay(50);
}