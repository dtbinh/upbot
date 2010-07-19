#include "supervisor.h"

/**
* This file contains the code for the Supervisor. All the functions
* that are needed for processing raw sensor data are contained in 
* this file as well as those for determining new commands
*
* Author: Dr. Andrew Nuxoll and Zachary Paul Faltersack
* Last Edit: July 5, 2010
*
*/

// The chance of choosing a random move
int g_randChance = 80;
// global strings for printing to console
char* g_forward = "forward";
char* g_right	= "right";
char* g_left	= "left";
char* g_adjustR	= "adjust right";
char* g_adjustL	= "adjust left";
char* g_blink	= "blink";
char* g_no_op	= "no operation";
char* g_song	= "song";
char* g_unknown	= "unknown";

char* g_forwardS = "FW";
char* g_rightS	 = "RT";
char* g_leftS	 = "LT";
char* g_adjustRS = "AR";
char* g_adjustLS = "AL";
char* g_blinkS	 = "BL";
char* g_no_opS	 = "NO";
char* g_songS	 = "SO";
char* g_unknownS = "$$";

// Keep track of goals
int g_goalCount = 0;						// Number of goals found so far
int g_goalIdx[NUM_GOALS_TO_FIND];




/**
* tick
*
* This function is called at regular intervals and processes
* the recent sensor data to determine the next action to take.
*
* @param sensorInput a char string wth sensor data
* @return int a command for the Roomba (negative is error)
*/
int tick(char* sensorInput)
{
	// Create new Episode
	Episode* ep = createEpisode(sensorInput);
	// Add new episode to the history
	addEpisode(g_episodeList, ep);
	updateRules();
	// Send ep to receive a command
	// Will return -1 if no command could be set
	chooseCommand(ep);
	displayRules();

	// Print out the parsed episode if not in statsMode
	if(g_statsMode == 0)
	{
		displayEpisode(ep);
	}
	return ep->cmd;
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
		if(g_statsMode == 0)
		{
			printf("Sensor data successfully parsed into new episode\n");
		}
	}
	return ep;
}// createEpisode

