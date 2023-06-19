/*
 * ui.h
 *
 *  Created on: Jun. 19, 2023
 *      Author: mrdri
 */

#ifndef SRC_UI_H_
#define SRC_UI_H_

_Bool initializeKnobStructs(void);
_Bool initializeButtonStructs(void);
void pollKnobs(void);
void debounceButtons(void);

#endif /* SRC_UI_H_ */
