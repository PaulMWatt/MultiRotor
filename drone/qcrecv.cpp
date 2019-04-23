/// @file qcrecv.cpp
/// 
/// Receives controls for the drone.
///
//  ****************************************************************************

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>

#include <iostream>
#include <stdlib.h>
#include <thread>
#include <chrono>

#include "qcrecv.h"
#include "drone.h"
#include "serial.h"

#include "utility/util.h"

using std::cout;
using std::endl;

namespace  // unnamed 
{

bool          g_is_listening    = false;
bool          g_is_connected    = false;

int           g_conn            = 0;

uint16_t      g_next_sequence   = 0;

uint16_t      g_last_recv_seq   = 0;

Drone        *gp_drone          = nullptr;
std::thread*  gp_receiver       = nullptr;

}

//  ****************************************************************************
bool IsListening()
{
  return g_is_listening;
}

//  ****************************************************************************
bool IsConnected()
{
  return g_is_connected;
}

//  ****************************************************************************
uint16_t GetSequenceId()
{
  return g_next_sequence++;
}

//  ****************************************************************************
bool UpdateLatestSeqId(uint16_t seq)
{
  if ( seq < g_last_recv_seq
    && g_last_recv_seq != std::numeric_limits<uint16_t>::max())
  {
    return false;
  }

  g_last_recv_seq = seq;

  return true;
}

//  ****************************************************************************
size_t Serialize(const QCBeaconAckMsg &data, uint8_t* p_buffer, size_t len)
{
  if ( !p_buffer
    || len < sizeof(QCBeaconAckMsg))
  {
    return 0;
  }

  size_t  offset = 0;
  uint8_t *p_cur = p_buffer;

  offset += Serialize(data.header, p_cur, len);

  offset += Serialize_int32(data.cookie,  &p_cur);
  offset += Serialize_int32(data.status,  &p_cur);

  return offset;
}

//  ****************************************************************************
size_t Serialize(const QCArmAck &data, uint8_t* p_buffer, size_t len)
{
  if ( !p_buffer
    || len < sizeof(QCArmAck))
  {
    return 0;
  }

  size_t  offset = 0;
  uint8_t *p_cur = p_buffer;

  offset += Serialize(data.header, p_cur, len);

  offset += Serialize_int32(data.cookie,  &p_cur);
  offset += Serialize_int32(data.status,  &p_cur);

  // Populate the position of the drones starting (base) location.
  p_cur[0] = data.base_position.is_valid;
  offset++;

  p_cur = p_buffer + offset;

  offset += Serialize_int32(data.base_position.latitude,   &p_cur);
  offset += Serialize_int32(data.base_position.longitude,  &p_cur);
  offset += Serialize_int32(data.base_position.altitude,   &p_cur);
  offset += Serialize_int32(data.base_position.height,     &p_cur);

  return offset;
}

//  ****************************************************************************
void ArmDrone(const uint8_t* p_buffer, size_t len)
{
  QCArmMsg msg;

  size_t  offset = 0;
  const uint8_t *p_cur = p_buffer;

  offset += Deserialize(msg.header, p_cur, len);

  int32_t cookie = 0;
  offset += Deserialize_int32(cookie, &p_cur);
  
  g_is_connected = true;

  QCArmAck ack;
  PopulateQCHeader(ack);

  const size_t  ack_len = sizeof(QCArmAck);
  uint8_t       ack_buffer[ack_len];
  ack.cookie = cookie;
  ack.status = 0;

  Drone  *p_drone = gp_drone;
  if (p_drone)
  {
    p_drone->base_location(ack.base_position);
  }

  Serialize(ack, ack_buffer, ack_len);

  write(g_conn, 
        ack_buffer,
        ack_len);

  cout << "Connect acknowledgement sent\n";

  if (p_drone)
  {
    p_drone->activate();
  }
}

//  ****************************************************************************
void ProcessCommand(const uint8_t* p_buffer, size_t len)
{
  if ( !p_buffer
    || len != sizeof (QCControlMsg))
  {
    cout << "An invalid command buffer has been received.\n";
    return;
  }

  QCControlMsg data;

  // Extract the values from the receive buffer.
  int      offset = 0;

  // Read the header fields.
  offset += Deserialize(data.header, p_buffer, len);

  if (!UpdateLatestSeqId(data.header.seq_id))
  {
    // Stale data, ignore this message.
    return;
  }

  // Read the control fields.
  const uint8_t *p_cur = p_buffer + offset;

  Deserialize_int16(data.control.roll,    &p_cur);
  Deserialize_int16(data.control.pitch,   &p_cur);
  Deserialize_int16(data.control.yaw,     &p_cur);
  Deserialize_int16(data.control.thrust,  &p_cur);

  Drone  *p_drone = gp_drone;
  if (p_drone)
  {
    p_drone->command(data.control);
  }
}

