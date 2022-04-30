
#include <Stepper.h>
#include <Servo.h>
#include <SPI.h>
#include <SD.h>
#include "SSD1306Ascii.h"
#include "SSD1306AsciiAvrI2c.h"

#define I2C_ADDRESS 0x3C

#define RST_PIN -1

SSD1306AsciiAvrI2c oled;

//-----------------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------------------
//                                                   declaration
#define STEPS 48
#define CSPin 10

File file;//file to be read
String lineString;//the current line of gcode as a string

//Setting factors to calibrate rotation and distances  <---------- calibration
float rotateFactor = 5.58;
float moveFactor = 5.5; 

int holdTime = 800;
int pressTime = 20;

//temp location for float values as string
String number;

//initial Values of robot
float currentAngle = 0;
float movementAngle = 0;
float movementDist = 0;
float stepperRotate = 0;
float stepperMove = 0;
float oldX = 0;
float oldY = 0;

//robot dimensions
float wheel = 30;
float width = 46;

//data structure to contain x and y values
struct line{
  float x = 0;
  float y = 0;
};
struct line lineStruct;

Stepper rightMotor(STEPS, A0, A2, A1, A3);
Stepper leftMotor(STEPS, 2, 4, 3, 5);

Servo penServo;//servo for pen up/down

String fileList[20];
int numberOfFiles = 0;

int menu = 0;

File root;
//-----------------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------------------
//                                                  initialization
void setup() {  
  Serial.begin(9600);
  initializeSD(CSPin);

  pinMode(A6, INPUT);
  pinMode(8, INPUT_PULLUP);//right
  pinMode(9, INPUT_PULLUP);//left
  
  rightMotor.setSpeed(250);
  leftMotor.setSpeed(250);
  penServo.attach(6);
  penServo.write(180);

  
#if RST_PIN >= 0
  oled.begin(&Adafruit128x64, I2C_ADDRESS, RST_PIN);
#else // RST_PIN >= 0
  oled.begin(&Adafruit128x64, I2C_ADDRESS);
#endif // RST_PIN >= 0
  //oled.setI2cClock(2000000); //to change from the default frequency.

  oled.setFont(Arial_bold_14);
  oled.clear();


  root = SD.open("/");
  printDirectory(root);

  
  for(int i; i<20; i++){
    if (fileList[i] != NULL){
      numberOfFiles++;
    }
  }

  splashScreen(3000);
  
  char toOpen[12];
  (listMenu(fileList)).toCharArray(toOpen, 12);
  oled.clear();
  Serial.println(toOpen);
  openFile(toOpen);

  splashScreen(5000);

  
  //openFile("599.TXT");

  Serial.println(freeRam());

  
}


//-----------------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------------------
//                                                      loop
void loop() {


  lineString = readLine();
  //Serial.println(lineString);

  //Command g1: convert to struct format and execute
  if(lineString.charAt(0) == 'G' && lineString.charAt(1) == '1'){
    int count = 4;
    number = "";
    //extract x value
    while(lineString.charAt(count)>= '-' && lineString.charAt(count)<= '9'){
      number.concat(lineString.charAt(count));
      lineStruct.x = number.toFloat();
      count = count + 1;
    }
    count = count + 2;
    number = "";
    //extract y value
    while(lineString.charAt(count)>= '-' && lineString.charAt(count)<= '9'){
      number.concat(lineString.charAt(count));
      lineStruct.y = number.toFloat();
      count = count + 1;
    }

    //apply calculation functions and move steppers accordingly
    movementAngle = calcMovementAngle(oldX, oldY, lineStruct.x, lineStruct.y, currentAngle);   
    currentAngle = calcCurrentAngle(oldX, oldY, lineStruct.x, lineStruct.y);
    movementDist = calcMovementDist(oldX, oldY, lineStruct.x, lineStruct.y);
    stepperRotate = calcStepperRotate(movementAngle, wheel, width) ;
    stepperMove = calcStepperMove(movementDist, wheel);
    draw((-stepperRotate * rotateFactor), (stepperMove * moveFactor));
    oldX = lineStruct.x;
    oldY = lineStruct.y;
    
  }
  //check for pen up/down commands
  if(lineString.charAt(0) == 'M' ){
    //if penup
    if(lineString.charAt(6) == '5'){
      penUp();
    }
    //if pendown
    else if(lineString.charAt(6) == '3'){
      penDown();
    }
  }
  if(lineString.charAt(0) == 'M' && lineString.charAt(6) == '2'){
    oled.println("");
    oled.println("        Done");      
    while(true){
      delay(1000);
    }
  }
}



