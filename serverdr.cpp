/*
** server.c -- a stream socket server demo
*/
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/time.h>//for getting the time
#include <iostream>
#define PORT "3490" // the port users will be connecting to
#define BACKLOG 10 // how many pending connections queue will hold
#define MAXBUFLEN 32*1024
using namespace std;

int count_of_loops = 0;

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
    {
         if (sa->sa_family == AF_INET) {
         return &(((struct sockaddr_in*)sa)->sin_addr);
         }
         return &(((struct sockaddr_in6*)sa)->sin6_addr);
    }

int main(void)
    {
         int sockfd, new_fd; // listen on sock_fd, new connection on new_fd
         struct addrinfo hints, *servinfo, *p;
         int numbytes, numbytes1;
         char buf[MAXBUFLEN];
         //struct sockaddr_in *port;
         struct sockaddr_storage their_addr; // connector's address information
         socklen_t sin_size;
         struct sigaction sa;
         int yes=1;
         char s[INET6_ADDRSTRLEN];
         int rv;
         memset(&hints, 0, sizeof hints);
         hints.ai_family = AF_UNSPEC;   //we can use either ipv6 or ipv4
         hints.ai_socktype = SOCK_STREAM;
         hints.ai_flags = AI_PASSIVE; // use my IP

        //fill in the struct addrinfo with all the info about the server side
         if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
         fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
         return 1;
         }

    // loop through all the results and bind to the first we can
         for(p = servinfo; p != NULL; p = p->ai_next) {
         if ((sockfd = socket(p->ai_family, p->ai_socktype,
         p->ai_protocol)) == -1) {
         perror("server: socket");
         continue;
         }

         if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
         sizeof(int)) == -1) {
         perror("setsockopt");
         exit(1);
         }
         if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
         close(sockfd);
         perror("server: bind");
         continue;
         }
         break;
         }

        freeaddrinfo(servinfo);        // all done with this structure

         if (p == NULL) 
         {
         fprintf(stderr, "server: failed to bind\n");
         return 2;
         }

         if (listen(sockfd, BACKLOG) == -1) 
         {
         perror("listen");
         exit(1);
         }

         //printf("server: waiting for connections on port number %d \n", &PORT);
         printf("server: waiting for connections... \n\n");

         while(1) // main accept() loop
         { 
         printf("waiting to receive data from the client side....\n"); 

         sin_size = sizeof their_addr;
         new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);

         if (new_fd == -1) 
         {
         perror("accept");
         continue;
         }

         inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), 
         s, sizeof s);

        printf("server: got connection from %s\n\n", s);

         if (!fork()) 
         { // this is the child process
         //close(sockfd); // child doesn't need the listener

         //receive the data from client into a buffer
         if ((numbytes = recv(new_fd, buf, MAXBUFLEN-1 , 0)) == -1) {
         perror("recv");
         exit(1);
         }

         //get the time2 into a varaiable
         struct timeval tp2;
         gettimeofday(&tp2, NULL);
         long int time2 = tp2.tv_sec * 1000 + tp2.tv_usec / 1000; //get current timestamp in milliseconds
         cout << "received time: "<<time2 << endl;

         //print the adress of the client 
         printf("server got packet from %s\n", inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), s, sizeof s));
         //print the number of bytes received and print the bytes.
         printf("server: received packet is %d bytes long\n", numbytes);
         buf[numbytes] = '\0';
         //printf("client: packet contains \"%s\"\n", buf);

         //capitalizing the string sent by client side
         int i = 0;
         char upper[numbytes];
         while (buf[i])
          {
            upper[i] = toupper(buf[i]);
            //printf("%c", upper[i]);
            i++;
          }
          upper[numbytes]='\0';

          //Add some delay here according to the desired data rate.
          //usleep( 2000 * 1000 );

        //sending the capitalized string to the client
         if ((numbytes1 = send(new_fd, upper, strlen(upper), 0)) == -1) {
         perror("server: send");
         exit(1);
         }
         printf("server: sent packet is %d bytes long\n", numbytes1);
         //get the time4 into a varaiable
         struct timeval tp4;
         gettimeofday(&tp4, NULL);
         long int time4 = tp4.tv_sec * 1000 + tp4.tv_usec / 1000; //get current timestamp in milliseconds
         cout << "sent time: "<<time4 << endl;

         printf("The string is capitalized and sent to the client at %s \n",  inet_ntop(their_addr.ss_family,
         get_in_addr((struct sockaddr *)&their_addr),
         s, sizeof s) );

         close(new_fd); // parent doesn't need this
         cout<<"**DONE**"<<endl;
         }
         count_of_loops++;
         cout<<"\n number of sends and receives is: "<<count_of_loops<<endl;
        }
         close(sockfd);
         return 0;
    }