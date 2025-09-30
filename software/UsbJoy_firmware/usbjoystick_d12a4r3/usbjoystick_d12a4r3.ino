// Make sure to edit arduino\avr\variants\leonards\pins_arduino.h and change LED_BUILT_RX and LED_BUILTIN_TX to 13
// And: TXLED0 to PORTD |= 0
// And: TXLED1 to PORTD &= 0xff
// And: RXLED0 to PORTB |= 0
// And: RXLED1 to PORTB &= 0xff
#include <Arduino.h>
#include <EEPROM.h>
#include <FastGPIO.h>
#include "Joystick.h"

#undef LED_BUILTIN_RX
#undef LED_BUILTIN_TX
#define LED_BUILTIN_myRX 17
#define LED_BUILTIN_myTX 30

uint8_t prog_pic = 0;  // stop driving upper 2 button outputs

#define BUTTON_COUNT 12
#define IDBUTTON_COUNT 4
#define TOGGLING_BUTTON_COUNT 1 // can be set to 1 or 0
#define ENCODER_LIMIT (600*4)
#define SAMPLES 25 // how many samples to grab to determine value(duration) of bit - 25 seems to work really well

struct {
	uint8_t len;
	uint8_t id; // 4-bits to set upper 4 id_buttons
	uint8_t neopixel_count;
	uint8_t led_mode[BUTTON_COUNT];
	uint8_t inv[5]; // invert the direction of the 4 analog inputs, and the encoder
	uint8_t pwren;
	uint8_t sb_mid[BUTTON_COUNT];
	uint8_t linearize;
	uint8_t is_dumb_button[BUTTON_COUNT];
} ee;

enum { // list of LED states for smart buttons
	LED_OFF,
	LED_ON,
	LED_ON_100MS, // on for 100ms when pressed
	LED_ON_PRESSED,
	LED_FLASHING,
	LED_SIN,
	LED_FLASHING_PRESSED,
	LED_TOGGLE_LIT_STARTON,
	LED_TOGGLE_FLASHING,
	LED_TOGGLE_LIT_STARTOFF,
	LED_NONE
};

uint8_t buttons[BUTTON_COUNT] = {2,3,4,5,6,7,8,9,10,16,14,15};
uint8_t analogin[4] = {A0,A1,A2,A3};

uint8_t led_state[BUTTON_COUNT];
int8_t smartbutton_test=0;
uint8_t responselen_min[BUTTON_COUNT];
uint8_t responselen_max[BUTTON_COUNT];
uint8_t neopixel_pattern = 1; // light blue
uint8_t debug = 0;
uint8_t last_count = 0;
uint8_t voltage_enables;

#define PGC 15
#define PGD 14
#define MCLR 9

Joystick_ joystick(
	JOYSTICK_DEFAULT_REPORT_ID, JOYSTICK_TYPE_JOYSTICK,
	BUTTON_COUNT + IDBUTTON_COUNT + TOGGLING_BUTTON_COUNT, 0, 
	true, true, true, // X and Y and Z axis
	true, true, true, // Rx and Ry axis, Rz axis (for higher resolution encoder)
	false, false, // no rudder nor throttle
	false, false, false // no accel, brake, steering
	);

uint8_t data[32];
int previousStates[6];
int neopixel_repeat_timer;

