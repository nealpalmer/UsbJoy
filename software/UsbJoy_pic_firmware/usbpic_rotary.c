#include <pic.h>
#include <xc.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>

#define EMBED_UART_INTERRUPT
#include "uart_fifos.c"

//#define USE_WDT
#define WATCHDOG_1mS 0
#define WATCHDOG_2S 11
#define WATCHDOG_4S 12
#define WATCHDOG_8S 13
#define WATCHDOG_16S 14
#define WATCHDOG_32S 15
#define WATCHDOG_64S 16
#define WATCHDOG_128S 17
#define WATCHDOG_256S 18

#pragma config FCMEN = 0
#pragma config IESO = 0
#pragma config CLKOUTEN = 1 // disable the clock output
#pragma config BOREN = 3
#pragma config CP = 1 // no code protection
#pragma config MCLRE = 1 // keep MCLRN function
#pragma config PWRTE = 1 // disabled
#ifdef USE_WDT
	#pragma config WDTE = 3 // always enabled watch dog timer
#else
	#pragma config WDTE = 1 // software enabled watch dog timer
#endif
#pragma config FOSC = 4 // internal HF oscillator

#pragma config LVP = 1 // low voltage programming enabled
//#pragma config DEBUG = 1 // disabled
#pragma config LPBOR = 0 // use low power
#pragma config BORV = 0 // high trip point
#pragma config STVREN = 0 // no reset on stack overflow
#pragma config PLLEN = 0 // pll disabled
#pragma config ZCD = 1 // disabled
#pragma config PPS1WAY = 0 // software controlled
#pragma config WRT = 3 // flash write protection disabled

#define PIN_LED LATC5 // output only

#define PIN_OUT0 LATB0 // output only
#define PIN_OUT1 LATB1 // output only
#define PIN_OUT2 LATA0 // output only
#define PIN_OUT3 LATA1 // output only

#define  PWR_DEFAULT 0 // default pin power is OFF

#if 1
	#define PIN_NEO LATB6 // PGC pin
	#define TRIS_NEO TRISB6 // PGC pin
#else
	#define PIN_NEO LATB5 // unconnected pin - necessary for debugging
	#define TRIS_NEO TRISB5 // unconnected pin - necessary for debugging
#endif

#define PPS_PORTA (0<<3)
#define PPS_PORTB (1<<3)
#define PPS_PORTC (2<<3)
#define PPS_PORTD (3<<3)

#define MHZ 16

#define PATTERN_BLACK 0x00 // 'b00rrggbb
#define PATTERN_WHITE 0x3f // 'b00rrggbb
#define PATTERN_ROTARY 0x40 // match the rotary encoder
#define PATTERN_FAST_ROTARY 0x41 // match the rotary encoder
#define PATTERN_SINGLE 0x80 // plus another 59 valid values after this one

//IRCF = 1111
//SCS = 10; // intosc

bool is_charger(void);
//uint8_t hex_to_int(char a,char b);
void handle_uart(void);
void reboot_now(void);
void kick_watchdog(void);
void send_neopixel_byte(uint8_t val);

uint8_t neopixel_count=0;
uint8_t neopixel_pattern=PATTERN_BLACK+0x08;

// *******************************************
// A/B decoder Interrupt routine
// *******************************************

#define NUM_POS (600*4)
#define JOY_MIN 236.0
#define JOY_RANGE 552.0
#define JOY_MAX (JOY_MIN+JOY_RANGE)

volatile int16_t position=0;
char prev_ab=0;
char changed=0;
int8_t last_direction=0;

// table[prev_ab][new_ab]
int8_t table[4][4] = {
	{0,-1,1,2}, // prev_ab==0
	{1,0,2,-1}, // prev_ab==1
	{-1,2,0,1}, // prev_ab==2
	{2,1,-1,0} // prev_ab==3
};


