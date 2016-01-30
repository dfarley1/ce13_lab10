//Daniel Farley, dfarley@ucsc.edu

#ifndef LEDS_H
#define	LEDS_H

#include <xc.h>

//Initializes the LEDs to be off
#define LEDS_INIT() (TRISE = LATE = 0x0)

//Returns the current status (on/off) of the LEDs
#define LEDS_GET() (PORTE & 0xFF)

//Sets the LEDs to the passed value
#define LEDS_SET(num) (LATE = (num))

#endif	// LEDS_H
