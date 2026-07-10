# Designed and built a fully functional electronic prosthetic limb across 3 development phases over 4 years
# Low-cost Carbon Fiber Bionic Arm with Smart Multi-Modal Sensor Fusion and IoT-Enabled Adaptive Control Bionic Arm with Edge AI & PID Control  

This repository contains the Firmware, Hardware designs, and Documentation for the 3rd generation of a Smart Bionic Arm. The project integrates advanced sensor fusion, embedded machine learning, and high-efficiency control loops to provide natural upper-limb prosthetic movement.

##  Key Achievements
- **IEEE YESIST12 2026:** Selected as a **Finalist** for the Grand Finale.
- **Genius Forum 2025 (ملتقى العباقرة):** Awarded **1st Place** overall.
- **Genius Forum 2023:** Awarded **2nd Place** (Version 2).
- Presented at the 3rd International Conference on Communications at the Air Defense College, Alexandria.

##  Technical Specifications & Tuning
- **Sensor Activation Timeout:** Calibrated strictly to **15 seconds** to optimize response and power consumption.
- **Servo Motor Runtime:** Tuned to **2200** for precise angular positioning and stable holding torque.
- **AI Signal Processing:** Embedded Edge AI analysis on the ESP32 to filter raw signals from the custom EMG and Armband Force sensors (Note: Signal analysis was performed purely on raw filtering and classification algorithms; **Z-score normalization was not utilized in this pipeline**).
- **Control System:** Closed-loop PID Controller simulated to smooth the output of the wrist stepper motor.

##  References, Credits & Custom Upgrades
This project evolved over three major iterations, building upon open-source and academic foundations with significant custom engineering upgrades:

*   **EMG Sensor Implementation:** The base schematic was adapted from the [CharlesLabs OpenEMG Arduino Sensor](https://charleslabs.fr/en/project-OpenEMG+Arduino+Sensor). **Custom Upgrades:** To eliminate signal noise, a custom hardware analog filter was engineered using the **LM358 Low-Power Dual Operational Amplifier**, coupled with a software-level **Moving Average Digital Filter** developed in the firmware.
*   **V1 Design:** The initial structural framework was based on [Project Three by mdesigns](https://mdesigns.space/projects/project-three-2jhsa). **Custom Upgrades:** Integrated a Bluetooth module and developed a custom mobile application for wireless control.
*   **V2 Design:** A fully custom mechanical and electronic design developed entirely from scratch.
*   **V3 Design (Current - 2025/2026):** The structural foundation was inspired by the compliant hand research by *Akhtar A. et al. (Annu Int Conf IEEE Eng Med Biol Soc. 2016; PMID: 28261008)*. **Custom Upgrades:** While the original research utilized 3D-printed plastic and a single EMG sensor, our V3 iteration was entirely re-engineered utilizing **Carbon Fiber** for an optimal strength-to-weight ratio. Furthermore, the system architecture was upgraded to implement **Multi-Sensor Fusion** and processes complex biological signals via **Edge AI**.

##  Future Work & TODOs (Incomplete/WIP)
- [x] Initial Firmware and Docs push.
- [ ] Upload full Hardware schematics (Altium/KiCad) and 3D printing models.
- [ ] Upload Simulation files (MATLAB/Proteus).
- [ ] Finalize the PCB layout for the power management and Step-Up circuit.
