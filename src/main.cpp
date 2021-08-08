#include <EEPROM.h>

#include <Servo.h>

#include <PS2X_lib.h>

PS2X ps2x;
// twelve servo objects can be created on most boards
// create servo object to control a servo

Servo servoBase; // This servo makes rotate the structure / Arm / or Neck
Servo servo01;
Servo servo02;
Servo servoHead;
Servo servoDoor;

byte initalServoDoor;
byte initalServoHead;
byte initalServoBase;
byte initalServo01;
byte initalServo02;

int headState = 6;
int servoBaseAddress = 7;
int servo01Address = 8;
int servo02Address = 9;
int servoHeadAddress = 10;
int servoDoorAddress = 11;

int a = 2000;

// variable to store the servo position
int posBase = 90;
int pos01 = 90;    // 180 = Vertical, 90 = Front, 38 = MAX LOW POSITION <<WARNING this is a very strong servo and it could brake other stuffs.>>
int pos02 = 147;   // 5 = Vertical, 90 = Front, 145 = MAX LOW POSITION
int posHead = 90;  // 0= Left, 90 = Front, 180 = Right
int posDoor = 180; // 180 = Vertical, 90 = Horizontal, 0 = Maybe you'll brake the flag lol
bool buttonPressed = false;
bool circlePressed = false;

unsigned long posStart = 0;
unsigned long circlePressedTime = 0;
unsigned long lastReadGamepad = 0;
unsigned long gamepadReadDelay = 50;

int error = 0;
byte vibrate = 0;
bool LED_OUTPUT = false;

void setup()
{
  Serial.begin(57600);
  delay(3000);
  error = ps2x.config_gamepad(13, 3, 12, 8, true, true); //GamePad(clock, command, attention, data, Pressures?, Rumble?)
  // error = ps2x.config_gamepad(22,23,24,25, true, true);   //GamePad(clock, command, attention, data, Pressures?, Rumble?)
  Serial.println(error);
  servoBase.attach(5);  // attaches the servo on pin 5 to the servo object
  servo01.attach(6);    // Pin 6
  servo02.attach(9);    // Pin 9
  servoHead.attach(10); // Pin 10
  servoDoor.attach(11); // Pin 11
  // servoBase.attach(7); // attaches the servo on pin 5 to the servo object
  // servo01.attach(6);   // Pin 6
  // servo02.attach(5);   // Pin 5
  // servoHead.attach(4); // Pin 4
  // servoDoor.attach(8); // Pin 11
  int servoHeadValue = EEPROM.read(servoHeadAddress);
  int servo01Value = EEPROM.read(servo01Address);
  int servo02Value = EEPROM.read(servo02Address);
  int servoDoorValue = EEPROM.read(servoDoorAddress);
  int servoBaseValue = EEPROM.read(servoBaseAddress);
  servoDoor.write(servoDoorValue);
  servoBase.write(servoBaseValue);
  servo01.write(servo01Value);
  servo02.write(servo02Value);
  servoHead.write(servoHeadValue);
}

void moveServoDirect(Servo servo, int servoAddress, int targetPos)
{
  if (targetPos > 180)
    targetPos = 180;
  else if (targetPos < 0)
    targetPos = 0;
  servo.write(targetPos);
  EEPROM.update(servoAddress, targetPos);
}

void moveServo(Servo servo, int servoAddress, byte currentPos, int startPos, int targetPos)
{
  int absDistance = abs(startPos - targetPos);
  bool inFirstQuarter = abs(currentPos - startPos) < absDistance / 4;
  bool inLastQuarter = abs(currentPos - startPos) > absDistance * 3 / 4;
  int isMovingPositive = 1;
  if (targetPos < startPos)
  {
    isMovingPositive = -1;
  }
  if (targetPos > startPos)
  {
    if (currentPos >= targetPos)
    {
      EEPROM.update(servoAddress, currentPos);
      return;
    }
  }
  else
  {
    if (currentPos <= targetPos)
    {
      EEPROM.update(servoAddress, currentPos);
      return;
    }
  }
  if (millis() % 4 == 0)
  {
    if (inFirstQuarter || inLastQuarter)
    {
      Serial.println("in edge quarter");
      int increment = 1 * isMovingPositive;
      servo.write(currentPos + increment);
      int endingPos = currentPos + increment;
      EEPROM.update(servoAddress, endingPos);
    }
    else
    {
      Serial.println("not in edge quarter");
      int increment = 3 * isMovingPositive;
      servo.write(currentPos + increment);
      int endingPos = currentPos + increment;
      EEPROM.update(servoAddress, endingPos);
    }
  }
}

