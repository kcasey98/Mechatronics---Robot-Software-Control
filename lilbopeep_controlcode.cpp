#in​​clude <Arduino.h>
#include <Servo.h>
#include <Metro.h>
/*---------------Module Defines-----------------------------*/
Servo s1;
static Metro pickupTimer = Metro(800); // Used to go forward and backward 5 times to pickup 5 balls
static Metro matchTimer = Metro(130000); // Finish the match in 2'10''
static Metro backwardTimer = Metro(3500); // Returning route designed to go backward for 3.5 sec
static Metro leftrightTimer = Metro(1000); // Alternating left/right at the loading zone, wait for resetSwitch
const int MOTOR1_PIN1 = 22;
const int MOTOR1_PIN2 = 23;
const int MOTOR2_PIN1 = 10;
const int MOTOR2_PIN2 = 9;
const int MOTOR3_PIN1 = 3;
const int MOTOR3_PIN2 = 4;
const int MOTOR4_PIN1 = 16;
const int MOTOR4_PIN2 = 17;
const int SWITCHFRONT = 19; // limit switches
const int SWITCHLEFT = 21;
const int SWITCHRIGHT = 18;
const int SWITCHBACK_TOP = 20;
const int SERVO = 6;
const int REDBLUESWITCH = 14; // HIGH is RED, LOW is BLUE
const int RESETSWITCH = 15; // Operated at the loading zone; switch one time to stop, two times to reset


/*---------------State Definitions--------------------------*/
typedef enum {
STATIONARY, FORWARD, BACKWARD, LEFT, RIGHT, BACKWARDRIGHT, BACKWARDLEFT, FORWARDRIGHT, FORWARDLEFT, DROPOFF, GAMEOVER, FINDLOADING
} States_t;


/*---------------Module Variables---------------------------*/
States_t driveState;
bool dropOffMovement = true;
bool pickupState = true;
bool returnState = false;
bool isLeft = false; //defined by the readings from the redBlue button, set the returning direction
int sheepNum = 0;
int resetNum; //defined by the initial state of resetSwitch
int resetButton = 0;


/*---------------Module Function Declarations-----------------*/
void moveForward(void);
void moveBackward(void);
void moveRight(void);
void moveLeft(void);
void evensOff(void);
void oddsOff(void);
void moveBackwardLeft(void);
void moveBackwardRight(void);
void moveForwardLeft(void);
void moveForwardRight(void);
void dropOff(void); // stop --> lifting and lowering to drop balls at target
void pickup(void); // start with forward; repeat forward and backward for 5 times(using timer and the front limit switch)
void gameOver (void);
void noMovement(void);
void checkGlobalEvent(void); // check conditions and set the states
void loadSheep(void);
void moveleftRight(void); // moving left and right around the loading zone; wait for the reset switch


/*---------------Main Functions----------------*/
void setup() {
    s1.attach(SERVO, 500, 2500);
    driveState = FORWARD;
    pinMode(MOTOR1_PIN1, OUTPUT);
    pinMode(MOTOR1_PIN2, OUTPUT);
    pinMode(MOTOR3_PIN1, OUTPUT);
    pinMode(MOTOR3_PIN2, OUTPUT);
    pinMode(MOTOR2_PIN1, OUTPUT);
    pinMode(MOTOR2_PIN2, OUTPUT);
    pinMode(MOTOR4_PIN1, OUTPUT);
    pinMode(MOTOR4_PIN2, OUTPUT);
    pinMode(SWITCHFRONT, INPUT);
    pinMode(SWITCHLEFT, INPUT);
    pinMode(SWITCHRIGHT, INPUT);
    pinMode(SWITCHBACK_TOP, INPUT);
    pinMode(LINESENSORLEFT, INPUT);
    pinMode(LINESENSORRIGHT, INPUT);
    pinMode(REDBLUESWITCH, INPUT);
    pinMode(RESETSWITCH, INPUT);
    Serial.begin(9600);
    matchTimer.reset();  //start match timer 2:10
    s1.write(0);
    resetNum = digitalRead(RESETSWITCH); //keep the initial value of resetSwitch
}


