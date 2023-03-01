#include <Arduino.h>
#include "config.h"
#include <yasm.h>
#include "state.h"
#include "web.h"
#include "Controls.h"


void SlaveModeReadyState(){
   if(btn_yasm.isFirstRun()){
    updateMode(SLAVE_READY,RunMode::SLAVE_READY);
    web = true;
  }
}

void slaveStatusState(){
    if(btn_yasm.isFirstRun()){
        updateMode(DSTATUS,RunMode::RUNNING);
        web = false;
    }
}

void slaveFeedingState(){
    if(btn_yasm.isFirstRun()){

        // TODO: should check for RPM 0 for this state?
        updateMode(DSTATUS,RunMode::RUNNING);
        feeding = true;
        web = false;
    }

}
