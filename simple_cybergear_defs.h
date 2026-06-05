#ifndef CYBERGEAR_DEFS_H
#define CYBERGEAR_DEFS_H

#include <Arduino.h>

enum CyberGearCommands : uint8_t {
    CMD_POSITION                  = 0x01,
    CMD_ENABLE                    = 0x03,
    CMD_STOP                      = 0x04,
    CMD_SET_CAN_ID                = 0x07,
    CMD_GET_STATUS                = 0x15,
    CMD_SET_MECH_POSITION_TO_ZERO = 0x06,
    CMD_RAM_READ                  = 0x11,
    CMD_RAM_WRITE                 = 0x12,
    // CMD_GET_PARAM_LIST            = 0x13
};

enum CyberGearAddresses : uint16_t {
    ADDR_RUN_MODE             = 0x7005,
    ADDR_POSITION_REF         = 0x7016,
    ADDR_POSITION_KP          = 0x701E,
    ADDR_CURRENT_REF          = 0x7006,
    ADDR_CURRENT_KP           = 0x7010,
    ADDR_CURRENT_KI           = 0x7011,
    ADDR_CURRENT_FILTER_GAIN  = 0x7014,
    ADDR_SPEED_REF            = 0x700A,
    ADDR_SPEED_KP             = 0x701F,
    ADDR_SPEED_KI             = 0x7020,
    ADDR_LIMIT_CURRENT        = 0x7018,
    ADDR_LIMIT_SPEED          = 0x7017,
    ADDR_LIMIT_TORQUE         = 0x700B
};

const float POS_MIN = -12.5f;
const float POS_MAX =  12.5f;

const float V_MIN   = -30.0f;
const float V_MAX   =  30.0f;

const float T_MIN   = -12.0f;
const float T_MAX   =  12.0f;

const float KP_MIN  =  0.0f;
const float KP_MAX  =  500.0f;

const float KD_MIN  =  0.0f;
const float KD_MAX  =  5.0f;

#endif
