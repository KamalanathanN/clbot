#include "Arduino.h"
#include "IMU.h"

IMU::IMU() {
//    mpu.setup(0x68);
//    mpu.begin(Adafruit_BNO055::OPERATION_MODE_COMPASS);
//    mpu.setExtCrystalUse(true);
//    mpu.calibrateAccelGyro();
//    delay(2000);
//    mpu.calibrateMag();
}

/*
| Author: Juan Huerta and Daniel Hunter
| Return: tuple conataining the quatw and quatz
| Remark: Reads absolute orientation(quaternion) values and returns w
|   and z components
*/
std::tuple<std_msgs::Float32, std_msgs::Float32> IMU::read_IMUmsg_data() {

    //imu::Quaternion quat = mpu.getQuat();
    std_msgs::Float32 quatw, quatz;
    quatw.data = (float)mpu.getQuaternionW();
    quatz.data = (float)mpu.getQuaternionZ();
    
    return std::make_tuple(quatw, quatz);
}

/*
| Author: Juan Huerta and Kevin Beher
| Return: Void
| Remark: Reads magnetometer value and converts it to degrees with respect to North
*/
void IMU::read_compass() {
    //sensor_msgs::Compass Compass_msg;
    //imu::Vector<3> magnetometer = mpu.getVector(Adafruit_BNO055::VECTOR_MAGNETOMETER);
    //imu::Vector<3> magnetometer = mpu.getMag();

    float Pi = 3.14159;
    //float magnetic_heading = (atan2(magnetometer.y(), magnetometer.x()) * 180) / Pi;
    float magnetic_heading = (atan2(mpu.getMagY(), mpu.getMagX()) * 180) / Pi;
 
    if (magnetic_heading < 0) {
        magnetic_heading = 360 + magnetic_heading;
    }

    Serial.println("Compass ");
    Serial.print(magnetic_heading);
}
