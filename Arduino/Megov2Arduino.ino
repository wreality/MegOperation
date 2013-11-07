#include <EEPROMEx.h>
#define CONFIG_VERSION "lA2"
#define memoryBase 32

bool ok  = true;
int configAddress = 0;

byte maxPins = 52;
byte searchPins[52];

// Example settings structure
struct StoreStruct {
  char version[4];   
  byte faultPins[12], successPins[12]; 
};
StoreStruct storage = { 
  CONFIG_VERSION,
  {
    0,0,0,0,0,0,0,0,0,0,0,0  }
  ,
  {
    0,0,0,0,0,0,0,0,0,0,0,0  }
};


unsigned long pinMap[12][3] = {
  {
    10,'A', 0  }
  ,
  {
    11,'B', 0  }
  ,
  {
    12,'C', 0  }
  ,
  {
    13,'D', 0  }
  ,
  {
    14,'E', 0  }
  ,
  {
    15,'F', 0  }
  ,
  {
    16,'G', 0  }
  ,
  {
    17,'H', 0  }
  ,
  {
    6,'I', 0  }
  ,
  {
    5,'J', 0  }
  ,
  {
    4,'K', 0  }
  ,
  {
    8,'L', 0  }
  ,
};

unsigned long switchMap[11][3] = {
  {
    22, 'M', 0  }
  ,
  {
    23, 'N', 0  }
  ,
  {
    24, 'O', 0  }
  ,
  {
    25, 'P', 0  }
  ,
  {
    26, 'Q', 0  }
  ,
  {
    27, 'R', 0  }
  ,
  {
    28, 'S', 0  }
  ,
  {
    29, 'T', 0  }
  ,
  {
    30, 'U', 0  }
  ,
  {
    31, 'V', 0  }
  ,
  {
    32, 'W', 0  }
  ,
};


int lampPin = 3;

int btnPin = 13;
int bellPin = 9;
unsigned long soundBell = 0;
int soundLength = 250;
int serialDelay = 500;
int btnBouncePause = 500;

unsigned long btnSent = 0;


void setup() {
  //EEPROM.setMaxAllowedWrites(200);
  Serial.begin(9600);
  EEPROM.setMemPool(memoryBase, EEPROMSizeMega); //Set memorypool base to 32, assume Arduino Uno board
  configAddress  = EEPROM.getAddress(sizeof(StoreStruct)); // Size of config object 
  pinMode(btnPin, INPUT_PULLUP);
  pinMode(bellPin, OUTPUT);
  pinMode(lampPin, OUTPUT);

  ok = loadConfig();
  if (!ok) { 
    configureModeStart(); 
  }
  if (!digitalRead(btnPin)) { 
    delay(5000);
    if (!digitalRead(btnPin)) {
      configureModeStart();
    }
  }
  Serial.println(lampPin);
  digitalWrite(bellPin, HIGH);
  delay (100);
  digitalWrite(bellPin, LOW);

}

void loop() {
  if (millis() < (soundBell + soundLength)) {
    //Sound the bell
    digitalWrite(bellPin, HIGH);
    digitalWrite(lampPin, HIGH);
  } 
  else {
    digitalWrite(bellPin, LOW);
    digitalWrite(lampPin, LOW);
  }
  for (int i = 0; i < 12; i++) {
    //Poll Wiretriggers
    if ((!digitalRead(pinMap[i][0])) && (millis() > (pinMap[i][2] + serialDelay))) {
      soundBell = millis();
      Serial.print(char(pinMap[i][1]));   
      pinMap[i][2] = millis();
    }
    /*
    //Poll Piece switches
     if (i == 11) continue;
     if ((!digitalRead(switchMap[i][0])) && (millis() > (switchMap[i][2] + serialDelay))) {
     Serial.print(char(switchMap[i][1]));
     switchMap[i][2] = millis();
     }
     */
  }  


  if (!digitalRead(btnPin)) {
    if ((millis() > (btnSent + btnBouncePause))) {
      Serial.print("Z");
    }
    btnSent = millis();
  } 
  else {
    btnSent = 0;
  }
}

bool loadConfig() {
  EEPROM.readBlock(configAddress, storage);
  for (int i = 0; i<=12; i++) {
    pinMap[i][0] = storage.faultPins[i];
    pinMode(storage.faultPins[i], INPUT_PULLUP);
  }
  /*
  for (int i = 0; i<=11; i++) {
   switchMap[i][0] = storage.successPins[i];
   pinMode(storage.successPins[i], INPUT_PULLUP);
   }
   */
  return (strcmp(storage.version, CONFIG_VERSION)) == 0;
}

