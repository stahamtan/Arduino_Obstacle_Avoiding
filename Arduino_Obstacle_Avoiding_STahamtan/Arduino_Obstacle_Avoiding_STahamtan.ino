/*************************************************************
Obstacle Avoiding Robot
Siavash Tahamtan
*************************************************************/
#define trigPinL 7   // LEFT Ultrasonic Sensor - Trigger
#define echoPinL 10  // LEFT Ultrasonic Sensor - Echo
#define trigPinR 6   // RIGHT Ultrasonic Sensor - Trigger
#define echoPinR 5   // RIGHT Ultrasonic Sensor - Echo
#define trigPinF 4   // FRONT Ultrasonic Sensor - Trigger
#define echoPinF 2   // FRONT Ultrasonic Sensor - Echo
#define dirA 12     // Motor A - Direction          RIGHT MOTOR --> A
#define brakeA 9    // Motor A - brake
#define speedA 3    // Motor A - Speed
#define dirB 13     // Motor B - Direction          LEFT MOTOR --> B
#define brakeB 8    // Motor B - brake
#define speedB 11   // Motor B - Speed
#define forward 1
#define backward 0
#define brake -1
#define minDistFront 20
#define minDistSide 20
#define leftMotorDefaultSpeed 110
#define rightMotorDefaultSpeed 115

long distanceL, distanceR, distanceF;

void setup() {
  Serial.begin(9600);           // set up Serial library at 9600 bps
  //Setup Channel A
  pinMode(dirA, OUTPUT); //Initiates Motor Channel A pin
  pinMode(brakeA, OUTPUT); //Initiates Brake Channel A pin

  //Setup Channel B
  pinMode(dirB, OUTPUT); //Initiates Motor Channel A pin
  pinMode(brakeB, OUTPUT);  //Initiates Brake Channel A pin

  //Setup ultrasonic sensor
  pinMode(trigPinL, OUTPUT);// set the trig pin to output (Send sound waves)
  pinMode(echoPinL, INPUT);// set the echo pin to input (recieve sound waves)
  pinMode(trigPinR, OUTPUT);// set the trig pin to output (Send sound waves)
  pinMode(echoPinR, INPUT);// set the echo pin to input (recieve sound waves)
  pinMode(trigPinF, OUTPUT);// set the trig pin to output (Send sound waves)
  pinMode(echoPinF, INPUT);// set the echo pin to input (recieve sound waves)

  delay(3000);    //initial delay to give the user some time to prep
}

void loop(){

  distanceL = getDistance(trigPinL, echoPinL);
  distanceR = getDistance(trigPinR, echoPinR);
  distanceF = getDistance(trigPinF, echoPinF);
  
  Serial.print("LEFT: ");
  Serial.println(distanceL);
  Serial.print("RIGHT: ");
  Serial.println(distanceR);
  Serial.print("FRONT: ");
  Serial.println(distanceF);

  avoidObstacle();
}

// =====================================================================================
// User-defined functions
// =====================================================================================

void drive (int spdA, int directionA, int spdB, int directionB){  
  digitalWrite(dirA, mapDirection(directionA)); //Establishes forward direction of Channel A
  digitalWrite(dirB, mapDirection(directionB)); //Establishes forward direction of Channel B

  digitalWrite(brakeA, LOW);   //Disengage the Brake for Channel A
  digitalWrite(brakeB, LOW);   //Disengage the Brake for Channel B
  
  analogWrite(speedA, spdA);   //Spins the motor A at speed equal to spdA
  analogWrite(speedB, spdB);   //Spins the motor B at speed equal to spdB
}

void makeTurn() {     // Check left and right distances, and make a turn to the side where there is more room!
  if (distanceL >= distanceR) {
    Serial.println("Turning LEFT..........!!!!");
    drive(rightMotorDefaultSpeed, forward, leftMotorDefaultSpeed, backward);
    delay(300);
  } else {
    Serial.println("Turning RIGHT..........!!!!");
    drive(rightMotorDefaultSpeed, backward, leftMotorDefaultSpeed, forward);
    delay(300);
  }
}

