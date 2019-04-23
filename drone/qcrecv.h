/// @file qcrecv.h
/// 
/// Receives control commands from transmitter
///
//  ****************************************************************************
#ifndef QCRECV_H_INCLUDED
#define QCRECV_H_INCLUDED

#include <cstdint>
#include "qc_msg.h"

//  Forward Declarations *******************************************************
class Drone;

//  ****************************************************************************
void StartListening(Drone *p_drone);
void HaltListening();
bool IsListening();
bool IsConnected();

uint32_t GetControlAddress();
uint16_t GetSequenceId();
bool UpdateLatestSeqId(uint16_t seq);


int  ReportDroneState(const DroneState& state);


#endif