/**
 * parseEpisode
 *
 *        dataArr contains string of the following format
 *        0000000011  <will be timestamp> 
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
	int i; // index

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

	if(g_connectToRoomba == 1)
	{
		// Pull out the timestamp
		parsedData->now = timeStamp++;
	}else
	{
		// Alg for determining timestamp from string of chars
		int time = 0;
		for(i = NUM_SENSORS; dataArr[i] != '\0'; i++)
		{
			if(dataArr[i] != ' ')
			{
				time = time * 10 + (dataArr[i] - '0');
			}
			if(dataArr[i] == ' ' && time != 0)
			{
				break;
			}
		}
		// Store the time
		parsedData->now = time;
	}

	// Found a goal so decrease chance of random move
	if(parsedData->sensors[SNSR_IR] == 1)
	{
		DECREASE_RANDOM(g_randChance);
		g_goalIdx[g_goalCount] = parsedData->now;
		g_goalCount++;
	}

	// Command gets a default value for now
	parsedData->cmd = CMD_NO_OP;

	return 0;
}// parseEpisode

/**
* updateRules
*/
int updateRules()
{
	// If episodeList has 1 or less episodes or the most recent complete episode was a goal
	// then no rule should be made
	if(g_episodeList->size <= 1 || ((Episode*)g_episodeList->array[g_episodeList->size - 2])->sensors[SNSR_IR])
	{
		return -1;
	}

	//Create a candidate rule that we would create from this current episode.  We won't add it to the rule 
	//list if an identical rule already exists.
	Rule* newRule = (Rule*) malloc(sizeof(Rule));
	newRule->outcome			= g_episodeList->size - 1;
	newRule->index 				= g_episodeList->size - 2;
	newRule->length 			= 1;
	newRule->freq				= 1;
	newRule->overallFreq 		= NULL;
	newRule->cousins			= NULL;
	newRule->isPercentageRule 	= FALSE;
	newRule->isSound 			= FALSE;

    printf("candidate rule: ");
    displayRule(newRule);
    printf("\n");
    fflush(stdout);
    
	//Iterate over every rule in the list and compare it to our new candidate rule.
	//If the candidate is unique, it'll be added to the rule list.
	//If it's a partial match (same LHS , different RHS) but can't be made unique without increasing 
	//size of LHS then create pool of percentage rules
	//If the candidate matches an existing rule, it'll be discarded and the existing rule will be updated
	int i,j;
	int matchComplete = FALSE;
	int addNewRule = TRUE;
	for(i = 0; i < g_ruleList->size; i++)
	{
		//Compare the i-th rule to the candidate rule
		Rule* curr = (Rule*)g_ruleList->array[i];

/*         printf("begin comparison to rule #%i: ", i); */
/*         displayRule(curr); */
/*         printf("\n"); */
        
		for(j = 0; j < newRule->length; j++)
		{
			//Compare the j-th episodes in the rules
			if(2 * NUM_SENSORS == compare(g_episodeList->array[newRule->index - j], g_episodeList->array[curr->index - j], FALSE))
			{
                printf("found match between %i-th episodes of: ", j);
                displayRule(curr);
                printf(" AND ");
                displayRule(newRule);
                printf("\n");
                fflush(stdout);

                //If they match so far but we haven't reached the end
                //of either rule then continue comparing them
                if (newRule->length > j+1 && curr->length > j+1)
                {
                    continue;
                }
                
				//If the episodes match then see if their LHS are the same length. If they are, then the LHS's match.
				if(newRule->length == curr->length)
				{
					if(curr->isPercentageRule)
					{
                printf("comparing cousins: \n");
						int k;
						//Iterate over cousins and find one with same outcome as candidate rule
						for(k = 0; k < curr->cousins->size; k++)
						{
							Rule* cousin = curr->cousins->array[k];

                printf("\t");
                displayRule(cousin);
                printf(" AND ");
                displayRule(newRule);
                printf("\n");
                fflush(stdout);

							if(NUM_SENSORS == compare(g_episodeList->array[newRule->outcome], g_episodeList->array[cousin->outcome], TRUE))
							{
								curr->freq++;
								addNewRule = FALSE;
                                break;
							}
						}

						//No cousins match candidate rule, so add it as a new cousin
						if(addNewRule)
						{
                            printf("new cousin is unique.  Adding...\n");
							newRule->isPercentageRule = TRUE;
							newRule->overallFreq = curr->overallFreq;
							newRule->cousins = curr->cousins;

                            addRule(newRule->cousins, newRule, FALSE);
						}

						// Regardless of whether candidate rule is unique, increase overall frequency for all cousins in list
						(*(curr->overallFreq))++;
                        matchComplete = TRUE;
					}
					else	//Not a percentage rule
					{
						//Now see if the RHS of both rules match
						if(NUM_SENSORS == compare(g_episodeList->array[newRule->outcome], g_episodeList->array[curr->outcome], TRUE))
						{
							//We have a complete match between the candidate and an existing rule, so just update the existing rule
							curr->freq++;
							curr->isSound = TRUE;
							newRule->isSound = TRUE;

                            //Done with update
							matchComplete = TRUE;
							addNewRule = FALSE;
						}
						else	//RHS does not match
						{
                            printf("LHS match but RHS doesn't while comparing to %i...\n", i);
                            fflush(stdout);
							// We want to expand the newRule and curr
							// to create (hopefully) distinct rules
							// There are 3 reasons this may not work.
                            
							// 1. Current rule is already maximum length
							// 2. Expanding current rule would
							//    overflow episodic memory
							// 3. Expanding curr/newRule would include
							//    a goal on LHS
							if(((Episode*)g_episodeList->array[newRule->index - (newRule->length + 1)])->sensors[SNSR_IR] == TRUE)
							{
                                printf("NewRule expands into goal: %i\n",
                                       ((Episode*)g_episodeList->array[newRule->index - (newRule->length + 1)])->sensors[SNSR_IR]);
                                fflush(stdout);
                                
                                //the new rule can't be expanded so we
                                //consider it degenerate so just abort
                                //and create no new rules or updates
                                matchComplete = TRUE;
                                addNewRule = FALSE;
                            }
							else if(curr->index - curr->length <= 0 || 
							   ((Episode*)g_episodeList->array[curr->index - (curr->length + 1)])->sensors[SNSR_IR] == TRUE)
							{
                                printf("avail space: %i,  curr expands into goal\n",
                                       curr->index - curr->length);
                                fflush(stdout);
                                //The current rule can't be expanded
                                //so we consider it degenerate and
                                //replace it with the new rule.
                                curr->index 				= newRule->index;
                                curr->outcome               = newRule->outcome;
                                curr->length 			    = newRule->length;
                                curr->freq				    = 1;
                                curr->isSound 			    = FALSE;

                                //done with update
                                matchComplete = TRUE;
                                addNewRule = FALSE;
                            }
                            else if (newRule->length < curr->length)
                            {
                                //if the newRule is currently shorter
                                //than the current rule, then only it
                                //should be expanded.
                                newRule->length++;
                                
                                printf("partial match with curr, extending new rule to %i\n",
                                       newRule->length);
                                fflush(stdout);
                                printf("new candidate: ");
                                displayRule(newRule);
                                printf("\n");
                                fflush(stdout);
                            }
                            else if(curr->length < MAX_LEN_LHS)
							{
                                printf("len of curr rule (%i) = %i < 4 so increasing to %i\n",
                                       i, curr->length, curr->length+1);
                                fflush(stdout);
                                
                                //both current rule and new rule can
                                //be expanded so do so in hopes that they will
                                //end up different
								curr->length++;
                                curr->freq = 1;
								newRule->length++;

                                
                                printf("new %i:   ");
                                displayRule(curr);
                                printf("\n");
                                printf("new cand: ");
                                displayRule(newRule);
                                printf("\n");
                                fflush(stdout);

							}
							else
							{
                                printf("cousins\n");
                                fflush(stdout);
                                
								// We reached the end of our search and will now convert both the current rule
								// and the candidate rule into percentage rules

								// allocate cousins and add both peer percentage rules into same cousins list
								curr->cousins = newVector();
								addRule(curr->cousins, curr, TRUE);
								addRule(curr->cousins, newRule, TRUE);
								newRule->cousins = curr->cousins;

								//Update rules
								curr->isPercentageRule = TRUE;
								newRule->isPercentageRule = TRUE;

								curr->overallFreq = (int*) malloc(sizeof(int));
								newRule->overallFreq = curr->overallFreq;

								*(curr->overallFreq) = curr->freq + 1;

								matchComplete = TRUE;
								addNewRule = TRUE;
							}// else
						}// else
					}// else
				}// if
				else  
				{
                    //If we make it here, the candidate rule and
                    //current rule are different lengths but they do
                    //match up to the length of the shorter rule.

                    //If the candidate is longer then consider it a
                    //degenerate rule and stop
                    if (newRule->length > curr->length)
                    {
                        matchComplete = TRUE;
                        addNewRule = FALSE;
                        printf("newRule matches but is bigger than current rule.  Aborting.\n");
                        fflush(stdout);
                    }

                    //If the new rule can be expanded, try doing so to
					//see if that makes it unique
					else if(newRule->length < MAX_LEN_LHS)
					{
						newRule->length++;
                        
                        printf("expanded new rule to len %i\n",
                               newRule->length);
                        printf("new candidate: ");
                        displayRule(newRule);
                        printf("\n");
                        fflush(stdout);
					}
					else
					{
                        //Should never happen 
						printf("Nux was wrong\n");
                        fflush(stdout);
					}
				}// else
			}// if
            else
            {
                //The current rule's nth entry doesn't match so we can
                //abort the comparison even if the candidate rule has
                //more entries in its LHS
                break;
            }
		}// for

		// if matched rule break out of loop and free memory 
		if(matchComplete == TRUE)
		{
			break;
		}
	}// for

	if(addNewRule == TRUE)
	{
		addRule(g_ruleList, newRule, FALSE);
	}
	else
	{
		free(newRule);
	}

	return 0;
}// updateRules

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
 * addRule
 */
