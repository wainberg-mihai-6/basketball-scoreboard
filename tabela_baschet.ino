#include <TM1637Display.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define CLK_MAIN_PIN 4
#define DIO_MAIN_PIN 5
#define CLK_SHOT_PIN 6
#define DIO_SHOT_PIN 7
#define BUZZER_PIN 13

#define BTN_A_PIN 8
#define BTN_B_PIN 9
#define BTN_CORR_PIN 10
#define BTN_S24_PIN 11
#define BTN_S14_PIN 12
#define BTN_BUZZ_PIN A0
#define BTN_SHOT_TG_PIN A1

#define BTN_GAME_PIN 2

TM1637Display gameDisp(CLK_MAIN_PIN, DIO_MAIN_PIN);
TM1637Display shotDisp(CLK_SHOT_PIN, DIO_SHOT_PIN);
LiquidCrystal_I2C lcd(0x27, 16, 2);

volatile bool gameRunning = false;
bool shotRunning = false;

volatile int gameSeconds = 600;
int shotSeconds = 24;

int scoreA = 0;
int scoreB = 0;

unsigned long lastButtonTime = 0;
unsigned long lastTick = 0;

int starePB8 = 0;
int starePB9 = 0;


static bool wasManualBuzz = false; //used to not cut off an auto-triggered tone 

void toggleGameTimer() {
  if (gameSeconds == 0 && !gameRunning) {
    gameSeconds = 600;
    shotSeconds=24;
    return;
  }
  gameRunning = !gameRunning;
}

void setup() {

  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(BTN_A_PIN, INPUT_PULLUP);
  pinMode(BTN_B_PIN, INPUT_PULLUP);
  pinMode(BTN_CORR_PIN, INPUT_PULLUP);
  pinMode(BTN_S24_PIN, INPUT_PULLUP);
  pinMode(BTN_S14_PIN, INPUT_PULLUP);
  pinMode(BTN_BUZZ_PIN, INPUT_PULLUP);
  pinMode(BTN_SHOT_TG_PIN, INPUT_PULLUP);

  //external interrupt
  pinMode(BTN_GAME_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(BTN_GAME_PIN), toggleGameTimer, FALLING);

  gameDisp.setBrightness(7);
  shotDisp.setBrightness(7);
  lcd.init();
  lcd.backlight();

  lcd.setCursor(0, 1);
  lcd.print(" SCOREBOARD READY ");
}

void loop() {
  unsigned long now = millis();

  bool pressA = (digitalRead(BTN_A_PIN) == LOW);
  bool pressB = (digitalRead(BTN_B_PIN) == LOW);
  bool pressCor = (digitalRead(BTN_CORR_PIN) == LOW);
  bool press24 = (digitalRead(BTN_S24_PIN) == LOW);
  bool press14 = (digitalRead(BTN_S14_PIN) == LOW);

  bool manualBuzz = (digitalRead(BTN_BUZZ_PIN) == LOW);
  bool pressShotT = (digitalRead(BTN_SHOT_TG_PIN) == LOW);

  if (manualBuzz) {
    tone(BUZZER_PIN, 1000);
    wasManualBuzz = true;
  } else if (wasManualBuzz) {
    noTone(BUZZER_PIN);
    wasManualBuzz = false;
  }

  unsigned long elapsed = now - lastButtonTime;

  if (elapsed > 200) {
    bool actionTaken = false;

    if (pressA) {
      if (starePB8 == 0) {
        starePB8 = 1;
        if (pressCor) scoreA = (scoreA > 0) ? scoreA - 1 : 0;
        else scoreA++;
        actionTaken = true;
      }
    } else {
      starePB8 = 0;
    }

    if (pressB) {
      if (starePB9 == 0) {
        starePB9 = 1;
        if (pressCor) scoreB = (scoreB > 0) ? scoreB - 1 : 0;
        else scoreB++;
        actionTaken = true;
      }
    } else {
      starePB9 = 0;
    }

    if (pressShotT) {
      shotRunning = !shotRunning;
      actionTaken = true;
    }

    if (press24) {
      shotSeconds = 24;
      actionTaken = true;
    }

    if (press14) {
      shotSeconds = 14;
      actionTaken = true;
    }

    if (actionTaken) {
      lastButtonTime = now;
    }
  }

  if (now - lastTick >= 1000) {
    lastTick = now;

    if (gameRunning && gameSeconds > 0) {
      gameSeconds--;
      if (gameSeconds == 0) {
        gameRunning=false;
        for (int i = 0; i < 3; i++) {
          tone(BUZZER_PIN, 2000, 150);
          delay(250);
        }
      }
    }

    if (shotRunning && shotSeconds > 0) {
      shotSeconds--;
      if (shotSeconds == 0) {
        tone(BUZZER_PIN, 2000, 1000);
      }
    }

    lcd.setCursor(0, 0);
    lcd.print("A:");
    lcd.print(scoreA);
    lcd.print(" ");
    lcd.setCursor(9, 0);
    lcd.print("B:");
    lcd.print(scoreB);
    lcd.print(" ");

    int mm = gameSeconds / 60;
    int ss = gameSeconds % 60;
    gameDisp.showNumberDecEx(mm * 100 + ss, 0b01000000, true);  

    shotDisp.showNumberDec(shotSeconds, true);
  }
}