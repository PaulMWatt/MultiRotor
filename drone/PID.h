/// @file PID.h
///
/// Generic PID Abstraction 
///
//  ****************************************************************************
#ifndef PID_H_INCLUDED
#define PID_H_INCLUDED

#include <cstdint>
#include <iostream>
#include "utility/robotics.h"

#undef min
#undef max

// TODO: There are plenty of things that can be done to improve on this class
//     X 1) Allow for integral windup
//       2) Deadband
//     X 3) Place a cap on change / error
//     X 4) Set point ramping
//       5) Separate the update call from the actual calculations
//          so the caller may call P(), PI(), PD() combinations, etc.


const 
  float k_pi      = 3.1415926535897932384626433832795;

const 
  float k_2pi     = 2 * k_pi;

const 
  float k_half_pi = 0.5 * k_pi;

const 
  float k_pi_6    = k_pi / 6.0;     // 30°

const 
  float k_pi_4    = k_pi / 4.0;     // 45°

const 
  float k_pi_3    = k_pi / 3.0;     // 60°

const 
  float k_pi_2    = k_pi / 2.0;     // 90°


//  ****************************************************************************
/// A simple abstraction of a PID control system.
///
class PID
{
public:

  //  **************************************************************************
  PID();
  PID(float K_p, float K_i, float K_d);


  //  **************************************************************************
  /// Returns the target setpoint value with smoothing filters factored into the result.
  ///
  float target() const
  {
    return m_setpoint;
  }

  //  **************************************************************************
  /// Updates the target setpoint value.
  ///
  void setpoint(float value);

  //  **************************************************************************
  /// Processes the next sample for the specified delta time.
  ///
  float update(float value, uint64_t timestamp);

  //  **************************************************************************
  /// Time delta for the current sample.
  ///
  float dt() const
  {
    return m_delta_time;
  }

  //  **************************************************************************
  float proportional() const
  {
    return Kp() * error();
  }

  //  **************************************************************************
  float integral() const
  {
    return Ki() * integrator();
  }

  //  **************************************************************************
  float integrator() const
  {
    return m_integral;
  }

  //  **************************************************************************
  float derivative() const
  {
    return Kd() * dError();
  }

  //  **************************************************************************
  void reset_integral()
  {
    m_integral        = 0.0f;
    m_prev_derivative = 0.0f;
  }

  //  **************************************************************************
  float error() const
  {
    return m_prev_error;
  }

  //  **************************************************************************
  float dError() const
  {
    float derivative = m_prev_derivative;
    return  derivative != NAN
            ? derivative
            : 0.0;
  }

  //  **************************************************************************
  float windup_limit() const
  {
    return m_windup_limit;
  }

  //  **************************************************************************
  void windup_limit(float limit) 
  {
    m_windup_limit = limit;
  }

  //  **************************************************************************
  float lowpass_freq() const
  {
    return m_cutoff_freq;
  }

  //  **************************************************************************
  void lowpass_freq(float freq) 
  {
    m_cutoff_freq = freq;
  }

  //  **************************************************************************
  float Kp() const
  {
    return m_gain_Kp;
  }

  //  **************************************************************************
  void Kp(float gain)
  {
    if (gain < 0.0)
      return;

    m_gain_Kp = gain;

    // Reset the error of the PID.
    clear();
  }

  //  **************************************************************************
  float Ki() const
  {
    return m_gain_Ki;
  }

  //  **************************************************************************
  void Ki(float gain)
  {
    if (gain < 0.0)
      return;

    m_gain_Ki = gain;

    // Reset the error of the PID.
    clear();
  }

  //  **************************************************************************
  float Kd() const
  {
    return m_gain_Kd;
  }

  //  **************************************************************************
  void Kd(float gain)
  {
    m_gain_Kd = gain;

    // Reset the error of the PID.
    clear();
  }

  //  **************************************************************************
  float scalar() const
  {
    return m_scalar;
  }

  //  **************************************************************************
  void scalar(float value)
  {
    m_scalar = value;
  }

  //  **************************************************************************
  float min() const
  {
    return m_range_min;
  }

  //  **************************************************************************
  void min(float range);

  //  **************************************************************************
  float max() const
  {
    return m_range_max;
  }

  //  **************************************************************************
  void max(float range);

  //  **************************************************************************
  /// Resets the state of this PID instance. 
  /// All accumulated errors and state are reset to zero.
  ///
  void clear();

private:
  //  PID Tracking Data ********************************************************
  float     m_setpoint;         ///< The commanded value for this PID.

  float     m_delta_time;       ///< The length of the last time step in seconds.
  uint64_t  m_prev_time;        ///< The last timestamp for the error integral.

  float     m_prev_position;    ///< The prev actual position.
  float     m_prev_error;       ///< The prev difference between the
                                ///  commanded setpoint and the measured setpoint.
  float     m_prev_derivative;  ///< The amount the error changed from the
                                ///  last measured state.
  float     m_integral;         ///< The total error accumulated over time.

  //  PID Tuning Data **********************************************************
  float     m_scalar;           ///< An arbitrary scalar factor for the entire PID.
  float     m_gain_Kp;          ///< The gain of the proportional factor.
  float     m_gain_Ki;          ///< The gain of the integral factor.
  float     m_gain_Kd;          ///< The gain of the derivative factor.

  float     m_range_min;        ///< The minimum allowed set-point.
  float     m_range_max;        ///< The maximum allowed set-point.

  float     m_windup_limit;     ///< Value that prevents the integral error from 
                                ///  becoming too large due to cold-starts 
                                ///  and large control changes.
  float     m_cutoff_freq;      ///< The cutoff frequency in the low-pass filter
                                ///  used to filter noise from the control system.

  float     m_setpoint_filter;  ///< A filter adjustment value to be applied to the
                                ///  setpoint when the value is changed.

  float     m_last_output;      ///< A cached instance of the last calculated
                                ///  output for use with smoothing filters.
  rc_filter_t m_average;        ///< An averaging filter to smooth the PID output.


  //  **************************************************************************
  /// Updates the sampled time slice.
  ///
  bool update_delta_time(
    float     cur_error,
    uint64_t  timestamp);

  //  **************************************************************************
  /// Updates the derivative calculations
  ///
  void update_derivative(float cur_error);

  //  **************************************************************************
  /// Updates the integral calculations
  ///
  void update_integral();

  //  **************************************************************************
  /// Smooths changes to the setpoint to eliminate jerky command movements.
  ///
  void apply_setpoint_filter();

  //  **************************************************************************
  /// Smooths the output of the PID.
  ///
  float apply_PID_filter();
};



#endif
