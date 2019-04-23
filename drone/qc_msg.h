/// @file qc_msg.h
/// 
/// Defines the quad-copter communication formats
///
//  ****************************************************************************
#ifndef QC_MSG_H_INCLUDED
#define QC_MSG_H_INCLUDED

#include <cstdint>
#include <cstring>
#include "serial.h"

#ifndef _WIN32

#include <arpa/inet.h>

//  ****************************************************************************
inline
uint64_t htonll(uint64_t value)
{
  return  (value >> (56))
        | (value >> (40) & 0x000000000000FF00LL)
        | (value >> (24) & 0x0000000000FF0000LL)
        | (value >> (8)  & 0x00000000FF000000LL)
        | (value << (8)  & 0x000000FF00000000LL)
        | (value << (24) & 0x0000FF0000000000LL)
        | (value << (40) & 0x00FF000000000000LL)
        | (value << (56));
}

//  ****************************************************************************
inline
uint64_t ntohll(uint64_t value)
{
  return htonll(value);
}

#endif

//  ****************************************************************************
enum PIDType
{
  k_none        = 0,
  k_roll        = 1,
  k_roll_rate   = 2,
  k_pitch       = 3,
  k_pitch_rate  = 4,
  k_rotate      = 5,
  k_rotate_rate = 6
};


enum ControlMode
{
  rate_control = 1,             // The user's input controls the rate of 
                                // angular rotation. Zero position on the
                                // control stick indicates no rotation.
                                // This is commonly called, "Acrobat Mode".
  angle_control                 // The user's input controls the orientation
                                // angle of the drone's frame. Zero position
                                // on the control stick indicates a level
                                // position. 
                                // This is commonly called, "Stability Mode".
};


//  ****************************************************************************
struct QCopter
{
  int16_t    roll;
  int16_t    pitch;
  int16_t    yaw;
  int16_t    thrust;
};


//  ****************************************************************************
struct Orientation
{
  int16_t    roll;
  int16_t    pitch;
  int16_t    yaw;
  int16_t    roll_rate;
  int16_t    pitch_rate;
  int16_t    yaw_rate;
};


//  ****************************************************************************
struct PIDDesc
{
  uint64_t Kp;
  uint64_t Ki;
  uint64_t Kd;

  int64_t range_min;
  int64_t range_max;
};

//  ****************************************************************************
struct PIDState
{
  int64_t set_point;
  int64_t delta_time;
  int64_t current_error;
  int64_t delta_error;
  int64_t integral_error;

  int64_t windup_limit;

  PIDDesc desc;
};


//  ****************************************************************************
struct DronePIDs
{
  PIDState roll_rate;
  PIDState roll;
  PIDState pitch_rate;
  PIDState pitch;
  PIDState rotation;
};

//  ****************************************************************************
struct Motors
{
  uint16_t  A;
  uint16_t  B;
  uint16_t  C;
  uint16_t  D;
  uint16_t  E;
  uint16_t  F;
  uint16_t  G;
  uint16_t  H;
};

//  ****************************************************************************
struct Battery
{
  uint8_t   cell_count;
  uint16_t  cell_level[4];
};

//  ****************************************************************************
struct Batteries
{
  uint8_t   count;
  Battery   battery[4];
};

//  ****************************************************************************
struct Location
{
  uint8_t   is_valid;
  int32_t   latitude;
  int32_t   longitude;
  int32_t   altitude;
  int32_t   height;
};

//  ****************************************************************************
struct DroneState
{
  uint8_t     is_armed;
  Orientation orientation;
  Location    position;
  Motors      motor;
  Batteries   batteries;
};


