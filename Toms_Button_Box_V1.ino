//Originally got this code from the AMStudio Youtube video. https://www.youtube.com/watch?v=Z7Sc4MJ8RPM&t=322s
//Much of the functionality is provided by the ArduinoJoyStickLibrary which can be found at: https://github.com/MHeironimus/ArduinoJoystickLibrary
//
//Added code to handle the Encoder buttons out of the Keyboard Map.
//You will need to add the ArduinoJoyStickLibrary found at the link above and your board must match one that supports it like Arduino Leonardo.  There are others.

//


#include <Keypad.h>
#include <Joystick.h>

//DEFINITIONS
#define ENABLE_PULLUPS
#define NUMROTARIES 4 //replace "?" with number of rotary encoders you are using
#define NUMBUTTONS 32 //replace "?"with number of buttong you are using
#define NUMROWS 3 //replace "?" with number of rows you have
#define NUMCOLS 3 //replace "?" with number of columns you have

const int encoder1ButtonPin = A0;
const int encoder2ButtonPin = A1;
const int encoder3ButtonPin = A2;
const int encoder4ButtonPin = A3;

int currentButtonState = 0;

int lastEncoder1ButtonState = 0;
int lastEncoder2ButtonState = 0;
int lastEncoder3ButtonState = 0;
int lastEncoder4ButtonState = 0;

//BUTTON MATRIX
//first change number of rows and columns to match your button matrix, 
//then replace all "?" with numbers (starting from 0)

//NOTE:  When I started with 0, I ran into the 0 button stepping on a button mapping with Discord so now starting with button 1 in the map below.
//
byte buttons[NUMROWS][NUMCOLS] = {
  {1,2,3},
  {4,5,6},
  {7,7,9}
 
};


struct rotariesdef {
  byte pin1;
  byte pin2;
  int ccwchar;
  int cwchar;
  volatile unsigned char state;
};

//ROTARY ENCODERS
//each line controls a different rotary encoder
//the first two numbers refer to the pins the encoder is connected to 
//the second two are the buttons each click of the encoder wil press 
//do NOT exceed 31 for the final button number
rotariesdef rotaries[NUMROTARIES] {
  {6,7,10,11,0}, //rotary 1
  {8,9,12,13,0}, //rotary 2
  {10,11,14,15,0}, //rotary 3
  {12,13,16,17,0} //rotary 4


};


#define DIR_CCW 0x10
#define DIR_CW 0x20
#define R_START 0x0

#ifdef HALF_STEP
#define R_CCW_BEGIN 0x1
#define R_CW_BEGIN 0x2
#define R_START_M 0x3
#define R_CW_BEGIN_M 0x4
#define R_CCW_BEGIN_M 0x5
const unsigned char ttable[6][4] = {
  // R_START (00)
  {R_START_M,            R_CW_BEGIN,     R_CCW_BEGIN,  R_START},
  // R_CCW_BEGIN
  {R_START_M | DIR_CCW, R_START,        R_CCW_BEGIN,  R_START},
  // R_CW_BEGIN
  {R_START_M | DIR_CW,  R_CW_BEGIN,     R_START,      R_START},
  // R_START_M (11)
  {R_START_M,            R_CCW_BEGIN_M,  R_CW_BEGIN_M, R_START},
  // R_CW_BEGIN_M
  {R_START_M,            R_START_M,      R_CW_BEGIN_M, R_START | DIR_CW},
  // R_CCW_BEGIN_M
  {R_START_M,            R_CCW_BEGIN_M,  R_START_M,    R_START | DIR_CCW},
};
#else
#define R_CW_FINAL 0x1
#define R_CW_BEGIN 0x2
#define R_CW_NEXT 0x3
#define R_CCW_BEGIN 0x4
#define R_CCW_FINAL 0x5
#define R_CCW_NEXT 0x6

