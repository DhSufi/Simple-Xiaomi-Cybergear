#ifndef SIMPLE_CYBERGEAR_H
#define SIMPLE_CYBERGEAR_H

#include "simple_cybergear_defs.h"
#include <driver/twai.h>

class TwaiManager {
public:
    TwaiManager();
    bool begin(uint8_t tx_pin, uint8_t rx_pin);
    bool transmitFrame(uint8_t motor_id, uint8_t cmd_id, uint16_t master_id, uint8_t (&data)[8]);
    bool receiveFrame(twai_message_t &rx_msg);

private:
    bool _is_initialized;
};

struct CyberGearStatus {
    uint8_t cmd_id;
    uint8_t host_id;
    uint8_t motor_id;
    uint8_t motor_mode;
    bool calibrated;
    bool hall_encoder;
    bool magnet_encoder;
    bool over_temp;
    bool over_curr;
    bool under_volt;
    float position;
    float speed;
    float torque;
    uint16_t temp;
    bool updated;
};

struct CyberGearParam {
    uint8_t cmd_id;
    uint8_t host_id;
    uint8_t motor_id;
    uint16_t index;
    union {
        uint32_t u32;    // for raw data
        int32_t  i32;
        float    f;
        int16_t  i16;
        uint16_t u16;
        uint8_t  u8;
    } value; 
    bool updated;
};

class CyberGearMotor {
public:
    CyberGearMotor(TwaiManager &twai, uint8_t motor_can_id, uint8_t master_can_id);
    CyberGearStatus getStatus();
    CyberGearParam getParam(uint16_t addr);
    bool enable();
    bool stop();
    bool setRunMode(uint8_t mode);
    bool setCanId(uint8_t new_can_id);
    bool setZeroPos();
    bool setCurrentLimit(float current);
    bool setTorqueLimit(float torque);
    bool setSpeedLimit(float speed);
    bool setCurrentKp(float kp);
    bool setCurrentKi(float ki);
    bool setFilterGain(float gain);
    bool setCurrentRef(float current);
    bool setPositionKp(float kp);
    bool setPositionRef(float position);
    bool setSpeedKp(float kp);
    bool setSpeedKi(float ki);
    bool setSpeedRef(float speed);
    bool setMITRef(float target_position, float target_speed, float target_torque, float kp, float kd);
    bool callStatus();
    bool callParam(uint16_t addr);

private:
    TwaiManager *_twai;
    uint8_t _motor_id;
    uint8_t _master_id;

    bool _writeFloatParam(uint16_t addr, float value);
    bool _readFloatParam(uint16_t addr);
    uint16_t _floatToUint(float x, float x_min, float x_max) const;
    float _uintToFloat(uint16_t x, float x_min, float x_max) const;
};

#endif
