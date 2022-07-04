//includes
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Stepper.h>
LiquidCrystal_I2C ecran(0x3F, 16, 2);

//pins
byte poten = A0, enter = A2, arret = A3, leds = 3, photo = 4;
//--> potentiometer / enter button / stop button / leds (white) / take picture relay

//moteur pas à pas
int nbPas = 48 * 64;
// stepper motor controller (ULN2003) pins
Stepper motor(nbPas, 9, 11, 10, 6);

//variables
int degres = 0;
int vitesse = 0;
int luminosite = 0;
boolean isStart = false;
boolean isRotate = false;
boolean isTakePicture = false;
boolean isFinishRotate = false;
int pas = 0;
int time = 0;

byte Degres[8] = {
  B00100,
  B01010,
  B00100,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000
};

/* Caractères personnalisés */
byte START_DIV_0_OF_1[8] = {
  B01111,
  B11000,
  B10000,
  B10000,
  B10000,
  B10000,
  B11000,
  B01111
}; // Char début 0 / 1

byte START_DIV_1_OF_1[8] = {
  B01111,
  B11000,
  B10011,
  B10111,
  B10111,
  B10011,
  B11000,
  B01111
}; // Char début 1 / 1

byte DIV_0_OF_2[8] = {
  B11111,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B11111
}; // Char milieu 0 / 2

byte DIV_1_OF_2[8] = {
  B11111,
  B00000,
  B11000,
  B11000,
  B11000,
  B11000,
  B00000,
  B11111
}; // Char milieu 1 / 2

byte DIV_2_OF_2[8] = {
  B11111,
  B00000,
  B11011,
  B11011,
  B11011,
  B11011,
  B00000,
  B11111
}; // Char milieu 2 / 2

byte END_DIV_0_OF_1[8] = {
  B11110,
  B00011,
  B00001,
  B00001,
  B00001,
  B00001,
  B00011,
  B11110
}; // Char fin 0 / 1

byte END_DIV_1_OF_1[8] = {
  B11110,
  B00011,
  B11001,
  B11101,
  B11101,
  B11001,
  B00011,
  B11110
};

void setup() {
  //initialisation
  Serial.begin(9600);
  pinMode(poten, INPUT);
  pinMode(enter, INPUT);
  pinMode(arret, INPUT);
  pinMode(photo, OUTPUT);
  pinMode(leds, OUTPUT);
  
  ecran.init();
  ecran.backlight();
  ecran.createChar(0, END_DIV_0_OF_1);
  ecran.createChar(1, END_DIV_1_OF_1);
  ecran.createChar(2, Degres);
  ecran.createChar(3, START_DIV_0_OF_1);
  ecran.createChar(4, START_DIV_1_OF_1);
  ecran.createChar(5, DIV_0_OF_2);
  ecran.createChar(6, DIV_1_OF_2);
  ecran.createChar(7, DIV_2_OF_2);
  //choix des paramètres
  int valmax = 0;
  //degres
  while (!analogTest(enter)) {
    int val = 10 * analogRead(poten) / 1023;
    val = val + 1;
    if (val == 11) {
      val = 10;
    }
    valmax = val;
    lcdprint("Nombre de degres", 0, 0, 0);
    lcdprint("par tour: " + String(val), 1, 0, 0);
    ecran.write(2);
    lcdprint("   ", '%', '%', 0);
    delay(250);
  }
  ecran.clear();
  degres = valmax;
  valmax = 0;
  //lâcher le bouton
  while (analogTest(enter)) {
    lcdprint("Rela", 0, 0, 0);
    lcdprint("chez le", '%', '%', 0);
    lcdprint("bouton", 1, 0, 0);
    delay(100);
  }
  //vitesse
  delay(150);
  ecran.clear();
  while (!analogTest(enter)) {
    int val = 9 * analogRead(poten) / 1023;
    val = val + 1;
    if (val == 10) {
      val = 9;
    }
    valmax = val;
    lcdprint("Vitesse de", 0, 0, 0);
    lcdprint("rotation: " + String(val), 1, 0, 0);
    delay(250);
  }
  ecran.clear();
  vitesse = valmax;
  //lâcher le bouton
  while (analogTest(enter)) {
    lcdprint("Rela", 0, 0, 0);
    lcdprint("chez le", '%', '%', 0);
    lcdprint("bouton", 1, 0, 0);
    delay(100);
  }
  delay(150);
  ecran.clear();
  motor.setSpeed(vitesse);
  //luminosité
  valmax = 0;
  while (!analogTest(enter)) {
    int val = 11 * analogRead(poten) / 1023;
    lcdprint("Luminosite", 0, 0, 0);
    if (val == 11) {
      val = 10;
    }
    lcdprint(":", '%', '%', 0);
    lcdprint(String(val), 1, 0, 0);
    lcdprint("   ", '%', '%', 0);
    valmax = map(analogRead(poten), 0, 1023, 0, 256);
    if (valmax == 256) {
      valmax = 255;
    }
    analogWrite(leds, valmax);
    delay(250);
  }
  ecran.clear();
  luminosite = valmax;
  analogWrite(leds, LOW);
  //lâcher le bouton
  while (analogTest(enter)) {
    lcdprint("Rela", 0, 0, 0);
    lcdprint("chez le", '%', '%', 0);
    lcdprint("bouton", 1, 0, 0);
    delay(100);
  }
  delay(150);
  ecran.clear();
  lcdprint("ENTER pour", 0, 0, 80);
  lcdprint("commencer", 1, 0, 80);
  int nbMinute = 0;
  int nbSeconde = 0;
  float t = estimedTime(2048);
  t = t / 1000;
  for (; t > 60;) {
    t = t - 60;
    nbMinute++;
  }
  nbSeconde = t;
  String text = String(nbMinute) + "m" + String(nbSeconde) + "s";
  lcdprint(text, 0, (16 - text.length()), 0);
  lcdprint(" ", 0, (15 - text.length()), 0);
}

