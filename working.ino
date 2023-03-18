#include <EEPROM.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x3F, 16, 2);

#include <SoftwareSerial.h>
SoftwareSerial GSM(10, 11);

#define buzzer 7
#define relay 8
#define bt_theft A0

// #define pulse_in 2

char inchar;
int LDR_value;  //variable to store result value read by LDR
int counter = 0;

int unt_a = 0, unt_b = 0, unt_c = 0, unt_d = 0;
long total_unt = 7;

int price = 0;
long price1 = 0;

int Set = 10;

int pulse = 0;

///////////////// LDR ////////////////////
#define LDR A2  //Light dependent resisotr (LDR) connected to analog pin A2
#define LED 2   //LED connected to digital pin 2
// const uint8_t analogPin = A1;
const uint8_t ledPin = LED_BUILTIN;


String phone_no1 = "+917710821947";
String phone_no2 = "+917718991947";

int flag1 = 0, flag2 = 0, flag3 = 0, lcd1 = 0;

void setup() {
  Serial.begin(9600);
  GSM.begin(9600);

  pinMode(bt_theft, INPUT_PULLUP);
  pinMode(relay, OUTPUT);
  pinMode(buzzer, OUTPUT);

  // pinMode(pulse_in, INPUT);
  pinMode(LED, OUTPUT);  // set LED as output
  pinMode(LDR, INPUT);   //set light dependent resistor as input
  attachInterrupt(digitalPinToInterrupt(2), ai0, RISING);

  lcd.begin();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(5, 0);
  lcd.print("WELCOME");
  lcd.setCursor(4, 1);
  lcd.print("Auto Meter");
  digitalWrite(buzzer, HIGH);

  Serial.println("Initializing....");
  initModule("AT", "OK", 1000);
  initModule("ATE1", "OK", 1000);
  initModule("AT+CPIN?", "READY", 1000);
  initModule("AT+CMGF=1", "OK", 1000);
  initModule("AT+CNMI=2,2,0,0,0", "OK", 1000);
  Serial.println("Initialized Successfully");

  sendSMS(phone_no1, "Welcome To Energy Meter");
  digitalWrite(buzzer, LOW);
  lcd.clear();

  if (EEPROM.read(50) == 0) {
  } else {
    Write();
  }

  EEPROM.write(50, 0);

  pulse = EEPROM.read(10);

  Read();
  if (total_unt > 0) {
    digitalWrite(relay, HIGH);
  }
}




void loop() {

  LDR_value = analogRead(LDR);
  // Serial.println(LDR_value);
  // /////////////////// recharge ////////////////////////////////////////
  if (GSM.available() > 0) {

    inchar = GSM.read();
    // Serial.println("available");
    // Serial.print("inchar : ");
    // Serial.println(inchar);
    if (inchar == 'A') {
      // Serial.print("available");
      inchar = GSM.read();
      if (inchar == 'T') {
        inchar = GSM.read();
        if (inchar == 'r') {
          inchar = GSM.read();
          if (inchar == 'e') {
            inchar = GSM.read();
            if (inchar == 'c') {
              inchar = GSM.read();
              if (inchar == 'h') {
                inchar = GSM.read();
                if (inchar == '1') {
                  price = 100 / Set;
                  total_unt = total_unt + price;
                  sendSMS(phone_no1, "Your Recharge Done: Rs 100");
                  sendSMS(phone_no2, "Your Recharge Done: Rs 100");
                  load_on();
                } else if (inchar == '2') {
                  price = 200 / Set;
                  total_unt = total_unt + price;
                  sendSMS(phone_no1, "Your Recharge Done: Rs 200");
                  sendSMS(phone_no2, "Your Recharge Done: Rs 200");
                  load_on();
                } else if (inchar == '3') {
                  price = 300 / Set;
                  total_unt = total_unt + price;
                  sendSMS(phone_no1, "Your Recharge Done: Rs 300");
                  sendSMS(phone_no2, "Your Recharge Done: Rs 300");
                  load_on();
                } else if (inchar == '4') {
                  price = 400 / Set;
                  total_unt = total_unt + price;
                  sendSMS(phone_no1, "Your Recharge Done: Rs 400");
                  sendSMS(phone_no2, "Your Recharge Done: Rs 400");
                  load_on();
                }
              }
            }
          }
        }
      }
    }


    /////// DATA command to send data on consumer number //////////
    else if (inchar == 'D') {
      inchar = GSM.read();
      if (inchar == 'a') {
        inchar = GSM.read();
        if (inchar == 't') {
          inchar = GSM.read();
          if (inchar == 'a') {
            Data();
          }
        }
      }
    }
  }

  if (LDR_value <= 10) {
    digitalWrite(LED, LOW);  //turn on the LED
    // Serial.println("its dark turn on the LED");
    // Serial.println("LDR value:");
    // Serial.println(LDR_value);

  } else {
    digitalWrite(LED, HIGH);  //turn off the LED
    // Serial.println("its bright turn off the LED");
    // Serial.println("LDR value:");
    // Serial.println(LDR_value);
  }
  // delay(2000);





  ////////////////// printing on LCD ///////////////////////////////
  lcd.setCursor(0, 0);
  lcd.print("Unit:");
  lcd.print(total_unt);
  lcd.print("   ");

  lcd.setCursor(0, 1);
  lcd.print("Price:");
  lcd.print(price1);
  lcd.print("   ");

  lcd.setCursor(11, 0);
  lcd.print("Pulse");

  lcd.setCursor(13, 1);
  lcd.print(pulse);
  lcd.print("   ");


  ///////////////// Balance is Low ///////////////////
  if (total_unt == 5) {
    if (flag1 == 0) {
      flag1 = 1;
      digitalWrite(buzzer, HIGH);
      sendSMS(phone_no1, "Your Balance is Low Please Recharge");
      digitalWrite(buzzer, LOW);
    }
  }


  ///////////// Balance is Finish////////////////////
  if (total_unt == 0) {
    Serial.print("total_unt: ");
    Serial.println(total_unt);
    digitalWrite(relay, LOW);
    if (flag2 == 0) {
      flag2 = 1;
      digitalWrite(buzzer, HIGH);
      sendSMS(phone_no1, "Your Balance is Finished Please Recharge");
      digitalWrite(buzzer, LOW);
    }
  }

  // /////////// "Theft Alarm", send sms on company number ///////////////////////
  if (digitalRead(bt_theft) == 1) {
    if (flag3 == 0) {
      lcd1 = 0;
      sendSMS(phone_no2, "Theft Alarm");
      digitalWrite(relay, LOW);
      lcd.clear();
      lcd.setCursor(6, 0);
      lcd.print("Theft");
      flag3 = 1;
    }

  } else {
    flag3 = 0;
    if (lcd1 == 0) {
      lcd.clear();
      lcd1 = 1;
    }
    digitalWrite(relay, HIGH);
  }
  ///////////////////////////////////////////////// loop end ///////////////////////////////////////////////////////
}


