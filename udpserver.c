/* udp_server.c */
/* Programmed by Adarsh Sethi */
/* Fall 2019 Emily Evans and Madison Adams */
/* November 17, 2019 */

#include <ctype.h>          /* for toupper */
#include <stdio.h>          /* for standard I/O functions */
#include <stdlib.h>         /* for exit */
#include <string.h>         /* for memset */
#include <sys/socket.h>     /* for socket, sendto, and recvfrom */
#include <netinet/in.h>     /* for sockaddr_in */
#include <unistd.h>         /* for close */
#include <stdlib.h>         /* for atof */
#include <time.h>           /* for seeding srand */
#include "udpStruct.h"

#define STRING_SIZE 1024

/* SERV_UDP_PORT is the port number on which the server listens for
 *   incoming messages from clients. You should change this to a different
 *   number to prevent conflicts with others in the class. 
 */

#define SERV_UDP_PORT 56789

/* Values for tracking statistics */
int totalPacketsReceivedSuccess = 0; //Number of data packets received successfully (without loss, without duplicates)
int totalBytesReceived = 0; //Total number of data bytes received which are delivered to the user
int totalDuplicatePackets = 0; //Total number of duplicate data packets received (without loss)
int totalPacketsLost = 0; //Number of data packets received but dropped due to loss
int allPacketsReceived = 0; //Total number of data packets received (including those that were successful, those lost, and duplicates)
int totalGoodACKs = 0; //Number of ACKs transmitted without loss
int totalLostACKs = 0; //Number of ACKs generated but dropped due to loss
int totalACKGenerated = 0; //Total number of ACKs generated (with and without loss)

int sock_server;  /* Socket on which server listens to clients */

struct sockaddr_in server_addr;  /* Internet address structure that stores server address */
unsigned short server_port;  /* Port number used by server (local port) */

struct sockaddr_in client_addr;  /* Internet address structure that stores client address */
unsigned int client_addr_len;  /* Length of client address structure */

unsigned int msg_len;  /* length of message */
int bytes_sent, bytes_recd; /* number of bytes sent or received */

int SimulateLoss(double PACKET_LOSS){
    double testVal = (double)(rand()%101)/100; //Generate a random number from 0-100, and divide by 100 to get a float from 0-1
    if(testVal < PACKET_LOSS){
        return 0; //The packet is lost
    }else{
        return 1; //The packet is recieved successfully.
    }
}

int SimulateACKLoss(double ACK_LOSS){
    double testVal = (double)(rand()%101)/100; //Generate a random number from 0-100, and divide by 100 to get a float from 0-1
    if(testVal < ACK_LOSS){
        return 0; //The ACK is lost
    }else{
        return 1; //The ACK is sent successfully
    }
}

int SendACK(int seqNum, float ACK_LOSS){
    /* prepare the message to send */
    int bytes_sent;
    int msg_len;
    totalACKGenerated++;
    ack.ACK = htons(seqNum);
    msg_len = sizeof(ack); 
    
    
    /* send message */
    
    if(SimulateACKLoss(ACK_LOSS)){
        bytes_sent = sendto(sock_server, &ack, msg_len, 0, (struct sockaddr*) &client_addr, client_addr_len); //If SimulateACKLoss returns 1, send the packet
        printf("ACK %i transmitted\n", ntohs(ack.ACK)); //The ACK has been converted to network form, so convert back temporarily for this print out
        totalGoodACKs++;
        return bytes_sent;
    }else{
        printf("Ack %i lost\n", ntohs(ack.ACK)); //The ACK has been converted to network form, so convert back temporarily for this print out
        totalLostACKs++; //If SimulateACKLoss returns 0, do not transmit an ACK
        return 0;
    }
}

