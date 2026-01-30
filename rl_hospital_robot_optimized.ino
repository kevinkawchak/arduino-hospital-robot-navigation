/*
 * ============================================================================
 * REINFORCEMENT LEARNING HOSPITAL ROBOT - MEMORY OPTIMIZED
 * Infinity Track with Q-Learning for ATmega32U4 (2.5KB RAM)
 * ============================================================================
 * 
 * PARADIGM: LLM-to-Real (Large Language Model to Real Hardware)
 * 
 * INFINITY TRACK HOSPITAL MAPPING:
 * 
 *         LEFT LOOP                    RIGHT LOOP
 *       (Patient Wing)              (Services Wing)
 *      
 *          [PT2]                       [RAD]
 *           ___                         ___
 *          /   \                       /   \
 *   [PT1] /     \                     /     \ [PHRM]
 *         \      \                   /      /
 *          \______\____[NS]________/______/
 *                   (CENTER X)
 *                       |
 *                    [START]
 * 
 * Segments: 0=Start, 1=Corridor, 2=PT1, 3=PT2, 4=Corridor, 
 *           5=NurseStation, 6=Corridor, 7=Pharmacy, 8=Radiology, 9=Corridor
 * 
 * ============================================================================
 */

#include <Zumo32U4.h>

// Hardware (no Wire.h needed - Zumo32U4.h handles it)
Zumo32U4Motors motors;
Zumo32U4Buzzer buzzer;
Zumo32U4LCD lcd;
Zumo32U4LineSensors lineSensors;
Zumo32U4Encoders encoders;
Zumo32U4ButtonA buttonA;

// Line sensors
#define NUM_SENSORS 5
uint16_t sensorVals[NUM_SENSORS];

// ==================== Q-LEARNING (Memory Optimized) ====================
// Reduced state space: 8 line states x 10 segments = 80 states
#define NUM_LINE_STATES 8    // Simplified: left/center/right + lost
#define NUM_SEGMENTS 10
#define NUM_ACTIONS 5
#define Q_TABLE_SIZE 80      // Reduced - only store visited states

// Q-table: 80 states x 5 actions = 400 bytes (using int8_t)
int8_t Q[Q_TABLE_SIZE][NUM_ACTIONS];

// Action speeds [left, right]
const int8_t ACTIONS[NUM_ACTIONS][2] PROGMEM = {
  {-80, 80},    // 0: Hard Left
  {40, 100},    // 1: Soft Left  
  {100, 100},   // 2: Straight
  {100, 40},    // 3: Soft Right
  {80, -80}     // 4: Hard Right
};

// Learning parameters (scaled integers to save memory)
uint8_t alpha = 38;      // 0.15 * 255
uint8_t gamma_d = 242;   // 0.95 * 255
uint8_t epsilon = 77;    // 0.30 * 255
const uint8_t EPS_MIN = 13;   // 0.05 * 255
const uint8_t EPS_DECAY = 253; // 0.995 * 255

// ==================== POSITION TRACKING ====================
// Segment distances (encoder ticks) - stored in PROGMEM
const uint16_t SEG_DIST[NUM_SEGMENTS] PROGMEM = {
  0, 600, 1400, 2200, 3000, 3600, 4400, 5200, 6000, 6800
};

// ==================== STATE VARIABLES ====================
uint8_t curSegment = 0;
uint8_t curLineState = 0;
uint8_t curAction = 2;
uint16_t totalDist = 0;
int16_t episodeReward = 0;
int16_t bestReward = -1000;
uint8_t episode = 0;
uint8_t tasksComplete = 0;
uint8_t taskFlags = 0;  // Bit flags for 5 tasks
bool running = false;
bool calibrated = false;
uint8_t crossings = 0;
unsigned long startTime = 0;

// Metrics
uint16_t centered = 0;
uint16_t readings = 0;

