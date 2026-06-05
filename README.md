# Simple-Xiaomi-Cybergear
A lightweight C++ library to control Xiaomi Cybergear brushless motors using ESP32 TWAI (CAN bus) and Arduino.

Based on [Daniel Kalicki](https://github.com/DanielKalicki/Xiaomi_CyberGear_Arduino) work.

[Xiaomi Instruction Manual (English)](https://github.com/belovictor/cybergear-docs/blob/main/instructionmanual/instructionmanual.md)

## 🛠️ Hardware Setup

Todo
---

## 📥 Installation
1.  Download as .ZIP.
2.  Arduino IDE: **Sketch** -> **Include Library** -> **Add .ZIP Library...**

---

## 💻 Code Example

Basic example of how to configure and control two Xiaomi CyberGear motors using an M5Stack AtomS3 (or any ESP32)

In this example, `motor2` is set to position mode and mechanically follows the movements detected by `motor1`.

```cpp
#include "simple_cybergear.h"

// M5 Stack AtomS3 TWAI pins:
#define RX_PIN G1
#define TX_PIN G2

// CAN ids
uint8_t motor1_CAN_ID = 15; // Xiaomi Recommended between 0 and 127 both inclusive
uint8_t motor2_CAN_ID = 16; // Xiaomi Recommended between 0 and 127 both inclusive
uint8_t MASTER_CAN_ID = 3;  // Recommended > 127. Xiaomi official software uses 253

// Create instance of class TwaiManager
TwaiManager myTwai;

// Create instances of class CyberGearMotor
CyberGearMotor motor1(myTwai, motor1_CAN_ID, MASTER_CAN_ID);
CyberGearMotor motor2(myTwai, motor2_CAN_ID, MASTER_CAN_ID);


void setup() {

  Serial.begin(9600);

  // Init the Twai communication
  if (myTwai.begin(TX_PIN, RX_PIN)) {
    Serial.println("Bus CAN (TWAI) initialized at 1Mbps.");
    // The TwaiManager will always init at 1Mbps as long as is the default Xiaomi Cybergear CAN bus bit rate
    // if you need to change it, manually change the line 10 with your bit rate in the file simple_cybergear.cpp
  } else {
      Serial.println("Critical init CAN Bus");
      while (1) { delay(1000); }
  }

  // In Xiaomi Cybergear instruction manual: to init motor we need to call "stop" and then call "set mode"
  // Let's init motor2 in position mode  
  motor2.stop();
  motor2.setRunMode(1); // 0: MIT mode, 1: Position mode, 2: Speed mode, 3: Current mode.

  // In Xiaomi Cybergear instruction manual: when using motor in Position mode, it is recommended to set the parameter "speed limit"
  // Let's set motor2 speed limit to 30 rad/s
  motor2.setSpeedLimit(30); // Range of speed limit is from 0 rad/s to 30 rad/s.

  // As long as we are using Position mode, we could ask the motor for the current position or jus reset it to zero.
  // Let's reset the position of motor2 to zero.
  motor2.setZeroPos(); // Now the current position of the motor2 is zero.

  // In Xiaomi Cybergear instruction manual: to be able to control motor we need to call "enable"
  motor2.enable();

  // In other hand, let's just stop the motor1, without seting any mode nor parameter.
  motor1.stop();

  // Also reset motor1 position to zero.
  motor1.setZeroPos(); // Now the current position of the motor1 is zero.

}

void loop() {

  // Let's ask the motor1 for its current position

  // We need to create a variable of type "CyberGearParam" to get any parameter asked to the motor.
  CyberGearParam positionMotor1; // In this case the parameter will be current position.

  // In Xiaomi Cybergear instruction manual: the parameter index for current position is "0x7019"
  // Let's use the index for asking the position of motor1 and store the response in the variable "positionMotor1"
  positionMotor1 = motor1.getParam(0x7019); // The getParam() method will block until response, timeout (default 20ms) or error happen.

  // When the response is available, the member "updated" of "positionMotor1" will be true. If timeout or error, the member "updated" will be false.
  // Let's check if response is available
  if (positionMotor1.updated) {
    // In Xiaomi Cybergear instruction manual: depending on the parameter asked to the motor, it could have different variable types: int16, uint8 or float.
    // The parameter 0x7019 is float.
    // The member "value" of "positionMotor1" will contain the response (current position)
    // To read the member "value" as float, we use the member "f"
    float currentPos = positionMotor1.value.f; // member "u8" for uint8 (positionMotor1.value.u8), member "i16" for int16 (positionMotor1.value.i16), member "f" for float (positionMotor1.value.f),

    // Let's print the current position
    Serial.print("MECH POS: "); Serial.println(currentPos);

    // Finally, Let's set the motor1 position to the motor2
    motor2.setPositionRef(currentPos);  // Now the motor2 follows the position of the motor1
  }

  // delay(20); Set delay up to you.

}
```

## ⚙️ USAGE

### Class `TwaiManager`
#### Parameters
- No parameters
#### Methods
##### `begin(uint8_t tx_pin, uint8_t rx_pin)`
Calling this function will start the TWAI driver.
- Returns:
  - `bool` - `True` if TWAI driver started correctly, `False` if any error starting driver.
- Parameters:
  - `tx_pin: uint8_t` - Transmit pin of microcontroller.
  - `rx_pin: uint8_t` - Receive pin of microcontroller.
##### `transmitFrame(uint8_t motor_id, uint8_t cmd_id, uint16_t master_id, uint8_t (&data)[8])`
Calling this function will transmit the CAN Frame.
- Returns:
  - `bool` - `True` if Frame was transmited correctly, else `False`
- Parameters:
  - `motor_id: uint8_t` - The target motor CAN ID
  - `cmd_id: uint8_t` - The Xiaomi Cybergear Protocol Communication Type
  - `master_id: uint8_t` - The Host/Master CAN ID
  - `data[8]: uint8_t` - The CAN Frame Data field. Array of fixed size 8 bytes.
##### `receiveFrame(twai_message_t &rx_msg)`
Calling this function will read the next received CAN frame from the RX buffer queue (note the queue can have multiple frames waiting. This function reads the oldest one).
The received frame is assigned to the *rx_msg* variable

- Returns:
  - `bool` - `True` if a CAN Frame available, else `False`
- Parameters:
  - `rx_msg: twai_message_t` - The [TWAI Struct](https://docs.espressif.com/projects/esp-idf/en/release-v4.3/esp32/api-reference/peripherals/twai.html#_CPPv414twai_message_t) for receive messages
---
### Class `CyberGearMotor`
#### Parameters
  - `twai: TwaiManager` - Object of [TwaiManager](#class-twaimanager)
  - `motor_can_id: uint8_t` - The motor CAN ID
  - `master_can_id: uint8_t` - The Host/Master ID
#### Methods
##### `getStatus()`
Calling this method will send CAN frame to the motor and will wait for the response. This method **BLOCKS** at least 20ms.
- Returns:
  - [`CyberGearStatus`](#struct-cybergearstatus) - Struct that contains the response data in its members.
- Parameters:
  - `No parameters`
##### `callStatus()`
Calling this method will send CAN frame to the motor. This method does **NOT** block. It is neccesary to manually get the response using [receiveFrame](#receiveframetwai_message_t-rx_msg) method from [Class TwaiManager](#class-twaimanager)
- Returns:
  - `bool` - `True` if Frame was transmited correctly, else `False`
- Parameters:
  - `No parameters`
##### `getParam(uint16_t addr)`
Calling this method will send CAN frame to the motor and will wait for the response. This method **BLOCKS** at least 20ms.
- Returns:
  - [`CyberGearParam`](#struct-cybergearparam) - Struct that contains the response data in its members.
- Parameters:
  - `addr: uint16_t` - The index of the parameter to be requested (See Xiaomi Instrucction Manual)
##### `callParam(uint16_t addr)`
Calling this method will send CAN frame to the motor. This method does **NOT** block. It is neccesary to manually get the response using [receiveFrame](#receiveframetwai_message_t-rx_msg) method from [Class TwaiManager](#class-twaimanager)
- Returns:
  - `bool` - `True` if Frame was transmited correctly, else `False`
- Parameters:
  - `addr: uint16_t` - The index of the parameter to be requested (See Xiaomi Instrucction Manual)
##### `enable()`
Calling this method will send *ENABLE* CAN frame to the motor. This method does **NOT** block. If a response is required, it is neccesary to manually get the response using [receiveFrame](#receiveframetwai_message_t-rx_msg) method from [Class TwaiManager](#class-twaimanager)
- Returns:
  - `bool` - `True` if Frame was transmited correctly, else `False`
- Parameters:
  - `No parameters`
##### `stop()`
Calling this method will send *STOP* CAN frame to the motor. This method does **NOT** block. If a response is required, it is neccesary to manually get the response using [receiveFrame](#receiveframetwai_message_t-rx_msg) method from [Class TwaiManager](#class-twaimanager)
- Returns:
  - `bool` - `True` if Frame was transmited correctly, else `False`
- Parameters:
  - `No parameters`
##### `setZeroPos()`
Calling this method will send *RESET MECHANICAL POSITION TO ZERO* CAN frame to the motor. This method does **NOT** block. If a response is required, it is neccesary to manually get the response using [receiveFrame](#receiveframetwai_message_t-rx_msg) method from [Class TwaiManager](#class-twaimanager)
- Returns:
  - `bool` - `True` if Frame was transmited correctly, else `False`
- Parameters:
  - `No parameters`
##### `setRunMode(uint8_t mode)`
Calling this method will send *SET MODE* CAN frame to the motor. This method does **NOT** block. If a response is required, it is neccesary to manually get the response using [receiveFrame](#receiveframetwai_message_t-rx_msg) method from [Class TwaiManager](#class-twaimanager)
- Returns:
  - `bool` - `True` if Frame was transmited correctly, else `False`
- Parameters:
  - `mode: uint8_t` - Mode could be: 0 (MIT Control), 1 (Position mode), 2 (Speed mode), 3 (Electric current mode)
##### `setCurrentLimit(float current)`
Calling this method will send *SET CURRENT LIMIT* CAN frame to the motor. This method does **NOT** block. If a response is required, it is neccesary to manually get the response using [receiveFrame](#receiveframetwai_message_t-rx_msg) method from [Class TwaiManager](#class-twaimanager)
- Returns:
  - `bool` - `True` if Frame was transmited correctly, else `False`
- Parameters:
  - `current: float` - The value desired to be set.
##### `setTorqueLimit(float torque)`
Calling this method will send *SET TORQUE LIMIT* CAN frame to the motor. This method does **NOT** block. If a response is required, it is neccesary to manually get the response using [receiveFrame](#receiveframetwai_message_t-rx_msg) method from [Class TwaiManager](#class-twaimanager)
- Returns:
  - `bool` - `True` if Frame was transmited correctly, else `False`
- Parameters:
  - `torque: float` - The value desired to be set.
##### `setSpeedLimit(float speed)`
Calling this method will send *SET SPEED LIMIT* CAN frame to the motor. This method does **NOT** block. If a response is required, it is neccesary to manually get the response using [receiveFrame](#receiveframetwai_message_t-rx_msg) method from [Class TwaiManager](#class-twaimanager)
- Returns:
  - `bool` - `True` if Frame was transmited correctly, else `False`
- Parameters:
  - `speed: float` - The value desired to be set.
##### `setCurrentKp(float kp)`
Calling this method will send *SET KP FOR CURRENT* CAN frame to the motor. This method does **NOT** block. If a response is required, it is neccesary to manually get the response using [receiveFrame](#receiveframetwai_message_t-rx_msg) method from [Class TwaiManager](#class-twaimanager)
- Returns:
  - `bool` - `True` if Frame was transmited correctly, else `False`
- Parameters:
  - `kp: float` - The value desired to be set.
##### `setCurrentKi(float ki)`
Calling this method will send *SET KI FOR CURRENT* CAN frame to the motor. This method does **NOT** block. If a response is required, it is neccesary to manually get the response using [receiveFrame](#receiveframetwai_message_t-rx_msg) method from [Class TwaiManager](#class-twaimanager)
- Returns:
  - `bool` - `True` if Frame was transmited correctly, else `False`
- Parameters:
  - `ki: float` - The value desired to be set.
##### `setFilterGain(float gain)`
Calling this method will send *SET FILTER COEFICIENT FOR CURRENT* CAN frame to the motor. This method does **NOT** block. If a response is required, it is neccesary to manually get the response using [receiveFrame](#receiveframetwai_message_t-rx_msg) method from [Class TwaiManager](#class-twaimanager)
- Returns:
  - `bool` - `True` if Frame was transmited correctly, else `False`
- Parameters:
  - `gain: float` - The value desired to be set.
##### `setCurrentRef(float current)`
Calling this method will send *SET CURRENT* CAN frame to the motor. This method does **NOT** block. If a response is required, it is neccesary to manually get the response using [receiveFrame](#receiveframetwai_message_t-rx_msg) method from [Class TwaiManager](#class-twaimanager)
- Returns:
  - `bool` - `True` if Frame was transmited correctly, else `False`
- Parameters:
  - `current: float` - The value desired to be set.
##### `setPositionKp(float kp)`
Calling this method will send *SET KP FOR POSITION* CAN frame to the motor. This method does **NOT** block. If a response is required, it is neccesary to manually get the response using [receiveFrame](#receiveframetwai_message_t-rx_msg) method from [Class TwaiManager](#class-twaimanager)
- Returns:
  - `bool` - `True` if Frame was transmited correctly, else `False`
- Parameters:
  - `kp: float` - The value desired to be set.
##### `setPositionRef(float position)`
Calling this method will send *SET POSITION* CAN frame to the motor. This method does **NOT** block. If a response is required, it is neccesary to manually get the response using [receiveFrame](#receiveframetwai_message_t-rx_msg) method from [Class TwaiManager](#class-twaimanager)
- Returns:
  - `bool` - `True` if Frame was transmited correctly, else `False`
- Parameters:
  - `position: float` - The value desired to be set.
##### `setSpeedKp(float kp)`
Calling this method will send *SET KP FOR SPEED* CAN frame to the motor. This method does **NOT** block. If a response is required, it is neccesary to manually get the response using [receiveFrame](#receiveframetwai_message_t-rx_msg) method from [Class TwaiManager](#class-twaimanager)
- Returns:
  - `bool` - `True` if Frame was transmited correctly, else `False`
- Parameters:
  - `kp: float` - The value desired to be set.
##### `setSpeedKi(float ki)`
Calling this method will send *SET KI FOR SPEED* CAN frame to the motor. This method does **NOT** block. If a response is required, it is neccesary to manually get the response using [receiveFrame](#receiveframetwai_message_t-rx_msg) method from [Class TwaiManager](#class-twaimanager)
- Returns:
  - `bool` - `True` if Frame was transmited correctly, else `False`
- Parameters:
  - `ki: float` - The value desired to be set.
##### `setSpeedRef(float speed)`
Calling this method will send *SET SPEED* CAN frame to the motor. This method does **NOT** block. If a response is required, it is neccesary to manually get the response using [receiveFrame](#receiveframetwai_message_t-rx_msg) method from [Class TwaiManager](#class-twaimanager)
- Returns:
  - `bool` - `True` if Frame was transmited correctly, else `False`
- Parameters:
  - `speed: float` - The value desired to be set.
##### `setMITRef(float target_position, float target_speed, float target_torque, float kp, float kd)`
Calling this method will send *SET MIT CONTROL VALUES* CAN frame to the motor. This method does **NOT** block. If a response is required, it is neccesary to manually get the response using [receiveFrame](#receiveframetwai_message_t-rx_msg) method from [Class TwaiManager](#class-twaimanager)
- Returns:
  - `bool` - `True` if Frame was transmited correctly, else `False`
- Parameters:
  - `target_position: float` - The value desired to be set.
  - `target_speed: float` - The value desired to be set.
  - `target_torque: float` - The value desired to be set.
  - `kp: float` - The value desired to be set.
  - `kd: float` - The value desired to be set.
##### `setCanId(uint8_t new_can_id)`
**The use of this method is discouraged** (it is better to use the official software tool for setting up the motors). Calling this method will send *SET CAN ID* CAN frame to the motor. This method does **NOT** block. If a response is required, it is neccesary to manually get the response using [receiveFrame](#receiveframetwai_message_t-rx_msg) method from [Class TwaiManager](#class-twaimanager). If the transmission is successful, this method also updates the instance motor ID (Note this method will **NOT** check the response from the motor, It is assumed that the motor ID change was successful if CAN frame was transmitted).
- Returns:
  - `bool` - `True` if Frame was transmited correctly, else `False`
- Parameters:
  - `new_can_id: uint8_t` - The new motor CAN ID (Recommemded between 0 and 127)



---

### Struct `CyberGearStatus`
**Parameters:**
- `cmd_id: uint8_t` - The index of the parameter
- `host_id: uint8_t` - The Host/Master ID
- `motor_id: uint8_t` - The motor CAN ID
- `motor_mode: uint8_t` - Motor mode
- `calibrated: bool` - Error in calibration
- `hall_encoder: bool` - Error in hall encoder
- `magnet_encoder: bool` - Error in magnet encoder
- `over_temp: bool` - Motor over temperature
- `over_curr: bool` - Motor over current
- `under_volt: bool` - Motor undervolt
- `position: float` - Motor position
- `speed: float` - Motor speed
- `torque: float` - Motor torque
- `temp: uint16_t` - Motor temperature
- `updated: bool` - Response status

---

### Struct `CyberGearParam`
**Parameters:**
- `cmd_id: uint8_t` - The index of the parameter
- `host_id: uint8_t` - The Host/Master ID
- `motor_id: uint8_t` - The motor CAN ID
- `updated: bool` - Response status
- `value: union` - Store the response value
  - `value u32: uint32_t` - For raw data or Read data as uint32_t
  - `value i32: int32_t`  - Read data as int32_t
  - `value u16: uint16_t` - Read data as uint16_t
  - `value i16: int16_t`  - Read data as int16_t
  - `value u8: uint8_t`   - Read data as uint8_t
  - `value f: float`      - Read data as float

---



