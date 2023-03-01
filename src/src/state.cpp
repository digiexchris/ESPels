#include "state.h"
//Initialize the starting memory. 
//TODO: move this into a class constructor

//common variables used by multipe things
volatile double jog_mm = 0;
volatile int rpm = 0;
double mmPerStep = 0;
int32_t stepsPerMM = 0;
int32_t relativePosition = 0;
int32_t absolutePosition = 0;

volatile bool z_feeding_dir = true;
volatile double targetToolRelPosMM = 0.0;
volatile double toolRelPosMM = 0.0;

//tool position in steps
volatile int64_t toolPos = 0;

//State Machine stuff
bool syncStart = true;
bool syncWaiting = false;
volatile bool jogging = false;
volatile bool rapiding = false;
volatile bool feeding = false;

//TODO there's a direction enum that can be used, maybe do it at the same time the state machine is refactored
volatile bool feeding_ccw = true;

RunMode run_mode = RunMode::STARTUP;