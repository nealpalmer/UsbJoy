
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <iostream>

void send_bits(uint32_t val, int bitcount);
void sendcommand6_read14(uint8_t val,uint16_t &data);
void sendcommand6_14(uint8_t val,uint16_t data);
void sendcommand6(uint32_t val);
void recvbits(uint16_t &data, int bitcount);
void parse_hex_file(FILE *fh);

 HANDLE hComm;
 uint8_t code_mem[65536];
 uint8_t config_mem[65536];


int main(int argc, char *argv[])
{
 FILE *fh_hex;

 if (argc!=3) {
		 printf("USAGE: %s filename.hex com#\n",argv[0]);
		 exit(-1);
 }

 	// open hex file
 fh_hex = fopen(argv[1],"r");
 if (fh_hex==NULL) {
		 printf("Couldn't open '%s' for reading\n",argv[1]);
		 exit(-1);
 }
 parse_hex_file(fh_hex);

 	// open com port
 LPCSTR szPortName = argv[2]; // Replace with your desired COM port, e.g., "COM2", "COM10"

 hComm = CreateFile(
	szPortName,                 // Port name (e.g., "COM1", "\\\\.\\COM10" for COM ports above 9)
	GENERIC_READ | GENERIC_WRITE, // Desired access: read and write
	0,                          // No sharing
	NULL,                       // No security attributes
	OPEN_EXISTING,              // Open existing port
	FILE_ATTRIBUTE_NORMAL,      // Normal file attributes
	NULL                        // No template file
 );

 if (hComm == INVALID_HANDLE_VALUE) {
	// Handle error, e.g., port not found or in use
	std::cerr << "Error opening COM port: " << GetLastError() << std::endl;
	return 1; // Or handle appropriately
 }

// Configure serial port parameters
 DCB dcbSerialParams = {0};
    dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
    if (!GetCommState(hComm, &dcbSerialParams)) {
        std::cerr << "Error getting comm state." << std::endl;
        CloseHandle(hComm);
        return 1;
    }
    dcbSerialParams.BaudRate = CBR_9600;
    dcbSerialParams.ByteSize = 8;
    dcbSerialParams.StopBits = ONESTOPBIT;
    dcbSerialParams.Parity = NOPARITY;
    if (!SetCommState(hComm, &dcbSerialParams)) {
        std::cerr << "Error setting comm state." << std::endl;
        CloseHandle(hComm);
        return 1;
    }
 

 COMMTIMEOUTS timeouts = {0};
 // Set timeouts
    timeouts.ReadIntervalTimeout = 50;
    timeouts.ReadTotalTimeoutConstant = 50;
    timeouts.ReadTotalTimeoutMultiplier = 10;
    timeouts.WriteTotalTimeoutConstant = 50;
    timeouts.WriteTotalTimeoutMultiplier = 10;
    if (!SetCommTimeouts(hComm, &timeouts)) {
        std::cerr << "Error setting timeouts." << std::endl;
        CloseHandle(hComm);
        return 1;
    }

 	// send a command that should generate a response
 DWORD bytes_xferred;
 char buff[1024];

 do {
 	WriteFile(hComm, "z\n", 2,&bytes_xferred, NULL);
 	printf("Wrote 'z\\n' command, expecting 'UsbJoy' response\n");

	bytes_xferred=0;

 	ReadFile(hComm, buff, 10, &bytes_xferred,NULL);
 	for (int i=0;i<bytes_xferred;i++)
		 printf("%c", buff[i]);

	if ((buff[bytes_xferred-1]=='\n') && (strncmp(buff,"UsbJoy",6)==0)) {
		break;
	}
 } while (1);

 	// ******************************
	// Setup the hardware for programming mode.
 	// ******************************
 printf("Enter programming mode (MCLRn=0)\n");
 WriteFile(hComm, "n0\n", 3,&bytes_xferred, NULL); // disable neopixel
 WriteFile(hComm, "q1\n", 3,&bytes_xferred, NULL);
 Sleep(1000);

 	// ******************************
	// Initialize low voltage programming.
	// Send: 0100 1101 0100 0011 0100 1000 0101 0000 (b2 c2 12 0a) "MCHP"
 	// ******************************
 printf("Initialize low voltage programming mode\n");
 for (int i=0;i<1;i++) {
	if (1) {
 		send_bits('P', 8);
 		send_bits('H', 8);
 		send_bits('C', 8);
 		send_bits('M', 8);
	}
 }
 send_bits(0,1); // don't know why this is necessary, but it works better this way...

 Sleep(100);


#define CMD_LOAD_CONFIG 0
#define CMD_LOAD_DATA 2
#define CMD_READ_DATA 4
#define CMD_INCREMENT_ADDR 6
#define CMD_RESET_ADDR 0x16
#define CMD_BEGIN_INT_TIMED_PROG 0x08
//#define CMD_BEGIN_EXT_TIMED_PROG 0x18
//#define CMD_END_EXT_TIMED_PROG 0x0A
#define CMD_BULK_ERASE 0x09
//#define CMD_ROW_ERASE 0x11
	
#if 1
 	// ******************************
	// Check the ID of the CPU to make sure it is the right one.
 	// ******************************
 printf("Read configuration words\n");
 sendcommand6_14(CMD_LOAD_CONFIG,0x0000);
 //sendcommand6(CMD_RESET_ADDR);
 uint16_t data;
 for (int i=0;i<10*1+6;i++) {
 	sendcommand6_read14(CMD_READ_DATA,data);
 	sendcommand6(CMD_INCREMENT_ADDR);
	printf("config[%d] = 0x%x\n",i,data);
	if (i==6) {
		if (data==0x308f) {
			printf("Found pic16f1778\n");
		} else {
			printf("Error: couldn't find expected PIC CPU\n");
			exit(-1);
		}
	}
 }
#endif

 	// ******************************
	// Erase the PIC
 	// ******************************
if (1) {
 sendcommand6(CMD_BULK_ERASE);
 Sleep(5); // 5ms wait for bulk erase
}

 	// ******************************
	// Program hex program into PIC
 	// ******************************
if (1) {
#define CODE_WORDS 0x4000
 printf("Programming PIC's code memory\n");fflush(stdout);
 sendcommand6(CMD_RESET_ADDR);
 bool need_prog = false;
 for (int i=0;i<CODE_WORDS*2;i+=2) {
	if ((code_mem[i]!=0xff) || (code_mem[i+1]!=0xff)) {
		sendcommand6_14(CMD_LOAD_DATA,(((uint16_t)code_mem[i])) | (((uint16_t)code_mem[i+1])<<8));
		need_prog = true;
	}
	if (need_prog && ((i&63)==62)) {
		sendcommand6(CMD_BEGIN_INT_TIMED_PROG);
		Sleep(3); // sleep for 2.5ms to let the programming operation finish
		need_prog = false;
	}
	sendcommand6(CMD_INCREMENT_ADDR);
 }
}	
 	// ******************************
	// Program configuration bits
 	// ******************************
if (1) {
 sendcommand6_14(CMD_LOAD_CONFIG,0); // move to configuration memory.

 for (int i=0;i<0x20;i+=2) {
	if ((config_mem[i]!=0xff) || (config_mem[i+1]!=0xff)) {
		printf("config[%d] = 0x%2.2x%2.2x\n",i,config_mem[i+1],config_mem[i]);
		sendcommand6_14(CMD_LOAD_DATA,(((uint16_t)config_mem[i])) | (((uint16_t)config_mem[i+1])<<8));
			// program the loaded word
		sendcommand6(CMD_BEGIN_INT_TIMED_PROG);
		Sleep(5); // sleep for 5ms to let the programming operation finish
	}
	sendcommand6(CMD_INCREMENT_ADDR);
 }
 printf("DONE: programming configuration words\n\n\n");

}

 	// ******************************
	// Verify hex program from PIC
 	// ******************************
if (1) {
 printf("Verify PIC's code memory\n");fflush(stdout);
 sendcommand6(CMD_RESET_ADDR);
 Sleep(1); // wait >10us
 for (int i=0;i<16;i++) {
	uint16_t data;
 	sendcommand6_read14(CMD_READ_DATA,data);
	printf("code[0x%x] = 0x%4.4x\n",i,data);
	sendcommand6(CMD_INCREMENT_ADDR);
 }
}

 	// ******************************
	// EXIT the hardware programming mode.
 	// ******************************
 printf("EXIT programming mode\n");
 WriteFile(hComm, "q0\n", 3,&bytes_xferred, NULL);
 
 if (0)
 for (int j=0;j<10;j++) {
 	ReadFile(hComm, buff, 1, &bytes_xferred,NULL);
 	for (int i=0;i<bytes_xferred;i++)
		 printf("%c", buff[i]);
 }
}