//  ****************************************************************************
const uint16_t  k_qc_msg_header           = 0x4EAD;
const uint16_t  k_qc_msg_beacon           = 0xBEAC;
const uint16_t  k_qc_msg_beacon_ack       = 0xFFFF;
const uint16_t  k_qc_msg_arm              = 0x0101;
const uint16_t  k_qc_msg_arm_ack          = 0x0202;
const uint16_t  k_qc_msg_control          = 0x0303;
const uint16_t  k_qc_msg_get_control_mode = 0x0310;
const uint16_t  k_qc_msg_control_mode     = 0x0311;
const uint16_t  k_qc_ack_control_mode     = 0x0312;
const uint16_t  k_qc_msg_adjust_gain      = 0x0404;
const uint16_t  k_qc_msg_drone_state      = 0x0505;
const uint16_t  k_qc_req_pid_state        = 0x050A;
const uint16_t  k_qc_msg_pid_state        = 0x051A;
const uint16_t  k_qc_msg_disarm           = 0x0909;
const uint16_t  k_qc_msg_halt             = 0x0911;


//  ****************************************************************************
struct QCHeader
{
  uint16_t header_id;                 // 0x4EAD
  uint16_t msg_type;
  uint16_t len;
  uint16_t seq_id;
};


//  ****************************************************************************
struct QCBeaconMsg
{
  QCHeader  header;
  uint32_t  cookie;
};

//  ****************************************************************************
struct QCBeaconAckMsg
{
  QCHeader  header;
  uint32_t  cookie;
  uint32_t  status;
};


//  ****************************************************************************
struct QCArmMsg
{
  QCHeader  header;
  uint32_t  cookie;
};


//  ****************************************************************************
struct QCArmAck
{
  QCHeader  header;
  uint32_t  cookie;
  uint32_t  status;
  Location  base_position;
};


//  ****************************************************************************
struct QCDisarmMsg
{
  QCHeader  header;
  uint32_t  status;
};

//  ****************************************************************************
struct QCHaltMsg
{
  QCHeader  header;
  uint32_t  status;
};

//  ****************************************************************************
struct QCControlMsg
{
  QCHeader  header;
  QCopter   control;
};

//  ****************************************************************************
struct QCGetControlModeMsg
{
  QCHeader  header;
};

//  ****************************************************************************
struct QCControlModeMsg
{
  QCHeader  header;
  uint8_t   control_mode;  // 1 angle control, 2 for rate control
  uint8_t   disable_roll;  // 0 allows this axis to be active (default)
  uint8_t   disable_pitch; // 1 Disables control of the axis
  uint8_t   disable_yaw;   //
};

//  ****************************************************************************
//  Simply echo the contents of the control mode change message.
struct QCControlModeAck
{
  QCHeader  header;
  uint8_t   control_mode;  // 1 angle control, 2 for rate control
  uint8_t   disable_roll;  // 0 allows this axis to be active (default)
  uint8_t   disable_pitch; // 1 Disables control of the axis
  uint8_t   disable_yaw;   //
};

//  ****************************************************************************
struct QCAdjustGainMsg
{
  QCHeader  header;
  PIDType   type;
  PIDDesc   desc;
};


//  ****************************************************************************
struct QCDroneStateMsg
{
  QCHeader    header;
  DroneState  state;
};


//  ****************************************************************************
struct QCPIDStateReq
{
  QCHeader    header;
  uint8_t     status;
  PIDType     type;
};


//  ****************************************************************************
struct QCPIDStateMsg
{
  QCHeader    header;
  PIDType     type;
  PIDState    state;
};


//  ****************************************************************************
inline
uint16_t DecodeMessageType(const uint8_t* p_buffer, size_t len)
{
  if ( !p_buffer 
    || len < 2)
  {
    return 0;
  }

  uint16_t type = 0;

  // The message id is found in bytes index at 2 and 3.
  ::memcpy(&type, p_buffer + 2, sizeof(type));

  return ntohs(type);
}

//  ****************************************************************************
inline
uint16_t DecodeMessageType(const char* p_buffer, size_t len)
{
  return DecodeMessageType((const uint8_t*)p_buffer, len);
}

//  ****************************************************************************
template <typename T>
uint16_t MessageType();


template <>
inline 
uint16_t MessageType<QCBeaconMsg>()
{
  return k_qc_msg_beacon;
}


template <>
inline 
uint16_t MessageType<QCBeaconAckMsg>()
{
  return k_qc_msg_beacon_ack;
}


template <>
inline 
uint16_t MessageType<QCArmMsg>()
{
  return k_qc_msg_arm;
}


