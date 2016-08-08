
#include <iostream>
#include <sstream>
#include <map>
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <cerrno>
using namespace std;

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <wait.h>
#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

#include "cmor/crypto.h"
#include "cmor/encodings.h"
#include <string.h>
//using namespace cgipp;

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

int socket_to_server (const char * IP, int port);
string read_packet (int socket);

class connection_closed {};
class socket_error {};


int main()
{	
	//response times in usec
   double min_response_time=100000;//100 milliseconds
   double max_response_time=1000000;//1 second
   bool took_too_long =false;
   
   unsigned char line [3]="16"; 
   int socket = socket_to_server ("127.0.0.1", 77458);
        // The function expects an IP address, and not a 
        // hostname such as "localhost" or ecelinux1, etc.

    if (socket < 0)
    {
       error("ERROR opening socket");
	}
	char recv_buffer[1024]; 
	int n;        
  
	string msg = read_packet(socket);
	istringstream buf(msg);
	string r;
	buf >> r;
	string p;
	buf >> p;
	
   //now that we received the message, start the timer to make sure response is not too soon
   //start timer
   struct timeval startTV, endTV;
   gettimeofday(&startTV,NULL);
      
	
	cout<<"r:"<<r<<"\np:"<<p<<endl;

	char buffer[128];
	bzero(buffer,128);//clear to 0 all values in buffer
	string rand_string="";
	string hash_val="";
	double duration =0;
	
	FILE * file = fopen("/dev/urandom", "r");
	
	string r_decoded = cgipp::hex_decoded(r);  
	cout<<"r decoded:"<<r_decoded<<endl; 
	
	while(hash_val.substr(0,p.length()).compare(p)!=0){

		int val=fread(buffer,1,16,file);//read a number of bytes -128/8=16
		//sometimes the read returns too many bytes, so truncate them
		buffer[strlen(buffer)-(strlen(buffer)-16)]='\0';
		//cout<<"trying: "<<r_decoded+buffer+r_decoded<<endl; 
		hash_val=cgipp::sha256(r_decoded+buffer+r_decoded);
		//check that we haven't taken too long
		gettimeofday(&endTV,NULL);
		duration=(endTV.tv_sec*1000000+endTV.tv_usec)-(startTV.tv_sec*1000000+startTV.tv_usec);
		if(duration>max_response_time){
			took_too_long = true;
			break;
		}
	}
	if(!took_too_long){
		//cout<<"it works. rand string:"<<buffer<<endl;
		cout<<"hash val:"<<hash_val<<endl;
		
		rand_string = cgipp::hex_encoded((unsigned char *)buffer); 
	   
	   if(duration<min_response_time)
		  usleep((__useconds_t)(min_response_time-duration));
	   
		n = write(socket,(r+rand_string+r+"\n").c_str(),(r+rand_string+r+"\n").length());
		if (n < 0) error("ERROR writing to socket");
		
		bzero(recv_buffer,1024);//clear to 0 all values in buffer
		n = read(socket,recv_buffer,1024);
		if (n < 0) error("ERROR reading from socket");
		if (strlen(recv_buffer)>0)printf("Received message:%s\n",recv_buffer);
	}
	else{ cout<<"I couldn't compute the hash in time :("<<endl;}
	close(socket);


	return 0;
}

int socket_to_server (const char * IP, int port)
{
    struct sockaddr_in address;

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr (IP);
    address.sin_port = htons(port);

    int sock = socket (AF_INET, SOCK_STREAM, 0);

    if (connect (sock, (struct sockaddr *) &address, sizeof(address)) == -1)
    {
           
return -1;
    }

    return sock;
}

string read_packet (int client_socket)
{
    string msg;

    const int size = 8192;
    char buffer[size];

    while (true)
    {
        int bytes_read = recv (client_socket, buffer, sizeof(buffer) - 2, 0);
            // Though extremely unlikely in our setting --- connection from 
            // localhost, transmitting a small packet at a time --- this code 
            // takes care of fragmentation  (one packet arriving could have 
            // just one fragment of the transmitted message)

        if (bytes_read > 0)
        {
            buffer[bytes_read] = '\0';
            buffer[bytes_read + 1] = '\0';

            const char * packet = buffer;
            while (*packet != '\0')
            {
                msg += packet;
                packet += strlen(packet) + 1;

                if (msg.length() > 1 && msg[msg.length() - 1] == '\n')
                {
                    //istringstream buf(msg);
                    //string msg_token;
                    //buf >> msg_token;
                    //return msg_token;
                    return msg;
                }
            }
        }

        else if (bytes_read == 0)
        {
            close (client_socket);
            throw connection_closed();
        }

        else
        {
            cerr << "Error " << errno << endl;
            throw socket_error();
        }
    }

    throw connection_closed();
}
