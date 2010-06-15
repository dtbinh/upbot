#include "supervisor.h"

/**
* This file contains the code for the Supervisor. All the functions
* that are needed for processing raw sensor data are contained in 
* this file as well as those for determining new commands
*
* Author: Dr. Andrew Nuxoll and Zachary Paul Faltersack
* Last Edit: June 7, 2010
*
*/

int g_randChance = 80;

/**
* tick
*
* This function is called at regular intervals and processes
* the recent sensor data to determine the next action to take.
*
* @param sensorInput a char string wth sensor data
* @return int : a command for the Roomba (negative is error)
*/
int tick(char* sensorInput)
{
	// Create new Episode
	Episode* ep = createEpisode(sensorInput);
	// Print out the parsed episode
	displayEpisode(ep);
	// Add new episode to the history
	addEpisode(g_episodeList, ep);
	// Send ep to receive a command and return the command that is chosen.
	// Will return -1 if no command could be set
	return chooseCommand(ep);
}// tick

/**
* createEpisode
*
* Takes a sensor data string and allocates space for episode
* then parses the data and populates the episode and adds it
* to the global episode list
*
* @arg sensorData char* filled with sensor information
* @return Episode* a pointer to the newly added episode
*/
Episode* createEpisode(char* sensorData)
{
	// Allocate space for episode and score
    Episode* ep = (Episode*) malloc(sizeof(Episode));
	int retVal;	

    // If error in parsing print appropriate error message and exit
    if((retVal = parseEpisode(ep, sensorData)) != 0)
    {
		char errBuf[1024];
		sprintf(errBuf, "Error in parsing: %s\n", sensorData);
		perror(errBuf);
		exit(retVal);
	}else
	{
#if STATS_MODE == 0
		printf("Sensor data successfully parsed into new episode\n");
#endif
	}
	return ep;
}// createEpisode

/**
* chooseCommand
*
* This function takes a pointer to a new episode and chooses a command
* that should accompany this episode
*
* @arg ep a pointer to the most recent episode
* @return int the command that was chosen
*
*/
int chooseCommand(Episode* ep)
{
	int random;						// int for storing random number
	int i, j;				// indices for loops
	
	// seed rand and allocate goal arr if this is first time a command is chosen
	static int needSeed = TRUE;
	if(needSeed == TRUE)
	{
		needSeed = FALSE;
		srand(time(NULL));
	}

	// Determine the next command, possibility of random command
	if((random = rand() % 100) < g_randChance || g_episodeList->size < NUM_TO_MATCH)
	{
		ep->cmd = rand() % NUM_COMMANDS;
	}else
	{
		// find the best match scores for the three commands
		// if no goal has been found (returns 0) then we take the command with the greatest score
		if(setCommand(ep) != 0)
		{
#if STATS_MODE == 0
			printf("Failed to set a Command");
#endif
			return -1;
		}
	}

	return ep->cmd;
}// chooseCommand

