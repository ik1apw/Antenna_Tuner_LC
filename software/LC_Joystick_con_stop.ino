#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <EEPROM.h>
#include <Encoder.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);
Encoder myEnc(3, 4);
const int buttonPin = 5;

// --- Motori ---
#define POT1_CW 6
#define POT1_CCW 7
#define POT2_CW 8
#define POT2_CCW 9

// --- Joystick ---
#define JOY_X A2
#define JOY_Y A3
#define JOY_SW 10
int JOY_MIN_X = 400;
int JOY_MAX_X = 600;
int JOY_MIN_Y = 400;
int JOY_MAX_Y = 600;

int pot1Val, pot2Val;
int memIndex = 0;
int lastMemIndex = -1;
long oldPosition = -999;
bool lastButtonState = HIGH;
bool autoMode = false;
bool calibMode = false;
const int tolerance = 1;
unsigned long buttonPressTime = 0;

unsigned long autoStartTime = 0;
const unsigned long autoTimeout = 30000;

bool movVisible = true;
unsigned long lastBlinkTime = 0;
const unsigned long blinkInterval = 300;

int pot1_min = 0, pot1_max = 1023;
int pot2_min = 0, pot2_max = 1023;

#define MAX_MEM 20
#define EEPROM_START 0
#define CALIB_EEPROM 900
#define EEPROM_MAGIC 0xA5A5

struct Memoria { byte pot1; byte pot2; };
Memoria memories[MAX_MEM];

byte arrowUp[8]   = {0b00100,0b01110,0b10101,0b00100,0b00100,0b00100,0b00100,0b00000};
byte arrowDown[8] = {0b00100,0b00100,0b00100,0b00100,0b10101,0b01110,0b00100,0b00000};

// ----------------------------------------------------------

void printAligned(int value, int digits) {
  if (digits == 3) { if (value < 10) lcd.print("  "); else if (value < 100) lcd.print(" "); }
  else if (digits == 2) { if (value < 10) lcd.print(" "); }
  lcd.print(value);
}

void saveMemory(int index){ if(index>=0 && index<MAX_MEM) EEPROM.put(EEPROM_START+index*sizeof(Memoria),memories[index]); }
void loadMemory(int index){ if(index>=0 && index<MAX_MEM) EEPROM.get(EEPROM_START+index*sizeof(Memoria),memories[index]); }
void saveCalibration(){ EEPROM.put(CALIB_EEPROM,pot1_min); EEPROM.put(CALIB_EEPROM+4,pot1_max);
  EEPROM.put(CALIB_EEPROM+8,pot2_min); EEPROM.put(CALIB_EEPROM+12,pot2_max); }
void loadCalibration(){ int check; EEPROM.get(CALIB_EEPROM+16,check);
  if(check==EEPROM_MAGIC){ EEPROM.get(CALIB_EEPROM,pot1_min); EEPROM.get(CALIB_EEPROM+4,pot1_max);
    EEPROM.get(CALIB_EEPROM+8,pot2_min); EEPROM.get(CALIB_EEPROM+12,pot2_max);} }
void markEEPROMValid(){ int c=EEPROM_MAGIC; EEPROM.put(CALIB_EEPROM+16,c); }
void resetMemories(){ for(int i=0;i<MAX_MEM;i++){memories[i].pot1=0;memories[i].pot2=0;
  EEPROM.put(EEPROM_START+i*sizeof(Memoria),memories[i]);} }

bool confirmReset(){
  lcd.clear(); lcd.setCursor(0,0); lcd.print("Reset Memories?");
  int choice=0; lcd.setCursor(0,1); lcd.print(">YES  NO   ");
  long lastEnc=myEnc.read()/4;
  while(true){
    long enc=myEnc.read()/4;
    if(enc!=lastEnc){
      lastEnc=enc; choice=constrain(enc,0,1);
      lcd.setCursor(0,1);
      if(choice==0) lcd.print(">YES  NO   "); else lcd.print(" YES  >NO   ");
    }
    bool btnState=digitalRead(buttonPin);
    if(btnState==LOW){ delay(50); while(digitalRead(buttonPin)==LOW); return (choice==0); }
  }
}

// ----------------------------------------------------------

