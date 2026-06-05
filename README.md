# Simple-Xiaomi-Cybergear
Simple library to control Xiaomi Cybergear motor

Based on [Daniel Kalicki](https://github.com/DanielKalicki/Xiaomi_CyberGear_Arduino) work. 


## Usage

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
