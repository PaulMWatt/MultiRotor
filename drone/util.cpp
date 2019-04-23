/// @util.cpp
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

#include "utility/util.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <termios.h>
#include <fcntl.h>

#include <time.h>
#include <sys/time.h>


//  ****************************************************************************
//void change_mode(int dir)
//{
//  static termios prev;
//  static termios cur;

//  if (dir == 1)
//  {
//    tcgetattr(STDIN_FILENO, &prev);
//    cur = prev;
//    cur.c_lflag &= ~(ICANON | ECHO);
//    tcsetattr(STDIN_FILENO, TCSANOW, &cur);
//  }
//  else
//  {
//    tcsetattr(STDIN_FILENO, TCSANOW, &prev);
//  }
//}

//  ****************************************************************************
//int kbhit (void)
//{
//  timeval tv    = {0};
//  fd_set  rdfs;

//  FD_ZERO(&rdfs);
//  FD_SET (STDIN_FILENO, &rdfs);

//  select(STDIN_FILENO+1, &rdfs, NULL, NULL, &tv);

//  return FD_ISSET(STDIN_FILENO, &rdfs);
//}


//  ****************************************************************************
uint64_t timestamp_ms()
{
  // The timestamp is very important for accurate 
  // calculations and control.
  timespec timestamp = {0};

  clock_gettime(CLOCK_MONOTONIC, &timestamp);

  // Convert the timestamp to milliseconds:
  uint64_t value = timestamp.tv_sec * 1000;
  value += timestamp.tv_nsec / 1000000;

  return value;
}



//  ****************************************************************************
/// Helper write function that writes a single string value to a file in the path provided
/// @param path The sysfs path of the file to be modified
/// @param filename The file to be written to in that path
/// @param value The value to be written to the file
/// @return
/// 
int write(std::string path, std::string filename, std::string value)
{
  std::ofstream fs;
  fs.open((path + filename).c_str());
   
  if (!fs.is_open())
  {
	  std::perror("GPIO: write failed to open file ");
	  return -1;
  }

  fs << value;
  fs.close();

  return 0;
}

//  ****************************************************************************
/// Helper read function that reads a single string value to a file from the path provided
/// @param path The sysfs path of the file to be read
/// @param filename Filename The file to be written to in that path
/// @return
/// 
std::string read(std::string path, std::string filename)
{
  std::ifstream fs;
  fs.open((path + filename).c_str( ));

  if (!fs.is_open( ))
  {
    std::perror("GPIO: read failed to open file ");
  }

  std::string input;
  getline(fs, input);
  fs.close( );

  return input;
}

//  ****************************************************************************
/// Private write method that writes a single int value to a file in the path provided
/// @param path The sysfs path of the file to be modified
/// @param filename The file to be written to in that path
/// @param value The int value to be written to the file
/// @return
/// 
int write(std::string path, std::string filename, int value)
{
  std::stringstream s;
  s << value;

  return write(path, filename, s.str( ));
}

