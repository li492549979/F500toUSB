# USB to MSX Keyboard Converter for Raspberry Pi Pico

This program implements a converter that transforms a USB keyboard into an MSX 13-pin matrix keyboard. Below are the main features and usage instructions:

## Main Functions

1. **USB HID Keyboard Support** - Supports standard USB keyboard input.
2. **MSX Keyboard Matrix Simulation** - Complete 11x8 matrix keyboard simulation.
3. **GPIO Interface Control** - Correctly handles all signals for the 13-pin interface.
4. **Real-time Conversion** - Real-time conversion of USB keystrokes to MSX keyboard signals.

## Hardware Connections

Based on the GPIO definitions in the program, you need to connect the Raspberry Pi Pico GPIO pins to the MSX 13-pin keyboard interface:

* **GPIO 0** → Pin 1 (KANA/CODE)
* **GPIO 1** → Pin 2 (CAPS)
* **GPIO 2** → Pin 3 (X0)
* **GPIO 3** → Pin 4 (X1)
* **GPIO 4** → Pin 5 (X2)
* **GPIO 5** → Pin 6 (X3)
* **GPIO 6** → Pin 7 (X4/YD)
* **GPIO 7** → Pin 8 (X5/YC)
* **GPIO 8** → Pin 9 (X6/YB)
* **GPIO 9** → Pin 10 (X7/YA)
* **GPIO 10** → Pin 11 (KBDIR)
* **3.3V/5V** → Pin 12 (+5V)
* **GND** → Pin 13 (GND)

## Compilation Requirements

You need to add the following dependencies to your `CMakeLists.txt`:

```cmake
target_link_libraries(your_target_name 
    pico_stdlib 
    tinyusb_host
    tinyusb_board
    hardware_gpio
)

```

## Key Features

1. **Complete Keyboard Mapping** - Includes letters, numbers, function keys, arrow keys, and the numeric keypad.
2. **Modifier Key Support** - Supports Shift, Ctrl, Alt, and other modifiers.
3. **Bidirectional GPIO Handling** - Correctly handles the MSX keyboard scanning protocol.
4. **Real-time Response** - 1ms scan cycle ensures timely response.

## USB Keyboard Connection Methods

### Method 1: Using a USB OTG Adapter (Recommended)

1. **Purchase a USB OTG Adapter**: You need a Micro USB to USB-A OTG adapter.
2. **Connection Steps**:
* Insert the USB OTG adapter into the Pico's Micro USB port.
* Insert the USB keyboard into the OTG adapter's USB-A port.
* The Pico will operate as a USB Host.



### Method 2: Direct Soldering of USB Interface

If you prefer a more stable connection, you can solder a USB-A female socket directly onto the Pico:

* **USB D+** → GPIO Pin (Configuration required in code)
* **USB D-** → GPIO Pin (Configuration required in code)
* **USB VCC** → 5V or 3.3V
* **USB GND** → GND

### Method 3: Using the Pico W USB Port

If using a Raspberry Pi Pico W, the connection method is the same, but power management is improved.

## USB Host Configuration in Code

The existing code is already configured for USB Host mode:

```c
// Initialize TinyUSB Host Mode
tuh_init(BOARD_TUH_RHPORT);

```

## Important Notes

1. **Power Requirements**: USB keyboards may require significant current; ensure the Pico has an adequate power supply.
2. **OTG Adapter Quality**: It is recommended to use a high-quality OTG adapter to avoid unstable connections.
3. **Compatibility**: Most standard USB keyboards are supported, but some keyboards with special functions may require additional drivers.

## Complete Connection Diagram

```
[USB Keyboard] 
    ↓ (USB-A)
[USB OTG Adapter]
    ↓ (Micro USB)
[Raspberry Pi Pico USB Port]

At the same time, Pico GPIO pins connect to the MSX:
[Pico GPIO 0-10] → [MSX 13-pin Keyboard Interface]

```

## Power Supply Suggestions

