# UsbJoy
USB Joystick Controller 12Digital/SmartButton, 4Analog, 1Rotary, 1NeoPixel

This is a couple of circuit boards that make up a fancy USB joystick controller for FRC #2984.
It has both a HID joystick interface and a HID uart interface presented to the computer (i.e. it works in Windows and Linux).

To add to this readme:
- mechanical drawing of the board (wid, hei, hole locations)
- design files for firmware, kicad, 3d, software
- upload 3d design files to makerworld (add links to it here)
- write the software application for programming the pic
- Create a bill of materials for all parts necessary, with ordering links (include arduino, 30mm led, cables, jst cable ends, 600ppm encoder)
- Add mechanical parts to make the spinning rotary encoder (acrylic piece, 3d printed springs)
- A description of how to get feedback from the robot to the leds.  Does this require a networktables application?  Or use Force-Feedback mechanisms?

![image](https://github.com/user-attachments/assets/ea97f88b-f23c-439b-81eb-e5cb5ec8e4a8)
![image](https://github.com/user-attachments/assets/5b4c13d2-de07-41db-afa2-7e9a51e008a0)
![image](https://github.com/user-attachments/assets/e703f007-66c7-4fff-99f2-dc9885223b1f)

**Features:**
- 12 Digital inputs for buttons (3-pin White/Yellow/Green JST XH: 5V, signal with 10k pullup, GND)
  - 12 is probably enough buttons for a FRC operator panel.  We use 2x of these on the operator panel, and 1 on the driver panel.
  - simple buttons and switches can be used, just connect to the 'signal' and 'GND' pins of the 3-pin connector, and ignore the '5V' pin.
- 4 Analog inputs for joysticks or linear/rotary potentiometers (10k ohms) (3-pin Blue JST XH: 5V, signal with 100k+100k pullup+pulldown, GND).
  - 1 Analog [joystick](https://www.amazon.com/dp/B08CGYGMJL) for driving (X & Y)
  - 2 [linear potentiometers](https://www.amazon.com/dp/B07HNY7VWC) are useful for controlling speeds or small adjustments to some positioning
  - There are more analog input pins that could be used for more analog inputs, but that is left upto the user to figure out if they care.  But it is probably simpler/cheaper just to make more of these boards.
- 1 Digital input can be used to drive a NeoPixel string (be careful of 500mA fuse) (3-pin Yellow)
  - Can mimic the robots LEDs (in case you can't see the robot)
  - Can give you feedback as to some positioning (i.e. target is to the left/right or up/down, and by how much)
  - Can give you feedback as to if you are holding the playing piece.
- 4-pin JST XH rotary encoder for Z-axis (600 ppr*4phase = 2400/rev resolution assumed)
  - We use the rotary encoder to control the angle that the robot is pointing on the field.  Turn the rotary knob by 5degrees, and the robot also turns by 5degrees...
- 4 extra 'virtual' buttons, 1 toggling, 3 as constants
  - 3 virtual constant buttons make it possible to distinguish between multiple controllers easily.
  - Multiple UsbJoy boards can be distinguished from each other from the FRC Robot software (just use those buttons).  This means we don't have to re-arrange the joystick order in driver station.
- Arduino Pro Micro USB-C 5V ATmega32U4 controller board used for ease of soldering.
  - Make sure to get one with the fuse on the TOP side of the board (i.e. no bottom side components).  [amazon.com](https://www.amazon.com/gp/product/B0B6HYLC44)
- SmartButton support for all 12 Digital inputs
  - You can choose what each button's LED looks like for infomational purposes.
  - Flickering the button when initially pressed is simple feedback that the button should be working (commuincation to UsbJoy, LED works, button works, cable connected appropriately).
  - Having the button light up when it should be useable is useful (i.e. the playing piece is being held, you can now press the shoot button).
  - Have a couple of buttons light up to indicate the state of the robot.
- 4 Digital connectors (Green) can have their 5V power switched on/off for use with simple led outputs (SmartButton requires it to always be powered on).
  - This sounded like a great idea initially, but I don't see a use for it now...
- ESD diodes on all 16 3-pin connectors
- Configuration of SmartButton settings done through the USB-UART interface.
  - Some settings are saved in EEPROM (Neopixel config, ...) , some are temporary (LED values).
  - The settings are stored in the UsbJoy's EEPROM, and are configured for that particular port.  The settings aren't saved in the SmartButton itself.
  - You could compile different firmware for each button, and fix each button's LED functionality that way.
- Coming soon: Programming of PIC micro-controller done over USB-UART with button press required.
- Colored JST-XH 3-pin connectors are available on [aliexpress.com](https://www.aliexpress.us/item/3256806937445015.html)
- Colored JST-XH terminal 3-pin connectors are available on [aliexpress.com](https://www.aliexpress.us/item/3256804014172692.html)
- pre-crimped 3-pin cables are available on [aliexpress.com](https://www.aliexpress.us/item/3256807213104605.html)  Just be aware that the pins have to be removed, and swapped so that it is a 1:1 cable instead of a 1:3 cable (or you are going to connect GND to 5V, and destroy your SmartButtons).
- pre-crimped 10-pin cables are available on [amazon.com](https://www.amazon.com/dp/B0B2RCW5JF) And you would have to replace the connectors with a 3-pin version (cheap, and comes from US, so comes quickly).

**Rotary Encoder**
- Uses a simple 4-pin {5V,A,B,GND} [encoder](https://www.amazon.com/dp/B01MZ4V1XP)
- 600pulses/revolution * 4 edges per pulse = 2400 positions/revolution (about 1/6degree resolution)
- Returns 0-2399 as the Z-axis to the Joystick HID interface.

**SmartButton features**
- About $1 for circuit board+cpu+connector.  About $2 per illuminated arcade button.
- Designed to solder directly to 30mm illuminated arcade buttons. [amazon EG-Starts Illuminated 30mm](https://www.amazon.com/EG-STARTS-Illuminated-Buttons-Raspberry/dp/B01N11BDX9) [Aliexpress 24mm&30mm](https://www.aliexpress.us/item/3256804217382377.html) [Coin/1P/2P](https://www.aliexpress.us/item/2251832825468632.html) [AutoRGB](https://www.aliexpress.us/item/3256805580977933.html) [50pack 28mm](https://www.aliexpress.us/item/3256805775007172.html) [Free ship $10](https://www.aliexpress.us/item/2251832849688596.html)
- 24mm illuminated buttons might work, but haven't tested them yet.
- Other buttons could be used.  Just solder the to switch pins to the button, and the LED pins to an LED+Resistor.
- 3-pin interface (5V, signal, GND)
- Some solder jumpers can be filled, and some jumpers cut to turn a smart button into a dumb button (i.e. no 5V present).
- State of the led_option is stored in UsbJoy, and associated with the port.  So the buttons don't have to store their operational state.
- Programming of pic16f15213 can be done with either PicKit5 or UsbJoy.
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
- Uses an off-the-shelf neopixel array (WS2812B 5V leds) [Strip_of_leds](https://www.amazon.com/dp/B09PBHJG6G) [ring_of_leds](https://www.amazon.com/dp/B08GPWVD57)
- Takes over Digital input #11 (the Yellow connector)
- You are going to have to make a cable that has the JST XH connector on the end of it.
- You MUST pay attention to the current draw of the string.  The entire UsbJoy is limited to 500mA, and if you draw much more than that, the fuse will blow, and everything will stop working.  So make sure that the length of string can't draw too much power.
- Possibly add an external power connection to power the neopixel array.
- Color choices:
  - 0x00-0x3f: Black-to-White solid color choices (4 values per R,G,B)
  - 0x80-0xff: Single white LED on (can be set to match the rotary encoder)
  - Rainbow
  - Moving Rainbow

**Files included:**
- UsbJoy_d12_a4_r-rev2/: Kicad layout files, and gerbers
- SmartButton-rev2/: Kicad layout files, and gerbers
- BillOfMaterials/: spreadsheets with all of the parts to purchase and quantities and ordering links, pcb, buttons, cables, 
- mechanics/: openscad & STL files for a couple of mounts for UsbJoy using the SmartButton, rotary dial, 
- software/SmartButton_firmware/:
- software/UsbJoy_firmware/: firmware for the Atmega32U4 microcontroller
- software/UsbJoy_pic_firmware/: firmware 
- software/program_pic/: small application to program the pic16f1778 on UsbJoy
- software/

**Manufacturing suggestions:**
- [oshpark.com](https://oshpark.com/) has wonderful pricing for qty3 of 2and4 layer boards and ships in 10 calendar days for almost everything (and you can pay more to get it earlier, or pay for faster shipping)
  - [UsbJoy_R2](https://oshpark.com/shared_projects/QPZCB1ZT) board order $20/3 boards
  - [SmartButton_R2](https://oshpark.com/shared_projects/OV1uAkH7) board order $1.70/3 boards ($6.80/12 boards)
- A 3d-printer can make the UsbJoy board attach directly to the screw threads of one of the buttons.  A couple of sample mount's source files are here.
- Testing for button functionality can be done in windows with the built-in "USB game controllers"->properties window to see the buttons that get pressed. NEED A PICTURE HERE!
- You can extract a 3d model of the boards from kicad to use in your 3d cad system.  Kicad: File->Export->STEP
- digikey has all of the electrical components necessary (except for the LED and Arduino board).
- 3x UsbJoy parts kit
- 12x SmartButton parts kit
- The arduino pro micro has a 0.5A fuse on it.  The bottom side of the board will NOT be accessible.  If that 0.5A fuse is on the bottom side, and you have over current, then your boards will not be salvageable.  So get a version that has all the components on the top side.
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
- Disconnect the last 2 buttons (Yellow and white next to each other)
- type "q1" on the uart to disable the last 2 button communications
- type "n0" to disable the neopixel functionality
- Run the Microchip IDE (what version minimum?)
- Load the project
- Compile the project for production
- Using PICKIT5: Use one of the current programmers pickit5 is probably preferred, and plug it into the 5-pin holes on the bottom side of the board.  And have the IDE program the device.
- Using USB: remove cables from 3 connectors (A, B, C), hold down the button on the board, and run "program_pic pic.hex", then release the button after programming completion
- type "q0" on the uart to re-enable the last 2 button functionality
- type "n10" to re-enable neopixel 0x10 led functionality (or change the number to your string length) - Neal fixme get rid of this...

**How to program pic16f15213 on SmartButton:**
- Run the Microchip IDE (what version minimum?)
- Load the project
- Compile the project for production
- Using PICKIT5: Make sure the button isn't pressed, Connect the 3-pin cable to the UsbJoy (for 5V power), plug the pickit5 into the 5-pin holes on the SmartButton board.  And have the IDE program the device.
- Using UsbJoy: Connect 3+3+3pin cable ends into UsbJoy (match yellow,white,green connectors), connect 5-pin cable end into SmartButton.  Run "program_pic smartbutton.hex".

**How to initially configure UsbJoy:**
- Connect USB to a PC.
- Open a terminal program (I like [MobaXterm](https://mobaxterm.mobatek.net/)), and connect to the correct com port (use windows device manager, and find the 'leonardo' port listed) (linux: 'dmesg' to find the correct '/dev/ttyUSB#' when you plug into USB).
- Any baud rate is fine, because it is a virtual serial port.
- 'h'+enter to get the help menu
- All settings commands are "letter+hex_number+enter".
- Important EEPROM settings to run:
- 'u#" to set the upper 3-bits for the virtual buttons to uniquely identify this particular board (saved to EEPROM)
- "m#val" to set the SmartLed DEFAULT setting for that port (not for the SmartLed itself) to the desired LED pattern (and toggle functionality) (saved to EEPROM)
- "n#" to set the number of neopixel leds in the string - set to "n0" to indicate that there isn't a neopixel string.
- "v#" to set the power-on setting for the Green ports - probably want them always on "vF" unless they are being used in some interesting way.
- "V#" to set the power-on DEFAULT for the Green ports (saved to EEPROM)

**Future enhancements**
- Add a 5-pin direct programming header for connecting to SmartButton (no need for weird programming cable).
- Move QR code to top side of board.

Neal Palmer,
**FRC team #2984 Vikings Robotics,**
La Jolla, CA, USA
  
