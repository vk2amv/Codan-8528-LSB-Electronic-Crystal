# Codan-8528-LSB-Electronic-Crystal

This program has been written by Lindsay Harvey VK2AMV for the Arduino IDE

Its intended purpose is for a Digispark microcontroller module, based on an Atmel ATTiny85 MCU, to initialize an Analog Devices AD9833 DDS Module to 
produce a 1.647Mhz waveform for injection into the Z3 crystal input of Codan 8528 radios for Lower Side Band usage.

5v power for the Digispark module and AD9833 module can be tapped off the 74HC02 NOR gate right next to the Z3 crystal position, pin 7 is GND and pin 14 is 5v VCC
I also recommend connecting a 10-100nf ceramic capacitor between the AD9833 output and the Z3 point on the board to remove the DC offset

This program was written for the use of the membership of Amateur Radio Central West Group, based in Orange NSW, Australia

All copyrights are retained by Lindsay Harvey VK2AMV

This program may be distributed outside the membership of Amateur Radio Central West Group providing this notice remains attached in its entirety
Email: vk2amv "at symbol" lgharvey "dot" net