//  ****************************************************************************
void ProcessGetControlMode(const uint8_t* p_buffer, size_t len)
{
  if ( !p_buffer
    || len != sizeof (QCGetControlModeMsg))
  {
    cout << "An invalid command buffer has been received.\n";
    return;
  }

  QCGetControlModeMsg data;

  // Extract the values from the receive buffer.
  int      offset = 0;

  // Read the header fields.
  offset += Deserialize(data.header, p_buffer, len);

  if (!UpdateLatestSeqId(data.header.seq_id))
  {
    // Stale data, ignore this message.
    return;
  }

  // That's it, generate the response with the current control directives.
  Drone  *p_drone = gp_drone;
  if (p_drone)
  {
    // TODO: Send the current configuration.
  }
}

//  ****************************************************************************
void ProcessSetControlMode(const uint8_t* p_buffer, size_t len)
{
  if ( !p_buffer
    || len != sizeof (QCControlModeMsg))
  {
    cout << "An invalid command buffer has been received.\n";
    return;
  }

  QCControlModeMsg data;

  // Extract the values from the receive buffer.
  int      offset = 0;

  // Read the header fields.
  offset += Deserialize(data.header, p_buffer, len);

  if (!UpdateLatestSeqId(data.header.seq_id))
  {
    // Stale data, ignore this message.
    return;
  }

  // Read the control mode fields.
  const uint8_t *p_cur = p_buffer + offset;

  data.control_mode  = p_cur[0];
  data.disable_roll  = p_cur[1];
  data.disable_pitch = p_cur[2];
  data.disable_yaw   = p_cur[3];

  Drone  *p_drone = gp_drone;
  if (p_drone)
  {
    p_drone->control_mode(data.control_mode == rate_control ? rate_control : angle_control);
    p_drone->use_roll_control (1 == data.disable_roll);
    p_drone->use_pitch_control(1 == data.disable_pitch);
    p_drone->use_yaw_control  (1 == data.disable_yaw);
  }
}

//  ****************************************************************************
void AdjustGain(const uint8_t* p_buffer, size_t len)
{
  if ( !p_buffer
    || len != sizeof (QCAdjustGainMsg))
  {
    cout << "An invalid adjust gain buffer has been received.\n";
    return;
  }

  QCAdjustGainMsg data;

  // Extract the values from the receive buffer.
  int      offset = 0;

  // Read the header fields.
  offset += Deserialize(data.header, p_buffer, len);

  if (!UpdateLatestSeqId(data.header.seq_id))
  {
    cout << "Ignoring due to stale counter.\n";
    // Stale data, ignore this message.
    return;
  }

  memcpy(&data.type, p_buffer + offset, sizeof(data.type));
  offset += sizeof(data.type);
  data.type = PIDType(ntohl(data.type));

  PIDDesc gain;
  // Read each of the gain fields
  // then convert the byte order of the entire structure.
  memcpy(&gain.Kp, p_buffer + offset, sizeof(int64_t));
  offset += sizeof(int64_t);
  memcpy(&gain.Ki, p_buffer + offset, sizeof(int64_t));
  offset += sizeof(int64_t);
  memcpy(&gain.Kd, p_buffer + offset, sizeof(int64_t));
  offset += sizeof(int64_t);
  memcpy(&gain.range_min, p_buffer + offset, sizeof(int64_t));
  offset += sizeof(int64_t);
  memcpy(&gain.range_max, p_buffer + offset, sizeof(int64_t));
  offset += sizeof(int64_t);

  data.desc = to_host(gain);

  Drone  *p_drone = gp_drone;
  if (p_drone)
  {
    // Convert the encoded integers back to doubles.
    double Kp = decode_PID(data.desc.Kp);
    double Ki = decode_PID(data.desc.Ki);
    double Kd = decode_PID(data.desc.Kd);

    cout << "Kp: " << Kp << ", Ki: " << Ki << ", Kd: " << Kd << endl;

    if (data.type == k_roll)
    {
      p_drone->adjust_gain_roll(Kp, Ki, Kd);
    }
    else if (data.type == k_roll_rate)
    {
      p_drone->adjust_gain_roll_rate(Kp, Ki, Kd);
    }
    else if (data.type == k_pitch)
    {
      p_drone->adjust_gain_pitch(Kp, Ki, Kd);
    }
    else if (data.type == k_pitch_rate)
    {
      p_drone->adjust_gain_pitch_rate(Kp, Ki, Kd);
    }
    else if (data.type == k_rotate)
    {
      p_drone->adjust_gain_rotation(Kp, Ki, Kd);
    }
  }
}

