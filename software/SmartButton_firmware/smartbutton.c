#include <pic.h>
#include <stdint.h>
//#include <types.h>

#pragma config VDDAR = 1 // 5V voltage range
#pragma config CLKOUTEN = 1 // CLKOUT pin is a GPIO
//#pragma config RSTOSC = 2 // HFINTOSC at 1Mhz
#pragma config RSTOSC = 0 // HFINTOSC at 32Mhz
#pragma config FEXTOSC = 1 // oscillator not enabled

#pragma config STVREN = 1 // restup upon stack issues
#pragma config BORV = 0 // 2.85V brown out voltage
#pragma config BOREN = 3 // always enabled
#pragma config WDTE = 0 // always disabled
#pragma config PWRTS = 3 // always disabled
#pragma config MCLRE = 1 // MCLRn pin functionality

#pragma config LVP = 1 // low voltage programming
#pragma config WRTSAF = 1 // not write protected
#pragma config SAFEN = 0 // reserve StorageAreaFlash starting at address 0x780 for 128 words

#define signal LATA5
#define button RA0

//#define LEDA4
//#ifdef LEDA4
//#define led LATA4
//#else
//#define led LATA2
//#endif

uint32_t command_word=0xffffffff;
int8_t response_mode = 1;
uint8_t led_mode;
uint8_t toggle_button = 2;
uint8_t toggle_mode = 0; // this is for toggling the response signal every single query (for figuring out the min vs max pulse widths per port)
uint8_t toggle_mode_val = 0;

//#define NOP asm("nop")

#define PULSE_MIN 10
#define PULSE_SPLIT 31
#define PULSE_MAX 60

// values:
// bit 0, led off
// bit 0, led command_enable(d) (default config of the LED)
// bit 1, led off
// bit 1, led command_enable(d) (default config of the LED)

//uint8_t command_enabled = 1;
uint8_t toggle;
uint8_t virtual_button;

void __interrupt() isr() {
		// only process rising edge interrupts
	//if (IOCAF5) { // rising edge interrrupt on A5/'signal'
		if (response_mode) {
			signal = 0; // drive low
			for (char i=20;i!=0;i--)
				NOP();
#if 1 // set to 0 to display binary counter value on o-scope
			signal = (__bit)virtual_button; // & button; // variable length pulse based on button (but only sent when the incoming pulse is of an appropriate length)
#else
			// short: 0000_1111 = 0x0f = 15
			// long:  0010_1111 = 0x2f = 47
			if (1) {
				for (int i=7;i>=0;i--) {
					signal = 0;
					signal = 0;
					signal = TMR1L >> i;
					signal = TMR1L >> i;
					signal = TMR1L >> i;
					signal = 1;
				}
			} else {
				if ((command_word & 0xfffff00f) == 0x0000a003)
				for (int i=32;i>=0;i--) {
					signal = 0;
					signal = 0;
					signal = command_word >> i;
					signal = command_word >> i;
					signal = command_word >> i;
					signal = 1;
				}
			}
#endif
			for (char i=40;i!=0;i--)
				NOP();
			signal = 1; // tri-state 'signal'
			toggle = (toggle^1)&1;
		}
		toggle_mode_val ^= 1; // toggle the value every time (hopefully)

		if ((TMR1H>0) || (TMR1L<PULSE_MIN) || (TMR1L>PULSE_MAX) || (TMR1IF==1)) {
			command_word = 0xffffffff;
		} else {
			command_word = (command_word<<1);
			if (TMR1L < PULSE_SPLIT) {
					// received a '1'
				command_word++;
				//command_enabled = 1;
				//command_enabled = (TMR1L > PULSE_1);
			} else {
					// received a '0'
				//command_enabled = (TMR1L > PULSE_3);
			}
		}

		TMR1IF = 0; // clear timer overflow interrupt (especially after response has been sent)
		TMR1H = 0;
		TMR1L = 0;
		T1GCONbits.GGO = 1; // start single acquisition (after response has been sent)
		IOCAF5 = 0; // clear rising edge interrupt
	//}
}


#define LED_OFF 0
#define LED_ON 1
#define LED_PULSE 2 // 0.1s on when button initially pressed
#define LED_ON_PRESSED 3 // on when button is held
#define LED_FLASHING 4 // {0.1s/0.1s} flashing
#define LED_SIN 5 // always on with sin() brightness
#define LED_FLASHING_PRESSED 6 // flashing when button is pressed
#define LED_TOGGLE_LIT_STARTON 7 // each press toggles the state of the button, LED on when simulating pressed.
#define LED_TOGGLE_FLASHING 8 // each press toggles the state of the button, LED flashing when simulating pressed.
#define LED_TOGGLE_LIT_STARTOFF 9 // each press toggles the state of the button, LED on when simulating pressed.

#define PWM_ON 0x80 // 50% brightness
#define PWM_OFF 0x00

uint16_t ms_time=0;

