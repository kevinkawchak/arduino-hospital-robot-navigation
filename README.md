# Arduino Hospital Robot Navigation

A demonstration of autonomous hospital delivery robot navigation using a Zumo32U4 robot platform accessed remotely through LabsLand's remote laboratory infrastructure.

![Hospital Robot Demo](assets/demo_screenshot.png)

## 🏥 Project Overview

This project simulates a hospital delivery/assistance robot that autonomously navigates through a hospital environment, visiting multiple stations to pick up and deliver medical supplies. The robot demonstrates careful, measured movements appropriate for healthcare settings with audio feedback at each station.

## 🌍 Remote Lab Access Details

| Parameter | Value |
|-----------|-------|
| **Platform** | LabsLand Remote Laboratories |
| **Robot Location** | San Diego, California, USA |
| **Robot Model** | Pololu Zumo32U4 |
| **Access URL** | `chemicalqd.labsland.com` |
| **IDE URL** | `arduino-robot-code.ide.labsland.com` |
| **Lab Provider** | ChemicalQDevice |

## 🤖 Robot Specifications

- **Platform**: Pololu Zumo32U4
- **Microcontroller**: ATmega32U4
- **Motors**: Dual DC motors with encoder feedback
- **Display**: 8x2 character LCD
- **Sensors**: Proximity sensors, line sensors, IMU
- **Audio**: Piezo buzzer for audio feedback

## 🏨 Hospital Route Simulation

The robot visits the following stations in sequence:

```
┌─────────────────┐
│  NURSE STATION  │ ◄─── Start/End Point
│   (Pickup)      │
└────────┬────────┘
         │
         ▼
┌─────────────────┐
│ PATIENT ROOM 1  │ ◄─── Medication Delivery
│  (Delivery)     │
└────────┬────────┘
         │
         ▼
┌─────────────────┐
│ PATIENT ROOM 2  │ ◄─── Supplies Delivery
│  (Delivery)     │
└────────┬────────┘
         │
         ▼
┌─────────────────┐
│    PHARMACY     │ ◄─── Prescription Pickup
│   (Pickup)      │
└────────┬────────┘
         │
         ▼
┌─────────────────┐
│  NURSE STATION  │ ◄─── Return & Complete
│   (Return)      │
└─────────────────┘
```

## ⚙️ Key Features for Hospital Robotics

### 1. Patient Safety Speed Control
```cpp
const int HOSPITAL_SPEED = 120;    // Reduced speed for patient safety
const int CAREFUL_SPEED = 80;      // Extra careful near patient rooms
const int TURN_SPEED = 100;        // Gentle turning movements
```
Standard robot speeds are significantly reduced to ensure safe operation around patients, staff, and medical equipment.

### 2. Smooth Acceleration/Deceleration
```cpp
void smoothAccelerate(int targetSpeed) {
  for (int speed = 0; speed <= targetSpeed; speed += 20) {
    motors.setSpeeds(speed, speed);
    delay(50);
  }
}
```
Prevents sudden jerky movements that could startle patients or spill medications.

### 3. Station-Based State Machine
The robot uses a finite state machine architecture for reliable, predictable navigation:
- `NURSE_STATION` - Initial pickup point
- `TO_PATIENT_1` / `AT_PATIENT_1` - First delivery
- `TO_PATIENT_2` / `AT_PATIENT_2` - Second delivery  
- `TO_PHARMACY` / `AT_PHARMACY` - Prescription pickup
- `RETURN_TO_NURSE` - Safe return path
- `MISSION_COMPLETE` - End state

### 4. Audio Feedback System
Different tones for different events enable staff awareness:

| Tone Type | Function | Frequency |
|-----------|----------|-----------|
| Startup Melody | Robot initializing | C-E-G (523-659-784 Hz) |
| Moving Tone | Robot in motion alert | 440 Hz |
| Arrival Tone | Reached destination | 698-880 Hz |
| Pickup Confirmation | Items loaded | Triple beep |
| Delivery Confirmation | Items delivered | Double 880 Hz |
| Mission Complete | All tasks done | C-E-G-C (ascending) |

### 5. LCD Status Display
Real-time status visible on robot's 8x2 LCD screen for at-a-glance monitoring by hospital staff.

## ⏱️ Timing Configuration

| Constant | Value (ms) | Purpose |
|----------|------------|---------|
| `CORRIDOR_TIME` | 1200 | Travel time for main corridors |
| `ROOM_APPROACH` | 800 | Careful approach to patient rooms |
| `STATION_PAUSE` | 2000 | Dwell time at pickup stations |
| `DELIVERY_TIME` | 1500 | Pause for item collection |
| `TURN_90` | 450 | 90-degree turn duration |
| `TURN_45` | 225 | 45-degree turn duration |