* If the USB keyboard consumes high power, it is recommended to power the Pico separately (via the VSYS pin).
* You can use a 5V power adapter to power the entire system.



# 一个树莓派Pico的程序，将USB键盘转换为MSX 13pin矩阵键盘。

USB to MSX Keyboard Converter for Raspberry Pi Pico

这个程序实现了USB键盘到MSX 13pin矩阵键盘的转换器。以下是主要特性和使用说明：

## 主要功能

1. **USB HID键盘支持** - 支持标准USB键盘输入
2. **MSX键盘矩阵模拟** - 完整的11x8矩阵键盘模拟
3. **GPIO接口控制** - 正确处理13pin接口的所有信号
4. **实时转换** - 实时将USB按键转换为MSX键盘信号

## 硬件连接

根据程序中的GPIO定义，你需要将树莓派Pico的GPIO引脚连接到MSX的13pin键盘接口：

- GPIO 0 → Pin 1 (KANA/CODE)
- GPIO 1 → Pin 2 (CAPS)
- GPIO 2 → Pin 3 (X0)
- GPIO 3 → Pin 4 (X1)
- GPIO 4 → Pin 5 (X2)
- GPIO 5 → Pin 6 (X3)
- GPIO 6 → Pin 7 (X4/YD)
- GPIO 7 → Pin 8 (X5/YC)
- GPIO 8 → Pin 9 (X6/YB)
- GPIO 9 → Pin 10 (X7/YA)
- GPIO 10 → Pin 11 (KBDIR)
- 3.3V/5V → Pin 12 (+5V)
- GND → Pin 13 (GND)

## 编译要求

你需要在CMakeLists.txt中添加以下依赖：

cmake

```cmake
target_link_libraries(your_target_name 
    pico_stdlib 
    tinyusb_host
    tinyusb_board
    hardware_gpio
)
```

## 主要特性

1. **完整键盘映射** - 包括字母、数字、功能键、方向键、数字小键盘
2. **修饰键支持** - Shift、Ctrl、Alt等修饰键
3. **双向GPIO处理** - 正确处理MSX键盘扫描协议
4. **实时响应** - 1ms扫描周期保证及时响应

## USB键盘连接方法

### 方法1：使用USB OTG适配器（推荐）

1. **购买USB OTG适配器**：需要一个Micro USB转USB-A的OTG适配器
2. **连接步骤**：
  - 将USB OTG适配器插入Pico的Micro USB端口
  - 将USB键盘插入OTG适配器的USB-A端口
  - Pico会作为USB主机工作

### 方法2：直接焊接USB接口

如果你想要更稳定的连接，可以直接在Pico上焊接USB-A母座：

- **USB D+** → GPIO引脚（需要在代码中配置）
- **USB D-** → GPIO引脚（需要在代码中配置）
- **USB VCC** → 5V或3.3V
- **USB GND** → GND

### 方法3：使用Pico W的USB端口

如果使用Pico W，连接方式相同，但功耗管理更好。

## 代码中的USB主机配置

现有代码已经配置为USB主机模式：


```c
// 初始化TinyUSB主机模式
tuh_init(BOARD_TUH_RHPORT);
```

## 注意事项

1. **供电要求**：USB键盘可能需要较大电流，确保Pico的供电充足
2. **OTG适配器质量**：建议使用质量好的OTG适配器，避免连接不稳定
3. **兼容性**：大多数标准USB键盘都支持，但某些带特殊功能的键盘可能需要额外驱动

## 完整的连接示意图

```
[USB键盘] 
    ↓ (USB-A)
[USB OTG适配器]
    ↓ (Micro USB)
[树莓派Pico的USB端口]

同时，Pico的GPIO引脚连接到MSX：
[Pico GPIO 0-10] → [MSX 13pin键盘接口]
```

## 供电建议

- 如果USB键盘功耗较大，建议给Pico单独供电（通过VSYS引脚）
- 可以使用5V电源适配器为整个系统供电
