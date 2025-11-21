#include <ESP32Servo.h>

#include "../../../../.platformio/packages/toolchain-riscv32-esp/riscv32-esp-elf/include/c++/8.4.0/iostream"

// przesylane beda stringi z nazwa funkcji w esp bedzie sprawdzal po if czy dobra funkcja jest wywolana

// NAPED KANAL 1
const int PWM_1 = 19;
const int KIERUNEK_1_1 = 47;
const int KIERUNEK_1_2 = 21;
// NAPED KANAL 2
const int PWM_2 = 20;
const int KIERUNEK_2_1 = 39;
const int KIERUNEK_2_2 = 40;
// SERWA
const int M_1 = 13;
const int M_2 = 14;
const int M_3 = 38;
const int M_4 = 37;
const int M_5 = 42;
// LIDAR
const int STAN = 16; //pin stanu IO
const int RDX = 17;
const int TDX = 18;
// KANALY PWM
const int KANAL_PWM_1 = 6;
const int KANAL_PWM_2 = 7;
// OBIEKTY SERVO
Servo servoM1; // OBSLUGIWANY W MIKROSEKUNDACH, //500 OTWARTY, 2380 ZAMKNIETY
Servo servoM2; // 150*
Servo servoM3; // 180*
Servo servoM4; // 180*
Servo servoM5; // 180*

//////////////////////////////////////////////////////////////////
///                                                            ///
///                                                            ///
///                   ZMIENNE GLOBALNE                         ///
///                                                            ///
///                                                            ///
//////////////////////////////////////////////////////////////////

int wartoscPwm1 = 0;
int wartoscPwm2 = 0;
int silaSkretu = 0;
int opoznienieRuchuSerwa = 10;
int opoznienieZamknieciaChwytaka = 5;
const int zeroM1 = 500; //500 - 2380
const int zeroM2 = 75;
const int zeroM3 = 30;
const int zeroM4 = 180;
const int zeroM5 = 90;
int ostatniCzasM1 = zeroM1;
int ostatniKatM2 = zeroM2;
int ostatniKatM3 = zeroM3;
int ostatniKatM4 = zeroM4;
int ostatniKatM5 = zeroM5;

//////////////////////////////////////////////////////////////////
///                                                            ///
///                                                            ///
///                   FUNKCJE RUCHU KADŁUBA                    ///
///                                                            ///
///                                                            ///
//////////////////////////////////////////////////////////////////

