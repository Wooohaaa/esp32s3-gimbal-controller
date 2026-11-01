#include <Arduino.h>
#include <math.h>
#include "CoordinateFrameParser.h"
#include "PidController.h"

namespace {
constexpr int kImageWidth = 320;
constexpr int kImageHeight = 240;
constexpr int kPanPin = 5;
constexpr int kTiltPin = 4;
constexpr int kUartRxPin = 17;
constexpr int kUartTxPin = 18;
constexpr uint32_t kUartBaud = 115200;
constexpr uint32_t kServoFrequency = 50;
constexpr uint8_t kServoResolution = 14;
constexpr uint32_t kServoPulseMinUs = 500;
constexpr uint32_t kServoPulseMaxUs = 2500;
constexpr float kPanMin = 0.0f;
constexpr float kPanMax = 180.0f;
constexpr float kTiltMin = 0.0f;
constexpr float kTiltMax = 185.0f;
constexpr bool kPanPositiveDirection = true;
constexpr bool kTiltPositiveDirection = false;
constexpr float kPanKp = 0.025f;
constexpr float kPanKi = 0.0f;
constexpr float kPanKd = 0.06f;
constexpr float kTiltKp = 0.025f;
constexpr float kTiltKi = 0.0f;
constexpr float kTiltKd = 0.06f;
constexpr uint32_t kControlPeriodMs = 50;
constexpr uint32_t kTargetLostTimeoutMs = 1500;
constexpr float kDeadbandPixels = 8.0f;
constexpr float kMaxAngleStep = 2.5f;
constexpr float kHomeStep = 1.0f;

CoordinateFrameParser frameParser;
PidController panController;
PidController tiltController;
float targetX = kImageWidth / 2.0f;
float targetY = kImageHeight / 2.0f;
float panAngle = 90.0f;
float tiltAngle = 90.0f;
uint32_t lastValidTargetMs = 0;
uint32_t lastControlMs = 0;
uint32_t lastStatusMs = 0;

uint32_t pulseToDuty(float angle) {
    const float bounded = constrain(angle, 0.0f, 185.0f);
    const float pulseUs = kServoPulseMinUs +
        (bounded / 180.0f) * (kServoPulseMaxUs - kServoPulseMinUs);
    return static_cast<uint32_t>(pulseUs * kServoFrequency *
                                 (1U << kServoResolution) / 1000000.0f);
}

void writeServos() {
    ledcWrite(0, pulseToDuty(panAngle));
    ledcWrite(1, pulseToDuty(tiltAngle));
}

void centerServos() {
    panAngle = 90.0f;
    tiltAngle = 90.0f;
    panController.reset();
    tiltController.reset();
    writeServos();
}

void acceptFrame(uint8_t x, uint8_t y) {
    if (x == 255 && y == 255) {
        centerServos();
        lastValidTargetMs = millis();
        Serial.println("Center command received");
        return;
    }
    if (x == 0) {
        targetX = kImageWidth / 2.0f;
        targetY = kImageHeight / 2.0f;
        return;
    }
    targetX = static_cast<float>(x);
    targetY = static_cast<float>(y);
    lastValidTargetMs = millis();
}

float boundedStep(float value) {
    return constrain(value, -kMaxAngleStep, kMaxAngleStep);
}

void updateControl(uint32_t now) {
    if (now - lastControlMs < kControlPeriodMs) return;
    lastControlMs = now;

    float panStep = 0.0f;
    float tiltStep = 0.0f;
    if (fabsf(targetX - kImageWidth / 2.0f) >= kDeadbandPixels)
        panStep = boundedStep(panController.update(targetX));
    else
        panController.reset();
    if (fabsf(targetY - kImageHeight / 2.0f) >= kDeadbandPixels)
        tiltStep = boundedStep(tiltController.update(targetY));
    else
        tiltController.reset();

    if (!kPanPositiveDirection) panStep = -panStep;
    if (!kTiltPositiveDirection) tiltStep = -tiltStep;
    panAngle = constrain(panAngle + panStep, kPanMin, kPanMax);
    tiltAngle = constrain(tiltAngle + tiltStep, kTiltMin, kTiltMax);
    writeServos();

    if (now - lastValidTargetMs > kTargetLostTimeoutMs) {
        targetX = kImageWidth / 2.0f;
        targetY = kImageHeight / 2.0f;
        panController.reset();
        tiltController.reset();
        if (panAngle > 90.0f) panAngle = fmaxf(90.0f, panAngle - kHomeStep);
        else if (panAngle < 90.0f) panAngle = fminf(90.0f, panAngle + kHomeStep);
        if (tiltAngle > 90.0f) tiltAngle = fmaxf(90.0f, tiltAngle - kHomeStep);
        else if (tiltAngle < 90.0f) tiltAngle = fminf(90.0f, tiltAngle + kHomeStep);
        writeServos();
    }

    if (now - lastStatusMs >= 500) {
        Serial.printf("PAN=%.1f TILT=%.1f target=(%.0f,%.0f)\n",
                      panAngle, tiltAngle, targetX, targetY);
        lastStatusMs = now;
    }
}
}  // namespace

void setup() {
    Serial.begin(115200);
    ledcSetup(0, kServoFrequency, kServoResolution);
    ledcAttachPin(kPanPin, 0);
    ledcSetup(1, kServoFrequency, kServoResolution);
    ledcAttachPin(kTiltPin, 1);
    writeServos();

    Serial1.begin(kUartBaud, SERIAL_8N1, kUartRxPin, kUartTxPin);
    panController.configure(kPanKp, kPanKi, kPanKd, kImageWidth / 2.0f);
    tiltController.configure(kTiltKp, kTiltKi, kTiltKd, kImageHeight / 2.0f);
    lastValidTargetMs = millis();
    lastControlMs = millis();
    Serial.println("ESP32S3 gimbal controller ready");
}

void loop() {
    while (Serial1.available() > 0) {
        const uint8_t value = static_cast<uint8_t>(Serial1.read());
        if (frameParser.push(value)) acceptFrame(frameParser.x(), frameParser.y());
    }
    updateControl(millis());
    delay(1);
}
