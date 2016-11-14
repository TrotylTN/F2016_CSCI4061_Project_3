all: packet_sender packet_receiver

packet_sender: mm.o packet_sender.c packet.h
	gcc -o packet_sender packet_sender.c mm.o -g

packet_receiver: mm.o packet_receiver.c packet.h
	gcc -o packet_receiver packet_receiver.c mm.o -g

mm.o: mm.c
	gcc -o mm.o -c mm.c

clean:
	rm *.o packet_receiver packet_sender
