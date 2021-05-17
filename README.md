RC-PWM-Mixer firmware for STM8 MCUs
===================================

This firmware turns an STM8 MCU into an RC (radio control) bidirectional 2-channel PWM motor controller:

* By default, RC channel 1 controls forward/reverse for both motors while RC channel 2 controls differential steering between them.
* In independent mode, RC channels 1 and 2 control the motors independently.
* iBUS servo interface can be used to connect to a FlySky receiver.
* Controls need to be returned to their initial positions (re-armed) on startup or signal loss.
* Status LED indicates how many RC channels are active by blinking that number of times.
* Watchdog resets the device in case of signal loss.


Supported MCUs
--------------

+ STM8S003xx
+ STM8S005xx
+ STM8S103xx
+ STM8S105xx


Dependencies
------------

+ cmake >= 3.12
+ sdcc
+ stm8flash


Installation
------------

Check out _#defines_ at the beginning of _src/config.h_ to see if you need to change settings before building the firmware. Options to consider:

* The default PWM frequency is 16kHz at 8Mhz clock. It can be decreased by a multiple of 2 using the PWM_DIV value. It can also be doubled by switching to 16Mhz clock via the CLK_16MHZ option.

* The output pins can be inverted via the *_INV options.

* If iBUS is used, RC-to-RX channel mapping can be configured using the IBUS_CHx values.

* Internal clock source trimming may be necessary for correct RC signal sampling by tuning the CLK_TRIM value. To check accuracy, build in debug mode (uncomment the corresponding _#define_ in _src/config.h_), install the firmware, connect your console to pin D5 (UART_TX) at 115200 baud, make sure the control sticks are properly adjusted and are in the middle position, and check that CHx values are as close to 1500 as possible. If they are higher, the clock is too fast (increase CLK_TRIM). If they are lower, the clock is too slow (decrease CLK_TRIM). Don't forget to disable debug mode, rebuild and reinstall. A more robust approach would be to use an external clock source (if present) by enabling the CLK_EXT option. Alternatively, a FlySky receiver can be connected via iBUS.


To build, run (with your own MCU):

    cmake -B build -D MCU=STM8S103F3
	cd build
    make

To install on the device, run (with your own programmer):

	sudo make PROG=stlinkv2 flash


Pinout
------

| Pin     | I/O | Description    |
|---------|-----|----------------|
| C1 (C6) | IN  | RC channel 1   |
| C3      | IN  | RC channel 2   |
| D6      | IN  | iBUS servo     |
| D4 (C5) | OUT | PWM 1          |
| D3      | OUT | PWM 2          |
| C7      | OUT | Forward 1      |
| C4      | OUT | Forward 2      |
| D2      | OUT | Reverse 1      |
| A3      | OUT | Reverse 2      |
| E5 (B5) | OUT | Status LED     |

Pins in brackets pertain to STM8Sx03Fx (20-pin) MCUs.
