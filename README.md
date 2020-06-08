# Breth helmet project code

This is the code for deco 7385 breathing helmet create by me, a wearable device to help you with breath training in jogging.

## Circuit diagram

Following diagram is the circuit diagram of this project.

Module list

- Arduino: Arduino Uno

- Display: XC-3728 1.3 inch monochrome 64x128 display screen

- Microphone: XC-4438

- Accelerometer: GY-521

The connection for display is not fully accurate, but due to the module missing on **fritzing** so there is one extra connector on display. Refer to officla manual for more information.

![Circuit diagram](https://i.imgur.com/fYxAuBY.jpg)

## Dependencies library

[u8glib](https://github.com/olikraus/u8glib) for display

## How to use

1. Connect Arduino and sensors folloing the diagram.

1. Install dependencies library on Arduino IDE.

1. upload project code to Arduino Uno board.

1. Connect to the power and start.
