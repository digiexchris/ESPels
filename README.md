## ESPels

A simple electronic lead screw for the esp32

this is currently in test mode and pre-alpha

[b] DANGER 
There is no estop right now, not safe.  Use at your own risk and hover over the lathe e-stop.  The thing could go crazy and crash the carriage into the spindle at any moment.  It shouldn't but bugs....

[demo](https://www.youtube.com/watch?v=uXhqEe8Kw6M&list=PLvpLfzys-jPumkXZj8ZZn11zyY3UYtSkn&index=6)

Disclaimer out of the way: i've been using it for a week and it has not freaked out yet.

## Frontend

![sample](https://user-images.githubusercontent.com/20271/104212553-33b6c200-53ea-11eb-899b-3ec2c22e56c2.png)

the webfrontend can be found here [https://github.com/jschoch/espELSfrontend]



## Configuration


Wifi:  ssid and password need to be defined, I did this via a .h file outside of the project

in config.h update Z_STEP_PIN, Z_DIR_PIN for your stepper.
Update  EA, and EB for your encoder signals.  Let me know if you want to have an index pulse defined as my encoder does not have one.


## HOW

Initially I was planning on making a pcb with buttons to control everything.  This is messy and hard to update.  Now I'm using websockets and a react SPA to control everything.  The websockets transmit json and msgPack for configuration and status updates.  This is working very nicely but there is some lag.

Because there is no feedback from a touch screen I plan on integrating an optional haptic input controller which will have bidirectional communication with the ELS controller.  


This is currently using a version of the "didge" algorythm here (https://github.com/prototypicall/Didge/blob/master/doc/How.md_).  The slope can't be > 1 so you may need to reduce your micro stepping.


## old depricated how

The control state machine waits for button state changes and changes modes based on input.  There will be several modes.  The general operating mode model is that we track the spindle position via the quadrature encoder.  This position is mapped to a int64_t of stepper steps.  The tool position is also tracked in steps.  When the motor is slaved to the spindle the motor control interrupt timer will calculate how many steps away from it's ideal position it is based on the current pitch setting.  This is calculated as 

```
motor_steps = microsteps * native_steps;

# current steps per revolution
factor= (motor_steps*pitch)/(lead_screw_pitch*spindle_encoder_resolution); 

# current position
calculated_stepper_pulses = (int32_t)(factor * encoder.getCount());

# the distance in steps from the ideal position
delta = toolPos - calculated_stepper_pulses; 
```

Once the delta is calculated and > 1, direction is determined and stepper pulses are generated until the delta is zero.  This works with the motor on or off, hand turning it or spinning at 2krpm.  There is not currently any acceleration done.  If the spindle stops cold (unlikely) the motor will get no step pulses.  If the spindle motor is already running at 2krpm and you try to set the pitch to 7mm it isn't likely to work out (just like on a real lathe).  This allows the things to bet setup in a simple slave mode where it operates like a regular lathe and the stepper acceleration is derived from the spindle motor's acceleration... Simple!

steps are generated by a timer, this could be offloaded to RMT or i2s.  This seems to work well but it is likely the steps generated have some small offset based on the nature of the period of the quadrature signal.

quadrature is done via the pulse counter periferal via the esp32encoder library.  It can support up to 10 encoders.  RPM measurement should be somewhat smoothed for display.

Buttons are updated in the main loop by the Bounce2 library.  They are polled for rise/fall events to drive the control state machine.  The lolin32 i'm using for the prototype doesn't have enough pins and a multiplexer may be implemented.

The control state machine uses the YASM library.

The display is integrated on the lolin32 (ssd1306), the display will be abstracted so that other displays can be used.


### how stepper.cpp works

Step signals run in onTimer() at frequency 80MHz
There are two types of movement, slaved and free.  Slaved movement is movement tied to the spindle encoder's position.  Speed and accel are determined by the spindle.   Free movement is independent of the spindle encoder.

Init_stepper() creates the timer and sets up the STEP/DIR pins.

The boolean values 'feeding' and 'jogging' control the stepper functions.

`read_buttons()` will update the position button_timer's frequency in the `toolRelPosMM` value.

####Slave Mode:

'feeding' drives the slave mode.  If 'feeding' is set to true the motor will move.   It is set  in Controls.cpp and SlaveMode.cpp.

'feeding_dir' will signal a pause before direction changes as well as modify the direction of travel.    

The value 'toolPos' determines the Z position.  It essentially chases an approximation of the correct position through a delta value.  If the delta value is !0 the stepper will attempt to move the delta to zero.  The toolPos value MUST be reset when slave jogging begins.  The start routine should be 

```
    Serial.println("status -> feeding left");
    resetToolPos();
    feeding_dir = true;
    feeding = true;
    btn_yasm.next(feedingState);
```
Btn_yasm is a state machine to deal with transitions and track machine state.

####Free Mode:

If `jogging` and `!jog_done` are both true the controller will continue trying to step `jog_steps` decrementing the value.  When `jog_steps` is zero jog_done is set to true.  

Plan:

1. [x] update DRO with toolRelMM 
1. [x] Slave Jog MM stepper mode
2. [x] get stops working, slave jog to stop mode
3. figure out how to sync thread start
4. implement full threading cycle

Cleanup:

1. rethink "modes", right now there are display_modes and state transitions.  The GUI's mode selection should select the state machine mode needed.  
  a. get rid of the menu thing.



** TODO

- [x ] Add HTML parameter updates
- [x ] Turn web off when in feed modes
- [ ] Add rapid jog when Mod btn is pressed
- [ ] Add optional encoder wheel input
- [ ] Add EEPROM saving of params
- [ ] Add virtual stops
- [ ] Add feed to stop mode
- [ ] Add feed to distance (mm) mode
- [ ] Add physical stops (adjustable hall sensors for example)
- [x] Add mm position to display
- [ ] Add feed to stop mode
- [x ] Add some js for frontend
- [ ] Add backlash compensation
- [ ] turn on task monitoring and display via web ui
- [ ] smooth filter EMA RPM for display
- [ ] store position after disengaging slave so you can re-index if you re-engage
- [ ] Add angle measure mode like a manual turned indexing plate.


Misc:

