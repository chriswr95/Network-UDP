/* Christopher Wright 
 * Lab 3 Client: 
 * Client.c contains code to impliment UDP sending functionality.
 * Client.c takes in a port number, and ip address, an input file name
 * and an ouput file name and transports the file via UDP protocol to
 * the server. The program also contains code to deal with corrupted data
 * which it detects using a checksum.
 */
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <math.h> 
#include <time.h>


//Structs

struct HEADER {
	int seq_ack;
	int len;
	int cksum;
};

struct PACKET {
	struct HEADER h;
	char data[10];
	
};


//Global Variables
int sock;
struct sockaddr_in serverAddr;
socklen_t addr_size;

int mywrite(char buff[], int bytesRead, int *state){
	int nBytes, recvCksum;
	struct PACKET p;
	struct HEADER ack; 
	int packetSent = 0;
	//initialize packet
	strcpy(p.data, buff);
	p.h.seq_ack = *state;
	p.h.len = bytesRead;

	while(!packetSent){	
		p.h.cksum = 0;
		p.h.cksum = checksum((char*) &p, sizeof(p.h) + p.h.len);
		if(rand() % 10 > 5 && bytesRead > 0){
			//send bad packet
			printf("sending bad packet\n");
			p.h.cksum = 0;
		}
		printf("cksum: %d\n", p.h.cksum);
		//send
		printf("Sending:  %s\n", buff);
		sendto (sock, &p, sizeof(p), 0, (struct sockaddr *)&serverAddr, addr_size);	

		//recieve
		recvfrom(sock, &ack, sizeof(struct HEADER), 0, (struct sockaddr *)&serverAddr, &addr_size);
		printf("recieved ACK %d\n", ack.seq_ack);
		printf("recieved cksum %d\n", ack.cksum);
		//check ack and state	
		recvCksum = ack.cksum;
		ack.cksum = 0;
		
		if(ack.seq_ack == *state && recvCksum == checksum((char*) &ack, sizeof(struct HEADER))){
			//packet recieved succesfully, change state and send next packet
			*state = switchState(*state);
			packetSent = 1;
		}
		//else resend packet
	}
	return bytesRead;
}

int checksum(char *packet, int n){
	int i;
	int sum = 0;
	for(i = 0; i < n; i++){
		sum = sum ^ packet[i];
	}
	return sum;
}

int switchState(int state){
	if (state == 0){
		return 1;
	}
	return 0;
}

int main (int argc, char *argv[]){
	int  portNum, nBytes, bytesRead, state;
	char buff1[1024];
	char buff[10];
	FILE *src;
	struct PACKET fname;

	if (argc != 5){  //supposed to be 5?
		printf ("Input Format: port IP filename\n");
		return 1;
	}
	//seed rand()
	srand(time(NULL));
	//configure address
	printf("Configuring address..\n");
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons (atoi (argv[1]));
	memset (serverAddr.sin_zero, '\0', sizeof (serverAddr.sin_zero));

	if(inet_pton (AF_INET, argv[2], &serverAddr.sin_addr.s_addr) <= 0){
		printf("inet_pton error occured\n");
		return 1;
	}
	addr_size = sizeof(serverAddr);
	
	/*Create UDP socket*/
	printf("Creating UDP socket...\n");
	if((sock = socket (PF_INET, SOCK_DGRAM, 0)) < 0){
		printf("Error: could not create socket \n");
		return 1;
	}

	//Open File
	src = fopen(argv[3], "rb");
	printf("Transmitting...");
	state = 0;

	//send file name
	strcpy(fname.data, argv[4]);
	sendto(sock, &fname, sizeof(struct PACKET), 0, (struct sockaddr *)&serverAddr, addr_size);

	while ( (bytesRead = fread(buff, 1, 10, src)) > 0){
		mywrite(buff, bytesRead, &state);
	}
	//send packet with length 0 to signal end of transmission
	mywrite(buff, 0, &state);

	return 0;
}
