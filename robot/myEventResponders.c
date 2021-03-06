#include "myEventResponders.h"

void selectNextER(char * erName, eventResponder* myER) {

	if (strcmp(erName,"go")==0) {
		initalizeWanderER(myER);
	} else if (strcmp(erName,"stop")==0) {
		initalizeStopER(myER);
	}

}

void initalizeWanderER(eventResponder* myER) {

	myER->curState = 0;
	myER->stateCount = 2;
	myER->states = malloc(sizeof(state)*2);

		myER->states[0].count = 4;  
		myER->states[0].clockTime = 5; 
		myER->states[0].transitions = malloc(sizeof(transition)*4); 

			myER->states[0].transitions[0].e = eventAlarm; 
			myER->states[0].transitions[0].r = respondDriveMed;
			myER->states[0].transitions[0].n = 1;

			myER->states[0].transitions[1].e = eventBump; 
			myER->states[0].transitions[1].r = respondTurn;
			myER->states[0].transitions[1].n = 0;

			myER->states[0].transitions[2].e = eventVWall; 
			myER->states[0].transitions[2].r = respondTurn;
			myER->states[0].transitions[2].n = 0;

			myER->states[0].transitions[3].e = eventTrue; 
			myER->states[0].transitions[3].r = respondDriveLow;
			myER->states[0].transitions[3].n = 0;

		myER->states[1].count = 3;
		myER->states[1].clockTime = 0;
		myER->states[1].transitions = malloc(sizeof(transition)*3); 

			myER->states[1].transitions[0].e = eventBump; 
			myER->states[1].transitions[0].r = respondTurn;
			myER->states[1].transitions[0].n = 0;

			myER->states[1].transitions[1].e = eventVWall; 
			myER->states[1].transitions[1].r = respondTurn;
			myER->states[1].transitions[1].n = 0;

			myER->states[1].transitions[2].e = eventTrue; 
			myER->states[1].transitions[2].r = respondDriveMed;
			myER->states[1].transitions[2].n = 1;

}

