int water = 9; //помпа для подачи воды
int fert = 8; //помпа для подачи удобрений
int left_speed = A0;  
int right_speed = A1;
int left_dir = 7;
int right_dir = 6;
int induction = A2; // датчик индуктивности
/*
Задачи по коду:
1) Остановка и полив
2) Движение и повороты
Как делать поворот? Тоже с датчиком идуктивности?
3) Выезд в определённое время
*/
void setup() {
  pinMode(water, OUTPUT);
  pinMode(fert, OUTPUT);
  pinMode(left_speed, OUTPUT);
  pinMode(right_speed, OUTPUT);
  pinMode(left_dir, OUTPUT);
  pinMode(left_dir, OUTPUT);
  pinMode(induction, INPUT);
}

void loop() {
  // остановка
  int ind = analogRead(induction);
  if (ind) {
    digitalWrite(left_speed, 0);
    digitalWrite(right_speed, 0);
    digitalWrite(left_dir, 0);
    digitalWrite(right_dir, 0);
    digitalWrite(water, 1); // подача воды
    delay(3000);
    digitalWrite(water, 0);

    digitalWrite(fert, 1); // подача удобрения
    delay(2000);
    digitalWrite(fert, 0);

    digitalWrite(left_speed, 1);
    digitalWrite(right_speed, 1);
    delay(1000); // как только осуществлён полив, робот сразу же должен чуть-чуть отъехать, чтобы датчики опять не сработали
  } else {
    digitalWrite(left_speed, 1);
    digitalWrite(right_speed, 1);
  }
}