template <>
inline 
uint16_t MessageType<QCArmAck>()
{
  return k_qc_msg_arm_ack;
}


template <>
inline 
uint16_t MessageType<QCControlMsg>()
{
  return k_qc_msg_control;
}


template <>
inline 
uint16_t MessageType<QCControlModeMsg>()
{
  return k_qc_msg_control_mode;
}


template <>
inline 
uint16_t MessageType<QCGetControlModeMsg>()
{
  return k_qc_msg_get_control_mode;
}


template <>
inline 
uint16_t MessageType<QCControlModeAck>()
{
  return k_qc_ack_control_mode;
}


template <>
inline 
uint16_t MessageType<QCAdjustGainMsg>()
{
  return k_qc_msg_adjust_gain;
}


template <>
inline 
uint16_t MessageType<QCDroneStateMsg>()
{
  return k_qc_msg_drone_state;
}


template <>
inline 
uint16_t MessageType<QCDisarmMsg>()
{
  return k_qc_msg_disarm;
}

template <>
inline 
uint16_t MessageType<QCHaltMsg>()
{
  return k_qc_msg_halt;
}

//  Forward Declarations *******************************************************
uint16_t GetSequenceId();

//  ****************************************************************************
template <typename T>
void PopulateQCHeader(T &msg)
{
  msg.header.header_id  = k_qc_msg_header;
  msg.header.msg_type   = MessageType<T>();
  msg.header.len        = sizeof(T);
  msg.header.seq_id     = GetSequenceId();
}


//  ****************************************************************************
inline 
PIDDesc to_host(const PIDDesc &desc) 
{
  PIDDesc result;

  result.Kp = ntohll(desc.Kp);
  result.Ki = ntohll(desc.Ki);
  result.Kd = ntohll(desc.Kd);

  result.range_min = ntohll(desc.range_min);
  result.range_max = ntohll(desc.range_max);

  return result;
}

//  ****************************************************************************
inline 
PIDDesc to_network(const PIDDesc &desc) 
{
  PIDDesc result;

  result.Kp = htonll(desc.Kp);
  result.Ki = htonll(desc.Ki);
  result.Kd = htonll(desc.Kd);

  result.range_min = htonll(desc.range_min);
  result.range_max = htonll(desc.range_max);

  return result;
}

//  ****************************************************************************
inline 
PIDState to_host(const PIDState &pid) 
{
  PIDState result;

  result.set_point      = ntohll(pid.set_point);
  result.delta_time     = ntohll(pid.delta_time);
  result.current_error  = ntohll(pid.current_error);
  result.delta_error    = ntohll(pid.delta_error);
  result.integral_error = ntohll(pid.integral_error);
  result.windup_limit   = ntohll(pid.windup_limit);

  result.desc           = to_host(pid.desc);

  return result;
}

//  ****************************************************************************
inline 
PIDState to_network(const PIDState &pid) 
{
  PIDState result;

  result.set_point      = htonll(pid.set_point);
  result.delta_time     = htonll(pid.delta_time);
  result.current_error  = htonll(pid.current_error);
  result.delta_error    = htonll(pid.delta_error);
  result.integral_error = htonll(pid.integral_error);
  result.windup_limit   = htonll(pid.windup_limit);

  result.desc           = to_network(pid.desc);

  return result;
}

//  ****************************************************************************
inline
size_t Serialize_int16(int16_t value, uint8_t** p_buffer)
{
  size_t offset = sizeof(int16_t);

  int16_t data = htons(value);
  ::memcpy(*p_buffer, (char*)&data, offset);
  *p_buffer += offset;

  return offset;
}

//  ****************************************************************************
inline
size_t Serialize_uint16(uint16_t value, uint8_t** p_buffer)
{
  return Serialize_int16((int16_t)value, p_buffer);
}

//  ****************************************************************************
inline
size_t Serialize_int32(int32_t value, uint8_t** p_buffer)
{
  size_t offset = sizeof(int32_t);

  int32_t data = htonl(value);
  ::memcpy(*p_buffer, (char*)&data, offset);
  *p_buffer += offset;

  return offset;
}

