/*
 * Rover Motor & Servo Controller
 *
 * Receives serial commands from Raspberry Pi Zero W and drives:
 *   - 2 x JGA25-370 motors via DRV8833 dual H-bridge
 *   - 3 x GDW DS041MG servos (front, middle, rear axle steering)
 *
 * Serial protocol: 9600 baud, newline-terminated text commands
 *   FWD — forward, BCK — backward, LFT — left, RGT — right, STP — stop
 *
 * Steering geometry: front and rear servos counter-rotate for tighter turns,
 * middle servo stays centered.
 */

#include <Servo.h>

// --- Pin assignments (all PWM-capable on Mega 2560) ---
// DRV8833 channel A — left motor
const int MOTOR_A_IN1 = 2;
const int MOTOR_A_IN2 = 3;
// DRV8833 channel B — right motor
const int MOTOR_B_IN1 = 4;
const int MOTOR_B_IN2 = 5;
// Servo signal pins
const int SERVO_FRONT_PIN = 9;
const int SERVO_MID_PIN   = 10;
const int SERVO_REAR_PIN  = 11;

// --- Tuning constants ---
const int MOTOR_SPEED  = 200;  // PWM duty cycle (0–255)
const int SERVO_CENTER = 90;   // Center position in degrees
const int STEER_ANGLE  = 30;   // Degrees offset from center for turning

// --- Servo objects ---
Servo servoFront;
Servo servoMid;
Servo servoRear;

// --- Motor helpers ---

void motorForward(int speed) {
  analogWrite(MOTOR_A_IN1, speed);
  analogWrite(MOTOR_A_IN2, 0);
  analogWrite(MOTOR_B_IN1, speed);
  analogWrite(MOTOR_B_IN2, 0);
}

void motorBackward(int speed) {
  analogWrite(MOTOR_A_IN1, 0);
  analogWrite(MOTOR_A_IN2, speed);
  analogWrite(MOTOR_B_IN1, 0);
  analogWrite(MOTOR_B_IN2, speed);
}

void motorStop() {
  // Brake mode: both pins HIGH
  analogWrite(MOTOR_A_IN1, 255);
  analogWrite(MOTOR_A_IN2, 255);
  analogWrite(MOTOR_B_IN1, 255);
  analogWrite(MOTOR_B_IN2, 255);
}

// --- Servo helpers ---

void steerCenter() {
  servoFront.write(SERVO_CENTER);
  servoMid.write(SERVO_CENTER);
  servoRear.write(SERVO_CENTER);
}

void steerLeft() {
  servoFront.write(SERVO_CENTER + STEER_ANGLE);  // Front turns left
  servoMid.write(SERVO_CENTER);                   // Middle stays centered
  servoRear.write(SERVO_CENTER - STEER_ANGLE);    // Rear counter-rotates
}

void steerRight() {
  servoFront.write(SERVO_CENTER - STEER_ANGLE);   // Front turns right
  servoMid.write(SERVO_CENTER);                    // Middle stays centered
  servoRear.write(SERVO_CENTER + STEER_ANGLE);     // Rear counter-rotates
}

// --- Arduino entry points ---

void setup() {
  Serial.begin(9600);

  // Motor pins as output
  pinMode(MOTOR_A_IN1, OUTPUT);
  pinMode(MOTOR_A_IN2, OUTPUT);
  pinMode(MOTOR_B_IN1, OUTPUT);
  pinMode(MOTOR_B_IN2, OUTPUT);

  // Attach servos
  servoFront.attach(SERVO_FRONT_PIN);
  servoMid.attach(SERVO_MID_PIN);
  servoRear.attach(SERVO_REAR_PIN);

  // Initialize to stopped/centered
  motorStop();
  steerCenter();
}

void loop() {
  if (!Serial.available()) return;

  String cmd = Serial.readStringUntil('\n');
  cmd.trim();
  if (cmd.length() == 0) return;

  if (cmd == "FWD") {
    motorForward(MOTOR_SPEED);
    steerCenter();
  } else if (cmd == "BCK") {
    motorBackward(MOTOR_SPEED);
    steerCenter();
  } else if (cmd == "LFT") {
    motorForward(MOTOR_SPEED);
    steerLeft();
  } else if (cmd == "RGT") {
    motorForward(MOTOR_SPEED);
    steerRight();
  } else if (cmd == "STP") {
    motorStop();
    steerCenter();
  }
}