int addRule(Vector* rules, Rule* item, int checkRedundant)
{
	if(checkRedundant && rules->size > 0)
	{
		int i;
		for(i = 0; i < rules->size - 1; i++)
		{
			if(rules->array[i] == item)
			{
				return -1;
			}
		}
	}

	return addEntry(rules, item);
}// addrule

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

void displayRules()
{
	int i;
	for(i = 0; i < g_ruleList->size; i++)
	{
		printf("%3i. ", i);
		displayRule(g_ruleList->array[i]);
		printf("\n");
	}

    //print the last 20 episodic memories
    printf("EpMem: ");
    for(i = 1; i <= 20; i++)
    {
        if (i > g_episodeList->size) break;
        
        Episode *ep = (Episode*)g_episodeList->array[g_episodeList->size - i];
        printf("%i %s, ", interpretSensorsShort(ep->sensors), interpretCommandShort(ep->cmd));
    }//for
    printf("\n");
	printf("---------------\n");
}// displayRules

void displayRule(Rule* rule)
{
	int i,j;
    printf("%i", interpretSensorsShort(((Episode*)g_episodeList->array[rule->outcome])->sensors));

	if(rule->isPercentageRule)
	{
		printf(" <--%2i-- ", rule->freq * 100 / *(rule->overallFreq));
	}
	else
	{
		printf(" <------ ");
	}

	for(i = 0; i < rule->length; i++)
	{
        printf("%i ", interpretSensorsShort(((Episode*)g_episodeList->array[rule->index - i])->sensors));	
		printf("%s, ", interpretCommandShort(((Episode*)g_episodeList->array[rule->index - i])->cmd));
    }


}// displayRule

