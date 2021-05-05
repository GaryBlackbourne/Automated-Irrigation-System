# Automated-Irrigation-System

This project is the successor of the watering-system project, which is to be discontinued.

The first prototype of the node unit is released, and ready to be used.

## Current state:
  - Hardware documentation completed
  - Board schematic and layout are in final state
  - board is ready (one prototype finished and tested)

  - node unit source code is ready, and tested
  - manual requests can be made by <node_ip>/measure

## Usage:
  - clone the project
  - install esp-idf framework, and create a project (details in doc/dev-diary.md)
  - insert source files into `main` directory
  - change the SSID and passphrase in `credentials.h`
  - build and flash the project
