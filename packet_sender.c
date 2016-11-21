/* CSci4061 F2016 Assignment 3
 * login: zhou0745
 * date: 11/19/16
 * name: Tiannan Zhou, Annelies Odermann
 * id: 5232494(zhou0745), 4740784(oderm008) */
#include <time.h>
#include "packet.h"

static int pkt_cnt = 0;     /* how many packets have been sent for current message */
static int pkt_total = 1;   /* how many packets to send send for the message */
static int msqid = -1; /* id of the message queue */
static int receiver_pid; /* pid of the receiver */

/*
   Returns the packet for the current message. The packet is selected randomly.
   The number of packets for the current message are decided randomly.
   Each packet has a how_many field which denotes the number of packets in the current message.
   Each packet is string of 3 characters. All 3 characters for given packet are the same.
   For example, the message with 3 packets will be aaabbbccc. But these packets will be sent out order.
   So, message aaabbbccc can be sent as bbb -> aaa -> ccc
   */
static packet_t get_packet() {
  static int which = -1;
  static int how_many;
  static int num_of_packets_sent = 0;
  static int is_packet_sent[MAX_PACKETS];
  int i;

  packet_t pkt;

  if (num_of_packets_sent == 0) {
    how_many = rand() % MAX_PACKETS;
    if (how_many == 0) {
      how_many = 1;
    }
    printf("Number of packets in current message: %d\n", how_many);
    which = -1;
    for (i = 0; i < MAX_PACKETS; ++i) {
      is_packet_sent[i] = 0;
    }
  }
  which = rand() % how_many;
  if (is_packet_sent[which] == 1) {
    i = (which + 1) % how_many;
    while (i != which) {
      if (is_packet_sent[i] == 0) {
        which = i;
        break;
      }
      i = (i + 1) % how_many;
    }

  }
  pkt.how_many = how_many;
  pkt.which = which;

  memset(pkt.data, 'a' + which, sizeof(data_t));

  is_packet_sent[which] = 1;
  num_of_packets_sent++;
  if (num_of_packets_sent == how_many) {
    num_of_packets_sent = 0;
  }

  return pkt;
}
// Once receive a ALARM signo, send a packet of message.
static void packet_sender(int sig) {
  packet_t pkt;

  pkt = get_packet();
  pkt_total = pkt.how_many;
  pkt_cnt++;
  // temp is just used for temporarily printing the packet.
  char temp[PACKET_SIZE + 2];
  strcpy(temp, pkt.data);
  temp[3] = '\0';
  printf ("Sending packet: %s\n", temp);
  packet_queue_msg packet_sent;
  packet_sent.mtype = msg_key;
  packet_sent.pkt = pkt;
  if (msgsnd(msqid, &packet_sent, sizeof(packet_queue_msg) - sizeof(long), 0) == -1) {
    perror("Error in sending message");
    return;
  }
  kill(receiver_pid, SIGIO);
}

// handle exit
void int_handler(int sig) {
  printf("Received SIGINT...\n");
  kill(receiver_pid, SIGINT);
  msgctl(msqid, IPC_RMID, 0);
  exit(0);
}

int main(int argc, char **argv) {
  if (argc != 2) {
    printf("Usage: packet_sender <num of messages to send>\n");
    exit(-1);
  }

  int k = atoi(argv[1]); /* number of messages  to send */
  srand(time(NULL)); /* seed for random number generator */

  int i;

  struct itimerval interval;
  struct sigaction act;

  msqid = msgget(key, 0666 | IPC_CREAT);
  // Try to receive PID of the receiver
  pid_queue_msg pid_pkt_recved;
  if (msgrcv(msqid, &pid_pkt_recved, sizeof(pid_queue_msg) - sizeof(long), rcv_key, 0) == -1) {
    perror("Error in Receiving Pid of Receiver");
    return -1;
  }
  receiver_pid = pid_pkt_recved.pid;
  printf("Got receiver's pid : %d\n", receiver_pid);
  // send PID to receiver
  pid_queue_msg pid_pkt_sent;
  pid_pkt_sent.mtype = snd_key;
  pid_pkt_sent.pid = getpid();
  if (msgsnd(msqid, (void *)&pid_pkt_sent, sizeof(pid_queue_msg) - sizeof(long), 0) == -1) {
    perror("Error in Sending Pid");
    return -1;
  }
  /* Set the action for SIGALRM */
  act.sa_handler = packet_sender;
  act.sa_flags = 0;
  sigfillset(&act.sa_mask);
  sigaction (SIGALRM, &act, NULL);
  // Set the handler for SIGINT
  struct sigaction actint;
  actint.sa_handler = int_handler;
  actint.sa_flags = 0;
  sigfillset(&actint.sa_mask);
  sigaction (SIGINT, &actint, NULL);
  /* Set the timer */
  interval.it_interval.tv_sec = INTERVAL;
  interval.it_interval.tv_usec = INTERVAL_USEC;
  interval.it_value.tv_sec = INTERVAL;
  interval.it_value.tv_usec = INTERVAL_USEC;
  setitimer (ITIMER_REAL, &interval, NULL);
  /* NOTE: the below code wont run now as you have not set the SIGALARM handler. Hence,
     set up the SIGALARM handler and the timer first. */
  for (i = 1; i <= k; i++) {
    printf("==========================%d\n", i);
    printf("Sending Message: %d\n", i);
    while (pkt_cnt < pkt_total) {
      pause(); /* block until next packet is sent. SIGALARM will unblock and call the handler.*/
    }
    pkt_cnt = 0;
    usleep(10000);
  }

  msgctl(msqid, IPC_RMID, 0);
  return EXIT_SUCCESS;
}
