/// @file LED.h
///
/// Controls for the on-board LEDs.
///
//  ****************************************************************************
#ifndef LED_H_INCLUDED
#define LED_H_INCLUDED

#include <string>



//  ****************************************************************************
class LED
{
public:
  LED(int index);
  ~LED();

  void on();
  void off();
  void blink(uint32_t delay_on,
             uint32_t delay_off);
  void reset();

  bool status();

private:
  int         m_index;
  std::string m_path;

  void  command(const std::string &action,
                const std::string &value);

};



#endif
