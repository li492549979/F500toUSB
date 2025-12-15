#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "bsp/board.h"
#include "tusb.h"

// HID key code for the quote (') key if not defined in your headers
#ifndef HID_KEY_QUOTE
#define HID_KEY_QUOTE 0x34
#endif

#ifndef BOARD_TUH_RHPORT
#define BOARD_TUH_RHPORT 0
#endif

// MSX键盘矩阵定义 - 13pin接口
// Pin 1: KANA/CODE (Output)
// Pin 2: CAPS (Output) 
// Pin 3: X0 (Input)
// Pin 4: X1 (Input)
// Pin 5: X2 (Input)
// Pin 6: X3 (Input)
// Pin 7: X4/YD (Input/Output)
// Pin 8: X5/YC (Input/Output) 
// Pin 9: X6/YB (Input/Output)
// Pin 10: X7/YA (Input/Output)
// Pin 11: KBDIR (Input/Output) - 键盘端口方向
// Pin 12: +5V
// Pin 13: GND

// GPIO引脚定义 (根据实际连接调整)
#define MSX_KANA_CODE_PIN   0   // Pin 1
#define MSX_CAPS_PIN        1   // Pin 2
#define MSX_X0_PIN          2   // Pin 3
#define MSX_X1_PIN          3   // Pin 4
#define MSX_X2_PIN          4   // Pin 5
#define MSX_X3_PIN          5   // Pin 6
#define MSX_X4_YD_PIN       6   // Pin 7
#define MSX_X5_YC_PIN       7   // Pin 8
#define MSX_X6_YB_PIN       8   // Pin 9
#define MSX_X7_YA_PIN       9   // Pin 10
#define MSX_KBDIR_PIN      10   // Pin 11

// MSX键盘矩阵 - Y行(输出) X列(输入)
// Y行选择通过X4-X7/YA-YD控制
#define MSX_MATRIX_ROWS 11
#define MSX_MATRIX_COLS 8

// MSX键盘矩阵状态
static bool msx_key_matrix[MSX_MATRIX_ROWS][MSX_MATRIX_COLS] = {0};

// USB键盘状态
static uint8_t prev_report[6] = {0};

// MSX键盘映射表 - USB HID码到MSX矩阵位置
typedef struct {
    uint8_t hid_code;    // USB HID键码
    uint8_t msx_row;     // MSX矩阵行
    uint8_t msx_col;     // MSX矩阵列
} key_mapping_t;

