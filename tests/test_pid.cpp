#include <gtest/gtest.h>
#include "pid.h"
#include <cmath>

class PIDTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create PID controller with known gains
        pid = PID(2.0, 0.1, 0.0);  // kp=2.0, ki=0.1, kd=0.0
        pid.set_setpoint(50.0);  // Setpoint at 50°C
    }
    
    PID pid;
};

/**
 * Test PID controller tracking toward setpoint
 * Should converge to setpoint over time
 */
TEST_F(PIDTest, TrackingTowardSetpoint) {
    const double setpoint = 50.0;
    const double dt = 0.2;
    
    // Start with temperature well below setpoint
    double temp = 20.0;
    double pwm;
    
    // Run for several steps
    for (int i = 0; i < 50; i++) {
        pwm = pid.calculate_output(temp, dt);
        
        // Temperature should increase toward setpoint
        temp += (pwm / 100.0) * 0.5;  // Simple thermal model
        
        // After some time, should be close to setpoint
        if (i > 30) {
            EXPECT_NEAR(temp, setpoint, 10.0) << "Temperature should converge to setpoint";
        }
    }
    
    // Final temperature should be reasonably close to setpoint
    EXPECT_NEAR(temp, setpoint, 20.0) << "Final temperature should be close to setpoint";
}

/**
 * Test PID output clamping to 0-100% range
 */
TEST_F(PIDTest, OutputClamping) {
    const double dt = 0.2;
    
    // Test with very high error (should be clamped to 100%)
    double pwm_high = pid.calculate_output(0.0, dt);  // 50°C error
    EXPECT_LE(pwm_high, 100.0) << "PWM should be clamped to maximum 100%";
    EXPECT_GE(pwm_high, 0.0) << "PWM should be clamped to minimum 0%";
    
    // Test with very low error (should be clamped to 0%)
    pid.reset();
    double pwm_low = pid.calculate_output(100.0, dt);  // -50°C error
    EXPECT_GE(pwm_low, 0.0) << "PWM should be clamped to minimum 0%";
    EXPECT_LE(pwm_low, 100.0) << "PWM should be clamped to maximum 100%";
}

/**
 * Test PID setpoint changes
 */
TEST_F(PIDTest, SetpointChanges) {
    const double dt = 0.2;
    double temp = 50.0;  // Start at current setpoint
    
    // Change setpoint
    pid.set_setpoint(60.0);
    double pwm = pid.calculate_output(temp, dt);
    
    // Should now have positive error, so PWM should be > 0
    EXPECT_GT(pwm, 0.0) << "PWM should be positive when temperature below new setpoint";
}

/**
 * Test PID reset functionality
 */
TEST_F(PIDTest, ResetFunctionality) {
    const double dt = 0.2;
    
    // Run PID for a while to build up integral
    for (int i = 0; i < 10; i++) {
        pid.calculate_output(30.0, dt);  // Large error
    }
    
    // Reset and check that integral accumulator is cleared
    pid.reset();
    
    // First step after reset should only have proportional term
    double pwm = pid.calculate_output(30.0, dt);
    double expected_pwm = 2.0 * (50.0 - 30.0);  // kp * error
    EXPECT_NEAR(pwm, expected_pwm, 1.0) << "After reset, PWM should be proportional only";
}

/**
 * Test PID with different time steps
 */
TEST_F(PIDTest, DifferentTimeSteps) {
    pid.reset();
    
    // Test with different time steps
    double pwm1 = pid.calculate_output(30.0, 0.1);  // Small time step
    pid.reset();
    double pwm2 = pid.calculate_output(30.0, 0.5);  // Large time step
    
    // With larger time step, integral term should be larger
    EXPECT_GT(pwm2, pwm1) << "Larger time step should result in larger integral contribution";
}