__interrupt() void ISR(void)
{
 char new_ab;
 int8_t tbl;

 if (IOCAF) {
 	IOCAF = 0; // clear interrupt source

 		// read A/B input pins (pins RA2 and RA3)
 	new_ab = (PORTA>>2)&3;
 	tbl = table[prev_ab][new_ab];
 	if (2 == tbl) {
		position += last_direction+last_direction;
 	} else {
 		position += tbl;
 		if (tbl!=0)
			last_direction=tbl;
 	}
 	prev_ab = new_ab;
 		// make sure position is in the range of {0,NUM_POS-1}
 	if (position<0)
	 	position+=NUM_POS;
 	if (position>=NUM_POS)
	 	position-=NUM_POS;
 	changed = 1;
 }

 	// embed the uart interrupt code here
 UART_INTERRUPT
}

void enable_AB_decoder(void)
{
 position = 0;
 changed = 0;
 	// enable pullup on A/B pins
 WPUA2 = 1;
 WPUA3 = 1;
 TRISA2 = 1;
 TRISA3 = 1;
 ANSA2 = 0;
 ANSA3 = 0;
 	// Enable ISR
 IOCAN = 0x0c; // enable interrupt on Negative change
 IOCAP = 0x0c; // enable interrupt on Positive change
 INTCONbits.IOCIE = 1; // enable interrupt on change
 INTCONbits.PEIE = 1; // enable all interrupts
 INTCONbits.GIE = 1; // enable all interrupts
}

/*
void sleep_ms(int16_t i)
{
 uint16_t j;
 while ((i--) > 0)
	 for (j=0;j<612;j++)
		 ;
}
*/

/*
uint8_t hex_to_int(char a,char b)
{
 uint8_t retval;
 if ((a>='0') && (a<='9')) {
	retval = (a-'0')*16;
 } else if ((a>='A') && (a<='F')) {
	retval = (a-'A'+10)*16;
 } else if ((a>='a') && (a<='f')) {
	retval = (a-'a'+10)*16;
 } else 
	retval = 0;

 if ((b>='0') && (b<='9')) {
	retval |= (b-'0');
 } else if ((b>='A') && (b<='F')) {
	retval |= (b-'A'+10);
 } else if ((b>='a') && (b<='f')) {
	retval |= (b-'a'+10);
 } 
 return(retval);
}
*/

