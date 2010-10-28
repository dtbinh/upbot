#ifndef _NSM_H_
#define _NSM_H_

/**
* nsm.h
*
* This is the header file for our implementation of McCallum's
* Nearest Sequence Memory Q-Learning agent. It is largely based
* off of the Supervisor agent being written by Dr. Andrew Nuxoll,
* Zachary Paul Faltersack and Brian Burns.
*
* Author: Zachary Paul Faltersack
* Last edit: October 22, 2010
*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#include "vector.h"
#include "../communication/communication.h"

// Boolean values
#define TRUE                1
#define FALSE               0
#define SUCCESS             0

// Matching defines
#define NUM_TO_MATCH        15
#define NUM_GOALS_TO_FIND   50

// Defines for Q-Learning algorithm
#define DISCOUNT            0.9
#define LEARNING_RATE       0.1
#define REWARD_SUCCESS      1.0
#define REWARD_FAIL         0.3

// Defines for NSM
#define K_NEAREST           8

// Collecting data for stats
#define STATS_MODE          0

#define DECREASE_RANDOM(randChance) if((randChance) > 5) { (randChance) *= .9;}

// Sensor data struct
typedef struct EpisodeStruct
{
    int     now;
    int     sensors[NUM_SENSORS];   // Percept
    int     action;                 // Action
    double  reward;                 // Reward
    double  qValue;                 // Expected future discount reward
} Episode;

// Neighborhood struct for finding k-nearest neighbors
typedef struct NeighborhoodStruct
{
    int action;         // int to represent action associated with this neighborhood
    int kValue;         // k is the number of neighbors
    int numNeighbors;   // represents how many neighbors we have so far in the array
    Episode** episodes; // init to array of len k to hold ptrs to k Nearest Neighbors
    int* nValues;       // init to array of len k to hold ptrs to k NN scores
} Neighborhood;

// Global variables for monitoring and connecting
int g_connectToRoomba;
int g_statsMode;

// This vector will contain all episodes received from Roomba
Vector* g_epMem;
Vector* g_neighborhoods;

// Function declarations
extern int   tick(char* sensorInput);
Episode* updateHistory(char* sensorData);
int      parseSensors(Episode* parsedData, char* dataArr);
int      addEpisode(Vector* episodes, Episode* item);
void     displayEpisode(Episode* ep);
void     displayEpisodeShort(Episode* ep);
//-----------------------------------------
// Functions to add for McCallum's algorithm
void     updateAllLittleQ(Episode* ep);
int      setNewLittleQ(Episode* ep, double utililty);
int      populateNeighborhoods();
Neighborhood* locateKNearestNeighbors(int action);
int      calculateNValue(int currState);
double   calculateQValue(Neighborhood* nbHd);
//-----------------------------------------
// Functions for creating, maintaining and viewing neighborhoods
Neighborhood* initNeighborhood(int action, int k);
void destroyNeighborhood(Neighborhood* nbHd);
void cleanNeighborhood(Neighborhood* nbHd);
int addNeighbor(Neighborhood* nbHd, Episode* ep, int n);
void sortNeighborhood(Neighborhood* nbHd);
Episode* getNeighbor(Neighborhood* nbHd, int i);
void displayNeighborhood(Neighborhood* nbHd);
void displayNeighborSequence(Episode* ep, int n, int isCurr);
//-----------------------------------------
int      chooseCommand(Episode* ep);
int      setCommand(Episode* ep);
int      equalEpisodes(Episode* ep1, Episode* ep2);
void     initNSM();
void     endNSM();
extern char* interpretCommand(int cmd);
char*    interpretCommandShort(int cmd);
int      interpretSensorsShort(int *sensors);

#endif // _NSM_H_