void sendcommand6_read14(uint8_t val,uint16_t &data)
{
 uint16_t tmp;
 send_bits(val,6);

 if (0) {
 	recvbits(tmp,1);
 	recvbits(data,14);
 	recvbits(tmp,1);
 } else {
 	recvbits(data,16);
	data = (data>> 1)&0x3fff;
 }

 data &= 0x3fff;
 //printf("read14 = 0x%x\n",data);
}

void sendcommand6_14(uint8_t val,uint16_t data)
{
 send_bits(val,6);

 //send_bits(0,1);
 send_bits((data&0x3fff)<<1,16);
 //send_bits(0,1);
}

void sendcommand6(uint32_t val)
{
 send_bits(val,6);
}

void send_bits(uint32_t val, int bitcount)
{
 char str[1024];
 DWORD bytes_xferred;
 sprintf(str,"w%x%4.4x\n",bitcount,val&0xffff);
 //printf("sent: %s",str);
 WriteFile(hComm, str, strlen(str),&bytes_xferred, NULL);
}

void recvbits(uint16_t &data, int bitcount)
{
 char str[1024];
 char buff[1024];
 int index=0;
 DWORD bytes_xferred;

 sprintf(str,"r%x\n",bitcount);
 //printf("sent: %s",str);
 WriteFile(hComm, str, strlen(str),&bytes_xferred, NULL);

 index=0;
 int attempts=0;
 do {
	buff[index]=0;
	buff[index+1]=0;
 	ReadFile(hComm, buff+index, 1, &bytes_xferred,NULL);
	//if (bytes_xferred!=0)
		//printf("%c",buff[index]);
	index+=bytes_xferred;
	attempts++;
 } while ((buff[index-1]!='\n') && (attempts<1000));

 int val;
 sscanf(buff+1,"%x",&val);
 data = val;
}


