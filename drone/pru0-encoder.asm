;
; PRUSS program to drive a HY-SRF05 sensor.
; The results are stored in memory that is accessible from Linux userspace.
; An interrupt is triggered each time a new sample is ready.
;
; This program is adapted from "ultrasonic", originally writen by Derek Molloy 
; for the book "Exploring BeagleBone"
;
; The following pins are used for signalling on the Beagle Bone Blue:
;
;	BLUE_GP0_PIN_6 (GPIO3_17):	pru0 r30 3
;   BLUE_ENCODER_4 (GPIO1_15):  pru0 r31 15
;

; Definitions to setup PRU shared memory
	.asg    C4,         CONST_SYSCFG
	.asg    C28,		CONST_PRUSHAREDRAM
	.asg	0x22000,	PRU0_CTRL
	.asg    0x28,       CTPPR0           ; page 75

	.asg	0x000,		OWN_RAM
	.asg	0x020,		OTHER_RAM
	.asg    0x100,		SHARED_RAM       ; This is so prudebug can find it.

	.asg    64,			CNT_OFFSET
	.asg    68,			DELAY_OFFSET
	.asg    72,			RAW_DIST_OFFSET
	.asg    76,			SAMPLE_OFFSET


; Definitions for the Trigger Pulse / Echo calculations
	.asg	10,			TRIGGER_PULSE_US
	.asg	200,		INS_PER_US
	.asg	2,			INS_PER_LOOP

	; This value calculates to: 1000
	.asg	((TRIGGER_PULSE_US * INS_PER_US) / INS_PER_LOOP),		TRIGGER_COUNT		
	
	; This value calculates to: 100000
	.asg	(1000 * INS_PER_US) / INS_PER_LOOP,						SAMPLE_DELAY_1MS
	
	; This value calculates to: 380000
	.asg	(SAMPLE_DELAY_1MS * 38),								ECHO_DELAY

	.asg	32,			PRU0_R31_VEC_VALID
	.asg	3,			PRU_EVTOUT_0
	.asg	4,			PRU_EVTOUT_1

; Using register 0 for all temporary storage (reused multiple times)
;  Register Usage Key:
;    r0 - Temporary storage
;    r1 - Delay, number of milliseconds to wait between samples
;    r2 - Echo Pulse Width counter, records the width of the return pulse.
;    r3 - Sample Counter
; 
; Define the entry-point and export it for the linker.
	.clink
	.global start
start:
; Notify the host process that we have loaded successfully
	MOV	  r0, r31
   zero   &r2, 4
   SBCO   &r2, CONST_PRUSHAREDRAM, CNT_OFFSET, 4

   LBCO   &r0, CONST_SYSCFG, 4, 4           ; Enable OCP master port  
   CLR    r0, r0, 4                         ; Clear SYSCFG[STANDBY_INIT] to enable OCP master port
   SBCO   &r0, CONST_SYSCFG, 4, 4

   LDI    r0, SHARED_RAM                    ; Set C28 to point to shared RAM
   LDI32  r1, PRU0_CTRL + CTPPR0            ; Note we use beginning of shared ram unlike example which
   SBBO   &r0, r1, 0, 4                     ;  page 25

;stall:
;   ADD    r3, r3, 1
;   SBCO   &r3, CONST_PRUSHAREDRAM, RAW_DIST_OFFSET, 4
;   JMP    stall


; Read number of samples to read and inter-sample delay
   LDI    r3, 0
   LBCO   &r1, CONST_PRUSHAREDRAM, DELAY_OFFSET, 4
;   LDI    r0, DELAY_OFFSET                  ; Read the delay length
;   LBBO   &r1, r0, 0, 4                     ;   between samples

mainloop:
   LDI    r0, TRIGGER_COUNT                 ; store length of the trigger pulse delay
   SET    r30, r30.t3                       ; set the trigger high

triggering:	                                ; The trigger is a 10us high-signal
   SUB    r0, r0, 1                         ; decrement loop counter
   QBNE   triggering, r0, 0                 ; repeat loop unless zero
   CLR    r30, r30.t3                       ; 10us over, set the triger low - pulse sent
   
; clear the counter and wait until the echo goes high
   LDI    r2, 0								; r2 stores the echo pulse width
   LDI    r5, 0
   LDI32  r6, ECHO_DELAY

wait_for_echo:
   ADD    r5, r5, 1
   
   QBGT   mainloop, r6, r5
   QBBC   wait_for_echo, r31, 15

; start counting (measuring echo pulse width)  until the echo goes low
counting:
   ADD    r2, r2, 1
   QBBS   counting, r31, 15                 ; jump if the echo bit is still high

record:
; at this point the echo is now low - write the value to shared memory
   SBCO   &r2, CONST_PRUSHAREDRAM, RAW_DIST_OFFSET, 4
   ADD    r3, r3, 1
   SBCO   &r3, CONST_PRUSHAREDRAM, SAMPLE_OFFSET, 4
   
update_loop:
; generate an interrupt to update the display on the host computer
   LDI R31.b0, PRU0_R31_VEC_VALID | PRU_EVTOUT_0

   QBNE   setup_delay, r1, 0
   LDI    r0, 1
   JMP    sampledelay

setup_delay:
   MOV    r0, r1                            ; Setup a delay between samples

sampledelay:			; do this loop r1 times (1ms delay each time)
   SUB    r0, r0, 1                         ; decrement counter by 1
   LDI32  r4, SAMPLE_DELAY_1MS

delay1ms:
   SUB	  r4, r4, 1             
   QBNE   delay1ms, r4, 0                   ; keep going until 1ms has elapsed
   QBNE   sampledelay, r0, 0                ; repeat loop unless zero   
   JMP    mainloop

end:
   LDI R31.b0, PRU0_R31_VEC_VALID | PRU_EVTOUT_1
   HALT

