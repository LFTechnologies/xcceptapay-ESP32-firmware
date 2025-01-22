# XceptaPay ESP32 Firmware

_Firmware for the ESP32 device to handle BLE payments using XRP._

## Overview

This firmware allows an ESP32 device to:

- Act as a BLE server for communication with the XceptaPay mobile app.
- Display transaction status on an OLED screen.
- Control external hardware via GPIO based on received BLE commands.

## Features

- BLE communication with UUID-based service and characteristic.
- OLED display integration for real-time transaction status.
- Secure transaction processing using predefined XRP wallet addresses.
- State machine implementation for handling transaction flow.

## Requirements

### Hardware
- ESP32 board
- OLED display (SSD1306 128x64)
- External hardware controlled via GPIO pin 8

### Software
- Arduino IDE with ESP32 board support
- Required libraries:
  - `BLEDevice.h`
  - `BLEUtils.h`
  - `BLEServer.h`
  - `U8g2lib.h`

## Installation

1. Clone the repository:
   ```sh
   git clone https://github.com/yourusername/xceptapay-esp32.git
   cd xceptapay-esp32
   ```
2. Open the project in Arduino IDE.
3. Install the required libraries via Library Manager.
4. Set the correct board to ESP32 and upload the firmware.

## Configuration

Update the following macros in the firmware if needed:

```cpp
#define SERVICE_UUID        "12345678-1234-5678-1234-56789abcdef0"
#define CHARACTERISTIC_UUID "12345678-1234-5678-1234-56789abcdef0"
#define XRP_WALLET_ADDRESS  "rh1Ms9YB16C5B4kBMDNQnvC7ybqPLHkrWg"
#define CONTROL_PIN         8
```

## Usage

1. Power on the ESP32 device.
2. The BLE service will start advertising with the name `ESP32_BLE_SERVER`.
3. Use the XceptaPay app to connect and initiate a transaction.
4. Monitor the OLED screen for transaction status updates.
5. Upon transaction completion, the control pin will be set HIGH.

## State Machine

The firmware handles transactions using the following states:

- **STATE_IDLE**: Waiting for a transaction.
- **STATE_TRANSACTION_PROCESSING**: Processing an incoming transaction.
- **STATE_TRANSACTION_COMPLETE**: Transaction successfully processed.
- **STATE_ERROR**: Error occurred during the transaction.

## Troubleshooting

- If the OLED screen does not initialize, check the SDA/SCL pin connections.
- Ensure the BLE device is visible by scanning with a BLE scanner app.
- Verify the control pin connection to the external hardware.

## Contributing

Contributions are welcome! Please follow these steps:

1. Fork the repository.
2. Create a feature branch (`git checkout -b feature-name`).
3. Commit your changes (`git commit -m 'Add feature'`).
4. Push to your branch (`git push origin feature-name`).
5. Create a pull request.

## License

This project is licensed under the MIT License.

## Support

For any issues, please open a ticket on the [GitHub Issues page](https://github.com/yourusername/xceptapay-esp32/issues).

