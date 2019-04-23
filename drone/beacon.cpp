/// @file beacon.cpp
///
/// Transmits a beacon at startup to locate and connect to the ground control station. 
///
/// If the user presses the "mode" button after both the red and green buttons
/// light up, the main Quad-copter program will be started. 
///
//  *****************************************************************************


#include <iostream>
#include <thread>
#include <vector>
#include <algorithm>

#include <fcntl.h>
#include <unistd.h>
#include <termios.h>

#include "../drone/qc_msg.h"
#include "../drone/serial.h"
#include "../drone/qcrecv.h"
#include <rc_usefulincludes.h> 
#include <roboticscape.h>

using std::cout;
using std::endl;
using std::vector;

namespace  // unnamed 
{

std::thread*  gp_transmitter  = nullptr;
const size_t  k_buffer_size   = 128;

}


//  ****************************************************************************
void on_mode_pressed();
void on_mode_released();
void set_system_state(rc_state_t state);

//  ****************************************************************************
void ReadMessage(int port)
{
  uint8_t buffer[k_buffer_size];
  
  int     len = read_message(port, buffer, k_buffer_size);

  if (len < 0)
  {
    cout << "-";
    cout.flush();
    return;
  }

  if (len == sizeof(QCBeaconAckMsg))
  {
    cout << "Received Control Beacon Response.\n";
    // Change the system state so it exits beacon mode.
    set_system_state(PAUSED);
  }
  else if (  EAGAIN      == errno
          || EWOULDBLOCK == errno)
  {
    cout << "-";
    cout.flush();
  }
}


//  ****************************************************************************
void TransmitBeacon()
{
  int result  = 0;
  int port    = open("/dev/ttyO1", O_RDWR | O_NOCTTY | O_NDELAY);

  if (port < 0)
  {
    cout << "Error - Cannot open serial port 1 to transmit beacon.\n";
    return;
  }

  fcntl(port, F_SETFL, FNDELAY);

  termios options;

  tcgetattr(port, &options);

  cfsetispeed(&options, B57600);
  cfsetospeed(&options, B57600);

  options.c_cflag &= ~(PARENB | CSIZE | CRTSCTS);
  options.c_cflag |= CS8 | CREAD;

  options.c_lflag &= ~(ICANON | ECHO | IEXTEN | ISIG);
  options.c_iflag &= ~(IXON | ISTRIP | INPCK | ICRNL | BRKINT);
  options.c_oflag &= ~(OPOST);

  options.c_cc[VMIN]  = 1;
  options.c_cc[VTIME] = 0;

  tcflush(port, TCIFLUSH);

  tcsetattr(port, TCSANOW, &options);

  QCBeaconMsg data_out = {0};

  while(UNINITIALIZED == rc_get_state())
  {
    uint8_t buffer[sizeof(data_out)];

    PopulateQCHeader(data_out);
    size_t offset = Serialize(data_out.header, buffer, sizeof(data_out));

    // TODO: Return and populate the cookie.
    data_out.cookie = htonl(1);
    ::memcpy(buffer + offset, (char*)&data_out.cookie, sizeof(data_out.cookie));
    offset += sizeof(data_out.cookie);

    result = write_message(port, buffer, sizeof(data_out));
    if (result < 0)
    {
      cout  << "Error: " << errno << ": " << strerror(errno) << "\n"
            << "Failed to send control beacon.\n";
      usleep(1000000);
      continue;
    }

    // Wait for one second for a response message.
    usleep(1000000);


    timeval  timeout = {0};
    timeout.tv_sec   = 1;

    fd_set  rfds;
    FD_ZERO(&rfds);
    FD_SET (port,&rfds);

    result = select(port+1, &rfds, 0, 0, &timeout);
    
    if (result < 0)
    {
      cout  << "Error: " << errno << ": " << strerror(errno) << "\n"
            << "Call to 'select' failed.\n";
      continue;    
    }
    else if (FD_ISSET(port, &rfds))
    {
      std::cout << "Select triggered, read message" << std::endl;
      // Read all of the current messages.
      ReadMessage(port);
    }

    cout << ".";
    cout.flush();
  }

  close(port);

}


// ******************************************************************************
int beacon_mode()
{
  int blink_counter = 0;

  // do your own initialization here
  cout  << "Beacon active...\n"
        << "Waiting for beacon response\n"
        << "  or\n"
        << "\"Mode\" button pressed to active QC\n" << endl;


  int status = rc_button_init(RC_BTN_PIN_MODE,
                              RC_BTN_POLARITY_NORM_HIGH,
                              RC_BTN_DEBOUNCE_DEFAULT_US);
  if (status > 0)
  {
    rc_button_set_callbacks(RC_BTN_PIN_MODE,
                            on_mode_pressed, 
                            on_mode_released);
        
  }
                 
 
  gp_transmitter = new std::thread(TransmitBeacon);

  // Keep looping until state changes to EXITING
  while(UNINITIALIZED == rc_get_state())
  {
    rc_led_set(RC_LED_GREEN, 0);
    rc_led_set(RC_LED_RED,   blink_counter%2 ? 0 : 1);

    // Delay for 1 second
    usleep(1000000);
    blink_counter++;
  }

  if (gp_transmitter)
  {
    gp_transmitter->join();

    delete gp_transmitter;
    gp_transmitter = nullptr;
  }

  return 0;
}


//  *****************************************************************************
/// Make the Mode button launch the QC application when released.
///
void on_mode_released()
{
  cout << "Mode button pressed.\n";
  set_system_state(PAUSED);
}

// *****************************************************************************
/// If the user holds the Mode button for 2 seconds, set state to exiting which
/// triggers the rest of the program to exit cleanly.
///
void on_mode_pressed()
{
  int i=0;
  const int samples = 100;	// check for release 100 times in this period
  const int us_wait = 2000000;  // 2 seconds

  // now keep checking to see if the button is still held down
  for(i=0;i<samples;i++)
  {
    rc_usleep(us_wait/samples);
    if (RC_BTN_STATE_RELEASED == rc_button_get_state(RC_BTN_PIN_MODE))
      return;
  }

  printf("long press detected, shutting down\n");
  rc_set_state(EXITING);

  return;
}

