# Smart Access-Controlled Door

An embedded smart-door controller built around an STM32F303RE microcontroller. The system combines NFC authentication, keypad-based management, scheduled access events, light-dependent sensors, a stepper motor, an LCD, and an audible/visual unauthorized-entry alert.

## Credit
- Matthew Su
- Pengyu Liu

## Features

- NFC/RFID authentication through a PN532 module over I2C.
- Separate authorization paths for:
  - A door-access card that requests an entrance-side unlock.
  - A manager card that opens the keypad management menu.
- 4x4 matrix keypad input.
- 16x2 character LCD user interface.
- Scheduled unlock events with configurable date, time, and duration.
- Stepper-motor door control with independent entrance and exit opening positions.
- Two LDR sensors for detecting movement through the doorway and determining travel direction.
- Automatic door closing after the configured open period.
- Unauthorized-entry detection when the number of people entering exceeds authorized access events.
- Buzzer tone and shift-register display alert for unauthorized access.
- UART2 debug output for authentication and sensor readings.

## Hardware

| Component | Interface / connection |
| --- | --- |
| STM32F303RETx / NUCLEO-F303RE | Main microcontroller |
| PN532 NFC reader | I2C1 |
| 4x4 matrix keypad | Rows: PB11-PB14; columns: PA8-PA11 |
| 16x2 LCD | 4-bit parallel interface |
| Stepper motor driver | Coil outputs on PA12, PB2, PC5, and PC7 |
| LDR sensor 1 | PC4 / ADC2 channel 5 |
| LDR sensor 2 | PB1 / ADC3 channel 1 |
| Shift register | Serial data: PB15; clock: PC0; latch: PB7 |
| Buzzer output | TIM1 channel 3 on PC2 |
| UART debug interface | USART2 on PA2/PA3 |

The exact pin configuration is also available in [`project.ioc`](project.ioc), which can be opened in STM32CubeMX or STM32CubeIDE.

## Software Requirements

- [STM32CubeIDE](https://www.st.com/en/development-tools/stm32cubeide.html)
- An ST-LINK-compatible programmer/debugger
- STM32F303RE hardware and the peripheral modules listed above
- A USB serial terminal, if UART debug output is required

The project was generated with STM32CubeMX 6.17.0 and uses the STM32 HAL and CMSIS drivers included in the repository.

## Getting Started

### 1. Clone the repository

```bash
git clone <repository-url>
cd desn2k_project
```

### 2. Import the project into STM32CubeIDE

1. Open STM32CubeIDE.
2. Select **File > Import**.
3. Choose **General > Existing Projects into Workspace**.
4. Select the cloned `desn2k_project` directory.
5. Import the project.

The Eclipse project metadata and STM32CubeMX configuration are included in the repository. The project name inside STM32CubeIDE is `project`.

### 3. Build and flash

1. Connect the STM32 board through an ST-LINK-compatible debugger.
2. Select the project in the Project Explorer.
3. Select **Project > Build Project**.
4. Select **Run > Debug** or **Run > Run** to flash the firmware.
5. Open a serial terminal on USART2 if you want to inspect debug output.

The generated build output is placed in the `Debug/` directory and is ignored by Git.

## System Operation

### Startup

After reset, the firmware initializes the motor, LCD, timers, ADC channels, I2C interface, UART, and PN532 reader. The LCD then prompts the operator to enter the initial date and time using the keypad. This time is used as the reference for scheduled events.

### Authentication

- Present the configured door-access NFC card to request an entrance-side unlock.
- Present the configured manager NFC card to enter the management menu.
- Unknown cards are rejected.

The stored card UIDs are currently defined in [`Core/Src/proj_auth.c`](Core/Src/proj_auth.c). Replace the example values with the UIDs for the cards used by your hardware before deploying the firmware.

### Keypad controls

The keypad is interpreted as follows while the manager menu is active:

| Key | Action |
| --- | --- |
| `1` | Previous scheduled event |
| `3` | Next scheduled event |
| `A` | Add a scheduled unlock event |
| `B` | Open the door manually from the keypad |
| `C` | Exit the manager menu |
| `D` | Delete the selected event |
| `#` | Confirm date, time, or duration input |
| `*` | Cancel the current input |

Scheduled events are stored in a sorted doubly linked list. Each event creates an unlock start and lock end action, allowing the door to remain available for a configured duration.

### Door and alert behavior

The two LDR sensors monitor the doorway in sequence to identify people entering or leaving. The controller opens the door from the appropriate side, waits for the configured open interval, and then closes it when the passage is clear.

If more people enter than the number of authorized access events, the firmware starts an unauthorized-entry alert using the buzzer and shift-register display. The alert can be stopped with the configured alert-control switch.

## Repository Structure

```text
.
├── Core/
│   ├── Inc/                 Application headers
│   ├── Src/                 Application and HAL initialization code
│   └── Startup/             STM32 startup assembly
├── Drivers/
│   ├── CMSIS/               ARM and STM32 CMSIS headers
│   └── STM32F3xx_HAL_Driver/ STM32F3 HAL drivers
├── project.ioc              STM32CubeMX pin and peripheral configuration
├── STM32F303RETX_FLASH.ld   Linker script
└── project Debug.launch     STM32CubeIDE launch configuration
```

Key application modules:

- `main.c` - initialization, door state machine, person-position tracking, and main loop.
- `proj_auth.c` - PN532 communication and NFC UID authorization.
- `proj_events.c` - keypad menu, event scheduling, time conversion, and LCD screens.
- `motor.c` - stepper-motor sequencing and door positions.
- `ldr.c` - ADC-based LDR sensor readings.
- `proj_keypad.c` - 4x4 keypad scanning and debounce.
- `proj_lcd.c` - 16x2 LCD driver.
- `ShiftReg.c` - shift-register alert display control.

## Configuration Notes

- The LDR activation threshold is defined by `LDR_THRES` in `Core/Src/ldr.c` and may need calibration for the physical sensors and lighting conditions.
- Door positions and motor travel are defined in `Core/Inc/motor.h`.
- The automatic door-open interval is configured by `DOPEN_PERIOD` in `Core/Src/main.c`.
- NFC card UIDs are stored in `Core/Src/proj_auth.c`.
- The firmware currently uses dynamic allocation for scheduled events, so the number of events should be kept within the available RAM.

## Development Notes

This repository contains the STM32CubeIDE project files and the generated STM32 HAL/CMSIS driver sources required to build the firmware. Hardware behavior should be tested with the motor driver disconnected or mechanically secured during initial firmware bring-up.

## License

The application source does not currently include a separate project license. The bundled STM32 HAL and CMSIS components retain their respective STMicroelectronics and ARM license files in the `Drivers/` directory.
