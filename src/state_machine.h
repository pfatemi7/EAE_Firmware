#pragma once
#include <string>

enum class State { OFF, READY, RUN, FAULT };

struct Inputs {
    bool ignition, level_ok, sensor_valid;
    double temp_c;
};

struct Outputs {
    bool pump_enable, alarm;
    int fan_pwm;
    State state;
    std::string why;
};

class StateMachine {
    State state = State::OFF;
public:
    Outputs process_inputs(const Inputs& in, int fan_pwm);
};
