/// @file PID_test.cpp 
///
/// HW7 - PID Integration
/// Paul Watt
///
//  ****************************************************************************
#include "drone.h"
#include "qcrecv.h"
#include "beacon.h"

#include "utility/robotics.h"
#include <stdlib.h>
#include <iostream>
using std::cout;
using std::endl;


//  ****************************************************************************
const unsigned int k_sample_rate_ms = 250;

//  ****************************************************************************
void set_system_state(rc_state_t state)
{
  rc_set_state(state);

  switch (state)
  {
  case UNINITIALIZED:
    rc_led_set(RC_LED_RED, 0);
    rc_led_set(RC_LED_GREEN, 0);
    break;
  case RUNNING:
    rc_led_set(RC_LED_GREEN, 1);
    break;
  case PAUSED:
    /// TODO: Set the LED to blink while paused.
    rc_led_set(RC_LED_GREEN, 0);
    break;
  case EXITING:
    rc_led_set(RC_LED_GREEN, 0);
    break;
  default:
    break;
  }
}

//  ****************************************************************************
void on_pause()
{
  rc_set_state(EXITING);
}

//  ****************************************************************************
void flag_error()
{
  /// TODO: Add error logging. For now, turn on the RED LED when an error occurs.
  rc_led_set(RC_LED_RED, 1);
}

//  ****************************************************************************
void clear_error()
{
  rc_led_set(RC_LED_RED, 0);
}

//  ****************************************************************************
int main()
{
  Drone drone;

  cout << "Drone Control Entry Point:\n\n"; 


  int status = rc_kill_existing_process(2.0);
  if (status < -2)
    return -1;

  set_system_state(UNINITIALIZED);

  status = rc_enable_signal_handler();
  if (status == -1)
  {
    cout  << "Error: " << status << "\n"
          << "Robotics API failed to initialize properly.\n";
    flag_error();
    return -1;
  	
  }


  // Transmit a beacon and wait for an action by the
  // user to move forward with drone initialization.

// TODO: Add a commandline option to bypass the beacon
  status = beacon_mode();
  if (status < 0)
  {
    return status;
  }

  // Initialization has been triggered.
  if (!drone.init())
  {
    cout  << "An error occurred during Drone::init()\n"
          << "  - Drone Control will now exit.\n";
    flag_error();
    return -1;
  }

  // Configure the buttons to trigger alternate states.
  rc_button_init(RC_BTN_PIN_PAUSE, 
                 RC_BTN_POLARITY_NORM_HIGH,
                 RC_BTN_DEBOUNCE_DEFAULT_US);
  rc_button_set_callbacks(RC_BTN_PIN_PAUSE, &on_pause, nullptr);

  // Set the CPU frequency to run steadily at the max.
  rc_cpu_set_governor(RC_GOV_PERFORMANCE);

  set_system_state(RUNNING);

  rc_make_pid_file();

  StartListening(&drone);
  while ( EXITING != rc_get_state()
       && IsListening())
  {
    // Report state to the controller.
    DroneState state = drone.state();
    ReportDroneState(state);

    // Wait the specified delay before requesting next data 
    // Convert the units to microseconds.
    usleep(k_sample_rate_ms * 1000); 
  }

  set_system_state(EXITING);

  cout << "Shutting down drone...\n"; 

  HaltListening();

  cout << "Terminating Drone Control Application.\n\n"; 
  cout.flush();

  // Restore the CPU frequency to "on demand"
  rc_cpu_set_governor(RC_GOV_ONDEMAND);

  rc_led_cleanup();
  rc_button_cleanup();
  rc_remove_pid_file();

  return 0;
}
