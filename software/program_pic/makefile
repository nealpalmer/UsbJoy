all:	program_pic
	#./program_pic usbjoy usbjoy.hex COM7
	./program_pic usbjoy ../UsbJoy_pic_firmware/usbjoystick_pic.X/dist/default/production/usbjoystick_pic.X.production.hex COM7

sb:	program_pic
	./program_pic sb smartbutton.hex COM7

program_pic:	program_pic.cpp
	g++ -o program_pic program_pic.cpp
