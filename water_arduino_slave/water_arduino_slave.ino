

//void loop() {
//  delay(100);
//  delay(1000);
//   if(!digitalRead(2) == HIGH){
//    //Serial.println("Warning");
//    Serial.print(1);
//  }else{
//    Serial.print(0);
//  }
//}

//#include <Wire.h>
//
//void setup() {
//  Wire.begin(8);                /* join i2c bus with address 8 */
//  //Wire.onReceive(receiveEvent); /* register receive event */
//  Wire.onRequest(requestEvent); /* register request event */
//  Serial.begin(9600);           /* start serial for debug */
//  pinMode(2, INPUT);
//}
//
//void loop(){
//  delay(100);
//  
//}
//
//void requestEvent() {
//  Wire.write(!digitalRead(2));  /*send string on request */
//  Serial.println("send");
//}

#include <Wire.h>

void setup() {
   Wire.begin(8);                /* join i2c bus with address 8 */
   Wire.onReceive(receiveEvent); /* register receive event */
   Wire.onRequest(requestEvent); /* register request event */
   Serial.begin(9600);           /* start serial for debug */
   pinMode(2, INPUT);
}

void loop() {
  delay(100);
}

// function that executes whenever data is received from master
void receiveEvent(int howMany) {
   while (0 <Wire.available()) {
      char c = Wire.read();      /* receive byte as a character */
      Serial.print(c);           /* print the character */
    }
   Serial.println();             /* to newline */
}

// function that executes whenever data is requested from master
void requestEvent() {
  Wire.write(!digitalRead(2));  /*send string on request */
}