/**
* setCommand
*
* Find the match scores for each of the available commands (currently condensed list)
* If a goal has been found, then find the index of the best match
*
* @arg ep pointer to new episode
* @return int status code
*
* @return int status code
*/
int setCommand(Episode* ep)
{	
	static int goalCount = 0;						// Number of goals found so far
	static int goalIdx[NUM_GOALS_TO_FIND];
	int tempIdx, tempDist;							// temp vars
	int i,j;										// looping indices
	int bestMatch;
	int forwardScore, rightScore, leftScore;

	// init goadx to 0s
	if(g_episodeList->size == 1)
	{
		for(i = 0; i < NUM_GOALS_TO_FIND; i++)
		{
			goalIdx[i] = 0;
		}
	}

	// Allocate space for the score pointer
	int* score = (int*) malloc(sizeof(int));

	// init distance to size of history
	tempDist = g_episodeList->size;

	// If new episode is goal state store idx and increase count
	if(ep->sensors[SNSR_IR] == 1)
	{
		goalIdx[goalCount] = ep->now;
		goalCount++;
	}

	// Test out three commands and find best match for each command
	// Then if a goal has been found, determine the distance to the goal
	// and find command with the least distance
	for(i = 0; i < 3; i++)
	{
		// can only successfully search if at minimum history contains
		// NUM_TO_MATCH episodes
		if(g_episodeList->size > NUM_TO_MATCH)
		{
			// for each run test out next command
			// keep track of index of the best match as well as its score
			if(i == 0)
			{
				ep->cmd = CMD_FORWARD;
				tempIdx = match(g_episodeList, score);
				forwardScore = *score;
			}else if(i == 1)
			{
				ep->cmd = CMD_RIGHT;
				tempIdx = match(g_episodeList, score);
				rightScore = *score;
			}else
			{
				ep->cmd = CMD_LEFT;
				tempIdx = match(g_episodeList, score);
				leftScore = *score;
			}
			// If the goal has been found then determine which of the three episodes
			// with the greatest scores is closest to the goal
			if(goalCount > 0)
			{
				for(j = 0; j < goalCount; j++)
				{
					// Make sure the goal is after the current episode
					if(abs(((Episode*)getEntry(g_episodeList, goalIdx[j]))->now - 
								((Episode*)getEntry(g_episodeList, tempIdx))->now) < 0)
					{

					}
					// If the distance between the episode and goal is less than previous
					// then save it
					else if(abs(((Episode*)getEntry(g_episodeList, goalIdx[j]))->now - 
								((Episode*)getEntry(g_episodeList, tempIdx))->now) < tempDist)
					{
						// keep track of the current best distance
						tempDist = abs(((Episode*)getEntry(g_episodeList, goalIdx[j]))->now - 
								((Episode*)getEntry(g_episodeList, tempIdx))->now);
						// keep track of which command gave the best distance so far
						bestMatch = i;
					}// if
				}
			}// if
		}// if
	}// for

	// If a goal has been found then we have a distance we can use above to find best match
	if(goalCount > 0)
	{
		switch(bestMatch)
		{
			case 0:
				ep->cmd = CMD_FORWARD;
				break;
			case 1:
				ep->cmd = CMD_RIGHT;
				break;
			case 2:
				ep->cmd = CMD_LEFT;
				break;
		}
	}
	else
	{
		// If this is > instead of >= then the Supervisor prefers CMD_LEFT
		// If this is >= instead of > then the Supervisor prefers CMD_FORWARD
		// My assumption for the reason is that the majority of the time, the scores
		// are the same. I think this problem will go away when we start incorporating
		// the Milestones into the decision process
		if(forwardScore >= rightScore && forwardScore >= leftScore)
		{
			ep->cmd = CMD_FORWARD;
		}else if(rightScore >= leftScore)
		{
			ep->cmd = CMD_RIGHT;
		}else
		{
			ep->cmd = CMD_LEFT;
		}
	}

	printf("Best score Forward: %i\n"	, forwardScore);
	printf("Best score Right: %i\n"		, rightScore);
	printf("Best score Left: %i\n"		, leftScore);
	printf("Index of best match: %i\n"	, bestMatch);


	// free the memory allocated for score
	free(score);

	// return success
	return 0;
}// setCommand


/**
 * parseEpisode
 *
 *        dataArr contains string of the following format
 *        0000000011  <will be timestamp>  <abort signal>
 *
 * Take a raw sensor packet from Roomba and parse information
 * out to an instance of Episode.
 *
 * @arg parsedData A pointer to an Episode to be populated
 * @arg dataArr the char array that contains the raw sensor data
 * @return int an error code
 *
 */
int parseEpisode(Episode * parsedData, char* dataArr)
{
	// temporary timestamp
	static int timeStamp = 0;
	// Allocate space for an episode
	int i;
	char* tmp;
	int foundDigitCount = 0;

	if(dataArr == NULL)
	{
		printf("data arr in parse is null");
		return -1;
	}


	// set the episodes sensor values to the sensor data
	for(i = 0; i < NUM_SENSORS; i++)
	{
		// convert char to int and return error if not 0/1
		int bit = (dataArr[i] - '0');
		if ((bit < 0) || (bit > 1))
		{
			printf("%s", dataArr);
			return -1;	
		}

		// else save sensor bit
		parsedData->sensors[i] = bit;
	}

	if(parsedData->sensors[SNSR_IR] == 1)
	{
		DECREASE_RANDOM(g_randChance);
	}

	// Pull out the timestamp
	parsedData->now = timeStamp++;

	// set these to default values for now
	parsedData->cmd = CMD_NO_OP;

	return 0;
}// parseEpisode