const unsigned char ttable[7][4] = {
  // R_START
  {R_START,    R_CW_BEGIN,  R_CCW_BEGIN, R_START},
  // R_CW_FINAL
  {R_CW_NEXT,  R_START,     R_CW_FINAL,  R_START | DIR_CW},
  // R_CW_BEGIN
  {R_CW_NEXT,  R_CW_BEGIN,  R_START,     R_START},
  // R_CW_NEXT
  {R_CW_NEXT,  R_CW_BEGIN,  R_CW_FINAL,  R_START},
  // R_CCW_BEGIN
  {R_CCW_NEXT, R_START,     R_CCW_BEGIN, R_START},
  // R_CCW_FINAL
  {R_CCW_NEXT, R_CCW_FINAL, R_START,     R_START | DIR_CCW},
  // R_CCW_NEXT
  {R_CCW_NEXT, R_CCW_FINAL, R_CCW_BEGIN, R_START},
};
#endif

//BUTTON MATRIX PART 2
byte rowPins[NUMROWS] = {5,4,3}; //change "?" to the pins the rows of your button matrix are connected to
byte colPins[NUMCOLS] = {2,1,0}; //change "?" to the pins the rows of your button matrix are connected to

Keypad buttbx = Keypad( makeKeymap(buttons), rowPins, colPins, NUMROWS, NUMCOLS);

//JOYSTICK SETTINGS
Joystick_ Joystick(JOYSTICK_DEFAULT_REPORT_ID,
  JOYSTICK_TYPE_JOYSTICK,
  32, //number of buttons
  0, //number of hat switches
  //Set as many axis to "true" as you have potentiometers for
  false, // y axis
  false, // x axis
  false, // z axis
  false, // rx axis
  false, // ry axis
  false, // rz axis
  false, // rudder
  false, // throttle
  false, // accelerator
  false, // brake
  false); // steering wheel

const int numReadings = 20;
 
int readings[numReadings];      // the readings from the analog input
int index = 0;              // the index of the current reading
int total = 0;                  // the running total
int currentOutputLevel = 0;

//POTENTIOMETERS PART 1
//add all the axis' which are enabled above
//int zAxis_ = 0;
//int RxAxis_ = 0;
//int RyAxis_ = 0;
//int RzAxis_ = 0;
   

               
//POTENTIOMETERS  PART 2
//Which pins are your potentiometers connected to?
//int potentiometerPin1 = A0; //Change "?" to the pin your potentiometer is connected to
//int potentiometerPin2 = A1;
//int potentiometerPin3 = A3; //Change "?" to the pin your potentiometer is connected to
//int potentiometerPin4 = A4 ;
const bool initAutoSendState = true;


void setup() {
  //initialize encoder buttons
  pinMode(encoder1ButtonPin, INPUT_PULLUP);
  pinMode(encoder2ButtonPin, INPUT_PULLUP);
  pinMode(encoder3ButtonPin, INPUT_PULLUP);
  pinMode(encoder4ButtonPin, INPUT_PULLUP);

  
  Joystick.begin();
  rotary_init();
  for (int thisReading = 0; thisReading < numReadings; thisReading++) {
    readings[thisReading] = 0;
  }
}

void loop() {

  CheckAllEncoders();
  CheckAllButtons();
  CheckAllEncoderButtons();
//  CheckAllPotentiometers();
 
}

//POTENTIOMETERS PART 3
//change the details to match teh details above for each potentiometer you are using
/*void CheckAllPotentiometers(){
                           
  //potentiometer 1
  currentOutputLevel = getAverageOutput(potentiometerPin1);
  zAxis_ = map(currentOutputLevel,0,1023,0,255);
  Joystick.setZAxis(zAxis_); 

  //potentiometer 2
  currentOutputLevel = getAverageOutput(potentiometerPin2);
  RxAxis_ = map(currentOutputLevel,0,1023,0,255);
  Joystick.setRxAxis(RxAxis_);

  //potentiometer 3
  currentOutputLevel = getAverageOutput(potentiometerPin3);
  RyAxis_ = map(currentOutputLevel,0,1023,0,255);
  Joystick.setRyAxis(RyAxis_);
    //potentiometer 4
  currentOutputLevel = getAverageOutput(potentiometerPin4);
  RzAxis_ = map(currentOutputLevel,0,1023,0,255);
  Joystick.setRzAxis(RzAxis_);

} 

int getAverageOutput(int pinToRead){
  index = 0;
  total = 0; 
 
  while (index < numReadings){
    readings[index] = analogRead(pinToRead);
    total = total + readings[index];
    index = index + 1;
    //delay (1);
  }
  return total / numReadings;
}

*/

