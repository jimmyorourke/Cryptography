
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
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <ctime>
#include<time.h>
#include <string>
#include "cmor/crypto.h"
#include "cmor/encodings.h"
#include <fcntl.h>
void error(const char *msg)
{
    perror(msg);
    exit(1);
}

class connection_closed {};
class socket_error {};

void listen_connections (int port);
void process_connection (int client_socket);
string read_packet (int client_socket);

int main (int na, char * arg[])
{
    listen_connections (77458);

    return 0;
}

void listen_connections (int port)
{
    int server_socket, client_socket;
    struct sockaddr_in server_address, client_address;
    socklen_t client_len;

    server_socket = socket (AF_INET, SOCK_STREAM, 0);

    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);
    server_address.sin_port = htons (port);

    bind (server_socket, (struct sockaddr *) &server_address, sizeof(server_address));

    listen (server_socket, 5);
    while (true){
    
        client_len = sizeof(client_address);
        client_socket = accept (server_socket,
                                (struct sockaddr *) &client_address,
                                &client_len);
    
        pid_t pid = fork();
        if (pid == 0)           // if we're the child process
        {
            close (server_socket);    // only the parent listens for new connections

            if (fork() == 0)    // detach grandchild process -- parent returns immediately
            {
                usleep (10000); // Allow the parent to finish, so that the grandparent
                                // can continue listening for connections ASAP

                process_connection (client_socket);
            }

            return;
        }

        else if (pid > 0)       // parent process; close the socket and continue
        {
            int status = 0;
            waitpid (pid, &status, 0);
            close (client_socket);
        }

        else
        {
            cerr << "ERROR on fork()" << endl;
            return;
        }
        
    }
    
}

void process_connection (int client_socket)
{ 
    try
    {
		int n;
		int p_length = 2; //length of p in bytes;
		int r_length=16; //128bits/8=16
		double min_response_time=100000;//100 milliseconds
		char buffer[r_length];
		bzero(buffer,r_length);//clear to 0 all values in buffer

		FILE * file = fopen("/dev/urandom", "r");
		
		int val=fread(buffer,1,r_length,file);//read a number of bytes
		//sometimes the read returns too many bytes, so truncate them
		buffer[strlen(buffer)-(strlen(buffer)-r_length)]='\0';
		printf("r: %s\n",buffer);
		string r_string = cgipp::hex_encoded((unsigned char *)buffer);
		
		bzero(buffer,r_length);//clear to 0 all values in buffer
		
		val=fread(buffer,1,p_length,file);//read a number of bytes for p
		//sometimes the read returns too many bytes, so truncate them
		buffer[strlen(buffer)-(strlen(buffer)-p_length)]='\0';
		string p_string = cgipp::hex_encoded((unsigned char *)buffer);

		fclose(file);
		
		string message = r_string+" "+p_string+"\n";
		cout<<"sending message:"<<message<<endl;
		printf("message length: %d = %d +%d +2 (space and newline)\n",message.length(),r_string.length(),p_string.length());
			
		n = write(client_socket,message.c_str(),message.length());
		if (n < 0) error("ERROR writing to socket");
		//start timer
		struct timeval startTV, endTV, timeoutTV;
		timeoutTV.tv_sec=1;//1 sec timeout, max response time
		timeoutTV.tv_usec=0; 
		setsockopt(client_socket,SOL_SOCKET,SO_RCVTIMEO, (char*)&timeoutTV, sizeof(struct timeval));//apply timeout to socket
		gettimeofday(&startTV,NULL);
      
		string msg = read_packet(client_socket);
		//stop timer
		gettimeofday(&endTV,NULL);
		double duration=(endTV.tv_sec*1000000+endTV.tv_usec)-(startTV.tv_sec*1000000+startTV.tv_usec);
		cout<<"duration:"<<duration<<"usec"<<endl;
		if(duration>min_response_time){
			istringstream buf(msg);
			buf >> msg;
			cout<<"msg encoded:"<<msg<<endl;
			cout<<"r:"<<msg.substr(0,r_string.length())<<endl;
			cout<<"r end:"<<msg.substr(msg.length()-r_string.length(),r_string.length())<<endl;
			string msg_decoded=cgipp::hex_decoded(msg);
			cout<<"message received decoded: "<<msg_decoded<<endl;
			cout<<"hash:"<<cgipp::sha256(msg_decoded)<<endl;
			cout<<"length:"<<msg_decoded.length()<<endl;
			//check message begins and ends with r and hash begins with p and length is correct
			if((msg.substr(0,r_string.length())==r_string)&&(msg.substr(msg.length()-r_string.length(),r_string.length())==r_string)&&(msg_decoded.length()==(2*r_length+128/8)) && (cgipp::sha256(msg_decoded).substr(0,p_string.length()).compare(p_string)==0)){	
				cout<<"response verified: correct!"<<endl;
				n = write(client_socket,"Ok\n",strlen("Ok\n"));
				if (n < 0) error("ERROR writing to socket");
			}
			else{ 
				cout<<"Response Incorrect!!"<<endl;  
			}
		} 
		else cout<<"Response was too quick!!"<<endl; 
			close (client_socket);
		
    }
    catch (connection_closed)
    {
    }
    catch (socket_error)
    {
        cerr << "Socket error" << endl;
    }
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
            cout<<"Response took too long!"<<endl;
            close (client_socket);
            //cerr << "Error " << errno << endl;
            throw connection_closed();
        }
    }

    throw connection_closed();
}
