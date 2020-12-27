#include "Controls.h"

#include "config.h"

#define BOUNCE_LOCK_OUT
#include <Bounce2.h>
#include "neotimer.h"
#include <ArduinoMap.h>
#include <yasm.h>
#include <Vector.h>
#include "rmtStepper.h"

Neotimer button_read_timer = Neotimer(10);
Neotimer button_print_timer = Neotimer(2000);
Neotimer dro_timer = Neotimer(200);



uint8_t menu = 3; 
int pitch_menu= 1;
volatile bool feeding = false;
volatile bool feeding_dir = true;



const int MENU_MAX = 5;
Menu feed_menu_items;
const char* feed_menu_storage[MENU_MAX];
int feed_mode_select = 0;

#ifdef DEBUG_CPU_STATS
char stats[ 2048];
#undef configGENERATE_RUN_TIME_STATS
#define configGENERATE_RUN_TIME_STATS 1
#undef configUSE_STATS_FORMATTING_FUNCTIONS
#define configUSE_STATS_FORMATTING_FUNCTIONS 1
#endif

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

int32_t stepsPerMM = 0;
int32_t relativePosition = 0;
int32_t absolutePosition = 0;

void make_menu(){
  
  feed_menu_items.setStorage(feed_menu_storage);
  feed_menu_items.push_back("Slave");
  feed_menu_items.push_back("Slave Jog");
  feed_menu_items.push_back("Feed To Stop");
  feed_menu_items.push_back("Thread");
  feed_menu_items.push_back("Debug");

}
void menu_next(int *index, Menu *menu){
  *index = *index + 1;
  if(*index >= (int)menu->size()){
    *index = 0;
  }
}

void menu_prev(int *index, Menu *menu){
  *index = *index -1;
  if(*index < 0){
    *index = (int)menu->size();
  }
}

// We don't want to go from 0.05 mm feed to 7.0mm feed!!!!!
void menu_prev_nowrap(int *index, Menu *menu){
  *index = *index - 1;
  if(*index < 0){
    *index = 0;
  }
}

void init_controls(){
  make_menu();
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
  updatePosition();
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
    /*
    Serial.print("enc: ");
    Serial.print((int)encoder.getCount());
    Serial.print(",target:");
    Serial.print(targetToolRelPos);
    Serial.print(",delta: ");
    Serial.print((int)delta);
    Serial.print(",TP: ");
    Serial.print((int)toolPos);
    Serial.print(",calc: ");
    Serial.print((int)calculated_stepper_pulses);
    Serial.print(",feed "); 
    Serial.print(feeding);
    Serial.print(",jogging: ");
    Serial.print(jogging);
    Serial.print(",jog_done: ");
    Serial.println(jog_done);
    */
    Serial.print("%");
    updateStatusDoc();

  }
  if(dro_timer.repeat()){
    updatePosition();
    updateStatusDoc();
  }
     
}

void updateMode(display_mode_t newDisplayMode,RunMode run){
  Serial.print("updating modes: ");
  Serial.print(display_mode);
  Serial.print(":");
  Serial.println((int)run);
  display_mode = newDisplayMode;
  run_mode = run;
  updateConfigDoc();
}

void startupState(){
  if(btn_yasm.isFirstRun()){
    updateMode(STARTUP,RunMode::STARTUP);
    web = true;
  }
  if(lbd.deb->rose()){
    
    
    if(strcmp(feed_menu_items[feed_mode_select],"Slave")== 0){
      Serial.println("startup -> slave ready");
      btn_yasm.next(SlaveModeReadyState);
    }
    if(strcmp(feed_menu_items[feed_mode_select],"Slave Jog")== 0){
      // TODO: break these out
      Serial.println("startup -> ready");
      btn_yasm.next(slaveJogReadyState);
    }
    if(strcmp(feed_menu_items[feed_mode_select],"Feed To Stop")== 0){

    }
    if(strcmp(feed_menu_items[feed_mode_select],"Thread")== 0){

    }
    if(strcmp(feed_menu_items[feed_mode_select],"Debug")== 0){
      Serial.println("startup -> debug ready");
      btn_yasm.next(debugState);
    }

    
  }
  if(ubd.deb->rose()){
    menu_next(&feed_mode_select,&feed_menu_items);
  }
  if(dbd.deb->rose()){
    menu_prev(&feed_mode_select,&feed_menu_items); 
  }
}

void slaveJogReadyState(){
  if(btn_yasm.isFirstRun()){
    updateMode(READY,RunMode::SLAVE_JOG_READY);
    setFactor();
    web = true;
  }
  if(lbd.deb->rose()){
    
    Serial.println("ready -> status");
    
    btn_yasm.next(slaveJogStatusState);
  }

}

