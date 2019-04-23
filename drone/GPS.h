/// @file GPS.h
///
/// This library provides access to the GPS information provided by the
/// AdaFruit industries Ultimate GPS breakout. It uses the software module
/// developed by AdaFruit Industries.
///
/// MIT license, all text above must be included in any redistribution
///
//  ****************************************************************************
#ifndef GPS_H_INCLUDED
#define GPS_H_INCLUDED

#include <time.h>
#include <cstdint>
#include <cstddef>
#include <thread>

//typedef uint8_t     char;



namespace GPS
{

enum FixType
{
  invalid = 0,
  GPS     = 1,
  DGPS    = 2
};


//  ****************************************************************************
struct fix_data_t
{
  time_t          time_stamp;
  uint16_t        ms;
  double          latitude;
  double          longitude;
  double          altitude;

  FixType         type;
  uint8_t         fix_count;
  float           HDOP;         // Horizontal Dilution of Precision
  float           wgs84_height; // Height above WGS84 Ellipsoid
};



//  ****************************************************************************
struct location_t
{
  time_t          time_stamp;
  uint16_t        ms;

  bool            is_valid;

  double          latitude;     // degrees
  double          longitude;    // degrees
  double          altitude;     // meters

  float           speed;        // knots
  float           true_course;
  float           variation;    // Magnetic variation, Easterly subtracts from true course
};


//  ****************************************************************************
class UltimateGPS
{
public:

  //  **************************************************************************
  UltimateGPS ();

  //  **************************************************************************
  bool  init                ();
  void  term                ();
  void  getSystemStatus     (uint8_t *system_status,
                             uint8_t *self_test_result,
                             uint8_t *system_error);

  uint8_t fix_count( )  const;

  //  **************************************************************************
  bool   is_init( )    const
  {
    return m_read_thread.joinable( );
  }

  //  **************************************************************************
  bool   is_valid( )    const
  {
    return m_cur_pos.is_valid;
  }

  //  **************************************************************************
  time_t time_stamp( ) const
  {
    return m_cur_pos.time_stamp;
  }

  //  **************************************************************************
  double latitude ( )   const
  {
    return m_cur_pos.latitude;
  }

  //  **************************************************************************
  double longitude( )   const
  {
    return m_cur_pos.longitude;
  }

  //  **************************************************************************
  double altitude ( )   const
  {
    return m_cur_pos.altitude;
  }

  //  **************************************************************************
  float  speed( )       const
  {
    return m_cur_pos.speed;
  }

  //  **************************************************************************
  const location_t& location( ) const
  {
    return m_cur_pos;
  }


private:
  //  **************************************************************************
  int           m_file;

  fix_data_t    m_fix_data;
  location_t    m_cur_pos;
  location_t    m_last_pos;

  std::thread   m_read_thread;
  bool          m_is_exit;


  bool process();
  bool verify_NMEA_checksum   (const char* p_sentence, int len);
  bool parse_NMEA             (const char* p_sentence, int len);
  bool parse_fix              (const char* p_sentence, int len);
  bool parse_location         (const char* p_sentence, int len);


  const char* parse_time      (const char* p_data, int len, time_t &value, uint16_t &value_ms);
  const char* parse_latitude  (const char* p_data, int len, double &value);
  const char* parse_longitude (const char* p_data, int len, double &value);

  const char* parse_field     (const char* p_data, int len, int      &value);
  const char* parse_field     (const char* p_data, int len, uint8_t  &value);
  const char* parse_field     (const char* p_data, int len, double   &value);
  const char* parse_field     (const char* p_data, int len, float    &value);


  static
    void thread_proc(UltimateGPS *p_this);
};


//  ****************************************************************************
using Sensor = UltimateGPS;


//  Utility Functions **********************************************************
void showStatus             (GPS::Sensor& gps);


} // namespace GPS


#endif
