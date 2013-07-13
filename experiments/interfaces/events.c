/**
 * events.c contains the events used by eventresponder.c
 *
 * An event is a function that is used to control check the status of a robot
 * It returns a boolean based upon a roombas sensor data passed into it
 * 
 */



/**
 * eventTrue
 * 
 * Default eventPredicate function.  Always returns true.
 */
int eventTrue(char * data)
{
  return 1;
}

/**
 * eventFalse
 * 
 * Default eventPredicate function.  Always returns true.
 */
int eventFalse(char * data)
{
  return 0;
}

/**
 * eventBump
 * 
 * Example eventPredicate function for checking for bump events.
 */
int eventBump(char * data) {

  // Check bump sensors.

  // TODO: Need to correct the use of literal 0 when checking
  // the state of the bump sensors.
  if(((data[0] & SENSOR_BUMP_RIGHT) == SENSOR_BUMP_RIGHT) || 
     ((data[0] & SENSOR_BUMP_LEFT ) == SENSOR_BUMP_LEFT))
    {
      return 1;
    }
  else
    return 0;
}

/**
 * eventNotBump
 * 
 * Example eventPredicate function for checking for bump events.
 */
int eventNotBump(char * data) {

  // Check bump sensors.

  // TODO: Need to correct the use of literal 0 when checking
  // the state of the bump sensors.
  if(((data[0] & SENSOR_BUMP_RIGHT) == SENSOR_BUMP_RIGHT) || 
     ((data[0] & SENSOR_BUMP_LEFT ) == SENSOR_BUMP_LEFT))
    {
      return 0;
    }
  else
    return 1;
}

/**
 * eventAlarm
 */
int eventAlarm(char * data) {
  if (data[15] != '0') {
   return 1;
   printf("got alarm 2\n");
  }
  return 0;
}

