/**
* unitTest.c
*
* Author: Zachary Paul Faltersack
* Created: September 17, 2010
*
* This is a set of functions that emulate a maze environment
* along with a virtual roomba. When unitTest is called,
* it is passed a command to complete within the virtual environment,
* along with a flag to release memory if the tests have been completed.
* A return sensor string is then constructed based upon the virtual
* roomba's location and orientation within the maze.
*/

#include "unitTest.h"

// Global vars to keep track of the init and goal coords
int g_init_x_R;
int g_init_y_R;

int g_init_x_G;
int g_init_y_G;

// init heading for doing resets
int g_init_heading;

// map dimensions
int g_map_width;
int g_map_height;

int g_X;
int g_Y;
int** g_world;
int g_heading;
int g_hitGoal;

//keep track of number times goal is found
int g_goalCount = 0;

// extra vars for seeing how agents react to changes in a map
int g_goalNumToSwitchOn = -1;
int g_newMap;


/**
* loadMap
*
* Load a map from a text file
* If you look at world.maps at the beginning is a description of the expected format 
* for defining new worlds
*/
void loadMap(int mapNum)
{
	// Create file pointer to file with maps and open it for reading
	FILE* maps = fopen("world.maps", "rt");

	// Iterate through maps until we find the map number mapNum
	char buffer[100];
	int currMap = -1;
	while(currMap != mapNum)
	{
		fgets(buffer, sizeof(buffer), maps);
		// Pull out the values into the appropriate variables
		sscanf(buffer, "%d %d %d %d %d", &currMap, 
                                         &g_map_width, 
                                         &g_map_height, 
                                         &g_goalNumToSwitchOn, 
                                         &g_newMap);
	}

	// allocate space for ptrs to columns
	g_world = (int**) malloc(g_map_width * sizeof(int*));

	int i, j;
	// allocate columns
	for(i = 0; i < g_map_width; i++)
	{
		g_world[i] = (int*) malloc(g_map_height * sizeof(int));
	}// for

	char c;
	int counter = 0;
	int virtualElement;
	while(counter < g_map_width * g_map_height)
	{
		// Retrieve next character
		c = (char)fgetc(maps);

		// Match the character to those available and set the possible vars
		switch(c)
		{
			// Virtual map elements
			case 'W': virtualElement = V_R_WALL; 		break;
			case ' ': virtualElement = V_R_HALLWAY; 	break;
			case 'G': virtualElement = V_R_GOAL; 		break;
			// Virtual roomba elements
			case '^': virtualElement = V_R_ROOMBA; g_init_heading = HDG_N; break;
			case '7': virtualElement = V_R_ROOMBA; g_init_heading = HDG_NE; break;
			case '>': virtualElement = V_R_ROOMBA; g_init_heading = HDG_E; break;
			case 'J':virtualElement = V_R_ROOMBA; g_init_heading = HDG_SE; break;
			case 'v': virtualElement = V_R_ROOMBA; g_init_heading = HDG_S; break;
			case 'L': virtualElement = V_R_ROOMBA; g_init_heading = HDG_SW; break;
			case '<': virtualElement = V_R_ROOMBA; g_init_heading = HDG_W; break;
			case 'F': virtualElement = V_R_ROOMBA; g_init_heading = HDG_NW; break;
			case '\n':counter--; break;	// Must subtract 1 here in order for counter to still
										// be accurate as position marker in map
		}

		// If we found the roomba or the goal, then use counter to set init coords
		if(virtualElement == V_R_ROOMBA)
		{
			g_init_x_R = counter % g_map_width;
			g_init_y_R = counter / g_map_width;
			
		}
		else if(virtualElement == V_R_GOAL)
		{
			g_init_x_G = counter % g_map_width;
			g_init_y_G = counter / g_map_width;
		}
	
		// Set the current virtual element into its spot in the world
		g_world[counter % g_map_width][counter / g_map_width] = virtualElement;
		counter++;
	}

	// close the map file
	fclose(maps);

	// Do not want stats mode most of the time
	g_statsMode = TRUE;
	
	// Set up the world vars according to what we read in
	resetWorld();

	// Print out the World that we just read in
	if(!g_statsMode) printf("====================== Initial Map =======================\n");
	displayWorld();
	if(!g_statsMode) printf("==========================================================\n");
}//loadMap

