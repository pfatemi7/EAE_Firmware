#pragma once

struct PID {
    double kp, ki, kd;
    double setpoint;
    double integral;
    double prev_error;
    
    PID(double p = 2.0, double i = 0.1, double d = 0.0) 
        : kp(p), ki(i), kd(d), setpoint(55.0), integral(0.0), prev_error(0.0) {}
    
    double calculate_output(double temp, double dt = 0.2);
    void set_setpoint(double sp) { setpoint = sp; }
    void reset() { integral = 0.0; prev_error = 0.0; }
};
