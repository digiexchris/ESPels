#include "Controls.h"
#include "config.h"

#define BOUNCE_LOCK_OUT
#include <Bounce2.h>
#include "neotimer.h"
#include <ArduinoMap.h>
#include <yasm.h>


Neotimer button_read_timer = Neotimer(10);
Neotimer button_print_timer = Neotimer(500);

int32_t left_limit_max = 2147483646;
volatile int32_t left_limit = 2147483646;
uint8_t btn_mode = FEED;
uint8_t menu = 33; 
volatile bool button_left = false;

int feed_menu = 1;
volatile bool feeding = false;
volatile bool feeding_dir = true;


YASM btn_yasm;

enum class BtnState{
  Startup,
  Ready,
  Status,
  Feeding
}; 

BtnState btnState = BtnState::Startup;

// Debouncer
Bounce debLBP = Bounce(); 
Bounce debRBP = Bounce();
Bounce debSBP = Bounce();
Bounce debUBP = Bounce();
Bounce debDBP = Bounce();
Bounce debMBP = Bounce();

const int NUM_BUTTONS = 6;
// Make a structure for button states
CreateMap(button_states, const char*, int, NUM_BUTTONS);

// button data definition

struct Bd{
  uint8_t pin;
  const char* name;
  bool changed;
  bool change_read;
  bool state;
  Bounce *deb;
  uint8_t idx;
};

// Button data structs

Bd lbd  {LBP,"left",false,true,true,&debLBP,0};
Bd rbd  {RBP,"right",false,true,true,&debRBP,1};
Bd ubd  {UBP,"up",false,true,true,&debUBP,2};
Bd dbd  {DBP,"down",false,true,true,&debDBP,3};
Bd sbd  {SBP,"menu",false,true,true,&debSBP,4};
Bd mbd  {MBP,"mod",false,true,true,&debMBP,5};

Bd bdata[NUM_BUTTONS] = {
    lbd,
    rbd,
    ubd,
    dbd,
    sbd,
    mbd
  };

int mode_select = FEED;


void init_controls(){

    uint16_t interval = 20;
  for(int i = 0; i < NUM_BUTTONS;i++){
    Bd bd = bdata[i];
    bd.deb->attach(bd.pin,INPUT_PULLUP);
    bd.deb->interval(interval);
    button_states[bd.name] = i;
  }
  btn_yasm.next(startupState);  

  Serial.println(display_mode);
}

void resetToolPos(){
  toolPos = factor * encoder.getCount();
}

void debugButtons(){
  for(int i = 0; i < NUM_BUTTONS;i++){
    Bd bd = bdata[i];
    
    
    Serial.print(bd.name);
    Serial.print(",");
    //Serial.print(bd.deb->read());
    //Serial.print(",");
    Serial.print(digitalRead(bd.pin));
    Serial.print(":");
    Serial.print(bd.deb->duration());
    Serial.print(",");
    
    Serial.print(bd.changed);
    Serial.println();
  }
}

void read_buttons(){
  if(button_read_timer.repeat()){
    for(int i = 0; i < NUM_BUTTONS;i++){
      Bd bd = bdata[i];
      bd.deb->update();
    }
    btn_yasm.run();
  }
  if(button_print_timer.repeat()){
    //debugButtons();
  }
     
}

void startupState(){
  if(btn_yasm.isFirstRun()){
    display_mode = STARTUP;
  }
  if(lbd.deb->rose()){
    Serial.println("startup -> ready");
    
    btn_yasm.next(readyState);
  }
}

void readyState(){
  if(btn_yasm.isFirstRun()){
    display_mode = READY;
  }
  if(lbd.deb->rose()){
    
    Serial.println("ready -> status");
    
    btn_yasm.next(statusState);
  }
}

void statusState(){
  if(btn_yasm.isFirstRun()){
    display_mode = DSTATUS;
  }

  if(lbd.deb->fell()){
    Serial.println("status -> feeding left");
    resetToolPos();
    feeding_dir = true;
    feeding = true;
    
    btn_yasm.next(feedingState);
    return; 
  }
  if(rbd.deb->fell()){
    Serial.println("status -> feeding right");
    resetToolPos();
    feeding_dir = false;

    feeding = true;

    btn_yasm.next(feedingState);
    return;
  }
  if(sbd.deb->rose() ){
    Serial.println("feeding -> startup");
    
    btn_yasm.next(startupState);
  }

  if(ubd.deb->rose()){
    feed_menu++;
    feed_parameters();
  }
  if(dbd.deb->rose()){
    feed_menu--;
    feed_parameters();
  }
}

