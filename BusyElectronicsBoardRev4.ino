//Rev 0 Created on 12/9/2024 by Jonathan DeWitt.  This revision allows the user to press the pushbutton and each press plays the next track on the micro SD card.  Note this code is controlling a knockoff clone of the DFPlayer Mini.
//Rev 1 Created on 12/10/2024 by Jonathan DeWitt.  This version adds two small WS2812B LED strips.
//Rev 1a Created on 12/10/2024 by Jonathan DeWitt.  The strip tied to leds1 that was supposed to run animations didn't fully work in Rev 1.  I wanted to save a copy before modifying it so I didn't break the progress I made in Rev 1 or could revert back to it if I did break the progress.  In this version I realized I didn't want the animations in a function.  The animations were only being called once.  I needed them in the main loop.
//Rev 1b Created on 12/10/2024 by Jonathan DeWitt.  Again, I didn't want to screw up the progress I'd made with Rev 1a. In Rev 1b I was able to get both LED strips working the way I wanted.  I also inserted frame rate coding from the rainbow bare minimum sketch to slow down how fast the colors were changing. Reb 1b also switches from using the playNext function to the playTrack function.  This is in preparation for adding more MP3 files later and wanting to access difference groups of MP3 files for specific button functionality.  Note that file 003 actually plays first.  But I think that might have to do which file was copied to the SD card first.  I tried copying them in the correct order.  But something in the system glitched I think.
//Rev 2 Created on 12/12/2024 by Jonathan DeWitt.  This version adds two more pushbuttons (one to control stepping through the MP3 files that state numbers and one to control stepping through the MP3 files that state letters).  I also fixed the dead zone that existed in previous versions on the last button press of pb1 before it cycled back down to the bottom of the first section of MP3 files.  It now jumps right back to the beginning of the playlist without a dead button press.
//Rev 3 Created on 12/12/2024 by Jonathan DeWitt.  This version of the code adds an OLED display to output the numbers and letters that the MP3 module is playing MP3 files for.
//Rev 4 Created on 12/13/2024 by Jonathan DeWitt.  This version fixed a few items.  It blanked the OLED screen initially.  Whenever the numbers or letters groups finished and went back to the beginning message it blanked the OLED on the beginning message.  It also added audio callouts for the colors.
#include <HardwareSerial.h> //Needed to use Hardware Serial pins of ESP32
//Now to Use a Hardware Serial Port to Control DFPlayer Mini Clone/Knockoff
//Connect ESP32 GPIO 27 to DFPlayer Mini Pin Tx through 1 kΩ Resistor
//Connect ESP32 GPIO 26 to DFPlayer Mini Pin Rx through 1 kΩ Resistor

#include <FastLED.h> //Include the FastLED library for LED control
#include <Adafruit_GFX.h> //Included for 0.96" SSD1306 OLED Display
#include <Adafruit_SSD1306.h> //Included for 0.96" SSD1306 OLED Display

#define SCREEN_WIDTH 128 //Define the OLED width
#define SCREEN_HEIGHT 64 //Define the OLED height

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1); //Create an instance of the OLED display to work with later in the program

HardwareSerial MySerial(1);
#define DFPlayerTx 27
#define DFPlayerRx 26
#define LED_PIN1 25  //Defines which microcontroller pin is connected to the LED strip signal wire
#define LED_PIN2 33  //Defines which microcontroller pin is connected to the LED strip signal wire

#define pb1 4 //Pushbutton 1 is connected to ESP32 GPIO Pin 4 - Steps through MP3 files that play random sound clips
#define pb2 16 //Pushbutton 2 is connected to ESP32 GPIO Pin 16 - Controls one LED strip
#define pb3 17 //Pushbutton 3 is connected to ESP32 GPIO Pin 17 - Controls second LED strip
#define pb4 5 //Pushbutton 4 is connected to ESP32 GPIO Pin 5 - Controls stepping through MP3 files that play numbers - note if this gives you any trouble it may be because according to the notes GPIO 5 "outputs PWM signal at boot, strapping pin".  I don't foresee any problems with this because I won't be doing anything with that pin (tying it to ground) until after the bootup sequence is gone through
#define pb5 18 //Pushbutton 5 is connected to ESP32 GPIO Pin 18 - Controls stepping through MP3 files that play letters
#define BRIGHTNESS  10 //Set the brightness of the LED strip. Integer between 0 and 255 where 255 is brightest
#define FRAMES_PER_SECOND  120
#define NUM_LEDS    20 //Change this to the number of LEDs in your strip
CRGB leds1[NUM_LEDS]; //Create an array to hold the LED data
CRGB leds2[NUM_LEDS]; //Create an array to hold the LED data

