
INC = ../include/
CFLAGS = -g 

objects = mkfs.o

.PHONY: clean all

%.o:%.c
	$(CC) -c $(CFLAGS) $< -o $@ -I$(INC) 


all: $(objects)
	gcc $(CFLAGS) $(objects) -o mkfs.epii

$(objects): %.o: %.c

clean:
	rm *.o
	rm mkfs.epii