void loop() {
    checkGlobalEvent();
    switch (driveState){ // check the current state and call the corresponding functions
         case STATIONARY:
             noMovement();
             break;
         case FORWARD:
             moveForward();
             break;
         case LEFT:
             moveLeft();
             break;
         case BACKWARD:
             moveBackward();
             break;
         case RIGHT:
             moveRight();
             break;
         case BACKWARDRIGHT:
             moveBackwardRight();
             break;
         case BACKWARDLEFT:
             moveBackwardLeft();
             break;
         case FORWARDLEFT:
           moveForwardLeft();
           break;
         case FORWARDRIGHT:
           moveForwardRight();
           break;
         case DROPOFF:
             dropOff();
             break;
         case FINDLOADING:
             moveleftRight();
             break;
         case GAMEOVER:
             gameOver();
             break;
    }
}


void checkGlobalEvent(){
    if(matchTimer.check()){ //Check if match has ended
      driveState = GAMEOVER;
    }
    if((digitalRead(SWITCHFRONT) == HIGH) && (driveState == FORWARD) && (pickupState == true)){ //backward motion in the pickup state
      driveState = BACKWARD;
      sheepNum++;
      pickupTimer.reset();
    }
    if(pickupTimer.check() && (pickupState == true) && (resetButton == 0) && (digitalRead(RESETSWITCH) == resetNum)){ //forward motion in the pickup state; counting the number of sheep
      driveState = FORWARD;
      Serial.println(sheepNum);
      if(sheepNum == 4) {
        pickupState = false;
        sheepNum =0;
      }
    }
    if((digitalRead(SWITCHFRONT) == HIGH) && (driveState == FORWARD) && (pickupState == false)){ //start heading to the scoring zone diagonally
      if(digitalRead(REDBLUESWITCH) == LOW) {
        driveState = BACKWARDRIGHT; //blue
      }
      else {
        driveState = BACKWARDLEFT; //red
      }
    }
    if((digitalRead(SWITCHLEFT) == HIGH) && ((driveState == LEFT) || (driveState == BACKWARDLEFT))) { //hit side wall; go backward toward the scoring zone
      driveState = BACKWARD;
    }  //RED team
    if((digitalRead(SWITCHRIGHT) == HIGH) && ((driveState == RIGHT) || (driveState == BACKWARDRIGHT))){
      driveState = BACKWARD;
    } //BLUE team
    if((digitalRead(SWITCHBACK_TOP) == HIGH) && (driveState == BACKWARD)){ // limit switch at the backtop is hit --> start dropping off
      driveState = DROPOFF;
    }
    if(resetNum != digitalRead(RESETSWITCH)) { // resetswitch switched for the first time --> stop
      driveState = STATIONARY;
      resetButton = 1;
      returnState = false;
    }
    if((resetNum == digitalRead(RESETSWITCH)) && (resetButton == 1) ) { // resetswitch switched for the second time --> restart with pickup state
      resetButton = 0;
      driveState = BACKWARD;
      pickupTimer.reset();
      pickupState = true;
    }
    if((returnState == true) && (digitalRead(SWITCHFRONT) == HIGH) ){ // Front switch hit --> around loading zone
      driveState = FINDLOADING;
      isLeft = digitalRead(REDBLUESWITCH);
      returnState = false;
    }
     
    if((returnState == true) && (driveState == FORWARD) && (backwardTimer.check())) { // returning route: backward timer expires, start going diagonal toward loading zone
       if (digitalRead(REDBLUESWITCH) == LOW){
         driveState = FORWARDLEFT;
       }
       else {
         driveState = FORWARDRIGHT;
       }
    }
}
 
void noMovement(){
    evensOff();
    oddsOff();
    Serial.println("Stationary");
}


void moveForward(){
    evensOff();
    digitalWrite(MOTOR1_PIN1, LOW);
    digitalWrite(MOTOR1_PIN2, HIGH);
    digitalWrite(MOTOR3_PIN1, LOW);
    digitalWrite(MOTOR3_PIN2, HIGH);
    Serial.println("Forward");
}


void moveLeft(){
    oddsOff();
    digitalWrite(MOTOR2_PIN1, HIGH);
    digitalWrite(MOTOR2_PIN2, LOW);
    digitalWrite(MOTOR4_PIN1, HIGH);
    digitalWrite(MOTOR4_PIN2, LOW);
    Serial.println("Left");
}