//  ****************************************************************************
void Halt(const uint8_t *p_buffer, size_t len)
{
  cout << "Received Halt Command.\n";

  QCHaltMsg msg;

  size_t  offset = 0;
  const uint8_t *p_cur = p_buffer;

  offset += Deserialize(msg.header, p_cur, len);

  uint32_t status = 0;
  offset += Deserialize_uint32(status, &p_cur);
  
  QCDisarmMsg disconnect;
  PopulateQCHeader(disconnect);

  const size_t  disconnect_len = sizeof(QCDisarmMsg);
  uint8_t       buffer[disconnect_len];

  Serialize(disconnect.header, buffer, disconnect_len);

  disconnect.status = status;

  write (g_conn, 
         buffer,
         disconnect_len);

  HaltListening();
}

//  ****************************************************************************
void DispatchMessage(const uint8_t* p_buffer, size_t len)
{
  if (len > 0)
  {
    // Identify the type of message.
    uint16_t type = DecodeMessageType(p_buffer, len);

    switch (type)
    {
    case k_qc_msg_arm:
      cout << "Received Arm Command" << endl;
      ArmDrone(p_buffer, len);
      break;
    case k_qc_msg_control:
      ProcessCommand(p_buffer, len);
      break;
    case k_qc_msg_adjust_gain:
      cout << "Received AdjustGain Command" << endl;
      AdjustGain(p_buffer, len);
      break;
    case k_qc_msg_disarm:
    case k_qc_msg_halt:
      cout << "Received Disarm Command" << endl;
      Halt(p_buffer, len);
      break;
    case k_qc_msg_get_control_mode:
      ProcessGetControlMode(p_buffer, len);
      break;
    case k_qc_msg_control_mode:
      ProcessSetControlMode(p_buffer, len);
      break;
    default:
      cout << "unknown msg-type: " << type << endl;
    }
  }
  else
  {
    cout  << "Error (" << errno 
          << "): Failed to read incoming command.\n";
  }
}

//  ****************************************************************************
void ControlReceiver()
{
  const size_t k_buffer_size = 2048;

  // Initialize the reply serial port for responses.
  g_conn = open("/dev/ttyO1", O_RDWR | O_NOCTTY | O_NDELAY);

  if (g_conn < 0)
  {
    cout << "Error - Cannot open serial port 1 to receive control commands.\n";
    return;
  }

  termios options;

  tcgetattr(g_conn, &options);

  // Configure as raw.
  cfsetispeed(&options, B57600);
  cfsetospeed(&options, B57600);

  options.c_cflag &= ~(PARENB | CSIZE | CRTSCTS);
  options.c_cflag |= CS8 | CREAD;

  options.c_lflag &= ~(ICANON | ECHO | IEXTEN | ISIG);
  options.c_iflag &= ~(IXON | ISTRIP | INPCK | ICRNL | BRKINT);
  options.c_oflag &= ~(OPOST);

  options.c_cc[VMIN]  = 1;
  options.c_cc[VTIME] = 0;


  tcflush(g_conn, TCIFLUSH);

  tcsetattr(g_conn, TCSANOW, &options);

  // Ready to receive content.
  cout << "System Ready to Receive Commands..." << endl;

  // Listen and dispatch the commands received 
  // from the control station.
  while (IsListening())
  {
    uint8_t     buffer[k_buffer_size];     
    
    // TODO: Read and parse into packets.
    int len = read_message( g_conn, 
                            buffer,
                            k_buffer_size);
    if (len > 0)
    {
      DispatchMessage(buffer, len);
    }
  }

  cout << "System shutting down receive port..." << endl;

  close(g_conn);
}


//  ****************************************************************************
void StartListening(Drone *p_drone)
{
  if (!p_drone)
    return;

  if (g_is_listening)
    return;

  // Release the previous instance.
  delete gp_receiver;
  gp_receiver = nullptr;

  // Initialize the drone instance.
  gp_drone = p_drone;

  g_is_listening = true;
  gp_receiver = new std::thread(ControlReceiver);
}

//  ****************************************************************************
void HaltListening()
{
  g_is_listening = false;

  Drone  *p_drone = gp_drone;
  if (p_drone)
  {
    p_drone->halt();
  }

  gp_drone       = nullptr;
}

