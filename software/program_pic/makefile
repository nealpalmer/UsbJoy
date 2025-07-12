all:	program_pic
	echo program_pic pic.hex COM7
	./program_pic ../usbjoystick_pic/usbjoystick_pic.X/dist/default/production/usbjoystick_pic.X.production.hex COM7

program_pic:	program_pic.cpp
	g++ -o program_pic program_pic.cpp
