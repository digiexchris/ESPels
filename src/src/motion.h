#pragma once

extern volatile bool feeding;

//TODO there's a direction enum that can be used, maybe do it at the same time the state machine is refactored
extern volatile bool feeding_ccw;

void init_motion(void);
void IRAM_ATTR calcDelta(void);
void IRAM_ATTR processMotion(void);
void start_rapid(double distance);
void init_hob_feed(void);
//void calcDelta(void *pvParaeters);

void init_pos_feed();