# GainMeter Plugin

A simple gain control plugin with a real-time peak meter, built using modern C++ and JUCE. 

**Work in Progress:** This project is actively being developed. Core architecture is complete — feature development and polish are ongoing.

## Project Goals

- Real-time gain adjustment (linear or dB scale)
- Peak meter visualization (per channel)
- Exported as a VST3 and AU plugin
- Built using modern C++ (C++17 or C++20)
- CMake-based JUCE project

## Tools & Technologies

- C++
- JUCE Framework
- CMake
- Xcode
- Git / GitHub

## Features

- Adjustable gain via slider
- Real-time peak meter display
- Clean UI using JUCE Components
- Modular code using modern OOP patterns

## Build Instructions

### Prerequisites

- JUCE 7.x installed
- CMake (3.15+)
- DAW / plugin host
- IDE (Xcode, Visual Studio, or CLion)

### Build

```bash
git clone git@github.com:sindiv/gainmeter-plugin.git
cd gainmeter-plugin
cmake -Bbuild -GXcode  # or -G"Visual Studio 17 2022"
cmake --build build --config Release