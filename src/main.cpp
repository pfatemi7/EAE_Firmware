#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <sstream>
#include <chrono>
#include <thread>
#include <cmath>
#include <cstring>

#include "can.h"
#include "pid.h"
#include "state_machine.h"

// Configuration
double target_temp = 55.0;
double max_temp = 90.0;
bool verbose = true;

void parse_args(int argc, char* argv[]) {
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg.find("--set=") == 0) {
            target_temp = std::stod(arg.substr(6));
        } else if (arg.find("--crit=") == 0) {
            max_temp = std::stod(arg.substr(7));
        } else if (arg == "--quiet") {
            verbose = false;
        } else if (arg == "--help" || arg == "-h") {
            std::cout << "EAE Firmware Simulator\nUsage: " << argv[0] << " [options]\n";
            std::cout << "  --set=<temp>    Setpoint (default: 55.0°C)\n";
            std::cout << "  --crit=<temp>   Critical temp (default: 90.0°C)\n";
            std::cout << "  --quiet         Reduce output\n";
            exit(0);
        }
    }
}

double get_temp(double t) {
    return 20.0 + t * 1.8 + 3.0 * std::sin(t * 0.3) + 1.5 * std::sin(t * 2.1) * std::cos(t * 1.3);
}

bool sensor_ok(double t) { return !(t >= 5.0 && t <= 6.0); }
bool level_ok(double t) { return !(t >= 8.0 && t <= 9.0); }

void send_telemetry(CanBusSim& can, double temp, int pwm) {
    std::ostringstream data;
    data << "T=" << std::fixed << std::setprecision(1) << temp << ",PWM=" << pwm;
    can.send(CanFrame(0x200, data.str()));
}

void print_header() {
    if (verbose) std::cout << "time,state,tempC,fanPWM,pump,alarm,reason\n";
}

void log_status(double t, const Outputs& out, double temp) {
    const char* states[] = {"OFF", "READY", "RUN", "FAULT"};
    std::cout << std::fixed << std::setprecision(1) 
              << t << "," << states[static_cast<int>(out.state)] << "," << temp << "," 
              << out.fan_pwm << "," << (out.pump_enable ? "1" : "0") 
              << "," << (out.alarm ? "1" : "0") << "," << out.why << "\n";
}

int main(int argc, char* argv[]) {
    parse_args(argc, argv);
    
    if (verbose) {
        std::cout << "EAE Firmware Simulator Starting...\n";
        std::cout << "Setpoint: " << target_temp << "°C\n";
        std::cout << "Critical: " << max_temp << "°C\n";
        std::cout << "Running simulation...\n\n";
    }
    
    CanBusSim can;
    can.send(CanFrame(0x100, "BOOT"));
    can.loopback();
    
    if (auto frame = can.recv()) {
        if (verbose) {
            std::cout << "[CAN RX] ID=0x" << std::hex << frame->id 
                      << std::dec << " Data=" << frame->payload << "\n";
        }
    }
    
    PID pid(2.0, 0.1, 0.0);
    pid.set_setpoint(target_temp);
    StateMachine sm;
    print_header();
    
    double t = 0.0;
    const double dt = 0.2;
    
    while (t < 30.0) {
        Inputs in;
        in.ignition = true;
        in.level_ok = level_ok(t);
        in.sensor_valid = sensor_ok(t);
        in.temp_c = get_temp(t);
        
        double pwm = pid.calculate_output(in.temp_c, dt);
        int fan_pwm = static_cast<int>(std::round(pwm));
        
        Outputs out = sm.process_inputs(in, fan_pwm);
        
        if (out.state == State::FAULT) {
            if (!in.sensor_valid || in.temp_c >= max_temp) {
                out.fan_pwm = 100;
            }
        }
        
        log_status(t, out, in.temp_c);
        
        if (static_cast<int>(t * 5) % 10 == 0) {
            send_telemetry(can, in.temp_c, out.fan_pwm);
            can.loopback();
            
            while (can.has_frames()) {
                if (auto frame = can.recv()) {
                    if (verbose) {
                        std::cout << "[CAN RX] ID=0x" << std::hex << frame->id 
                                  << std::dec << " Data=" << frame->payload << "\n";
                    }
                }
            }
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        t += dt;
    }
    
    if (verbose) std::cout << "\nSimulation complete.\n";
    
    return 0;
}