//-----------------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------------------
//                                        Functions to be called by program

//functions for pen up/down
void penUp(){
  delay(500);
  penServo.write(135);
  delay(500);
}

void penDown(){
  delay(500);
  penServo.write(180);
  delay(500);
}

float calcCurrentAngle(float x, float y, float newX, float newY){
  float newAngle;
  if ((newX-x) > 0 && (newY-y) >0){
    newAngle =((atan((newY-y)/(newX-x)))*(180/PI));
  }
  if ((newX-x) > 0 && (newY-y) <0){
    newAngle =((atan((newY-y)/(newX-x)))*(180/PI));
  }
  if ((newX-x) < 0 && (newY-y) >0){
    newAngle =((atan((newY-y)/(newX-x)))*(180/PI));
    newAngle = 180 + newAngle;
  }
  if ((newX-x) < 0 && (newY-y) <0){
    newAngle =((atan((newY-y)/(newX-x)))*(180/PI));
    newAngle = newAngle+180;
  }

  if ((newX-x) == 0){
    if(newY-y > 0){
      newAngle = 90;
    }
    if((newY-y) < 0){
      newAngle = -90;
    }
  }
  if ((newY-y) == 0){
    if(newX-x > 0){
      newAngle = 0;
    }
    if((newX-x) < 0){
      newAngle = -180;
    }
  }
  
  return(newAngle);
}

//calculate direction of movement
float calcMovementAngle(float x, float y, float newX, float newY, float currentAngle){
  float newAngle;
  if ((newX-x) > 0 && (newY-y) >0){
    newAngle =((atan((newY-y)/(newX-x)))*(180/PI));
  }
  if ((newX-x) > 0 && (newY-y) <0){
    newAngle =((atan((newY-y)/(newX-x)))*(180/PI));
  }
  if ((newX-x) < 0 && (newY-y) >0){
    newAngle =((atan((newY-y)/(newX-x)))*(180/PI));
    newAngle = 180 + newAngle;
  }
  if ((newX-x) < 0 && (newY-y) <0){
    newAngle =((atan((newY-y)/(newX-x)))*(180/PI));
    newAngle = newAngle+180;
  }

  if ((newX-x) == 0){
    if(newY-y > 0){
      newAngle = 90;
    }
    if((newY-y) < 0){
      newAngle = -90;
    }
  }
  if ((newY-y) == 0){
    if(newX-x > 0){
      newAngle = 0;
    }
    if((newX-x) < 0){
      newAngle = -180;
    }
  }
  
  float toMove = (newAngle - currentAngle);
  if( toMove < -180 ){
    toMove = (360 + toMove); 
  }
  if( toMove > 180 ){
    toMove = (toMove-360); 
  }
  return(toMove);
}

//calculate movement distance
float calcMovementDist(float x, float y, float newX, float newY){ 
  return(sqrt((sq(newY-y))+(sq(newX-x))));
}

//calculate stepper rotation to rotate
float calcStepperRotate(float movementAngle, float wheelRadius, float axleWidth){
  float dist = ((2*PI*axleWidth)*(movementAngle/360));
  return((dist/(2*PI*wheelRadius))*360);
}

//calculate stepper rotation to move
float calcStepperMove(float movementDist, float wheelRadius){
  return((movementDist/(2*PI*wheelRadius))*360);
}