void CheckAllButtons(void) {
      if (buttbx.getKeys())
    {
       for (int i=0; i<LIST_MAX; i++)   
        {
           if ( buttbx.key[i].stateChanged )   
            {
            switch (buttbx.key[i].kstate) { 
                    case PRESSED:
                    case HOLD:
                              Joystick.setButton(buttbx.key[i].kchar, 1);
                              break;
                    case RELEASED:
                    case IDLE:
                              Joystick.setButton(buttbx.key[i].kchar, 0);
                              break;
            }
           }   
         }
     }
}

void CheckAllEncoderButtons(void) {
  
// Read pin values
  
    currentButtonState = !digitalRead(encoder1ButtonPin);
    
    if (currentButtonState != lastEncoder1ButtonState)
    {
      Joystick.setButton(20, currentButtonState);
      lastEncoder1ButtonState = currentButtonState;
    }

    currentButtonState = !digitalRead(encoder2ButtonPin);
    
    if (currentButtonState != lastEncoder2ButtonState)
    {
      Joystick.setButton(21, currentButtonState);
      lastEncoder2ButtonState = currentButtonState;
    }

    currentButtonState = !digitalRead(encoder3ButtonPin);
    
    if (currentButtonState != lastEncoder3ButtonState)
    {
      Joystick.setButton(22, currentButtonState);
      lastEncoder3ButtonState = currentButtonState;
    }

    currentButtonState = !digitalRead(encoder4ButtonPin);
    
    if (currentButtonState != lastEncoder4ButtonState)
    {
      Joystick.setButton(23, currentButtonState);
      lastEncoder4ButtonState = currentButtonState;
    }
  
}

void rotary_init() {
  for (int i=0;i<NUMROTARIES;i++) {
    pinMode(rotaries[i].pin1, INPUT);
    pinMode(rotaries[i].pin2, INPUT);
    #ifdef ENABLE_PULLUPS
      digitalWrite(rotaries[i].pin1, HIGH);
      digitalWrite(rotaries[i].pin2, HIGH);
    #endif
  }
}


unsigned char rotary_process(int _i) {
  //Serial.print("Processing rotary: ");
  //Serial.println(_i);
  unsigned char pinstate = (digitalRead(rotaries[_i].pin2) << 1) | digitalRead(rotaries[_i].pin1);
  rotaries[_i].state = ttable[rotaries[_i].state & 0xf][pinstate];
  return (rotaries[_i].state & 0x30);
}

void CheckAllEncoders(void) {
  Serial.println("Checking rotaries");
  for (int i=0;i<NUMROTARIES;i++) {
    unsigned char result = rotary_process(i);
    if (result == DIR_CCW) {
      Serial.print("Rotary ");
      Serial.print(i);
      Serial.println(" <<< Going CCW");
      Joystick.setButton(rotaries[i].ccwchar, 1); delay(50); Joystick.setButton(rotaries[i].ccwchar, 0);
    };
    if (result == DIR_CW) {
      Serial.print("Rotary ");
      Serial.print(i);
      Serial.println(" >>> Going CW");
      Joystick.setButton(rotaries[i].cwchar, 1); delay(50); Joystick.setButton(rotaries[i].cwchar, 0);
    };
  }
  Serial.println("Done checking");
  
}