void ee_read(void);
void ee_update(void);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(57600); // USB-uart
  Serial1.begin(57600); // RX/TX pins - uart
  //Serial1.begin(9600); // RX/TX pins - uart
  pinMode(LED_BUILTIN_myTX, OUTPUT);
  pinMode(LED_BUILTIN_myRX, OUTPUT);
  joystick.begin(false); // auto=false
  joystick.setXAxisRange(0,1023);
  joystick.setYAxisRange(0,1023);
  joystick.setZAxisRange(0,ENCODER_LIMIT-1); // rotary encoder
  joystick.setRxAxisRange(0,1023);
  joystick.setRyAxisRange(0,1023);
  joystick.setRzAxisRange(0,199); // rotary encoder LSB values only (for FRC controller which has a range of -1.00 to 1.00 = 200 values total)
  DynamicHID().prepareOutput(data, sizeof(data));

  ee_read(); // fetch the parameters from the internal eeprom

  for (uint8_t btn=0;btn<BUTTON_COUNT;btn++) {
	pinMode(buttons[btn],INPUT_PULLUP);
	led_state[btn] = ee.led_mode[btn];
		// set a reasonable value in the eeprom, if it was out of range
	if (led_state[btn]>=LED_NONE) {
		ee.led_mode[btn] = LED_ON_100MS;
		led_state[btn] = LED_ON_100MS;
	}
		// set a reasonable value in the eeprom
	if ((ee.sb_mid[btn]<3) || (ee.sb_mid[btn]>=SAMPLES-2)) {
			// the expected response lengths vary by pin (arduino library speeds)
		ee.sb_mid[btn] = ((((0xe65 >> btn)&1)==0) ? 7 : 11);
	}
	if (ee.is_dumb_button[btn]>1)
		ee.is_dumb_button[btn] = 0; // default all to smart buttons
  }
  for (uint8_t i=0;i<4;i++) {
	pinMode(analogin[i],INPUT_PULLUP);
  }

	// setup the neopixels
  if (ee.neopixel_count==0xff) {
		ee.neopixel_count=0;
  }
  Serial1.write('n');
  Serial1.println(ee.neopixel_count, HEX);
  neopixel_repeat_timer = millis()+10000;

	// Set the Green connector's power enables
  Serial1.write('v');
  Serial1.println(voltage_enables = (ee.pwren&0xf),HEX);

		// set the static ID buttons (this should only have to happen upon startup, but it doesn't cost much here)
  for (uint8_t id_btn=0;id_btn<IDBUTTON_COUNT;id_btn++) {
	joystick.setButton(id_btn+BUTTON_COUNT, (ee.id>>id_btn)&1);
  }

  if (ee.linearize>0xf)
	ee.linearize = 0;

  ee_update();
}

int32_t next_inc=0;
int16_t pic_inc=0;

int16_t encoder = 0; // accumulator for PIC UART data
int8_t uart_value=0; // accumulator for UART data
int32_t uart_value32=0; // accumulator for UART data
char uart_command;

char txled = 0;
char rxled = 0;

char command_index = 0;

void set_pulse(char pinnum,char len)
{
 switch(pinnum) {
		// drive low for a while, then drive high, and immediately tri-state.  This way the pullup resistor won't change the pulse time as much.
	case 2: while((len--)) FastGPIO::Pin<2>::setOutput(0); FastGPIO::Pin<2>::setOutput(1); FastGPIO::Pin<2>::setInputPulledUp(); break;
	case 3: while((len--)) FastGPIO::Pin<3>::setOutput(0); FastGPIO::Pin<3>::setOutput(1); FastGPIO::Pin<3>::setInputPulledUp(); break;
	case 4: while((len--)) FastGPIO::Pin<4>::setOutput(0); FastGPIO::Pin<4>::setOutput(1); FastGPIO::Pin<4>::setInputPulledUp(); break;
	case 5: while((len--)) FastGPIO::Pin<5>::setOutput(0); FastGPIO::Pin<5>::setOutput(1); FastGPIO::Pin<5>::setInputPulledUp(); break;
	case 6: while((len--)) FastGPIO::Pin<6>::setOutput(0); FastGPIO::Pin<6>::setOutput(1); FastGPIO::Pin<6>::setInputPulledUp(); break;
	case 7: while((len--)) FastGPIO::Pin<7>::setOutput(0); FastGPIO::Pin<7>::setOutput(1); FastGPIO::Pin<7>::setInputPulledUp(); break;
	case 8: while((len--)) FastGPIO::Pin<8>::setOutput(0); FastGPIO::Pin<8>::setOutput(1); FastGPIO::Pin<8>::setInputPulledUp(); break;
	case 9: while((len--)) FastGPIO::Pin<9>::setOutput(0); FastGPIO::Pin<9>::setOutput(1); FastGPIO::Pin<9>::setInputPulledUp(); break;
	case 10: while((len--)) FastGPIO::Pin<10>::setOutput(0); FastGPIO::Pin<10>::setOutput(1); FastGPIO::Pin<10>::setInputPulledUp(); break;
	case 14: while((len--)) FastGPIO::Pin<14>::setOutput(0); FastGPIO::Pin<14>::setOutput(1); FastGPIO::Pin<14>::setInputPulledUp(); break;
	case 15: while((len--)) FastGPIO::Pin<15>::setOutput(0); FastGPIO::Pin<15>::setOutput(1); FastGPIO::Pin<15>::setInputPulledUp(); break;
	case 16: while((len--)) FastGPIO::Pin<16>::setOutput(0); FastGPIO::Pin<16>::setOutput(1); FastGPIO::Pin<16>::setInputPulledUp(); break;
 }
}
void set_pin(char pinnum,char value)
{
 switch(pinnum) {
	case 2: FastGPIO::Pin<2>::setOutput(value); break;
	case 3: FastGPIO::Pin<3>::setOutput(value); break;
	case 4: FastGPIO::Pin<4>::setOutput(value); break;
	case 5: FastGPIO::Pin<5>::setOutput(value); break;
	case 6: FastGPIO::Pin<6>::setOutput(value); break;
	case 7: FastGPIO::Pin<7>::setOutput(value); break;
	case 8: FastGPIO::Pin<8>::setOutput(value); break;
	case 9: FastGPIO::Pin<9>::setOutput(value); break;
	case 10: FastGPIO::Pin<10>::setOutput(value); break;
	case 14: FastGPIO::Pin<14>::setOutput(value); break;
	case 15: FastGPIO::Pin<15>::setOutput(value); break;
	case 16: FastGPIO::Pin<16>::setOutput(value); break;
 }
}

