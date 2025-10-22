# EAE Firmware Simulator

### CAN Bus Simulation
- Thread-safe queue-based implementation
- Loopback functionality for testing
- Frame structure with ID and payload

### PID Controller
- Proportional, Integral, Derivative terms
- Output clamped to 0-100% range
- Configurable gains (kp=2.0, ki=0.1, kd=0.0)

### State Machine
- **OFF**: No ignition
- **READY**: Ignition ON, waiting for temperature
- **RUN**: Normal operation with PID control
- **FAULT**: Low level, sensor fault, or overtemperature