// MSX标准键盘布局映射
static const key_mapping_t key_map[] = {
    // 数字行 (Row 0)
    {HID_KEY_0, 0, 0},          // 0
    {HID_KEY_1, 0, 1},          // 1
    {HID_KEY_2, 0, 2},          // 2
    {HID_KEY_3, 0, 3},          // 3
    {HID_KEY_4, 0, 4},          // 4
    {HID_KEY_5, 0, 5},          // 5
    {HID_KEY_6, 0, 6},          // 6
    {HID_KEY_7, 0, 7},          // 7
    
    // 第二行 (Row 1)
    {HID_KEY_8, 1, 0},          // 8
    {HID_KEY_9, 1, 1},          // 9
    {HID_KEY_MINUS, 1, 2},      // -
    {HID_KEY_EQUAL, 1, 3},      // =
    {HID_KEY_BACKSLASH, 1, 4},  // "\"
    {HID_KEY_BRACKET_LEFT, 1, 5}, // [
    {HID_KEY_BRACKET_RIGHT, 1, 6}, // ]
    {HID_KEY_SEMICOLON, 1, 7},  // ;
    
    // 第三行 (Row 2) - QWERTY行
    {HID_KEY_QUOTE, 2, 0},      // '
    {HID_KEY_A, 2, 1},          // A
    {HID_KEY_B, 2, 2},          // B
    {HID_KEY_C, 2, 3},          // C
    {HID_KEY_D, 2, 4},          // D
    {HID_KEY_E, 2, 5},          // E
    {HID_KEY_F, 2, 6},          // F
    {HID_KEY_G, 2, 7},          // G
    
    // 第四行 (Row 3)
    {HID_KEY_H, 3, 0},          // H
    {HID_KEY_I, 3, 1},          // I
    {HID_KEY_J, 3, 2},          // J
    {HID_KEY_K, 3, 3},          // K
    {HID_KEY_L, 3, 4},          // L
    {HID_KEY_M, 3, 5},          // M
    {HID_KEY_N, 3, 6},          // N
    {HID_KEY_O, 3, 7},          // O
    
    // 第五行 (Row 4)
    {HID_KEY_P, 4, 0},          // P
    {HID_KEY_Q, 4, 1},          // Q
    {HID_KEY_R, 4, 2},          // R
    {HID_KEY_S, 4, 3},          // S
    {HID_KEY_T, 4, 4},          // T
    {HID_KEY_U, 4, 5},          // U
    {HID_KEY_V, 4, 6},          // V
    {HID_KEY_W, 4, 7},          // W
    
    // 第六行 (Row 5)
    {HID_KEY_X, 5, 0},          // X
    {HID_KEY_Y, 5, 1},          // Y
    {HID_KEY_Z, 5, 2},          // Z
    {HID_KEY_SHIFT_LEFT, 5, 3}, // SHIFT
    {HID_KEY_CONTROL_LEFT, 5, 4}, // CTRL
    {HID_KEY_GRAVE, 5, 5},      // `
    {HID_KEY_COMMA, 5, 6},      // ,
    {HID_KEY_PERIOD, 5, 7},     // .
    
    // 第七行 (Row 6)
    {HID_KEY_SLASH, 6, 0},      // /
    {HID_KEY_SHIFT_RIGHT, 6, 1}, // Right SHIFT (Dead Key)
    {HID_KEY_ARROW_UP, 6, 2},   // ↑
    {HID_KEY_ARROW_RIGHT, 6, 3}, // →
    {HID_KEY_ARROW_DOWN, 6, 4}, // ↓
    {HID_KEY_ARROW_LEFT, 6, 5}, // ←
    {HID_KEY_DELETE, 6, 6},     // DEL
    {HID_KEY_INSERT, 6, 7},     // INS
    
    // 第八行 (Row 7)
    {HID_KEY_HOME, 7, 0},       // HOME/CLR
    {HID_KEY_PAGE_UP, 7, 1},    // Previous Page
    {HID_KEY_PAGE_DOWN, 7, 2},  // Next Page
    {HID_KEY_F1, 7, 3},         // F1
    {HID_KEY_F2, 7, 4},         // F2
    {HID_KEY_F3, 7, 5},         // F3
    {HID_KEY_F4, 7, 6},         // F4
    {HID_KEY_F5, 7, 7},         // F5
    
    // 第九行 (Row 8)
    {HID_KEY_ESCAPE, 8, 0},     // ESC
    {HID_KEY_TAB, 8, 1},        // TAB
    {HID_KEY_END, 8, 2},        // STOP
    {HID_KEY_BACKSPACE, 8, 3},  // BS
    {HID_KEY_ENTER, 8, 4},      // RETURN
    {HID_KEY_SPACE, 8, 5},      // SPACE
    {HID_KEY_ALT_LEFT, 8, 6},   // GRAPH
    {HID_KEY_CAPS_LOCK, 8, 7},  // CAPS
    
    // 第十行 (Row 9) - 数字键盘
    {HID_KEY_KEYPAD_1, 9, 0},   // NUM 1
    {HID_KEY_KEYPAD_2, 9, 1},   // NUM 2
    {HID_KEY_KEYPAD_3, 9, 2},   // NUM 3
    {HID_KEY_KEYPAD_4, 9, 3},   // NUM 4
    {HID_KEY_KEYPAD_5, 9, 4},   // NUM 5
    {HID_KEY_KEYPAD_6, 9, 5},   // NUM 6
    {HID_KEY_KEYPAD_7, 9, 6},   // NUM 7
    {HID_KEY_KEYPAD_8, 9, 7},   // NUM 8
    
    // 第十一行 (Row 10)
    {HID_KEY_KEYPAD_9, 10, 0},  // NUM 9
    {HID_KEY_KEYPAD_0, 10, 1},  // NUM 0
    {HID_KEY_KEYPAD_SUBTRACT, 10, 2}, // NUM -
    {HID_KEY_KEYPAD_ADD, 10, 3}, // NUM +
    {HID_KEY_KEYPAD_MULTIPLY, 10, 4}, // NUM *
    {HID_KEY_KEYPAD_DIVIDE, 10, 5}, // NUM /
    {HID_KEY_KEYPAD_DECIMAL, 10, 6}, // NUM .
    {HID_KEY_KEYPAD_ENTER, 10, 7}, // NUM Enter
};

