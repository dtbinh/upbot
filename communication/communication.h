#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include "../roomba/roomba.h"
#include "commandQueue.h"

#ifndef _COMMUNICATION_H
#define _COMMUNICATION_H
void *get_in_addr(struct sockaddr *sa);
int createListener(const char * name);
void sigchld_handler(int s);
void writeCommandToFile(char* cmd, FILE* fp);
int checkValue(char v);
int readSensorDataFromFile(char* data, FILE* fp);
int receiveDataAndStore(int newSock, char* cmdBuf, char* sensData, FILE* cmdFile, FILE* sensorFile, int* fd, caddr_t sensArea, caddr_t cmdArea);
int createSharedMem(char * deviceName, caddr_t* area);
int createServer(void);
int establishConnection(int s);
int readSensorDataFromSharedMemory(char* data, caddr_t shm);
int checkArgName(int argc, char* argv[], char addresses[3][13]);



//Human issued commands from the keyboard
#define ssDriveLow 'i'
#define ssDriveMed 'w'
#define ssDriveHigh 'W'
#define ssDriveBackwardLow 'k'
#define ssDriveBackwardMed 's'
#define ssDriveBackwardHigh 'S'
#define ssTurnCwise 'd'
#define ssTurnCCwise 'a'
#define ssStop 'x'
#define ssQuit 'q'
#define ssDriveDistance 'n'
#define ssAdjustLeft 'h'
#define ssAdjustRight 'j'
#define ssNoOp 'o'
#define ssBlinkLED 'l'
#define ssSong 'y'
#define NUM_TOTAL_CMDS 23  // The total number of commands issued to the iRobot (i.e. excluding ssQuit)

#define BACKLOG 10
#define MSG "And indeed there will be time\nTo wonder, 'Do I dare?' and, 'Do I dare?'\n"
#define PORT "8080"
#define MAXDATASIZE 150 // max number of bytes we can get at once 
//#define MAXDATASIZE 100 // max number of bytes we can get at once 
#define SEMAPHORE_OFFSET 0x400


// Command definitions
#define CMD_ILLEGAL			0x0
#define CMD_NO_OP 			0x1
#define CMD_FORWARD			0x2
#define CMD_LEFT			0x3
#define CMD_RIGHT			0x4
#define CMD_ADJUST_LEFT		0x5
#define CMD_ADJUST_RIGHT	0x6
#define CMD_SACC_1			0x7
#define CMD_SACC_2			0x8
#define CMD_SACC_3			0x9
#define CMD_SACC_4			0xA
#define CMD_SONG			0xB
#define CMD_BLINK			0xC


#define NUM_COMMANDS		0xD	// Always make sure this is at the end
                                // The NUM_COMMANDS is the total number
                                // of commands that can be issued by
                                // the artificially intelligent
                                // supervisor.

#define LAST_MOBILE_CMD		0xA  //last command that is not generally used for
                                 //human debugging
#define FIRST_SACC_CMD      0x7  //One or more saccade commands are
#define LAST_SACC_CMD       0xA  //internal commands used by saccFilt.c


// Sensor Data Indices
#define SNSR_IR			0x0
#define SNSR_CLIFF_RIGHT	0x1
#define SNSR_CLIFF_F_RIGHT	0x2
#define SNSR_CLIFF_F_LEFT	0x3
#define SNSR_CLIFF_LEFT		0x4
#define SNSR_CASTER		0x5
#define SNSR_DROP_LEFT		0x6
#define SNSR_DROP_RIGHT		0x7
#define SNSR_BUMP_LEFT		0x8
#define SNSR_BUMP_RIGHT		0x9
#define NUM_SENSORS		0xA	// Always make sure this is at the end

// WME defines: types
#define WME_INT 0x0
#define WME_CHAR 0x1
#define WME_DOUBLE 0x2
#define WME_STRING 0x3

// Moves for Eaters environment
#define CMD_MOVE_N          0x2
#define CMD_MOVE_S          0x3
#define CMD_MOVE_E          0x4
#define CMD_MOVE_W          0x5
#define CMD_EATERS_RESET    0x6

// Virtual objects in Eaters environment
#define V_E_WALL		(1)
#define V_E_AGENT		(2)
#define V_E_EMPTY		(0)
#define V_E_FOOD1		(5)
#define V_E_FOOD2		(10)

// Virtual objects in Roomba environment
#define V_R_HALLWAY	0
#define V_R_WALL		1
#define V_R_ROOMBA	2
#define V_R_GOAL		3
#define V_R_DROP		4

//Eaters configuration
#define EATERS_MAX_STEPS 2000   // number of steps per run
#define NUM_EATERS_RUNS  5      // number of runs before simulation stops
#define FIND_LAST_REWARD 0      // if set, this flag causes eaters agent to
                                // search only up until last reward when looking
                                // for a match



#endif
