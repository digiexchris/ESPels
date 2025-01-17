#pragma once
#include "yasm.h"

extern YASM bounce_yasm;

extern volatile bool bouncing;
extern double old_jog_mm;

void do_state();
void BounceJogState();
void BounceIdleState();
void BounceRapidState();