// 初始化MSX键盘接口GPIO
void msx_keyboard_init() {
    // 设置输出引脚 (KANA/CODE, CAPS)
    gpio_init(MSX_KANA_CODE_PIN);
    gpio_set_dir(MSX_KANA_CODE_PIN, GPIO_OUT);
    gpio_put(MSX_KANA_CODE_PIN, 1); // 默认高电平
    
    gpio_init(MSX_CAPS_PIN);
    gpio_set_dir(MSX_CAPS_PIN, GPIO_OUT);
    gpio_put(MSX_CAPS_PIN, 1); // 默认高电平
    
    // 设置输入引脚 (X0-X3)
    gpio_init(MSX_X0_PIN);
    gpio_set_dir(MSX_X0_PIN, GPIO_IN);
    gpio_pull_up(MSX_X0_PIN);
    
    gpio_init(MSX_X1_PIN);
    gpio_set_dir(MSX_X1_PIN, GPIO_IN);
    gpio_pull_up(MSX_X1_PIN);
    
    gpio_init(MSX_X2_PIN);
    gpio_set_dir(MSX_X2_PIN, GPIO_IN);
    gpio_pull_up(MSX_X2_PIN);
    
    gpio_init(MSX_X3_PIN);
    gpio_set_dir(MSX_X3_PIN, GPIO_IN);
    gpio_pull_up(MSX_X3_PIN);
    
    // 设置双向引脚 (X4-X7/YA-YD, KBDIR)
    gpio_init(MSX_X4_YD_PIN);
    gpio_init(MSX_X5_YC_PIN);
    gpio_init(MSX_X6_YB_PIN);
    gpio_init(MSX_X7_YA_PIN);
    gpio_init(MSX_KBDIR_PIN);
    
    // 默认设置为输入模式
    gpio_set_dir(MSX_X4_YD_PIN, GPIO_IN);
    gpio_set_dir(MSX_X5_YC_PIN, GPIO_IN);
    gpio_set_dir(MSX_X6_YB_PIN, GPIO_IN);
    gpio_set_dir(MSX_X7_YA_PIN, GPIO_IN);
    gpio_set_dir(MSX_KBDIR_PIN, GPIO_IN);
    
    gpio_pull_up(MSX_X4_YD_PIN);
    gpio_pull_up(MSX_X5_YC_PIN);
    gpio_pull_up(MSX_X6_YB_PIN);
    gpio_pull_up(MSX_X7_YA_PIN);
    gpio_pull_up(MSX_KBDIR_PIN);
    
    printf("MSX键盘接口初始化完成\n");
}

