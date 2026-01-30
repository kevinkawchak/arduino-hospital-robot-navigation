/*
 * ============================================================
 * SCENARIO B: Hospital Robot Navigation Course
 * ============================================================
 * 
 * DESCRIPTION:
 * This code simulates a hospital delivery/assistance robot that
 * must navigate through a hospital environment. The robot visits
 * multiple "stations" representing:
 *   - Nurse Station (pickup point)
 *   - Patient Room 1
 *   - Patient Room 2
 *   - Pharmacy
 *   - Return to Nurse Station
 * 
 * The robot demonstrates careful, measured movements appropriate
 * for a healthcare environment with pauses at each station.
 * 
 * EXPECTED ROBOT BEHAVIOR:
 * 1. Starts at "Nurse Station" - plays greeting tone
 * 2. Navigates slowly and carefully to Patient Room 1
 * 3. Pauses at Patient Room 1 (simulating delivery)
 * 4. Proceeds to Patient Room 2 with careful navigation
 * 5. Pauses at Patient Room 2 (simulating delivery)
 * 6. Navigates to Pharmacy pickup
 * 7. Returns to Nurse Station via safe route
 * 8. All movements are slower and more deliberate than standard
 * 9. Audio announcements at each station
 * 
 * HOW TO USE IN LABSLAND:
 * 1. Click the orange "Access" button to reserve the robot
 * 2. Wait for your turn in the queue (shows wait time)
 * 3. Once connected, you'll see the live camera feed
 * 4. Copy this entire code into the Arduino IDE text editor
 * 5. Click "Upload" or "Compile & Run" button
 * 6. Watch the robot perform its hospital rounds
 * 7. Note: You may need to adjust timing values based on arena size
 * 
 * HOSPITAL SAFETY FEATURES:
 * - Slower speeds for patient safety
 * - Audible alerts when moving
 * - Pauses at each station for "delivery"
 * - Smooth acceleration/deceleration
 * 
 * ============================================================
 */

#include <Wire.h>
#include <Zumo32U4.h>

// Initialize robot components
Zumo32U4Motors motors;
Zumo32U4Buzzer buzzer;
Zumo32U4LCD lcd;
Zumo32U4ProximitySensors proxSensors;

// Hospital robot moves slower for safety
const int HOSPITAL_SPEED = 120;      // Slower for patient safety
const int CAREFUL_SPEED = 80;        // Extra careful near rooms
const int TURN_SPEED = 100;          // Gentle turns

// Timing constants
const int CORRIDOR_TIME = 1200;      // Time to travel corridor
const int ROOM_APPROACH = 800;       // Approach to room
const int STATION_PAUSE = 2000;      // Pause at each station
const int DELIVERY_TIME = 1500;      // Simulate delivery action
const int TURN_90 = 450;             // 90-degree turn time
const int TURN_45 = 225;             // 45-degree turn time

// State machine
enum HospitalState {
  NURSE_STATION,
  TO_PATIENT_1,
  AT_PATIENT_1,
  TO_PATIENT_2,
  AT_PATIENT_2,
  TO_PHARMACY,
  AT_PHARMACY,
  RETURN_TO_NURSE,
  MISSION_COMPLETE
};

HospitalState currentState = NURSE_STATION;

void setup() {
  Serial.begin(9600);
  
  // Initialize proximity sensors
  proxSensors.initThreeSensors();
  
  // Display startup
  lcd.clear();
  lcd.print("HOSPITAL");
  lcd.gotoXY(0, 1);
  lcd.print("ROBOT");
  
  Serial.println("========================================");
  Serial.println("  HOSPITAL DELIVERY ROBOT ACTIVATED");
  Serial.println("========================================");
  
  // Play hospital robot startup melody
  playStartupMelody();
  
  delay(1000);
  
  Serial.println("Beginning hospital rounds...");
  Serial.println("");
}

void loop() {
  switch(currentState) {
    case NURSE_STATION:
      handleNurseStation();
      break;
      
    case TO_PATIENT_1:
      navigateToPatient1();
      break;
      
    case AT_PATIENT_1:
      handlePatientRoom1();
      break;
      
    case TO_PATIENT_2:
      navigateToPatient2();
      break;
      
    case AT_PATIENT_2:
      handlePatientRoom2();
      break;
      
    case TO_PHARMACY:
      navigateToPharmacy();
      break;
      
    case AT_PHARMACY:
      handlePharmacy();
      break;
      
    case RETURN_TO_NURSE:
      returnToNurseStation();
      break;
      
    case MISSION_COMPLETE:
      missionComplete();
      break;
  }
}