//  ****************************************************************************
int SendDroneState(int conn, const DroneState& state)
{
  QCDroneStateMsg data_out;

  PopulateQCHeader(data_out);

  // Serialize the structure:
  const size_t k_data_len = sizeof(QCDroneStateMsg);
  uint8_t  buffer[k_data_len] = {0};

  size_t offset = 0;

  offset = Serialize(data_out.header, buffer, k_data_len);

  uint8_t *p_cur = buffer + offset;
  
  p_cur[0] = state.is_armed;
  p_cur++;
  offset++;

  // Serialize the Orientation values.
  offset += Serialize_int16(state.orientation.roll,       &p_cur);
  offset += Serialize_int16(state.orientation.pitch,      &p_cur);
  offset += Serialize_int16(state.orientation.yaw,        &p_cur);
  offset += Serialize_int16(state.orientation.roll_rate,  &p_cur);
  offset += Serialize_int16(state.orientation.pitch_rate, &p_cur);
  offset += Serialize_int16(state.orientation.yaw_rate,   &p_cur);

  // Serialize the Location
  *(buffer + offset) = state.position.is_valid;
  offset++;
  p_cur = buffer + offset;

  offset += Serialize_int32(state.position.latitude,  &p_cur);
  offset += Serialize_int32(state.position.longitude, &p_cur);
  offset += Serialize_int32(state.position.altitude,  &p_cur);
  offset += Serialize_int32(state.position.height,    &p_cur);


  // Serialize the Motor state.
  p_cur = buffer + offset;

  offset += Serialize_uint16(state.motor.A,  &p_cur);
  offset += Serialize_uint16(state.motor.B,  &p_cur);
  offset += Serialize_uint16(state.motor.C,  &p_cur);
  offset += Serialize_uint16(state.motor.D,  &p_cur);
  offset += Serialize_uint16(state.motor.E,  &p_cur);
  offset += Serialize_uint16(state.motor.F,  &p_cur);
  offset += Serialize_uint16(state.motor.G,  &p_cur);
  offset += Serialize_uint16(state.motor.H,  &p_cur);


  // Serialize the battery-level information
  offset += Serialize(state.batteries, buffer + offset, k_data_len-offset);


  // Send the datagram to the ground control station for monitoring.
  return write_message(conn, buffer, k_data_len);
}


//  ****************************************************************************
int SendPIDState(
  int              conn, 
  PIDType          type, 
  const DronePIDs& state)
{
  QCDroneStateMsg data_out;

  PopulateQCHeader(data_out);

  // Serialize the structure:
  const size_t k_data_len = sizeof(QCPIDStateMsg);
  uint8_t  buffer[k_data_len] = {0};

  size_t offset = 0;

  offset =  Serialize(data_out.header, buffer, k_data_len);

  uint8_t* p_cur = buffer + offset;
  offset += Serialize_uint32(type, &p_cur);

  // Serialize the specified PID
  switch (type)
  {
  case k_roll:
    offset += Serialize(state.roll_rate, buffer + offset, k_data_len - offset);
    break;
  case k_roll_rate:
    offset += Serialize(state.roll, buffer + offset, k_data_len - offset);
    break;
  case k_pitch:
    offset += Serialize(state.pitch_rate, buffer + offset, k_data_len - offset);
    break;
  case k_pitch_rate:
    offset += Serialize(state.pitch, buffer + offset, k_data_len - offset);
    break;
  case k_rotate:
    offset += Serialize(state.rotation, buffer + offset, k_data_len - offset);
    break;
  case k_rotate_rate:
  default:
    // No current operations
    break;
  }

  // Send the datagram to the ground control station for monitoring.
  return write_message(conn, buffer, k_data_len);
}


//  ****************************************************************************
size_t Serialize(const QCControlModeAck &data, uint8_t* p_buffer, size_t len)
{
  if ( !p_buffer
    || len < sizeof(QCControlModeAck))
  {
    return 0;
  }

  size_t  offset = 0;
  uint8_t *p_cur = p_buffer;

  offset += Serialize(data.header, p_cur, len);

  p_cur[0] = uint8_t(data.control_mode);
  p_cur[1] = data.disable_roll;
  p_cur[2] = data.disable_pitch;
  p_cur[3] = data.disable_yaw;

  offset += 4;

  return offset;
}

//  ****************************************************************************
int ReportControlMode()
{
  QCControlModeAck data_out;

  PopulateQCHeader(data_out);

  Drone  *p_drone = gp_drone;
  if (p_drone)
  {
    data_out.control_mode  = p_drone->control_mode();
    data_out.disable_roll  = p_drone->use_roll_control()  ? 0 : 1;
    data_out.disable_pitch = p_drone->use_pitch_control() ? 0 : 1;
    data_out.disable_yaw   = p_drone->use_yaw_control()   ? 0 : 1;
  }
 
  // Serialize the structure:
  const size_t k_data_len = sizeof(QCControlModeAck);
  uint8_t  buffer[k_data_len] = { 0 };

  size_t offset = 0;

  offset = Serialize(data_out.header, buffer, k_data_len);
  offset = Serialize(data_out, buffer, k_data_len - offset);

  // Send the datagram to the ground control station for monitoring.
  return write_message(g_conn, buffer, k_data_len);
}
 

//  ****************************************************************************
int  ReportDroneState(const DroneState& state)
{
  return SendDroneState(g_conn, state);
}