// *******************************************
// *******************************************
// *******************************************
// *******************************************
// void main()
// *******************************************
// *******************************************
// *******************************************
// *******************************************
void main() {
int8_t last_button = 1;
int16_t timeout_command = 0;

// *******************************************
// setup oscillator to run 1Mhz HFINTOSC
// *******************************************
//OSCFRQ = 0; // 1Mhz 

led_mode = LED_PULSE; // default to pulsing the led when the button gets pressed
toggle_button = 2;

// *******************************************
// Setup pins
// *******************************************

// setup ‘button’ pin
//ANSELA0 = 0;
WPUA0 = 1;
TRISA0 = 1;

// setup ‘signal’ pin to the joystick controller board
//ANSELA5 = 0;
WPUA5 = 1;
ODCA5 = 1; // set as open-drain
signal = 1;
TRISA5 = 0; // drive the pin as an output
RA5PPS = 0; // LATA5 is source for the pin output

ANSELA = 0;


// setup ‘led’ pins
LATA4 = 0;
TRISA4 = 0;
// second LED pin
LATA2 = 0;
TRISA2 = 0;

// *******************************************
// drive one LED pin to GND, pullup the other, try both ways.
// The pullup connected to the 5V side of the LED should have a significant voltage droop.
// Whichever way has a lower voltage is the 5V side of the LED.
// *******************************************
ANSELA = 0;
WPUA4 = 1;
WPUA2 = 1;
uint8_t adc_vals[2];
for (int i=0;i<2;i++) {
	if (i==0) {
		ANSELA = 0x04;
		TRISA2 = 1; // let it get pulled high
		TRISA4 = 0; // drive to GND
		ADCON0bits.CHS = 2; // RA2
	} else {
		ANSELA = 0x10;
		TRISA2 = 0; // drive to GND
		TRISA4 = 1; // let it get pulled high
		ADCON0bits.CHS = 4; // RA4
	}
	ADCON1bits.CS = 2; // Fosc/32 (32us)
	ADCON1bits.FM = 0; // result: {ADRESH[7:0], ADRESL[7:6]}
	ADCON1bits.PREF = 0; // 5V is Vref+
	ADCON0bits.ON = 1;
	ADCON0bits.GO = 1;
	while (ADCON0bits.GO==1)
		;
	adc_vals[i] = ADRESH;
}
ADCON0bits.ON = 0;
ANSELA = 0;
WPUA4 = 0;
WPUA2 = 0;
TRISA2 = 0;
TRISA4 = 0;
LATA2 = 0;
LATA4 = 0;

if (adc_vals[0] < adc_vals[1]) {
		// A2 is the 5V side of the LED
		RA2PPS = 3; // use PWM3
} else {
		// A4 is the 5V side of the LED
		RA4PPS = 3; // use PWM3
}
PWM3DCH = 0; // turn off the LED
PWM3DCH = 0x10; // turn on the LED

// *******************************************
// setup timer0 to count 0.001s
// *******************************************
T0CON0bits.OUTPS = 0; // no postscalar
T0CON1bits.CS = 4; // LFINTOSC (32khz)
T0CON1bits.ASYNC = 0; // resynchronized to Fosc
T0CON1bits.CKPS = 0; // 1:1 prescale
T0CON0bits.MD16 = 0; // 8-bit mode
TMR0H = 32-1; // count 32 edges of the clock
T0CON0bits.EN = 1;

// *******************************************
// setup timer1 to time the low pulse on ‘signal’
// *******************************************
T1GCONbits.GTM = 0; // 
T1GCONbits.GPOL = 0; // enable when ‘signal’ is low
T1GCONbits.GSPM = 1; // single acquisition enable
T1GATE = 0; // gate signal from from T1GPPS
T1GPPS = 5; // RA5 = ‘signal’ pin;
TMR1L = 0;
TMR1H = 0;
T1CLK = 2; // HFINTOSC (32Mhz) is clock source 
T1CONbits.CKPS = 0;
T1CONbits.nSYNC = 1; // synchronize the sampling clock with system clock
T1GCONbits.GE = 1;
TMR1IF = 0; // clear overflow interrupt flag
TMR1IE = 0; // disable overflow interrupt
T1CONbits.ON = 1;
T1GCONbits.GGO = 1; // start single acquisition


// *******************************************
// setup timer2/pwm for led output
// *******************************************

T2PR = 0xfe; // longest PWM period == 10'b1111_1111_11
T2CLKCON = 1; // use Fosc/4
T2HLTbits.MODE = 0; // free-run mode
T2HLTbits.PSYNC = 1;
T2CONbits.CKPS = 1; // 1:1 prescale
T2CONbits.ON = 1; // start the timer

//PWM3DCH = 0xf0; // long pulse width
PWM3DCH = PWM_OFF; // no pulse width
PWM3DCL = 0; // only [7:6] are used
PWM3CONbits.EN = 1;

//PWM3DCH = PWM_OFF; // turn off the LED
PWM3DCH = PWM_ON; // turn on the LED

// *******************************************
// enable rising edge interrupt for A5
// *******************************************
IOCAP = 0;
IOCAN = 0;
IOCAP5 = 1; // A5 rising edge interrupt (end of pulse, after timing has completed)
IOCIE = 1;
INTCONbits.GIE = 1;

	// ****************************
	// process button press, LED output, and signal reception
	// ****************************
 while (1) {
	 uint32_t cmd = command_word;
	if ((cmd & 0xfffff00f) == 0x0000a003) {
		response_mode = 1;
		timeout_command = 0;
		toggle_mode = (cmd>>11)&1;

		// ************
		// decode 32-bit words from Arduino Leonardo (usb joystick controller)
		// 16'b0
		// 4'b1010
		// 4'b0000
		// 4'b led_mode
		// 4'b0011
		// ************
		led_mode = (cmd>>4)&0xf;
	}

		// ************
		// count 0.001s increments
		// ************
	if (TMR0IF) {
		TMR0IF=0; // clear interrupt flag
		ms_time++;
		timeout_command++;
		if (timeout_command > 4000) {
			timeout_command = 4000;
			response_mode = 0;
		}
	}

		// ************
		// Send button in non-response mode (this should never be used by our controller)
		// ************
	if (response_mode==0)
		signal = button; // open-drain always drive the button input to the pin.
						 
		// ************
		// Drive the LED
		// ************
#if 0
	PWM3DCH = PWM_ON; // mid brightness LED
#else
	if (toggle_mode) {
		virtual_button = toggle_mode_val;
	} else {
	switch (response_mode /*command_enabled*/ ? led_mode : LED_OFF) {
		case LED_OFF: PWM3DCH = PWM_OFF; virtual_button=button; break;
		case LED_ON: PWM3DCH = PWM_ON; virtual_button=button; break;
		case LED_PULSE:
			if (button==1) {
					// button not pressed
				ms_time = 0;
				TMR0L = 0; // restart the 0.1s timer
				PWM3DCH = PWM_OFF; // turn off LED
				last_button = 1;
			} else {
					// button is pressed
				if (ms_time>100)
					PWM3DCH = PWM_OFF; // turn off LED after 100ms
				if (last_button==1)
					PWM3DCH = PWM_ON; // turn on LED (for 100ms)
				last_button = 0;
			}
			virtual_button = button;
			break;
		case LED_ON_PRESSED:
			PWM3DCH = ((button==0) ? PWM_ON : PWM_OFF);
			virtual_button = button;
			break;
		case LED_FLASHING:
			PWM3DCH = (((ms_time>>7)&1) ? PWM_ON : PWM_OFF);
			virtual_button = button;
			break;
		case LED_FLASHING_PRESSED:
			if (button==1)
				PWM3DCH = PWM_OFF; // led off
			else
				PWM3DCH = (((ms_time>>7)&1) ? PWM_ON : PWM_OFF);
			virtual_button = button;
			break;
		case LED_SIN:
			{
				uint8_t cycle = (uint8_t)(ms_time>>7);
				switch (cycle & 0xf) {
					case 0: cycle = 0x00 | 0x18; break;
					case 1: case 15:cycle = 0x20 | 0x18; break;
					case 2: case 14:cycle = 0x40 | 0x18; break;
					case 3: case 13:cycle = 0x60 | 0x18; break;
					case 4: case 12:cycle = 0x80 | 0x18; break;
					case 5: case 11:cycle = 0xa0 | 0x18; break;
					case 6: case 10:cycle = 0xc0 | 0x18; break;
					case 7: case 9: cycle = 0xd0 | 0x18; break;
					case 8: cycle = 0xff | 0x18; break;
				}
				PWM3DCH = cycle;
			}
			virtual_button = button;
			break;
		case LED_TOGGLE_LIT_STARTON:
		case LED_TOGGLE_LIT_STARTOFF:
		case LED_TOGGLE_FLASHING:
			if (toggle_button>1)
				toggle_button = ((led_mode==LED_TOGGLE_LIT_STARTOFF) ? 1 : 0);

			if (ms_time > 250) {
				if ((last_button==1) && (button == 0)) {
					toggle_button = (toggle_button==0 ? 1 : 0);
					ms_time = 0;
				}
				last_button = button;
				if (ms_time > 250+200)
					ms_time = 250; // make a toggling pattern.
			}
			if (toggle_button==1) 
				PWM3DCH = PWM_OFF;
			else if ((led_mode==LED_TOGGLE_LIT_STARTON) || (led_mode==LED_TOGGLE_LIT_STARTOFF))
				PWM3DCH = PWM_ON/2;
			else
				PWM3DCH = (ms_time < 250+100) ? PWM_ON/2 : PWM_OFF;

			virtual_button = toggle_button;
			break;
		Default:
			PWM3DCH = ((button==0) ? PWM_ON : PWM_OFF);
			virtual_button = button;
	}
	}
#endif
 }
}