void Zatrzymanie(){
  digitalWrite(KIERUNEK_1_1, HIGH);
  digitalWrite(KIERUNEK_1_2, LOW);
  digitalWrite(KIERUNEK_2_1, HIGH);
  digitalWrite(KIERUNEK_2_2, LOW);
  ledcWrite(KANAL_PWM_1, 0);
  ledcWrite(KANAL_PWM_2, 0);
  Serial.print("Zatrzymanie");
}
void RuchDoPrzodu(){
  digitalWrite(KIERUNEK_1_1, HIGH);
  digitalWrite(KIERUNEK_1_2, LOW);
  digitalWrite(KIERUNEK_2_1, HIGH);
  digitalWrite(KIERUNEK_2_2, LOW);
  ledcWrite(KANAL_PWM_1, wartoscPwm1);
  ledcWrite(KANAL_PWM_2, wartoscPwm2);
  Serial.print("Jade w przod");
}
void RuchDoTylu(){
  digitalWrite(KIERUNEK_1_1, LOW);
  digitalWrite(KIERUNEK_1_2, HIGH);
  digitalWrite(KIERUNEK_2_1, LOW);
  digitalWrite(KIERUNEK_2_2, HIGH);
  ledcWrite(KANAL_PWM_1, wartoscPwm1);
  ledcWrite(KANAL_PWM_2, wartoscPwm2);
  Serial.print("Jade w tyl");
}
void SkretWPrawo(){
  digitalWrite(KIERUNEK_1_1, HIGH);
  digitalWrite(KIERUNEK_1_2, LOW);
  digitalWrite(KIERUNEK_2_1, HIGH);
  digitalWrite(KIERUNEK_2_2, LOW);
  ledcWrite(KANAL_PWM_1, wartoscPwm1 - silaSkretu);
  ledcWrite(KANAL_PWM_2, wartoscPwm2);
  Serial.print("Skrecam w prawo");
}
void SkretWLewo(){
  digitalWrite(KIERUNEK_1_1, HIGH);
  digitalWrite(KIERUNEK_1_2, LOW);
  digitalWrite(KIERUNEK_2_1, HIGH);
  digitalWrite(KIERUNEK_2_2, LOW);
  ledcWrite(KANAL_PWM_1, wartoscPwm1);
  ledcWrite(KANAL_PWM_2, wartoscPwm2 - silaSkretu);
  Serial.print("Skrecam w lewo");
}
void SkretWMiejscuWPrawo(){
  digitalWrite(KIERUNEK_1_1, HIGH);
  digitalWrite(KIERUNEK_1_2, LOW);
  digitalWrite(KIERUNEK_2_1, LOW);
  digitalWrite(KIERUNEK_2_2, HIGH);
  ledcWrite(KANAL_PWM_1, wartoscPwm1);
  ledcWrite(KANAL_PWM_2, wartoscPwm2);
  Serial.print("Skrecam w miejscu w prawo");
}
void SkretWMiejcuWLewo(){
  digitalWrite(KIERUNEK_1_1, LOW);
  digitalWrite(KIERUNEK_1_2, HIGH);
  digitalWrite(KIERUNEK_2_1, HIGH);
  digitalWrite(KIERUNEK_2_2, LOW);
  ledcWrite(KANAL_PWM_1, wartoscPwm1);
  ledcWrite(KANAL_PWM_2, wartoscPwm2);
  Serial.print("Skrecam w miejscu w lewo");
}
//////////////////////////////////////////////////////////////////
///                                                            ///
///                                                            ///
///                   FUNKCJE RUCHU RAMIENIA                   ///
///                                                            ///
///                                                            ///
//////////////////////////////////////////////////////////////////
// TRZEBA DODAĆ OPOZNIENIA DLA RUCHU!!!
void ZerowaPozycjaRamienia(){
  servoM5.write(zeroM5);
  servoM4.write(zeroM4);
  servoM3.write(zeroM3);
  servoM2.write(zeroM2);
  servoM1.writeMicroseconds(zeroM1);
  Serial.print("Pozycja ramienia wyzerowana");

}
void RuchServaM1(int nowyCzas){
  if (nowyCzas < 0) nowyCzas = 0;
  if (nowyCzas > 2380) nowyCzas = 2380;
  if (nowyCzas < ostatniCzasM1){
    for (int czas = ostatniCzasM1; czas >=nowyCzas; czas--) {
      servoM1.writeMicroseconds(czas);
      delay(opoznienieZamknieciaChwytaka);
    }
  }
  else if (nowyCzas > ostatniCzasM1){
    for (int czas = ostatniCzasM1; czas <=nowyCzas; czas++) {
      servoM1.writeMicroseconds(czas);
      delay(opoznienieZamknieciaChwytaka);
    }
  }
  Serial.print("Servo M1 przesuniete na nowy czas: ");
  Serial.println(nowyCzas);
  ostatniCzasM1 = nowyCzas;
}
void RuchServaM2(int nowyKat){
  if (nowyKat < 0) nowyKat = 0;
  if (nowyKat > 150) nowyKat = 150;
  if (nowyKat < ostatniKatM2){
    for (int kat = ostatniKatM2; kat >=nowyKat; kat--) {
      servoM2.write(kat);
      delay(opoznienieRuchuSerwa);
    }
  }
  else if (nowyKat > ostatniKatM2){
    for (int kat = ostatniKatM2; kat <=nowyKat; kat++) {
      servoM2.write(kat);
      delay(opoznienieRuchuSerwa);
    }
  }
  Serial.print("Servo M2 przesuniete na nowy kat: ");
  Serial.println(nowyKat);
  ostatniKatM2 = nowyKat;
}
void RuchServaM3(int nowyKat){
  if (nowyKat < 0) nowyKat = 0;
  if (nowyKat > 180) nowyKat = 180;
  if (nowyKat < ostatniKatM3){
    for (int kat = ostatniKatM3; kat >=nowyKat; kat--) {
      servoM3.write(kat);
      delay(opoznienieRuchuSerwa);
    }
  }
  else if (nowyKat > ostatniKatM3){
    for (int kat = ostatniKatM3; kat <=nowyKat; kat++) {
      servoM3.write(kat);
      delay(opoznienieRuchuSerwa);
    }
  }
  Serial.print("Servo M3 przesuniete na nowy kat: ");
  Serial.println(nowyKat);
  ostatniKatM3 = nowyKat;
}
void RuchServaM4(int nowyKat){
  if (nowyKat < 0) nowyKat = 0;
  if (nowyKat > 180) nowyKat = 180;
  if (nowyKat < ostatniKatM4){
    for (int kat = ostatniKatM4; kat >=nowyKat; kat--) {
      servoM4.write(kat);
      delay(opoznienieRuchuSerwa);
    }
  }
  else if (nowyKat > ostatniKatM4){
    for (int kat = ostatniKatM4; kat <=nowyKat; kat++) {
      servoM4.write(kat);
      delay(opoznienieRuchuSerwa);
    }
  }
  Serial.print("Servo M4 przesuniete na nowy kat: ");
  Serial.println(nowyKat);
  ostatniKatM4 = nowyKat;
}
void RuchServaM5(int nowyKat){
  if (nowyKat < 0) nowyKat = 0;
  if (nowyKat > 180) nowyKat = 180;
  if (nowyKat < ostatniKatM5){
    for (int kat = ostatniKatM5; kat >=nowyKat; kat--) {
      servoM5.write(kat);
      delay(opoznienieRuchuSerwa);
    }
  }
  else if (nowyKat > ostatniKatM5){
    for (int kat = ostatniKatM5; kat <=nowyKat; kat++) {
      servoM5.write(kat);
      delay(opoznienieRuchuSerwa);
    }
  }
  Serial.print("Servo M5 przesuniete na nowy kat: ");
  Serial.println(nowyKat);
  ostatniKatM5 = nowyKat;
}


