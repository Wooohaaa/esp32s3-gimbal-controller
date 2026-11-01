#pragma once

class PidController {
public:
    PidController();
    void configure(float kp, float ki, float kd, float setpoint);
    void reset();
    float update(float measurement);

private:
    float kp_;
    float ki_;
    float kd_;
    float setpoint_;
    float integral_;
    float previousError_;
};
