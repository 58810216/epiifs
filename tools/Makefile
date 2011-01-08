
CFLAGS = -g 

objects = mkfs.o

.PHONY: clean all

%.o:%.c
	$(CC) -c $(CFLAGS) $< -o $@ 


all: $(objects)
	gcc $(CFLAGS) $(objects) -o mkfs.llfs

$(objects): %.o: %.c

clean:
	rm *.o
	rm mkfs.llfs