// ============ STATE HANDLERS ============

void handleNurseStation() {
  Serial.println("[NURSE STATION] Picking up deliveries...");
  lcd.clear();
  lcd.print("NURSE");
  lcd.gotoXY(0, 1);
  lcd.print("STATION");
  
  // Simulate pickup
  playPickupTone();
  delay(STATION_PAUSE);
  
  Serial.println("[NURSE STATION] Deliveries loaded. Proceeding to Patient Room 1.");
  Serial.println("");
  
  currentState = TO_PATIENT_1;
}

void navigateToPatient1() {
  lcd.clear();
  lcd.print("-> PT 1");
  Serial.println("[NAVIGATING] En route to Patient Room 1...");
  
  // Navigate through main corridor
  playMovingTone();
  smoothAccelerate(HOSPITAL_SPEED);
  delay(CORRIDOR_TIME);
  
  // Turn into patient wing
  smoothDecelerate();
  delay(200);
  turnLeftSmooth(TURN_90);
  
  // Approach patient room carefully
  lcd.clear();
  lcd.print("APPROACH");
  lcd.gotoXY(0, 1);
  lcd.print("PT RM 1");
  
  smoothAccelerate(CAREFUL_SPEED);
  delay(ROOM_APPROACH);
  smoothDecelerate();
  
  Serial.println("[NAVIGATING] Arrived at Patient Room 1.");
  currentState = AT_PATIENT_1;
}

void handlePatientRoom1() {
  Serial.println("[PATIENT ROOM 1] Delivering medication...");
  lcd.clear();
  lcd.print("DELIVERY");
  lcd.gotoXY(0, 1);
  lcd.print("ROOM 1");
  
  // Play arrival notification
  playArrivalTone();
  delay(500);
  
  // Simulate delivery
  Serial.println("[PATIENT ROOM 1] Please collect your items.");
  delay(DELIVERY_TIME);
  
  // Confirmation tone
  playConfirmTone();
  Serial.println("[PATIENT ROOM 1] Delivery complete. Proceeding to Patient Room 2.");
  Serial.println("");
  
  delay(500);
  currentState = TO_PATIENT_2;
}

void navigateToPatient2() {
  lcd.clear();
  lcd.print("-> PT 2");
  Serial.println("[NAVIGATING] En route to Patient Room 2...");
  
  // Back out and continue down hall
  playMovingTone();
  
  // Turn to continue down patient wing
  turnRightSmooth(TURN_90);
  delay(200);
  
  // Travel down corridor
  smoothAccelerate(HOSPITAL_SPEED);
  delay(CORRIDOR_TIME);
  smoothDecelerate();
  
  // Turn into room 2
  turnLeftSmooth(TURN_90);
  delay(200);
  
  // Careful approach
  lcd.clear();
  lcd.print("APPROACH");
  lcd.gotoXY(0, 1);
  lcd.print("PT RM 2");
  
  smoothAccelerate(CAREFUL_SPEED);
  delay(ROOM_APPROACH);
  smoothDecelerate();
  
  Serial.println("[NAVIGATING] Arrived at Patient Room 2.");
  currentState = AT_PATIENT_2;
}

void handlePatientRoom2() {
  Serial.println("[PATIENT ROOM 2] Delivering supplies...");
  lcd.clear();
  lcd.print("DELIVERY");
  lcd.gotoXY(0, 1);
  lcd.print("ROOM 2");
  
  playArrivalTone();
  delay(500);
  
  Serial.println("[PATIENT ROOM 2] Please collect your items.");
  delay(DELIVERY_TIME);
  
  playConfirmTone();
  Serial.println("[PATIENT ROOM 2] Delivery complete. Proceeding to Pharmacy.");
  Serial.println("");
  
  delay(500);
  currentState = TO_PHARMACY;
}

void navigateToPharmacy() {
  lcd.clear();
  lcd.print("-> PHARM");
  Serial.println("[NAVIGATING] En route to Pharmacy...");
  
  playMovingTone();
  
  // Navigate back to main corridor
  turnRightSmooth(TURN_90);
  delay(200);
  
  smoothAccelerate(HOSPITAL_SPEED);
  delay(CORRIDOR_TIME);
  smoothDecelerate();
  
  // Turn toward pharmacy
  turnRightSmooth(TURN_90);
  delay(200);
  
  // Proceed to pharmacy
  smoothAccelerate(HOSPITAL_SPEED);
  delay(CORRIDOR_TIME * 0.8);
  smoothDecelerate();
  
  Serial.println("[NAVIGATING] Arrived at Pharmacy.");
  currentState = AT_PHARMACY;
}