void draw(float rotate, float draw){
  float currentDraw = 0;
  float currentRotate = 0;
  if(rotate > 0){
    while (currentRotate < rotate){
      rightMotor.step(1);
      leftMotor.step(1);
      currentRotate = currentRotate + 1;
    }
  }
  if(rotate < 0){
    while (currentRotate > rotate){
      rightMotor.step(-1);
      leftMotor.step(-1);
      currentRotate = currentRotate - 1;
    }
  }
  while(currentDraw < draw){
    rightMotor.step(1);
    leftMotor.step(-1);
    currentDraw = currentDraw + 1; 
  }
  
  
}

//initialise sd card
void initializeSD(int CS_PIN)
{
  //Serial.println("Initializing SD card...");
  pinMode(CS_PIN, OUTPUT);

  if (SD.begin())
  {
    //Serial.println("SD card is ready to use.");
  } else
  {
    //Serial.println("SD card initialization failed");
    return;
  }
}

//open a file from sd card
int openFile(char filename[])
{
  file = SD.open(filename);
  if (file)
  {
    //Serial.println("File opened with success!");
    return 1;
  } else
  {
    //Serial.println("Error opening file...");
    return 0;
  }
}


//closefile form sd card
void closeFile()
{
  if (file)
  {
    file.close();
    //Serial.println("File closed");
  }
}

//readline from sd card
String readLine()
{
  String received = "";
  char ch;
  while (file.available())
  {
    ch = file.read();
    if (ch == '\n')
    {
      return String(received);
    }
    else
    {
      received += ch;
    }
  }
  return "";
}


void printDirectory(File dir) {
  int i = 0;
  File entry =  dir.openNextFile();
  entry.close(); 
  while (true) {
    File entry =  dir.openNextFile();
    if (! entry) {
      // no more files
      break;
    }
    
    fileList[i] = entry.name();
    entry.close();
    i++;
  }
}


//check button with paramater id for hold or press
int checkButton(int id){
  unsigned long timeCount = 0;
    
  unsigned long prevTime = 0;
  unsigned long currentTime = 0;

  int action = 0;
  
  if (digitalRead(id) == LOW){
    currentTime = millis();  //record press duration to differentiate hold and tap
    prevTime = currentTime;
    while (digitalRead(id) == LOW){
      currentTime = millis();
      /*if ((currentTime - prevTime) > holdTime){
        break;
      }*/
    }
    timeCount = currentTime - prevTime;
    if (timeCount > holdTime){//hold     
      action = 2;
      Serial.println("held");
    }
    else if(timeCount > 50){//single press
      action = 1;
      Serial.println("pressed");
    }    
    timeCount = 0;
  }
  return action;  
}

//counts the number of strings in an array
int countElements(String list[]){
  int i = 0;
  while (list[i] != NULL){
    i++;
  }
  return i;
}
//function will list a scrollable selectable list to the oled
String listMenu(String list[]){
  bool escape = false; 
  int itemCount = countElements(list);
  int startPoint;
  int prevHover;
  int currentHover;
  while(escape == false){ 
    currentHover = ((int)(analogRead(A6)/(1023/float(itemCount))));    
    if (prevHover != currentHover){
      oled.clear();
      startPoint = ((int)(analogRead(A6)/(1023/float(itemCount))));
      if (startPoint < 4){
        startPoint = 0;
      }
      else if(startPoint >(itemCount - 4)){
        startPoint = (itemCount - 4);
      }
      for(int i = startPoint; i < (startPoint+4); i++){
         if ((analogRead(A6)/(1023/float(itemCount)))>=(i) && (analogRead(A6)/(1023/float(itemCount)))<(i+1)){        
           oled.print(list[i]);
           oled.println(" <");
           if (checkButton(8) > 0){
             return(list[i]);
             escape = true;
           }
         }
         else{
           oled.println(list[i]);
         }
      }
      prevHover = currentHover;
    }
    if (checkButton(8) > 0){
      return(list[currentHover]);
    }
  }
  oled.clear();
}
//splash screen with duration parameter
void splashScreen(int duration){
   oled.clear();
   oled.println("=======================");
   oled.println("     Drawinator");
   oled.println("        MK 2.5");
   oled.println("=======================");
   delay(duration);
   oled.clear();
   delay(500);
}


//function will return RAM that is currently free
int freeRam(){
  extern int __heap_start, *__brkval; 
  int v; 
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
}