int main(int argc, char** argv) {
    srand(time(0));
    
    /* Floating values for the packet loss and ack loss chances */
    double PACKET_LOSS;
    double ACK_LOSS;
    
    /* Expected Sequence number for next data packet */
    int expectSeq = 0;
        
    if(argc != 3){
        printf("Format to type is:\nudpserver <Packet Loss Rate> <ACK Loss Rate>\n");
        exit(1);
    }else{
        PACKET_LOSS = atof(*(argv+1));
        ACK_LOSS = atof(*(argv+2));
    }
    
    /* open a socket */
    
    if ((sock_server = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
        perror("Server: can't open datagram socket\n");
        exit(1);
    }
    
    /* initialize server address information */
    
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl (INADDR_ANY);  /* This allows choice of
    any host interface, if more than one
    are present */
    server_port = SERV_UDP_PORT; /* Server will listen on this port */
    server_addr.sin_port = htons(server_port);
    
    /* bind the socket to the local server port */
    
    if (bind(sock_server, (struct sockaddr *) &server_addr, sizeof (server_addr)) < 0) {
        perror("Server: can't bind to local address\n");
        close(sock_server);
        exit(1);
    }
    
    /* wait for incoming messages in an indefinite loop */
    
    printf("Waiting for incoming messages on port %hu\n\n", server_port);
    
    client_addr_len = sizeof (client_addr);
    
    /* Open/Create the file for output*/
    FILE *out;
    out = fopen("output.txt", "w");
    
    while(1) {
        
        bytes_recd = recvfrom(sock_server, &dataRecv, STRING_SIZE, 0, (struct sockaddr *) &client_addr, &client_addr_len);
        allPacketsReceived++;
        /* Check the count field of the received packet, if it is 0, that is the EOF, so imediately quit*/
        dataRecv.count = ntohs(dataRecv.count);   
        if(dataRecv.count == 0){
            break;
        }
        /* Check if the packet is "lost." If so, then restart the loop and do nothing else. If not, then convert the ints and performs other operations depending on the status of count and seqNum. */
        if(!SimulateLoss(PACKET_LOSS)){
            printf("Packet %i lost\n",ntohs(dataRecv.seqNum));
            totalPacketsLost++;
            continue; //Restart the loop (go back to waiting for packet) if the packet is "lost"
        }
        
        /* Convert the ints in the recieved data from network to host long. */
        
        dataRecv.seqNum = ntohs(dataRecv.seqNum);
        
        /* Check the recieved sequence number against the expected sequnce number */
        if(dataRecv.seqNum != expectSeq){
            printf("Duplicate Packet %i recieved with %i data bytes\n", dataRecv.seqNum, dataRecv.count);
            totalDuplicatePackets++;
            bytes_sent = SendACK(1 - expectSeq, ACK_LOSS);
            continue; //If the sequence number is not the expected sequence number (Meaning the packet was a duplicate), send an ACK of the last sequence number, restart the loop and wait for a new packet
        }else{
            printf("Packet %i received with %i data bytes\n", dataRecv.seqNum, dataRecv.count);
            totalPacketsReceivedSuccess++;
        }
        
        /* If the packet is not "lost," or the packet's data field is empty, write the data from the packet to the output buffer string. */
        
        if(dataRecv.count != 0){
            for(int i = 0; i < dataRecv.count; i++){
                fputc(dataRecv.payload[i],out);
            }
            totalBytesReceived+=dataRecv.count;
        }else{
            break; //If the count field is 0, this is the EOF packet, and the server must quit, close the file, and print telemetry information
        }
        
        bytes_sent = SendACK(expectSeq, ACK_LOSS);
        expectSeq = 1 - expectSeq; //Flip the expected sequencer number between 0 and 1
        
    }
    /* Close the output file */
    fclose(out);
    /* Print the telemetry information */
    printf("After the file transfer:\n\n");
    printf("Number of data packets received successfully: %i\n", totalPacketsReceivedSuccess);
    printf("Number of data bytes received and sent to the user: %i\n", totalBytesReceived);
    printf("Total number of duplicate data packets received: %i\n", totalDuplicatePackets);
    printf("Number of data packets dropped due to loss: %i\n", totalPacketsLost); 
    printf("Total number of data packets received (including lost and duplicate packets): %i\n", allPacketsReceived);
    printf("Number of ACKs transmitted without loss: %i\n", totalGoodACKs);
    printf("Number of ACKs generated but dropped due to loss: %i\n", totalLostACKs);
    printf("Total number of ACKs generated (with and without loss): %i\n", totalACKGenerated);
}