void handlePharmacy() {
  Serial.println("[PHARMACY] Picking up prescriptions...");
  lcd.clear();
  lcd.print("PHARMACY");
  lcd.gotoXY(0, 1);
  lcd.print("PICKUP");
  
  playArrivalTone();
  delay(STATION_PAUSE);
  
  playPickupTone();
  Serial.println("[PHARMACY] Prescriptions loaded. Returning to Nurse Station.");
  Serial.println("");
  
  delay(500);
  currentState = RETURN_TO_NURSE;
}

void returnToNurseStation() {
  lcd.clear();
  lcd.print("RETURN");
  lcd.gotoXY(0, 1);
  lcd.print("NURSE ST");
  Serial.println("[NAVIGATING] Returning to Nurse Station...");
  
  playMovingTone();
  
  // Turn around
  turnRightSmooth(TURN_90 * 2);  // 180 degree turn
  delay(200);
  
  // Navigate back through corridor
  smoothAccelerate(HOSPITAL_SPEED);
  delay(CORRIDOR_TIME * 1.5);
  smoothDecelerate();
  
  // Final turn to nurse station
  turnLeftSmooth(TURN_90);
  delay(200);
  
  // Final approach
  smoothAccelerate(CAREFUL_SPEED);
  delay(ROOM_APPROACH);
  smoothDecelerate();
  
  Serial.println("[NAVIGATING] Arrived at Nurse Station.");
  currentState = MISSION_COMPLETE;
}

void missionComplete() {
  Serial.println("");
  Serial.println("========================================");
  Serial.println("  HOSPITAL ROUNDS COMPLETE");
  Serial.println("  All deliveries successful!");
  Serial.println("========================================");
  
  lcd.clear();
  lcd.print("MISSION");
  lcd.gotoXY(0, 1);
  lcd.print("COMPLETE");
  
  playCompleteMelody();
  
  // Stop and idle
  stopMotors();
  
  // Prevent repeat
  while(true) {
    delay(1000);
  }
}

// ============ SMOOTH MOVEMENT FUNCTIONS ============

void smoothAccelerate(int targetSpeed) {
  for (int speed = 0; speed <= targetSpeed; speed += 20) {
    motors.setSpeeds(speed, speed);
    delay(50);
  }
  motors.setSpeeds(targetSpeed, targetSpeed);
}

void smoothDecelerate() {
  for (int speed = HOSPITAL_SPEED; speed >= 0; speed -= 20) {
    motors.setSpeeds(speed, speed);
    delay(50);
  }
  stopMotors();
}

void turnLeftSmooth(int duration) {
  motors.setSpeeds(-TURN_SPEED, TURN_SPEED);
  delay(duration);
  stopMotors();
  delay(200);
}

void turnRightSmooth(int duration) {
  motors.setSpeeds(TURN_SPEED, -TURN_SPEED);
  delay(duration);
  stopMotors();
  delay(200);
}

void stopMotors() {
  motors.setSpeeds(0, 0);
}

// ============ AUDIO FEEDBACK FUNCTIONS ============

void playStartupMelody() {
  // Friendly hospital robot startup
  buzzer.playFrequency(523, 150, 12);  // C
  delay(200);
  buzzer.playFrequency(659, 150, 12);  // E
  delay(200);
  buzzer.playFrequency(784, 300, 12);  // G
  delay(400);
}

void playMovingTone() {
  // Soft beep to alert people robot is moving
  buzzer.playFrequency(440, 100, 8);
}

void playArrivalTone() {
  // Two-tone arrival notification
  buzzer.playFrequency(698, 150, 10);
  delay(200);
  buzzer.playFrequency(880, 200, 10);
}

void playPickupTone() {
  // Pickup confirmation
  buzzer.playFrequency(523, 100, 10);
  delay(150);
  buzzer.playFrequency(659, 100, 10);
  delay(150);
  buzzer.playFrequency(523, 100, 10);
}

void playConfirmTone() {
  // Delivery confirmed
  buzzer.playFrequency(880, 150, 10);
  delay(100);
  buzzer.playFrequency(880, 150, 10);
}

void playCompleteMelody() {
  // Mission complete celebration
  buzzer.playFrequency(523, 150, 12);
  delay(200);
  buzzer.playFrequency(659, 150, 12);
  delay(200);
  buzzer.playFrequency(784, 150, 12);
  delay(200);
  buzzer.playFrequency(1047, 400, 12);
  delay(500);
}
