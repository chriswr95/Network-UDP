/* Christopher Wright
 * Server.c cointains code to impliement UDP with data corruption, and packet loss. 
 */


#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>
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
struct sockaddr_in serverAddr, clientAddr;
struct sockaddr_storage serverStorage;
socklen_t addr_size, client_addr_size;


int myrecv(char buff[]){
	printf("in myrecv \n");
	int nBytes;
	struct PACKET p;
	struct PACKET recvP;
	//recvP = (struct PACKET *) malloc(sizeof(struct PACKET)); 
    	while(1){    
		//recv
		recvfrom (sock, &recvP, sizeof(struct PACKET), 0, (struct sockaddr *)&serverStorage, &addr_size);
			printf("Packet recieved!\n");
			buff = recvP.data;
			nBytes = recvP.h.len;	

			//check	
			//	
			return nBytes;
		
	}
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

int main (int argc, char *argv[]) {
    int sock, nBytes, currentState, recvCksum;
    char buff[10], fileName[10];
    int i;
    FILE *dst; 
    struct PACKET recvP;
    struct HEADER ack;
    if (argc != 2){
        printf ("need the port number\n");
        return 1;
    }
     
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons ((short)atoi (argv[1]));
    serverAddr.sin_addr.s_addr = htonl (INADDR_ANY);
    clientAddr.sin_addr.s_addr = htonl (INADDR_ANY);
    memset ((char *)serverAddr.sin_zero, '\0', sizeof (serverAddr.sin_zero));
    addr_size = sizeof (serverStorage);
    
    
    if ((sock = socket (AF_INET, SOCK_DGRAM, 0)) < 0){
        printf ("socket error\n");
        return 1;
    }
    
    if (bind (sock, (struct sockaddr *)&serverAddr, sizeof (serverAddr)) != 0){
        printf ("bind error\n");
        return 1;
    }
    //initialize current state and seed rand fxn
    currentState = 0;
    srand(time(NULL));
	
	//recieve filename
	recvfrom(sock, &recvP, sizeof(struct PACKET), 0, (struct sockaddr *)&serverStorage, &addr_size);
	strcpy(fileName, recvP.data);	
	dst = fopen(fileName, "wb");

	while(1){
		if((nBytes = recvfrom (sock, &recvP, sizeof(struct PACKET), 0, (struct sockaddr *)&serverStorage, &addr_size)) > 0){
			printf("Packet recieved!\n");
			//Simulate packet timout
			if(rand() % 4 == 1){
				printf("dropping packet..\n");
				continue;
			}
			//check for debug
			printf("cksum: %d\n", recvP.h.cksum);
			printf("data: %s\n", recvP.data);

			//check checksum and currentState are correct
			recvCksum = recvP.h.cksum;	
			recvP.h.cksum = 0;
			printf("calced cksum: %d\n",  checksum((char*) &recvP, sizeof(recvP.h) + recvP.h.len));
			if( recvP.h.seq_ack == currentState && recvCksum == checksum((char*) &recvP, sizeof(recvP.h) + recvP.h.len)){

				//Write data to file
				fwrite(recvP.data, sizeof(char), recvP.h.len, dst);

				//setup ack
				ack.seq_ack = currentState; //will need to change to current state
				ack.len = 0;
				ack.cksum = 0;
				ack.cksum = checksum((char*)&ack, sizeof(struct HEADER));

				sendto(sock, &ack, sizeof(struct HEADER), 0, (struct sockaddr *)&serverStorage, addr_size);
				currentState = switchState(currentState);
			} else {
				printf("bad packet recieved\n");
				printf("sending state: %d\n", switchState(currentState));
				ack.seq_ack = switchState(currentState);
				ack.len = 0;
				ack.cksum = 0;
				ack.cksum = checksum((char*)&ack, sizeof(struct HEADER));
				sendto(sock, &ack, sizeof(struct HEADER), 0, (struct sockaddr *)&serverStorage, addr_size);
			}
			if(recvP.h.len == 0){
				fclose(dst);
			}
		}

	}
    
    return 0;
}
