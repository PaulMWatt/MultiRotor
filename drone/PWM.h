/// @file PWM.h
///
/// Abstraction for Pulse-Width Modulation for Beagle Board.
///
//  ****************************************************************************
#ifndef PWM_H_INCLUDED
#define PWM_H_INCLUDED


//  ****************************************************************************
///
///
class PWM 
{
public:
  enum Polarity
  { 
    k_active_high = 0, 
    k_active_low  = 1
  };

  //  ****************************************************************************
  PWM(int channel);

  //  ****************************************************************************
  unsigned int period() const;
  int period(unsigned int period_ns);

  //  ****************************************************************************
  double frequency() const;
  int frequency(double frequency_hz);

  //  ****************************************************************************
  unsigned int duty_cycle() const;
  int duty_cycle(unsigned int duration_ns);

  double duty_cycle_percent() const;
  int duty_cycle_percent(double percentage);

  //  ****************************************************************************
  Polarity polarity() const;
  int polarity(Polarity);
  void invert_polarity();

  //  ****************************************************************************
  int arm();
  int unarm();

  bool is_armed() const;




private:
  //  ****************************************************************************
  int           m_channel;      ///< PWM Channel
  double        m_level;        ///< The PWM's commanded level.
  bool          m_arm;          ///< 

  int           m_period;       ///< PWM Period between updates.
  int           m_duty_cycle;   ///< PWM duty-cycle period.
  int           m_min_offset;   ///< Lowest value that keeps the rotors spinning.

  Polarity      m_polarity;     ///< Polarity
};


#endif

