all: packet_sender packet_receiver

packet_sender: mm.o
	gcc -o packet_sender packet_sender.c mm.o

packet_receiver: mm.o
	gcc -o packet_receiver packet_receiver.c mm.o

mm.o:
	gcc -o mm.o -c mm.c

clean:
	rm *.o packet_receiver packet_sender
