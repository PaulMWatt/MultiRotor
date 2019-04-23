/// @file PWM.cpp
///
/// Abstraction for Pulse-Width Modulation for Beagle Board.
///
/// This information is valid for the following BeagleBone Black version:
/// 
/// Linux beaglebone 3.8.13-bone79 #1 SMP armv71
///
/// Adapted from Derek Molloy's
/// Copyright (c) 2014 Derek Molloy (www.derekmolloy.ie)
/// Made available for the book "Exploring BeagleBone" 
/// See: www.exploringbeaglebone.com
/// Licensed under the EUPL V.1.1
///
//  ****************************************************************************

#include "PWM.h"
#include "utility/util.h"
#include "utility/robotics.h"
#include <cstdlib>


//  ****************************************************************************
double to_frequency(unsigned int period_ns)
{
  double period_s = (double)period_ns/1000000000;

  return 1.0f / period_s;
}

//  ****************************************************************************
unsigned int to_ns(double frequency_hz)
{
  double period_s = 1.0f / frequency_hz;

  return static_cast<unsigned int>(period_s * 1000000000);
}




//  ****************************************************************************
PWM::PWM(int channel) 
  : m_channel(channel)
  , m_level(0.0)
  , m_period(2000000)
  , m_duty_cycle(1000000)
{
 
}

//  ****************************************************************************
int PWM::period(unsigned int period_ns)
{
  m_period = period_ns;
  return m_period;
}

//  ****************************************************************************
unsigned int PWM::period() const
{
  return m_period;
}

//  ****************************************************************************
double PWM::frequency() const
{
  return to_frequency(period());
}

//  ****************************************************************************
int PWM::frequency(double frequency_hz)
{
  period(to_ns(frequency_hz));
  return frequency();
}

//  ****************************************************************************
unsigned int PWM::duty_cycle() const
{
  return m_duty_cycle;
}

//  ****************************************************************************
int PWM::duty_cycle(unsigned int duty_ns)
{
  m_duty_cycle = duty_ns;
  return rc_servo_send_esc_pulse_normalized(m_channel, m_duty_cycle);
}

//  ****************************************************************************
double PWM::duty_cycle_percent() const
{
  return m_level;
}

//  ****************************************************************************
int PWM::duty_cycle_percent(double percentage)
{
  double adjusted = 0.0;

  // Only set the level if the system is armed.
  if (is_armed())
  {
    m_level = percentage;

    if (m_level < 0.0)
    {
      m_level = 0.0;
    }
    else if (m_level > 1.0)
    {
      m_level = 1.0;
    }

    // TODO: revisit and incorporate into the API.
    //Scale to the spinning range (+20%)
    adjusted = 0.2 + (0.8 * m_level);
  }

  return rc_servo_send_esc_pulse_normalized(m_channel, adjusted);
}

//  ****************************************************************************
PWM::Polarity PWM::polarity() const
{
  return k_active_high;
}

//  ****************************************************************************
int PWM::polarity(PWM::Polarity polarity)
{
  return 0;
}

//  ****************************************************************************
void PWM::invert_polarity() 
{
  // Currently does ni action;
}

//  ****************************************************************************
int PWM::arm()
{
  m_arm = true;
  return 0;
}

//  ****************************************************************************
int PWM::unarm()
{
  m_arm = false;
  return 0;
}

//  ****************************************************************************
bool PWM::is_armed() const
{
  return m_arm;
}