/**
 * freeWorld
 *
 * This function frees the map of the environment used by the unit test
 */
void freeWorld()
{
	int i;
	for(i = 0; i < g_map_width; i++)
	{
		free(g_world[i]);
	}
	free(g_world);
}//freeWorld

/**
* resetWorld
*
* After a goal is found this is called to reset the world
*/
void resetWorld()
{
	// reset goal flag
	g_hitGoal = FALSE;
	// reset heading
	g_heading = g_init_heading;
	// reset goal 
	g_world[g_init_x_G][g_init_y_G] = V_R_GOAL;

    // reset current coords
	g_X = g_init_x_R;
	g_Y = g_init_y_R;

	// return roomba to init
	g_world[g_X][g_Y] = V_R_ROOMBA;

    g_goalCount++;
	if(g_statsMode)	printf("Hit Goal %d\n", g_goalCount);
}//resetWorld

/**
 * performMapMod
 *
 * This is a function that will add in a wall at a desired location
 * after a desired number of goals. These values are set in the world.maps
 * file in: /upbot/communication/world.maps
 */
void performMapMod()
{
	// free memory used by old map
	int i;
	for(i = 0; i < g_map_width; i++)
	{
		free(g_world[i]);
	}
	free(g_world);

    loadMap(g_newMap);

    //loadMap calls reset map which increments the goal count.
    //We don't want that here so unset it
    g_goalCount--;
}//performMapMod

/**
 * displayWorld
 *
 * Print the current view of the world
 */
void displayWorld()
{
    int i,j;
    // Iterate down columns
    for(i = 0; i < g_map_height; i++)
    {
        // Iterate across rows
        for(j = 0; j < g_map_width; j++)
        {
            // Switch on item under pointer
            switch(g_world[j][i])
            {
                case V_R_WALL:
                    if(!g_statsMode) printf("W");
                    break;
                case V_R_HALLWAY:
                    if(!g_statsMode) printf(" ");
                    break;
                case V_R_GOAL:
                    if(!g_statsMode) printf("G");
                    break;
                case V_R_ROOMBA:
                    // Have a different symbol depending on the heading of the Roomba
                    // This is simply for visual feedback
                    switch(g_heading)
                    {
                        case HDG_N:
                            if(!g_statsMode) printf("^");
                            break;
                        case HDG_NE:
                            if(!g_statsMode) printf("7");
                            break;
                        case HDG_E:
                            if(!g_statsMode) printf(">");
                            break;
                        case HDG_SE:
                            if(!g_statsMode) printf("J");
                            break;
                        case HDG_S:
                            if(!g_statsMode) printf("v");
                            break;
                        case HDG_SW:
                            if(!g_statsMode) printf("L");
                            break;
                        case HDG_W:
                            if(!g_statsMode) printf("<");
                            break;
                        case HDG_NW:
                            if(!g_statsMode) printf("F");
                            break;
                    }//switch
                    break;
            }//switch
        }//for
        if(!g_statsMode) printf("\n");
    }//for
    if(!g_statsMode) printf("\n");
}//displayWorld

/**
 * unitTest
 *
 * This subroutine emulates a Roomba in the grid world defined by world
 * It receives an action from the supervisor and updates the world map with its
 * location. This allows us to determine the next set of sensor data to return
 * to the supervisor.
 *
 * @arg command This is a command from the supervisor
 * @arg needCleanup Use as Boolean, if TRUE then test is over and need to free memory
 *
 * @return char* a string containing fake sensor data
 */