// ==================== LOCATION NAMES IN PROGMEM ====================
const char L0[] PROGMEM = "START";
const char L1[] PROGMEM = "CORR1";
const char L2[] PROGMEM = "PT_RM1";
const char L3[] PROGMEM = "PT_RM2";
const char L4[] PROGMEM = "CORR2";
const char L5[] PROGMEM = "NURSE";
const char L6[] PROGMEM = "CORR3";
const char L7[] PROGMEM = "PHARM";
const char L8[] PROGMEM = "RADIO";
const char L9[] PROGMEM = "CORR4";

const char* const LOC_NAMES[] PROGMEM = {L0,L1,L2,L3,L4,L5,L6,L7,L8,L9};

// Task locations: PT1=2, PT2=3, NURSE=5, PHARM=7, RAD=8
const uint8_t TASK_LOCS[] PROGMEM = {5, 2, 3, 7, 8};

// ==================== SETUP ====================
void setup() {
  Serial.begin(9600);
  lineSensors.initFiveSensors();
  
  // Initialize Q-table to zero
  memset(Q, 0, sizeof(Q));
  
  lcd.clear();
  lcd.print(F("RL HOSP"));
  lcd.gotoXY(0, 1);
  lcd.print(F("ROBOT"));
  
  Serial.println(F("=== RL HOSPITAL ROBOT ==="));
  Serial.println(F("Paradigm: LLM-to-Real"));
  Serial.println(F("Press A to calibrate"));
  
  buttonA.waitForButton();
  calibrate();
  
  Serial.println(F("Press A to start"));
  buttonA.waitForButton();
  startEpisode();
}

// ==================== MAIN LOOP ====================
void loop() {
  if (!running) return;
  
  // 1. Read state
  readSensors();
  updatePosition();
  uint8_t state = encodeState();
  
  // 2. Select action (epsilon-greedy)
  curAction = selectAction(state);
  
  // 3. Execute action
  int8_t lSpd = pgm_read_byte(&ACTIONS[curAction][0]);
  int8_t rSpd = pgm_read_byte(&ACTIONS[curAction][1]);
  motors.setSpeeds(lSpd, rSpd);
  
  // 4. Calculate reward
  int8_t reward = calcReward();
  episodeReward += reward;
  
  // 5. Update Q-table
  uint8_t nextState = encodeState();
  updateQ(state, curAction, reward, nextState);
  
  // 6. Check waypoints
  checkWaypoints();
  
  // 7. Output metrics (every 500ms)
  static unsigned long lastOut = 0;
  if (millis() - lastOut > 500) {
    outputMetrics();
    lastOut = millis();
  }
  
  // 8. Check episode end
  if (checkEnd()) {
    endEpisode();
  }
  
  delay(20);
}

// ==================== CALIBRATION ====================
void calibrate() {
  Serial.println(F("[CAL] Sweep over line..."));
  lcd.clear();
  lcd.print(F("CALIBR8"));
  buzzer.playFrequency(440, 150, 10);
  
  for (uint8_t i = 0; i < 80; i++) {
    motors.setSpeeds(i < 40 ? 80 : -80, i < 40 ? -80 : 80);
    lineSensors.calibrate();
    delay(20);
  }
  motors.setSpeeds(0, 0);
  
  calibrated = true;
  buzzer.playFrequency(880, 150, 10);
  Serial.println(F("[CAL] Done!"));
}

// ==================== SENSOR READING ====================
void readSensors() {
  lineSensors.readCalibrated(sensorVals);
  
  // Simplified line state (0-7)
  // 0=lost, 1=far left, 2=left, 3=slight left, 
  // 4=center, 5=slight right, 6=right, 7=far right
  
  uint8_t bits = 0;
  for (uint8_t i = 0; i < NUM_SENSORS; i++) {
    if (sensorVals[i] > 500) bits |= (1 << (4 - i));
  }
  
  // Map 5-bit to 3-bit state
  if (bits == 0) curLineState = 0;           // Lost
  else if (bits & 0b00100) curLineState = 4; // Center
  else if (bits & 0b01000) curLineState = 3; // Slight left
  else if (bits & 0b00010) curLineState = 5; // Slight right
  else if (bits & 0b10000) curLineState = 2; // Left
  else if (bits & 0b00001) curLineState = 6; // Right
  else if (bits == 0b11111) {
    curLineState = 4;  // Crossing
    if (millis() - startTime > 2000) crossings++;
  }
  else curLineState = 4;
  
  readings++;
  if (curLineState >= 3 && curLineState <= 5) centered++;
}