int pb1St=0, pb2St=0, pb3St=0, pb4St=0, pb5St=0; //Variables to store the current state of the pushbutton(s) 
int pb1StP=0, pb2StP=0, pb3StP=0, pb4StP=0, pb5StP=0; //Variables to store the previous state of the pushbutton(s)

//LED Strip Animation and Solid Color Function Definitions Begin
//Note: Strip 1 (i.e. leds1) will do the animations. Strip 2 (i.e. leds2) will do the solid colors. The various functions below are tailored to the strips in question.
uint8_t hue = 0; //Global variable for hue for color cycling
void rainbow() 
{ //Start of rainbow function definition
  fill_rainbow(leds1, NUM_LEDS, hue++, 7); //Fill LEDs with a rainbow pattern
} //Start of rainbow function definition

void rainbowWithGlitter() 
{ //Start of rainbowWithGlitter function definition
  rainbow(); //Call the rainbow function
  if (random8() < 80) // Add glitter with a probability of 80/256 - See the block comment at the end of the program for further explanation
  { //Start of if (random8() < 80)
    leds1[random16(NUM_LEDS)] += CRGB::White; //Add a white sparkle to a random LED
  } //End of if (random8() < 80)
} //End of rainbowWithGlitter function definition

void confetti() 
{ //Start of confetti function definition
  fadeToBlackBy(leds1, NUM_LEDS, 10); //Dim the LEDs slightly for a fade effect
  int pos = random16(NUM_LEDS); //Choose a random LED position
  leds1[pos] += CHSV(random8() + hue, 200, 255); //Add a random color at the position
  hue++; //Increment hue for color cycling
} //End of confetti function definition

void juggle() 
{ //Start of juggle function definition
  fadeToBlackBy(leds1, NUM_LEDS, 20); //Dim the LEDs slightly for a fade effect
  for (int i = 0; i < 8; i++) //Loop to create 8 juggling points
  { //Start of for (int i = 0; i < 8; i++)
    leds1[beatsin16(i + 7, 0, NUM_LEDS - 1)] += CHSV(hue + i * 32, 200, 255); //Add color at positions based on sine waves
  } //End of for (int i = 0; i < 8; i++)
  hue++; //Increment hue for continuous color changes
} //End of juggle function definition

void allRed() 
{ //Start of allRed function definition
  fill_solid(leds2, NUM_LEDS, CRGB::Red); //Fill all LEDs with red color
  FastLED.show(); //Update the LEDs with the new pattern
  delay(20); //Wait 20 milliseconds between updates
} //End of allRed function definition

void allOrange() 
{ //Start of allOrange function definition
  fill_solid(leds2, NUM_LEDS, CRGB::Orange); //Fill all LEDs with orange color
  FastLED.show(); //Update the LEDs with the new pattern
  delay(20); //Wait 20 milliseconds between updates
} //End of allOrange function definition

void allYellow() 
{ //Start of allYellow function definition
  fill_solid(leds2, NUM_LEDS, CRGB::Yellow); //Fill all LEDs with yellow color
  FastLED.show(); //Update the LEDs with the new pattern
  delay(20); //Wait 20 milliseconds between updates
} //End of allYellow function definition

void allGreen() 
{ //Start of allGreen function definition
  fill_solid(leds2, NUM_LEDS, CRGB::Green); //Fill all LEDs with green color
  FastLED.show(); //Update the LEDs with the new pattern
  delay(20); //Wait 20 milliseconds between updates
} //End of allGreen function definition

void allBlue() 
{ //Start of allBlue function definition
  fill_solid(leds2, NUM_LEDS, CRGB::Blue); //Fill all LEDs with blue color
  FastLED.show(); //Update the LEDs with the new pattern
  delay(20); //Wait 20 milliseconds between updates
} //End of allBlue function definition

void allBlueViolet() 
{ //Start of allBlueViolet function definition
  fill_solid(leds2, NUM_LEDS, CRGB::BlueViolet); //Fill all LEDs with blue violet color
  FastLED.show(); //Update the LEDs with the new pattern
  delay(20); //Wait 20 milliseconds between updates
} //End of allBlueViolet function definition

