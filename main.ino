//Здесь реализовано: 1) остновка, полив и удобрение; 2) поворот на 90 градусов вправо. Не реализовано: 3) выезд в опрделенное время


int water = 9;      // помпа воды
int fert = 8;       // помпа удобрений
int left_speed = A0;
int right_speed = A1;
int left_dir = 7;
int right_dir = 6;
int induction = A2; // датчик индукции
int ir_sensor = A3; // ИК-датчик расстояния (слева)

int IR_THRESHOLD = 700; // считаем, что чем выше значение, тем ближе стена (подбирается экспериментально) 
//при падении значения ниже считаем, что стена исчезла
int TURN_DELAY = 800;   // длительность поворота (мс), подбирается по опыту

void setup() {
  pinMode(water, OUTPUT);
  pinMode(fert, OUTPUT);
  pinMode(left_speed, OUTPUT);
  pinMode(right_speed, OUTPUT);
  pinMode(left_dir, OUTPUT);
  pinMode(right_dir, OUTPUT);
  pinMode(induction, INPUT);
  pinMode(ir_sensor, INPUT);
}

void stopMotors() {
  digitalWrite(left_speed, 0);
  digitalWrite(right_speed, 0);
}

void moveForward() {
  digitalWrite(left_dir, 1);
  digitalWrite(right_dir, 1);
  digitalWrite(left_speed, 1);
  digitalWrite(right_speed, 1);
}

//небольшое движание прямо, чтобы отъехать от датчика после полива
void moveForwardShort() {
  moveForward();
  delay(1000);
}

void turnRight() {
  // левое колесо вперёд, правое назад — робот поворачивает направо
  digitalWrite(left_dir, 1);
  digitalWrite(right_dir, 0);
  digitalWrite(left_speed, 1);
  digitalWrite(right_speed, 1);
  delay(TURN_DELAY);
  stopMotors();
  moveForwardShort(); // чтобы выйти из угла
}

//полив и удобрение
void waterAndFertilize() {
  digitalWrite(water, 1);
  delay(3000);
  digitalWrite(water, 0);

  digitalWrite(fert, 1);
  delay(2000);
  digitalWrite(fert, 0);
}

void loop() {
  int ind = analogRead(induction); // сигнал растения
  int ir = analogRead(ir_sensor);  // расстояние до стены

  moveForward(); //обычное дивижение вперед

  //Срабатывание индукционного датчика — полив и удобрение
  if (ind > 500) { // порог подбирается экспериментально (считаем что это int)
    stopMotors();
    waterAndFertilize();
    moveForwardShort();
  }

  //Проверяем, не пропала ли стена — поворот направо
  else if (ir < IR_THRESHOLD / 3) { // резко меньше нормы => стены нет => поворот (в зависимости о эксперимента тройка в заменателе может быть изменена)
    turnRight();
  }
}