void moveRight(){
    oddsOff();
    digitalWrite(MOTOR2_PIN1, LOW);
    digitalWrite(MOTOR2_PIN2, HIGH);
    digitalWrite(MOTOR4_PIN1, LOW);
    digitalWrite(MOTOR4_PIN2, HIGH);
    Serial.println("Right");
}


void moveBackward(){
    evensOff();
    digitalWrite(MOTOR1_PIN1, HIGH);
    digitalWrite(MOTOR1_PIN2, LOW);
    digitalWrite(MOTOR3_PIN1, HIGH);
    digitalWrite(MOTOR3_PIN2, LOW);
    Serial.println("Backward");
}


void oddsOff(void){
    digitalWrite(MOTOR1_PIN1, LOW);
    digitalWrite(MOTOR1_PIN2, LOW);
    digitalWrite(MOTOR3_PIN1, LOW);
    digitalWrite(MOTOR3_PIN2, LOW);
}
void evensOff(void){
    digitalWrite(MOTOR2_PIN1, LOW);
    digitalWrite(MOTOR2_PIN2, LOW);
    digitalWrite(MOTOR4_PIN1, LOW);
    digitalWrite(MOTOR4_PIN2, LOW);
}
 
void moveForwardLeft(void){
    digitalWrite(MOTOR1_PIN1, LOW);
    digitalWrite(MOTOR1_PIN2, HIGH);
    digitalWrite(MOTOR3_PIN1, LOW);
    digitalWrite(MOTOR3_PIN2, HIGH);
    digitalWrite(MOTOR2_PIN1, HIGH);
    digitalWrite(MOTOR2_PIN2, LOW);
    digitalWrite(MOTOR4_PIN1, HIGH);
    digitalWrite(MOTOR4_PIN2, LOW);
    Serial.println("ForwardLeft");
}
 
void moveForwardRight(void){
    digitalWrite(MOTOR1_PIN1, LOW);
    digitalWrite(MOTOR1_PIN2, HIGH);
    digitalWrite(MOTOR2_PIN1, LOW);
    digitalWrite(MOTOR2_PIN2, HIGH);
    digitalWrite(MOTOR3_PIN1, LOW);
    digitalWrite(MOTOR3_PIN2, HIGH);
    digitalWrite(MOTOR4_PIN1, LOW);
    digitalWrite(MOTOR4_PIN2, HIGH);
    Serial.println("ForwardRight");
}
 
void moveBackwardRight(void){
    evensOff();
    oddsOff();
    digitalWrite(MOTOR1_PIN1, HIGH);
    digitalWrite(MOTOR1_PIN2, LOW);
    digitalWrite(MOTOR2_PIN1, LOW);
    digitalWrite(MOTOR2_PIN2, HIGH);
    digitalWrite(MOTOR3_PIN1, HIGH);
    digitalWrite(MOTOR3_PIN2, LOW);
    digitalWrite(MOTOR4_PIN1, LOW);
    digitalWrite(MOTOR4_PIN2, HIGH);
    Serial.println("BackRight");
}


void moveBackwardLeft(void){
    evensOff();
    oddsOff();
    digitalWrite(MOTOR1_PIN1, HIGH);
    digitalWrite(MOTOR1_PIN2, LOW);
    digitalWrite(MOTOR2_PIN1, HIGH);
    digitalWrite(MOTOR2_PIN2, LOW);
    digitalWrite(MOTOR3_PIN1, HIGH);
    digitalWrite(MOTOR3_PIN2, LOW);
    digitalWrite(MOTOR4_PIN1, HIGH);
    digitalWrite(MOTOR4_PIN2, LOW);
    Serial.println("BackLeft");
}


void moveleftRight(){ //alternating between left and right around loading zone by timer
    if (isLeft){
      moveLeft();
    }
    else{
      moveRight();
    }
    if (leftrightTimer.check()){
      isLeft = !isLeft;
      leftrightTimer.reset();
    }
}


void dropOff() { //drop off motion through servo motor
    noMovement();
    Serial.println("Dropoff");
    for(int i = 0; i<=100; i++) {
        s1.write(i);
        delay(20);
    }
    delay(100);
    for(int i = 100; i>=0; i--) {
        s1.write(i);
        delay(20);
    }
    dropOffMovement = true;
    returnState = true;
    backwardTimer.reset();
    driveState = FORWARD;
}


void gameOver() { // end of the game
    noMovement();
}

