# MSP430 PWM Control with UART and Directional LEDs

The project includes interrupt-driven UART communication, button interrupts, PWM control using Timer_B, and real-time clock (RTC) based control logic.
The project uses two buttons to control the direction and operation of the PWM signal.
When the first button is pressed, it sends the string "STANGA" via UART and switches the output to the left direction, turning on the corresponding LED.
Similarly, pressing the second button sends the string "DREAPTA" via UART, which switches the output to the right direction and activates the appropriate LED.
The duty cycle of the PWM signal can be adjusted through UART inputs. Values ranging from 0 to 3 incrementally adjust the duty cycle, either increasing or decreasing it.
In terms of UART control, the system receives data over UART that directly impacts the PWM signal’s duty cycle.