void setup() {
  lcd.init(); lcd.backlight();
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(JOY_SW, INPUT_PULLUP);
  pinMode(POT1_CW, OUTPUT); pinMode(POT1_CCW, OUTPUT);
  pinMode(POT2_CW, OUTPUT); pinMode(POT2_CCW, OUTPUT);

  digitalWrite(POT1_CW,LOW); digitalWrite(POT1_CCW,LOW);
  digitalWrite(POT2_CW,LOW); digitalWrite(POT2_CCW,LOW);

  for(int i=0;i<MAX_MEM;i++) loadMemory(i);
  loadCalibration();

  lcd.createChar(0,arrowUp); lcd.createChar(1,arrowDown);
  lcd.setCursor(0,0); lcd.print("System Init...");
  delay(500);

  // --- Calibrazione iniziale joystick ---
  int xCenter = analogRead(JOY_X);
  int yCenter = analogRead(JOY_Y);
  JOY_MIN_X = xCenter - 90;  JOY_MAX_X = xCenter + 90;
  JOY_MIN_Y = yCenter - 90;  JOY_MAX_Y = yCenter + 90;

  lcd.setCursor(0,1); lcd.print("Joystick Cal OK");
  delay(800); lcd.clear();

  digitalWrite(POT1_CW,LOW); digitalWrite(POT1_CCW,LOW);
  digitalWrite(POT2_CW,LOW); digitalWrite(POT2_CCW,LOW);
  delay(1000);
}

// ----------------------------------------------------------