/**
 * chooseCommand
 *
 * This function takes a pointer to a new episode and chooses a command
 * that should accompany this episode
 *
 * @arg ep a pointer to the most recent episode
 * @return int the command that was chosen
 */
int chooseCommand(Episode* ep)
{
	int i, j;	// indices for loops

	// seed rand if first tiem called
	static int needSeed = TRUE;
	if(needSeed == TRUE)
	{
		needSeed = FALSE;
		srand(time(NULL));
	}

	// Determine the next command, possibility of random command
	if((rand() % 100) < g_randChance || g_episodeList->size < NUM_TO_MATCH)
	{
		// Command 0 is now illegal command so adjust NUM_COMMANDS to account for this
		// Then increment to push back into valid command range
		ep->cmd = (rand() % LAST_MOBILE_CMD) + CMD_NO_OP;
	}else
	{
		// find the best match scores for the three commands
		// if no goal has been found (returns 0) then we take the command with the greatest score
		if(setCommand(ep) != 0)
		{
			if(g_statsMode == 0)
			{
				printf("Failed to set a Command");
			}
			return -1;
		}
	}
	//	printf("COMMAND TO BE SENT %s (%i)\n", interpretCommand(ep->cmd), ep->cmd);


	return ep->cmd;
}// chooseCommand

/**
 * setCommand
 *
 * Find the match scores for each of the available commands (currently condensed list)
 * If a goal has been found, then find the index of the best match (closest to subsequent goal)
 *
 * @arg ep pointer to new episode
 * @return int status code
 */
