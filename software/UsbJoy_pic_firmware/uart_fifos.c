
#include <pic.h>
#include <stdint.h>
#include <stdbool.h>

#define B9600 96
#define B57600 576
#define B38400 384

#ifdef RX_SIZE // can be overridden by the #include-ing file
#else
	#define RX_SIZE 16
#endif
unsigned char rx_buff[RX_SIZE];
uint8_t rx_rptr = 0;
uint8_t rx_wptr = 0;

#ifdef TX_SIZE // can be overridden by the #include-ing file
	#if TX_SIZE==0
		#define TX_IMMEDIATE
	#endif
#else
	#define TX_SIZE 8
#endif

#ifdef TX_IMMEDIATE
#else
unsigned char tx_buff[TX_SIZE];
uint8_t tx_rptr = 0;
uint8_t tx_wptr = 0;
#endif


void write_uart(char val);
int16_t read_uart(void);
void uart_setup(int baud_rate, bool mhz32);

bool is_uart_write_empty(void)
{
 return (tx_wptr==tx_rptr);
}
bool is_uart_read_empty(void)
{
 return (tx_wptr==tx_rptr);
}

void write_uart(char val)
{
#ifdef TX_IMMEDIATE
	// wait while the previous character is still in the TX1REG register.
 while (TXIF==0)
	 ;

 	// send the next byte, and return immediately
 TX1REG = val;
 hasn't been tested yet
 return;
#else
 	// if the TX buffer is full, then wait until it isn't full
 while (((tx_wptr-tx_rptr)&(TX_SIZE*2-1)) >= TX_SIZE)
	 TXIE=1; // force the interrupt to be on, just incase something was missed

 	// put the byte in the TX buffer
 tx_buff[tx_wptr & (TX_SIZE-1)] = val;
 tx_wptr++;

 	// Enable the TX interrupt
	// - when hardware output is empty, the ISR will push the byte, and clear TXIE
 TXIE = 1;
#endif
}

int16_t read_uart(void)
{
 if (OERR==1) {
	 // clear the overrun error
	CREN = 0;
 	CREN = 1;
 }
 // returns (-1) if no character waiting
 if (rx_rptr==rx_wptr) {
	return(-1);
 }
 	// return the character read.
 unsigned char retval = rx_buff[rx_rptr];
 rx_rptr = (rx_rptr+1)%RX_SIZE;
 return retval;
}


#ifdef EMBED_UART_INTERRUPT
#define UART_INTERRUPT \
 	if (RCIF==1) { \
		rx_buff[rx_wptr] = RC1REG; /* read the character, and should disable the interrupt */ \
		rx_wptr = (rx_wptr+1)%RX_SIZE; \
 	} \
	if (TXIF==1) { \
		if (tx_wptr!=tx_rptr) { \
			TX1REG = tx_buff[tx_rptr&(TX_SIZE-1)]; \
			tx_rptr++; \
		} else { \
			TXIE = 0; \
		} \
	}
#else
__interrupt() void ISR(void)
{
 if (RCIF==1) {
	rx_buff[rx_wptr] = RC1REG; /* read the character, and should disable the interrupt */
	rx_wptr = (rx_wptr+1)%RX_SIZE;
 }
 if (TXIF==1) {
	if (tx_wptr!=tx_rptr) {
		TX1REG = tx_buff[tx_rptr&(TX_SIZE-1)];
		tx_rptr++;
	} else {
		TXIE = 0;
	}
 }
}
#endif

void write_uart_hex(uint32_t val,char nibblecount)
{
 for (int i=nibblecount-1;i>=0;i--) {
	 char h = (val >> (4*i)) & 0xf;
	 if (h<=9)
		 write_uart('0'+h);
	 else
		 write_uart('A'+h-10);
 }
 return;
}

#define MHZ32 true
#define MHZ28p636 false

void uart_setup(int baud_rate, bool mhz32)
{
	// ***************
	// setup UART for 38400 Baud
	//
	// 38400 = 28.636 / 4 / (SPBRG+1)
	// SPBRG = 185.4 = 186 (0.3% error)
	// ***************
 if (baud_rate==B9600) {
	if (mhz32) {
 		SP1BRGH = 3;
 		SP1BRGL = 0x3c; // from charger.c
	} else {
			// 28.636Mhz
 		SP1BRGH = 2;
 		SP1BRGL = 0xe4; // from power_sense.c
	}
 } else if (baud_rate==B57600) {
	 	// requires 32Mhz
	SP1BRGH = 0;
 	SP1BRGL = 0x8a; // from charger.c
 } else {
	if (mhz32) {
 		SP1BRGH = 0;
 		SP1BRGL = 207; // from charger.c
	} else {
			// 28.636Mhz
 		SP1BRGH = 0;
 		SP1BRGL = 185; // from power_sense.c
	}
 }
 TX1STAbits.BRGH = 1;
 BAUD1CONbits.BRG16 = 1;

 TX1STAbits.TX9 = 0; // 8-bit mode
 BAUD1CONbits.SCKP = 0; // non-inverted transmit
 TX1STAbits.TXEN = 1;
 RC1STAbits.CREN = 1;
 TX1STAbits.SYNC = 0;
 RC1STAbits.SPEN = 1;

 tx_wptr = 0;
 tx_rptr = 0;
 rx_wptr = 0;
 rx_rptr = 0;

 GIE = 1;
 PEIE = 1;
 RCIE = 1; // enable uart receive interrupt
}

