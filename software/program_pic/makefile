all:	program_pic
	#./program_pic usbjoy usbjoy.hex COM7
	./program_pic.exe usbjoy ../UsbJoy_pic_firmware/usbjoystick_pic.X/dist/default/production/usbjoystick_pic.X.production.hex COM7

sb:	program_pic
	./program_pic sb smartbutton.hex COM7

program_pic:	program_pic.cpp makefile
	#g++ -o program_pic program_pic.cpp
	x86_64-w64-mingw32-gcc program_pic.cpp -o program_pic 
