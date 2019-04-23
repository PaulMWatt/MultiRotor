//
// PRUSS program to drive a HY-SRF05 sensor.
// The results are stored in memory that is accessible from Linux userspace.
// An interrupt is triggered each time a new sample is ready.
//
// This program is adapted from "ultrasonic", originally writen by Derek Molloy 
// for the book "Exploring BeagleBone"
//

.origin 0                                 // offset of start of program in PRU memory
.entrypoint START                         // program entry point used by the debugger

#define TRIGGER_PULSE_US    10
#define INS_PER_US          200
#define INS_PER_LOOP        2
#define TRIGGER_COUNT       ((TRIGGER_PULSE_US * INS_PER_US) / INS_PER_LOOP)
#define SAMPLE_DELAY_1MS    (1000 * INS_PER_US) / INS_PER_LOOP
#define ECHO_DELAY          (SAMPLE_DELAY_1MS * 38)
#define PRU0_R31_VEC_VALID  32;
#define PRU_EVTOUT_0	    3
#define PRU_EVTOUT_1	    4

// Using register 0 for all temporary storage (reused multiple times)
//  Register Usage Key:
//    r0 - Temporary storage
//    r1 - Delay, number of milliseconds to wait between samples
//    r2 - Echo Pulse Width counter, records the width of the return pulse.
//    r3 - Sample Counter
// 
START:
   // Read number of samples to read and inter-sample delay
   MOV    r3, 0
   MOV    r0, 0x00000000                  // Read the delay length
   LBBO   r1, r0, 0, 4                    //   between samples

MAINLOOP:
   MOV    r0, TRIGGER_COUNT               // store length of the trigger pulse delay
   SET    r30.t5                          // set the trigger high

TRIGGERING:                               // The trigger is a 10us high-signal
   SUB    r0, r0, 1                       // decrement loop counter
   QBNE   TRIGGERING, r0, 0               // repeat loop unless zero
   CLR    r30.t5                          // 10us over, set the triger low - pulse sent
   
   // clear the counter and wait until the echo goes high
   MOV    r2, 0                           // r2 stores the echo pulse width
   MOV    r5, 0
   MOV    r6, ECHO_DELAY
WAIT_FOR_ECHO:
   ADD    r5, r5, 1
   QBGT   MAINLOOP, r6, r5
   QBBC   WAIT_FOR_ECHO, r31.t3

   // start counting (measuring echo pulse width)  until the echo goes low
COUNTING:
   ADD    r2, r2, 1
   QBBS   COUNTING, r31.t3                // jump if the echo bit is still high

RECORD:
   // at this point the echo is now low - write the value to shared memory
   MOV    r0, 0x00000004                  // going to write the result to this address
   SBBO   r2, r0, 0, 4                    // store the count at this address

   ADD    r3, r3, 1
   MOV    r0, 0x00000008                  
   SBBO   r3, r0, 0, 4                    
   
UPDATE_LOOP:
   // generate an interrupt to update the display on the host computer
   MOV R31.b0, PRU0_R31_VEC_VALID | PRU_EVTOUT_1

   QBNE   SETUP_DELAY, r1, 0
   MOV    r0, 1
   JMP    SAMPLEDELAY

SETUP_DELAY:
   MOV    r0, r1                          // Setup a delay between samples

SAMPLEDELAY:			// do this loop r1 times (1ms delay each time)
   SUB    r0, r0, 1                       // decrement counter by 1
   MOV	  r4, SAMPLE_DELAY_1MS            // load 1ms delay into r4

DELAY1MS:
   SUB	  r4, r4, 1             
   QBNE   DELAY1MS, r4, 0                 // keep going until 1ms has elapsed
   QBNE   SAMPLEDELAY, r0, 0              // repeat loop unless zero   
   JMP    MAINLOOP

END:
   MOV R31.b0, PRU0_R31_VEC_VALID | PRU_EVTOUT_0
   HALT

