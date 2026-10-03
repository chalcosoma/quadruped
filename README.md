# Quadruped Robot

A custom 12-degree-of-freedom quadruped robot built around hobby servos and a Raspberry Pi Pico.

<p align="center">
  <img src="https://github.com/user-attachments/assets/b1dc6760-4037-48ae-9d39-96bc865864ea" alt="Quadruped Robot" width="900" />
</p>

## Overview

This project brings together mechanical design, embedded firmware, and electronics. I designed the robot's custom 3D-printed chassis, circuits, and firmware from scratch.

The system is organized around a modular firmware architecture:
- `Joint`: individual servo control and angle management
- `Leg`: kinematic solving and leg motion primitives
- `Quadruped`: multi-leg coordination, posture control, and gait behavior

The goal of the project was to build a functioning robot capable of standing, crouching, tilting, and walking using a basic gait cycle.

## Features

- Fully actuated 3-degree-of-freedom leg design
- Multiple robot poses and movement states
- Bluetooth serial control interface
- Modular firmware structure for easy expansion

## Hardware

The robot combines several custom and off-the-shelf systems:

- Raspberry Pi Pico microcontroller
- 12 RC servos
- Custom 3D-printed chassis and leg structure
- Custom PCB-based electronics for power and control distribution
- Bluetooth communication interface

## Software Architecture

The firmware is split into a few key components:

```text
firmware/
└── pico/
    ├── CMakeLists.txt
    ├── pico_sdk_import.cmake
    ├── src/
    │   └── main.cpp
    └── lib/
        ├── Joint/
        │   ├── Joint.h
        │   └── Joint.cpp
        ├── Leg/
        │   ├── Leg.h
        │   └── Leg.cpp
        └── Quadruped/
            ├── Quadruped.h
            └── Quadruped.cpp
```

## Motion and Control

The robot supports a small command set for direct behavior control over serial/Bluetooth:

- `home`
- `stand`
- `crouch`
- `walk`
- `up`
- `down`
- `right`
- `off`
- `on`

These commands enable:
- returning to a neutral home position
- standing posture
- crouched posture
- diagonal trot gait
- pitch and roll adjustments for body movement
- power toggling for the legs

## CAD and Design

The repository includes CAD models for the robot chassis and structural components under:

```text
CAD/
└── step/
    └── v4/
```

## Project Goals

This robot was designed as a hands-on embedded systems and robotics project to explore:
- mechanical design and fabrication
- embedded firmware
- actuator control and kinematics
- electronics design and validation
- system debugging and integration

## Future Improvements

Potential next steps for the project include:
- implementing smoother walking transitions
- adding inertial sensing and stabilization feedback
- improving gait tuning and terrain adaptation
- expanding remote control and autonomous behaviors
- investigating more robust mechanical and electrical redesigns
- upgrading to brushless motors for greater precision and torque

## License

This project is for personal and educational use.
