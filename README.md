# FYP-Vital-Monitoring-Ventilator-with-Webpage-for-Rural-Healthcare


Smart Portable Vital Monitoring Ventilator

Student: Muhammad Hamza Taher
Supervisor: Dr Munira Raja
Assessor: Dr Roberto Ferrero



A low-cost portable ventilator and vital monitoring system designed for rural healthcare environments with limited access to electricity and medical infrastructure.

The system integrates patient monitoring sensors, a Raspberry Pi Pico W microcontroller, wireless data transmission, and a solar-powered energy system to create a portable and sustainable medical support device.




Project Overview

- Rural areas in South Asia suffer from high levels of chronic respiratory diseases, contributing to approximately 12% of deaths across the region. 

- Poster Update 5

- Combined with limited access to medical facilities, this leads to increased mortality rates.

- This project aims to develop a cost-effective monitoring ventilator system that can assist healthcare workers by:

- Monitoring patient vital signs

- Providing ventilation assistance

- Transmitting medical data through a webpage

- Operating in low-power environments using solar energy





Key Features

- Portable ventilator monitoring system

- Real-time vital sign monitoring

- Wireless data transmission via WiFi

- Fast responsive companion webpage

- Alarm system for abnormal readings

- Solar powered charging system

- Adjustable ventilator operation

- Low-cost hardware design





System Architecture

- The system is built around a Raspberry Pi Pico W, which manages:

- Sensor data acquisition

- Motor control for ventilation

- LCD display updates

- Wireless webpage communication

- The microcontroller balances these tasks to avoid signal starvation and maintain stable operation. 




Hardware Components
- Component	Purpose
- Raspberry Pi Pico W	Main microcontroller
- Vital Sign Sensors	Monitor patient health data
- Servo / Motor System	Controls ventilator airflow
- LCD Display	Displays patient data locally
- LiPo Battery	Portable power source
- Solar Tracker	Tracks sunlight to maximise energy capture
- Solar Panel	Renewable power supply
- ON/OFF Switch	Power control
- Solar Power System

- The system includes a solar tracking module to support operation in areas with limited electricity.




The solar tracker:

- Tracks the sun throughout the day

- Maximises solar panel efficiency

- Converts solar energy into a stable 5V output

- Charges a rechargeable LiPo battery

- This allows the ventilator to operate in off-grid environments. 





Electronics Design

- The circuit was designed using KiCad.

- Key features of the schematic include:

- Shared 3.3V power rail

- I2C communication between sensors and Pico

- Dedicated power subsystem for the LiPo battery

- Direct servo connection for ventilator control

- ON/OFF power control system

- The device boots and initializes sensors before establishing WiFi communication.





Sensor Calibration

- All sensors were calibrated against known reference devices to ensure reliable monitoring.


Temperature

- Reference: Medical thermometer

- Adjustment: Code libraries

- Issues: Sensor refresh rate


Air Pressure

- Reference: Manufacturer datasheet

- Adjustment: Code libraries

- Issues: Tube leakage


Heart Rate

- Reference: Apple Watch

- Adjustment: Library calibration

- Issues: Tube pressure affecting readings


Blood Oxygen (SpO₂)

- Reference: Apple Watch

- Adjustment: Library calibration

- Issues: Adjustments required for darker skin tones




Market Context

- The global medical technology market is expected to reach:

- $712.9 billion by 2030

- IoT monitoring devices represent one of the largest growth sectors, accounting for approximately 18% of the market share. 

- This project demonstrates how low-cost embedded systems can contribute to this growing healthcare technology space.



Future Improvements

- Potential future developments include:

- Improved solar tracker battery system

- Longer wireless transmission range

- Secure global patient database

Data encryption for medical compliance

Additional sensors (e.g., ECG monitoring)

Medical regulatory compliance (WHO standards)