//////////////////////////////////////////////////////////////////
///                                                            ///
///                                                            ///
///                  FUNKCJE STANU + LOGIKA                    ///
///                                                            ///
///                                                            ///
//////////////////////////////////////////////////////////////////

void WlaczLidar(){
  digitalWrite(STAN, HIGH);
  Serial.println("LIDAR Wlaczony");
}
void WylaczLidar(){
  digitalWrite(STAN, LOW);
  Serial.println("LIDAR Wylaczony");
}
int OdczytLidaru(){
  static const int ROZMIAR_PAKIETU_LIDAR = 9;
  static byte pakietLidar[ROZMIAR_PAKIETU_LIDAR];
  static int indeksPakietu = 0;
  static int odleglosc = 0;
  while (Serial2.available() > 0) {
    byte bajtLidar = Serial2.read();
    if (indeksPakietu == 0 && bajtLidar != 0x59) {
      continue;
    }
    if (indeksPakietu == 1 && bajtLidar != 0x59) {
      indeksPakietu = 0;
      continue;
    }
    pakietLidar[indeksPakietu] = bajtLidar;
    indeksPakietu++;
    if (indeksPakietu == ROZMIAR_PAKIETU_LIDAR) {
      odleglosc = pakietLidar[2] + (pakietLidar[3] << 8);
      indeksPakietu = 0;
    }
  }
  Serial.print("Odczyt danych z LIDAR wlaczony");
  return odleglosc;
}
void WysylDanychPoUsb (int daneDoWyslania) {
  Serial.print("ODLEGLOSC:"+daneDoWyslania);
  Serial.print("Wysylam dane po USB");
}
String OdbiorDanychZUsb() {
  static String inputString = "";
  while (Serial.available()) {
    char inChar = Serial.read();
    if (inChar == '\n') {
      String completeCommand = inputString;
      inputString = "";
      return completeCommand;
    } else {
      inputString += inChar;
    }
  }
  return "";
}
void WykonajFunkcjeZKomenda(char* receivedCommand) {
  char tempCommand[64];
  strncpy(tempCommand, receivedCommand, sizeof(tempCommand) - 1);
  tempCommand[sizeof(tempCommand) - 1] = '\0';
  int len = strlen(tempCommand);
  while (len > 0 && (tempCommand[len-1] == '\r' || tempCommand[len-1] == '\n' || tempCommand[len-1] == ' ')) {
    tempCommand[--len] = '\0';
  }
  char* commandName = strtok(tempCommand, ":");
  char* argString = strtok(NULL, ":");
  if (commandName == NULL) return;

  if (argString == NULL) {
    if (strcmp(commandName, "STOP") == 0) {
      Zatrzymanie();
    }
    else if (strcmp(commandName, "Przod") == 0) {
      RuchDoPrzodu();
    }
    else if (strcmp(commandName, "Tyl") == 0) {
      RuchDoTylu();
    }
    else if (strcmp(commandName, "Prawo") == 0) {
      SkretWPrawo();
    }
    else if (strcmp(commandName, "Lewo") == 0) {
      SkretWLewo();
    }
    else if (strcmp(commandName, "WMiejscuWLewo") == 0) {
      SkretWMiejcuWLewo();
    }
    else if (strcmp(commandName, "WMiejscuWPrawo") == 0) {
      SkretWMiejscuWPrawo();
    }
    else if (strcmp(commandName, "WlaczLidar") == 0) {
      WlaczLidar();
    }
    else if (strcmp(commandName, "WylaczLidar") == 0) {
      WylaczLidar();
    }
    else if (strcmp(commandName, "ZerowaPozycja") == 0) {
      ZerowaPozycjaRamienia();
    }
    else if (strcmp(commandName, "OdczytLidaru") == 0) {
      OdczytLidaru();
    }
    else {
      Serial.print("PODANO NIEPOPRAWNA KOMENDE: ");
      Serial.println(commandName);
    }
    return;
  }
  int argumentValue = atoi(argString);
  if (strcmp(commandName, "M1") == 0) {
    RuchServaM1(argumentValue);
  }
  else if (strcmp(commandName, "M2") == 0) {
    RuchServaM2(argumentValue);
  }
  else if (strcmp(commandName, "M3") == 0) {
    RuchServaM3(argumentValue);
  }
  else if (strcmp(commandName, "M4") == 0) {
    RuchServaM4(argumentValue);
  }
  else if (strcmp(commandName, "M5") == 0) {
    RuchServaM5(argumentValue);
  }
  else if (strcmp(commandName, "silaSkretu") == 0) {
    Serial.print("Ustawiam sile skretu na: ");
    Serial.println(argumentValue);
    silaSkretu = argumentValue;
  }
  else if (strcmp(commandName, "PWM") == 0) {
    Serial.print("Ustawiam PWM na: ");
    Serial.println(argumentValue);
    wartoscPwm1 = argumentValue;
    wartoscPwm2 = argumentValue;
  }
  else if (strcmp(commandName, "opoznienieChwytaka") == 0) {
    Serial.print("Ustawiam opoznienie chwytaka na: ");
    Serial.println(argumentValue);
    opoznienieZamknieciaChwytaka = argumentValue;
  }
  else if (strcmp(commandName, "opoznienieRuchuServa") == 0) {
    Serial.print("Ustawiam opoznienie ruchu serva na: ");
    Serial.println(argumentValue);
    opoznienieRuchuSerwa = argumentValue;
  }
  else {
    Serial.print("PODANO NIEPOPRAWNA KOMENDE: ");
    Serial.println(commandName);
  }
}
/*                DO DODANIA
 *
 *  Dodać stany lidaru włączony lub wyłączony , stan przechowywany jako zmienna i jak powiedzmy int stanLidaru == 1, to odczyt dziala a jak == 0 to nie dziala.
 *  Uzależnić odczyt od stanu włączania lidaru
 *  dodać wysył danych po usb
 *  ZMIENIC FUNKCJE SERW NA millis() TAK, ZEBY NIE BYLY BLOKUJACE
 *
 */
