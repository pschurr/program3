all: myftp

myftp: myftp.c
	gcc myftp.c -o myftp

clean:
	rm *.o myftp
