#include "m202md28a.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"

static const char *TAG = "M202MD28A";

M202MD28A::M202MD28A(uart_port_t uart, int tx, int rx, int baud)
    : uart_num(uart), tx_pin(tx), rx_pin(rx), baud_rate(baud)
{
}

M202MD28A::~M202MD28A()
{
    uart_driver_delete(uart_num);
}

void M202MD28A::init()
{
    uart_config_t uart_config = {
        .baud_rate = baud_rate,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE};

    uart_driver_install(uart_num, BUF_SIZE * 2, 0, 0, NULL, 0);
    uart_param_config(uart_num, &uart_config);
    uart_set_pin(uart_num, tx_pin, rx_pin, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
}

void M202MD28A::send_bytes(const uint8_t *data, size_t length)
{
    uart_write_bytes(uart_num, (const char *)data, length);
}

void M202MD28A::send_command(const uint8_t *cmd, size_t length)
{
    send_bytes(cmd, length);
    // ESP_LOGI(TAG, "Sent command:");
    // for (size_t i = 0; i < length; ++i)
    // {
    //     printf("0x%02X ", cmd[i]);
    // }
    // printf("\n");
}

void M202MD28A::send_string(const char *str)
{
    uart_write_bytes(uart_num, str, strlen(str));
}

void M202MD28A::clear_screen()
{
    uint8_t cmd[] = {0x0C};
    send_command(cmd, sizeof(cmd));
}

void M202MD28A::cursor_home()
{
    uint8_t cmd[] = {0x0B};
    send_command(cmd, sizeof(cmd));
}

void M202MD28A::set_cursor(uint8_t x, uint8_t y)
{
    if (x >= 1 && x <= 20 && y >= 1 && y <= 2)
    {
        uint8_t cmd[] = {0x1F, 0x24, x, y};
        send_command(cmd, sizeof(cmd));
    }
}

void M202MD28A::delay_ms(int ms)
{
    vTaskDelay(ms / portTICK_PERIOD_MS);
}

// 示例字符放大渲染
bool M202MD28A::str_big_number(const std::string &input, std::vector<uint8_t> &upper, std::vector<uint8_t> &lower)
{
    // 字符到上半区和下半区字形码的映射
    const std::map<char, std::vector<uint8_t>> upper_map = {
        {'0', {0x80, 0x81}}, {'1', {0x84, 0x85}}, {'2', {0x88, 0x89}}, {'3', {0x8C, 0x8D}}, {'4', {0x90, 0x91}}, {'5', {0x94, 0x95}}, {'6', {0x98, 0x99}}, {'7', {0x9C, 0x9D}}, {'8', {0xA0, 0xA1}}, {'9', {0xA4, 0xA5}}, {' ', {0x20}}, {':', {0x2C}}};

    const std::map<char, std::vector<uint8_t>> lower_map = {
        {'0', {0x82, 0x83}}, {'1', {0x86, 0x87}}, {'2', {0x8A, 0x8B}}, {'3', {0x8E, 0x8F}}, {'4', {0x92, 0x93}}, {'5', {0x96, 0x97}}, {'6', {0x9A, 0x9B}}, {'7', {0x9E, 0x9F}}, {'8', {0xA2, 0xA3}}, {'9', {0xA6, 0xA7}}, {' ', {0x20}}, {':', {0x27}}};

    upper.clear();
    lower.clear();

    for (char ch : input)
    {
        auto it_upper = upper_map.find(ch);
        auto it_lower = lower_map.find(ch);

        if (it_upper == upper_map.end() || it_lower == lower_map.end())
        {
            std::cerr << "Unsupported character: " << ch << std::endl;
            return false;
        }

        upper.insert(upper.end(), it_upper->second.begin(), it_upper->second.end());
        lower.insert(lower.end(), it_lower->second.begin(), it_lower->second.end());
    }
    return true;
}

void M202MD28A::draw_string(const char *str)
{
    send_string(str);
}


void M202MD28A::display_time_loop()
{
    clear_screen();
    delay_ms(100);
    while (true)
    {
        cursor_home();
        time_t now;
        struct tm timeinfo;
        time(&now);
        localtime_r(&now, &timeinfo);

        char time_str[20];
        strftime(time_str, sizeof(time_str), "   %H:%M:%S   ", &timeinfo);

        uint8_t line1[64], line2[64];
        std::vector<uint8_t> upper, lower;

        str_big_number(time_str, upper, lower);

        // size_t len = upper.size();
        size_t len = 20;
        if (len > sizeof(line1))
            len = sizeof(line1); // 最多只发送64字节

        memcpy(line1, upper.data(), len);
        memcpy(line2, lower.data(), len);

        set_cursor(1, 1);
        send_bytes(line1, 20);
        set_cursor(1, 2);
        send_bytes(line2, 20);
        delay_ms(10);
    }
}

void M202MD28A::display_test()
{
    clear_screen();
    delay_ms(100);
    uint8_t i = 0;
    uint8_t ms_2digits = 0;
    uint8_t line1[20] = {'0'};
    while (true)
    {
        ms_2digits++;
        // snprintf(line1, sizeof(line1), "%04d", ms_2digits);
        set_cursor(1, 1);
        send_bytes(line1, 20);
        set_cursor(1, 2);
        send_bytes(line1, 20);
        for (uint8_t i = 0; i<20; i++)
        {
            line1[i]++;
            if(line1[i] > 'z' )
            {
                line1[i] = '0';
            }
        }
    }
}