void setup(){
  // SERIALE
  Serial.println("LADOWANIE BLOKOW USTAWIEN");
  Serial.println("INICJOWANIE BLOKU SZEREGOWEGO...");
  Serial.begin(115200);
  Serial.println("Blok szeregowy 0 zainicjowany");
  delay(500);
  Serial2.begin(115200,SERIAL_8N1, RDX, TDX);
  Serial.println("Blok szeregowy 2 zainicjowany");
  Serial.println("BLOK SZEREGOWY ZAINICJOWANY POMYSLNIE!");
  // STANY PINÓW
  Serial.println("INICJOWANIE BLOKU STANOW...");
  pinMode(KIERUNEK_1_1, OUTPUT);
  pinMode(KIERUNEK_1_2, OUTPUT);
  pinMode(KIERUNEK_2_1, OUTPUT);
  pinMode(KIERUNEK_2_2, OUTPUT);
  pinMode(STAN, OUTPUT);
  delay(500);
  Serial.println("BLOK STANOW ZAINICJOWANY POMYSLNIE!");
  // PWM
  Serial.println("INICJOWANIE BLOKU PWM...");
  ledcSetup(KANAL_PWM_1, 1000, 8);
  Serial.println("Czestotliwosc PWM_1 zainicjowana");
  ledcSetup(KANAL_PWM_2, 1000,8);
  Serial.println("Czestotliwosc PWM_2 zainicjowana");
  delay(500);
  ledcAttachPin(PWM_1, KANAL_PWM_1);
  Serial.println("PWM_1 zainicjowany");
  ledcAttachPin(PWM_2, KANAL_PWM_2);
  Serial.println("PWM_2 zainicjowany");
  delay(500);
  Serial.println("BLOK PWM ZAINICJOWANY POMYSLNIE!");
  // SERVA
  Serial.println("INICJOWANIE BLOKU SERVO...");
  servoM1.attach(M_1);
  Serial.println("Servo M_1 zainicjowane");
  servoM2.attach(M_2);
  Serial.println("Servo M_2 zainicjowane");
  servoM3.attach(M_3);
  Serial.println("Servo M_3 zainicjowane");
  servoM4.attach(M_4);
  Serial.println("Servo M_4 zainicjowane");
  servoM5.attach(M_5);
  Serial.println("Servo M_5 zainicjowane");
  Serial.println("BLOK SERVO ZAINICJOWANY POMYSLNIE!");
  Serial.println("WSZYSTKIE BLOKI ZAINICJOWANE POMYSLNIE");
  Serial.println("PROGRAM GOTOWY DO PRACY");
}


void loop(){

  String komenda = OdbiorDanychZUsb();
  if (komenda.length() > 0) {
    char commandArray[komenda.length() + 1];
    komenda.toCharArray(commandArray, komenda.length() + 1);
    WykonajFunkcjeZKomenda(commandArray);
  }

}