void loop() {
  pot1Val = map(analogRead(A0),0,1023,0,100);
  pot2Val = map(analogRead(A1),0,1023,0,100);

  long newPosition=myEnc.read()/4;
  if(newPosition!=oldPosition){ oldPosition=newPosition; memIndex=constrain(newPosition,0,MAX_MEM-1); }

  // Lettura joystick diretta
  int joyX = analogRead(JOY_X);
  int joyY = analogRead(JOY_Y);

  bool buttonState=digitalRead(buttonPin);
  if(buttonState==LOW && lastButtonState==HIGH) buttonPressTime=millis();
  else if(buttonState==HIGH && lastButtonState==LOW){
    unsigned long pressDuration=millis()-buttonPressTime;

    if(calibMode){
      if(pressDuration>15000){
        saveCalibration(); markEEPROMValid();
        lcd.clear(); lcd.setCursor(0,0); lcd.print("Calibration OK");
        lcd.setCursor(0,1); lcd.print("Values Saved!"); delay(2000);
        calibMode=false; lcd.clear();
      } else if(pressDuration<3000){
        pot1_min=0; pot1_max=1023; pot2_min=0; pot2_max=1023;
        lcd.clear(); lcd.setCursor(0,0); lcd.print("Reset Calibration");
        lcd.setCursor(0,1); lcd.print("Restart P1 e P2"); delay(1500); lcd.clear();
      }
    } else {
      if(pressDuration>=30000){ calibMode=true; lcd.clear(); }
      else if(pressDuration>=15000){
        if(confirmReset()){
          resetMemories(); lcd.clear();
          lcd.setCursor(0,0); lcd.print("All Memories");
          lcd.setCursor(0,1); lcd.print("Reset!"); delay(2000);
        } else lcd.clear();
      } 
      else if(pressDuration>=6000){
        memories[memIndex].pot1=pot1Val; memories[memIndex].pot2=pot2Val;
        saveMemory(memIndex);
        lcd.setCursor(0,1);
        lcd.print("M"); if(memIndex<10) lcd.print("0");
        lcd.print(memIndex); lcd.print(" ");
        lcd.print(pot1Val); lcd.print("/"); lcd.print(pot2Val); lcd.print(" Saved!");
        delay(1000); lcd.setCursor(0,1); lcd.print("                ");
      } 
      else if(pressDuration>=2000){
        autoMode=true; autoStartTime=millis(); movVisible=true; lastBlinkTime=millis();
      }
    }
  }
  lastButtonState=buttonState;

  // --- STOP emergenza joystick ---
  if (autoMode && digitalRead(JOY_SW) == LOW) {
    autoMode = false;
    digitalWrite(POT1_CW, LOW);
    digitalWrite(POT1_CCW, LOW);
    digitalWrite(POT2_CW, LOW);
    digitalWrite(POT2_CCW, LOW);
    lcd.setCursor(13,1); lcd.print("STP");
    delay(500);
    lcd.setCursor(13,1); lcd.print("   ");
  }

  if(calibMode){
    int raw1=analogRead(A0), raw2=analogRead(A1);
    if(raw1<pot1_min) pot1_min=raw1; if(raw1>pot1_max) pot1_max=raw1;
    if(raw2<pot2_min) pot2_min=raw2; if(raw2>pot2_max) pot2_max=raw2;
    int pot1_c=map(raw1,pot1_min,pot1_max,0,100);
    int pot2_c=map(raw2,pot2_min,pot2_max,0,100);
    if(millis()-lastBlinkTime>=blinkInterval){ movVisible=!movVisible; lastBlinkTime=millis(); }
    lcd.setCursor(0,0); lcd.print("P1:"); printAligned(pot1_c,3);
    lcd.setCursor(0,1); lcd.print("P2:"); printAligned(pot2_c,3);
    lcd.setCursor(13,1); if(movVisible) lcd.print("CAL"); else lcd.print("   ");
    digitalWrite(POT1_CW,LOW); digitalWrite(POT1_CCW,LOW);
    digitalWrite(POT2_CW,LOW); digitalWrite(POT2_CCW,LOW);
    delay(100); return;
  }

  char dirP1=' ', dirP2=' ';
  if(autoMode){
    if(millis()-autoStartTime>autoTimeout) autoMode=false;
    if(pot1Val<memories[memIndex].pot1-tolerance){
      digitalWrite(POT1_CW,HIGH); digitalWrite(POT1_CCW,LOW); dirP1=char(0);
    } else if(pot1Val>memories[memIndex].pot1+tolerance){
      digitalWrite(POT1_CW,LOW); digitalWrite(POT1_CCW,HIGH); dirP1=char(1);
    } else { digitalWrite(POT1_CW,LOW); digitalWrite(POT1_CCW,LOW); dirP1=' '; }

    if(pot2Val<memories[memIndex].pot2-tolerance){
      digitalWrite(POT2_CW,HIGH); digitalWrite(POT2_CCW,LOW); dirP2=char(0);
    } else if(pot2Val>memories[memIndex].pot2+tolerance){
      digitalWrite(POT2_CW,LOW); digitalWrite(POT2_CCW,HIGH); dirP2=char(1);
    } else { digitalWrite(POT2_CW,LOW); digitalWrite(POT2_CCW,LOW); dirP2=' '; }

    if(abs(pot1Val-memories[memIndex].pot1)<=tolerance &&
       abs(pot2Val-memories[memIndex].pot2)<=tolerance) autoMode=false;
    if(millis()-lastBlinkTime>=blinkInterval){ movVisible=!movVisible; lastBlinkTime=millis(); }
  } else {
    bool p1Active=false, p2Active=false;

    // ⚙️ ASSE SCAMBIATI (X controlla P1, Y controlla P2)
    if(joyX < JOY_MIN_X){ digitalWrite(POT1_CW,HIGH); digitalWrite(POT1_CCW,LOW); dirP1=char(0); p1Active=true; }
    else if(joyX > JOY_MAX_X){ digitalWrite(POT1_CW,LOW); digitalWrite(POT1_CCW,HIGH); dirP1=char(1); p1Active=true; }

    if(joyY > JOY_MAX_Y){ digitalWrite(POT2_CW,HIGH); digitalWrite(POT2_CCW,LOW); dirP2=char(0); p2Active=true; }
    else if(joyY < JOY_MIN_Y){ digitalWrite(POT2_CW,LOW); digitalWrite(POT2_CCW,HIGH); dirP2=char(1); p2Active=true; }

    if(!p1Active){ digitalWrite(POT1_CW,LOW); digitalWrite(POT1_CCW,LOW); }
    if(!p2Active){ digitalWrite(POT2_CW,LOW); digitalWrite(POT2_CCW,LOW); }
    movVisible=false;
  }

  lcd.setCursor(0,0);
  lcd.print("P1:"); printAligned(pot1Val,3); lcd.write(dirP1);
  lcd.print(" P2:"); printAligned(pot2Val,3); lcd.write(dirP2);

  lcd.setCursor(15,0);
  int check; EEPROM.get(CALIB_EEPROM+16,check);
  lcd.print((check==EEPROM_MAGIC)?"*":" ");

  if(memIndex!=lastMemIndex){ lcd.setCursor(0,1); lcd.print("                "); lastMemIndex=memIndex; }
  lcd.setCursor(0,1);
  lcd.print("M"); if(memIndex<10) lcd.print("0"); lcd.print(memIndex); lcd.print("->");
  if(memories[memIndex].pot1<10) lcd.print(" "); lcd.print(memories[memIndex].pot1);
  lcd.print("/"); if(memories[memIndex].pot2<10) lcd.print(" "); lcd.print(memories[memIndex].pot2);
  lcd.setCursor(13,1); if(movVisible) lcd.print("MOV"); else lcd.print("   ");

  delay(20);  // ⚡ refresh veloce 50 Hz
}
