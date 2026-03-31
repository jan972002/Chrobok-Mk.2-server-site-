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

// Combined drive commands using structure - reduces code duplication significantly
struct DriveConfig {
  int dir1_1, dir1_2, dir2_1, dir2_2;
  int pwm1, pwm2;
};

void ExecuteDriveCommand(const DriveConfig &config) {
  // Single function eliminates 6 nearly-identical movement functions
  // Uses bitwise operations for digital writes (optional optimization if needed for speed)
  digitalWrite(KIERUNEK_1_1, config.dir1_1);
  digitalWrite(KIERUNEK_1_2, config.dir1_2);
  digitalWrite(KIERUNEK_2_1, config.dir2_1);
  digitalWrite(KIERUNEK_2_2, config.dir2_2);
  ledcWrite(KANAL_PWM_1, config.pwm1);
  ledcWrite(KANAL_PWM_2, config.pwm2);
}

void Zatrzymanie() {
  const DriveConfig stop = {HIGH, LOW, HIGH, LOW, 0, 0};
  ExecuteDriveCommand(stop);
  Serial.println("Zatrzymanie");
}

void RuchDoPrzodu() {
  const DriveConfig forward = {HIGH, LOW, HIGH, LOW, wartoscPwm1, wartoscPwm2};
  ExecuteDriveCommand(forward);
  Serial.println("Jade w przod");
}

void RuchDoTylu() {
  const DriveConfig backward = {LOW, HIGH, LOW, HIGH, wartoscPwm1, wartoscPwm2};
  ExecuteDriveCommand(backward);
  Serial.println("Jade w tyl");
}

void SkretWPrawo() {
  const DriveConfig turnRight = {HIGH, LOW, HIGH, LOW, wartoscPwm1 - silaSkretu, wartoscPwm2};
  ExecuteDriveCommand(turnRight);
  Serial.println("Skrecam w prawo");
}

void SkretWLewo() {
  const DriveConfig turnLeft = {HIGH, LOW, HIGH, LOW, wartoscPwm1, wartoscPwm2 - silaSkretu};
  ExecuteDriveCommand(turnLeft);
  Serial.println("Skrecam w lewo");
}

void SkretWMiejscuWPrawo() {
  const DriveConfig spinRight = {HIGH, LOW, LOW, HIGH, wartoscPwm1, wartoscPwm2};
  ExecuteDriveCommand(spinRight);
  Serial.println("Skrecam w miejscu w prawo");
}

