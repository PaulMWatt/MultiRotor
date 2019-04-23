/// @file LED.h
///
/// Controls for the on-board LEDs.
///
//  ****************************************************************************
#include "LED.h""
#include <fstream>


//  ****************************************************************************
LED::LED(int index)
  : m_index(index)
  , m_path("/sys/class/leds/beaglebone/green/usr")
{
  m_path += std::to_string(m_index);
}

//  ****************************************************************************
LED::~LED()
{
  off();
}

//  ****************************************************************************
void LED::on()
{
  reset();
  command("/brightness", "1");
}

//  ****************************************************************************
void LED::off()
{
  reset();
  command("/brightness", "0");
}

//  ****************************************************************************
void LED::blink(
  uint32_t delay_on, 
  uint32_t delay_off)
{
  command("/trigger",   "timer");
  command("/delay_on",  std::to_string(delay_on));
  command("/delay_off", std::to_string(delay_off));
}

//  ****************************************************************************
void LED::reset()
{
  command("/trigger", "none");
}

//  ****************************************************************************
bool LED::status()
{
  // TODO: Complete this.
  return false;
}

//  ****************************************************************************
void LED::command(
  const std::string &action,
  const std::string &value)
{
  std::ofstream out(m_path + action);
  if (out.is_open())
  {
    out << value;
  }
}

