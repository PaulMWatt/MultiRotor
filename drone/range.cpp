/// @file range.cpp
///
/// PRUSS driver program to read data from the ultrasonic range sensor HY-SRF05.
/// This program loads the PRU binary and maps the memory addresses used to 
/// interpret the results in user-space.
///
/// Adapted from "Exploring BeagleBone" by Derek Molloy
///
//  ****************************************************************************
#include <stdio.h>
#include <stdlib.h>
#include <prussdrv.h>
#include <pruss_intc_mapping.h>
#include <pthread.h>
#include <unistd.h>
#include <stdint.h>

#include <iostream>
#include <iomanip>

using std::cout;
using std::endl;

#include <rc/pru.h>
#include <rc/time.h>

namespace 
{

// pru shared memory pointer
static volatile
uint32_t  *pru0_data = nullptr;

const uint32_t k_mem_offset     = 16;
const uint32_t k_sample_delay   = k_mem_offset + 1;
const uint32_t k_raw_distance   = k_mem_offset + 2;
const uint32_t k_sample_count   = k_mem_offset + 3;

const uint32_t k_PRU_num        = 0;
const char*    k_PRU_firmware   = "am335x-pru0-qc-range-fw";

static 
bool g_is_init                  = false;

}

//  ****************************************************************************
bool initialize_PRU( )
{
	// map memory
	pru0_data = rc_pru_shared_mem_ptr();
	if (nullptr == pru0_data)
  {
    cout << "ERROR in initialize_PRU, failed to map shared memory pointer" << endl;
		g_is_init = false;
		return false;
	}

	// set first channel to be nonzero, PRU binary will zero this out later
	pru0_data[k_mem_offset] = 42;

  //prussdrv_init( );
  // Open access to the PRU interrupt.
  //int retVal = prussdrv_open(PRU_EVTOUT_0);
  //if (retVal < 0)
  //{
  //  cout << "call to prussdrv_open failed (" << retVal << ")" <<  endl;
  //}

	// start pru
	if (rc_pru_start(k_PRU_num, k_PRU_firmware))
  {
    cout << "ERROR in initialize_PRU, failed to start PRU0" << endl;
		return false;
	}


	// make sure memory actually got zero'd out
	for (int i=0; i < 40; i++)
  {
		if (0 == pru0_data[k_mem_offset])
    {
			g_is_init = true;

      // Initialize control values. 
      pru0_data[k_sample_delay]   = 2;   // 2 milli seconds between samples
      pru0_data[k_raw_distance]   = 0;
      pru0_data[k_sample_count]   = 0;

			return true;
		}

		rc_usleep(100000);
	}

  cout << "ERROR in rc_encoder_pru_init, " << k_PRU_firmware << " failed to load" << endl; 
  cout << "Attempting to stop PRU0" << endl;

  rc_pru_stop(k_PRU_num);
	
  g_is_init = false;
	
  return false;}


//  ****************************************************************************
bool terminate_PRU( )
{
	// zero out shared memory
	if (nullptr == pru0_data)
  {
		pru0_data[k_mem_offset] = 0;
	}

	rc_pru_stop(k_PRU_num);

	pru0_data = nullptr;
	g_is_init = false;

  return true;
}

//  ****************************************************************************
void *updateRangeCalculations(void *value)
{
  cout << "Distance reported by ultrasonic sensor:\n\n";

  do
  {
    //prussdrv_pru_wait_event (PRU_EVTOUT_0);
     
    uint32_t raw   = pru0_data[k_raw_distance];

    float inches   = (raw / float(100 * 148));
    float cm       = (raw / float(100 * 58));

    cout  << "\r" << std::setw(8) << inches   << " in"
          << "\t" << std::setw(8) << cm       << " cm";

    //prussdrv_pru_clear_event (PRU_EVTOUT_0, PRU0_ARM_INTERRUPT);

  }
  while (1);
}


//  ****************************************************************************
void start_range_sensor()
{
  initialize_PRU();

  pthread_t thread;
  if (pthread_create(&thread, NULL, &updateRangeCalculations, NULL))
  {
    cout << "Failed to create range calculation thread.\n";
  }

}

//  ****************************************************************************
void shutdown_range_sensor()
{
  terminate_PRU();
}


//  ****************************************************************************
/// Reports the measured range in meters.
///
float get_range()
{
  if ( nullptr == pru0_data
    || !g_is_init)
  { 
    cout << "ERROR in get_range(), call start_range_sensor() first" << endl;
    return -1.0;
  }

  uint32_t raw   = pru0_data[k_raw_distance];
  float cm       = (raw / float(100 * 58));

  return cm / 1000.0f;
}