int setCommand(Episode* ep)
{	
	int tempIdx, tempDist;							// temp vars
	int i,j,k;										// looping indices
	int bestMatch = CMD_NO_OP;
	double commandScores[NUM_COMMANDS];				// Array to store scores for commands
	int commandIdxs[NUM_COMMANDS];
	int toggle = 0;
	double partialScoreTable[g_episodeList->size];

	// initialize scores to 0
	for(i = 0; i < NUM_COMMANDS; i++)
	{
		commandScores[i] = 0;
		commandIdxs[i] = 0;
	}

	// generate partial score table
	generateScoreTable(g_episodeList, partialScoreTable);

	// determine index with top match per command
	for(i = CMD_NO_OP; i <= LAST_MOBILE_CMD; i++)
	{
		commandIdxs[i] = findTopMatch(partialScoreTable, commandScores, i);
	}

	// can only successfully search if at minimum history contains
	// NUM_TO_MATCH episodes
	if(g_goalCount > 0)
	{
		// Set distance to goal equal to largest possible distance
		tempDist = g_episodeList->size;
		// Test out the available commands
		for(i = CMD_NO_OP; i <= LAST_MOBILE_CMD; i++)
		{
			// find index closest to a subsequent goal
			tempIdx = commandIdxs[i];

			// If the goal has been found then determine which of the three episodes
			// with the greatest scores is closest to the goal

			for(j = 0; j < g_goalCount; j++)
			{
				/*						printf("dist: %i\n", g_goalIdx[j] - tempIdx);
										printf("idx1: %i idx2: %i\n", g_goalIdx[j], tempIdx);
				 */

				// Make sure distance is greater than 0 and 
				// If the distance between the episode and goal is less than previous
				// then save it
				if(g_goalIdx[i] - tempIdx > 0 && g_goalIdx[j] - tempIdx < tempDist)
				{
					// keep track of the current best distance
					tempDist = g_goalIdx[j] - tempIdx;
					// keep track of which command gave the best distance so far
					bestMatch = i;
					toggle = 1;
					//							printf("Setting toggle, cmd: %s, tempDist: %i\n", interpretCommand(i), tempDist);
				}// if
			}// for
		}// for 
	}// if


	// If a goal has been found then we have an index of closest goal match
	if(g_goalCount > 0 && toggle == 1)
	{
		ep->cmd = bestMatch;
	}
	else
	{ 
		// else we find cmd with greatest score
		int max = CMD_NO_OP;
		for(i = CMD_NO_OP; i <= LAST_MOBILE_CMD; i++)
		{
			//Debug lines
			/*			double maxScore = NUM_TO_MATCH * NUM_SENSORS * 2;
						printf("Max score: %g out of: %g for command: %s\n", commandScores[i], maxScore, interpretCommand(i)); */
			if(commandScores[max] < commandScores[i])
			{
				max = i;
			}
		}
		ep->cmd = max;
	}

	// If not stats mode print scores for cmds
	if(g_statsMode == 0)
	{
		for(i = CMD_NO_OP; i < NUM_COMMANDS; i++)
		{
			printf("%s score: %f\n", interpretCommand(i), commandScores[i]);
		}
	}
	/*
	   if(toggle != 1)
	   {
	   printf("Still have not found valid bestMatch, cmd: %s\n", interpretCommand(ep->cmd));
	   }
	   else
	   {
	   printf("Found valid bestMatch, cmd: %s\n", interpretCommand(ep->cmd));
	   }
	 */


	// Report malformed episode and location of error report
	if(ep->cmd <= CMD_ILLEGAL || ep->cmd >= NUM_COMMANDS)
	{
		printf("Episode is bad: setCommand %s (%i)\n", interpretCommand(ep->cmd), ep->cmd);
	}

	// return success
	return 0;
}// setCommand


/**
 * findTopMatch
 */
int findTopMatch(double* scoreTable, double* indvScore, int command)
{
	int i, max;
	double maxVal = 0.0, tempVal = 0.0;
	for(i = g_episodeList->size - 1; i >= 0; i--)
	{
		tempVal = scoreTable[i] + (((Episode*)(g_episodeList->array[i]))->cmd == command ? NUM_TO_MATCH : 0);

		if(tempVal > maxVal)
		{
			max = i;
			maxVal = tempVal;
		}
	}

	indvScore[command] = maxVal;

	return max;
}// findTopMatch


/**
 * match
 *
 * search for a series of episodes that best matches the last NUM_TO_MATCH
 * episodes
 *
 * @arg vector the vector containing full history of episodes
 * @arg score a pointer to double we can use to store/return score
 * @arg topIdxArr pointer to array of 3 ints to store top 3 matching indices
 * @return int Error code
 */
int generateScoreTable(Vector* vector, double* score)
{
	int i,j, returnIdx = 0;
	double tempScore = 0.0, discount = 1.0;
	int start = g_episodeList->size - NUM_TO_MATCH - 1; // subtract 1 to begin 1 episode before curr state

	if(g_goalCount > 0)
	{
		start = g_goalIdx[g_goalCount - 1] - NUM_TO_MATCH;
	}

	// Iterate through vector and search from each index
	for(i = start; i >= 0; i--)
	{
		// reset tempscore
		tempScore = 0;

		for(j = NUM_TO_MATCH - 1; j >= 0; j--)
		{
			// If any state that occurs before the final episode in the match is a goal
			// state then break because that means our current final episode is being 
			// matched to one after a goal, which doesn't help to find the goal again
			if(((Episode*)(vector->array[i + j]))->sensors[SNSR_IR] == 1)
			{
				break;
			}

			// compare two episodes and add result to score with appropriate discount
			//					V-index iterate to beginning		V-index to current episode frame
			tempScore += (discount * compare(vector->array[i + j], vector->array[vector->size - NUM_TO_MATCH + j], (j == NUM_TO_MATCH - 1 ? TRUE : FALSE)));
			discount *= DISCOUNT;
		}//for

		score[i + NUM_TO_MATCH - 1] = tempScore;
	}// for
	// return success
	return 0;
}// match

