## ESPels

A simple electronic lead screw for the esp32

this is currently in test mode and pre-alpha

Plan:

1. Make "move" command, start with Debug Jog move distance and direction.
1a. rethink DRO, need a new param to track when the tool actually moves in slave_jog mode.
2. get stops working
3. figure out how to sync thread start
4. implement full threading cycle

Cleanup:

1. rethink "modes", right now there are display_modes and state transitions.  The GUI's mode selection should select the state machine mode needed.  
  a. get rid of the menu thing.
2. Add speed factor for accel table

## Frontend

the webfrontend can be found here [https://github.com/jschoch/espELSfrontend]

## how

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
- [ ] - [ ] Add mm position to display
- [ ] Add feed to stop mode
- [ ] Add some js for frontend
- [ ] Add backlash compensation
- [ ] turn on task monitoring and display via web ui
- [ ] smooth RPM for display
- [ ] store position after disengaging slave so you can re-index if you re-engage
- [ ] Add angle measure mode like a manual turned indexing plate.


Misc:

add debug move 10mm via web ui when in debug mode
display mm
add diag screen for old display mode of steps vs mm
fix display wrapping
add slave mode
add jog mode
add encoder control
figure out why fault pin use makes screen stop working
