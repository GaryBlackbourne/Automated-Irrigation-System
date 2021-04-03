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

## 19/03/21

### Hardware bugfixes:

An issue has been found in the 555 timer-based capacity sensor. The RA and RB resistors were soldered in a wrong order, which caused extremely high duty-cycle while measured. After the exchange, the board is fully functional, perfect 50% duty-cycle was measured with oscilloscope, frequency was shifted when the capacity connected to the board changed. There is no hardware malfunction discovered yet, the external power-, and drive systems are to be tested.

### Change in prototype:

No explanations were found concerning the Pulse Counter malfunction, in order to speed up the development process, this bug is no longer on todo list. I have a working example with stk3700, and planned to develop one with ESP.

### New Controler:

Experiments started with ESP32 development. Used development tools:
- espressif framework
- vs code
- platformIO plugin

Arduino framework was planned to be used, but with arduino, there seem to be no way to use the PCNT module of the chip.
Further research needs to be done about the characteristics of this module (especially frequency).

The ESP32 uses freeRTOS system, which is preinstalled. This might come handy when I develop networking while reading different sensor values. Although learning freeRTOS system is now a prioritized task.

### New platform:

Espressif controllers can be developed with Arduino IDE, there own IDE, or platformIO. The choice for this project is platformIO, because of the comfortable code completion, function tracking and compatibility with both frameworks (Arduino / Espressif-SDK).

Minor problems occcured during the first compile, after getting used to the environment problems were solved.
A basic project was created, testing GPIO-s with a 'blink' functionality.


## 03/04/21

For easier development I have installed a pure ESP-IDF. Removed platformIO, I try to work with only the framework, and compile my code via terminal. Installation was tough, fish script doesnt seem to work, I needed to use bash. This should be taken care of in the near future to remove a large handicap.
The problem lies with environment variables, I'll write down the solution, as soon as I find them.

## Tutorials were followed:
https://www.youtube.com/watch?v=5IuZ-E8Tmhg
https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/index.html *note that the fish scripts are failed to work*

## First test:
First test project, a simple blink example was successfully uploaded to the chip. I used VS Code, but mostly integrated terminal. Further research needed, how to embed a terminal like bash or fish, into VS Code, and how to get fish working.

For now, I can create projects manually, and compile them from CLI using ESP-IDFs `idf.py build`, and `idf.py flash -p /dev/ttyUSB0` commands. Debugging is not available for the time being.

Tasks are given:

-find fish-s problem in the scripts
-integrate fish, or bash into VS Code
-create a project, and test GPIO, I2C, and PCNT modules.