//  ****************************************************************************
inline
size_t Serialize_uint32(uint32_t value, uint8_t** p_buffer)
{
  return Serialize_int32((int32_t)value, p_buffer);
}

//  ****************************************************************************
inline
size_t Serialize_int64(int64_t value, uint8_t** p_buffer)
{
  size_t offset = sizeof(int64_t);

  int64_t data = htonll(value);
  ::memcpy(*p_buffer, (char*)&data, offset);
  *p_buffer += offset;

  return offset;
}

//  ****************************************************************************
inline
size_t Serialize_uint64(uint64_t value, uint8_t** p_buffer)
{
  return Serialize_int64((int64_t)value, p_buffer);
}

//  ****************************************************************************
inline
size_t Deserialize_int16(int16_t &value, const uint8_t** p_buffer)
{
  size_t offset = sizeof(int16_t);

  int16_t data = 0;
  ::memcpy((char*)&data, *p_buffer, offset);
  value = ntohs(data);
  *p_buffer += offset;

  return offset;
}

//  ****************************************************************************
inline
size_t Deserialize_uint16(uint16_t &value, const uint8_t** p_buffer)
{
  return Deserialize_int16((int16_t &)value, p_buffer);
}

//  ****************************************************************************
inline
size_t Deserialize_int32(int32_t &value, const uint8_t** p_buffer)
{
  size_t offset = sizeof(int32_t);

  int32_t data = 0;
  ::memcpy((char*)&data, *p_buffer, offset);
  value = ntohl(data);
  *p_buffer += offset;

  return offset;
}

//  ****************************************************************************
inline
size_t Deserialize_uint32(uint32_t &value, const uint8_t** p_buffer)
{
  return Deserialize_int32((int32_t &)value, p_buffer);
}

//  ****************************************************************************
inline
size_t Deserialize_int64(int64_t &value, const uint8_t** p_buffer)
{
  size_t offset = sizeof(int64_t);

  int64_t data = 0;
  ::memcpy((char*)&data, *p_buffer, offset);
  value = ntohll(data);
  *p_buffer += offset;

  return offset;
}

//  ****************************************************************************
inline
size_t Deserialize_uint64(uint64_t &value, const uint8_t** p_buffer)
{
  return Deserialize_int64((int64_t &)value, p_buffer);
}

//  ****************************************************************************
inline
size_t Serialize(const QCHeader &data, uint8_t* p_buffer, size_t len)
{
  if ( !p_buffer
    || len < sizeof(QCHeader))
  {
    return 0;
  }

  size_t   offset= 0;
  uint8_t* p_cur = p_buffer;

  offset += Serialize_uint16(data.header_id, &p_cur);
  offset += Serialize_uint16(data.msg_type,  &p_cur);
  offset += Serialize_uint16(data.len,       &p_cur);
  offset += Serialize_uint16(data.seq_id,    &p_cur);

  return offset;
}

//  ****************************************************************************
inline
size_t Deserialize(QCHeader &data, const uint8_t* p_buffer, size_t len)
{
  if ( !p_buffer
    || len < sizeof(QCHeader))
  {
    return 0;
  }

  size_t   offset= 0;
  const uint8_t* p_cur = p_buffer;

  offset += Deserialize_uint16(data.header_id, &p_cur);
  offset += Deserialize_uint16(data.msg_type,  &p_cur);
  offset += Deserialize_uint16(data.len,       &p_cur);
  offset += Deserialize_uint16(data.seq_id,    &p_cur);

  return offset;
}

//  ****************************************************************************
inline
size_t Serialize(const Location &data, uint8_t* p_buffer, size_t len)
{
  if ( !p_buffer
    || len < sizeof(Location))
  {
    return 0;
  }

  size_t   offset= 0;
  uint8_t* p_cur = p_buffer;

  offset += Serialize_int32(data.latitude,   &p_cur);
  offset += Serialize_int32(data.longitude,  &p_cur);
  offset += Serialize_int32(data.altitude,   &p_cur);
  offset += Serialize_int32(data.height,     &p_cur);

  return offset;
}

