#define LINELEN 80 /*no more than 80 characters per line inlcuding newline character */
#define HEADERSIZE 4 /* 4 byte long header */
#define PACKETSIZE 84 /* Max packet size */
#define STRING_SIZE 1024

struct packet {
    /* Values from the client */
    short count;          /* Number of data characters in the packet (0-80) */
    short seqNum;		/* 0 or 1, in accordance with the stop and wait protocol*/
    char payload[LINELEN];       /* Characters from the document sent from the client  */
}dataRecv;

struct clientData {
    /* Values from the client */
    short count;          /* Number of data characters in the packet (0-80) */
    short seqNum;		/* 0 or 1, in accordance with the stop and wait protocol*/
    char payload[LINELEN];       /* Characters from the document sent from the client */
}dataSend;

struct ACKpacket {
    /* Response from the server */
    short ACK;            /* ACK to return for the data packet (0 or 1) */
}ack;