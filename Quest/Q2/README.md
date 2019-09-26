# Quest2-Team9-BiyaoLiang-LinMa

Authors: Biyao Liang, Lin Ma, 2018-Oct-14

## Summary
The goal of this quest is to build a system to learn how to use most of the sensors avaliable in the EC444 class tool kit to collect and visualize data with the intention to prepare the group for later projects such as self-driving, navigation and positioning, etc.

Once launched, the device starts turning its motors in order to move itself. The sensors would start collecting data and start plotting data on the program as time goes.

## Evaluation Criteria
- able to collect data from ultrasound sensor
- able to collect data from optical encoder (wheel speed sensor)
- able to collect data from infrared sensor
- able to collect data from lidar
- able to turn motors
- able to display data values on console
- able to provide a visualization of the data using node.js
* Reading the wheel sensor can be a separate demo (does not need to be integrated as it requires battery on USB port)

## Solution Design

As the user launch the device, ESP32 wroom is powered by usb power input and then provide power to the motors and the sensors. The sensors then send collected data back to ESP32, and the data is sent back to computer through an UART connection. The data is then visualized by node.js and chartJS on the computer.

## Sketches and Photos
Diagram:
<center><img src="https://github.com/BU-EC444/Quest2-Team9-BiyaoLiang-LinMa/blob/master/sketches/diagram.jpg" width="50%" /></center>
Wireout:
<center><img src="https://github.com/BU-EC444/Quest2-Team9-BiyaoLiang-LinMa/blob/master/sketches/wire_out.jpg" width="50%" /></center>

## Modules, Tools, Source Used in Solution
<pre>
Example code from esp32 toolchain that we used as reference:

Motor - /esp/esp-idf/examples/peripherals/mcpwm
Lidar - esp/esp-idf/examples/peripherals/uart_echo
Optical encoder (speed monitor) - /esp/esp-idf/examples/peripherals/adc
Ultrasound - /esp/esp-idf/examples/peripherals/rmt_nec_tx_rx
Infrared - /esp/esp-idf/examples/peripherals/adc

Tools:
node.js and canvasJS for visualization.
Other sources:
wireout for optical encoder:https://learn.sparkfun.com/tutorials/qrd1114-optical-detector-hookup-guide#example-circuit
Reference: whizzer.bu.edu
</pre>
## Supporting Artifacts
[Video](https://drive.google.com/file/d/1DzF8CAlQO3bS5qXDRlsxiTDm8q86T76h/view?usp=sharing)

or at this link
[Video](https://youtu.be/NE28Wws4yCg)
