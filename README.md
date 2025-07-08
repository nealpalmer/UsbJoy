# UsbJoy
USB Joystick Controller 12Digital/SmartButton, 4Analog, 1Rotary, 1NeoPixel

This is a couple of circuit boards that make up a fancy USB joystick controller for FRC #2984.
It has both a HID joystick interface and a HID uart interface presented to the computer.

To add to this readme:
- mechanical drawing of the board (wid, hei, hole locations)
- design files for firmware, kicad, 3d, software
- upload 3d design files to makerworld
- write the software application for programming the pic
- Create a bill of materials for all parts necessary, with ordering links (include arduino, 30mm led, cables, jst cable ends, 600ppm encoder)
- Add mechanical parts to make the spinning rotary encoder.

![image](https://github.com/user-attachments/assets/ea97f88b-f23c-439b-81eb-e5cb5ec8e4a8)
![image](https://github.com/user-attachments/assets/5b4c13d2-de07-41db-afa2-7e9a51e008a0)
![image](https://github.com/user-attachments/assets/e703f007-66c7-4fff-99f2-dc9885223b1f)

**Features:**
- 12 Digital inputs for buttons (3-pin White/Yellow/Green JST XH: 5V, signal with 10k pullup, GND)
- 4 Analog inputs for joysticks or linear/rotary potentiometers (10k ohms) (3-pin Blue JST XH: 5V, signal with 100k+100k pullup+pulldown, GND).
- 1 Digital input can be used to drive a NeoPixel string (be careful of 500mA fuse) (3-pin Yellow)
- 4-pin JST XH rotary encoder for Z-axis (600 ppr*4phase = 2400/rev resolution assumed)
- Arduino Pro Micro USB-C 5V ATmega32U4 controller board used for ease of soldering.  Make sure to get one with the fuse on the TOP side of the board (i.e. no bottom side components).  [amazon.com](https://www.amazon.com/gp/product/B0B6HYLC44)
- SmartButton for all 12 Digital inputs
- 4 Digital connectors (Green) can have their 5V power switched on/off for use with simple led outputs (SmartButton requires it to always be powered on).
- ESD diodes on all 16 3-pin connectors
- Configuration of SmartButton configuration done through the USB-UART interface.  Some settings are saved in EEPROM (Neopixel config, ...) , some are temporary (LED values).
- Coming soon: Programming of PIC micro-controller done over USB-UART with button press required.
- Colored JST-XH 3-pin connectors are available on [aliexpress.com](https://www.aliexpress.us/item/3256806937445015.html)
- Colored JST-XH terminal 3-pin connectors are available on [aliexpress.com](https://www.aliexpress.us/item/3256804014172692.html)
- pre-crimped 3-pin cables are available on [aliexpress.com](https://www.aliexpress.us/item/3256807213104605.html)  Just be aware that the pins have to be removed, and swapped so that it is a 1:1 cable instead of a 1:3 cable (or you are going to connect GND to 5V, and destroy your SmartButtons).
- pre-crimped 10-pin cables are available on [amazon.com](https://www.amazon.com/dp/B0B2RCW5JF) And you would have to replace the connectors with a 3-pin version (cheap, and comes from US, so comes quickly).

**SmartButton features**
- Uses 30mm illuminated arcade buttons. [https://www.amazon.com/EG-STARTS-Illuminated-Buttons-Raspberry/dp/B01N11BDX9](https://www.amazon.com/EG-STARTS-Illuminated-Buttons-Raspberry/dp/B01N11BDX9)
- 24mm buttons might work, but haven't tested them yet.
- 3-pin interface (5V, signal, GND)
- Some solder jumpers can be filled, and some jumpers cut to turn a smart button into a dumb button (i.e. no 5V).
- LED options
  - 0: OFF
  - 1: ON
  - 2: ON for 100ms when button gets pressed
  - 3: ON while button is pressed
  - 4: FLASHING 100ms on/100ms off
  - 5: SIN()
  - 6: FLASHING while button is pressed
  - 7: Button toggles state each time it is pressed: LED lit when in 'pressed' state
  - 8: Button toggles state each time it is pressed: LED flashes when in 'pressed' state
  - 9: you can add more if you want...

**NeoPixel**
- Uses an off-the-shelf neopixel array (WS2812B 5V leds) [amazon.com](https://www.amazon.com/dp/B09PBHJG6G) [ring_of_leds](https://www.amazon.com/dp/B08GPWVD57)
- Takes over Digital input #11 (the Yellow connector)
- Color choices:
- 0x00-0x3f: Black-to-White solid color choices (4 values per R,G,B)
- 0x80-0xff: Single white LED on (can be set to match the rotary encoder)
- Rainbow
- Moving Rainbow

**Files included:**
- UsbJoy_d12_a4_r-rev2/: Kicad layout files, and gerbers
- SmartButton-rev2/: Kicad layout files, and gerbers
- mechanics/: openscad & STL files for a couple of mounts for UsbJoy using the SmartButton
- software/SmartButton_firmware/:
- software/UsbJoy_firmware/: firmware for the Atmega32U4 microcontroller
- software/UsbJoy_pic_firmware/: firmware 
- software/program_pic/: small application to program the pic16f1778 on UsbJoy
- software/

**Manufacturing suggestions:**
- [oshpark.com](https://oshpark.com/) has wonderful pricing for qty3 of 2and4 layer boards and ships in 10 calendar days for almost everything (and you can pay more to get it earlier, or pay for faster shipping)
  - [UsbJoy_R2](https://oshpark.com/shared_projects/QPZCB1ZT) board order
  - [SmartButton_R2](https://oshpark.com/shared_projects/OV1uAkH7) board order
- A 3d-printer can make the UsbJoy board attach directly to the screw threads of one of the buttons.  A couple of sample mount's source files are here.
- Testing for button functionality can be done in windows with the built-in "USB game controllers"->properties window to see the buttons that get pressed. NEED A PICTURE HERE!
- You can extract a 3d model of the boards from kicad to use in your 3d cad system.
- digikey has all of the electrical components necessary (except for the LED and Arduino board).
- The arduino pro micro has a 0.5A fuse on it.  The bottom side of it will NOT be accessible.  If that 0.5A fuse is on the bottom side, and you have over current, then your boards will not be salvageable.  So get a version that has all the components on the top side.
- The discrete components are 0603 packages for easier soldering.
- Get some illuminated magnifying glasses for ease of soldering of small components. [amazon.com](https://www.amazon.com/YOCTOSUN-Magnifying-Rechargeable-1-5X-3-5X-Magnifier/dp/B0D8J3TYL3)
- Solder all of the SMT components before any thru-hole components.  Required for both boards, because some of the thru-hole soldering will make SMT parts unreachable.

**How to program Arduino Pro Micro's Atmega32U4 (the USB device):**
- Run The Arduino GUI version X.XX (whatever is current as of May 2025 works).
- Load the *.ino file
- Set up the board to be "leonardo". -- need to verify this
- Plug in the board into USB
- Download from the Arduino GUI.
- Programming does not clear the EEPROM, so any settings should remain unaffected.

**How to program pic16f1778 on UsbJoy:**
- Run the Microchip IDE (what version minimum?)
- Load the project
- Compile the project for production
- Using PICKIT5: Use one of the current programmers pickit5 is probably preferred, and plug it into the 5-pin holes on the bottom side of the board.  And have the IDE program the device.
- Using USB: remove cables from 3 connectors (A, B, C), hold down the button on the board, and run "program_pic pic.hex", then release the button after programming completion

**How to program pic16f15213 on SmartButton:**
- Run the Microchip IDE (what version minimum?)
- Load the project
- Compile the project for production
- Using PICKIT5: Make sure the button isn't pressed, Connect the 3-pin cable to the UsbJoy (for 5V power), plug the pickit5 into the 5-pin holes on the SmartButton board.  And have the IDE program the device.

**How to initially configure UsbJoy:**
- Connect USB to a PC.
- Open a terminal program (I like [MobaXterm](https://mobaxterm.mobatek.net/)), and connect to the correct com port (use windows device manager, and find the 'leonardo' port listed) (linux: 'dmesg' to find the correct '/dev/ttyUSB#' when you plug into USB).
- Any baud rate is fine, because it is a virtual serial port.
- 'h'+enter to get the help menu
- All settings commands are "letter+hex_number+enter".
- Important EEPROM settings to run:
- "m#val" to set the SmartLed power-on setting for that port (not for the SmartLed itself) to the desired LED pattern (and toggle functionality)
- "n#" to set the number of neopixel leds in the string - set to "n0" to indicate that there isn't a neopixel string.
- "v#" to set the power-on setting for the Green ports - probably want them always on "vF" unless they are being used in some interesting way
- 
  
