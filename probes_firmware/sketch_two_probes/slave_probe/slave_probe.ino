/*  @Description firmware that runs on the slave probe                               */
/*  @Version 0.1                                                                    */
/*  @author Aurelio Buarque  (abuarquemf@gmail.com)                                  */
/*  Created on July 6, 2015, 3:17 AM, Maceio-AL, Brazil                              */

//Used libraries
#include <Servo.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <RF24_config.h>

const double k = 0.0001; //math const to decrease the input values 
int myClass;
bool check = false;

int notMyClass;

//Objects servo 
Servo servoLeft, servoRight;

//Vector for the last string symbole
int msg[1];
//Object NRF24L01
RF24 myTransceiver(9,10); //CE, CSN
//comunication pipe
const uint64_t pipe = 0xE8E8F0F0E1LL;
//Final symbol of string
const int lastSymbol = 1;

//pin that buzzer is conected
const byte buzzerPin = 2;

//Pin of LDR
const byte LDRPin = A0;
//Pin of voltage sensor
const byte voltageSensorPin = A1;

//pin of HCSR04 trig
const byte trigPin = 12;
//pin of HCSR04 echo
const byte echoPin = 13;

void setup(void) {
  //Starts serial comunication 
  Serial.begin(9600);
  //Clean serial line
  Serial.flush();
  //starts the transceiver
  myTransceiver.begin();
  //define the comunication pipe
  myTransceiver.openReadingPipe(1, pipe);
  //Say to the transceiver start to listening
  myTransceiver.startListening();
  //define the servoLeft pin as 5
  servoLeft.attach(5);
  //define the servoRight pin as 6
  servoRight.attach(6);
  //define the buzzer pin as output
  pinMode(buzzerPin, OUTPUT);
  //Define the HCSR04 trig as output
  pinMode(trigPin, OUTPUT);
  //Define the HCSR04 echo as input
  pinMode(echoPin, INPUT);
}

/**
 * Function that gets the data from the master probe
 *
 * @return data entered
*/
String dataEntered(void) {
  //string that stores the data entered
  String data = "";
  //If there is something on the pipeline
  if(myTransceiver.available()) {
    bool done = false;
    done = myTransceiver.read(msg, 1);
    char theChar = msg[0];
    //if the current char isn't 2...
    if(msg[0] != 2) 
      data.concat(theChar);
    return data;
    data = "";
  }
}

/**
 * Function that moves the probe forward
 *
*/
void forward(int delayTime) {
  servoLeft.write(0);
  servoRight.write(180);
  delay(delayTime);
}

/**
 * Function that moves the probe backward
 *
*/
void backward(int delayTime) {
  servoLeft.write(180);
  servoRight.write(0);
  delay(delayTime);
}

/**
 * Function that moves the probe left
 *
*/
void left(int delayTime) {
  servoLeft.write(90);
  servoRight.write(180);
  delay(delayTime);
}

/**
 * Function that moves the probe right
 *
*/
void right(int delayTime) {
  servoLeft.write(180);
  servoRight.write(90);
  delay(delayTime);
}

/**
 * Function that makes the probe avoid of some object
 *
*/
void avoid(void) {
  backward(100);
  delay(500);
  left(100);
  delay(200);
  forward(100);
}

/**
 * Function that makes a noise using buzzer. The delay between
 * turn on and turn off is diferent for diferent classes.
 * 
 * @param delay time
*/
void makeNoise(int delayTime) {
  digitalWrite(buzzerPin, HIGH);
  delay(delayTime);
  digitalWrite(buzzerPin, LOW);
}

/**
 * Function that reads the LDR values and returns it
 *
 * @return the LDR readed value
*/
float LDRData(void) {
  return analogRead(LDRPin) * k; 
} 

/**
 * Function that reads the voltage sensor values and returns it
 *
 * @return the voltage sensor readed value
*/
float voltageSensorData(void) {
  return analogRead(voltageSensorPin) * k; 
}

/**
 * Function that mesures the distance from a nearby object
 *
 * @return distance
*/
float distance(void) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  float duration = pulseIn(echoPin, HIGH);
  float distance = duration/2 /29.1;
  return distance;
}

/**
 * This function implements the sigmoid function
 * 
 * @param data
 * @return sigmoid value
*/
double sigmoid(double data) {
    return (1/(1+exp(-data)));
}

/**
 * Function that gets the data readed and returns the class
 *
 * @param LDR data
 * @param voltade data
 * @return class
*/
int neuralNet(float LDRData, float voltageData) {
  double y; //net output
  //weights and bias
  double  w111,
        w112,
        w121,
        w122,
        w211,
        w221,
        b11,
        b12,
        b21,
        Dw111,Dw112,Dw121,Dw122,Dw211,Dw221,Db11,Db12,Db21;
  
  //This net was trained to find 1!
  double v11 = (LDRData * w111) + (voltageData * w121) + b11;
  double v12 = (LDRData * w112) + (voltageData * w122) + b12;
  double v21 = (sigmoid(v11) * w211) + (sigmoid(v12) * w221) + b21;
  y = sigmoid(v21);
  if(y > 0.9) { //close output
    return 1;
  } else {
    return 0;
  }
}

/**
 * This function parses strings to integers
 * @param string to parse
 * @return integer
*/
int stringToInt(String data) {
  return data.toInt();
}

/**
 * This function says to this probe the master class
 *
*/
void defineMyClass() {
  String masterClass = dataEntered();
  if(masterClass != "tb") {
    notMyClass = stringToInt(masterClass);
  }
}

/**
 * This function tell the probe to find objects
*/
void findObjects() {
  //Go forward
  forward(100);
  //if there's something in front of... (6 cm)
  if(distance() < 6) { //start getting toguether
    while(distance() <= 3) { //while the voltage sensor is not in touch with the ball
      forward(200);
      delay(100);
      backward(100);
    }
    
    int foundClass = neuralNet(LDRData(), voltageSensorData());
    if(foundClass == notMyClass) {
      avoid();
    } else {
      makeNoise(2000);
    } 
  }
}

/**
 * This function does all the operation;
*/
void operation() {
  defineMyClass();
  findObjects();
  
}

void loop(void) {
  operation();
}
