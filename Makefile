all: main.c
	avr-gcc -mmcu=attiny2313 -DF_CPU=12000000 -Wall -funsigned-char -Os -o joystick.elf -I. \
		main.c usbdrv/usbdrv.c usbdrv/usbdrvasm.S usbdrv/oddebug.c
	avr-objcopy -j .text -j .data -O ihex joystick.elf joystick.hex

flash: all
	avrdude -v -p m8 -c usbasp -U main.hex

clean:
	del joystick.elf joystick.hex
