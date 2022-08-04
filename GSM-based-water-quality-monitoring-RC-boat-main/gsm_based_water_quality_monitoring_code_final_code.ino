//lcd display-buzzer-LEDs-sd card-gps-gsm-water sensor-temperature sensor-pH sensor-turbidity sensor
//lcd display 2 3 4 5 6 7, gsm 9 8, gps 10 11,buzzer 31, led1 33, led2 35, sd module 53 52 51 50, temperature sensor A0, water sensor A1, pH sensor A0, turbidity sensor A0
#include <SoftwareSerial.h>
#include <TinyGPS++.h>
#include <SPI.h>
#include <SD.h>
#include<LiquidCrystal.h>
#include <OneWire.h>
#include <DallasTemperature.h>

File myFile;
TinyGPSPlus gps;
static const uint32_t GPSBaud = 9600;

SoftwareSerial sgsm(8, 9);
SoftwareSerial sgps(11, 10);

#define led1 33
#define led2 35
#define Buzzer 31
#define SD_CSPin 53
#define ONE_WIRE_BUS A0
#define water_pin A1
#define pH_pin A2
#define turbidity_pin A3
LiquidCrystal lcd(6, 7, 2, 3, 4, 5);

OneWire oneWire(ONE_WIRE_BUS);  // Setup a oneWire instance to communicate with any OneWire device
DallasTemperature sensors(&oneWire); // Pass oneWire reference to DallasTemperature library

float Latitude, Longitude, temp_val, pH_val, tur_val, water_presences;
String  day, month, year, Date;
int temp_adc_val, turbidity_val; 
unsigned long int avgValue;
int buf[10],temp;
float b;

void setup(){
  Serial.begin(9600);
  sgsm.begin(9600);
  sgps.begin(GPSBaud);
  pinMode(water_pin,INPUT);
  pinMode(turbidity_pin,INPUT);
  pinMode(pH_pin,INPUT);
  pinMode(led1,OUTPUT);digitalWrite(led1,LOW);
  pinMode(led2,OUTPUT);digitalWrite(led2,LOW);
  pinMode(Buzzer,OUTPUT);digitalWrite(Buzzer,LOW);
  
  lcd.begin(16, 2);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(" WATER  QUALITY ");
  lcd.setCursor(0, 1);
  lcd.print("MEASURING RCBOAT");
  delay(1000);
  
  while (!Serial) {;}
  Serial.print("Initializing SD card...");
  if (!SD.begin(SD_CSPin)) { 
    lcd.clear();
    lcd.setCursor(4, 0);
    lcd.print("SD-CARD");
    lcd.setCursor(2, 1);
    lcd.print("Not Connected");
    delay(1000);
    Serial.println("initialization failed!");
    while (1);
    }
  Serial.println("initialization done.");
  lcd.clear();
  lcd.setCursor(4, 0);
  lcd.print("SD-CARD");
  lcd.setCursor(3, 1);
  lcd.print("Connected");
  delay(1000);

  sysstart();// send messege alert to user "system is started" 
     
  myFile = SD.open("text.txt", FILE_WRITE);
  if (myFile) {
    Serial.println("GSM BASED WATER QUALITY MONITORING RC BOAT is started");
    myFile.println( "   ");
    myFile.print( "GSM BASED WATER QUALITY MONITORING RC BOAT");
    myFile.println(" is started");
    myFile.print( "..........");
    myFile.println( "   ");
    myFile.close();
    }
    else {Serial.println("error opening text.txt");}
}

float getpH(){
  for(int i=0;i<10;i++)
  { 
    buf[i]=analogRead(pH_pin);
    delay(10);
  }
  for(int i=0;i<9;i++){
    for(int j=i+1;j<10;j++){
      if(buf[i]>buf[j]){
        temp=buf[i];
        buf[i]=buf[j];
        buf[j]=temp;
      }
    }
  }
  avgValue=0;
  for(int i=2;i<8;i++)
  avgValue+=buf[i];
  pH_val=(float)avgValue*5.0/1024/6;
  pH_val=3.5*pH_val;
  return pH_val;
}
float getturbidity() { 
  turbidity_val = analogRead(turbidity_pin);
  tur_val = turbidity_val* (5.0 / 1024.0);
  delay(100);
  return tur_val;
}
float gettemperature(){
  sensors.requestTemperatures(); 
  temp_val = sensors.getTempCByIndex(0); 
  return temp_val;
} 