void set_pin_inputpullup(char pinnum)
{
 switch(pinnum) {
	case 2: FastGPIO::Pin<2>::setInputPulledUp(); break;
	case 3: FastGPIO::Pin<3>::setInputPulledUp(); break;
	case 4: FastGPIO::Pin<4>::setInputPulledUp(); break;
	case 5: FastGPIO::Pin<5>::setInputPulledUp(); break;
	case 6: FastGPIO::Pin<6>::setInputPulledUp(); break;
	case 7: FastGPIO::Pin<7>::setInputPulledUp(); break;
	case 8: FastGPIO::Pin<8>::setInputPulledUp(); break;
	case 9: FastGPIO::Pin<9>::setInputPulledUp(); break;
	case 10: FastGPIO::Pin<10>::setInputPulledUp(); break;
	case 14: FastGPIO::Pin<14>::setInputPulledUp(); break;
	case 15: FastGPIO::Pin<15>::setInputPulledUp(); break;
	case 16: FastGPIO::Pin<16>::setInputPulledUp(); break;
 }
}

#define MAX_SIMPLE_COUNT 10
uint8_t simple_count[BUTTON_COUNT];

uint32_t time_last_debug_print;


uint16_t lin_adj[][2] = {
	{0,0},
	{14,0},
	{27,85},
	{51,171},
	{87,256},
	{204,341},
	{385,426},
	{551,512},
	{716,597},
	{876,682},
	{972,767},
	{998,853},
	{1015,938},
	{1023, 1023}
};

uint16_t linearize_pot(uint16_t pot,int8_t analognum)
{
 if (((ee.linearize>>analognum)&1)==0)
	return pot;
 for (int i=0;;i++) {
	if ((lin_adj[i][0] <= pot) && (pot <= lin_adj[i+1][0])) {
		int16_t range = lin_adj[i+1][0] - lin_adj[i][0];
		int16_t dist = pot - lin_adj[i][0];
		int16_t change = lin_adj[i+1][1]-lin_adj[i][1];
		return( lin_adj[i][1] + ((float) change) * dist / range );
	}
 }
 return(512);
}


  // *******************************************
  // put your main code here, to run repeatedly:
  // *******************************************