void feedingState(){
  if(btn_yasm.isFirstRun()){
    display_mode = FEEDING;
    Serial.println("enter feedingState");
    // set the feeding flag for the stepper.
  }
  if(lbd.deb->rose() || rbd.deb->rose()){
    Serial.println("feeding -> status");
    feeding = false;
    btn_yasm.next(statusState);
  }
}

void setFactor(){
  if(menu<4){
    factor= (motor_steps*pitch)/(lead_screw_pitch*spindle_encoder_resolution);            
  }
  else
    {
    if(menu<20)
      {
        // the depth of cut in mm on the compound slide I need for each thread pitch.  
        // I use this during operation rather than looking it up each time

        depth=pitch_factor*25.4/tpi;

        // the imperial factor needed to account for details of lead screw pitch, 
        // stepper motor #pulses/rev and encoder #pulses/rev
        factor= motor_steps*25.4/(tpi*lead_screw_pitch*spindle_encoder_resolution);  
        }
      else
        {
        // the depth of cut in mm on the compound slide
        depth=pitch_factor*pitch; 
        // the metric factor needed to account for details of lead screw pitch, 
        // stepper motor #pulses/rev and encoder #pulses/rev
        factor=pitch*motor_steps/(lead_screw_pitch*spindle_encoder_resolution);
        }
      }
                                                     
} 

//this defines the parameters for the thread and turning for both metric and imperial threads

void thread_parameters()                                           
  { 
  switch(menu) {
    case(1):     pitch=0.085;                  break;  // Normal Turning
    case(2):     pitch=0.050;                  break;  // Fine Turning
    case(3):     pitch=0.160;                  break;  // Coarse Turning
    //...........................................................................................imperial data              
    case(4):     tpi=11;   break;
    case(6):     tpi=12;   break;
    case(7):     tpi=13;   break;
    case(8):     tpi=16;   break;
    case(9):     tpi=18;   break;
    case(10):    tpi=20;   break;
    case(11):    tpi=24;   break;
    case(12):    tpi=28;   break;
    case(13):    tpi=32;   break;
    case(14):    tpi=36;   break;
    case(15):    tpi=40;   break;
    case(16):    tpi=42;   break;
    case(17):    tpi=44;   break;
    case(18):    tpi=48;   break;
    case(19):    tpi=52;   break;
    //.............................................................................................metric data               
    case(20):    pitch=0.4;   break;      
    case(21):    pitch=0.5;   break;      
    case(22):    pitch=0.7;   break;      
    case(23):    pitch=0.75;  break;      
    case(24):    pitch=0.8;   break;      
    case(25):    pitch=1.0;   break;      
    case(26):    pitch=1.25;  break;      
    case(27):    pitch=1.5;   break;      
    case(28):    pitch=1.75;  break;      
    case(29):    pitch=2.0;   break;      
    case(30):    pitch=2.5;   break;      
    case(31):    pitch=3.0;   break;      
    case(32):    pitch=3.5;   break;      
    case(33):    pitch=4.0;   break;      
    case(34):    pitch=5.0;   break;
    case(35):    pitch=7.0;   break;
    }
  // TODO: this is a bit of a hack, changing feed changes the factor which changes the delta.  not sure of a good way to update this and maintain positions.
  toolPos = factor * encoder.getCount();
  setFactor();
}

void feed_parameters(){
  
  if(feed_menu > 4) feed_menu = 1;
  if(feed_menu < 1) feed_menu = 4;

  switch(feed_menu) {
    case(1):     pitch=0.05;                  break;  // Normal Turning
    case(2):     pitch=0.085;                  break;  // Fine Turning
    case(3):     pitch=0.16;                  break;  // Coarse Turning
    case(4):     pitch=0.2;                  break;  // Coarse Turning
  }
  toolPos = factor * encoder.getCount();
  factor= (motor_steps*pitch)/(lead_screw_pitch*spindle_encoder_resolution); 
}
