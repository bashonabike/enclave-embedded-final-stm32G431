/*
 * floodlight.h
 *
 *  Created on: Jun. 19, 2023
 *      Author: mrdri
 */

#ifndef SRC_FLOODLIGHT_H_
#define SRC_FLOODLIGHT_H_

_Bool initializeFloodlightStructs(void);
_Bool isFloodlightRdy(void);

//Pass in new RGB values to update the floodlight colour
void updateFloodlight(uint8_t lightIdx, uint8_t R, uint8_t G, uint8_t B);

//Called in timer interrupt
void pulseFloodlight(void);

#endif /* SRC_FLOODLIGHT_H_ */
