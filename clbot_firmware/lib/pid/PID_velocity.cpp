#include "Arduino.h"
#include "PID_velocity.h"

PID_velocity::PID_velocity(int PWM_PIN,int MOTOR_EN1,int MOTOR_EN2,float Kd,float Kp,float Ki,float ticks_per_m) : motor(PWM_PIN, MOTOR_EN1, MOTOR_EN2){
    pid_error = 0;
    pid_target = 0;
    pid_motor = 0;
    vel = 0;
    pid_integral = 0;
    pid_derivative = 0;
    pid_previous_error = 0;
    wheel_prev = 0;
    wheel_mult = 0;
    prev_encoder = 0;

    then = millis();
    prev_pid_time = millis();
    pid_Kp = Kp;
    pid_Ki = Ki;
    pid_Kd = Kd;
    out_min = -255;
    out_max = 255;
    rate = 30;
    ticks_per_meter = ticks_per_m; //encoder ticks per meter
    velocity_threshold = 0.001;
    encoder_min = -32768;
    encoder_max = 32768;
    encoder_low_wrap = (encoder_max - encoder_min) * 0.3 + encoder_min; // 10k
    encoder_high_wrap = (encoder_max - encoder_min) * 0.7 + encoder_min; // 22k
    wheel_latest = 0.0;   
}

/*
| Author: Juan Huerta and Daniel Hunter
| Return: Void
| Remark: Function that calculates the current wheel velocity
*/
void PID_velocity::calc_velocity() {
    // check how much time has elapsed
    double dt_duration = millis() - then; // In milliseconds
    double dt = dt_duration / 1000; // Convert to seconds
    double cur_vel;
    
    if (wheel_latest == wheel_prev) {
        cur_vel = (1.0 / (double)ticks_per_meter) / dt;
        if (abs(cur_vel) < velocity_threshold) { // too slow
            append_vel(0);
            calc_rolling_vel();
        } else { // moving
            if ((vel >= 0 && vel > cur_vel && cur_vel >= 0) ||
                    (vel < 0 && vel < cur_vel && cur_vel <= 0)) {
                append_vel(cur_vel);
                calc_rolling_vel();
            }
        }
    } else {      
        cur_vel = (wheel_latest - wheel_prev) / dt;
        append_vel(cur_vel);
        calc_rolling_vel();
        wheel_prev = wheel_latest;
        then = millis();
    }    
}

/*
| Author: Daniel Hunter
| Param: current velocity value
| Return: Void
| Remark: Appends current velocity to an array holding previous velocity values.
|     The prev_vel array is modeled as a FILO queue: elements are added at
|     index 0, and fall off the array at index ROLLING_PTS
*/

void PID_velocity::append_vel(double val) {
    for (int i = 1; i < ROLLING_PTS; i++) {
        prev_vel[i] = prev_vel[i - 1];
    }
    prev_vel[0] = val;
}

/*
| Author: Daniel Hunter 
| Return: Void
| Remark: Calculates the average rolling velocity
*/
void PID_velocity::calc_rolling_vel() {
    float sum = 0;
    for (int i = 0; i < ROLLING_PTS; i++) {
        sum += prev_vel[i];
    }
    vel = sum / ROLLING_PTS;
}

/*
| Author: Juan Huerta and Daniel Hunter
| Return: Void
| Remark: calculates pid_motor function 
*/
void PID_velocity::do_pid() {
    unsigned long pid_dt_duration = millis() - prev_pid_time;
    double pid_dt = pid_dt_duration / 1000.0; // Must cast to float, otherwise int division
    prev_pid_time = millis();

    pid_error = pid_target - vel;
    pid_integral = pid_integral + (pid_error * pid_dt); 
    pid_derivative = (pid_error - pid_previous_error) / pid_dt;
    pid_previous_error = pid_error;
    
    pid_motor = (pid_Kp * pid_error) + (pid_Ki * pid_integral) + (pid_Kd * pid_derivative);
    
    if (pid_motor > out_max) { //if calculated PWM is more than max it sets PWM to max
        pid_motor = out_max;
        pid_integral = pid_integral - (pid_error * pid_dt); 
    } else if (pid_motor < out_min) { //if caclulated PWM is less than min sets PWM to min
        pid_motor = out_min;
        pid_integral = pid_integral - (pid_error * pid_dt); 
    } 

    if(pid_target > 0) { //prevents neg output for pos target
      pid_motor = std::max(0.0,pid_motor*1.0);
    } else if(pid_target < 0) { // prevents pos output for neg target
      pid_motor = std::min(pid_motor*1.0, 0.0);
    } else if (pid_target == 0) {
      pid_motor = 0;
    }
}

/*
| Author: Juan Huerta and Daniel Hunter
| Param: enc
| Return: Void
| Remark: Takes current encoder value and updates cumulative encoder value to account
|       variable wrap around.
*/
void PID_velocity::cumulative_enc_val(int enc) {
    if (enc < encoder_low_wrap && prev_encoder > encoder_high_wrap) {
        wheel_mult++;
    }
    if (enc > encoder_high_wrap && prev_encoder < encoder_low_wrap) {
        wheel_mult--;
    }

    wheel_latest = 1.0 * (enc + wheel_mult * (encoder_max - encoder_min)) / ticks_per_meter;
    prev_encoder = enc;
}

/*
| Author: Juan Huerta and Daniel Hunter
| Return: Void
| Remark: Calculates the current velocity and runs the pid. Calls motor_cmd 
|       from motor API to apply PWM calculcated by PID
*/
void PID_velocity::pid_spin() {    
    calc_velocity();
    do_pid();
    motor.motor_cmd(pid_motor);
}

/*
| Author: Juan Huerta
| Param: curr_encoder_val
| Return: Void
| Remark: Calls check_motor_stall from motor API and returns value set to halt_highlevel
*/
int PID_velocity::check_motor_stall(float curr_encoder_val){
  motor.check_motor_stall(curr_encoder_val);
  return motor.halt_highlevel;
}