int main(void)
{
	// ***************
	// setup cpu clock to 32Mhz
	// ***************
 //PRIMUX = 1; // clock from HF
 //PLLMUX = 0; // disable 4x pll
 OSCCONbits.IRCF = 0xe; // 8Mhz
 OSCCONbits.SPLLEN = 1; // enable 4x pll
 OSCCONbits.SCS = 0; // use pll

 PPSLOCK = 0;

	// ***************
	// Setup Watchdog Timer
	// ***************
#ifdef USE_WDT
 WDTCONbits.WDTPS = WATCHDOG_256S; // set initial 256s timeout
 WDTCONbits.SWDTEN = 1;
#endif

	// ***************
	// setup IO pins
	// ***************
	
OPTION_REGbits.nWPUEN = 0; // enable global Weak Pull Ups by individual WPUx# registers
	
 	// ICSPCLK/DAT RB6/RB7
	// These are also shared by buttons, so don't drive them.
LATB6 = 0;
LATB7 = 0;
TRIS_NEO = 1;
TRISB7 = 1;
WPUB6 = 1;
WPUB7 = 1;
	
	// LED
ANSC5 = 0;
TRISC5 = 0; // always drive
PIN_LED = 1; // turn on the LED
RC5PPS = 0;
WPUC5 = 0;

	// ENC_A,ENC_B as inputs from the AB encoder
ANSA2 = 0;
TRISA2 = 1; // always an input
RA2PPS = 0;
WPUA2 = 1; // pullup on board also

ANSA3 = 0;
TRISA3 = 1; // always an input
RA3PPS = 0;
WPUA3 = 1; // pullup on board also

	// OUT[0:3] to enable power to LED/BUTTON connectors
ANSB0 = 0;
TRISB0 = 0; // always drive
PIN_OUT0 = PWR_DEFAULT; // turn on/off power
RB0PPS = 0;
WPUB0 = 0;

ANSB1 = 0;
TRISB1 = 0; // always drive
PIN_OUT1 = PWR_DEFAULT; // turn on/off power
RB1PPS = 0;
WPUB1 = 0;

ANSA0 = 0;
TRISA0 = 0; // always drive
PIN_OUT2 = PWR_DEFAULT; // turn on/off power
RA0PPS = 0;
WPUA0 = 0;

ANSA1 = 0;
TRISA1 = 0; // always drive
PIN_OUT3 = PWR_DEFAULT; // turn on/off power
RA1PPS = 0;
WPUA1 = 0;

//uart_setup(B9600, MHZ32);
uart_setup(B57600, MHZ32);

	// RX input Pin (pin RC3)
ANSC3 = 0;
TRISC3 = 1;
RXPPS = 0x10 | 0x3; // source is pin RC3
WPUC3 = 0;

	// TX output (pin RC1)
//ANSC1 = 0;
LATC1 = 0; // drive 0, because it won't clobber the UART
//TRISC1 = 0;
RC1PPS = 0x24; // TX/CK
WPUC1 = 0;

TRISB7 = 1; // ICSPDAT (can have a jumper to GND)
WPUB7 = 1;
TRIS_NEO = 1; // ICSPCLK/NeoPixel (can have a jumper to GND)
WPUB6 = 1;

enable_AB_decoder();
uint8_t rotary_led;

uint8_t neopixel_index=0xff;
while (1) {
	PIN_LED = (__bit) (position&1);

		// send the updated position over the uart
	if (changed && is_uart_write_empty()) {
		changed = 0;

			// copy the position variable with interrupts disabled
 		INTCONbits.GIE = 0; // dis-able all interrupts
		int16_t pos = position;
 		INTCONbits.GIE = 1; // enable all interrupts

		write_uart_hex((unsigned long)position,3);
		write_uart('\n');
		//sleep_ms(100);
	}

	handle_uart();

	if (neopixel_count != 0) {
			neopixel_index++;
			if (neopixel_index>=neopixel_count) {
				// send a stop (wait a while)
				for (int16_t i=0;i<1000;i++)
					NOP();
				neopixel_index = 0;
				if (neopixel_pattern==PATTERN_ROTARY)
					rotary_led = neopixel_count-1-(position/(int)(NUM_POS/neopixel_count));
				if (neopixel_pattern==PATTERN_FAST_ROTARY)
					rotary_led = neopixel_count-1-(position%neopixel_count);
				//rotary_led = 3;
				//neopixel_pattern = (neopixel_pattern+1)%PATTERN_WHITE;
					// set the first pixel to black
				if (0) {
					send_neopixel_byte(0x00); // G
					send_neopixel_byte(0x00); // R
					send_neopixel_byte(0x00); // B
				}
			}

			TRIS_NEO = 0; // drive the pin
			//neopixel_count = 12;
			//neopixel_pattern = PATTERN_BLACK+4;
			for (neopixel_index=0;neopixel_index<neopixel_count;neopixel_index++) {
				uint8_t r = neopixel_pattern&0x30;
				uint8_t g = (neopixel_pattern<<2)&0x30;
				uint8_t b = (neopixel_pattern<<4)&0x30;
 //INTCONbits.GIE = 0; // disable all interrupts
				if (neopixel_pattern<=PATTERN_WHITE) {
					send_neopixel_byte(g); // G
					send_neopixel_byte(r); // R
					send_neopixel_byte(b); // B
				} else if (neopixel_pattern>=PATTERN_SINGLE) {
					if ((neopixel_pattern-PATTERN_SINGLE)==neopixel_index) {
						send_neopixel_byte(0x3f); // G
						send_neopixel_byte(0x3f); // R
						send_neopixel_byte(0x3f); // B
				    } else {
						send_neopixel_byte(0); // G
						send_neopixel_byte(0); // R
						send_neopixel_byte(0); // B
					}
				} else if ((neopixel_pattern==PATTERN_ROTARY) || (neopixel_pattern==PATTERN_FAST_ROTARY)) {
					if (rotary_led==neopixel_index) {
						send_neopixel_byte(0x3f); // G
						send_neopixel_byte(0x3f); // R
						send_neopixel_byte(0x3f); // B
				    } else {
						send_neopixel_byte(0); // G
						send_neopixel_byte(0); // R
						send_neopixel_byte(0); // B
					}
				} else {
					switch (neopixel_pattern) {
						case PATTERN_BLACK:
							send_neopixel_byte(0x00); // G
							send_neopixel_byte(0x00); // R
							send_neopixel_byte(0x00); // B
						break;
						case PATTERN_WHITE:
							send_neopixel_byte(0x40); // G
							send_neopixel_byte(0x40); // R
							send_neopixel_byte(0x40); // B
						break;
					}
				}
 //INTCONbits.GIE = 1; // enable all interrupts
 
					// get out of this loop if there are any bytes being received
 				if (!is_uart_read_empty()) {
					neopixel_index = neopixel_count;
				}
			}
	} else {
			TRIS_NEO = 1; // float the pin
	}
 }
}

