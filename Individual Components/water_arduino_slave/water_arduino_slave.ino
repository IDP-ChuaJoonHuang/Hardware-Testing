bool is_water_level_dangerous = false;



void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(2, INPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  if(!digitalRead(2) == HIGH){
    //Serial.println("Warning");
    Serial.write((unsigned char)'1');
  }else{
    Serial.write((unsigned char)'0');
  }
}
