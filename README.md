# UsbJoy
USB Joystick Controller 12Digital/SmartButton, 4Analog, 1Rotary, 1NeoPixel

This is a couple of circuit boards that make up a fancy USB joystick controller for FRC #2984.
It has both a HID joystick interface and a HID uart interface (for configuration) presented to the computer (i.e. it works in Windows and Linux).  No embedded programming experience is necessary to get it to work!  It also looks like just a HID analog joystick to Windows.

To add to this readme+files:
- mechanical drawing of the board (wid, hei, hole locations)
- photo of driver's panel
- update code when I'm done fiddling with stuff
- Add pictures where appropriate (pickit programming, USB programming cable)
- A description of how to get feedback from the robot to the leds.  Does this require a networktables application?  Or use Force-Feedback mechanisms?

![image](https://github.com/user-attachments/assets/ea97f88b-f23c-439b-81eb-e5cb5ec8e4a8)
![image](https://github.com/user-attachments/assets/5b4c13d2-de07-41db-afa2-7e9a51e008a0)
![image](https://github.com/user-attachments/assets/e703f007-66c7-4fff-99f2-dc9885223b1f)
<img width="397" height="770" alt="image" src="https://github.com/user-attachments/assets/d25ffc2e-d5bf-4f97-ad72-5a2e8fb099a8" />


# Features:
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
- 4or5 extra 'virtual' buttons, 1 toggling (optional), 4 as constants
  - 4 virtual constant buttons make it possible to distinguish between multiple controllers easily.
  - Multiple UsbJoy boards can be distinguished from each other from the FRC Robot software (just use those constant buttons).  This means we don't have to re-arrange the joystick order in driver station.
- Arduino Pro Micro USB-C 5V ATmega32U4 controller board used for ease of soldering.
  - Make sure to get one with the fuse on the TOP side of the board (i.e. no bottom side components).  [amazon.com](https://www.amazon.com/gp/product/B0B6HYLC44)
- SmartButton support for all 12 Digital inputs
  - You can choose what each button's LED looks like for informational purposes.
  - Flickering the button when initially pressed is simple feedback that the button should be working (commuincation to UsbJoy, LED works, button works, cable connected appropriately).
  - Having the button light up when it should be useable is useful (i.e. the playing piece is being held, you can now press the shoot button).
  - Have a couple of buttons light up to indicate the state of the robot.
- 4 Digital connectors (Green) can have their 5V power switched on/off for use with simple led outputs (SmartButton requires it to always be powered on).
  - This sounded like a great idea initially, but I don't see a use for it now...
- ESD diodes on all 16 3-pin connectors
- Configuration of SmartButton settings done through the USB-UART interface.
  - Some settings are saved in EEPROM (Neopixel length, LED starting value, joystick invert axis, ...) , some are temporary (LED current values, neopixel pattern).
  - The settings are stored in the UsbJoy's EEPROM, and are configured for that particular port.  The settings aren't saved in the SmartButton itself.
  - You could compile different firmware for each button, and fix each button's LED functionality that way.
- Programming of PIC micro-controller done over USB-UART with MCLRn button press required (don't have anything plugged into J8, J11, J12).
- Programming of SmartButton PIC micro-controller over USB-UART (use a custom cable from J8+J11+J12 to SmartButton 5-pin header).
- Colored JST-XH 3-pin connectors are available on [aliexpress.com](https://www.aliexpress.us/item/3256806937445015.html)
- Colored JST-XH terminal 3-pin connectors are available on [aliexpress.com](https://www.aliexpress.us/item/3256804014172692.html)
- pre-crimped 3-pin cables are available on [aliexpress.com](https://www.aliexpress.us/item/3256807213104605.html)  Just be aware that the pins have to be removed, and swapped so that it is a 1:1 cable instead of a 1:3 cable (or you are going to connect GND to 5V, and destroy your SmartButtons).
- pre-crimped 10-pin cables are available on [amazon.com](https://www.amazon.com/dp/B0B2RCW5JF) And you would have to replace the connectors with a 3-pin version (cheap, and comes from US, so comes quickly).

# UsbJoy Connector description
- ![image](https://github.com/user-attachments/assets/ea97f88b-f23c-439b-81eb-e5cb5ec8e4a8)
- J1-J12 are the button inputs.  Pin 1: 5V (square pad), Pin 2: 'signal' (short to GND during button press), Pin 3: GND.
- J7-J10 (Green connectors) can have their 5V pin switched on/off if you want to use an LED directly connected instead of a SmartButton.  They have a 100ohm series resistor for the 5V output to limit it to about 25mA (plenty for a SmartButton).
- J12 can be setup to drive a neopixel string (a programmable strip of RGB leds).  Be aware that the max current for the string should be about 400mA, any higher and the 500mA USB fuse on the Pro Micro is likely to blow.  So for very long strings, external power should be provided for the array.
- J13-J16 are the analog inputs.  Pin 1: 5V, Pin 2: analog signal, Pin 3: GND.
- J17 is the encoder input.  Pin 1: 5V (square pad), Pin 2,3: A&B encoder inputs, Pin 4: GND.  Any AB encoder can be used.  The firmware is setup to make a reasonable output for a 600 pulse/revolution=2400 change/revolution encoder (i.e. Z-axis has a range of 0-2399).  You can change the 2400 limit to any other number in the code for both the Arduino and PIC to make a different resolution encoder look right.
- J8, J11, J12 are used for PIC programming (both the UsbJoy and SmartButton PIC).
- Button between J17 and J5 is the PIC MCLRn button that connects J8.signal to MCLRn so that it is possible to program the PIC.
- J18 (5-pin holes on back side) is the PIC ICSP header.  Connect the PICKIT5 here.  You must disconnect PICKIT5 for normal operation.

# Rotary Encoder
- Uses a simple 4-pin {5V,A,B,GND} [encoder](https://www.amazon.com/dp/B01MZ4V1XP)
- 600pulses/revolution * 4 edges per pulse = 2400 positions/revolution (about 1/6degree resolution)
- Returns 0-2399 as the Z-axis to the Joystick HID interface.

# SmartButton features
- About $1 for circuit board+cpu+connector.  About $2 per illuminated arcade button.
- Designed to solder directly to 30mm illuminated arcade buttons. [amazon EG-Starts Illuminated 30mm](https://www.amazon.com/EG-STARTS-Illuminated-Buttons-Raspberry/dp/B01N11BDX9) [Aliexpress 30mm](https://www.aliexpress.us/item/3256804217382377.html) [Coin/1P/2P](https://www.aliexpress.us/item/2251832825468632.html) [AutoRGB](https://www.aliexpress.us/item/3256805580977933.html) [50pack 28mm](https://www.aliexpress.us/item/3256805775007172.html) [Free ship $10](https://www.aliexpress.us/item/2251832849688596.html)
- NOTE: 24mm illuminated buttons don't work, the pins are too close together and the button binds when the board is connected (unless you pre-bend the pins).  And they are only 1mm smaller diameter for the nut, so no reason to use them.
- Other buttons could be used.  Just solder the switch pins to the button, and the LED pins to an LED+Resistor.
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

# NeoPixel
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

# Files included:
- UsbJoy_d12_a4_r-rev2/: Kicad layout files, and gerbers
- SmartButton-rev2/: Kicad layout files, and gerbers
- UsbJoy_bill_of_materials.csv: spreadsheets with all of the parts to purchase and quantities and ordering links, pcb, buttons, cables, other stuff...
- mechanics/: openscad & STL files for a couple of mounts for UsbJoy using the SmartButton, rotary dial, 
- software/SmartButton_firmware/:
- software/UsbJoy_firmware/: firmware for the Atmega32U4 microcontroller
- software/UsbJoy_pic_firmware/: firmware 
- software/program_pic/: small application to program the pic16f1778 on UsbJoy, and pic15f15213 on SmartButton

# Manufacturing suggestions:
- [oshpark.com](https://oshpark.com/) has wonderful pricing for qty3 of 2and4 layer boards and ships in 10 calendar days for almost everything (and you can pay more to get it earlier, or pay for faster shipping)
  - [UsbJoy_R2](https://oshpark.com/shared_projects/QPZCB1ZT) board order $20/3 boards
  - [SmartButton_R2](https://oshpark.com/shared_projects/OV1uAkH7) board order $1.70/3 boards ($6.80/12 boards)
- A 3d-printer can make the UsbJoy board attach directly to the screw threads of one of the buttons.  A couple of sample mount's source files are [here](https://makerworld.com/en/models/1602536-usbjoy-controller-mount-30mm-arcade-button).
- An 3d printed [encoder spring](https://makerworld.com/en/models/1602721-rotary-encoder-rotary-friction-spring) makes the encoder much better for not spinning the robot wildly.
- An [acrylic wheel](https://makerworld.com/en/models/1605249-encoder-finger-wheel) with finger holes for the encoder is very useful.
- Testing for button functionality can be done in windows with the built-in "USB game controllers"->properties window to see the buttons that get pressed. NEED A PICTURE HERE!
- You can extract a 3d model of the boards from kicad to use in your 3d cad system.  Kicad: File->Export->STEP
- digikey has all of the electrical components necessary (except for the LED and Arduino board).
- 3x UsbJoy parts kit
- 12x SmartButton parts kit
- The arduino pro micro has a 0.5A fuse on it.  The bottom side of the board will NOT be accessible.  If that 0.5A fuse is on the bottom side, and you have over current, then your boards will not be salvageable.  So get a version that has all the components on the top side.
- The discrete components are 0603 packages for easier soldering.
- Get some [illuminated magnifying glasses](https://www.amazon.com/YOCTOSUN-Magnifying-Rechargeable-1-5X-3-5X-Magnifier/dp/B0D8J3TYL3) for ease of soldering of small components. 
- Solder all of the SMT components before any thru-hole components.  Required for both boards, because some of the thru-hole soldering will make SMT parts unreachable.

# How to program Arduino Pro Micro's Atmega32U4 (the USB device):
- Run the [Arduino GUI](https://www.arduino.cc/en/software/) version 1.8.19 (or newer than May 2025).
- Load the *.ino file
- Set up the board to be "Arduino Leonardo".
- Plug in the board into USB, then select the right Com port in the Arduino GUI.
- "Upload" <img width="28" height="27" alt="image" src="https://github.com/user-attachments/assets/270d445c-2d22-4503-96b8-3c62cd0f49b2" />
 from the Arduino GUI.
- Programming does not clear the EEPROM, so any settings should remain unaffected.

# How to program pic16f1778 on UsbJoy using PICKIT5 programmer:
- Disconnect the last 2 buttons (Yellow and white next to each other)
- type "q1" on the uart to disable the 7,10,11 button communications
- Run the [Microchip IDE](https://www.microchip.com/en-us/tools-resources/develop/mplab-x-ide)
- Load the project
- Compile the project for production
- Use one of the current PIC programmers (pickit5 is probably preferred), and plug it into the 5-pin holes on the bottom side of the board.  And have the IDE program the device.
- type "q0" on the uart to re-enable the last 2 button functionality
# How to program pic16f1778 on UsbJoy using UsbJoy's USB connection:
- Disconnect buttons 7,10,11 (Green, White, Yellow) - they will interfere with the programming.
- Connect UsbJoy to the Windows PC.
- Figure out what com port (use Windows Device Manager) UsbJoy is using.
- Run "program_pic.exe usbjoy [filename].hex COM[#]"
- Press and hold the button next to the rotary connector for the full programming time (about 15 seconds).  If you forget this step, then you will receive a message "Error: couldn't find expected PIC CPU".
- Wait for programming to complete.
- Release the button.
- You can now reconnect the disconnected buttons.

# How to program pic16f15213 on SmartButton using PICKIT5:
- Run the [Microchip IDE](https://www.microchip.com/en-us/tools-resources/develop/mplab-x-ide)
- Load the project
- Compile the project for production
- Make sure the SmartButton isn't pressed, Connect the 3-pin cable to the UsbJoy (for 5V power), plug the pickit5 into the 5-pin holes on the SmartButton board.  And have the IDE program the device.
# How to program pic16f15213 on SmartButton using UsbJoy's USB interface:
- Connect 3+3+3pin cable ends into UsbJoy (match yellow J12,white J11,green J8 connectors), connect 5-pin cable end into SmartButton.
- Run "program_pic sb smartbutton.hex COM[#]".
- Quick Test: While plugged into the 5-pin cable, you should see the led in the button flicker VERY VERY dim.
- Quick Test: While plugged into the 5-pin cable, if you press the button, the LED should light up.
- You will need to create a cable to connect J8,J11,J12 to a 5-pin header.  It is probably easiest and more reliable to make an 3+3+3-pin to 5-pin XH only cable, and then solder the B5B-XH header to a 0.1" header.
 - pin1: MCLRn -> Red/Green/J8.2
 - pin2: 5V -> White/White/J11.1
 - pin3: GND -> Blue/Yellow/J12.3
 - pin4: PGD -> Blue/Yellow/J12.2
 - pin5: PGC -> White/White/J11.2
 - <img width="676" height="497" alt="image" src="https://github.com/user-attachments/assets/cd0be9e9-08c2-4cd5-b7af-086c0df9257c" />
 - <img width="385" height="551" alt="image" src="https://github.com/user-attachments/assets/d54e4672-3bee-4afb-915b-5ecb00b70a6f" />



# How to initially configure UsbJoy:
- Connect USB to a PC.
- Open a terminal program (I like [MobaXterm](https://mobaxterm.mobatek.net/)), and connect to the correct com port (use windows device manager, and find the 'leonardo' port listed) (linux: 'dmesg' to find the correct '/dev/ttyUSB#' when you plug into USB).
- Any baud rate is fine, because it is a virtual serial port.
- 'h'+enter to get the help menu
- All settings commands are "letter+hex_number+enter".
- Important EEPROM settings to run:
  - 'u#" to set the upper 4 bits for the virtual buttons to uniquely identify this particular board (saved to EEPROM)
  - "m#val" to set the SmartLed DEFAULT setting for that port (not for the SmartLed itself) to the desired LED pattern (and toggle functionality) (saved to EEPROM)
  - "n#" to set the number of neopixel leds in the string - set to "n0" to indicate that there isn't a neopixel string.
  - "v#" to set the power-on setting for the Green ports - probably want them always on "vF" unless they are being used in some interesting way.
  - "V#" to set the power-on DEFAULT for the Green ports (saved to EEPROM)

# Future enhancements
- Add a 5-pin direct programming header for connecting to SmartButton (no need for weird programming cable).
- Move QR code to top side of board.

# History of major changes
- 8/8/2025: Added RZ axis to have the full resolution of the 2400/rev encoder when used with FRC DriverStation that only has 200 values per axis.
- 7/16/2025: Added MPLAB project files.
- 7/16/2025: SmartButton pic programming working over USB
- 7/16/2025: Added PDF of schematics.
- 7/15/2025: Uploaded 3d and lasercut objects to makerworld.com

Neal Palmer,
**FRC team #2984 Vikings Robotics,**
La Jolla, CA, USA
  