void setup() 
{ //Start of void setup
  Serial.begin(9600);
  MySerial.begin(9600, SERIAL_8N1, DFPlayerTx, DFPlayerRx); //Start up the serial communication to the DFPlayer Mini Clone
  delay(3500);//let everything initialise
  changeVolume(25);//Set volume to 25....30 is very loud even on small speaker.

  FastLED.addLeds<WS2812, LED_PIN1, GRB>(leds1, NUM_LEDS); //Initialize the LED strip
  FastLED.addLeds<WS2812, LED_PIN2, GRB>(leds2, NUM_LEDS); //Initialize the LED strip
  FastLED.setBrightness(BRIGHTNESS); //Set the brightness of the LED strip

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) //Initialize the OLED display
  { //Start of if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
        Serial.println(F("SSD1306 allocation failed")); //Print a failure message to the serial monitor should failure occur
        for (;;); //Create an infinite loop to half execution if the OLED display fails to initialize
  } //End of if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))

  display.clearDisplay(); //Clear the internal buffer of the OLED display
  display.display(); //Send the current contents of the internal buffer to the physical OLED display

  pinMode(pb1, INPUT_PULLUP); //This line makes the pushbutton named an input and uses the internal pullup resistor
  pinMode(pb2, INPUT_PULLUP); //This line makes the pushbutton named an input and uses the internal pullup resistor
  pinMode(pb3, INPUT_PULLUP); //This line makes the pushbutton named an input and uses the internal pullup resistor
  pinMode(pb4, INPUT_PULLUP); //This line makes the pushbutton named an input and uses the internal pullup resistor
  pinMode(pb5, INPUT_PULLUP); //This line makes the pushbutton named an input and uses the internal pullup resistor
} //End of void setup

static uint8_t animationIndex = 0; // Variable to keep track of the current animation index
int soundClipIndex1 = 0, soundClipIndex2=7, soundClipIndex3=19; // Variable to keep track of the current sound clip indexes. soundClipIndex1 keeps track of the index for the random sound clips. soundClipIndex2 keeps track of the index for the numbers sound clips. soundClipIndex3 keeps track of the index for the letters sound clips.  These variables are initiated to one less than the track number for the track that begins that section of the MP3 files on the micro SD card.  This is because the first time the button is pressed the index will be incremented to the first track in that section.
int soundClipIndex1P = 0, soundClipIndex2P = 7, soundClipIndex3P = 19; //Variable to keep track of the current sound clip index