/**
 * compare
 *
 * Compare the sensor arrays of two episodes and return if they match or not
 *
 * @arg ep1 a pointer to an episode
 * @arg ep2 a pointer to another episode
 * @return double The score telling us how close these episodes match
 */
double compare(Episode* ep1, Episode* ep2, int isCurrMatch)
{
	int i;
	double match = 0;

	// Iterate through the episodes' sensor data and determine if they are matching episodes
	for(i = 0; i < NUM_SENSORS; i++)
	{
		if(ep1->sensors[i] == ep2->sensors[i])
		{
			match++;
		}
	}

	// Only add the value for command match if not the last episode in state
	if(isCurrMatch == FALSE)
	{
		// add num_sensors to give cmd 1/2 value
		if(ep1->cmd == ep2->cmd)
		{
			match += NUM_SENSORS;
		}
	}

	// return the total value of the match between episodes
//	printf("Match score %g\n", match);
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
	g_episodeList 		= newVector();
	g_ruleList			= newVector();
	g_milestoneList 	= newVector();
	g_connectToRoomba 	= 0;
	g_statsMode 		= 0;
}// initSupervisor

/**
 * endSupervisor
 *
 * Free the memory allocated for the Supervisor
 */
void endSupervisor()
{
	freeVector(g_episodeList);
	freeVector(g_ruleList);
	freeVector(g_milestoneList);
}// endSupervisor

/**
 * interpretCommand
 *
 * Return a char* with the string equivalent of a command
 * Use for printing to console
 *
 * @arg cmd Integer representing the command
 * @return char* Char array with command as string
 */
char* interpretCommand(int cmd)
{
	switch(cmd)
	{
		case CMD_NO_OP:
			return g_no_op;
			break;
		case CMD_FORWARD:
			return g_forward;
			break;
		case CMD_LEFT:
			return g_left;
			break;
		case CMD_RIGHT:
			return g_right;
			break;
		case CMD_BLINK:
			return g_blink;
			break;
		case CMD_ADJUST_LEFT:
			return g_adjustL;
			break;
		case CMD_ADJUST_RIGHT:
			return g_adjustR;
			break;
		case CMD_SONG:
			return g_song;
			break;
		default:
			return g_unknown;
			break;
	}
}// interpretCommand

/**
 * interpretCommandShort
 *
 * Return a char* with the string equivalent of a command
 * Use for printing to console
 *
 * @arg cmd Integer representing the command
 * @return char* Char array with command as string
 */
char* interpretCommandShort(int cmd)
{
	switch(cmd)
	{
		case CMD_NO_OP:
			return g_no_opS;
			break;
		case CMD_FORWARD:
			return g_forwardS;
			break;
		case CMD_LEFT:
			return g_leftS;
			break;
		case CMD_RIGHT:
			return g_rightS;
			break;
		case CMD_BLINK:
			return g_blinkS;
			break;
		case CMD_ADJUST_LEFT:
			return g_adjustLS;
			break;
		case CMD_ADJUST_RIGHT:
			return g_adjustRS;
			break;
		case CMD_SONG:
			return g_songS;
			break;
		default:
			return g_unknownS;
			break;
	}
}// interpretCommandShort

/**
 * interpretSensorsShort
 *
 * Return an integer that summaries the sensor values in an episdode.
 * Since all sensors are binary we can combine them all into one
 * binary integer.
 *
 * @arg int* Sensors array of ints representing the sensors (must be
 *           of length NUM_SENSORS)
 * @return int that summarizes sensors
 */
int interpretSensorsShort(int *sensors)
{
    int i, result = 0;
    int sumval = 1;  //This is always = to 2^i
    for(i = NUM_SENSORS-1; i >= 0; i--)
    {
        if (sensors[i])
        {
            result += sumval;
        }

        sumval *= 2;
    }

    return result;
}// interpretSensorsShort

