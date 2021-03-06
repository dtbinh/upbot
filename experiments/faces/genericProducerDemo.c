/**
 * robot.c
 *
 * A simple demo for testing the basic functionality of acceptors and
 * connectors.  This demo tests the connector side of the
 * functionality and is equivalent to the robot in the UPBOT robotics
 * system.  That is, in order for the nerves to start a Data Collector
 * service endpoint, it would need to make a similar call as seen
 * below.
 *
 * Others should use this demo for * guidance on how to utilize the
 * serv*() functionality available in * services.[c,h].
 *
 * @author Tanya L. Crenshaw
 * @since July 2013
 */

#include <stdio.h>
#include "connector.h"

int main(int argc, char * argv[])
{

  // Check command line parameters.
  if( ! (argc == 2 || argc ==3 ))
    {
      printf("This is a command line program that requires the interface name you'd like to communicate on, e.g., en1 or wlan0.  It also has an optional third parameter to manually set a remote ip address of the entity to whom you want to connect.  If no ip is given, it will run in broadcast mode. \n");
      
      printf("usage: %s <interface name> <optional remote ip>\n", argv[0]);
      return 0;
    }

  serviceHandler sh;

#ifdef GUMSTIX
  printf("\n\nIt is my belief that this producer demo is running on a gumstix.\n");
#endif

  // The first step is to set the default values for the
  // serviceHandler.
  servHandlerSetDefaults(&sh);

  // Manually set the remote ip if we have a third argument.
  if(argc == 3)
    {
      printf("...Executing program in manual mode using %s\n", argv[2]);
      servHandlerSetRemoteIP(&sh, argv[2]);

      // Start up a data service, collector endpoint.
      servStart(SERV_DATA_SERVICE_COLLECTOR, argv[1], SERV_BROADCAST_OFF, &sh);
    }
  
  else { 
      printf("...Executing program in broadcast mode.");

      // Start up a data service, collector endpoint.
      servStart(SERV_DATA_SERVICE_COLLECTOR, argv[1], SERV_BROADCAST_ON, &sh);
  }


  servHandlerPrint(&sh);

  dsWrite(&sh, "Hi!");

  return 0;

}
