/*
This program has been written by Lindsay Harvey VK2AMV for the Arduino IDE

Its intended purpose is for a Digispark microcontroller module, based on an Atmel ATTiny85 MCU, to initialize an Analog Devices AD9833 DDS Module to produce a 1.647Mhz waveform for injection into the Z3 crystal input of Codan 8528 radios for Lower Side Band usage

5v power for the Digispark module and AD9833 module can be tapped off the 74HC02 NOR gate right next to the Z3 crystal position, pin 7 is GND and pin 14 is 5v VCC
I also recommend connecting a 10-100nf ceramic capacitor between the AD9833 output and the Z3 point on the board to remove the DC offset

This program was written for the use of the membership of Amateur Radio Central West Group, based in Orange NSW, Australia

All copyrights are retained by Lindsay Harvey VK2AMV
This program may be distributed outside the membership of Amateur Radio Central West Group providing this notice remains attached in its entirety
Email: vk2amv "at symbol" lgharvey "dot" net

Version 1.2
*/



/*
Change Log

6/07/2016   V1.0
Initial version

31/07/2016  V1.1
Added Nointerrupts command to the sleep function as the timer could still wake the MCU again which we don't want.
Cleaned up some of the other code a little bit

17/10/2016  V1.2
Changed from Square Wave output to Sine Wave output to kill a high pitched whine that was being introduced into the audio output via the Z3 input while in Upper Side Band Mode (LSB was not effected)
This has a result of reducing the DDS module output, Square wave does around 4.5 volts peak to peak, Sine wave output is only around 0.3 volts peak to peak
This level still seems to be plenty to drive the crystal buffer in the radio however, and in my radio it killed off the whine completely.
Many thanks for Dave VK2JDS for noting this problem and his assitance in fixing it.


Still to do: Change the frequency calculation so a decimal value can be used instead of having to pre calculate the HEX value.
*/



#define __AVR_ATtiny85__   // This tells the compiler that the MCU is an ATTiny85 otherwise the sleep library won't work as it won't know what the MCU is without this line
#include <avr/sleep.h>	   // This tells the compiler to include the sleep library for putting the MCU to sleep when we have finished initializing the AD9833

#define ss 1  // This defines what pin of the Arduino the FSYNC pin of the AD9833 is connected, active is low to enable the AD9833 register's for data input
#define mosi 3  // This defines what pin of the Arduino the Data In pin of the AD9833 is connected
#define sclk 2  // This defines what pin of the Arduino the Clock pin of the AD9833 is connected

#define pulseLow(pin) {digitalWrite(pin, LOW); digitalWrite(pin, HIGH); } // This function puleses a pin low and then sets it high again for clocking in data to the AD9833.

const int reset = 0x100;   //Used below to write a '1' to AD9833 Control register bit D8 to do a power on reset of the chip.
word mask = 1; // The bitmask for use inside the send_word function

// When you do an exit reset after sending the frequency and phase data you need to specify the waveform value as part of the exit reset command.
// In reality for this purpose for use in the Codan 8528 radio you only need SQUARE wave, the other options are just included here for completness
const int SINE = 0x2000;                   // Binary 00100000 00000000 for SINE wave output
const int SQUARE = 0x2020;                 // Binary 00100000 00100000 for SQUARE wave output
const int TRIANGLE = 0x2002;               // Binary 00100000 00000010 for TRIANGLE wave output

int freqlow = 0x0;   // Define the frequency low word integer
int freqhigh = 0x0;  // Define the frequency high word integer
int waveform = 0x0;  // Define the waveform output setting integer