void send_neopixel_byte(uint8_t val)
{
 INTCONbits.GIE = 0; // disable all interrupts
 for (int8_t i=8;i!=0;i--) {
	if ((val&0x80)==0) {
		PIN_NEO = 1;
		//NOP();
		PIN_NEO = 0;
	} else {
		PIN_NEO = 1;
		NOP();
		NOP();
		NOP();
		NOP();
		//NOP();
		//NOP();
		PIN_NEO = 0;
	}
	val = (uint8_t) (val<<1);
 }
 INTCONbits.GIE = 1; // enable all interrupts
}

		// *************************************
		// receive uart commands from Arduino:
		// "n#" - set number of neopixels (0==disable the output pin)
		// "p#" - set neopixel color pattern
		// *************************************
char uart_command;
uint8_t uart_value=0;
void handle_uart(void) {
	int16_t ch = read_uart();
	if (ch!=-1) {
		//PIN_LED = RC5^1;
		if ((ch>='0') && (ch<='9'))
			uart_value = (uint8_t) ((uart_value<<4) + (uint8_t)ch-(uint8_t)'0');
		else if ((ch>='A') && (ch<='F'))
			uart_value = (uint8_t) ((uart_value<<4) + (uint8_t)ch-(uint8_t)'A'+(uint8_t)10);
		else if ((ch>='a') && (ch<='f'))
			uart_value = (uint8_t) ((uart_value<<4) + (uint8_t)ch-(uint8_t)'a'+(uint8_t)10);
		else if (ch=='\r') {
		} else if (ch=='\n') {
			char led;
			switch (uart_command) {
				case 'n':
					neopixel_count = uart_value;
					//neopixel_pattern = PATTERN_BLACK+0x10;
					if (neopixel_count == 0) {
						TRIS_NEO = 1;
					} else {
						TRIS_NEO = 0;
						//PIN_NEO = 0;
					}
				break;
				case 'p':
					neopixel_pattern = uart_value;
				break;
				case 'v': // voltage enables
					PIN_OUT0 = (uart_value>>0)&1;
					PIN_OUT1 = (uart_value>>1)&1;
					PIN_OUT2 = (uart_value>>2)&1;
					PIN_OUT3 = (uart_value>>3)&1;
				break;
			}
			uart_command = 0;
			uart_value = 0;
		} else {
			uart_command = (uint8_t)ch;
		}
	}
}


/*
void reboot_now(void)
{
 INTCONbits.GIE = 0; // disable all interrupts
 WDTCONbits.WDTPS = WATCHDOG_1mS; // set 1ms timeout
 WDTCONbits.SWDTEN = 1;
 while (1) ; // wait till the PIC reboots (should be ~1ms)
}

void kick_watchdog(void)
{
#ifdef USE_WDT
 //WDTCONbits.WDTPS = WATCHDOG_64S; // set 64s timeout
 WDTCONbits.WDTPS = WATCHDOG_8S; // set 8s timeout
 WDTCONbits.SWDTEN = 1;
 CLRWDT();
#endif
}
*/