void loop(){
  getpH();
  getturbidity();
  gettemperature();
  while (sgps.available()>0){
    int c = sgps.read();
    if (gps.encode(sgps.read())){gps_data();datastore();}
    }
  if(!sgps.available()){
    Latitude = 16.5661300;    
    Longitude = 81.5215360;
    datastore();
    }
    
  //water sensor with gps-gsm
  water_presences=analogRead(water_pin);
  if(water_presences<800){
    digitalWrite(Buzzer,HIGH);
    rescue();
    digitalWrite(Buzzer,LOW);
    Serial.println("about to be sink");
    }
  if(water_presences>800){
    lcddisplay();
    Serial.println("RC boat safe");
    }
    
  //pH & turbidity values were compare with ideal values, pH ,turbidity sensors with gps-gsm
  if((pH_val<6.5||pH_val>8.5) || (tur_val<3)){
    deviation();
    Serial.println("deviation happend");
    }
  }
  
void gps_data(){
  if (gps.location.isUpdated()) {
      digitalWrite(led2,HIGH);
      delay(500);
      digitalWrite(led2,LOW   );
      Latitude = gps.location.lat();   
      Longitude = gps.location.lng();
  }
  else{Serial.println("Location is not available");}
}
void lcddisplay(){
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("pH:");
  lcd.setCursor(3,0);
  lcd.print(pH_val);
  lcd.setCursor(9, 0);
  lcd.print("T:");
  lcd.setCursor(11,0);
  lcd.print(temp_val);
  lcd.print("C");
  lcd.setCursor(0, 1);
  lcd.print("TURB:");
  lcd.setCursor(7,1);
  lcd.print(tur_val);
  lcd.print(" NTU");
}
void datastore(){
  //Sd card module stores the data of sensors w.r.t location( pH, turbidity, temperature sensors and gps coordinates) 
  myFile = SD.open("text.txt", FILE_WRITE);
  if (myFile) {
    Serial.print("Writing to store data in text.txt...");
    myFile.print("pH value :");
    myFile.print(pH_val);
    myFile.print(" pH   ");
    myFile.print("Turbidity :");
    myFile.print(tur_val);
    myFile.print(" NTU   ");
    myFile.print("Temp :");
    myFile.print(temp_val);
    myFile.print(" °C   ");
    myFile.print("lat :");
    myFile.print(Latitude,7);
    myFile.print("  long :");
    myFile.println(Longitude,7);
    myFile.close();
    Serial.println("done.");
    
    Serial.print("pH value :");
    Serial.print(pH_val);
    Serial.print(" pH   ");
    Serial.print("Turbidity :");
    Serial.print(tur_val);
    Serial.print(" NTU   ");
    Serial.print("Temp :");
    Serial.print(temp_val);
    Serial.print(" °C   ");
    Serial.print("lat :");
    Serial.print(Latitude,7);
    Serial.print("  long :");
    Serial.println(Longitude,7);
    
    digitalWrite(led1,HIGH);
    delay(1000);
    digitalWrite(led1,LOW);
    } 
  else {Serial.println("error opening test.txt");}
  }
void deviation(){
  sgsm.print("\r");
  delay(1000);
  sgsm.print("AT+CMGF=1\r");
  delay(1000);
  sgsm.print("AT+CMGS=\"+917981669483\"\r");
  delay(1000);
  sgsm.print("pH :");
  sgsm.println(pH_val);
  sgsm.print("turbidity :");
  sgsm.println(tur_val);
  sgsm.print("temp :");
  sgsm.println(temp_val);
  sgsm.println("Pollution found in water in this location");
  sgsm.print("https://www.google.com/maps/?q=");
  sgsm.print(Latitude, 7);
  sgsm.print(",");
  sgsm.print(Longitude, 7);
  delay(500);
  sgsm.write(0x1A);
  delay(500);
  }
void rescue(){
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("SENDING MESSAGE");
  
  sgsm.print("\r");
  delay(1000);
  sgsm.print("AT+CMGF=1\r");
  delay(1000);
  sgsm.print("AT+CMGS=\"+917981669483\"\r");
  delay(1000);
  sgsm.println("RC Boat is about to be sink");
  sgsm.println("Rescue the boat");
  sgsm.print("https://www.google.com/maps/?q=");
  sgsm.print(Latitude, 7);
  sgsm.print(",");
  sgsm.print(Longitude, 7);
  delay(500);
  sgsm.write(0x1A);
  delay(500);
  
  lcd.setCursor(0, 1);
  lcd.print("MESSAGE SENT");
  delay(1000);
  }
void sysstart(){
  sgsm.print("\r");
  delay(1000);
  sgsm.print("AT+CMGF=1\r");
  delay(1000);
  sgsm.print("AT+CMGS=\"+917981669483\"\r");
  delay(1000);
  sgsm.println("Water Quality Monitoring system is start");
  delay(1000);
  sgsm.write(0x1A);
  delay(1000);
  }