void setup() {


// This is the waveform to set the AD9833 to, can be SINE, SQUARE, or TRIANGLE
// The AD9833 is limited to 0.6Volts when in SINE wave mode so we are using SQUARE wave mode because the output is about 4.5Volts in that mode
// which suits the logic level inputs on the Codan Z3 crystal input nicely
waveform = SINE;


// Frequency is made up of two 14 bit words, low word and high word sent in that order, they are sent as two 16 bit words in MSB first order
// the two most significant bits of each 16 bit word is the register select (Always 01 for the this usage in the AD9833), with the remaining 14 bits setting the frequency 

// I calculated the settings here using this site http://stompville.co.uk/AD9833.php
// For reference whan calculating frequency values the AD9833 has a 25Mhz clock crystal
// The values here produce 1.647Mhz when outputting a SINE wave
//freqlow = 0x5830;
//freqhigh = 0x4437;

// Temp change to 1,000 Hz for 2 meter Fox audio
freqlow = 0x69f1;
freqhigh = 0x4000;

// This is for 3.294Mhz, seems square wave output needs to be set to double the required frequency
// still investigating why exactly but I think I am just missing something completly obvious
// not too worried though as it works fine so think I will just leave it be
//freqlow = 0x7060;
//freqhigh = 0x486e;


 pinMode(ss, OUTPUT);  // Sets the ATTiny85 pin connected to the AD9833 FSYNC pin as an output
 pinMode(sclk, OUTPUT); // Sets the ATTiny85 pin connected to the AD9833 clock pin as an output
 pinMode(mosi, OUTPUT); // Sets the ATTiny85 pin connected to the AD9833 data in pin as an output

 
 digitalWrite(ss, HIGH);  // Sets the AD9833 FSYNC pin initial state to HIGH for inactive
 digitalWrite(sclk, HIGH); // Sets clock pin initial state to HIGH for clock idle
 
 delay(500);  // Wait 500 milliseconds to let everything settle
 send_word(reset);  //Power on reset of AD9833 as recommended in Datasheet
 delay(500);   // Wait 500 milliseconds to let the reset complete and let everything settle again before sending frequency setting


 
 // Ok time to send the required data to get the required waveform out of the AD9833 module
 // The lines below send the required data in words of 16 bits at a time using the send_word function to clock it out to the AD9833

 send_word(0x2100);  // These two bytes puts the AD9833 in reset mode and also ready to receive frequency settings
 send_word(freqlow);  // Frequency low word, first two bits is the register select, then 14 bits of frequency information
 send_word(freqhigh);  // Frequency high word, first two bits is the register select, then 14 bits of frequency information
 send_word(0xC000);  // These two bytes set the phase register, in this case it is 0
 send_word(waveform);  // These two bytes set the AD9833 waveform output, and do an exit reset to start waveform output



// Now lets put the Arduino to sleep and halt the CPU and stop the internal oscillator
// no point having an unneeded oscillator generating signals inside the radio when its job is now finished

sleepNow();     // sleep function called here

}

void loop() { // No code is here as it is only needed simply because the compiler expects the void loop to be here
}


void sleepNow()  // This function is for putting the MCU to sleep, shutting down all functions and stopping the internal oscillator
{
noInterrupts();          // This turns off all interrupts so nothing will wake the MCU again once it has gone to sleep
set_sleep_mode(SLEEP_MODE_PWR_DOWN);   // sleep mode is set here
sleep_enable();          // enables the sleep bit in the MCUCR register
sleep_mode();            // here the device is actually put to sleep!!
}


void send_word(int data)  // This function is for clocking the data into the AD9833, we need to bit bang this manually because the ATTiny85 MCU does not have hardware SPI built in
{
  digitalWrite(ss, LOW); // Put the FSYNC pin active LOW to notify the AD9833 we are going to be sending it data
  delayMicroseconds(100); // Delay to give time for the AD9833 to get its registers ready to receive data as per the datasheet
  
   for (mask = 1000000000000000; mask>0; mask >>= 1) { //This iterates through bit mask to select one bit of the word at a time to clock out to the AD9833
     if (data & mask){ // If bitwise AND resolves to true then
       digitalWrite(mosi,HIGH); // Set DATA pin to 1
       pulseLow(sclk);   // After the DATA pin is set, the CLOCK pin is pulsed low to clock the bit into the AD9833 register
     }
     else{ // If bitwise AND resolves to false then
       digitalWrite(mosi,LOW); // Set DATA pin to 0
       pulseLow(sclk);   // After the DATA pin is set, the CLOCK pin is pulsed low to clock the bit into the AD9833 register
     }
     delayMicroseconds(20); //Small delay before returning to the beginning of the FOR loop to clock the next bit into AD9833 register
   }
     digitalWrite(ss, HIGH); // When all bits have been clocked into the AD9833 we put FSYNC back to inactive HIGH to end the data transaction
}
