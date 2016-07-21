CFLAGS=-std=c99

all: 	cap notch update

cap:	cap.c libtuff.c
notch:	notch.c libtuff.c
update:	update.c libtuff.c
