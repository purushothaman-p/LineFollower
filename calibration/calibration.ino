#include<EEPROM.h>
#include <QTRSensors.h>

#define NUM_SENSORS  8      // number of sensors used
#define TIMEOUT      4   // waits for 2500 us for sensor outputs to go low
#define EMITTER_PIN  QTR_NO_EMITTER_PIN    // emitter is not controlled
QTRSensorsAnalog qtra((unsigned char[]){0, 1, 2, 3, 4, 5, 6, 7} ,NUM_SENSORS, TIMEOUT, EMITTER_PIN);  

struct calib_data{
 unsigned int Min[8], Max[8]; 
}cal;

void Calibrate() {
  // put your setup code here, to run once:
  pinMode(12, INPUT);
  pinMode(13, OUTPUT);
  Serial.begin(38400);  
  if(digitalRead(12) == HIGH){
      Serial.println("\nCallibrating");
      for (int i = 0; i < 300; i++){
        qtra.calibrate();
        delay(20);
      }

      for (int i = 0; i < 8; i++){
        cal.Min[i] = qtra.calibratedMinimumOn[i];
        cal.Max[i] = qtra.calibratedMaximumOn[i];
      }
       EEPROM.put(0, cal);
       delay(50);
       Serial.println("Calibrated");
  }
  else{
    Serial.println("Callibrated values");
    qtra.calibrate();
    EEPROM.get(0, cal);
    for(int i = 0; i<8; i++){
      Serial.print(cal.Min[i]); 
      Serial.print('\t');
    }
    Serial.println();
    for(int i = 0; i<8; i++){
      Serial.print(cal.Max[i]); 
      Serial.print('\t');
    }
    Serial.println();
  }
  digitalWrite(13, HIGH);
}
void setup(){
  Calibrate();
}

void loop() {
  // put your main code here, to run repeatedly:

}