void loop() 
{ //Start of void loop
  pb1St=!digitalRead(pb1); //Reads the state of the pushbutton named and stores it to the variable on the left side of the equals sign (reads either 0 or 1)
  pb2St=!digitalRead(pb2); //Reads the state of the pushbutton named and stores it to the variable on the left side of the equals sign (reads either 0 or 1)
  pb3St=!digitalRead(pb3); //Reads the state of the pushbutton named and stores it to the variable on the left side of the equals sign (reads either 0 or 1)  
  pb4St=!digitalRead(pb4); //Reads the state of the pushbutton named and stores it to the variable on the left side of the equals sign (reads either 0 or 1)  
  pb5St=!digitalRead(pb5); //Reads the state of the pushbutton named and stores it to the variable on the left side of the equals sign (reads either 0 or 1)  

  if(pb1St != pb1StP && pb1St==HIGH)
  { //Start of if(pb1St != pb1StP && pb1St==HIGH)
    //soundClipIndex1 = (soundClipIndex1 + 1) % 8; // Increment the sound clip index and wrap around if needed - Note: There are 7 MP3 files.  But 8 had to be used with the modulo operator because 7 causes it to skip the last MP3 file.  There is still the issue of a dead button press (a button press where no song plays).  But it isn't a big issue.  
    soundClipIndex1++; //Add 1 each time the button is pressed
    
    if(soundClipIndex1>7)
    { //Start of if(soundClipIndex1>7)
      soundClipIndex1 = 1; //Set it back to 1
    } //End of if(soundClipIndex1>7)
  } //End of if(pb1St != pb1StP && pb1St==HIGH)

  if(pb2St != pb2StP && pb2St==HIGH)
  { //Start of if(pb2St != pb2StP && pb2St==HIGH)
    animationIndex = (animationIndex + 1) % 4; // Increment the animation index and wrap around if needed     
  } //End of if(pb2St != pb2StP && pb2St==HIGH)

  if(pb3St != pb3StP && pb3St==HIGH)
  { //Start of if(pb3St != pb3StP && pb3St==HIGH)
    changeColor();   
  } //End of if(pb3St != pb3StP && pb3St==HIGH)

  if(pb4St != pb4StP && pb4St==HIGH)
  { //Start of if(pb4St != pb4StP && pb4St==HIGH)
    soundClipIndex2++; //Add 1 each time the button is pressed

    if(soundClipIndex2>19)
    { //Start of if(soundClipIndex2>19)
      soundClipIndex2 = 8; //Set it back to 8
    } //End of if(soundClipIndex2>19)
  } //End of if(pb4St != pb4StP && pb4St==HIGH)

  if(pb5St != pb5StP && pb5St==HIGH)
  { //Start of if(pb5St != pb5StP && pb5St==HIGH)
    soundClipIndex3++; //Add 1 each time the button is pressed

    if(soundClipIndex3>46)
    { //Start of if(soundClipIndex3>46)
      soundClipIndex3 = 20; //Set it back to 20
    } //End of if(soundClipIndex3>46)
  } //End of if(pb5St != pb5StP && pb5St==HIGH)

  switch(animationIndex)
  { // Start of switch(animationIndex)
    case 0:
      FastLED.delay(1000/FRAMES_PER_SECOND); //Insert a delay to keep the framerate modest
      EVERY_N_MILLISECONDS(20) { hue++; }
      rainbow(); //Call the rainbow animation
      FastLED.show(); //Display the changes on the LED strip     
      break;
    case 1:
      FastLED.delay(1000/FRAMES_PER_SECOND); //Insert a delay to keep the framerate modest
      EVERY_N_MILLISECONDS(20) { hue++; }
      rainbowWithGlitter(); // Call the rainbow with glitter animation
      FastLED.show(); //Display the changes on the LED strip
      break;
    case 2:
      FastLED.delay(1000/FRAMES_PER_SECOND); //Insert a delay to keep the framerate modest
      EVERY_N_MILLISECONDS(20) { hue++; }
      confetti(); // Call the confetti animation
      FastLED.show(); //Display the changes on the LED strip
      break;
    case 3:
      FastLED.delay(1000/FRAMES_PER_SECOND); //Insert a delay to keep the framerate modest
      EVERY_N_MILLISECONDS(20) { hue++; }
      juggle(); // Call the juggle animation
      FastLED.show(); //Display the changes on the LED strip
      break;
    default:
      fill_solid(leds2, NUM_LEDS, CRGB::Black); // Default to turning LEDs off
      FastLED.show();
      delay(20);
      break;
  } // End of switch(animationIndex)

  if(soundClipIndex1 != soundClipIndex1P) //Only play a new MP3 file if the index has changed
  { //Start of if(soundClipIndex1 != soundClipIndex1P)
      switch(soundClipIndex1)
      { //Start of switch(soundClipIndex1)
        case 1:
          playTrack(1);
          break;
        case 2:
          playTrack(2);
          break;
        case 3:
          playTrack(3);
          break;
        case 4:
          playTrack(4);
          break;
        case 5:
          playTrack(5);
          break;
        case 6:
          playTrack(6);
          break;
        case 7:
          playTrack(7);
          break;
      } //End of switch(soundClipIndex1)
  } //End of if(soundClipIndex1 != soundClipIndex1P)

  if(soundClipIndex2 != soundClipIndex2P) //Only play a new MP3 file if the index has changed
  { //Start of if(soundClipIndex2 != soundClipIndex2P)
      switch(soundClipIndex2)
      { //Start of switch(soundClipIndex2)
        case 8:
          display.clearDisplay(); //Clear the internal buffer of the OLED display
          display.display(); //Send the current contents of the internal buffer to the physical OLED display
          playTrack(8);
          break;
        case 9:
          playTrack(9);
          displayCenteredString("0");
          break;
        case 10:
          playTrack(10);
          displayCenteredString("1");
          break;
        case 11:
          playTrack(11);
          displayCenteredString("2");
          break;
        case 12:
          playTrack(12);
          displayCenteredString("3");
          break;
        case 13:
          playTrack(13);
          displayCenteredString("4");
          break;
        case 14:
          playTrack(14);
          displayCenteredString("5");
          break;
        case 15:
          playTrack(15);
          displayCenteredString("6");
          break;
        case 16:
          playTrack(16);
          displayCenteredString("7");
          break;
        case 17:
          playTrack(17);
          displayCenteredString("8");
          break;
        case 18:
          playTrack(18);
          displayCenteredString("9");
          break;
        case 19:
          playTrack(19);
          displayCenteredString("10");
          break;
      } //End of switch(soundClipIndex2)
  } //End of if(soundClipIndex2 != soundClipIndex2P)

  if(soundClipIndex3 != soundClipIndex3P) //Only play a new MP3 file if the index has changed
  { //Start of if(soundClipIndex3 != soundClipIndex3P)
      switch(soundClipIndex3)
      { //Start of switch(soundClipIndex3)
        case 20:
          display.clearDisplay(); //Clear the internal buffer of the OLED display
          display.display(); //Send the current contents of the internal buffer to the physical OLED display
          playTrack(20);
          break;
        case 21:
          playTrack(21);
          displayCenteredString("A");
          break;
        case 22:
          playTrack(22);
          displayCenteredString("B");
          break;
        case 23:
          playTrack(23);
          displayCenteredString("C");
          break;
        case 24:
          playTrack(24);
          displayCenteredString("D");
          break;
        case 25:
          playTrack(25);
          displayCenteredString("E");
          break;
        case 26:
          playTrack(26);
          displayCenteredString("F");
          break;
        case 27:
          playTrack(27);
          displayCenteredString("G");
          break;
        case 28:
          playTrack(28);
          displayCenteredString("H");
          break;
        case 29:
          playTrack(29);
          displayCenteredString("I");
          break;
        case 30:
          playTrack(30);
          displayCenteredString("J");
          break;
        case 31:
          playTrack(31);
          displayCenteredString("K");
          break;
        case 32:
          playTrack(32);
          displayCenteredString("L");
          break;
        case 33:
          playTrack(33);
          displayCenteredString("M");
          break;
        case 34:
          playTrack(34);
          displayCenteredString("N");
          break;
        case 35:
          playTrack(35);
          displayCenteredString("O");
          break;
        case 36:
          playTrack(36);
          displayCenteredString("P");
          break;
        case 37:
          playTrack(37);
          displayCenteredString("Q");
          break;
        case 38:
          playTrack(38);
          displayCenteredString("R");
          break;
        case 39:
          playTrack(39);
          displayCenteredString("S");
          break;
        case 40:
          playTrack(40);
          displayCenteredString("T");
          break;
        case 41:
          playTrack(41);
          displayCenteredString("U");
          break;
        case 42:
          playTrack(42);
          displayCenteredString("V");
          break;
        case 43:
          playTrack(43);
          displayCenteredString("W");
          break;
        case 44:
          playTrack(44);
          displayCenteredString("X");
          break;
        case 45:
          playTrack(45);
          displayCenteredString("Y");
          break;
        case 46:
          playTrack(46);
          displayCenteredString("Z");
          break;
      } //End of switch(soundClipIndex3)
  } //End of if(soundClipIndex3 != soundClipIndex3P)

  pb1StP=pb1St; //Stores the current pushbutton state before it is overwritten in the next iteration of the void loop
  pb2StP=pb2St; //Stores the current pushbutton state before it is overwritten in the next iteration of the void loop
  pb3StP=pb3St; //Stores the current pushbutton state before it is overwritten in the next iteration of the void loop
  pb4StP=pb4St; //Stores the current pushbutton state before it is overwritten in the next iteration of the void loop
  pb5StP=pb5St; //Stores the current pushbutton state before it is overwritten in the next iteration of the void loop
  soundClipIndex1P = soundClipIndex1; //Stores the current sound clip index before it is possibly changed in the next iteration of the void loop
  soundClipIndex2P = soundClipIndex2; //Stores the current sound clip index before it is possibly changed in the next iteration of the void loop
  soundClipIndex3P = soundClipIndex3; //Stores the current sound clip index before it is possibly changed in the next iteration of the void loop
} //End of void loop