// ==================== POSITION TRACKING ====================
void updatePosition() {
  int16_t l = encoders.getCountsAndResetLeft();
  int16_t r = encoders.getCountsAndResetRight();
  totalDist += (abs(l) + abs(r)) / 2;
  
  // Determine segment
  for (int8_t i = NUM_SEGMENTS - 1; i >= 0; i--) {
    if (totalDist >= pgm_read_word(&SEG_DIST[i])) {
      if (curSegment != i) {
        curSegment = i;
        // Print location name from PROGMEM
        char buf[8];
        strcpy_P(buf, (char*)pgm_read_ptr(&LOC_NAMES[i]));
        Serial.print(F("[SEG] "));
        Serial.println(buf);
        lcd.clear();
        lcd.print(buf);
      }
      break;
    }
  }
}

// ==================== Q-LEARNING ====================
uint8_t encodeState() {
  return curSegment * NUM_LINE_STATES + curLineState;
}

uint8_t selectAction(uint8_t state) {
  // Epsilon-greedy
  if (random(255) < epsilon) {
    return random(NUM_ACTIONS);  // Explore
  }
  
  // Exploit: find best action
  int8_t bestVal = -128;
  uint8_t bestAct = 2;
  for (uint8_t a = 0; a < NUM_ACTIONS; a++) {
    if (Q[state][a] > bestVal) {
      bestVal = Q[state][a];
      bestAct = a;
    }
  }
  return bestAct;
}

void updateQ(uint8_t s, uint8_t a, int8_t r, uint8_t s2) {
  // Find max Q(s',a')
  int8_t maxNext = -128;
  for (uint8_t i = 0; i < NUM_ACTIONS; i++) {
    if (Q[s2][i] > maxNext) maxNext = Q[s2][i];
  }
  
  // Q-learning update (integer math)
  int16_t target = r + ((int16_t)gamma_d * maxNext / 255);
  int16_t delta = target - Q[s][a];
  int16_t update = (int16_t)alpha * delta / 255;
  
  int16_t newQ = Q[s][a] + update;
  if (newQ > 127) newQ = 127;
  if (newQ < -128) newQ = -128;
  Q[s][a] = (int8_t)newQ;
}

int8_t calcReward() {
  int8_t r = 0;
  
  // Line following
  if (curLineState == 4) r += 3;        // Centered
  else if (curLineState == 3 || curLineState == 5) r += 1;  // Slight off
  else if (curLineState == 0) r -= 10;  // Lost
  else r -= 1;  // Off center
  
  // Crossing bonus
  if (curLineState == 4 && curSegment == 5) r += 5;
  
  // Time penalty
  r -= 1;
  
  return r;
}

// ==================== WAYPOINTS ====================
void checkWaypoints() {
  for (uint8_t i = 0; i < 5; i++) {
    uint8_t loc = pgm_read_byte(&TASK_LOCS[i]);
    if (curSegment == loc && !(taskFlags & (1 << i))) {
      taskFlags |= (1 << i);
      tasksComplete++;
      episodeReward += 30;
      
      Serial.print(F("[TASK] Complete #"));
      Serial.println(i + 1);
      
      buzzer.playFrequency(880, 100, 10);
      
      lcd.gotoXY(0, 1);
      lcd.print(F("TASK "));
      lcd.print(tasksComplete);
      lcd.print(F("/5"));
    }
  }
}

