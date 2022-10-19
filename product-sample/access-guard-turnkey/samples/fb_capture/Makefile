CROSS_COMPILE	?= arm-linux-gnueabihf-

EXE_BOARD	:= ZAM180

MINIGUI := -I /usr/local/minigui3012-arm-zam180/include/
SOURCE 		:= *.c */*.c 
CFLAGS		:= ./iolib/
EXECUTABLE	:= capture

$(EXE_BOARD) :
	$(CROSS_COMPILE)gcc -o $(EXECUTABLE) $(SOURCE)

clean:
	rm -f $(EXECUTABLE)