# UsbJoy
USB Joystick Controller 12Digital/SmartButton, 4Analog, 1Rotary, 1NeoPixel

This is a couple of circuit boards that make up a fancy USB joystick controller for FRC #2984.
It has both a HID joystick interface and a HID uart interface presented to the computer.

![image](https://github.com/user-attachments/assets/ea97f88b-f23c-439b-81eb-e5cb5ec8e4a8)


Features:
- 12 Digital inputs for buttons (3-pin White/Yellow/Green JST XH: 5V, signal with 10k pullup, GND)
- 4 Analog inputs for joysticks or linear/rotary potentiometers (10k ohms) (3-pin Blue JST XH: 5V, signal with 100k+100k pullup+pulldown, GND).
- 1 Digital input can be used to drive a NeoPixel string (be careful of 500mA fuse) (3-pin Yellow)
- 4-pin JST XH rotary encoder for Z-axis (600 ppr*4phase = 2400/rev resolution assumed)
- Arduino Pro Micro USB-C 5V ATmega32U4 controller board used for ease of soldering
- SmartButton for all 12 Digital inputs
- 4 Digital connectors (Green) can have their 5V power switched on/off for use with simple led outputs (SmartButton requires it to always be powered on).
- ESD diodes on all 16 3-pin connectors
- Configuration of SmartButton configuration done through the USB-UART interface.  Some settings are saved in EEPROM (Neopixel config, ...) , some are temporary (LED values).
- Coming soon: Programming of PIC micro-controller done over USB-UART with button press required.

SmartButton
- Uses 30mm illuminated arcade buttons.
- 3-pin interface (5V, signal, GND)
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