void loop() {
	digitalWrite(LED_BUILTIN_myTX, txled); // mostly override other libraries's driving of the LED
	digitalWrite(LED_BUILTIN_myRX, (millis()>>8)&1);

		// toggle one of the virtual buttons so that you can see it is updating in any joystick gui
	if (TOGGLING_BUTTON_COUNT>0)
		joystick.setButton(BUTTON_COUNT + IDBUTTON_COUNT, ((millis()>>8)&1) ^ 1); // make the upper joystick input toggle

	if ((millis()%1000)==0 /*|| (neopixel_repeat_timer > millis()+10000)*/) {
		Serial1.write('n');
		Serial1.println(ee.neopixel_count, HEX);
		Serial1.write('p');
		Serial1.println(neopixel_pattern, HEX); // should send white
		Serial1.write('v');
		Serial1.println(voltage_enables, HEX); // enable/disable dumb power enables
/*
		Serial.write('n');
		Serial.println(ee.neopixel_count, HEX);
		Serial.write('p');
		Serial.println(neopixel_pattern, HEX); // should send white
*/
		neopixel_repeat_timer = millis()+10000;
	}

/*
	int bytes = DynamicHID().available();
	if (bytes>0) {
		digitalWrite(LED_BUILTIN_myTX, DynamicHID().read());
		for (int i=1;i<bytes;i++)
			DynamicHID().read();
	}
*/


	txled = 1;
	command_index=(command_index-1)&0x1f; // send next bit in the 32-bit command
	for (uint8_t btn=0;btn<BUTTON_COUNT;btn++) {
			//continue;
			//if ((btn==7) || (btn==10) || (btn==11))
				//continue;
				// Don't drive the PGC and PGD pins to the on-board PIC
			if ((prog_pic!=0) && ((btn==7) || (btn>=BUTTON_COUNT-2))) {
				//pinMode(buttons[btn],INPUT_PULLUP);
				continue;
			}

				// skip the neopixel button
			if ((btn==BUTTON_COUNT-1) && (ee.neopixel_count>0)) {
				joystick.setButton(BUTTON_COUNT-1, ((millis()>>6)&1) ^ 1); // make the neopixel joystick input toggle very fast to make it clear that a button won't work
				pinMode(buttons[btn],INPUT_PULLUP);
				continue; // don't drive the LAST button if neopixels are implemented
			}

				// *****************
				// read simple buttons
				// *****************
			if ((0==digitalRead(buttons[btn])) || (ee.is_dumb_button[btn]==1)) {
				joystick.setButton(btn, !digitalRead(buttons[btn]));
				txled ^= (digitalRead(buttons[btn])==0 ? 1 : 0);
				simple_count[btn] = MAX_SIMPLE_COUNT;
				continue;
			}

				// *****************
				// read smart buttons
				// *****************

				// figure out what the duration of the command bit should be:
				// {16'b0, 4'b1010, 1'bTOGGLE, 3'b0, 4'b LED, 4'b0011}
			uint32_t command = 0x0000a003 + ((smartbutton_test>0)?0x800:0) + ((led_state[btn]&0xf)<<4);
			char cmd_len = (1^((command>>(command_index&0x1f))&1)) * 2+1;

				// Drive low for a certain amount of time
			uint8_t high_count=0;
			noInterrupts();
			//pinMode(buttons[btn],OUTPUT);
			//digitalWrite(buttons[btn],0);
			set_pulse(buttons[btn],cmd_len);  // send a programmable pulse width
			//set_pulse(buttons[btn],4);  // send a programmable pulse width

			for (uint8_t j=SAMPLES;j!=0;j--)
				high_count += (digitalRead(buttons[btn])&1);
			interrupts();

			int low_count = SAMPLES-high_count;
			if ((low_count>=1) && (low_count<SAMPLES-1)) {
					// long pulse means smart button was pressed
				joystick.setButton(btn, (low_count >= ee.sb_mid[btn]) ? 1 : 0);
				txled ^= ((low_count>=ee.sb_mid[btn]) ? 1 : 0);
				simple_count[btn] = 0;
			} else {
					// if no smart response for a while, assume that it must be a simple button
				if ((++simple_count[btn]) >= MAX_SIMPLE_COUNT) {
					simple_count[btn] = MAX_SIMPLE_COUNT;
					joystick.setButton(btn, !digitalRead(buttons[btn]));
					txled ^= (digitalRead(buttons[btn])==0 ? 1 : 0);
				}
			}

				// always save the shortest and longest responses
			if (low_count < responselen_min[btn])
				responselen_min[btn] = low_count;
			if (low_count > responselen_max[btn])
				responselen_max[btn] = low_count;

if (debug)
if (0)
			if ((low_count>0) && (last_count!=low_count)) {
				last_count = low_count;
				Serial.print(btn);
				Serial.print("->");
				Serial.println(low_count);
			}

			
	}
	
		// **************************
		// count down the number of iterations for the smartbutton_test (set to 100)
		// take the average of the min and max responselengths as the mid point to detect short vs long.
		// **************************
	smartbutton_test--;
	if (smartbutton_test==0) {
		for (uint8_t btn=0;btn<BUTTON_COUNT;btn++) {
	Serial.print(led_state[btn]);
	Serial.print(", ");
			Serial.print(responselen_min[btn]);
			Serial.print(" to ");
			Serial.print(responselen_max[btn]);
			if ((responselen_min[btn]>0) && (responselen_max[btn]>responselen_min[btn]+4)) {
				ee.sb_mid[btn] = (responselen_min[btn]+responselen_max[btn])/2; // average of min&max
				Serial.print(" -> ");
				Serial.print(ee.sb_mid[btn]);
				ee_update();
			} else {
				Serial.print(" (EE ");
				Serial.print(ee.sb_mid[btn]);
				Serial.print(")");
			}
			Serial.println(' ');
		}
	} else if (smartbutton_test<=0) {
		smartbutton_test = 0;
	}

	joystick.setXAxis(linearize_pot(ee.inv[0] ? 1023-analogRead(analogin[0]) : analogRead(analogin[0]),0));
	joystick.setYAxis(linearize_pot(ee.inv[1] ? 1023-analogRead(analogin[1]) : analogRead(analogin[1]),1));
	joystick.setRxAxis(linearize_pot(ee.inv[2] ? 1023-analogRead(analogin[2]) : analogRead(analogin[2]),2));
	joystick.setRyAxis(linearize_pot(ee.inv[3] ? 1023-analogRead(analogin[3]) : analogRead(analogin[3]),3));

	if (1) {
		if (debug) {
			if ((time_last_debug_print+250 < millis()) || (time_last_debug_print > millis())) {
				time_last_debug_print = millis();
				
				for (int k=0;k<4;k++) {
					if ((ee.linearize&(1<<k))!=0) {
						Serial.print("[");Serial.print(k);Serial.print("] 0x"); 
						Serial.print(analogRead(analogin[k]), HEX);
						Serial.print("-> 0x"); Serial.print(linearize_pot(analogRead(analogin[k]),k), HEX);
						Serial.print(", ");
					}
				}
				Serial.print("\n");
			}
		} else {
			time_last_debug_print  = millis();
		}
	}

	// 6.1mm 0x3ff
	// 5.0mm 0x3e0
	// 4.0mm 0x350
	// 3.0mm 0x204
	// 2.0mm 0x92
	// 1.0mm 0x2e
	// 0.0mm 0xe
	
		// *************************
		// send an incrementing value to the PIC.
		// *************************
	if (1) { // Neal, todo, fixme: replace this with FFB data (or serial commands from the PC)
		if ((next_inc <= millis()) || (next_inc > millis()+1000)) {
			pic_inc ++;
			next_inc = millis()+5; // delay 100ms
			//Serial1.println(pic_inc & 0xfff, HEX);
		}
	}

		// *************************
		// receive the encoder update from the PIC
		// *************************
	while (Serial1.available()>0) {
		char ch = Serial1.read();
		//Serial.write(ch); // echo it to the PC for debug purposes
		if ((ch>='0') && (ch<='9'))
			encoder = (encoder<<4) + ch-'0';
		if ((ch>='A') && (ch<='F'))
			encoder = (encoder<<4) + ch-'A'+10;
		if (ch=='\n') {
			//Serial.println(encoder);
			joystick.setZAxis(ee.inv[4] ? (ENCODER_LIMIT-1 - encoder) : encoder);
			joystick.setRzAxis((ee.inv[4] ? (ENCODER_LIMIT-1 - encoder) : encoder)%200);
			encoder = 0;
		}
		encoder %= ENCODER_LIMIT;
	}

	//Serial1.write('\n');  // send a stream of bytes to the PIC
	//Serial.write('x');
		// *************************
		// Process UART data from the host PC
		// *************************
			// *************************
			// First character is the command.
			// Remainder of the line is a hex number followed by '\n'.
			// *************************
	while (Serial.available()>0) {
		char ch = Serial.read();
		//Serial.write(ch);
		if ((ch>='0') && (ch<='9')) {
			uart_value = (uart_value<<4) + ch-'0';
			uart_value32 = (uart_value32<<4) + ch-'0';
		} else if ((ch>='A') && (ch<='F')) {
			uart_value = (uart_value<<4) + ch-'A'+10;
			uart_value32 = (uart_value32<<4) + ch-'A'+10;
		} else if ((ch>='a') && (ch<='f')) {
			uart_value = (uart_value<<4) + ch-'a'+10;
			uart_value32 = (uart_value32<<4) + ch-'a'+10;
		} else if (ch=='\n') {
			char led;
			switch (uart_command) {
				case 'z':
					Serial.println("UsbJoy");
					break;
				case 'm':
				case 'l': // set the desired smart-led LED setting "lX#" X=which led, #=led pattern
					led = (uart_value>>4)&0xf;
					if ((led < BUTTON_COUNT) && ((uart_value&0xf)<LED_NONE))
						led_state[led] = (uart_value&0xf);
					if (uart_command=='m') {
						ee.led_mode[led] = led_state[led];
						ee_update();
						//Serial.println("Saved");
					}
					break;
				case 'n': // Set the neo-pixel length to the PIC.
					Serial1.write('n');
					Serial1.println(uart_value, HEX);
					ee.neopixel_count=uart_value;
					ee_update();
					break;
				case 'p': // Set the neo-pixel color to the PIC.
					Serial1.write('p');
					Serial1.println(neopixel_pattern=uart_value, HEX);
					break;
				case 'i': // Invert joystick axis
					if (uart_value>4)
						break;
					ee.inv[uart_value] = (1^ee.inv[uart_value])&1;
					ee_update(); // save the new value to the EEPROM
					break;
				case 'u': // set iD of this USB device using buttons 12-15
					ee.id = (uart_value&0xf);
					ee_update(); // save the new value to the EEPROM
					for (uint8_t id_btn=0;id_btn<IDBUTTON_COUNT;id_btn++) {
						joystick.setButton(id_btn+BUTTON_COUNT, (ee.id>>id_btn)&1);
					}
					break;
				case 'V':
					ee.pwren = (uart_value & 0xf);
					ee_update();
					// no break, continue on to 'v'
				case 'v':
					voltage_enables = (uart_value&0xf);
					Serial1.print('v');
					Serial1.println(voltage_enables,HEX); // send the voltage enables to the PIC
					break;
				case 'g':
					debug = !debug;
					for (uint8_t btn=0;btn<BUTTON_COUNT;btn++) {
						Serial.print(btn);
						Serial.print(":");
						Serial.println(led_state[btn]);
					}
					break;
				case 's':
					smartbutton_test = 100;
					for (uint8_t i=0;i<BUTTON_COUNT;i++) {
						responselen_min[i] = 0xff;
						responselen_max[i] = 0;
					}
					break;
				case 'q':
					prog_pic = uart_value;
					switch (uart_value) {
						case 0:
							pinMode(MCLR,INPUT_PULLUP);
							pinMode(PGC,INPUT_PULLUP);
							pinMode(PGD,INPUT_PULLUP);
							break;
						case 1:
							pinMode(PGC,OUTPUT);
							digitalWrite(PGC, 0);
							pinMode(MCLR,OUTPUT);
							digitalWrite(MCLR, 0);
							break;
					}
					break;
				case 'r': { // LSB first
						// "r#", #=hex number of bits to read back (1 to 16)
						// response: "r#" hex value read
					uint16_t val=0;
					Serial.print("r");
					pinMode(PGC,OUTPUT);
					digitalWrite(PGC, 0);
					pinMode(PGD,INPUT_PULLUP);
						// shift in the bits LSB first
					for (int index=0;index<uart_value;index++) {
						digitalWrite(PGC, 1);
						digitalWrite(PGC, 0);
						val |= ((digitalRead(PGD)&1) << index);
					}
					Serial.println(val,HEX);
					break;
				}
				case 'R': { // MSB first
						// "r#", #=hex number of bits to read back (1 to 16)
						// response: "r#" hex value read
					uint16_t val=0;
					Serial.print("r");
					pinMode(PGC,OUTPUT);
					digitalWrite(PGC, 0);
					pinMode(PGD,INPUT_PULLUP);
						// shift in the bits LSB first
					for (int index=uart_value-1;index>=0;index--) {
						digitalWrite(PGC, 1);
						digitalWrite(PGC, 0);
						val |= ((digitalRead(PGD)&1) << index);
					}
					Serial.println(val,HEX);
					break;
				}
				case 'w': // send LSB first
						// "w#val16", #=hex number of bits to transfer (1 to 16), val16=16-bit hex number to write
					pinMode(PGC,OUTPUT);
					pinMode(PGD,OUTPUT);
					digitalWrite(PGC, 0);
						// shift out the bits LSB first
					for (int index=0;index<(uart_value32>>16);index++) {
						digitalWrite(PGD,(uart_value32>>index)&1);
						digitalWrite(PGC, 1);
						digitalWrite(PGC, 0);
					}
					pinMode(PGD,INPUT_PULLUP);
					break;
				case 'W': // send MSB first
						// "w#val16", #=hex number of bits to transfer (1 to 16), val16=16-bit hex number to write
					pinMode(PGC,OUTPUT);
					pinMode(PGD,OUTPUT);
					digitalWrite(PGC, 0);
						// shift out the bits MSB first
					for (int index=(uart_value32>>16)-1;index>=0;index--) {
						digitalWrite(PGD,(uart_value32>>index)&1);
						digitalWrite(PGC, 1);
						digitalWrite(PGC, 0);
					}
					pinMode(PGD,INPUT_PULLUP);
					break;
				case '-':
					if (uart_value32<12)
						ee.is_dumb_button[uart_value32] = 1;
					ee_update(); // save the new value to the EEPROM
					break;
				case '+':
					if (uart_value32<12)
						ee.is_dumb_button[uart_value32] = 0;
					ee_update(); // save the new value to the EEPROM
					break;
				case '/':
					ee.linearize = (uart_value32&0xf);
					ee_update(); // save the new value to the EEPROM
					//Serial.println("updated linearize\n");
					break;
				case 'h':
				default:
					Serial.println("https://github.com/nealpalmer/UsbJoy");
					Serial.println("commands:");
					Serial.println("'lX#'=set smart-led X (volatile) #={off,on,100ms,on pressed,flashing,sin(),flashing pressed,toggle_ON,toggle_FLASH,toggle_OFF}");
					Serial.println("'mX#'=set smart-led X to EEPROM #={0=off,1=on,2=100ms,3=on pressed,4=flashing,5=sin(),6=flashing pressed,7=toggle_ON,8=toggle_FLASH,9=toggle_OFF}");
					Serial.println("'v#'=Set Volage Enables (green conn) voltatile");
					Serial.println("'V#'=Set Volage Enables (green conn) EEPROM");
					Serial.println("'u#'=Set Button 12-15 constant Unique-ID pattern EEPROM");
					Serial.println("'iQ'=invert joystick 'Q' axis {X,Y,Rx,Ry,Rotary} EEPROM");
					Serial.println("'n##'=Set num neopixels (0==standard button) EEPROM");
					Serial.println("'p##'=Set neopixels pattern");
					Serial.println("'s'=detect and save smartbutton response len EEPROM (use if a smartbutton has button issues)");
					Serial.println("'/#'=linearize_pots - #=hex enables");
					Serial.println("'-#'=is dumb button - #=button num EEPROM");
					Serial.println("'+#'=is smart button - #=button num EEPROM");
					Serial.println("'g'=Debug Data");
					//Serial.println("'q'=PIC program assert MCLRn (J8)");
					//Serial.println("'w'=PIC program write Data");
					//Serial.println("'r'=PIC program read Data");
			}
			uart_value = 0;
			uart_value32 = 0;
			uart_command = 0;
		} else {
			uart_command = ch;
		}
	}


	joystick.sendState();
	digitalWrite(LED_BUILTIN_myTX, txled);
}

void ee_read(void)
{
 for (int i=0;i<sizeof(ee);i++)
	((uint8_t*)(&ee))[i] = EEPROM.read(i);
}

void ee_update(void)
{
 ee.len = sizeof(ee);
 for (int i=0;i<sizeof(ee);i++) {
		// only write when the value has changed
	if ( ((uint8_t*)(&ee))[i] != EEPROM.read(i))
		EEPROM.write(i, ((uint8_t*)(&ee))[i]);
 }
}