char* unitTest(int command, int needCleanup)
{
    // Check if we've completed the unit test and need to clean up
    if(needCleanup)
    {
        freeWorld();
        return NULL;
    }

    // If we hit a goal last time reset Roomba and Goal locations
    if(g_hitGoal) 
    {
        // Check to see if the map needs to be altered
        if(g_goalNumToSwitchOn != -1 && g_goalCount == g_goalNumToSwitchOn) 
        {
            performMapMod();
        }

        resetWorld();
    }

    return doMove(command);

    // //Use this method instead of doMove to simulate a McCallum-like environment 
    // return doMoveMcCallum(command);
}//unitTest

/**
 * doMove
 *
 * This function applies the command received from the Supervisor to the
 * world and returns the resulting sensor string
 *
 * @arg command An integer representing the command to complete
 * @return A pointer to char buffer containing the sensor string to return
 */
char* doMove(int command)
{
    int	lBump, rBump, 	// Sensor vars
        lCliff, rCliff, 
        lCliffFront, rCliffFront,
        lDrop, rDrop,	
        IR, caster;
    int sensorReturn;	// Total sensor return
    int abort;			// Abort bit (deprecated)

    // init sensor values
    lBump 		= rBump 		= lCliff 	= rCliff 	= IR 		= SNSR_OFF;
    lCliffFront = rCliffFront 	= lDrop 	= rDrop 	= caster 	= SNSR_OFF;
    sensorReturn = NONE_HIT;
    abort = FALSE;

    // Switch on the command that was received to update world appropriatelY
    switch(command)
    {
        // Left and Right just turn the Roomba
        case CMD_LEFT:
            if(!g_statsMode) printf("Turn left 45 degrees\n");
            g_heading = (g_heading > HDG_N ? g_heading - 1 : HDG_NW);
            break;
        case CMD_RIGHT:
            if(!g_statsMode) printf("Turn right 45 degrees\n");
            g_heading = (g_heading < HDG_NW ? g_heading + 1 : HDG_N);
            break;
            // Adjusts currently not implemented in 'perfect' world
        case CMD_ADJUST_LEFT:
            if(!g_statsMode) printf("Adjust left\n");
            break;
        case CMD_ADJUST_RIGHT:
            if(!g_statsMode) printf("Adjust right\n");
            break;
            // Forward must check for walls and IR in direction of g_heading
        case CMD_FORWARD:
            if(!g_statsMode) printf("Move forward one world unit\n");

            // First check the diagonal g_headings
            if(g_heading == HDG_NE)
            {
                sensorReturn = bumpSensor(g_world[g_X][g_Y-1], g_world[g_X+1][g_Y]);
            }
            else if(g_heading == HDG_SE)
            {
                sensorReturn = bumpSensor(g_world[g_X+1][g_Y], g_world[g_X][g_Y+1]);
            }
            else if(g_heading == HDG_SW)
            {
                sensorReturn = bumpSensor(g_world[g_X][g_Y+1], g_world[g_X-1][g_Y]);
            }
            else if(g_heading == HDG_NW)
            {
                sensorReturn = bumpSensor(g_world[g_X-1][g_Y], g_world[g_X][g_Y-1]);
            }
            // Next check the horizontal and vertical headings
            // Also begin check for IR in spot we move to
            else if(g_heading == HDG_N)
            {
                if(g_world[g_X][g_Y-1] == V_R_WALL)
                {
                    sensorReturn = BOTH_HIT;
                }
                else if(g_world[g_X][g_Y-1] == V_R_GOAL)
                {
                    IR = SNSR_ON;
                }
            }
            else if(g_heading == HDG_E)
            {
                if(g_world[g_X+1][g_Y] == V_R_WALL)
                {
                    sensorReturn = BOTH_HIT;
                }
                else if(g_world[g_X+1][g_Y] == V_R_GOAL)
                {
                    IR = SNSR_ON;
                }
            }
            else if(g_heading == HDG_S)
            {
                if(g_world[g_X][g_Y+1] == V_R_WALL)
                {
                    sensorReturn = BOTH_HIT;
                }
                else if(g_world[g_X][g_Y+1] == V_R_GOAL)
                {
                    IR = SNSR_ON;
                }
            }
            else if(g_heading == HDG_W)
            {
                if(g_world[g_X-1][g_Y] == V_R_WALL) 
                {
                    sensorReturn = BOTH_HIT;
                }
                else if(g_world[g_X-1][g_Y] == V_R_GOAL) 
                {
                    IR = SNSR_ON;
                }
            }

            // Set sensor values according to result from completing 
            // the command and if none are hit then update roomba location
            if(sensorReturn == BOTH_HIT)
            {
                lBump = rBump = SNSR_ON;
                abort = TRUE;
            }
            else if(sensorReturn == RIGHT_HIT)
            {
                rBump = SNSR_ON;
                abort = TRUE;
            }
            else if(sensorReturn == LEFT_HIT)
            {
                lBump = SNSR_ON;
                abort = TRUE;
            }
            else if(sensorReturn == NONE_HIT)
            {
                if(IR == SNSR_ON)
                {
                    g_hitGoal = TRUE;
                }

                // If no sensors were hit then update roomba location accordingly
                switch(g_heading)
                {
                    case HDG_N:
                        g_world[g_X][g_Y] = V_R_HALLWAY;
                        g_Y--;
                        g_world[g_X][g_Y] = V_R_ROOMBA;
                        break;
                    case HDG_E:
                        g_world[g_X][g_Y] = V_R_HALLWAY;
                        g_X++;
                        g_world[g_X][g_Y] = V_R_ROOMBA;
                        break;
                    case HDG_S:
                        g_world[g_X][g_Y] = V_R_HALLWAY;
                        g_Y++;
                        g_world[g_X][g_Y] = V_R_ROOMBA;
                        break;
                    case HDG_W:
                        g_world[g_X][g_Y] = V_R_HALLWAY;
                        g_X--;
                        g_world[g_X][g_Y] = V_R_ROOMBA;
                        break;
                }//switch
            }//if
            break;
        case CMD_BLINK:
            if(!g_statsMode) printf("Blink\n");
            break;
        case CMD_NO_OP:
            if(!g_statsMode) printf("No operation\n");
            break;
        case CMD_SONG:
            if(!g_statsMode) printf("Song\n");
            break;
        case CMD_ILLEGAL:
            if(!g_statsMode) printf("Resetting world\n");
            resetWorld();
        default:
            if(!g_statsMode) printf("Invalid command: %i\n", command);
            break;
    }//switch

    if(!g_statsMode) displayWorld();

    return setSensorString(IR, rCliff, rCliffFront, lCliffFront, lCliff, 
            caster, lDrop, rDrop, lBump, rBump, abort);
}//doMove

