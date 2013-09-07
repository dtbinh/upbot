# Filename: Makefile (For compiling portions that are run on the  
# 	gumstix/roomba)
# Author: Tanya L. Crenshaw
# Date created: 7/9/2013
# 

# This makefile currently builds for two targets.
# 1. nerves.out.     The executable file for the network-capable roomba 
#                    communicating with a networked supervisor.
# 2. nervesSolo.out  The executable file for a stand-alone roomba 
#                    executing an event-responder.

CC=arm-linux-gcc
VPATH=./communication: ./robot/roomba: ./robot 

# The two targets have a common set of flags, but the nervesSolo.out
# target has one additional flag.
CFLAGS+=-lrt -I ./communication -I ./robot/roomba -I ./robot 
nervesSolo.out:	CFLAGS+=-DNO_NET

# The two targets have a common set of object files.
OBJS=nerves.o erQueue.o netDataProtocol.o led.o commands.o utility.o sensors.o responders.o events.o clock.o erControl.o myEventResponders.o services.o acceptor.o connector.o mkaddr.o netERProtocol.o

### GUMSTIX TARGETS ####

nerves.out nervesSolo.out:	$(OBJS)
	$(CC) $(CFLAGS) -o $@ -DGUMSTIX $(OBJS)

%.o:	%.c
	$(CC) $(CFLAGS) -c -o $@ $<

.PHONY:	clean

clean:
	rm -f *.o
	rm -f *.out
