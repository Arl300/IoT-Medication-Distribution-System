# IoT-Based Medication Distribution System

An IoT-based medication distribution system designed to automate medication dispensing while enabling remote monitoring and scheduling through a Java desktop application.

This project was developed as part of my Master's Thesis in Electronic Engineering and combines embedded systems, electronics, IoT communication, and software development into a complete hardware–software solution.

---

# Overview

The system automatically dispenses medication at scheduled times and communicates with a desktop application using the MQTT protocol.

The project consists of:

* Embedded hardware based on ESP32
* Java desktop application
* MQTT communication
* Power management circuitry
* Mechanical assembly
* Electronic circuit integration

---

# Features

* Automated medication dispensing
* Configurable medication schedules
* MQTT-based communication
* Remote control over different Wi-Fi networks
* Medication retrieval detection using IR sensors
* Visual status indication using LEDs
* Audible notifications using a buzzer
* Battery-powered operation
* Expandable hardware architecture

---

# Hardware

Main components:

* ESP32D Wi-Fi Microcontroller
* ESP32 Extension Board
* 3 Servo Motors
* 3 IR Sensors
* LEDs
* Buzzer
* BMS Module
* 2 Buck Converters
* 2 Rechargeable Lithium Batteries
* Capacitors for servo stabilization
* Current limiting resistors

---

# Software Architecture

Desktop Application

* Java
* Medication scheduling
* Device management

Embedded Firmware

* ESP32
* Servo control
* Sensor monitoring
* MQTT communication
* Alarm management

Communication

* MQTT Broker
* Publish / Subscribe architecture
* Real-time synchronization

---

# System Architecture

Java Desktop Application

⬇

MQTT Broker

⬇

ESP32 Embedded Device

⬇

Servo Motors • IR Sensors • LEDs • Buzzer

---

# Mechanical Assembly

The project includes:

* Electronic module soldering
* Mechanical enclosure assembly
* Component integration
* Wiring
* Power distribution
* Hardware testing

---

# Skills Demonstrated

* Embedded Systems
* Internet of Things (IoT)
* Electronics Engineering
* Embedded Programming
* Java Development
* MQTT Communication
* Hardware Integration
* Soldering
* Circuit Design
* System Integration
* Firmware Development

---

# Future Improvements

* ESP-IDF implementation
* Embedded C firmware architecture
* Custom PCB design
* Android mobile application
* Cloud dashboard
* OTA firmware updates
* Battery monitoring
* Medication history logging
* Multi-user support

---

# Project Gallery

Images and demonstration videos are available inside the repository.

---

# License

This project is intended for educational and portfolio purposes.
