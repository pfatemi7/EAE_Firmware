#include "state_machine.h"

Outputs StateMachine::process_inputs(const Inputs& in, int fan_pwm) {
    Outputs out;
    out.state = state;
    out.pump_enable = out.alarm = false;
    out.fan_pwm = 0;
    out.why = "";
    
    switch (state) {
        case State::OFF:
            if (in.ignition) {
                state = State::READY;
                out.state = State::READY;
                out.why = "Ignition ON";
            } else {
                out.why = "No ignition";
            }
            break;
            
        case State::READY:
            if (!in.ignition) {
                state = State::OFF;
                out.state = State::OFF;
                out.why = "Ignition OFF";
            } else if (!in.level_ok) {
                state = State::FAULT;
                out.state = State::FAULT;
                out.alarm = true;
                out.why = "Low coolant level";
            } else if (!in.sensor_valid) {
                state = State::FAULT;
                out.state = State::FAULT;
                out.fan_pwm = 100;
                out.alarm = true;
                out.why = "Sensor fault";
            } else if (in.temp_c >= 80.0) {
                state = State::RUN;
                out.state = State::RUN;
                out.pump_enable = true;
                out.fan_pwm = fan_pwm;
                out.why = "Temperature control active";
            } else {
                out.pump_enable = true;
                out.why = "Ready, waiting for temperature";
            }
            break;
            
        case State::RUN:
            if (!in.ignition) {
                state = State::OFF;
                out.state = State::OFF;
                out.why = "Ignition OFF";
            } else if (!in.level_ok) {
                state = State::FAULT;
                out.state = State::FAULT;
                out.alarm = true;
                out.why = "Low coolant level";
            } else if (!in.sensor_valid) {
                state = State::FAULT;
                out.state = State::FAULT;
                out.fan_pwm = 100;
                out.alarm = true;
                out.why = "Sensor fault";
            } else if (in.temp_c >= 90.0) {
                state = State::FAULT;
                out.state = State::FAULT;
                out.fan_pwm = 100;
                out.alarm = true;
                out.why = "Overtemperature";
            } else {
                out.pump_enable = true;
                out.fan_pwm = fan_pwm;
                out.why = "Normal operation";
            }
            break;
            
        case State::FAULT:
            if (!in.ignition) {
                state = State::OFF;
                out.state = State::OFF;
                out.why = "Ignition OFF";
            } else if (in.level_ok && in.sensor_valid && in.temp_c < 80.0) {
                state = State::READY;
                out.state = State::READY;
                out.pump_enable = true;
                out.why = "Fault cleared";
            } else {
                if (!in.level_ok) {
                    out.alarm = true;
                    out.why = "Low coolant level";
                } else if (!in.sensor_valid) {
                    out.fan_pwm = 100;
                    out.alarm = true;
                    out.why = "Sensor fault";
                } else if (in.temp_c >= 90.0) {
                    out.fan_pwm = 100;
                    out.alarm = true;
                    out.why = "Overtemperature";
                } else {
                    out.alarm = true;
                    out.why = "Fault condition";
                }
            }
            break;
    }
    
    return out;
}