// ==================== EPISODE MANAGEMENT ====================
void startEpisode() {
  episode++;
  episodeReward = 0;
  totalDist = 0;
  curSegment = 0;
  tasksComplete = 0;
  taskFlags = 0;
  crossings = 0;
  centered = 0;
  readings = 0;
  startTime = millis();
  
  encoders.getCountsAndResetLeft();
  encoders.getCountsAndResetRight();
  
  running = true;
  
  Serial.println();
  Serial.print(F("=== EPISODE "));
  Serial.print(episode);
  Serial.println(F(" ==="));
  
  lcd.clear();
  lcd.print(F("EP "));
  lcd.print(episode);
  
  buzzer.playFrequency(523, 100, 10);
}

bool checkEnd() {
  // Time limit 45s
  if (millis() - startTime > 45000) {
    Serial.println(F("[END] Timeout"));
    return true;
  }
  
  // All tasks + returned
  if (tasksComplete >= 5 && curSegment >= 9) {
    Serial.println(F("[END] Complete!"));
    return true;
  }
  
  // Lost too long
  static uint16_t lostCount = 0;
  if (curLineState == 0) {
    lostCount++;
    if (lostCount > 100) {
      Serial.println(F("[END] Lost"));
      return true;
    }
  } else {
    lostCount = 0;
  }
  
  return false;
}

void endEpisode() {
  motors.setSpeeds(0, 0);
  running = false;
  
  // Decay epsilon
  epsilon = (uint16_t)epsilon * EPS_DECAY / 255;
  if (epsilon < EPS_MIN) epsilon = EPS_MIN;
  
  // Update best
  if (episodeReward > bestReward) bestReward = episodeReward;
  
  // Summary
  uint8_t lfa = readings > 0 ? (uint16_t)centered * 100 / readings : 0;
  uint8_t tcr = tasksComplete * 20;  // *100/5
  
  Serial.println();
  Serial.println(F("=== EPISODE SUMMARY ==="));
  Serial.print(F("Tasks: "));
  Serial.print(tasksComplete);
  Serial.println(F("/5"));
  Serial.print(F("TCR: "));
  Serial.print(tcr);
  Serial.println(F("%"));
  Serial.print(F("LFA: "));
  Serial.print(lfa);
  Serial.println(F("%"));
  Serial.print(F("Reward: "));
  Serial.println(episodeReward);
  Serial.print(F("Best: "));
  Serial.println(bestReward);
  Serial.print(F("Epsilon: "));
  Serial.println(epsilon * 100 / 255);
  Serial.println(F("======================="));
  
  lcd.clear();
  lcd.print(F("TCR:"));
  lcd.print(tcr);
  lcd.print(F("%"));
  lcd.gotoXY(0, 1);
  lcd.print(F("R:"));
  lcd.print(episodeReward);
  
  buzzer.playFrequency(784, 200, 10);
  delay(150);
  buzzer.playFrequency(1047, 300, 10);
  
  Serial.println(F("Press A for next EP"));
  buttonA.waitForButton();
  startEpisode();
}

// ==================== METRICS OUTPUT ====================
void outputMetrics() {
  uint8_t lfa = readings > 0 ? (uint16_t)centered * 100 / readings : 0;
  uint8_t tcr = tasksComplete * 20;
  
  // Format for video overlay parsing
  Serial.print(F("@OVR|LOC:"));
  char buf[8];
  strcpy_P(buf, (char*)pgm_read_ptr(&LOC_NAMES[curSegment]));
  Serial.print(buf);
  Serial.print(F("|TCR:"));
  Serial.print(tcr);
  Serial.print(F("%|LFA:"));
  Serial.print(lfa);
  Serial.print(F("%|R:"));
  Serial.print(episodeReward);
  Serial.print(F("/"));
  Serial.print(bestReward);
  Serial.print(F("|EPS:"));
  Serial.print(epsilon * 100 / 255);
  Serial.print(F("%|T:"));
  Serial.print(tasksComplete);
  Serial.println(F("/5"));
}