/*======================================================================
 * Alternative command names (McCallum Environment)
 *
 * These #defines supersede the ones normally used.  The numbers are the
 * same but their semantics are different.
 */

// Command definitions
#define CMD_NORTH	0x1    //was CMD_NO_OP 		
#define CMD_SOUTH	0x2    //was CMD_FORWARD		
#define CMD_WEST	0x3    //was CMD_LEFT		
#define CMD_EAST	0x4    //was CMD_RIGHT		


/**
 * doMoveMcCallum
 *
 * This is an alternative implementation of doMove that simulates McCallum's
 * environment.
 *
 * @arg command An integer representing the command to complete
 * @return A pointer to char buffer containing the sensor string to return
 */
char* doMoveMcCallum(int command)
{
    //Sensor variables.  Set all to off by default
    int northWall = SNSR_OFF;
    int southWall = SNSR_OFF;
    int eastWall = SNSR_OFF;
    int westWall = SNSR_OFF;
    int goal = SNSR_OFF;

    // Switch on the command that was received to update world appropriatelY
    switch(command)
    {
        // Left and Right just turn the Roomba
        case CMD_NORTH:
            if (g_world[g_X][g_Y-1] == V_R_GOAL)
            {
                goal = SNSR_ON;
            }
            else if (g_world[g_X][g_Y-1] == V_R_HALLWAY)
            {
                g_world[g_X][g_Y] = V_R_HALLWAY;
                g_Y--;
                g_world[g_X][g_Y] = V_R_ROOMBA;
            }
            break;
            
        case CMD_SOUTH:
            if (g_world[g_X][g_Y+1] == V_R_GOAL)
            {
                goal = SNSR_ON;
            }
            else if (g_world[g_X][g_Y+1] == V_R_HALLWAY)
            {
                g_world[g_X][g_Y] = V_R_HALLWAY;
                g_Y++;
                g_world[g_X][g_Y] = V_R_ROOMBA;
            }
            break;
            
        case CMD_EAST:
            if (g_world[g_X+1][g_Y] == V_R_GOAL)
            {
                goal = SNSR_ON;
            }
            else if (g_world[g_X+1][g_Y] == V_R_HALLWAY)
            {
                g_world[g_X][g_Y] = V_R_HALLWAY;
                g_X++;
                g_world[g_X][g_Y] = V_R_ROOMBA;
            }
            break;
            
        case CMD_WEST:
            if (g_world[g_X-1][g_Y] == V_R_GOAL)
            {
                goal = SNSR_ON;
            }
            else if (g_world[g_X-1][g_Y] == V_R_HALLWAY)
            {
                g_world[g_X][g_Y] = V_R_HALLWAY;
                g_X--;
                g_world[g_X][g_Y] = V_R_ROOMBA;
            }
            break;

        //All other legal commands are no-ops
        case CMD_ADJUST_LEFT:
        case CMD_ADJUST_RIGHT:  
        case CMD_BLINK:
        case CMD_SONG:
            break;
        case CMD_ILLEGAL:
            if(!g_statsMode) printf("Resetting world\n");
            resetWorld();
        default:
            if(!g_statsMode) printf("Invalid command: %i\n", command);
            break;
    }//switch

    //If goal was not found, init sensor values from current position
    if (goal == SNSR_OFF)
    {
        //North wall
        if (g_world[g_X][g_Y-1] == V_R_WALL)
        {
            northWall = SNSR_ON;
        }

        //South wall
        if (g_world[g_X][g_Y+1] == V_R_WALL)
        {
            southWall = SNSR_ON;
        }

        //East wall
        if (g_world[g_X+1][g_Y] == V_R_WALL)
        {
            eastWall = SNSR_ON;
        }

        //West wall
        if (g_world[g_X-1][g_Y] == V_R_WALL)
        {
            westWall = SNSR_ON;
        }
    }//if
    else
    {
        g_world[g_X][g_Y] = V_R_HALLWAY;
        g_hitGoal = TRUE;
    }
                            
        
    
    
    if(!g_statsMode) displayWorld();

    return setSensorString(goal, SNSR_OFF, SNSR_OFF, SNSR_OFF, SNSR_OFF, 
            SNSR_OFF, northWall, southWall, westWall, eastWall, FALSE);
}//doMoveMcCallum

