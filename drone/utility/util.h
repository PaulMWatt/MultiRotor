/// @util.h
///
/// Adapted from the work of Derek Molloy
///
/// Copyright (c) 2014 Derek Molloy (www.derekmolloy.ie)
/// Made available for the book "Exploring BeagleBone" 
/// See: www.exploringbeaglebone.com
/// Licensed under the EUPL V.1.1
/// 
/// For more details, see http://www.derekmolloy.ie/
///
//  **************************************************************************** 
#ifndef UTIL_H_INCLUDED
#define UTIL_H_INCLUDED

#ifdef min
# undef min
#endif

#ifdef max
# undef max
#endif

#include <string>
#include <limits>
#include <algorithm>
#include <cmath>

void change_mode(int dir);
int kbhit (void);

uint64_t timestamp_ms();

int write(std::string path, std::string filename, std::string value);
int write(std::string path, std::string filename, int value);
std::string read(std::string path, std::string filename);


//  **************************************************************************** 
/// Converts a PID variable to int64 for serialization.
/// Allows for values in range -1000 to 1000
///
inline
int64_t encode_PID(float value)
{
  // Scale to an integer,
  // then truncate into range.
  int64_t int_val = int64_t(value * 1000);

  int_val = std::min<int64_t>(int_val, 1000000);
  int_val = std::max<int64_t>(int_val, -1000000);

  return int_val;
}

//  **************************************************************************** 
/// Converts an int64_t into a PID variable from serialization.
/// Allows for values in range -999.999 to 999.999
///
inline
float decode_PID(int64_t value)
{
  // Scale to an integer,
  // then truncate into range.
  float pid_val = value / 1000.0;

  return pid_val;
}


//  **************************************************************************** 
/// Converts the input float to a normalized 64-bit integer value.
/// The range is clamped between -1 and 1.
///
inline
int64_t to_int64(float value)
{
  if (value < -1.0)
    value = -1.0;
  else if (value > 1.0)
    value = 1.0;

  return  (value < 0.0)
          ? (int64_t)(value * std::numeric_limits<int64_t>::min() * -1.0)
          : (int64_t)(value * std::numeric_limits<int64_t>::max());
}

//  **************************************************************************** 
/// Converts the input float to a normalized 64-bit integer value.
/// The range is clamped between -1 and 1.
///
inline
uint64_t to_uint64(float value)
{
  if (value < 0.0)
    value = 0.0;
  else if (value > 1.0)
    value = 1.0;

  return  (uint64_t)(value * std::numeric_limits<uint64_t>::max());
}

//  **************************************************************************** 
/// Converts the input float to a normalized 32-bit integer value.
/// The range is clamped between -1 and 1.
///
inline
int32_t to_int32(float value)
{
  if (value < -1.0)
    value = -1.0;
  else if (value > 1.0)
    value = 1.0;

  return  (value < 0.0)
          ? (int32_t)(value * std::numeric_limits<int32_t>::min() * -1.0)
          : (int32_t)(value * std::numeric_limits<int32_t>::max());
}

//  **************************************************************************** 
/// Converts the input float to a normalized 16-bit integer value.
/// The range is clamped between -1 and 1.
///
inline
int16_t to_int16(float value)
{
  if (value < -1.0)
    value = -1.0;
  else if (value > 1.0)
    value = 1.0;

  return  (value < 0.0)
          ? (int16_t)(value * std::numeric_limits<int16_t>::min() * -1.0)
          : (int16_t)(value * std::numeric_limits<int16_t>::max());
}

//  **************************************************************************** 
/// Converts the input float to a normalized 64-bit integer value.
/// The range is clamped between -1 and 1.
///
inline
uint16_t to_uint16(float value)
{
  if (value < 0.0)
    value = 0.0;
  else if (value > 1.0)
    value = 1.0;

  return  (uint16_t)(value * std::numeric_limits<uint16_t>::max());
}

//  **************************************************************************** 
/// Converts the input int16_t to a normalized float value, that is
/// within the range between -1.0 and 1.0.
///
inline
float to_normalized(int16_t value)
{
  return  (value < 0.0)
          ? (value / (float)std::numeric_limits<int16_t>::min()) * -1.0
          : (value / (float)std::numeric_limits<int16_t>::max());
}

//  **************************************************************************** 
/// Converts the input int32_t to a normalized float value, that is
/// within the range between -1.0 and 1.0.
///
inline
float to_normalized(int32_t value)
{
  return  (value < 0.0)
          ? (value / (float)std::numeric_limits<int32_t>::min()) * -1.0
          : (value / (float)std::numeric_limits<int32_t>::max());
}


//  **************************************************************************** 
/// Converts the input int64_t to a normalized float value, that is
/// within the range between -1.0 and 1.0.
///
inline
float to_normalized(int64_t value)
{
  return  (value < 0.0)
          ? (value / (float)std::numeric_limits<int64_t>::min()) * -1.0
          : (value / (float)std::numeric_limits<int64_t>::max());
}


//  **************************************************************************** 
/// Converts the input int64_t to a normalized float value, that is
/// within the range between -1.0 and 1.0.
///
inline
float euclid_distance(float lhs, float rhs)
{
  return sqrtf(lhs*lhs + rhs*rhs);
}


//  ****************************************************************************
//  Small rounding errors in floating point calculations can result in values
//  that are less than zero.
//
inline
float safe_sqrt(float value)
{
  return value < 0.0
        ? 0.0
        : sqrt(value);
}


#endif 
