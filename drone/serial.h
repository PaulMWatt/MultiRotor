/// @file serial.hint
///
/// Interface to read and write through a serial port.
///

//  ****************************************************************************
#ifndef SERIAL_H_INCLUDED
#define SERIAL_H_INCLUDED

#include <cstdint>

#if defined(_WIN64) || defined(_WIN32)

#include <windows.h>

typedef HANDLE      COMPORT;

#else

#include <fcntl.h>
#include <unistd.h>
#include <termios.h>

typedef int         COMPORT;

#endif


typedef uint8_t     byte_t;

bool read_byte(COMPORT comm, byte_t& data);

int serial_write(COMPORT        comm,
                 const uint8_t* buffer,
                 size_t         len);

  int serial_read(COMPORT   comm,
                  uint8_t*  buffer,
                  size_t    len);

#endif

