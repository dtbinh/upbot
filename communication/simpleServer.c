/*
 * simpleServer.c -- a stream socket server demo
 *
 * Description: This server program awaits connections on port PORT.
 * Upon connection, it delivers a brief snippet from "The Love Song
 * of J. Alfred Prufrock."
 *
 * This simple program is largely based on Beej's Socket Programming
 * Tutorial.  Thanks Brian Hall!
 *
 * NOTE: Though not part of the iRobot testbed functionality, it is
 * included in the repository as a nice working example of a simple
 * socket server.
 * 
 */

// Header files
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
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


// Constant definitions
#define PORT "8080"       // Source port
#define BACKLOG 10        // The number of allowed backlog requests to come into the server

#define BIND_ERROR (-2)   // Possible error value from createListener()
#define SOCKET_ERROR (-1) // Possible error value from createListener()

#define MSG "And indeed there will be time\nTo wonder, 'Do I dare?' and, 'Do I dare?'\n"


// Function prototypes.
int createListener(const char * name);
void *get_in_addr(struct sockaddr *sa);
void sigchld_handler(int s);



// main() entrypoint of the program.
int main(void)
{

  // connector's address information
  struct sockaddr_storage theirAddr; 
  

  socklen_t size;
  struct sigaction sa;
  char p[INET6_ADDRSTRLEN];

  int s;
  int newSock;

  // Create a socket to listen on the port.  
  s = createListener(PORT);


  // Handle any problems raised by createListener().
  if(s == BIND_ERROR)
    {
      perror("bind");
      exit(1);
    }

  else if(s == SOCKET_ERROR)
    {
      perror("socket");
    }


  // Listen to the socket, wait for incoming requests.  Allow 
  // a BACKLOG of requests to come in.
  if (listen(s, BACKLOG) == -1)
    {
      perror("listen");
      exit(1);
    }


  // Set up the child reaper, a sig-action handler.
  sa.sa_handler = sigchld_handler; 
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_RESTART;

  
  if (sigaction(SIGCHLD, &sa, NULL) == -1) 
    {
      perror("sigaction");
      exit(1);
    }

  printf("server: waiting for connections...\n");

  // Wait forever to get connections from clients.  Yes, to quit this program
  // one must use Ctrl+C.
  while(1)
    {
      size = sizeof(theirAddr);

      // A new connection coming in. 
      newSock = accept(s, (struct sockaddr *)&theirAddr, &size);

      if (newSock == -1)
	{
	  perror("accept");
	  continue;
	}

      // Convert the IP address to a textual IP address that
      // can be printed to the console. 
      inet_ntop(theirAddr.ss_family, get_in_addr((struct sockaddr *)&theirAddr), p, sizeof(p));
      

      // Print the client's IP address to the console.
      printf("server: got connection from %s.\n", p);


      // Make a child process to handle the request.  The server will continue
      // to listen on s for new connections while the child will deliver the message
      // unto the client. 
      if(!fork()) 
	{
	  // The child process does not need the original socket.  The parent
	  // process will continue to use it to listen for new connections.
	  close(s);   

	  // Send the message to the client.
	  if(send(newSock, MSG, sizeof(MSG), 0) == -1)
	    perror("send");

	  // All done.  Close the socket used to send the message
	  // to the client.
	  close(newSock);

	  // The child function is done.
	  exit(0);
	     
	}

      // The parent process doesn't need the new socket created to
      // handle request.
      close(newSock);  
   
    }

  return EXIT_SUCCESS;  

}


//
// Function: get_in_addr()
// Description: Get an IPv4 or IPv6 socket address
// 
void *get_in_addr(struct sockaddr *sa)
{
  if (sa->sa_family == AF_INET) {
    return &(((struct sockaddr_in*)sa)->sin_addr);
  }

  return &(((struct sockaddr_in6*)sa)->sin6_addr);
}


// serv_listen()
//   Create a server endpoint of communication.
//   Adapted from: "Advance Programming in the UNIX Environment."  page 501
//   as well as "Beej's Guide to Network Programming."
int createListener(const char * name)
{
  int status;
  int s;
  int yes = 1;
 
  /* Defining the fields for socket structs is challenging.  The
   * fields depend of the address, type of socket, and communication
   * protocol.  This function uses getaddrinfo() to aid in defining
   * the struct socket fields.  This function fills a struct of type
   * addrinfo.
   */
  struct addrinfo hints;
  struct addrinfo * servinfo, *p; 
  
  // Initialize the hints structure based on what little we care about
  // in terms of the socket.  The goal is to listen in on host's IP
  // address on port 22.
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;        // don't care if its IPv4 or IPv6.
  hints.ai_socktype = SOCK_STREAM;    // stream-style sockets.
  hints.ai_flags = AI_PASSIVE;        // fill in my IP automatically.

  if((status = getaddrinfo(NULL, name, &hints, &servinfo)) != 0)
    {
      return -1;
    }

  // Servinfo now points to a linked list of 1 or more struct
  // addrinfos.  Note that they may not all be valid.  Scan through
  // the servinfo until something makes sense.
  for(p = servinfo; p != NULL; p = p->ai_next) {
    if ((s = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
      perror("server: socket");
      continue;
    }
    
    if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
      perror("setsockopt");
      exit(1);
    }
    
    if (bind(s, p->ai_addr, p->ai_addrlen) == -1) {
      close(s);
      perror("server: bind");
      continue;
    }
    
    break;

  }

  if (p == NULL)  {
    fprintf(stderr, "server: failed to bind\n");
    return -1;
  }

  // Don't need servinfo anymore
  freeaddrinfo(servinfo);

  return s;
}


// sigchld_handler()
//  This function is an interrupt-driven function which 
//  reaps dead children that are forked by the server.
void sigchld_handler(int s)
{
  while(waitpid(-1, NULL, WNOHANG) > 0);
}

