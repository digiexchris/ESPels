#pragma once
//State that's common to multiple contexts. 
//TODO: (Chris) Eventually move this to a class, perhaps the state machine. Then we don't have to 
// extern all these things to make the linker happy, and each object can guarantee nothing else
// is writing to this memory when it shouldn't be (makes future expansion easier)
//TODO: identify which of these belong to other objects and make those objects "own" these
#include "Encoder.h"
#include "Controls.h"
#include "genStepper.h"
#include "moveConfig.h"

//common variables used by multipe things
extern volatile int rpm;
extern double mmPerStep;
extern int32_t stepsPerMM;
extern int32_t relativePosition;
extern int32_t absolutePosition;
extern bool sendDebug;


extern GenStepper::State gs;
extern MoveConfig::State mc;

//State Machine stuff
extern bool syncStart;
extern bool syncWaiting;
extern volatile bool jogging;
extern volatile bool rapiding;

extern RunMode run_mode;

extern double lead_screw_pitch;
// extern int microsteps;
// extern int native_steps;
// extern int motor_steps;

//these 3 probably belong in a thread class or struct
extern int tpi;
extern double pitch; //  the thread pitch
extern double depth;                                       // a parameter to define the thread depth in mm on the compound slide. This is set at 75% of the pitch which seems to work

// tracks the current tool position based on delivered steps
extern volatile int64_t toolPos;
