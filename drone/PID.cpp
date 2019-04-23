/// @file PID.cpp 
///
/// HW7 - PID Integration
/// Paul Watt
///
//  ****************************************************************************
#include "PID.h"

#include <iostream>
using std::cout;
using std::endl;

namespace 
{

//const float k_dt  = 1.0 / 200.0;
const float k_dt  = 1.0;

}


//  ****************************************************************************
PID::PID()
  : m_setpoint(0.0)
  , m_scalar(1.0)
  , m_gain_Kp(1.0)
  , m_gain_Ki(0.0)
  , m_gain_Kd(0.0)
  , m_range_min(-1.0)
  , m_range_max(1.0)
  , m_windup_limit(0.5)
  , m_cutoff_freq(20.0f)
  , m_last_output(0.0f)
  , m_average(rc_filter_empty())
{
  rc_filter_moving_average(&m_average, 11, k_dt);
  rc_filter_prefill_inputs(&m_average, 0.0);
  clear();
}

//  ****************************************************************************
PID::PID(float K_p, float K_i, float K_d)
  : m_setpoint(0.0)
  , m_scalar(1.0)
  , m_gain_Kp(K_p)
  , m_gain_Ki(K_i)
  , m_gain_Kd(K_d)
  , m_range_min(-1.0)
  , m_range_max(1.0)
  , m_windup_limit(0.5)
  , m_cutoff_freq(20.0f)
  , m_last_output(0.0f)
  , m_average(rc_filter_empty())
{
  rc_filter_moving_average(&m_average, 11, k_dt);
  rc_filter_prefill_inputs(&m_average, 0.0);
  clear();
}
 
//  ****************************************************************************
void PID::setpoint(float value)
{
  if (value < min())
  {
    setpoint(min());
  }
  else if (value > max())
  {
    setpoint(max());   
  }
  else
  {
    m_setpoint = value;
  }
}

//  ****************************************************************************
bool PID::update_delta_time(float    actual,
                            uint64_t timestamp)
{
  m_prev_error = target() - actual;

  // Do not report if this is the first sample.
  if (0 == m_prev_time)
  {
    m_prev_time       = timestamp;
    m_prev_derivative = 0.0f;
    m_integral        = 0.0f;

    return false;
  }

  // The size of the time-slice drives
  // the remaining calculations.
  m_delta_time = (timestamp - m_prev_time) / 1000.0;
  m_prev_time  = timestamp;

  // If too much time has passed since the last sample, 
  // reset PIDs filter state.
  if (m_delta_time > 1.0f)
  {
    clear();
  }  
  else if (m_delta_time == 0.0f)
  {
    return false;
  }

  return true;
}

//  ****************************************************************************
void PID::update_derivative(float position)
{
  // Calculate the derivative from the process variable,
  // because it remains continuous even when the setpoint is modified.

  // The derivative requires a first step with change.
  float cur_derivative = 0.0f;
  float previous       = m_prev_derivative;
  if (!isnan(previous))
  {
    cur_derivative  = (m_prev_position - position) / dt();

    // Calculate the RC coefficient for the low-pass noise filter.
    float RC    = 1.0  / (k_2pi * lowpass_freq());
    float alpha = dt() / (RC + dt());
    float delta = cur_derivative - previous;

    cur_derivative  = previous + (delta * alpha);
  }

  m_prev_position   = position;
  m_prev_derivative = cur_derivative;
}


//  ****************************************************************************
void PID::update_integral()
{
  // Only update the integral if the system is not already saturated.
  float level = proportional()
              + integral()
              + derivative();
  if ( level > min()
    && level < max())
  {
    // Add the current slice to the integral error.
    m_integral += error() * dt(); 
  }

  // Prevent the integral, steady-state, error from growing too large.
  if (m_integral > m_windup_limit)
  {
    m_integral = m_windup_limit;
  }
  else if (m_integral < -m_windup_limit)
  {
    m_integral = -m_windup_limit;
  }
}


//  ****************************************************************************
/// Processes the next sample for the specified delta time.
///
float PID::update(float actual, uint64_t timestamp)
{
  if (!update_delta_time(actual, timestamp))
  {
    return 0;
  }

  update_derivative(actual);
  update_integral();

  // Complete the final calculation
  return  apply_PID_filter();
}


//  ****************************************************************************
void PID::min(float range)
{
  if (range >= max())
    return;

  m_range_min = range;

  // Make sure the set-point is within range.
  if (target() < min())
  {
    setpoint(range);
  }

  // Reset the error of the PID.
  clear();
}


//  ****************************************************************************
void PID::max(float range)
{
  if (range <= min())
    return;

  m_range_max = range;

  // Make sure the set-point is within range.
  if (target() > max())
  {
    setpoint(range);
  }

  // Reset the error of the PID.
  clear();
}

//  ****************************************************************************
void PID::clear()
{
  m_delta_time      = 0;
  m_prev_time       = 0;
  m_prev_error      = 0.0;

  m_prev_derivative = NAN;
  m_integral        = 0.0;
}

//  ****************************************************************************
float PID::apply_PID_filter()
{
  float level = proportional()
              + integral()
              + derivative();

  //float output  = rc_filter_march(&m_average, level);
  //return output * scalar();

  m_last_output = 0.87 * m_last_output + 0.13 * level * scalar();
//m_last_output *= scalar();
  return m_last_output;
}