uint8_t hex_to_uchar(char *ptr)
{
 char str[3];
 str[0] = ptr[0];
 str[1] = ptr[1];
 str[2] = 0;
 return strtol(str,NULL,16);
}


void parse_hex_file(FILE *fh)
{
 for (int i=0;i<65536;i++) code_mem[i] = 0xff;
 for (int i=0;i<65536;i++) config_mem[i] = 0xff;

 uint8_t region=0; // program memory 
 int remaining = 5;
 while (!feof(fh)) {
	 char oneline[1024];
	 oneline[0] = 0;
	 fgets(oneline,1024,fh);
	 if (oneline[0]!=':')
		 break;
	 //printf("%s",oneline);

	 uint8_t count=hex_to_uchar(oneline+1);
	 uint16_t offset=(hex_to_uchar(oneline+1+2)<<8) + hex_to_uchar(oneline+1+2+2);
	 uint16_t type=hex_to_uchar(oneline+1+2+2+2);
	 if (type!=0)
			 remaining = 5;
	 if (remaining>0)
	 	printf(": %d 0x%x %d: ",count,offset,type);
	 if (type==0) {
		 // is data
		 if (region==0) {
			 // program data
			 for (int i=0;i<count;i++) {
				code_mem[offset+i] = hex_to_uchar(oneline+1+8+i*2);
	 			if (remaining>0)
					printf("%2.2x ",code_mem[offset+i]);
			 }
		 }
		 if (region==1) {
			 // configuration words
			 for (int i=0;i<count;i++) {
				config_mem[offset+i] = hex_to_uchar(oneline+1+8+i*2);
	 			if (remaining>0)
					printf("%2.2x ",config_mem[offset+i]);
			 }
		 }
	 }
	 if (type==4) {
		 // is memory region
		 region = (hex_to_uchar(oneline+1+2+2+2+2)<<8) + hex_to_uchar(oneline+1+2+2+2+2+2);
		if (remaining>0)
		 	printf("region = %d",region);
	 }
	 if (type==1) {
		 printf("eof");
	 }
	if (remaining>0)
		 printf("\n");
	remaining--;
 }
 fclose(fh);
}