/*

When in slaveJogReadyState if the webUI sends the "do jog" this state is entered.
Display should prompt user to press a button to execute the jog
*/
void slaveJogPosState(){
  if(btn_yasm.isFirstRun()){
    // TODO update the modes correctly
    //updateMode()

    // TODO: should the webui be disabled?
    // web = false;
    Serial.println("entering slaveJogPosState() press sbd to start jogging");
    //setFactor();
  }
  if(sbd.deb->rose() || feeding == false){
    Serial.println("start slave jog to position");
    updatePosition();
    btn_yasm.next(slaveJogReadyState);
  }

}

// This is "slave jog" status mode, "slave" status is in SlaveMode.cpp
void slaveJogStatusState(){
  if(btn_yasm.isFirstRun()){
    updateMode(DSTATUS,RunMode::RUNNING);
    web = false;
  }

  if(lbd.deb->fell()){
    Serial.println("status -> feeding left");
    // TODO: sync to spindle rotation
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
    pitch_menu++;
    feed_parameters();
  }
  if(dbd.deb->rose()){
    Serial.println("DOWN");
    pitch_menu--;
    feed_parameters();
  }
}


//  slave jog mode feeding state
void feedingState(){
  if(btn_yasm.isFirstRun()){
    updateMode(FEEDING,RunMode::RUNNING);
    Serial.println("enter feedingState");
    // set the feeding flag for the stepper.
  }

  // if the button is released stop moving
  if(lbd.deb->rose() || rbd.deb->rose()){
    Serial.println("feeding -> status");
    feeding = false;
    btn_yasm.next(slaveJogStatusState);
  }
}

void setFactor(){

  // TODO: timer refactor  get rid of factor and all related code
  //factor= (motor_steps*pitch)/(lead_screw_pitch*spindle_encoder_resolution);            
  stepsPerMM = motor_steps / lead_screw_pitch;
  mmPerStep = (double) 1/stepsPerMM;
  int den = lead_screw_pitch * spindle_encoder_resolution ;
  int nom = motor_steps * pitch;

  Serial.printf("nom: %d den: %d",nom,den);
    if (!xstepper.gear.setRatio(nom,den)){
      // TODO:  send error to GUI
      Serial.println(" ratio no good!!!!  too big!!!!");
      pos_feeding = false;
      btn_yasm.next(startupState);
      return;
    }

  updatePosition();
  /*   TODO: add back imperial threads
  else
    {
    if(menu<20)
      {
        // the depth of cut in mm on the compound slide I need for each thread pitch.  
        // I use this during operation rather than looking it up each time

        depth=pitch_factor*25.4/tpi;

        // the imperial factor needed to account for details of lead screw pitch, 
        // stepper motor #pulses/rev and encoder #pulses/rev
        factor= motor_steps*25.4/(tpi*lead_screw_pitch*spindle_encoder_resolution);  // imperial
        }
      else
        {
        // the depth of cut in mm on the compound slide
        depth=pitch_factor*pitch; 
        // the metric factor needed to account for details of lead screw pitch, 
        // stepper motor #pulses/rev and encoder #pulses/rev
        factor=pitch*motor_steps/(lead_screw_pitch*spindle_encoder_resolution); // keep redundant
        }
      }
  */              
} 

//this defines the parameters for the thread and turning for both metric and imperial threads

void thread_parameters()                                           
  { 
    /* TODO: add back when you work on thread cycle
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
  //toolPos = factor * encoder.getCount();
  setFactor();

  */
}

void feed_parameters(){
  
  if(pitch_menu> 21) pitch_menu= 1;

  // do not wrap!!
  if(pitch_menu< 1) pitch_menu= 1;

  switch(pitch_menu) {
    case(1):     pitch=0.05;                  break;  // Normal Turning
    case(2):     pitch=0.085;                  break;  // Fine Turning
    case(3):     pitch=0.16;                  break;  // Coarse Turning
    case(4):     pitch=0.2;                  break;  // Coarse Turning
    case(5):    pitch=0.4;   break;      
    case(6):    pitch=0.4;   break;      
    case(7):    pitch=0.5;   break;      
    case(8):    pitch=0.7;   break;      
    case(9):    pitch=0.75;  break;      
    case(10):    pitch=0.8;   break;      
    case(11):    pitch=1.0;   break;      
    case(12):    pitch=1.25;  break;      
    case(13):    pitch=1.5;   break;      
    case(14):    pitch=1.75;  break;      
    case(15):    pitch=2.0;   break;      
    case(16):    pitch=2.5;   break;      
    case(17):    pitch=3.0;   break;      
    case(18):    pitch=3.5;   break;      
    case(19):    pitch=4.0;   break;      
    case(20):    pitch=5.0;   break;
    case(21):    pitch=7.0;   break;
  }

  setFactor();
  // TODO:  this needs some thought
  //toolPos = factor * encoder.getCount();
  updateConfigDoc();
}