// 将USB HID键码映射到MSX矩阵
void map_usb_to_msx(uint8_t hid_code, bool pressed) {
    size_t map_size = sizeof(key_map) / sizeof(key_mapping_t);
    for (size_t i = 0; i < map_size; i++) {
        if (key_map[i].hid_code == hid_code) {
            msx_key_matrix[key_map[i].msx_row][key_map[i].msx_col] = pressed;
            // 只在有变化时输出，且可选关闭
            //#define MSX_DEBUG
#ifdef MSX_DEBUG
            printf("按键 %s: HID=0x%02X -> MSX[%d][%d]\n", 
                   pressed ? "按下" : "释放", 
                   hid_code, 
                   key_map[i].msx_row, 
                   key_map[i].msx_col);
#endif
            return;
        }
    }
    // 未映射的键
#ifdef MSX_DEBUG
    if (pressed) {
        printf("未映射的按键: HID=0x%02X\n", hid_code);
    }
#endif
}

// 扫描MSX键盘矩阵
void scan_msx_keyboard() {
    // 检查KBDIR状态，确定是输入还是输出模式
    bool kbdir_state = gpio_get(MSX_KBDIR_PIN);
    if (kbdir_state) {
        // KBDIR为高，MSX正在读取键盘
        // 设置Y行为输出模式，响应MSX的扫描
        gpio_set_dir(MSX_X4_YD_PIN, GPIO_OUT);
        gpio_set_dir(MSX_X5_YC_PIN, GPIO_OUT);
        gpio_set_dir(MSX_X6_YB_PIN, GPIO_OUT);
        gpio_set_dir(MSX_X7_YA_PIN, GPIO_OUT);

        // 读取X列输入，确定MSX要扫描哪一行（假设X0~X3为行选择）
        uint8_t row_sel = 0;
        row_sel |= (gpio_get(MSX_X0_PIN) ? 0 : 1) << 0;
        row_sel |= (gpio_get(MSX_X1_PIN) ? 0 : 1) << 1;
        row_sel |= (gpio_get(MSX_X2_PIN) ? 0 : 1) << 2;
        row_sel |= (gpio_get(MSX_X3_PIN) ? 0 : 1) << 3;

        // 只反馈被选中的那一行的列状态到Y引脚（X4~X7）
        if (row_sel < MSX_MATRIX_ROWS) {
            uint8_t col_bits = 0;
            for (int col = 0; col < MSX_MATRIX_COLS; col++) {
                if (msx_key_matrix[row_sel][col]) {
                    col_bits |= (1 << col);
                }
            }
            // 输出到Y引脚（X4~X7），低电平表示有按键
            gpio_put(MSX_X4_YD_PIN, (col_bits & 0x08) ? 0 : 1);
            gpio_put(MSX_X5_YC_PIN, (col_bits & 0x10) ? 0 : 1);
            gpio_put(MSX_X6_YB_PIN, (col_bits & 0x20) ? 0 : 1);
            gpio_put(MSX_X7_YA_PIN, (col_bits & 0x40) ? 0 : 1);
        } else {
            // 未选中有效行，全部拉高
            gpio_put(MSX_X4_YD_PIN, 1);
            gpio_put(MSX_X5_YC_PIN, 1);
            gpio_put(MSX_X6_YB_PIN, 1);
            gpio_put(MSX_X7_YA_PIN, 1);
        }
    } else {
        // KBDIR为低，设置为输入模式
        gpio_set_dir(MSX_X4_YD_PIN, GPIO_IN);
        gpio_set_dir(MSX_X5_YC_PIN, GPIO_IN);
        gpio_set_dir(MSX_X6_YB_PIN, GPIO_IN);
        gpio_set_dir(MSX_X7_YA_PIN, GPIO_IN);
        gpio_pull_up(MSX_X4_YD_PIN);
        gpio_pull_up(MSX_X5_YC_PIN);
        gpio_pull_up(MSX_X6_YB_PIN);
        gpio_pull_up(MSX_X7_YA_PIN);
    }
}

