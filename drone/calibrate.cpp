/// @file PID_test.cpp 
///
/// HW7 - PID Integration
/// Paul Watt
///
//  ****************************************************************************
#include "PWM.h"

#include "utility/robotics.h"
#include <stdlib.h>
#include <iostream>
#include <termios.h>

using std::cout;
using std::endl;
using std::cin;


//  ****************************************************************************
int main()
{
  cout << "Starting ESC Calibration:\n\n"; 

  int status = rc_initialize();
  if (0 != status)
  {
    cout  << "Error: " << status << "\n"
          << "Robotics API failed to initialize properly.\n";
  }

	rc_send_esc_pulse_normalized_all(0);
	rc_usleep(50/1000000);

  bool toggle = false;
	while(rc_get_state()!=EXITING)
  {
    rc_send_esc_pulse_normalized_all(0);
		// blink green led
		rc_set_led(GREEN, toggle);
		toggle = !toggle;
		
		// sleep roughly enough to maintain frequency_hz
		rc_usleep(1000000/50);
	}
//  while (toggle);

  cout << "Setting motor level to max."; 

  rc_send_esc_pulse_normalized_all(1.0);

  cout << "Motors armed.\n"; 

  cout << "Hit 'L' to change to LOW\n"; 
  cout << "Hit 'H' to change to HIGH\n"; 
  cout << "Hit 'Q' to quit\n\n"; 

  char input = 0;

  bool done = false;
  do
  {
    cin >> input;

    switch (input)
    {
    case 'H':
    case 'h':
      cout << "Change to HIGH\n"; 
      rc_send_esc_pulse_normalized_all(1.0);
      break;

    case 'L':
    case 'l':
      cout << "Change to LOW\n"; 
      rc_send_esc_pulse_normalized_all(0.0);
      break;
    
    case 'Q':
    case 'q':
      done = true;
      break;

    default:
      break;
    }

  
  }
  while (!done);


  cout << "Exiting ESC Calibration.\n\n"; 

  rc_cleanup();

  return 0;
}
