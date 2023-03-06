#include "state.h"
//Initialize the starting memory. 
//TODO: move this into a class constructor

//common variables used by multipe things
volatile int rpm = 0;
double mmPerStep = 0;
int32_t stepsPerMM = 0;
int32_t relativePosition = 0;
int32_t absolutePosition = 0;
bool sendDebug = false;

volatile bool z_feeding_dir = true;

//tool position in steps
volatile int64_t toolPos = 0;

//State Machine stuff
bool syncStart = true;
bool syncWaiting = false;
volatile bool jogging = false;
volatile bool rapiding = false;


RunMode run_mode = RunMode::STARTUP;
