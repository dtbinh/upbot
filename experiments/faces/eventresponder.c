/**
 * eventresponder.c
 *
 * The UPBOT System treats a robot as a single-threaded state machine
 * that is programmed using an event:responder.
 *
 * Events are the sensor data provided by the robot.  The UPBOT system
 * considers the following subset of sensor data: Wheeldrop caster,
 * wheeldrop left, wheeldrop right, bump left, bump right, cliff left,
 * cliff front left, cliff front right, cliff right, and the virtual
 * wall.  
 *
 * Checking for a particular event is done by an eventPredicate
 * function.
 *
 * Responses are the commands that may be issued to the iRobot Create
 * that alter its behaviour in the physical world, such as drive or 
 * blink LEDs.
 *
 * Issuing a response is done by a response function.
 *
 * An event:responder is a set of states. Each state has a set
 * of transitions which include an conitional event and a
 * reactionary responder
 * 
 * 
 * @author: Tanya L. Crenshaw, Matthew Holland
 * @since: July 20, 2013
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/time.h>
#include <time.h>

#include "../../roomba/roomba.h"
#include "eventresponder.h"
#include "robot.h"
#include "services.h"

#include "roomba/sensors.c"
#include "roomba/utility.c"
#include "roomba/commands.c"

#include "events.c"
#include "responders.c"

// Global value to keep track of the alarm occurrence 
// Set nonzero on receipt of SIGALRM 
// TODO: Global?  Really?  Can this be better?
static volatile sig_atomic_t gotAlarm = 0; 

//NOTE: removed the function to create eventResponders
//  I don't think it is needed with new data format
//  If we decide to be put back it would have needed
//  to be completly rewriten anyway

/*
  simple singal handler to set set the gotAlarm variable
  so to notify the user about alarms
*/
static void signalrmHandler(int sig)
{
    gotAlarm = 1;
}


void setupClock() {
    struct sigaction sa;        // Signal sets
    struct sigaction toggledsa;   

    // Initialize a random number generator to help with the
    // generation of random data.
    srand(time(NULL));  


    //TODO: move timer setup to a seperate function
    //with the intent of reducing clutter
    
    // Initialize and empty a signal set
    sigemptyset(&sa.sa_mask);

    // Set the signal set.
    sa.sa_flags = 0;
    sa.sa_handler = signalrmHandler;

    // Update the signal set with the new flags and handler.
    if (sigaction(SIGALRM, &sa, NULL) == -1)
    {
        exit(EXIT_FAILURE);
    }
}

void setClock(int sec, int usec) {
   
    //TODO: figure out how to cancel old alarms
    
    struct itimerval itv;       // Specify when a timer should expire 
    
    // Initialize timer start time and period:
    // First, the period between now and the first timer interrupt 
    itv.it_value.tv_sec = sec; //seconds
    itv.it_value.tv_usec = usec; // microseconds

    // Second, the intervals between successive timer interrupts 
    itv.it_interval.tv_sec = 0; // seconds
    itv.it_interval.tv_usec = 0; // microseconds

    if (setitimer(ITIMER_REAL, &itv, NULL) == -1) {
        exit(EXIT_FAILURE);
    }

}




// ************************************************************************
// EXAMPLE MAIN.
// Create timer-based interrupts and an event:responder to handle a set
// of clocks and sensor data events in a faked up system.
//
// This example main should eventually evolve into the new "nerves"
// portion of the UPBOT robotics system.
// ************************************************************************