/////////////// Turns relay high to turn on load //////////////////////////
void load_on() {
  Write();
  Read();
  digitalWrite(relay, HIGH);
  flag1 = 0, flag2 = 0;
}

void sendSMS(String number, String msg) {
  GSM.print("AT+CMGS=\"");
  GSM.print(number);
  GSM.println("\"\r\n");
  delay(500);
  GSM.println(msg);
  delay(500);
  GSM.write(byte(26));
  // delay(5000);
}


/////////////////////// send data to consumer number ///////////////////////////////
void Data() {
  GSM.print("AT+CMGS=\"");
  GSM.print(phone_no1);
  GSM.println("\"\r\n");
  // delay(1000);
  GSM.print("Unit:");
  GSM.println(total_unt);
  GSM.print("Price:");
  GSM.println(price1);
  delay(500);
  GSM.write(byte(26));  // (signals end of message)
  // delay(5000);
}

void Read() {
  unt_a = EEPROM.read(1);
  unt_b = EEPROM.read(2);
  unt_c = EEPROM.read(3);
  unt_d = EEPROM.read(4);
  total_unt = unt_d * 1000 + unt_c * 100 + unt_b * 10 + unt_a;
  price1 = total_unt * Set;
}

void Write() {
  unt_d = total_unt / 1000;
  total_unt = total_unt - (unt_d * 1000);
  unt_c = total_unt / 100;
  total_unt = total_unt - (unt_c * 100);
  unt_b = total_unt / 10;
  unt_a = total_unt - (unt_b * 10);

  EEPROM.write(1, unt_a);
  EEPROM.write(2, unt_b);
  EEPROM.write(3, unt_c);
  EEPROM.write(4, unt_d);
}


void initModule(String cmd, char *res, int t) {
  while (1) {
    Serial.println(cmd);
    GSM.println(cmd);
    delay(100);
    while (GSM.available() > 0) {
      if (GSM.find(res)) {
        Serial.println(res);
        // delay(t);
        return;
      } else {
        Serial.println("Error");
      }
    }
    // delay(t);
  }
}

void ai0() {
  if (digitalRead(LED) == HIGH) {
    pulse = pulse + 1;
    Serial.print("pulse : ");
    Serial.println(pulse);
    counter++;
    Serial.print("counter : ");
    Serial.println(counter);
    if (pulse > 9) {
      pulse = 0;
      if (total_unt > 0) {
        total_unt = total_unt - 1;
      }

      // writes the pulse data on  eeprom
      Write();
      Read();
    }
    EEPROM.write(10, pulse);
  }
}