//Function Definitions
//this function sends the actual command
//It receives the command byte and ParData is optional info such as track number or volume depending on the command
void sendDFCommand(byte Command, int ParData) 
{ //Start of sendDFCommand function definition
byte commandData[10]; //This holds all the command data to be sent
byte q;
int checkSum;
Serial.print("Com: ");
Serial.print(Command, HEX);
//Each command value is being sent in Hexadecimal
commandData[0] = 0x7E;//Start of new command
commandData[1] = 0xFF;//Version information
commandData[2] = 0x06;//Data length (not including parity) or the start and version
commandData[3] = Command;//The command that was sent through
commandData[4] = 0x01;//1 = feedback
commandData[5] = highByte(ParData);//High byte of the data sent over
commandData[6] = lowByte(ParData);//low byte of the data sent over
checkSum = -(commandData[1] + commandData[2] + commandData[3] + commandData[4] + commandData[5] + commandData[6]);
commandData[7] = highByte(checkSum);//High byte of the checkSum
commandData[8] = lowByte(checkSum);//low byte of the checkSum
commandData[9] = 0xEF;//End bit
for (q = 0; q < 10; q++) {
//Serial3.write(commandData[q]); //This version was when the command was using the hardware serial port built into the Arduino Mega
MySerial.write(commandData[q]);
}
Serial.println("Command Sent: ");
for (q = 0; q < 10; q++) {
Serial.println(commandData[q],HEX);
}
Serial.println("End Command: ");
delay(100);
} //End of sendDFCommand function definition
//play a specific track number
void playTrack(int tracknum){
Serial.print("Track selected: ");
Serial.println(tracknum);
sendDFCommand(0x03, tracknum);
}
//plays the next track
void playNext(){
Serial.println("Play Next");
sendDFCommand(0x01, 0);
}
//volume increase by 1
void volumeUp() {
Serial.println("Vol UP");
sendDFCommand(0x04, 0);
}
//volume decrease by 1
void volumeDown() {
Serial.println("Vol Down");
sendDFCommand(0x05, 0);
}
//set volume to specific value
void changeVolume(int thevolume) {
sendDFCommand(0x06, thevolume);
}