void position01()
{
  if (posStart == 0)
  {
    initalServoDoor = EEPROM.read(servoDoorAddress);
    initalServoHead = EEPROM.read(servoHeadAddress);
    initalServoBase = EEPROM.read(servoBaseAddress);
    initalServo01 = EEPROM.read(servo01Address);
    initalServo02 = EEPROM.read(servo02Address);
    posStart = millis();
  }
  if (millis() < posStart + 2 * a)
  {
    // open the door
    moveServo(servoDoor, servoDoorAddress, EEPROM.read(servoDoorAddress), initalServoDoor, 56);
  }
  else if (millis() < posStart + 2 * a + 50)
  {
    initalServoDoor = EEPROM.read(servoDoorAddress);
  }
  else if (millis() < posStart + 4 * a)
  {
    // stash the head in the compartment
    moveServo(servoBase, servoBaseAddress, EEPROM.read(servoBaseAddress), initalServoBase, 84);
    moveServo(servo01, servo01Address, EEPROM.read(servo01Address), initalServo01, 30);
    moveServo(servo02, servo02Address, EEPROM.read(servo02Address), initalServo02, 110);
    moveServo(servoHead, servoHeadAddress, EEPROM.read(servoHeadAddress), initalServoHead, 90);
  }
  else if (millis() < posStart + 6 * a)
  {
    // close the door
    moveServo(servoDoor, servoDoorAddress, EEPROM.read(servoDoorAddress), initalServoDoor, 0);
  }
  else
  {
    Serial.println("Finished 1");
    EEPROM.update(headState, 1);
    posStart = 0;
  }
}

void position02()
{
  if (posStart == 0)
  {
    initalServoDoor = EEPROM.read(servoDoorAddress);
    initalServoHead = EEPROM.read(servoHeadAddress);
    initalServoBase = EEPROM.read(servoBaseAddress);
    initalServo01 = EEPROM.read(servo01Address);
    initalServo02 = EEPROM.read(servo02Address);
    posStart = millis();
  }
  if (millis() < posStart + 2 * a)
  {
    // open the door 
    moveServo(servoDoor, servoDoorAddress, EEPROM.read(servoDoorAddress), initalServoDoor, 56);
  }
  else if (millis() < posStart + 2 * a + 50)
  {
    initalServoDoor = EEPROM.read(servoDoorAddress);
  }
  else if (millis() < posStart + 4 * a)
  {
    // move the head out of the compartment into position 2
    moveServo(servoBase, servoBaseAddress, EEPROM.read(servoBaseAddress), initalServoBase, 90);
    moveServo(servo01, servo01Address, EEPROM.read(servo01Address), initalServo01, 138);
    moveServo(servo02, servo02Address, EEPROM.read(servo02Address), initalServo02, 160);
    moveServo(servoHead, servoHeadAddress, EEPROM.read(servoHeadAddress), initalServoHead, 90);
  }
  else if (millis() < posStart + 4 * a + 50)
  {
    initalServoDoor = EEPROM.read(servoDoorAddress);
  }
  else if (millis() < posStart + 8 * a)
  {
    // close the door
    moveServo(servoDoor, servoDoorAddress, EEPROM.read(servoDoorAddress), initalServoDoor, 0);
  }
  else
  {
    Serial.println("Finished 2");
    EEPROM.update(headState, 2);
    posStart = 0;
  }
}