int main(void)
{

    //setup a event responder
    //this one drives forward until it hits something
    //and then turns a random direction. Aftew a few
    //seconds without hitting anything it speeds up

    //*
    transition erS0[3] = {//transitions for state 0
        {eventAlarm, respondDriveMed, 1},
        {eventNotBump, respondDriveLow, 0},
        {eventBump, respondTurn, 0}    
    }; 
    state S0 = {
        5, //alarm time
        erS0,
        3  //number of events
    };

    transition erS1[2] = {//transitions for state 1
        {eventNotBump, respondDriveMed, 1},
        {eventBump, respondTurn, 0}
    };
    state S1 = {
        0, //alarm time
        erS1,
        2  //number of events
    };


    state states[2] = {S0,S1};

    eventResponder myER = {
        states,//list of states
        0,   //default state
        2    //number of states
    };
    //*/

    /*
    //   :( this format doesn't work, I miss you Lua.
    //   I'll keep this in the hope that I can find 
    //   a way to do this
    
    er myER[2] = {
        { //states
            { //state 0
                5,  //alarm time
                {
                    {eventAlarm, respondDriveMed, 1},
                    {eventNotBump, respondDriveLow, 0},
                    {eventBump, respondTurn, 0}
                },
                3   //number of events
            },
            { //state 1
                0,  //alarm time
                {
                    {eventNotBump, respondDriveMed, 1},
                    {eventBump, respondTurn, 0}
                },
                2   //number of events
            }
        },
        0,    //default state
        2     //number of states
    }
    //*/

    setupClock();

    if (myER.states[myER.curState].clockTime > 0)
    {
        setClock(myER.states[myER.curState].clockTime,0);
    }

    // Create and initialize a robot.
    robot myRobot = {NULL, 
        "Webby", 
        "10.81.3.181", 
        &myER, 
        NULL, 
        NULL};

    //Start up robotly things
    if (openPort() == 0) {
        printf("Port failed to open");
        exit(-1);
    }
    initialize();
    sleep(1);

    // Access the event:responder via the robot variable.  (I am doing
    // this to make sure I understand the types.  I know that I can
    // access the event:responder via myEventResponder, declared above).
    eventResponder * er = myRobot.er;

    // Need a pointer for the e's and a pointer for the r's.
    eventPredicate * e;
    responder * r;
    nextState n;


    transition * transitions= myER.states[myER.curState].transitions;
    int transitionsCount = myER.states[myER.curState].count;

    // Create an event loop
    while(1)
    {

        //usleep(200000);

        //read sensor data
        char sensDataFromRobot[ER_SENS_BUFFER_SIZE] = {'\0'};   //TODO: this array can be smaller
        receiveGroupOneSensorData(sensDataFromRobot);
        sensDataFromRobot[15] = '0'+gotAlarm; 
        gotAlarm = 0;



        //Debug printing
        printf("bump right: %d\n",(sensDataFromRobot[0] & SENSOR_BUMP_RIGHT));
        printf("bump left: %d\n",(sensDataFromRobot[0] & SENSOR_BUMP_LEFT));
        printf("wheeldrops: %d\n",(sensDataFromRobot[0] & SENSOR_WHEELDROP_BOTH));

        printf("vwall: %d\n",sensDataFromRobot[0]);

        // ***********************************************************
        // I leave this here because I may need to know about masking
        // signals among threads later. -- TLC
        // 
        // Mask signals ?
        //   Kernel maintains a _signal mask_ for each process
        //   The signal mask is actually a per-thread attribute!

        //  Note that, "The pthread_sigmask() function is just like
        //  sigprocmask(2), with the difference that its use in
        //  multithreaded programs is explicitly specified by
        //  POSIX.1-2001."
        // ***********************************************************

        int eventOccured = 0;  

        // Loop over all of the eventPredicate and responder pairs
        // in the current state.
        int i = 0;
        for(i = 0; i < transitionsCount; i++)
        {

            if (eventOccured == 0) 
            {
                e = transitions[i].e; //event to check against

                if((e)(sensDataFromRobot))
                {
                    r = transitions[i].r; //the responder to execute
                    r();


                    n = transitions[i].n; //the next state that we need to be in
                    if (myER.curState != n) {
                        printf("State changing from %d to %d\n",myER.curState,n);
                        
                        myER.curState = n;
                        transitions = myER.states[n].transitions;
                        transitionsCount = myER.states[n].count;
                        
                        int nextAlarm =  myER.states[myER.curState].clockTime;
                        if (nextAlarm > 0)  {
                            //TODO: reset the alarm somewhere
                            setClock(nextAlarm,0);
                        }
                    }
 
                    //event occured, we don't want to check anymore
                    eventOccured = 1;
                }
            }
        }
    }
    return 0;

}
