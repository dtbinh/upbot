/**
* eaters.c
*
* This runs a virtual environment called Eaters
* where a Supervisor attempts to amass the greatest score
* possible by 'eating' items in a small world.
*
* Author: Zachary Paul Faltersack
* Last Edit: June 1, 2011
*
*/

#include "eaters.h"

#define DEBUGGING 0

// map dimensions
int g_map_width;
int g_map_height;

int g_X;
int g_Y;
char* g_color;
int** g_world;

//keep track of number times goal is found
int g_numMoves;
int g_score;
int g_reward;

/**
 * initWorld
 *
 * This function initializes the Eaters environment in a 'random'
 * layout of walls and food.
 *
 * @param firstInit A boolean to indicate if memory needs to be allocate for the map
 */
void initWorld(int firstInit)
{
    int i,j;
    // At some point I'm figuring we may want to randomly choose the columns that have the special food.
    int sFoodC1 = 2, sFoodC2 = 5, sFoodC3 = 8, sFoodC4 = 11, sFoodC5 = 14;

	//--Allocate memory if this is the first time initWorld is called
	if(firstInit)
    {
        g_map_width 	= MAP_WIDTH;
        g_map_height 	= MAP_HEIGHT;
#if DEBUGGING
        printf("Allocating memory1\n");
#endif
        srand(time(NULL)); 	
#if DEBUGGING
        printf("Allocating memory2\n");
#endif
        g_world = (int**)malloc(g_map_width * sizeof(int*));
        for(i = 0; i < g_map_width; i++) g_world[i] = (int*)malloc(g_map_height * sizeof(int));

#if DEBUGGING
        printf("Allocating memory3\n");
#endif
        g_color = (char*)malloc(sizeof(char) * 10);
        strcpy(g_color, "red");
    }//if

#if DEBUGGING
    printf("Setting globals\n");
#endif
    //--Set up globals------------
    g_X 			= (rand() % (g_map_width - 2)) + 1;;
    g_Y 			= (rand() % (g_map_height - 2)) + 1;;
    g_numMoves 		= 0;
    g_score 		= 0;
    g_reward        = 0;
    g_statsMode 	= FALSE;
    //----------------------------

#if DEBUGGING
    printf("Setting environment\n");
#endif
    //--Set up the environment with bounding walls and food supplies
    for(i = 0; i < g_map_width; i++)
    {
        for(j = 0; j < g_map_height; j++)
        {
            if(i == 0 || i == g_map_width - 1 || 
                    j == 0 || j == g_map_height - 1) g_world[i][j] = V_WALL;
            else if(i == sFoodC1 || i == sFoodC2 || 
                    i == sFoodC3 || i == sFoodC4 || 
                    i == sFoodC5) g_world[i][j] = V_FOOD2;
            else g_world[i][j] = V_FOOD1;
        }//for
    }//for

#if DEBUGGING
    printf("Adding internal walls\n");
#endif
    //--Set up internal walls in random layout
    int numWalls = 0, percentWalls = MAP_PERCENT_WALLS;
    while(numWalls++ < (g_map_height - 2) * (g_map_width - 2) * (percentWalls) / 100)
    {
        i = (rand() % (g_map_width - 2)) + 1;
        j = (rand() % (g_map_height - 2)) + 1;

        g_world[i][j] = V_WALL;
    }//while

#if DEBUGGING
    printf("Inserting agent\n");
#endif
    //--Insert our agent
    g_world[g_X][g_Y] = V_AGENT;
}//initWorld

/**
 * freeWorld
 *
 * This function frees the map of the environment used by the unit test
 */
