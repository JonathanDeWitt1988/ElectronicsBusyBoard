# ElectronicsBusyBoard
These are the files needed to create an electronics busy board toy for a young child.

This project was born from the idea that my 1-year-old son loves to be involved when he sees me working on circuits.  I figured since he loves to play with circuits I would build him one of his own that he couldn't harm the circuitry or himself but could interact with it.  The inputs/outputs are as follows:
Inputs- 5 momentary contact pushbuttons
Outputs- 2 WS2812B LED strips (20 LEDs each), a 0.96" SSD1306 OLED display, and a DFPlayer Mini MP3 module with an 8 Ohm speaker

Button Functions:
After each button is done stepping through the MP3 files/animations the button controls the next press starts back over at the beginning (it wraps back around).
Pushbutton 1- This steps through the first 7 MP3 files (my wife and I saying encouraging messages to our son and singing him songs)
Pushbutton 2- This steps through the animations for the bottom LED strip. This LED strip does start off with a default animation without any user input.
Pushbutton 3- This steps through the colors on the top LED strip and also causes the MP3 files to play which name the color as well as for the text name of the color to appear on the OLED so that our son learns to say and spell the color corresponding to what he is seeing on the LED strip.
Pushbutton 4- This steps through the numbers 0 through 10 while also stating the number (DFPlayer Mini again) and showing the number on the OLED.
Pushbutton 5- This steps through all 26 letters of the alphabet while also stating the letter (DFPlayer Mini again) and showing the capitalized letter on the OLED.

The SD card on the DFPlayer Mini must be formatted in Fat16 or Fat32 format.  The order you add the files to the SD card matters.  The first file you add is what the DFPlayer mini considers file 001 regardless of file name.  The second file you add is what the DFPlayer mini considers file 002 regardless of file name.  You get the idea.

I'm uploading a text file which shows the actual names of the files on my micro SD card so you can see how to name your files.  You'll have to go in and edit the code to select your files with whatever scheme you want to use.  I recorded the audio clips using Sound Recorder built into windows.  Note you'll want to change the default Sound Recorder setting to record to MP3 format.  You have the code and schematic.  The schematic is available as a .png file and .pdf file.  Both are the same schematic.  You can zoom in.

Source of ESP32 Node MCU board used
https://www.aliexpress.us/item/2251832773226694.html
I buy the ESP-32 38Pin option.

Demonstration Video
https://www.youtube.com/watch?v=pJFP9uuFRVQ