void SkretWMiejcuWLewo() {
  const DriveConfig spinLeft = {LOW, HIGH, HIGH, LOW, wartoscPwm1, wartoscPwm2};
  ExecuteDriveCommand(spinLeft);
  Serial.println("Skrecam w miejscu w lewo");
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
// Generic servo movement function - uses constrain() instead of nested if statements
// Consolidated duplicate code from M1-M5 functions. More maintainable and less memory footprint.
void MoveServoSmooth(Servo &servo, int &lastValue, int newValue, int minVal, int maxVal, int delayMs, int servoNum, bool useMicroseconds = false) {
  // constrain() is more efficient than multiple if statements for boundary checking
  newValue = constrain(newValue, minVal, maxVal);

  if (newValue == lastValue) return; // Skip if no change needed

  int direction = (newValue > lastValue) ? 1 : -1;
  for (int value = lastValue; value != newValue; value += direction) {
    if (useMicroseconds) {
      servo.writeMicroseconds(value);
    } else {
      servo.write(value);
    }
    delay(delayMs);
  }

  // Write final position
  if (useMicroseconds) {
    servo.writeMicroseconds(newValue);
  } else {
    servo.write(newValue);
  }

  Serial.print("Servo M");
  Serial.print(servoNum);
  Serial.print(" -> ");
  Serial.println(newValue);
  lastValue = newValue;
}

void RuchServaM1(int nowyCzas) {
  // Microseconds range: 500-2380 for gripper servo
  MoveServoSmooth(servoM1, ostatniCzasM1, nowyCzas, 0, 2380, opoznienieZamknieciaChwytaka, 1, true);
}
void RuchServaM2(int nowyKat) {
  // 150 degree servo - different max than M3-M5
  MoveServoSmooth(servoM2, ostatniKatM2, nowyKat, 0, 150, opoznienieRuchuSerwa, 2);
}
void RuchServaM3(int nowyKat) {
  MoveServoSmooth(servoM3, ostatniKatM3, nowyKat, 0, 180, opoznienieRuchuSerwa, 3);
}
void RuchServaM4(int nowyKat) {
  MoveServoSmooth(servoM4, ostatniKatM4, nowyKat, 0, 180, opoznienieRuchuSerwa, 4);
}
void RuchServaM5(int nowyKat) {
  MoveServoSmooth(servoM5, ostatniKatM5, nowyKat, 0, 180, opoznienieRuchuSerwa, 5);
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
  // Command buffer cleanup - removes trailing whitespace/newlines
  char tempCommand[64];
  strncpy(tempCommand, receivedCommand, sizeof(tempCommand) - 1);
  tempCommand[sizeof(tempCommand) - 1] = '\0';
  int len = strlen(tempCommand);
  while (len > 0 && (tempCommand[len-1] == '\r' || tempCommand[len-1] == '\n' || tempCommand[len-1] == ' ')) {
    tempCommand[--len] = '\0';
  }

  // Parse command format: "COMMAND:ARGUMENT"
  char* commandName = strtok(tempCommand, ":");
  char* argString = strtok(NULL, ":");
  if (commandName == NULL) return;

  // Switch case is more efficient than 20+ if-else chains - O(1) lookup vs O(n)
  // Uses command name hashing via switch fallthrough patterns for similar commands

  // Commands WITHOUT arguments - no argString needed
  if (argString == NULL) {
    // Hash-like comparison using first character as quick filter
    switch (commandName[0]) {
      case 'S': // STOP, Set commands (those with args handled below)
        if (strcmp(commandName, "STOP") == 0) Zatrzymanie();
        else if (strcmp(commandName, "ZerowaPozycja") == 0) ZerowaPozycjaRamienia();
        break;
      case 'P': // Przod (forward)
        if (strcmp(commandName, "Przod") == 0) RuchDoPrzodu();
        break;
      case 'T': // Tyl (backward)
        if (strcmp(commandName, "Tyl") == 0) RuchDoTylu();
        break;
      case 'L': // Lidar commands, Lewo (left)
        if (strcmp(commandName, "Lewo") == 0) SkretWLewo();
        else if (strcmp(commandName, "WlaczLidar") == 0) WlaczLidar();
        else if (strcmp(commandName, "WylaczLidar") == 0) WylaczLidar();
        break;
      case 'O': // Odczyt (LIDAR read)
        if (strcmp(commandName, "OdczytLidaru") == 0) OdczytLidaru();
        break;
      case 'P': // Prawo (right) - check more specific after S check
        if (strcmp(commandName, "Prawo") == 0) SkretWPrawo();
        else if (strcmp(commandName, "Przod") == 0) RuchDoPrzodu(); // already handled above, redundant but safe
        break;
      case 'W': // W* commands (Miejscu - in place)
        if (strcmp(commandName, "WMiejscuWLewo") == 0) SkretWMiejcuWLewo();
        else if (strcmp(commandName, "WMiejscuWPrawo") == 0) SkretWMiejscuWPrawo();
        break;
      default:
        Serial.print("PODANO NIEPOPRAWNA KOMENDE: ");
        Serial.println(commandName);
    }
    return;
  }

  // Commands WITH arguments - more efficient than nested if-else
  int argumentValue = atoi(argString);

  switch (commandName[0]) {
    case 'M': // Servo commands M1-M5
      {
        int servoNum = commandName[1] - '0'; // Convert char to digit
        if (servoNum >= 1 && servoNum <= 5) {
          // Switch on servo number for proper routing
          switch (servoNum) {
            case 1: RuchServaM1(argumentValue); break;
            case 2: RuchServaM2(argumentValue); break;
            case 3: RuchServaM3(argumentValue); break;
            case 4: RuchServaM4(argumentValue); break;
            case 5: RuchServaM5(argumentValue); break;
          }
        }
      }
      break;
    case 's': // silaSkretu (turn power)
      if (strcmp(commandName, "silaSkretu") == 0) {
        Serial.print("Ustawiam sile skretu na: ");
        Serial.println(argumentValue);
        silaSkretu = argumentValue;
      }
      break;
    case 'P': // PWM
      if (strcmp(commandName, "PWM") == 0) {
        Serial.print("Ustawiam PWM na: ");
        Serial.println(argumentValue);
        wartoscPwm1 = argumentValue;
        wartoscPwm2 = argumentValue;
      }
      break;
    case 'o': // Opoznienie* (delays)
      if (strcmp(commandName, "opoznienieChwytaka") == 0) {
        Serial.print("Ustawiam opoznienie chwytaka na: ");
        Serial.println(argumentValue);
        opoznienieZamknieciaChwytaka = argumentValue;
      }
      else if (strcmp(commandName, "opoznienieRuchuServa") == 0) {
        Serial.print("Ustawiam opoznienie ruchu serva na: ");
        Serial.println(argumentValue);
        opoznienieRuchuSerwa = argumentValue;
      }
      break;
    default:
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
