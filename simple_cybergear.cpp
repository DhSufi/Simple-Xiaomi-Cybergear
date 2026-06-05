#include "simple_cybergear.h"
#include <cstring>


TwaiManager::TwaiManager() : _is_initialized(false) {}

bool TwaiManager::begin(uint8_t tx_pin, uint8_t rx_pin) {
    twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT((gpio_num_t)tx_pin, (gpio_num_t)rx_pin, TWAI_MODE_NORMAL);
    g_config.rx_queue_len = 20; 
    twai_timing_config_t t_config = TWAI_TIMING_CONFIG_1MBITS(); 
    twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

    if (twai_driver_install(&g_config, &t_config, &f_config) != ESP_OK) return false;
    if (twai_start() != ESP_OK) return false;

    uint32_t alerts_to_enable = TWAI_ALERT_TX_FAILED |
                                TWAI_ALERT_BUS_ERROR |
                                TWAI_ALERT_BUS_OFF;

    if (twai_reconfigure_alerts(alerts_to_enable, NULL) != ESP_OK) return false;

    _is_initialized = true;
    return true;
}

bool TwaiManager::transmitFrame(uint8_t motor_id, uint8_t cmd_id, uint16_t master_id, uint8_t (&data)[8]) {
    if (!_is_initialized) return false;

    twai_message_t msg;

    msg.identifier = ((uint32_t)cmd_id << 24) | ((uint32_t)master_id << 8) | motor_id;
    msg.extd = 1;
    msg.rtr = 0; // important. Multiple calls without specifing RTR can pick garbage bit and set RTR to 1, so the frame is not transmited
    msg.data_length_code = 8;
    msg.self = 0; // important. Set to 0 so when calling twai_receive, so it does not store the transmited frame in the receive buffer. (Do not listen to yourself)

    for (int i = 0; i < 8; i++) {
        msg.data[i] = data[i];
    }

    return (twai_transmit(&msg, pdMS_TO_TICKS(10)) == ESP_OK);
}

bool TwaiManager::receiveFrame(twai_message_t &rx_msg) {
    if (!_is_initialized) return false;

    if (twai_receive(&rx_msg, pdMS_TO_TICKS(0)) == ESP_OK) {
        return true; 
    }

    uint32_t alerts_triggered = 0;
    esp_err_t err = twai_read_alerts(&alerts_triggered, pdMS_TO_TICKS(0));
    // if (err == ESP_OK && alerts_triggered != 0) {
    //     if (alerts_triggered & TWAI_ALERT_BUS_OFF) {
    //         Serial.println("Critical Error: CAN bus Bus-Off.");
    //     }
    //     if (alerts_triggered & TWAI_ALERT_TX_FAILED) {
    //         Serial.println("Error: No ACK or transmission failed.");
    //     }
    //     if (alerts_triggered & TWAI_ALERT_BUS_ERROR) {
    //         Serial.println("Error: EMI noise or missing 120-ohm resistor.");
    //     }
    // }
    return false;
}



CyberGearMotor::CyberGearMotor(TwaiManager &twai, uint8_t motor_can_id, uint8_t master_can_id) 
    : _twai(&twai), _motor_id(motor_can_id), _master_id(master_can_id) {}

bool CyberGearMotor::enable() {
    if (_twai == nullptr) return false;

    uint8_t data[8] = {0x00};
    return _twai->transmitFrame(_motor_id, CMD_ENABLE, _master_id, data);
}

bool CyberGearMotor::stop() {
    if (_twai == nullptr) return false;

    uint8_t data[8] = {0x00};
    return _twai->transmitFrame(_motor_id, CMD_STOP, _master_id, data);
}

bool CyberGearMotor::setRunMode(uint8_t mode) {
    if (_twai == nullptr) return false;
    
    uint8_t data[8] = {
        static_cast<uint8_t>(ADDR_RUN_MODE & 0xFF),
        static_cast<uint8_t>(ADDR_RUN_MODE >> 8), 
        0x00, 0x00, 
        mode, 
        0x00, 0x00, 0x00
    };

    return _twai->transmitFrame(_motor_id, CMD_RAM_WRITE, _master_id, data);
}