void loop() {
  time++;
  if (isStart) {
    /*
      2s400ms -> 1000, v=9
      2s500ms -> 1000, v=8
      2s600ms -> 1000, v=7
      3s200ms -> 1000, v=6
      4s200ms -> 1000, v=5
      5s000ms -> 1000, v=4
      6s700ms -> 1000, v=3
      9s800ms -> 1000, v=2
      19s700ms -> 1000, v=1
    */
    int nbMinute = 0;
    int nbSeconde = 0;
    float t = estimedTime(2048 - pas);
    t = t / 1000;
    for (; t > 60;) {
      t = t - 60;
      nbMinute++;
    }
    nbSeconde = t;
    String text = String(nbMinute) + "m" + String(nbSeconde) + "s";
    lcdprint(text, 0, (16 - text.length()), 0);
    lcdprint(" ", 0, (15 - text.length()), 0);
    int pourcent = map(pas, 0, 2048, 0, 100);
    lcdprint(String(pourcent) + "%", 0, 0, 0);
    writePourcent(byte(pourcent));
    if (pas >= 2048) {
      isStart = false;
      isTakePicture = false;
      isRotate = false;
      pas = 0;
      isFinishRotate = false;
      ecran.clear();
      lcdprint("ENTER pour", 0, 0, 80);
      lcdprint("commencer", 1, 0, 80);
      int nbMinute = 0;
      int nbSeconde = 0;
      float t = estimedTime(2048);
      t = t / 1000;
      for (; t > 60;) {
        t = t - 60;
        nbMinute++;
      }
      nbSeconde = t;
      String text = String(nbMinute) + "m" + String(nbSeconde) + "s";
      lcdprint(text, 0, (16 - text.length()), 0);
      lcdprint(" ", 0, (15 - text.length()), 0);
      return;
    }
    if (!isRotate && !isTakePicture && !isFinishRotate) {
      motor.step(degres / 0.08); //4° -> 50, 2048 -> un tour
      pas = pas + degres / 0.08;
      isRotate = true;
      time = 0;
    }
    if (isRotate && !isTakePicture && !isFinishRotate && time == 15) {
      analogWrite(leds, luminosite);
      digitalWrite(photo, HIGH);
      isTakePicture = true;
      delay(500);
      digitalWrite(photo, LOW);
      time = 5;
    }
    if (isRotate && isTakePicture && !isFinishRotate && time == 18) {
      analogWrite(leds, LOW);
      isTakePicture = false;
      isRotate = false;
      isFinishRotate = false;
      time = 0;
    }
  }
  if (analogTest(enter) && !isStart) {
    ecran.clear();
    pas = 0;
    isStart = true;
  }
  if (analogTest(arret) && isStart) {
    isStart = false;
    isTakePicture = false;
    isRotate = false;
    isFinishRotate = false;
    pas = 0;
    ecran.clear();
    lcdprint("ENTER pour", 0, 0, 80);
    lcdprint("commencer", 1, 0, 80);
    int nbMinute = 0;
    int nbSeconde = 0;
    float t = estimedTime(2048);
    t = t / 1000;
    for (; t > 60;) {
      t = t - 60;
      nbMinute++;
    }
    nbSeconde = t;
    String text = String(nbMinute) + "m" + String(nbSeconde) + "s";
    lcdprint(text, 0, (16 - text.length()), 0);
    lcdprint(" ", 0, (15 - text.length()), 0);
  }
  delay(100);
}

