#include <string.h>
#include "eaters.h"
#include "vector.h"
#include "supervisor.h"

int main()
{
    int i;
    for(i = 1; i >= 0; i--)
    {
        printf("Initializing world: %d\n", i);
        initWorld(i);
        printf("Printing world\n");
        displayWorld();

        printf("Running through commands\n");
        printf("Moving east\n");
        printf("%s\n", unitTest(MOVE_E, FALSE));
        printf("Moving south\n");
        printf("%s\n", unitTest(MOVE_S, FALSE));
        printf("Moving west\n");
        printf("%s\n", unitTest(MOVE_W, FALSE));
        printf("Moving north\n");
        printf("%s\n", unitTest(MOVE_N, FALSE));
    }
    printf("Freeing memory\n");
    unitTest(0, TRUE);

    return 0;
}//main