void changeColor() 
{ //Start of changeColor function definition
  static uint8_t colorIndex = 0; // Variable to keep track of the current color index

  switch(colorIndex)
  { //Start of switch(colorIndex)
    case 0:
      display.clearDisplay(); //Clear the internal buffer of the OLED display
      display.display(); //Send the current contents of the internal buffer to the physical OLED display
      playTrack(47);      
      break;
    case 1:
      allRed();
      displayLeftString("RED");
      playTrack(48);
      break;
    case 2:
      allOrange();
      displayLeftString("ORANGE");
      playTrack(49);
      break;
    case 3:
      allYellow();
      displayLeftString("YELLOW");
      playTrack(50);
      break;
    case 4:
      allGreen();
      displayLeftString("GREEN");
      playTrack(51);
      break;
    case 5:
      allBlue();
      displayLeftString("BLUE");
      playTrack(52);
      break;
    case 6:
      allBlueViolet();
      displayLeftString("VIOLET");
      playTrack(53);
      break;
    default:
      fill_solid(leds2, NUM_LEDS, CRGB::Black); // Default to turning LEDs off
      FastLED.show();
      delay(20);
      break;  
  } //End of switch(colorIndex)

  colorIndex = (colorIndex + 1) % 7; // Increment the color index and wrap around if needed  
} //End of changeColor function definition

void displayCenteredString(const char* input) //Function to display a string (up to two characters) centered on the OLED display
{ //Start of displayCenteredString function definition
    // Clear the display
    display.clearDisplay();

    //Set text size and color
    display.setTextSize(4); // Large text size
    display.setTextColor(SSD1306_WHITE);

    //Calculate text width and height in pixels
    int16_t x1, y1;
    uint16_t w, h;
    display.getTextBounds(input, 0, 0, &x1, &y1, &w, &h);

    //Calculate coordinates to center the text
    int16_t x = (SCREEN_WIDTH - w) / 2;
    int16_t y = (SCREEN_HEIGHT - h) / 2;

    //Set cursor position and print input
    display.setCursor(x, y);
    display.print(input);

    display.display(); //Update the display
} //End of displayCenteredString function definition

void displayLeftString(const char* input) //Function to display a string (more than two characters) at the left side on the OLED display
{ //Start of displayLeftString function definition
    // Clear the display
    display.clearDisplay();

    // Set text size and color
    display.setTextSize(3); // Medium text size
    display.setTextColor(SSD1306_WHITE);

    // Calculate text height in pixels (width is not needed since text starts at the left)
    int16_t x1, y1;
    uint16_t w, h;
    display.getTextBounds(input, 0, 0, &x1, &y1, &w, &h);

    // Set the y-coordinate to center the text vertically
    int16_t y = (SCREEN_HEIGHT - h) / 2;

    // Set the cursor to the far left and vertically centered
    display.setCursor(0, y);
    display.print(input);

    display.display(); // Update the display
} //End of displayLeftString function definition

//References
//Reference 1- ESP32AutoTrumpetRev0.ino
//Reference 2- buttonExampleRev0.ino
//Reference 3- DemoReel100RainbowBareMinimumRev0.ino
//Reference 4- LEDStripAsyncWebServerControlRev1.ino