boolean analogTest(int pin) {
  if (analogRead(pin) <= 1000) {
    return false;
  }
  else {
    return true;
  }
}

void lcdprint(String s, char cases, char ligne, int animation) {
  if (cases != '%' && ligne != '%') {
    ecran.setCursor(ligne, cases);
  }
  for (int i = 0; i < s.length(); i++) {
    ecran.print(s.substring(i, i + 1));
    delay(animation);
  }
}

float estimedTime(int nbPas) {
  /*
      2s400ms -> 1000, v=9
      2s500ms -> 1000, v=8
      2s600ms -> 1000, v=7
      3s200ms -> 1000, v=6
      4s200ms -> 1000, v=5
      5s000ms -> 1000, v=4
      6s700ms -> 1000, v=3
      9s800ms -> 1000, v=2
      19s700ms -> 1000, v=1
  */
  float var = 0;
  for (int h = 0; h < nbPas / 10; h++) {
    if (vitesse == 1) {
      var = var + 197;
    }
    if (vitesse == 2) {
      var = var + 98;
    }
    if (vitesse == 3) {
      var = var + 67;
    }
    if (vitesse == 4) {
      var = var + 50;
    }
    if (vitesse == 5) {
      var = var + 42;
    }
    if (vitesse == 6) {
      var = var + 32;
    }
    if (vitesse == 7) {
      var = var + 26;
    }
    if (vitesse == 8) {
      var = var + 25;
    }
    if (vitesse == 9) {
      var = var + 24;
    }
  }
  for (int h = 0; h < (nbPas / (degres / 0.08)); h++) {
    var = var + 4200;
  }
  return var;
}

void writePourcent(byte pourcent) {
  byte nb_columns = map(pourcent, 0, 100, 0, 16 * 2 - 2);
  // Chaque caractère affiche 2 barres verticales, mais le premier et dernier caractère n'en affiche qu'une.
  ecran.setCursor(0, 1);
  /* Dessine chaque caractère de la ligne */
  for (byte i = 0; i < 16; ++i) {

    if (i == 0) { // Premiére case

      /* Affiche le char de début en fonction du nombre de colonnes */
      if (nb_columns > 0) {
        ecran.write(4); // Char début 1 / 1
        nb_columns -= 1;

      } else {
        ecran.write((byte) 3); // Char début 0 / 1
      }

    } else if (i == 16 - 1) { // Derniére case

      /* Affiche le char de fin en fonction du nombre de colonnes */
      if (nb_columns > 0) {
        ecran.write(1); // Char fin 1 / 1

      } else {
        ecran.write(0); // Char fin 0 / 1
      }

    } else { // Autres cases

      /* Affiche le char adéquat en fonction du nombre de colonnes */
      if (nb_columns >= 2) {
        ecran.write(7); // Char div 2 / 2
        nb_columns -= 2;

      } else if (nb_columns == 1) {
        ecran.write(6); // Char div 1 / 2
        nb_columns -= 1;

      } else {
        ecran.write(5); // Char div 0 / 2
      }
    }
  }
}