void saveConfig(struct StoreStruct settings) {
  Serial.println(configAddress);
  Serial.println(settings.version);
  Serial.println(EEPROM.writeBlock(configAddress, settings));
}


int configureFindPin() {
  while (true) {

    for (int i=0;i<maxPins;i++){
      if (!searchPins[i]) continue;
      if (!digitalRead(i+1)) {
        delay(100);
        if (!digitalRead(i+1)) {
          searchPins[i] = false;
          Serial.print("Pin ");
          Serial.print(i+1);
          Serial.println("");
          return int(i+1);

        }
      }
    } 
  }
}

void configureModeStart() {
  for (int i=0;i<maxPins;i++) {
    switch (i+1) {
    case 13:
    case 9:
    case 3:
      searchPins[i] = false;
      break;
    default:
      digitalWrite(i+1, HIGH);
      searchPins[i] = true;
      break;
    }
  }
  StoreStruct settings = { 
    CONFIG_VERSION,
    {
      0,0,0,0,0,0,0,0,0,0,0,0    }
    ,
    {
      0,0,0,0,0,0,0,0,0,0,0,0    }
  };
  digitalWrite(bellPin, HIGH);
  delay(150);
  digitalWrite(bellPin, LOW);
  delay(300);
  digitalWrite(bellPin, HIGH);
  delay(150);
  digitalWrite(bellPin, LOW);
  Serial.print("\n\rEntering Configuration Mode\n\r\n\r");
  Serial.print("Fault Wires\n\r");
  Serial.print("===========\n\r");
  Serial.print("Touch Ground to Each Pin\n\r");

  Serial.print("Adam's Apple: ");
  settings.faultPins[0] = configureFindPin();
  Serial.print("Wishbone: ");
  settings.faultPins[1] = configureFindPin();
  Serial.print("Broken Heart: ");
  settings.faultPins[2] = configureFindPin();
  Serial.print("Funny Bone: ");
  settings.faultPins[3] = configureFindPin();
  Serial.print("Spare Ribs: ");
  settings.faultPins[4] = configureFindPin();
  Serial.print("Butterflies: ");
  settings.faultPins[5] = configureFindPin();
  Serial.print("Writer's Cramp: ");
  settings.faultPins[6] = configureFindPin();
  Serial.print("Breadbasket: ");
  settings.faultPins[7] = configureFindPin();
  Serial.print("Charlie Horse: ");
  settings.faultPins[8] = configureFindPin();
  Serial.print("Water on Knee: ");
  settings.faultPins[9] = configureFindPin();
  Serial.print("Wrenched Ankle: ");
  settings.faultPins[10] = configureFindPin();
  Serial.print("Ankle Bone (Rubberband): ");
  settings.faultPins[11] = configureFindPin();
  /*
  Serial.print("\r\nSensors\r\n");
   Serial.print("=======\r\n");
   Serial.print("Touch Magnet to each Sensor\r\n");
   Serial.print("Adam's Apple: ");
   settings.successPins[0] = configureFindPin();
   Serial.print("Wishbone: ");
   settings.successPins[1] = configureFindPin();
   Serial.print("Broken Heart: ");
   settings.successPins[2] = configureFindPin();
   Serial.print("Funny Bone: ");
   settings.successPins[3] = configureFindPin();
   Serial.print("Spare Ribs: ");
   settings.successPins[4] = configureFindPin();
   Serial.print("Butterflies: ");
   settings.successPins[5] = configureFindPin();
   Serial.print("Writer's Cramp: ");
   settings.successPins[6] = configureFindPin();
   Serial.print("Breadbasket: ");
   settings.successPins[7] = configureFindPin();
   Serial.print("Charlie Horse: ");
   settings.successPins[8] = configureFindPin();
   Serial.print("Water on Knee: ");
   settings.successPins[9] = configureFindPin();
   Serial.print("Wrenched Ankle: ");
   settings.successPins[10] = configureFindPin();
   //  Serial.print("Ankle Bone (Rubberband): ");
   //  settings.successPins[11] = configureFindPin();
   */
  Serial.print("Saving config..Version: ");
  Serial.println(settings.version);
  saveConfig(settings);
  Serial.print("Reset Board or power cycle.\r\n");
  while(true){
  }

}


