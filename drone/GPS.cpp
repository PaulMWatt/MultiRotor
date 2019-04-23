/// @file GPS.cpp
///
/// This library provides access to the GPS information provided by the
/// AdaFruit industries Ultimate GPS breakout. It uses the software module
/// developed by AdaFruit Industries.
///
/// MIT license, all text above must be included in any redistribution
///
//  ****************************************************************************
#include "GPS.h"

#include <iostream>

#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <sys/time.h>
#include <cstring>

// TODO: Potential improvements to this class to make it more resusable include:
//       * Create an abstraction for serial port management
//       * Allow configurable communication settings
//       * 


//#define PMTK_STANDBY "$PMTK161,0*28"
//#define PMTK_STANDBY_SUCCESS "$PMTK001,161,3*36"  // Not needed currently
//#define PMTK_AWAKE "$PMTK010,002*2D"
//
//// ask for the release and version
//#define PMTK_Q_RELEASE "$PMTK605*31"


using std::cout;
using std::endl;

namespace GPS
{

//  ****************************************************************************
UltimateGPS::UltimateGPS()
  : m_file(0)
  , m_fix_data{0}
  , m_cur_pos{0}
  , m_last_pos{0}
  , m_is_exit(false)
{ }


//  ****************************************************************************
//  Communication is fixed at 57600 baud, and updates 5 times / second
//
bool UltimateGPS::init()
{
  m_file = open("/dev/ttyO2", O_RDWR | O_NOCTTY | O_NDELAY);
  if (m_file < 0)
  {
    cout << "UART: Failed (" << m_file << ") to open the GPS Serial Port 2." << endl;
    return false;
  }

  termios options;
  tcgetattr(m_file, &options);

  cout << "GPS cflags: " << options.c_cflag
       << "\nGPS iflag: " << options.c_iflag
       << "\nGPS oflag: " << options.c_oflag
       << "\nGPS vmin:  " << options.c_cc[VMIN]
       << "\nGPS vtime: " << options.c_cc[VTIME] << endl;


  
  options.c_cflag = B9600 | CS8 | CREAD | CLOCAL;
  options.c_iflag = IGNPAR | ICRNL;
  options.c_oflag = 0;
  
  tcflush(m_file, TCIFLUSH);
  tcsetattr(m_file, TCSANOW, &options);

  // Request the fix and minimal location strings.
  const char k_pmtk_set_fix_rate[]  = "$PMTK300,200,0,0,0,0*2F";
  const char k_pmtk_set_baud_rate[] = "$PMTK251,9600*2C";
  const char k_pmtk_set_output[]    = "$PMTK314,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0*28";

  if (write(m_file, k_pmtk_set_fix_rate, strlen(k_pmtk_set_fix_rate) + 1) < 0)
  {
     cout << "Failed to write the GPS fix rate command.\n" << endl;
     return false;
  }

  if (write(m_file, k_pmtk_set_baud_rate, strlen(k_pmtk_set_baud_rate) + 1) < 0)
  {
     cout << "Failed to write the GPS baud rate command.\n" << endl;
     return false;
  }

  if (write(m_file, k_pmtk_set_output, strlen(k_pmtk_set_output) + 1) < 0)
  {
     cout << "Failed to write the GPS output request command.\n" << endl;
     return false;
  }

    
//  usleep(100000);

  cout << "GPS - init has completed" << endl;

  m_read_thread = std::thread(thread_proc, this);

  return m_read_thread.joinable();
}


//  ****************************************************************************
void UltimateGPS::term( )
{
  m_is_exit = true;

  if (m_read_thread.joinable( ))
  {
    m_read_thread.join( );
  }

  ::close(m_file);
  m_file = 0;
}


//  ****************************************************************************
/// Gets the latest system status info
///
void UltimateGPS::getSystemStatus(uint8_t *system_status, 
                                  uint8_t *self_test_result, 
                                  uint8_t *system_error)
{
  // TODO: Complete This
}


//  ****************************************************************************
void UltimateGPS::thread_proc(UltimateGPS *p_this)
{
  if (!p_this)
    return;

  while (!p_this->m_is_exit)
  {
    p_this->process( );
  }
}


//  ****************************************************************************
bool UltimateGPS::process()
{
  // NMEA sentences have a limit of 82 characters.
  // Read until a new line is encountered.
  char  receive[100];
  int   len       = 0;
  char  last_byte = 0;

  while ( last_byte != '\n'
       && len < 100)
  {
    int result = read(m_file, (void*) &receive[len], 1);
    if (result < 0)
    {
//      cout << "Failed (" << result << ") to read from the input.\n" 
//           << "Discarding " << len << "bytes." << endl;

      return false;
    }
    else if (result == 0)
    {
      usleep(100000);
      continue;
    }

    last_byte = receive[len];
    len++;
  }


  return parse_NMEA(receive, len);
}


//  ****************************************************************************
bool UltimateGPS::verify_NMEA_checksum(const char* p_sentence, int len)
{
  // TODO: Complete this.
  return true;

  // The checksum is found after the asterisk at the end of the buffer.
  if (len < 3)
    return false;

  if (p_sentence[len-3] != '*')
    return false;

  // TODO: Read in the hex string checksum value.
  int checksum = 0;

  for (uint8_t i = 0; i < len - 3; ++i)
  {
    checksum ^= p_sentence[i];
  }
  
  return 0 == checksum;
}


//  ***************************************************************************
bool UltimateGPS::parse_NMEA(const char* p_sentence, int len)
{
  if (!verify_NMEA_checksum(p_sentence, len))
  {
    return false;
  }

  const char* p_pos = nullptr;

  p_pos = strstr(p_sentence, "$GPGGA");
  if (nullptr != p_pos)
  {
  //  cout << p_sentence << endl;
// TODO: cout << "Have a GPGGA Fix string... skipping" << endl;
return false;

    const     char*  p_data = &p_pos[6];
    ptrdiff_t offset = p_sentence - p_data; 
    return parse_fix(p_data, len - offset);
  }

  p_pos = strstr(p_sentence, "$GPRMC");
  if (nullptr != p_pos)
  {
    const     char*  p_data = &p_pos[6];
    ptrdiff_t offset = p_sentence - p_data; 
    return parse_location(p_data, len - offset);
  }

  return false;
}


//  ****************************************************************************
bool UltimateGPS::parse_fix(const char* p_sentence, int len)
{
  fix_data_t    fix = {0};

  const char* p_pos = p_sentence;

  p_pos = parse_time     (p_pos, len, fix.time_stamp, fix.ms);
  p_pos = parse_latitude (p_pos, len, fix.latitude);
  p_pos = parse_longitude(p_pos, len, fix.longitude);

  int quality = 0;
  p_pos = parse_field    (p_pos, len, quality);

  if (quality == 1)
    fix.type = GPS;
  else if (quality == 2)
    fix.type = DGPS;
  else
    fix.type = invalid;

  int value = 0;
  p_pos = parse_field    (p_pos, len, value);
  fix.fix_count = static_cast<uint8_t>(value);

  p_pos = parse_field    (p_pos, len, fix.HDOP);
  p_pos = parse_field    (p_pos, len, fix.altitude);

  // Skip this field.
  p_pos = strchr(p_pos, ',') + 1;

  p_pos = parse_field    (p_pos, len, fix.wgs84_height);

  memcpy(&m_fix_data, &fix, sizeof(m_fix_data));

  return true;
}


//  ****************************************************************************
bool UltimateGPS::parse_location(const char* p_sentence, int len)
{
  location_t    loc = {0};

  const char* p_pos = p_sentence;

  p_pos = parse_time     (p_pos, len, loc.time_stamp, loc.ms);

  // Record the validity of the data.
  if ('A' == *p_pos)
  {
    loc.is_valid = true;
  }
  else if ('V' == *p_pos)
  {
    loc.is_valid = false;
  }
  else
  {
	  cout << "Invalid data" << endl;
    return false;
  }

  p_pos = strchr(p_pos, ',')+1;

  // Extract the location on the globe.
  p_pos = parse_latitude (p_pos, len, loc.latitude);

  p_pos = parse_longitude(p_pos, len, loc.longitude);

  p_pos = parse_field    (p_pos, len, loc.speed);

  p_pos = parse_field    (p_pos, len, loc.true_course);

  // TODO: Other values are available to extract.

  memcpy(&m_last_pos, &m_cur_pos, sizeof(m_cur_pos));
  memcpy(&m_cur_pos,  &loc, sizeof(m_cur_pos));

  //cout  << "latitude: "      << loc.latitude 
  //      << ", longitude: "   << loc.longitude 
  //      << ", speed: "       << loc.speed
  //      << ", true_course: " << loc.true_course << endl;

  return true;
}

 
//  ****************************************************************************
const char* UltimateGPS::parse_time(const char* p_data, int len, time_t &value, uint16_t &value_ms)
{
  p_data = strchr(p_data, ',')+1;

  float     time_f    = atof(p_data);
  uint32_t  time_dec  = static_cast<uint32_t>(time_f);
  tm        time_parts {0};

  time_parts.tm_hour    = time_dec / 10000;
  time_parts.tm_min     = (time_dec % 10000) / 100;
  time_parts.tm_sec     = (time_dec % 100);

  value = ::mktime(&time_parts);

  value_ms = 0;// fmod(time_f, 1.0) * 1000;

  p_data = strchr(p_data, ',')+1;

  return p_data;
}

//  ****************************************************************************
const char* UltimateGPS::parse_latitude(const char* p_data, int len, double &value)
{
  char degreebuff[10];

  strncpy(degreebuff, p_data, 2);
  p_data       += 2;
  degreebuff[2] = '\0';
  long degree   = atol(degreebuff) * 10000000;

  strncpy(degreebuff, p_data, 2); // minutes
  p_data        += 3; // skip decimal point

  strncpy(degreebuff + 2, p_data, 4);
  degreebuff[6]           = '\0';

  long   minutes          = 50 * atol(degreebuff) / 3;
  double latitude         = degree / 100000 + minutes * 0.000006F;
  double latitudeDegrees  = (latitude - 100 * int(latitude / 100)) / 60.0;
 
  latitudeDegrees        += int(latitude / 100);


  p_data = strchr(p_data, ',')+1;

  // Now determine if it is above or below the equator.
  if ('S' == *p_data)
  {
    latitudeDegrees *= -1.0;
  }

  p_data = strchr(p_data, ',')+1;

  value  = latitudeDegrees;

  return p_data;
}

//  ****************************************************************************
const char* UltimateGPS::parse_longitude(const char* p_data, int len, double &value)
{
  char degreebuff[10];

  strncpy(degreebuff, p_data, 3);
  p_data       += 3;
  degreebuff[3] = '\0';
  long degree   = atol(degreebuff) * 10000000;

  strncpy(degreebuff, p_data, 2); // minutes
  p_data       += 3; // skip decimal point

  strncpy(degreebuff + 2, p_data, 4);
  degreebuff[6]           = '\0';
  long   minutes          = 50 * atol(degreebuff) / 3;
  double longitude        = degree / 100000 + minutes * 0.000006F;
  double longitudeDegrees = (longitude - 100 * int(longitude / 100)) / 60.0;

  longitudeDegrees       += int(longitude / 100);

  p_data = strchr(p_data, ',')+1;
  // Skip whitespace.
  while (*p_data == ' ')
  {
    p_data++;
  }

  // Now determine if it is East or West of the GML.
  if ('W' == *p_data)
  {
    longitudeDegrees *= -1.0;
  }

  p_data = strchr(p_data, ',')+1;

  value  = longitudeDegrees;

  return p_data;
}


//  ****************************************************************************
const char* UltimateGPS::parse_field(const char* p_data, int len, int &value)
{
  if (',' != *p_data)
  {
    value = atoi(p_data);

    // Find the next field
    p_data = strchr(p_data, ',')+1;
  }

  return p_data;
}


//  ****************************************************************************
const char* UltimateGPS::parse_field(const char* p_data, int len, uint8_t &value)
{
  int         field;
  const char* retval = parse_field(p_data, len, field);

  value = static_cast<uint8_t>(field);

  return retval;
}


//  ****************************************************************************
const char* UltimateGPS::parse_field(const char* p_data, int len, double &value)
{
  if (',' != *p_data)
  {
    value = atof(p_data);

    // Find the next field
    p_data = strchr(p_data, ',')+1;
  }

  return p_data;
}

//  ****************************************************************************
const char* UltimateGPS::parse_field(const char* p_data, int len, float &value)
{
  double      field;
  const char* retval = parse_field(p_data, len, field);

  value = static_cast<float>(field);

  return retval;
}



//  ****************************************************************************
/// Display some basic info about the sensor status
///
void showStatus(GPS::Sensor& gps)
{
  cout  << "//  Sensor Status *********************\n";

  // Get the system status values (mostly for debugging purposes) 
  uint8_t system_status = 0;
  uint8_t BIT_status    = 0;
  uint8_t system_error  = 0;

  gps.getSystemStatus(&system_status, &BIT_status, &system_error);

  // Display the results in the Serial Monitor 
  // Overall System Status
  const char* k_status_messages[] = 
  {
    "Idle",
    "System Error",
    "Initializing Peripherals",
    "System Iniitalization",
    "Executing Self-Test",
    "Sensor fusion algorithm running",
    "System running without fusion algorithms"
  };

  cout  << "\nSystem Status: " << (int)system_status;

  // Decode the results:
  switch (system_status)
  {
  case 0:
  case 1:
  case 2:
  case 3:
  case 4:
  case 5:
  case 6:
    cout << "\n\t" << k_status_messages[system_status] << "\n";
    break;
  };

  // BIT
  cout  << "\nSelf Test:     "   << (int)BIT_status;
  cout  << "\n\tAccelerometer: " << ((BIT_status & 0x01) ? "Passed" : "Failed");
  cout  << "\n\tMagnetometer:  " << ((BIT_status & 0x02) ? "Passed" : "Failed");
  cout  << "\n\tGyroscope:     " << ((BIT_status & 0x04) ? "Passed" : "Failed");
  cout  << "\n\tMCU Self Test: " << ((BIT_status & 0x08) ? "Passed" : "Failed");
  cout  << "\n";


  // System Errors
  const char* k_error_messages[] = 
  {
    "No error",
    "Peripheral initialization error",
    "System initialization error",
    "Self test result failed",
    "Register map value out of range",
    "Register map address out of range",
    "Register map write error",
    "BNO low power mode not available for selected operation mode",
    "Accelerometer power mode not available",
    "Fusion algorithm configuration error",
    "Sensor configuration error"
  };

  cout  << "\nSystem Error:  " << (int)system_error;
  // Decode the results:
  switch (system_error)
  {
  case 0:
  case 1:
  case 2:
  case 3:
  case 4:
  case 5:
  case 6:
  case 7:
  case 8:
  case 9:
  case 10:
    cout<< "\n\t" << k_error_messages[system_error] << "\n";
    break;
  };

  cout  << "\n\n";

  usleep(250);
}



} // namespace GPS