void position03()
{
  if (posStart == 0)
  {
    initalServoDoor = EEPROM.read(servoDoorAddress);
    initalServoHead = EEPROM.read(servoHeadAddress);
    initalServoBase = EEPROM.read(servoBaseAddress);
    initalServo01 = EEPROM.read(servo01Address);
    initalServo02 = EEPROM.read(servo02Address);
    posStart = millis();
    circlePressed = false;
    circlePressedTime = 0;
    Serial.println("Started 3");
  }

  if (millis() < posStart + 2 * a)
  {
    // move base to look left
    // moveServo(servo02, servo02Address, EEPROM.read(servo02Address), initalServo02, 114);
    // moveServo(servoBase, servoBaseAddress, EEPROM.read(servoBaseAddress), initalServoBase, 90);
  }
  else if (millis() < posStart + 2 * a + 50)
  {
    initalServoBase = EEPROM.read(servoBaseAddress);
  }
  else if (millis() < posStart + 4 * a)
  {
    //move base to look a bit right
    // moveServo(servoBase, servoBaseAddress, EEPROM.read(servoBaseAddress), initalServoBase, 107);
  }
  else if (millis() < posStart + 4 * a + 50)
  {
    initalServoBase = EEPROM.read(servoBaseAddress);
  }
  else if (!circlePressed)
  {
    // control head with ps2 controller
    int x1Value = ps2x.Analog(PSS_RX);
    int y1Value = ps2x.Analog(PSS_LY);
    int x2Value = ps2x.Analog(PSS_LX);
    int y2Value = ps2x.Analog(PSS_RY);
    if (x1Value > 138 && ps2x.Button(PSB_R2))
    {
      int mutedMovement = (x1Value - 138) / 40;
      int targetPos = EEPROM.read(servoBaseAddress) + mutedMovement;
      Serial.println(targetPos);
      moveServoDirect(servoBase, servoBaseAddress, targetPos);
    }
    else if (x1Value < 120 && ps2x.Button(PSB_R2))
    {
      int mutedMovement = (120 - x1Value) / 40;
      int targetPos = EEPROM.read(servoBaseAddress) - mutedMovement;
      Serial.println(targetPos);
      moveServoDirect(servoBase, servoBaseAddress, targetPos);
    }
    if (x2Value > 138 && ps2x.Button(PSB_R2))
    {
      int mutedMovement = (x2Value - 138) / 40;
      int targetPos = EEPROM.read(servoHeadAddress) + mutedMovement;
      Serial.println(targetPos);
      moveServoDirect(servoHead, servoHeadAddress, targetPos);
    }
    else if (x2Value < 120 && ps2x.Button(PSB_R2))
    {
      int mutedMovement = (120 - x2Value) / 40;
      int targetPos = EEPROM.read(servoHeadAddress) - mutedMovement;
      Serial.println(targetPos);
      moveServoDirect(servoHead, servoHeadAddress, targetPos);
    }
    if (y1Value > 138 && ps2x.Button(PSB_R2))
    {
      int mutedMovement = (y1Value - 138) / 40;
      int targetPos = EEPROM.read(servo01Address) + mutedMovement;
      Serial.println(targetPos);
      moveServoDirect(servo01, servo01Address, targetPos);
    }
    else if (y1Value < 120 && ps2x.Button(PSB_R2))
    {
      int mutedMovement = (120 - y1Value) / 40;
      int targetPos = EEPROM.read(servo01Address) - mutedMovement;
      Serial.println(targetPos);
      moveServoDirect(servo01, servo01Address, targetPos);
    }
    if (y2Value > 138 && ps2x.Button(PSB_R2))
    {
      int mutedMovement = (y2Value - 138) / 40;
      int targetPos = EEPROM.read(servoHeadAddress) + mutedMovement;
      Serial.println(targetPos);
      moveServoDirect(servoHead, servoHeadAddress, targetPos);
    }
    else if (y2Value < 120 && ps2x.Button(PSB_R2))
    {
      int mutedMovement = (120 - y2Value) / 40;
      int targetPos = EEPROM.read(servoHeadAddress) - mutedMovement;
      Serial.println(targetPos);
      moveServoDirect(servoHead, servoHeadAddress, targetPos);
    }
    else if (ps2x.ButtonPressed(PSB_BLUE) && ps2x.Button(PSB_R2))
    {
      Serial.println("pressed circle");
      circlePressed = true;
      circlePressedTime = millis();
    }
    delay(30);
  }
  else if (millis() < circlePressedTime + 50)
  {
    initalServoBase = EEPROM.read(servoBaseAddress);
  }
  else if (millis() < circlePressedTime + 6 * a)
  {
    // after robot has looked around, move back to position 2
    moveServo(servoBase, servoBaseAddress, EEPROM.read(servoBaseAddress), initalServoBase, 90);
    moveServo(servo01, servo01Address, EEPROM.read(servo01Address), initalServo01, 138);
    moveServo(servo02, servo02Address, EEPROM.read(servo02Address), initalServo02, 160);
    moveServo(servoHead, servoHeadAddress, EEPROM.read(servoHeadAddress), initalServoHead, 90);
  }
  else
  {
    Serial.println("Finished 3");
    EEPROM.update(headState, 3);
    posStart = 0;
  }
}

void moveHead()
{
  byte value = EEPROM.read(headState);
  position03();
  return;
  if (ps2x.ButtonPressed(PSB_BLUE))
  {
    buttonPressed = true;
    if (posStart == 0)
    {
      posStart = millis();
      initalServoDoor = EEPROM.read(servoDoorAddress);
    }
  }
  if (buttonPressed && millis() < posStart + 3 * a)
  {
    // Open door
    moveServo(servoDoor, servoDoorAddress, EEPROM.read(servoDoorAddress), initalServoDoor, 56);
  }
  if (buttonPressed && millis() > posStart + 2 * a)
  {
    // start the sequence
    EEPROM.update(headState, 0);
    posStart = 0;
    buttonPressed = false;
  }
  if (value == 0)
  {
    position01();
  }
  value = EEPROM.read(headState);
  if (value == 1)
  {
    position02();
  }
  value = EEPROM.read(headState);
  if (value == 2)
  {
    position03();
  }
  value = EEPROM.read(headState);
}

void loop()
{
  long current = millis();
  if (current - lastReadGamepad > gamepadReadDelay)
  {
    ps2x.read_gamepad();
    lastReadGamepad = current;
  }
  moveHead();
}
