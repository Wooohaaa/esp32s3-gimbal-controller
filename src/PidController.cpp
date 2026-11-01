#include "PidController.h"

PidController::PidController()
    : kp_(0.0f), ki_(0.0f), kd_(0.0f), setpoint_(0.0f),
      integral_(0.0f), previousError_(0.0f) {}

void PidController::configure(float kp, float ki, float kd, float setpoint) {
    kp_ = kp;
    ki_ = ki;
    kd_ = kd;
    setpoint_ = setpoint;
    reset();
}

void PidController::reset() {
    integral_ = 0.0f;
    previousError_ = 0.0f;
}

float PidController::update(float measurement) {
    const float error = measurement - setpoint_;
    integral_ += error;
    if (integral_ > 100000.0f) integral_ = 100000.0f;
    if (integral_ < -100000.0f) integral_ = -100000.0f;
    const float derivative = error - previousError_;
    previousError_ = error;
    return kp_ * error + ki_ * integral_ + kd_ * derivative;
}
