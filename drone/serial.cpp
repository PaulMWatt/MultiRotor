/// @file serial.cpp
///
/// Interface to read and write through a serial port.
///

//  ****************************************************************************
#include "serial.h"


//  ****************************************************************************
int serial_write(COMPORT        comm, 
                 const uint8_t* buffer, 
                 size_t         len)
{
#if defined(_WIN64) || defined(_WIN32) 

  DWORD bytes;
  WriteFile(comm, buffer, len, &bytes, NULL);

  return int(bytes);

#else

  return write(comm, (const char*)(buffer), len);

#endif
}







//  ****************************************************************************
int serial_read(COMPORT   comm, 
                uint8_t*  buffer,
                size_t    len)
{
#if defined(_WIN64) || defined(_WIN32) 

  DWORD bytes;
  ReadFile(comm, buffer, len, &bytes, NULL);

  return int(bytes);

#else

  return read(comm, (void*)buffer, len);

#endif
}


//  ****************************************************************************
bool read_byte(COMPORT comm, byte_t &data)
{
  if (serial_read(comm, &data, 1) <= 0)
  {
    return false;
  }

  return true;
}