void turn180Degrees(){   // Check left and right distances, and turn 180degree in the direction where there is more room!
  if (distanceL >= distanceR) {
    Serial.println("Turning LEFT..........!!!!");
    drive(1.5*rightMotorDefaultSpeed, forward, 1.5*leftMotorDefaultSpeed, backward);
    delay(400);  
  } else {
    Serial.println("Turning RIGHT..........!!!!");
    drive(1.5*rightMotorDefaultSpeed, backward, 1.5*leftMotorDefaultSpeed, forward);
    delay(400);
  }
}

void stopHere(){
  digitalWrite(brakeA, HIGH);   //Engage the Brake for Channel A
  digitalWrite(brakeB, HIGH);   //Engage the Brake for Channel B
  delay(100);
}

long getDistance(int trigPin, int echoPin) {
  long duration, distance; // start the scan
  digitalWrite(trigPin, LOW);  
  delayMicroseconds(2); // delays are required for a succesful sensor operation.
  digitalWrite(trigPin, HIGH);

  delayMicroseconds(10); //this delay is required as well!
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  distance = (duration/2) / 29.1;// convert the distance to centimeters.
  return distance;
}

uint8_t mapDirection (int motorDir) {
  if (motorDir == forward) {
    return HIGH;
  } else if (motorDir == backward) {
    return LOW;
  }
}

boolean isObstacle(){
  if (distanceF<=minDistFront){
    return true;
  } else {
    return false;
  }
}

void avoidObstacle() {
  int delta;
  if (isObstacle()) {   // On obstacle, make a turn
    Serial.println("OBSTACLE...............!!!!!!!!!!!!!");
    makeTurn();
  } else {              // NO obstacle....
    
    if (distanceL < minDistSide || distanceR < minDistSide) {             // Close to at least one side!
      Serial.println("Close to at least one side..........!!!!");
      
      if (distanceL < 0.5*minDistSide && distanceR < 0.5*minDistSide) {   // TOO Close to both side!

        if ((distanceL + distanceR) > minDistSide) {        // enough space to stay center!          
          // Narrow Passage
          manageNarrowPassage();
        } else {                                            // Not enough space! Stop, back up and turn around...
          Serial.println("CAN'T MOVE FORWARD...STOPPING");
          stopHere();
          Serial.println("BACKING UP....");
          drive(rightMotorDefaultSpeed, backward, leftMotorDefaultSpeed, backward);
          delay(200);
          Serial.println("Turning AROUND..........!!!!");
          turn180Degrees();
        }
        
      } else {                                                            // NOT TOO Close sides!
        if (distanceL < minDistSide && distanceR > minDistSide) {         // Close to LEFT side!
          
          delta = minDistSide - distanceL;  //delta should POSITIVE to turn right
          Serial.println("Steering RIGHT..........!!!!");
          steer(delta);
                    
        } else if (distanceL > minDistSide && distanceR < minDistSide) {  // Close to RIGHT side!
          
          delta = distanceR - minDistSide;  //delta should NEGATIVE to turn left
          Serial.println("Steering LEFT..........!!!!");
          steer(delta);
          
        } else {                                                          // Close to BOTH sides
          
          // Narrow Passage
          manageNarrowPassage();
          
        }
      }  
    } else {                                                              // NOT Close to sides!
      
      Serial.println("Away from sides....MOVING ON......!!!!");
      drive(rightMotorDefaultSpeed, forward, leftMotorDefaultSpeed, forward);
      delay(30);
      
    }  
  }
}

void steer(int delta) {   //delta > 0 --> right     delta<0 --> LEFT
  int leftMotorSpeed, rightMotorSpeed;
  leftMotorSpeed = leftMotorDefaultSpeed + 6*delta;
  rightMotorSpeed = rightMotorDefaultSpeed - 6*delta;
  drive(rightMotorSpeed, forward, leftMotorSpeed, forward);
  delay(20);
}

void manageNarrowPassage() {
  Serial.println("Narrow Passage..........!!!!");
  int delta;
  if (distanceL >= distanceR) { // steer right, so delta should be NEGATIVE
    delta = (distanceL + distanceR)/2 - distanceL;
    Serial.println("Narrow...Steering RIGHT..........!!!!");
  } else {  // steer left, so delta should be POSITIVE
    delta = (distanceL + distanceR)/2 - distanceR;
    Serial.println("Narrow...Steering LEFT..........!!!!");
  }
  steer(delta);
}
