#include <Arduino.h>
#include "config.h"
#include "util.h"



/* Gutted
#include "display.h"

#include <Wire.h>
#include "SSD1306Wire.h"
#include "neotimer.h"
#include "state.h"
#include "Machine.h"
#include "Stepper.h"


SSD1306Wire  display(0x3c, 5, 4);
Neotimer display_timer = Neotimer(500);

int display_mode = STARTUP;


void clear(){
  display.clear();
  //display.flipScreenVertically();
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(ArialMT_Plain_10);
}
// TODO: figure out best way to deal with different displays or using Serial.

void init_display(){
//#ifdef USESSD1306
  // setup the display
  
  display.init();
  delay(200);
  clear();

  
//#endif

}

static String sp = String(" ");




// this is the feed mode
void do_startup_display(){
 
  display.drawString(0,0,"Display gutted ") ;
  display.display();
}

void do_ready_display(){
  display.drawString(0,0," Display gutted") ;
  display.display();
}
// this is the config mode
void do_configure_display(){
  display.drawString(0,0, "config: ");
  display.display();
}


void do_debug_ready_display(){
  display.drawString(0,0,"Debug Jog" + String(vel));
  display.drawString(0,11," Left to jog 10mm");
  //display.drawString(0,21," back: middle " + String(step_delta) );
  display.drawString(0,31,"J:"+ String(jogging) + " JS: " + String(jog_done));
  display.drawString(0,41,"s: " +String(jog_steps));
  display.display();
}

void do_status_display(){
  // Slave logging/feeding
  // this mode only moves slaved to spindle and with a direction button pressed
  display.drawString(0,0,"RPM:" + String(rpm) +  " Rapid: " + String(rapids) );
  display.drawString(0,11,  "Feed Pitch: " + String(pitch,2));
  display.drawString(0,21,"Pos: " + String(relativePosition))  ;
  display.drawString(0,31," Delta: " + String( (int32_t)delta ));
  display.display();
}

void do_display(){

  if(display_timer.repeat()){

    
    clear();
    switch (display_mode) {
      case STARTUP:
        do_startup_display();
        break;
      case CONFIGURE:
        do_configure_display();
        break; 
      case DSTATUS: 
        do_status_display();
        break;
      case READY:
        do_ready_display();
        break;
      case FEEDING:
        do_status_display();
        break;
      case DEBUG_READY:
        do_debug_ready_display();
        break;

    }
    
  }
  
}
*/  
