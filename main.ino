// Здесь реализовано:
// 1) Остановка, полив и удобрение при срабатывании индуктивного датчика (SN04-N)
// 2) Движение вперёд по умолчанию
// 3) Поворот на 90 градусов вправо при исчезновении стены слева по ИК-датчику
// Не реализовано: выезд/старт в заданное время

// --- Пины исполнительных устройств ---
int water       = 9;   // помпа воды
int fert        = 8;   // помпа удобрений

int left_speed  = A0;  // управление скоростью левого мотора
int right_speed = A1;  // управление скоростью правого мотора
int left_dir    = 7;   // направление левого мотора
int right_dir   = 6;   // направление правого мотора

// --- Датчики ---
int induction   = 2;   // ИНДУКТИВНЫЙ ДАТЧИК SN04-N (цифровой выход, NPN, активный LOW)
int ir_sensor   = A3;  // ИК-датчик расстояния (аналоговый, слева)

// --- Константы поведения робота ---
int IR_THRESHOLD = 700;   // чем выше значение, тем ближе стена (подбирается экспериментально)
// при падении значения ниже IR_THRESHOLD, считаем, что стена слева исчезла
int TURN_DELAY   = 800;   // длительность поворота (мс), подбирается по опыту

void setup() {
  // Помпы
  pinMode(water, OUTPUT);
  pinMode(fert, OUTPUT);

  // Моторы
  pinMode(left_speed,  OUTPUT);
  pinMode(right_speed, OUTPUT);
  pinMode(left_dir,    OUTPUT);
  pinMode(right_dir,   OUTPUT);

  // Датчик индукции SN04-N:
  // SN04-N — открытый коллектор, замыкает выход на GND при срабатывании.
  // Поэтому включаем внутреннюю подтяжку к +5 В.
  // Подключение:
  //   коричневый -> +5 В (VCC)
  //   синий      -> GND
  //   чёрный     -> D2 (сюда, на пин "induction")
  pinMode(induction, INPUT_PULLUP);

  // ИК-датчик как аналоговый вход
  pinMode(ir_sensor, INPUT);

  // Включаем Serial для отладки (по желанию можно убрать)
  Serial.begin(115200);
}

// Полная остановка моторов
void stopMotors() {
  digitalWrite(left_speed,  LOW);
  digitalWrite(right_speed, LOW);
}

// Движение вперёд
void moveForward() {
  // направление вперёд для обоих моторов
  digitalWrite(left_dir,  HIGH);
  digitalWrite(right_dir, HIGH);
  // включаем оба мотора
  digitalWrite(left_speed,  HIGH);
  digitalWrite(right_speed, HIGH);
}

// Небольшое движение вперёд, чтобы отъехать от растения/датчика после полива
void moveForwardShort() {
  moveForward();
  delay(1000);  // время подбирается по факту
  // можно добавить stopMotors(), если нужно реально остановиться
  // stopMotors();
}

// Поворот направо на месте
void turnRight() {
  // Левое колесо вперёд, правое назад — робот поворачивает направо
  digitalWrite(left_dir,  HIGH);  // левый мотор вперёд
  digitalWrite(right_dir, LOW);   // правый мотор назад

  digitalWrite(left_speed,  HIGH);
  digitalWrite(right_speed, HIGH);

  delay(TURN_DELAY);  // реально калибруем этот параметр в поле

  stopMotors();
  moveForwardShort(); // чтобы выйти из угла/от стены
}

// Последовательность "полив + удобрение" на месте
void waterAndFertilize() {
  // Сначала полив
  digitalWrite(water, HIGH);
  delay(3000);             // длительность полива (мс)
  digitalWrite(water, LOW);

  // Затем удобрение
  digitalWrite(fert, HIGH);
  delay(2000);             // длительность подачи удобрений (мс)
  digitalWrite(fert, LOW);
}

void loop() {
  // --- Считываем датчики ---

  // Индуктивный датчик SN04-N:
  // digitalRead(induction) вернёт:
  //   HIGH (1) — металла нет (датчик отпущен, пин подтянут к +5 В)
  //   LOW  (0) — металл есть под датчиком (SN04-N тянет выход к GND)
  int ind_raw = digitalRead(induction);

  // Приводим к логической переменной: true = есть растение/металл
  bool plant_detected = (ind_raw == LOW);

  // ИК-датчик расстояния — аналоговый (0..1023)
  int ir = analogRead(ir_sensor);

  // Отладочный вывод (закоментировать после колибровки для того, чтобы не засорять Serial)
  Serial.print("IND=");
  Serial.print(plant_detected ? 1 : 0);
  Serial.print("  IR=");
  Serial.println(ir);

  // --- Базовое поведение: робот едет вперёд ---
  moveForward();

  // --- 1) Срабатывание индукционного датчика — полив и удобрение ---
  if (plant_detected) {
    stopMotors();          // остановка у растения
    waterAndFertilize();   // полив + удобрение
    moveForwardShort();    // чуть отъехать от растения/датчика
  }

  // --- 2) Исчезновение стены слева — поворот направо ---
  // Если ИК-датчик показывает значение сильно меньше нормы,
  // считаем, что «стена ушла» и нужно повернуть в новый ряд.
  else if (ir < IR_THRESHOLD / 3) {
    // Коэффициент (1/3) подбирается экспериментально под конкретный ИК-датчик
    turnRight();
  }

  // Если ни одно условие не сработало — робот просто продолжает ехать вперёд
}
