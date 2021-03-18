# AIS Project development diary

## 18/03/21:

### Initial state:
The projects initial state is inherited from "temalab" project, where from we have the board with the following properties:

- connectivity with stk3700 uController dev board (EFM32GG)
- I2C bus
  - Light sensor (VEML7700)
  - temperature sensor (LM-75)
- pump drive circuit
- device for capacity measurement (555 timer)
- 12VDC input
- screw terminals
  - capacity probe
  - pump

the board is manufactured, and measured, results:

- all I2C peripherals working
- timer working with strange duty cycle
- pump drive untested as of yet

### Goal:

The final goal is to make a system which can be set up in a domestic environment. The system consists of two main parts:
- node modules, which are to communicate with the core unit, recieving instructions, and watering plants. The applied controller is an ESP32.

- Core module, which is most likely a raspberry pi taking care of any type of user input, and controlling the nodes.

### Progression:

Several projects have been created using simplicity studio. The main goal is to test all devices using this prototype board, and the stk3700 and test all functionalities.

The last couple days I was struggling with PCNT configuration and timer configuration.
I2C configuration was done through the simplicity studios built in configurator. With some help, I was able to get I2C working.

Pulse counter is however irresponsive in the main project. I decided to launch a sample project where everything is configured properly. The results was good, the peripheral was working as intended.

I have built an external square signal generator from an ESP32 for testing purposes, using arduino framework, with platformIO. (This framework is easy to use, and platformIO is an easy to use, comfortable tool) This small 'module' is generating a low frequency square signal, and makes it easier to setup a new peripheral.

I started to change the code in the sample project, and comment out anything that is not strictly required for my application. This way the project is still functioning, compilation was successful.  In debug mode i tracked the counter value in a variable, the peripheral was working.

This makes me think that something was not properly configured with the original project.

Simplicity Studio 5s new project structure is much more chaotic for this type of small projects. I was thinking about creating a raw project where all unnecessary stuff is removed. Basic configuration, and learning could be much efficient that way.
