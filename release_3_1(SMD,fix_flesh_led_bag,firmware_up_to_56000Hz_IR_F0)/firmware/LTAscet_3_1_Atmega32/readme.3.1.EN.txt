- In the firmware bug fixed with LED flash shot.
- Interrupt that ispolzyetsya to generate IR carrier signal stop if there is no reception or transmission of the packet.
- Firmware is now able to operate with a carrier frequency of IR signal up to 56 kHz.

To compile the firmware for the desired frequency, in the file miles_protocol.h two override two parameters:

To 56000 Hz
# define IR_F0 56000
# define ERROR_TOLERANCE 22 / / (If F0 = 56000) Tolerance duration of the received bit (in the "ticks" of the timer)

To 36000 Hz
# define IR_F0 36000
# define ERROR_TOLERANCE 14 / / (If F0 = 36000) Tolerance duration of the received bit (in the "ticks" of the timer)