bool CyberGearMotor::setCanId(uint8_t new_can_id) {
    if (_twai == nullptr) return false;

    uint8_t data[8] = {0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

    uint16_t master_and_id = ((uint16_t)new_can_id << 8) | _master_id;

    if (_twai->transmitFrame(_motor_id, CMD_SET_CAN_ID, master_and_id, data)) {
        delay(5); 
        
        _motor_id = new_can_id;
        return true;
    }
    return false;
}

bool CyberGearMotor::setZeroPos() {
    if (_twai == nullptr) return false;

    uint8_t data[8] = {0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    return _twai->transmitFrame(_motor_id, CMD_SET_MECH_POSITION_TO_ZERO, _master_id, data);
}

bool CyberGearMotor::setCurrentLimit(float current) {
    return _writeFloatParam(ADDR_LIMIT_CURRENT, current);
}

bool CyberGearMotor::setTorqueLimit(float torque) {
    return _writeFloatParam(ADDR_LIMIT_TORQUE, torque);
}

bool CyberGearMotor::setSpeedLimit(float speed) {
    return _writeFloatParam(ADDR_LIMIT_SPEED, speed);
}

bool CyberGearMotor::setCurrentKp(float kp) {
    return _writeFloatParam(ADDR_CURRENT_KP, kp);
}
    
bool CyberGearMotor::setCurrentKi(float ki) {
    return _writeFloatParam(ADDR_CURRENT_KI, ki);
}

bool CyberGearMotor::setFilterGain(float gain) {
    return _writeFloatParam(ADDR_CURRENT_FILTER_GAIN, gain);
}

bool CyberGearMotor::setCurrentRef(float current) {
    return _writeFloatParam(ADDR_CURRENT_REF, current);
}

bool CyberGearMotor::setPositionKp(float kp) {
    return _writeFloatParam(ADDR_POSITION_KP, kp);
}

bool CyberGearMotor::setPositionRef(float position) {
    return _writeFloatParam(ADDR_POSITION_REF, position);
}

bool CyberGearMotor::setSpeedKp(float kp) {
    return _writeFloatParam(ADDR_SPEED_KP, kp);
}

bool CyberGearMotor::setSpeedKi(float ki) {
    return _writeFloatParam(ADDR_SPEED_KI, ki);
}

bool CyberGearMotor::setSpeedRef(float speed) {
    return _writeFloatParam(ADDR_SPEED_REF, speed);
}

bool CyberGearMotor::setMITRef(float target_position, float target_speed, float target_torque, float kp, float kd) {

    uint16_t pos_int = _floatToUint(target_position, POS_MIN, POS_MAX);
    uint16_t spe_int = _floatToUint(target_speed, V_MIN, V_MAX);
    uint16_t trq_int = _floatToUint(target_torque, T_MIN, T_MAX);
    uint16_t kp_int = _floatToUint(kp, KP_MIN, KP_MAX);
    uint16_t kd_int = _floatToUint(kd, KD_MIN, KD_MAX);
    
    uint8_t data[8];
    data[0] = static_cast<uint8_t>(pos_int >> 8);
    data[1] = static_cast<uint8_t>(pos_int & 0xFF);
    data[2] = static_cast<uint8_t>(spe_int >> 8);
    data[3] = static_cast<uint8_t>(spe_int & 0xFF);
    data[4] = static_cast<uint8_t>(kp_int >> 8);
    data[5] = static_cast<uint8_t>(kp_int & 0xFF);
    data[6] = static_cast<uint8_t>(kd_int >> 8);
    data[7] = static_cast<uint8_t>(kd_int & 0xFF);

    return _twai->transmitFrame(_motor_id, CMD_POSITION, trq_int, data);
}

bool CyberGearMotor::_writeFloatParam(uint16_t addr, float value) {
    if (_twai == nullptr) return false;

    uint8_t data[8] = {
        static_cast<uint8_t>(addr & 0xFF),
        static_cast<uint8_t>(addr >> 8),
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00 
    };
    memcpy(&data[4], &value, sizeof(float));

    return _twai->transmitFrame(_motor_id, CMD_RAM_WRITE, _master_id, data);
}

bool CyberGearMotor::_readFloatParam(uint16_t addr) {
    if (_twai == nullptr) return false;

    uint8_t data[8] = {
        static_cast<uint8_t>(addr & 0xFF),
        static_cast<uint8_t>(addr >> 8),
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };

    return _twai->transmitFrame(_motor_id, CMD_RAM_READ, _master_id, data);
}

uint16_t CyberGearMotor::_floatToUint(float x, float x_min, float x_max) const {
    float span = x_max - x_min;

    if (span <= 0.0f) return 0; 
    float inv_span = 1.0f / span;
    
    x = (x > x_max) ? x_max : ((x < x_min) ? x_min : x);

    return static_cast<uint16_t>((x - x_min) * 65535.0f * inv_span);
}

float CyberGearMotor::_uintToFloat(uint16_t x, float x_min, float x_max) const {
    float span = x_max - x_min;
    return x_min + (static_cast<float>(x) * span / 65535.0f);
}

bool CyberGearMotor::callStatus() {
    if (_twai == nullptr) return false;

    uint8_t data[8] = {0x00};
    return _twai->transmitFrame(_motor_id, CMD_GET_STATUS, _master_id, data);
}

bool CyberGearMotor::callParam(float addr) {
    return _readFloatParam(addr);
}

CyberGearStatus CyberGearMotor::getStatus() {
    CyberGearStatus status = {0, 0, 0, 0, false, false, false, false, false, false, 0.0f, 0.0f, 0.0f, 0, false};

    if (_twai == nullptr) return status;

    // clean rx buffer
    twai_message_t purge_msg;
    while (_twai->receiveFrame(purge_msg)) {}

    uint8_t data[8] = {0x00};
    if(!_twai->transmitFrame(_motor_id, CMD_GET_STATUS, _master_id, data)){
        return status;
    }

    unsigned long startTime = millis();
    const unsigned long timeoutMs = 20;
    twai_message_t rx_msg;

    while ((millis() - startTime) < timeoutMs) {
        if (_twai->receiveFrame(rx_msg)) {

            uint32_t id = rx_msg.identifier;
            status.cmd_id        = (id >> 24) & 0x1F;
            status.motor_id      = (id >> 8) & 0xFF;

            if (status.motor_id == _motor_id && status.cmd_id == 0x02) {

                status.host_id          = id & 0xFF;
                status.motor_mode       = (id >> 22) & 0x03;
                status.calibrated       = (id & (1UL << 21)) != 0;
                status.hall_encoder     = (id & (1UL << 20)) != 0;
                status.magnet_encoder   = (id & (1UL << 19)) != 0;
                status.over_temp        = (id & (1UL << 18)) != 0;
                status.over_curr        = (id & (1UL << 17)) != 0;
                status.under_volt       = (id & (1UL << 16)) != 0;

                uint16_t pos_raw = (rx_msg.data[0] << 8) | rx_msg.data[1];
                uint16_t spd_raw = (rx_msg.data[2] << 8) | rx_msg.data[3];
                uint16_t trq_raw = (rx_msg.data[4] << 8) | rx_msg.data[5];
                uint16_t temp_raw = (rx_msg.data[6] << 8) | rx_msg.data[7];

                status.position = _uintToFloat(pos_raw, POS_MIN, POS_MAX);
                status.speed    = _uintToFloat(spd_raw, V_MIN, V_MAX);
                status.torque   = _uintToFloat(trq_raw, T_MIN, T_MAX);
                status.temp   = temp_raw;

                status.updated = true;
                return status;
            }
        }

        delayMicroseconds(100);
    }
    return status;
}

CyberGearParam CyberGearMotor::getParam(uint16_t addr) {
    //19 communication type, needs host 253 and data the MCU. Returns list of all param names (strings)
    CyberGearParam param = {0, 0, 0, 0, 0, false};

    if (_twai == nullptr) return param;

    // clean rx buffer
    twai_message_t purge_msg;
    while (_twai->receiveFrame(purge_msg)) {}

    uint8_t data[8] = {
        static_cast<uint8_t>(addr),
        static_cast<uint8_t>(addr >> 8),
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };
    if(!_twai->transmitFrame(_motor_id, CMD_RAM_READ, _master_id, data)){
        return param;
    }

    unsigned long startTime = millis();
    const unsigned long timeoutMs = 20;
    twai_message_t rx_msg;

    while ((millis() - startTime) < timeoutMs) {
        if (_twai->receiveFrame(rx_msg)) {

            uint32_t id = rx_msg.identifier;
            param.cmd_id        = (id >> 24) & 0x1F;
            param.motor_id      = (id >> 8) & 0xFF;
            param.index         = rx_msg.data[0] | (rx_msg.data[1] << 8);

            if (param.motor_id == _motor_id && param.cmd_id == 0x11 && param.index == addr) {
                
                param.host_id = id & 0xFF;
                param.value.u32 = *(uint32_t*)&rx_msg.data[4];

                param.updated = true;
                return param;
            }
        }

        delayMicroseconds(100);
    }
    return param;
}