/**
 * setSensorString
 *
 * This function takes the sensor values and fills out the sensor string 
 * to be sent to the Supervisor
 *
 * @arg IR 			Sensor values
 * @arg rCliff 		^
 * @arg rCliffFront	^
 * @arg lCliffFront	^
 * @arg lCliff		^
 * @arg caster		^
 * @arg lDrop		^
 * @arg rDrop		^
 * @arg lBump		^
 * @arg rBump		^
 * @return char*	The sensor data to be sent to the Supervisor
 */
char* setSensorString(int IR, int rCliff, int rCliffFront, int lCliffFront, int lCliff,
        int caster, int lDrop, int rDrop, int lBump, int rBump, int abort)
{
    static int timeStamp = 0;
    // Memory for data string to return to Supervisor
    char* str = (char*) malloc(sizeof(char) * 24); 

#if SEND_CRUTCH
    // This is temporary to check how Soar does if we tell it
    // there is a wall in front of it.

    if(g_heading == HDG_NE || g_heading == HDG_SE || 
       g_heading == HDG_SW || g_heading == HDG_NW) rCliff = SNSR_ON;
    else if(g_heading == HDG_N && g_world[g_X][g_Y-1] == V_R_WALL) rCliff = SNSR_ON;
    else if(g_heading == HDG_E && g_world[g_X+1][g_Y] == V_R_WALL) rCliff = SNSR_ON;
    else if(g_heading == HDG_S && g_world[g_X][g_Y+1] == V_R_WALL) rCliff = SNSR_ON;
    else if(g_heading == HDG_W && g_world[g_X-1][g_Y] == V_R_WALL) rCliff = SNSR_ON;
#endif

    // Fill out sensor string
    sprintf(&str[SNSR_IR],				"%i", IR);
    sprintf(&str[SNSR_CLIFF_RIGHT], 	"%i", rCliff);
    sprintf(&str[SNSR_CLIFF_F_RIGHT], 	"%i", rCliffFront);
    sprintf(&str[SNSR_CLIFF_F_LEFT], 	"%i", lCliffFront);
    sprintf(&str[SNSR_CLIFF_LEFT], 		"%i", lCliff);
    sprintf(&str[SNSR_CASTER], 			"%i", caster);
    sprintf(&str[SNSR_DROP_LEFT],		"%i", lDrop);
    sprintf(&str[SNSR_DROP_RIGHT], 		"%i", rDrop);
    sprintf(&str[SNSR_BUMP_LEFT], 		"%i", lBump);
    sprintf(&str[SNSR_BUMP_RIGHT], 		"%i", rBump);

    // Fill out rest of episode string with timestamp and abort signal
    int i, temp;
    for(i = 10; i < 24; i++)
    {
        // insert blank space
        if(i < 15)
        {
            str[i] = ' ';
            // insert timestamp
        }else if(i < 16)
        {
            sprintf(&str[i], "%i", timeStamp);	// print timestamp into str
            // determine num digits in timeStamp and adjust i accordingly
            temp = timeStamp;
            while(temp > 9)
            {
                temp = temp / 10;
                i++;
            }
            // more padding
        }else if(i < 22)
        {
            str[i] = ' ';
            // insert null terminating char
        }else if(i < 23)
        {
            sprintf(&str[i], "%i", abort);
        }else
        {
            str[i] = '\0';
        }
    }//for

    timeStamp++;

    return str;
}//setSensorString

/**
 * bumpSensor
 *
 * This function returns a value that tells the calling function
 * which bump sensors have been hit
 *
 * @arg north int that represents the wall N of the Roomba when normalized to point NE
 * @arg east int that represents the wall E of the Roomba when normalized to point NE
 * @return int that represents bump sensor values
 */
int bumpSensor(int north, int east)
{
    // return the appropriate value representing which, if any, bumpers are hit
    // account for value representing goal spot which is also a hallway
    if((north == V_R_HALLWAY || north == V_R_GOAL) && (east == V_R_HALLWAY || east == V_R_GOAL))
        return BOTH_HIT;
    if((north == V_R_HALLWAY || north == V_R_GOAL) && east == V_R_WALL)
        return RIGHT_HIT;
    if(north == V_R_WALL && (east == V_R_HALLWAY || east == V_R_GOAL))
        return LEFT_HIT;
    if(north == V_R_WALL && east == V_R_WALL)
        return BOTH_HIT;

    return NONE_HIT;
}//bumpSensor


