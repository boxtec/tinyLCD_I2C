tinyLCD_I2C
===========

Description:
------------
tinyLCD_I2C is a Firmware for LCD I2C backpack based on ATtiny84 with an Arduino library as drop-in replacment for the LiquidCrystal library.

Instructions
------------
Please make sure to use the TinyWireS library from this repo with the ATtiny implementation from
http://code.google.com/p/arduino-tiny/

Also you should make the following change in ./hardware/tiny/cores/tiny/wiring.c to improve performance of the TinyWireS library:

```ISR(MILLISTIMER_OVF_vect)
{
  sei();
  // copy these to local variables so they can be stored in registers
  // (volatile variables must be read from memory on every access)
```
Basically add the line
```
  sei();
```
to the *ISR(MILLISTIMER_OVF_vect)* function so that it looks like above excerpt.

See http://forum.boxtec.ch/index.php/topic,2225.msg2845.html#msg2845 for a discussion on this issue.

Boards / PCBs
-------------
The boards directory contains a Fritzing THT design on which current development is based and an Eagle 6.x SMD design which is currently in production for first prototypes.

BOM
---
THT-Board:
- 220nF Cer Cap
- 100k(R4), 4.7k(R2), 150R(R3)
- ATtiny84A-PU
- 14 Pin DIP socket (optional)
- Header female (16pins)
- 2x3 Pin Header male

SMD-Board:
- 220nF Cer Cap
- 100k, 1k, 150R
- ATtiny84-SU (SOIC)
- Header female (16pins)
- 2x3 Pin Header male

Forum / contact
---------------
Issues around the tinyLCD_I2C board and firmware can be discussed here:
- http://forum.boxtec.ch/index.php/topic,2225.0.html

Credits
-------
Credits go to ..

.. *brohogan* for bringing the [TinyWireS library](http://playground.arduino.cc/Code/USIi2c) to life in the first place.

.. pylon for fixing all the timing issue and race conditions in above library and enhancing the tinyLCD_I2C greatly (i.e. by adding SPI)

.. MathiasW for adding a SMD design with reduced size

