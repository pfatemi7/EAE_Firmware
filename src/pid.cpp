#include "pid.h"
#include <algorithm>

double PID::calculate_output(double temp, double dt) {
    double error = setpoint - temp;
    integral += error * dt;
    double derivative = (error - prev_error) / dt;
    
    double output = kp * error + ki * integral + kd * derivative;
    output = std::clamp(output, 0.0, 100.0);
    
    prev_error = error;
    return output;
}