/**
 * addEpisode
 *
 * Add new episode to episode history vector
 *
 * @arg episodes pointer to vector containing episodes
 * @arg item pointer to episode to be added
 * @return int status code (0 == success)
 */
int addEpisode(Vector* episodes, Episode* item)
{
	return addEntry(episodes, item);
}// addEpisode

/**
 * displayEpisode
 *
 * Display the contents of an episode
 *
 * @arg ep a pointer to an episode
 */
void displayEpisode(Episode * ep)
{
	int i;
	printf("\nSensors:    ");

	// iterate through sensor values and print to stdout
	for(i = 0; i < NUM_SENSORS; i++)
	{
		printf("%i", ep->sensors[i]);
	}

	// print rest of episode data to stdout
	printf("\nTime stamp: %i\nCommand:    %i\n\n", (int)ep->now, ep->cmd);
}// displayEpisode

/**
 * match
 *
 * search for a series of episodes that best matches the last NUM_TO_MATCH
 * episodes
 *
 * @arg vector the vector containing full history of episodes
 * @arg score a pointer to int we can use to store/return score
 * @return int index into the vector for the best matching series
 */
int match(Vector* vector, int* score)
{
	int i,j, tempScore = 0, returnIdx = 0;
	*score = 0;

	// Iterate through vector and search from each index
	for(i = vector->size - NUM_TO_MATCH - 1; i >= 0; i--)
	{
		// reset tempscore
		tempScore = 0;
		for(j = 0; j < NUM_TO_MATCH; j++)
		{
			// compare two episodes and add result to score
			tempScore += compare(vector->array[i + j], vector->array[vector->size - NUM_TO_MATCH + j]);

			// greatest possible score is 2 * NUM_SENSORS * NUM_TO_MATCH
			// give a goal episode half this value for 1/3 total value
			// also quit searching if at goal state
			if(((Episode*)(vector->array[i + j]))->sensors[SNSR_IR] == 1)
			{
				tempScore += (NUM_TO_MATCH * NUM_SENSORS);
				j = NUM_TO_MATCH;
			}
		}

		// If we ended up with a greater score than previous, store index and score
		if(tempScore > *score)
		{
			*score = tempScore;
			returnIdx = i;
		}
	}




	// The index of the -closest- match, was not necessarily a full milestone match
	return returnIdx;
}// match

/**
 * compare
 *
 * Compare the sensor arrays of two episodes and return if they match or not
 *
 * @arg ep1 a pointer to an episode
 * @arg ep2 a pointer to another episode
 * @return int The score telling us how close these episodes match
 */
int compare(Episode* ep1, Episode* ep2)
{
	int i, match = 0;

	// Iterate through the episodes' sensor data and determine if they are matching episodes
	for(i = 0; i < NUM_SENSORS; i++)
	{
		if(ep1->sensors[i] == ep2->sensors[i])
			match++;
	}

	// add num_sensors to give cmd 1/2 value
	if(ep1->cmd == ep2->cmd && ep1->cmd != CMD_NO_OP)
	{
		match += NUM_SENSORS;
	}

	// If we have a full match return 1 to add to score, else 0
	return match;
}// compare

/**
 * initSupervisor
 *
 * Initialize the Supervisor vectors
 * 
 */
void initSupervisor()
{
	g_episodeList = newVector();
	g_milestoneList = newVector();
}// initSupervisor

/**
 * endSupervisor
 *
 * Free the memory allocated for the Supervisor
 */
void endSupervisor()
{
	freeVector(g_episodeList);
	freeVector(g_milestoneList);
}// endSupervisor
