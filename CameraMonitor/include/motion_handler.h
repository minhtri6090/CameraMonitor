#ifndef MOTION_HANDLER_H
#define MOTION_HANDLER_H

#include "config.h"

extern bool systemReady;
extern bool motionDetected;
extern unsigned long lastMotionTime;
extern const unsigned long motionCooldown;

extern int pirState;
extern int pirVal;

void initializePIR();
void handleMotionLoop();  
void getPIRStatus();
void IRAM_ATTR motionISR();

#endif