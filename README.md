# Designed and built a fully functional electronic prosthetic limb across 3 development phases over 4 years
# Low-cost Carbon Fiber Bionic Arm with Smart Multi-Modal Sensor Fusion and IoT-Enabled Adaptive Control Bionic Arm with Edge AI & PID Control  


![Bionic Arm Prototype](<img width="300" height="400" alt="20251206_154928" src="https://github.com/user-attachments/assets/4bf4adc4-fecf-4fb3-894e-2f8aaefda2c6" />
)

![Bionic Arm Prototype](<img width="400" height="300" alt="20251016_204006" src="https://github.com/user-attachments/assets/3f609c2c-e592-4168-8b93-bffc2982213a" />
)
This repository highlights the final, highly optimized firmware for the 3rd generation of a Smart Bionic Arm. The project integrates advanced sensor fusion, embedded machine learning, and high-efficiency control loops to provide natural upper-limb prosthetic movement.

## 📌 Repository Scope & 4-Year Development Journey
This project is the culmination of **4 years of continuous research, prototyping, and iterative development (2022 - 2026)**. Due to the massive volume of experimental files, test data, and obsolete iterations accumulated over the years, this repository focuses strictly on the most stable and efficient finalized codes. 

Comprehensive technical details, full system architecture, and exhaustive clinical data are detailed in our upcoming academic research paper, which has been prepared for publication in the **Menoufia Journal of Electronic Engineering Research**.

## 📂 Final Optimized Firmware
Currently, the repository contains the two primary, fully-functioning controller codes that drive the V3 architecture:
* **`ESP32-C3 Code`:** Handles the IoT pipeline, wireless communication (Blynk IoT), and Edge AI integrations for live monitoring and speed adjustments.
* **`Arduino Uno Code`:** Manages the core kinematics, physical actuation (PID control), and precise motor coordination of the bionic arm.

## Key Achievements
- **IEEE YESIST12 2026:** Selected as a **Finalist** for the Grand Finale in Jakarta, Indonesia.
- **Genius Forum 2025 (ملتقى العباقرة):** Awarded **1st Place** overall.
- **Genius Forum 2023:** Awarded **2nd Place** (Version 2).
- Presented at the 3rd International Conference on Communications at the Air Defense College, Alexandria.

##  Technical Specifications & Tuning
- **Sensor Activation Timeout:** Calibrated strictly to **15 seconds** to optimize response and power consumption.
- **Servo Motor Runtime:** Tuned to **2200** for precise angular positioning and stable holding torque.
- **AI Signal Processing:** Embedded Edge AI analysis to filter raw signals from the custom EMG and Armband Force sensors (Note: Signal analysis was performed purely on raw filtering and classification algorithms; **Z-score normalization was not utilized in this pipeline**).

##  References, Credits & Custom Upgrades
This project evolved over three major iterations:
* **EMG Sensor Implementation:** Base schematic adapted from [CharlesLabs OpenEMG](https://charleslabs.fr/en/project-OpenEMG+Arduino+Sensor). **Custom Upgrades:** Developed a custom hardware analog filter using the **LM358 Low-Power Dual Op-Amp**, coupled with a software-level **Moving Average Digital Filter**.
* **V1 Design:** Structural framework based on [Project Three by mdesigns](https://mdesigns.space/projects/project-three-2jhsa). **Custom Upgrades:** Integrated Bluetooth and a mobile app for wireless control.
* **V2 Design:** Fully custom mechanical and electronic redesign.
* **V3 Design (Current):** Structural foundation inspired by compliant hand research by *Akhtar A. et al. (https://www.researchgate.net/publication/309332233_A_Low-Cost_Open-Source_Compliant_Hand_for_Enabling_Sensorimotor_Control_for_People_with_Transradial_Amputations)*. **Custom Upgrades:** Re-engineered utilizing **Carbon Fiber**. Upgraded to implement **Multi-Sensor Fusion** and processes signals via **Edge AI** instead of the original single EMG setup.

##  Future Work & TODOs (Incomplete/WIP)
- [x] Initial Firmware and Docs push.
- [ ] Upload full Hardware schematics (Altium/KiCad) and 3D printing models.
- [ ] Upload Simulation files (MATLAB/Proteus).
- [ ] Finalize the PCB layout for the power management and Step-Up circuit.