//  ****************************************************************************
inline
size_t Deserialize(Location &data, const uint8_t* p_buffer, size_t len)
{
  if ( !p_buffer
    || len < sizeof(Location))
  {
    return 0;
  }

  size_t   offset= 0;
  const uint8_t* p_cur = p_buffer;

  offset += Deserialize_int32(data.latitude,   &p_cur);
  offset += Deserialize_int32(data.longitude,  &p_cur);
  offset += Deserialize_int32(data.altitude,   &p_cur);
  offset += Deserialize_int32(data.height,     &p_cur);

  return offset;
}


//  ****************************************************************************
inline
size_t Serialize(const PIDState &data, uint8_t* p_buffer, size_t len)
{
  if ( !p_buffer
    || len < sizeof(PIDState))
  {
    return 0;
  }

  size_t   offset= 0;
  uint8_t* p_cur = p_buffer;

  offset += Serialize_int64(data.set_point,       &p_cur);
  offset += Serialize_int64(data.delta_time,      &p_cur);
  offset += Serialize_int64(data.current_error,   &p_cur);
  offset += Serialize_int64(data.delta_error,     &p_cur);
  offset += Serialize_int64(data.integral_error,  &p_cur);
  offset += Serialize_int64(data.windup_limit,    &p_cur);

  offset += Serialize_uint64(data.desc.Kp,        &p_cur);
  offset += Serialize_uint64(data.desc.Ki,        &p_cur);
  offset += Serialize_uint64(data.desc.Kd,        &p_cur);
  offset += Serialize_int64 (data.desc.range_min, &p_cur);
  offset += Serialize_int64 (data.desc.range_max, &p_cur);

  return offset;
}

//  ****************************************************************************
inline
size_t Deserialize(PIDState &data, const uint8_t* p_buffer, size_t len)
{
  if ( !p_buffer
    || len < sizeof(PIDState))
  {
    return 0;
  }

  size_t   offset= 0;
  const uint8_t* p_cur = p_buffer;

  offset += Deserialize_int64(data.set_point,       &p_cur);
  offset += Deserialize_int64(data.delta_time,      &p_cur);
  offset += Deserialize_int64(data.current_error,   &p_cur);
  offset += Deserialize_int64(data.delta_error,     &p_cur);
  offset += Deserialize_int64(data.integral_error,  &p_cur);
  offset += Deserialize_int64(data.windup_limit,    &p_cur);
  
  offset += Deserialize_uint64(data.desc.Kp,        &p_cur);
  offset += Deserialize_uint64(data.desc.Ki,        &p_cur);
  offset += Deserialize_uint64(data.desc.Kd,        &p_cur);
  offset += Deserialize_int64 (data.desc.range_min, &p_cur);
  offset += Deserialize_int64 (data.desc.range_max, &p_cur);

  return offset;
}


//  ****************************************************************************
inline
size_t Serialize(const Battery &data, uint8_t* p_buffer, size_t len)
{
  if ( !p_buffer
    || len < sizeof(Battery))
  {
    return 0;
  }

  size_t   offset= 0;
  uint8_t* p_cur = p_buffer;

  ::memcpy(p_cur, &data.cell_count, sizeof(uint8_t));
  offset += sizeof(uint8_t);
  p_cur  += sizeof(uint8_t);

  offset += Serialize_uint16(data.cell_level[0], &p_cur);
  offset += Serialize_uint16(data.cell_level[1], &p_cur);
  offset += Serialize_uint16(data.cell_level[2], &p_cur);
  offset += Serialize_uint16(data.cell_level[3], &p_cur);

  return offset;
}

//  ****************************************************************************
inline
size_t Deserialize(Battery &data, const uint8_t* p_buffer, size_t len)
{
  if ( !p_buffer
    || len < sizeof(Battery))
  {
    return 0;
  }

  size_t   offset= 0;
  const uint8_t* p_cur = p_buffer;

  ::memcpy(&data.cell_count, p_cur, sizeof(uint8_t));
  offset += sizeof(uint8_t);
  p_cur  += sizeof(uint8_t);

  offset += Deserialize_uint16(data.cell_level[0], &p_cur);
  offset += Deserialize_uint16(data.cell_level[1], &p_cur);
  offset += Deserialize_uint16(data.cell_level[2], &p_cur);
  offset += Deserialize_uint16(data.cell_level[3], &p_cur);

  return offset;
}