void freeWorld()
{
#if DEBUGGING
    printf("Freeing memory\n");
#endif
    int i;
    for(i = 0; i < g_map_width; i++)
    {
        free(g_world[i]);
    }
    free(g_world);
}//freeWorld

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
                case V_EMPTY:
                    if(!g_statsMode) printf(" ");
                    break;
                case V_WALL:
                    if(!g_statsMode) printf("W");
                    break;
                case V_FOOD1:
                    if(!g_statsMode) printf("-");
                    break;
                case V_FOOD2:
                    if(!g_statsMode) printf("+");
                    break;
                case V_AGENT:
                    if(!g_statsMode) printf("O");
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
    }//if

    return doMove(command);
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
    /*
       If the spot is empty, then it will contain a 0, else
       it will contain one of the two rewards. If a wall is
       there we won't move into it.
       So first we check for a wall in our destination.
       If free, we first gather the reward in the location
       (0,5,10)
       Then we add that to the running score, free our current
       location, and move into the destination square.
     */
    g_reward = 0;   // Reset here just in case a wall is in our way
    // Switch on the command that was received to update world appropriatelY
    switch(command)
    {
        // Left and Right just turn the Roomba
        case CMD_MOVE_N:
            if(!g_statsMode) printf("Move north...\n");
            if(g_world[g_X][g_Y - 1] != V_WALL)
            {
                g_reward = g_world[g_X][g_Y - 1];
                g_score += g_reward;
                g_world[g_X][g_Y] = V_EMPTY;
                g_Y--;
                g_world[g_X][g_Y] = V_AGENT;
            }//if
            else if(!g_statsMode) printf("Cannot complete command.\n");
            break;
        case CMD_MOVE_S:
            if(!g_statsMode) printf("Move south...\n");
            if(g_world[g_X][g_Y + 1] != V_WALL)
            {
                g_reward = g_world[g_X][g_Y + 1];
                g_score += g_reward;
                g_world[g_X][g_Y] = V_EMPTY;
                g_Y++;
                g_world[g_X][g_Y] = V_AGENT;
            }//if
            else if(!g_statsMode) printf("Cannot complete command.\n");
            break;
        case CMD_MOVE_E:
            if(!g_statsMode) printf("Move east...\n");
            if(g_world[g_X + 1][g_Y] != V_WALL)
            {
                g_reward = g_world[g_X + 1][g_Y];
                g_score += g_reward;
                g_world[g_X][g_Y] = V_EMPTY;
                g_X++;
                g_world[g_X][g_Y] = V_AGENT;
            }//if
            else if(!g_statsMode) printf("Cannot complete command.\n");
            break;
        case CMD_MOVE_W:
            if(!g_statsMode) printf("Move west...\n");
            if(g_world[g_X - 1][g_Y] != V_WALL)
            {
                g_reward = g_world[g_X - 1][g_Y];
                g_score += g_reward;
                g_world[g_X][g_Y] = V_EMPTY;
                g_X--;
                g_world[g_X][g_Y] = V_AGENT;
            }//if
            else if(!g_statsMode) printf("Cannot complete command.\n");
            break;
        case CMD_NO_OP:
            if(!g_statsMode) printf("No operation...\n");
            break;
        default:
            if(!g_statsMode) printf("Invalid command: %i\n", command);
            break;
    }//switch

    g_numMoves++;

    if(!g_statsMode) displayWorld();

    return setSenseString();
}//doMove

/**
 * setSenseString
 *
 * This function puts together the sensor string to be sent to the agent.
 *
 * @return char*	The sensor data to be sent to the Supervisor
 */
char* setSenseString()
{
    // Memory for data string to return to Supervisor
    char* str = (char*) malloc(sizeof(char) * 150); 

    // Fill out sensor string
    sprintf(str, ":UL,i,%d:UM,i,%d:UR,i,%d:LT,i,%d:RT,i,%d:LL,i,%d:LM,i,%d:LR,i,%d:score,i,%d:steps,i,%d:color,s,%s:reward,i,%d:", 
            g_world[g_X - 1][g_Y - 1], g_world[g_X][g_Y - 1], g_world[g_X + 1][g_Y - 1], 
            g_world[g_X - 1][g_Y], g_world[g_X + 1][g_Y], 
            g_world[g_X - 1][g_Y + 1], g_world[g_X][g_Y + 1], g_world[g_X + 1][g_Y + 1],
            g_score, g_numMoves, g_color, g_reward);

    return str;
}//setSenseString

