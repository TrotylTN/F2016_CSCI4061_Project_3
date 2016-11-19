~~~
＊ CSci4061 F2016 Assignment 3
＊ login: zhou0745
＊ date: 11/19/2016
＊ name: Tiannan Zhou, Annelies Odermann
＊ id: 5232494 (zhou0745), 4740784 (oderm008)
~~~

**How to compile the program.**

~~~
make
~~~

You just need to use GNU makefile to compile this project.

**Who did what on the program**
* Tiannan Zhou
	* Implemented packets sender and receiver (Part B)
* Annelies Odermann
	* Completed Memory Management (Part A) and fixed stack smash bug.


**Usage**
0. Run `./mm` to test and compare the original malloc and our memory manager.
1. Run `./packet_receiver <# of packets to receive>` in the terminal to start the receiver.
2. Run `./packet_sender <# of packets to send>` in another terminal to start the sender. Please note that the number of packets to receive which is entered here should be equal to the number entered when starting the receiver.
3. Wait the two programs to complete the transmission.

**Description**
1. The receiver would send its PID to the sender then it would receive and assemble the messages sent from the sender.
2. The sender would receive the PID of the receiver and send unordered messages to the receiver. It would also send a SIGIO signal to the receiver each time when a packet is sent.