> **Note**: These values are calibrated for the LabsLand arena. Adjust based on your specific environment dimensions.

## 📋 Console Output Log

```
========================================
  HOSPITAL DELIVERY ROBOT ACTIVATED
========================================

Beginning hospital rounds...

[NURSE STATION] Picking up deliveries...
[NURSE STATION] Deliveries loaded. Proceeding to Patient Room 1.

[NAVIGATING] En route to Patient Room 1...
[NAVIGATING] Arrived at Patient Room 1.
[PATIENT ROOM 1] Delivering medication...
[PATIENT ROOM 1] Please collect your items.
[PATIENT ROOM 1] Delivery complete. Proceeding to Patient Room 2.

[NAVIGATING] En route to Patient Room 2...
[NAVIGATING] Arrived at Patient Room 2.
[PATIENT ROOM 2] Delivering supplies...
[PATIENT ROOM 2] Please collect your items.
[PATIENT ROOM 2] Delivery complete. Proceeding to Pharmacy.

[NAVIGATING] En route to Pharmacy...
[NAVIGATING] Arrived at Pharmacy.
[PHARMACY] Picking up prescriptions...
[PHARMACY] Prescriptions loaded. Returning to Nurse Station.

[NAVIGATING] Returning to Nurse Station...
[NAVIGATING] Arrived at Nurse Station.

========================================
  HOSPITAL ROUNDS COMPLETE
  All deliveries successful!
========================================
```

## 🚀 Getting Started

### Using LabsLand Remote Lab

1. Navigate to the LabsLand Arduino Robot lab at `chemicalqd.labsland.com`
2. Click **"Access"** to join the queue
3. Once connected, paste the code into the Arduino IDE
4. Click **"Verify / Compile"** to check for errors
5. Click **"Upload to robot"** to run the demonstration
6. Watch via the live camera feed

### Local Development (Zumo32U4 Required)

1. Install [Arduino IDE](https://www.arduino.cc/en/software)
2. Install the Zumo32U4 library via Library Manager
3. Connect your Zumo32U4 via USB
4. Upload the sketch
5. Place robot at starting position

## 📁 Repository Structure

```
arduino-hospital-robot-navigation/
├── README.md
├── hospital_robot_navigation.ino
├── assets/
│   ├── demo_screenshot.png
│   └── demo_video.mp4
├── docs/
│   └── timing_calibration.md
└── .gitignore
```

## 🔧 Customization

### Adjusting for Different Arena Sizes

Modify the timing constants in the code header to match your environment:

```cpp
const int CORRIDOR_TIME = 1200;   // Increase for longer corridors
const int ROOM_APPROACH = 800;    // Adjust for room spacing
```

### Adding More Stations

Extend the `HospitalState` enum and add corresponding handler functions:

```cpp
enum HospitalState {
  // ... existing states ...
  TO_RADIOLOGY,
  AT_RADIOLOGY,
  TO_ICU,
  AT_ICU,
  // ... etc
};
```

### Modifying Speed Profiles

For different hospital environments:
```cpp
// High-traffic areas (slower)
const int HOSPITAL_SPEED = 80;

// Low-traffic corridors (faster)  
const int HOSPITAL_SPEED = 150;
```

## 🔬 Technical Implementation Notes

### State Machine Architecture
The navigation uses an enumerated state machine that ensures:
- Predictable, repeatable behavior
- Easy debugging via state tracking
- Simple extension for additional stations
- Clear separation between navigation and action states

### Motion Smoothing
Acceleration ramping prevents:
- Medication spillage
- Patient startling
- Mechanical stress on motors
- Wheel slippage on smooth hospital floors

### Audio Design Considerations
- Frequencies chosen to be audible but not alarming
- Volume levels appropriate for hospital environment
- Distinct tones for different events prevent confusion

## 📚 References

- [Pololu Zumo32U4 Documentation](https://www.pololu.com/docs/0J63)
- [LabsLand Remote Laboratories](https://labsland.com)
- [Arduino Reference](https://www.arduino.cc/reference/en/)

## 📄 License

This project is released under the MIT License. See [LICENSE](LICENSE) for details.

## 👤 Author

Created for hospital robotics research and education demonstrations.

---

*This demonstration was conducted using LabsLand's remote laboratory infrastructure, enabling global access to physical robotics hardware for education and research purposes.*
