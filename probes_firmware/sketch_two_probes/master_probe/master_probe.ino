/*  @Description firmware that runs on the master probe                               */
/*  @Version 0.1                                                                     */
/*  @author Aurelio Buarque  (abuarquemf@gmail.com)                                   */
/*  Created on July 6, 2015, 3:17 AM, Maceio-AL, Brazil                               */

//Used libraries
#include <Servo.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <RF24_config.h>

const double k = 0.0001; //math const to decrease the input values 
int myClass;
bool check = false;

//Servos
Servo servoRight, servoLeft;

//Vector for the finishing symbol of sending string
int msg[1];
//Object NRF24L01
RF24 myTransceiver(9,10); //CE, CSN
//The comunication pipe
const uint64_t pipe = 0xE8E8F0F0E1LL;

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
  //Starting serial comunication with 9600 of baud
  Serial.begin(9600);
  //Clean serial line
  Serial.flush();
  //Starting the transceiver
  myTransceiver.begin();
  //Openning the pipe
  myTransceiver.openWritingPipe(pipe);
  
  //setting left servo
  servoLeft.attach(5);
  //setting right servo
  servoRight.attach(6);
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
  backward(500);
  delay(200);
  left(400);
  delay(200);
  forward(500);
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
 * Function that sends strings to other Arduino board
 *
 * @param message to send
*/
void sendMessage(String message) {
  //get the size of string
  int messageSize = message.length();
  //vector for the sending char
  int charToSend[1];
  //For each carachter...
  for(int i=0;i<messageSize;i++) {
    charToSend[0] = message.charAt(i);
    myTransceiver.write(charToSend, 1);
  }
  //Symbol that finishes the string
  msg[0] = 2;
  //Send the finishing symbol
  myTransceiver.write(msg, 1);
  //Turns off the transceptor
  myTransceiver.powerDown();
  //waits a while
  delay(1);
  //turns on again
  myTransceiver.powerUp();
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
 * Function that starts the operation.
 *
*/
void defineMyClass(void) {
  //Go forward
  forward(100);
  //if there's something in front of... (6 cm)
  if(distance() < 6) { //start getting toguether
    while(distance() <= 3) { //while the voltage sensor is not in touch with the ball
      forward(200);
      delay(100);
      backward(100);
    }
    
    myClass = neuralNet(LDRData(), voltageSensorData());
    if(myClass == 1) {
      sendMessage("1");
    } else {
      sendMessage("0");
    }
    check = true;
    avoid();
  }
}

/**
 * This function does the process of serching the righ objects
 *
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
    if(foundClass == myClass) {
      makeNoise(100);
    } else {
      avoid();
    } 
  }
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
 * Operation of define my class and look over objects
 *
*/
void operation() {
  while(check == false) {
    sendMessage("tb");
    defineMyClass();
  }
  findObjects();
}

void loop(void){
 operation(); 
}
