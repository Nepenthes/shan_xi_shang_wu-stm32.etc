/*----------------------------------------------------------------------------
 * CMSIS-RTOS 'main' function template
 *---------------------------------------------------------------------------*/
#define osObjectsPublic                    		 // define objects in main module
#include "includes.h"

int main (void const *argument)
{
	
	osKernelInitialize ();                    	// initialize CMSIS-RTOS
	
	// initialize peripherals here

	// create 'thread' functions that start executing,
	// example: tid_name = osThreadCreate (osThread(name), NULL);
	
		BSP_Init();
		
		wirelessThread_Active();
		
		MoudleDEC_Init();
		
		LCD144Disp_Active();
		
		keyMboardActive();
		
		tipsLEDActive();
	
	osKernelStart ();                         	// start thread execution 
}