//  ****************************************************************************
inline
size_t Serialize(const Batteries &data, uint8_t* p_buffer, size_t len)
{
  if ( !p_buffer
    || len < sizeof(Batteries))
  {
    return 0;
  }

  size_t   offset= 0;
  uint8_t* p_cur = p_buffer;

  ::memcpy(p_cur, &data.count, sizeof(uint8_t));
  offset += sizeof(uint8_t);
  p_cur  += sizeof(uint8_t);

  for (uint8_t i = 0; i < data.count; ++i)
  {
    offset += Serialize(data.battery[i], p_cur, len - offset); 
    p_cur  += offset;
  }

  return offset;
}

//  ****************************************************************************
inline
size_t Deserialize(Batteries &data, const uint8_t* p_buffer, size_t len)
{
  if ( !p_buffer
    || len < sizeof(Batteries))
  {
    return 0;
  }

  size_t   offset= 0;
  const uint8_t* p_cur = p_buffer;

  ::memcpy(&data.count, p_cur, sizeof(uint8_t));
  offset += sizeof(uint8_t);
  p_cur  += sizeof(uint8_t);

  for (uint8_t i = 0; i < data.count; ++i)
  {
    offset += Deserialize(data.battery[i], p_cur, len - offset); 
    p_cur  += offset;
  }

  return offset;
}


//  ****************************************************************************
inline
int write_message(COMPORT comm, 
                  const uint8_t* p_buffer, 
                  size_t len)
{
  return serial_write(comm, p_buffer, len);
}



//  ****************************************************************************
inline
int read_message(COMPORT comm, 
                 uint8_t* p_buffer, 
                 size_t len)
{
  if (!p_buffer)
  {
    return 0;
  }


  enum read_state
  {
    HDR_1     = 1,
    HDR_2,
    ID_1,
    ID_2,
    LEN_1,
    LEN_2,
    DATA,
    DONE
  };

  read_state state      = HDR_1;
  size_t     bytes_read = 0;
  uint16_t   msg_len    = 0;

//  uint8_t bb[300];
//  int     count = 0;

  // Read the bytes one-by-one looking for the message header indicator.
  // Continue progressing the parse through a message until 
  // a complete message has been read.
  do
  {
    byte_t data;
    if (!read_byte(comm, data))
      continue;

//    bb[count] = data;
//    count++;

//    if (count >= 300)
//      count = 0;


    switch (state)
    {
    case HDR_1:
      if (data == k_qc_msg_header >> 8)
      { 
        p_buffer[bytes_read] = data;
        bytes_read++;

        state = HDR_2;
      }

      break;

    case HDR_2:
      if (data == (k_qc_msg_header & 0x00FF))
      { 
        p_buffer[bytes_read] = data;
        bytes_read++;

        state = ID_1;
      }
      else
      {
        // False-alarm, reset.
        state      = HDR_1;
        bytes_read = 0;
      }

      break;

    case ID_1:
      p_buffer[bytes_read] = data;
      bytes_read++;

      state = ID_2;
      break;

    case ID_2:
      p_buffer[bytes_read] = data;
      bytes_read++;

      state = LEN_1;
      break;

    case LEN_1:
      msg_len = uint16_t(data) << 8;

      p_buffer[bytes_read] = data;
      bytes_read++;

      state = LEN_2;
      break;

    case LEN_2:
      msg_len |= uint16_t(data);

// TODO: Return and inspect, 
//      msg_len  = htons(msg_len);

      p_buffer[bytes_read] = data;
      bytes_read++;

      state = DATA;
      break;

    case DATA:
      p_buffer[bytes_read] = data;
      bytes_read++;

      if (bytes_read >= msg_len)
      {
        state = DONE;
      }
      break;

    case DONE:
      break;

    default:
      bytes_read = 0;
      state      = HDR_1;
    }

  }
  while ( bytes_read < len
       && state != DONE);

  return int(bytes_read);
}


#endif