// USB HID键盘报告处理
void process_hid_report(hid_keyboard_report_t const *report) {
    // 检查修饰键变化
    static uint8_t prev_modifier = 0;
    if (prev_modifier != report->modifier) {
        map_usb_to_msx(HID_KEY_CONTROL_LEFT, report->modifier & KEYBOARD_MODIFIER_LEFTCTRL);
        map_usb_to_msx(HID_KEY_SHIFT_LEFT, report->modifier & KEYBOARD_MODIFIER_LEFTSHIFT);
        map_usb_to_msx(HID_KEY_ALT_LEFT, report->modifier & KEYBOARD_MODIFIER_LEFTALT);
        map_usb_to_msx(HID_KEY_GUI_LEFT, report->modifier & KEYBOARD_MODIFIER_LEFTGUI);
        map_usb_to_msx(HID_KEY_CONTROL_RIGHT, report->modifier & KEYBOARD_MODIFIER_RIGHTCTRL);
        map_usb_to_msx(HID_KEY_SHIFT_RIGHT, report->modifier & KEYBOARD_MODIFIER_RIGHTSHIFT);
        map_usb_to_msx(HID_KEY_ALT_RIGHT, report->modifier & KEYBOARD_MODIFIER_RIGHTALT);
        map_usb_to_msx(HID_KEY_GUI_RIGHT, report->modifier & KEYBOARD_MODIFIER_RIGHTGUI);
        prev_modifier = report->modifier;
    }

    // 检查是否所有keycode都为0（无按键）
    bool all_zero = true;
    for (int i = 0; i < 6; i++) {
        if (report->keycode[i] != 0) {
            all_zero = false;
            break;     
        }
    }
    if (all_zero) {
        // 清空所有按键状态
        for (int row = 0; row < MSX_MATRIX_ROWS; row++) {
            for (int col = 0; col < MSX_MATRIX_COLS; col++) {
                msx_key_matrix[row][col] = false;
            }
        }
        memset(prev_report, 0, sizeof(prev_report));
        return;
    }

    // 检查普通按键变化
    for (int i = 0; i < 6; i++) {
        uint8_t key = report->keycode[i];
        if (key != 0) {
            bool found_in_prev = false;
            for (int j = 0; j < 6; j++) {
                if (prev_report[j] == key) {
                    found_in_prev = true;
                    break;
                }
            }
            if (!found_in_prev) {
                map_usb_to_msx(key, true);
            }
        }
    }
    // 检查释放的键
    for (int i = 0; i < 6; i++) {
        uint8_t key = prev_report[i];
        if (key != 0) {
            bool found_in_current = false;
            for (int j = 0; j < 6; j++) {
                if (report->keycode[j] == key) {
                    found_in_current = true;
                    break;
                }
            }
            if (!found_in_current) {
                map_usb_to_msx(key, false);
            }
        }
    }
    memcpy(prev_report, report->keycode, 6);
}

// TinyUSB回调函数
void tuh_hid_mount_cb(uint8_t dev_addr, uint8_t instance, uint8_t const* desc_report, uint16_t desc_len) {
    printf("HID设备已连接\n");
    tuh_hid_receive_report(dev_addr, instance);
}

void tuh_hid_umount_cb(uint8_t dev_addr, uint8_t instance) {
    printf("HID设备已断开\n");
}

void tuh_hid_report_received_cb(uint8_t dev_addr, uint8_t instance, uint8_t const* report, uint16_t len) {
    if (len == sizeof(hid_keyboard_report_t)) {
        process_hid_report((hid_keyboard_report_t const*)report);
    }
    
    // 继续接收报告
    tuh_hid_receive_report(dev_addr, instance);
}

int main() {
    // 初始化Pico
    board_init();
    stdio_init_all();
    
    printf("USB转MSX键盘转换器启动中...\n");
    
    // 初始化MSX键盘接口
    msx_keyboard_init();
    
    // 初始化TinyUSB主机模式
    tuh_init(BOARD_TUH_RHPORT);
    
    printf("等待USB键盘连接...\n");
    
    // 主循环
    while (1) {
        // 处理USB事件
        tuh_task();
        
        // 扫描MSX键盘矩阵
        scan_msx_keyboard();
        
        // 短暂延迟
        sleep_ms(1);
    }
    
    return 0;
}