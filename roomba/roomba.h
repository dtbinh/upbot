#include <stdio.h>
#ifndef _GUMSTIX_H
#define _GUMSTIX_H

int defineSongs();
int openPort();
int closePort();
char readAndexecute(FILE *fp);
void byteTx(char value);
void byteRx(char* buffer, int nbytes, int iter);
void initialize();
void *ReaderThread( void *param);
//void calcFileLoc(int c);
void calcFileLoc(char c);
char* getTime();
void fprintBinaryAsString(FILE* fp, int n);
int checkSensorData(char *x);
void writeSensorDataToFile(char* sensorArray, FILE* fp, char* currTime);

int driveStraightWithFeedback(int velocity);
void driveStraightUntil(int sec, int speed);
void driveStraight(int velocity);
void turnCounterClockwise(int degrees);
void turnClockwise(int degrees);
void driveBackwardsUntil(int sec, int speed);
void driveBackwards(int speed);
void stop();

void setLED(int powerSetting, int playSetting, int advanceSetting);

#define HIGH_BYTE 0
#define LOW_BYTE 1
#define FALSE 0
#define TRUE 1
#define ACTIVE_SENSOR 1

//Command Values
#define CmdStart    128
#define CmdBaud	    129
#define CmdControl  130
#define CmdSafe     131
#define CmdFull     132
#define CmdPower    133
#define CmdSpot     134
#define CmdClean    135
#define CmdMax      136
#define CmdDrive    137
#define CmdMotors   138
#define CmdLeds     139
#define CmdSong     140
#define CmdPlay     141
#define CmdSensors  142


// iRobot Create Sensor Data Values
#define SENSOR_BUMP_LEFT   0x02
#define SENSOR_BUMP_RIGHT  0x01
#define SENSOR_BUMP_BOTH   0x03
#define SENSOR_WHEELDROP_RIGHT 0x04
#define SENSOR_WHEELDROP_LEFT  0x08
#define SENSOR_WHEELDROP_BOTH  0x0C
#define SENSOR_WHEELDROP_CASTER 0x10

// TLC: The following define is deprecated.  Remove it
// once it becomes clear that no other file is using it.
// 'SENSOR_BUMP_BOTH should be used in its place.
//#define BUMP_SENSORS 0x03

// Baud Rate definitions according to the
// iRobot Serial Command Interface
#define BaudRate_9600   5
#define BaudRate_57600  10
#define BaudRate_115200 11

// iRobot Create Speed Values
#define HIGH_SPEED 0x01F3
#define MED_SPEED 0x012c
#define LOW_SPEED 0x0064

// LED values
#define PWR_RED 255
#define PWR_GREEN 0
#define PWR_BLUE 100
#define SET_ADVANCE 0x08
#define SET_PLAY 0x02
#define SET_ALL 0xFF
#define FULL_INTENSITY 255
#define OFF 0
#define RED 1
#define GREEN 2
#define PLAY_OFF 0
#define PLAY_ON 1
#define ADVANCE_OFF 0
#define ADVANCE_ON 1

// Drive values
#define HIGH 3	//+-500mm/s
#define MED 2	//+-300mm/s
#define LOW 1	//+-100mm/s
#define FORWARD 1
#define BACKWARD -1
#define CLOCKWISE 90
#define CCLOCKWISE -90

// Sensor packet indices for group 1
#define SP_GROUP_ONE 1
#define SP_G1_BUMPS_WHEELDROPS 0
#define SP_G1_CLIFF_LEFT 2
#define SP_G1_CLIFF_FRONT_LEFT 3
#define SP_G1_CLIFF_FRONT_RIGHT 4
#define SP_G1_CLIFF_RIGHT 5
#define SP_G1_VIRTUAL_WALL 6

// Sensor packet numbers according to the 
// iRobot Open Interface
#define SP_BUMPS_WHEELDROPS 7
#define SP_CLIFF_LEFT 9
#define SP_CLIFF_FRONT_LEFT 10
#define SP_CLIFF_FRONT_RIGHT 11
#define SP_CLIFF_RIGHT 12
#define SP_VIRTUAL_WALL 13
#define SP_REQUESTED_VELOCITY 39

// Timing values
#define HALF_SECOND 500000
#define QUARTER_SECOND 250000
#define EIGHTH_SECOND 125000
#define DEGREES_45 45000000
#define DEGREES_90 1000000

#endif