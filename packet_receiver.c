
#include "packet.h"

int msqid = -1;

static message_t message;   /* current message structure */
static mm_t mm;             /* memory manager will allocate memory for packets */
static int pkt_cnt = 0;     /* how many packets have arrived for current message */
static int pkt_total = 1;   /* how many packets to be received for the message */

/*
   Handles the incoming packet.
   Store the packet in a chunk from memory manager.
   The packets for given message will come out of order.
   Hence you need to take care to store and assemble it correctly.
   Example, message "aaabbb" can come as bbb -> aaa, hence, you need to assemble it
   as aaabbb.
   Hint: "which" field in the packet will be useful.
 */
static void packet_handler(int sig) {
  packet_t pkt;
  void *chunk;

  chunk = mm_get(&mm);
  if (chunk == NULL) {
    perror("Error in allocate memory");
    return;
  }
  if (msgrcv(msqid, &chunk, CHUNK_SIZE, 0, 0) == -1) {
    perror("Error in Receiving Packets");
    return;
  }
  message.data[message.num_packets] = chunk;
  message.num_packets++;
  pkt_cnt++;
}

static char *assemble_message() {

  char *msg;
  int i;
  int msg_len = message.num_packets * sizeof(data_t);

  msg = (char *) malloc(msg_len * sizeof(char));
  for (i = 0; i < message.num_packets; i++) {
    strncpy(msg + ((packet_t *)message.data[i])->which,
            ((packet_t *)message.data[i])->data,
            PACKET_SIZE);
    mm_put(&mm, message.data[i]);
  }
  pkt_total = 1;
  pkt_cnt = 0;
  message.num_packets = 0;

  return msg;
}

int main(int argc, char **argv) {
  if (argc != 2) {
    printf("Usage: packet_sender <num of messages to receive>\n");
    exit(-1);
  }

  int k = atoi(argv[1]); /* no of messages you will get from the sender */
  int i;
  char *msg;

  mm_init(&mm, NUM_CHUNKS, CHUNK_SIZE);

  message.num_packets = 0;

  msqid = msgget(key, 0666 | IPC_CREAT);
  if (msqid == -1) {
    perror("Error in Creating Queue");
    return -1;
  }

  pid_queue_msg pid_pkt_sent;
  pid_pkt_sent.mtype = 1;
  pid_pkt_sent.pid = getpid();
  if (msgsnd(msqid, (void *)&pid_pkt_sent, sizeof(pid_queue_msg), 0) == -1) {
    perror("Error in Sending Pid");
    return -1;
  }
  /* TODO set up SIGIO handler to read incoming packets from the queue. Check packet_handler()*/
  struct sigaction act;
  act.sa_handler = packet_handler;
  sigaction(SIGIO, &act, NULL);

  for (i = 1; i <= k; i++) {
    while (pkt_cnt < pkt_total) {
      pause(); /* block until next packet */
    }

    msg = assemble_message();
    if (msg == NULL) {
      perror("Failed to assemble message");
    }
    else {
      fprintf(stderr, "GOT IT: message=%s\n", msg);
      free(msg);
    }
  }

  // TODO deallocate memory manager
  mm_release(&mm);
  // TODO remove the queue once done
  msgctl(msqid, IPC_RMID, 0);
  return EXIT_SUCCESS;
}
