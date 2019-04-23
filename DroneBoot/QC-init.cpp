/*******************************************************************************
* QC-init.c
*
* System Startup program for the Quad-copter.
* This program is designed to launch at system startup. 
*
* If the user presses the "mode" button after both the red and green buttons
* light up, the main Quad-copter program will be started. 
*
*******************************************************************************/

extern "C"
{

#include <rc_usefulincludes.h> 
#include <roboticscape.h>

}

#include <iostream>
#include <thread>

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "../drone/qc_msg.h"


using std::cout;
using std::endl;

namespace  // unnamed 
{

std::thread*  gp_transmitter  = nullptr;

uint32_t      g_control_port  = 8101;
int           g_error         = 0;
uint16_t      g_next_sequence = 0;

uint32_t      g_beacon_address= 0xC0A802FF;  // 192.168.2.255
uint32_t      g_drone_address = 0xC0A80271;  // 192.168.2.113

}


// function declarations
void on_mode_pressed();
void on_mode_released();


//  ****************************************************************************
uint16_t GetSequenceId()
{
  return g_next_sequence++;
}


//  ****************************************************************************
void DroneInit()
{
  //system("../drone/QC");

  system("/home/debian/drone/QC");

  // Wait for a few seconds. 
  // If the utility does not launch, flag an error.
  usleep(2000000);
  g_error = 1;
}


//  ****************************************************************************
void TransmitBeacon()
{
  int result = 0;
  int sock   = socket(AF_INET, SOCK_DGRAM, 0);
  if (sock < 0) 
  {
    cout  << "Error - Cannot create socket to transmit beacon.\n";
	  return;
  }

  sockaddr_in addr      = {0};

  addr.sin_family       = AF_INET;
  addr.sin_port         = htons(g_control_port);
  addr.sin_addr.s_addr  = htonl(g_drone_address);
  //if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) 
  //{
  //  cout  << "Error (" << errno 
  //        << "): Cannot bind to interface for the drone.\n";
  //  return;
  //}

  //addr.sin_addr.s_addr  = htonl(0xC0A80272);

  QCBeaconMsg data_out = {0};

  PopulateQCHeader(data_out);

  //char enabled = 1;
  //result = setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &enabled, sizeof(enabled));
  //if (result < 0)
  //{
  //  cout  << "Error: " << errno << "\n"
  //        << "Failed to configured interface for broadcast mode.\n";
  //}


  //addr.sin_addr.s_addr  = htonl(INADDR_BROADCAST);
  //result = sendto(sock,
  //                    
  //                "Hello",
  //                5,
  //                0,
  //                (const sockaddr*)(&addr),
  //                sizeof(addr));
  //cout << "Hello test: " << result << "\n";

  while(EXITING != rc_get_state())
  {
    // TODO: Return and populate the cookie.
    data_out.cookie = htonl(1);

    // Broadcast the beacon packet looking for a control station.
    result = sendto(sock,
                    (const char*)(&data_out),
                    sizeof(data_out),
                    0,
                    (const sockaddr*)(&addr),
                    sizeof(addr));
    if (result < 0)
    {
      cout  << "Error: " << errno << "\n"
            << "Failed to send control beacon.\n";
      usleep(1000000);
      continue;
    }

    // Wait for one second for a response message.
    timeval  timeout = {0};
    timeout.tv_sec   = 1;

    fd_set rfds;
    FD_ZERO(&rfds);
    FD_SET(0,&rfds);

    result = select(sock, &rfds, 0, 0, &timeout);
    if (result < 0)
    {
      cout  << "Error: " << errno << "\n"
            << "Call to 'select' failed.\n";
      continue;    
    }
    else if (FD_ISSET(0, &rfds))
    {
      cout << "Attempting to read\n";
      // If one is not received, loop and send another beacon.
      const size_t  k_buffer_size = 128;
      uint8_t       buffer[k_buffer_size];     
      sockaddr_in   xmit_addr = {0};     
      socklen_t     addr_len  = sizeof(xmit_addr);

      int len = recvfrom(sock, 
                         buffer, 
                         k_buffer_size, 
                         0, 
                         (sockaddr *)&xmit_addr, 
                         &addr_len);
      if (len == sizeof(QCBeaconAckMsg))
      {
        // TODO: Extract the address of the sender.
        DroneInit();
      }
    }

    cout << ".";
  }

  close(sock);
  sock = 0;
}


// ******************************************************************************
/// This template main function contains these critical components
/// - call to rc_initialize() at the beginning
/// - main while loop that checks for EXITING condition
/// - rc_cleanup() at the end
///
int main()
{
  int blink_counter = 0;

  // always initialize cape library first
  if(rc_initialize())
  {
    fprintf(stderr,"ERROR: failed to initialize rc_initialize(), are you root?\n");
    return -1;
  }

  // do your own initialization here
  printf("\nQC-Init waiting for \"Mode\" button pressed to start QC.\n");
  rc_set_mode_pressed_func (&on_mode_pressed);
  rc_set_mode_released_func(&on_mode_released);

  gp_transmitter = new std::thread(TransmitBeacon);

  // done initializing so set state to RUNNING
  rc_set_state(RUNNING);

  // Keep looping until state changes to EXITING
  while(EXITING != rc_get_state())
  {
    if (0 == g_error)
    {
      rc_set_led(GREEN, blink_counter%2 ? ON : OFF);
    }
    else
    {
      rc_set_led(GREEN, OFF);
    }

    rc_set_led(RED,   blink_counter%2 ? OFF : ON);

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

  // exit cleanly
  rc_cleanup();
  return 0;
}


//  *****************************************************************************
/// Make the Mode button launch the QC application when released.
///
void on_mode_released()
{
  DroneInit();
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
    if (RELEASED == rc_get_mode_button())
      return;
  }

  printf("long press detected, shutting down\n");
  rc_set_state(EXITING);

  return;
}

