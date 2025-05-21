#ifndef __M202MD28A_HPP__
#define __M202MD28A_HPP__

#include "driver/uart.h"
#include "esp_log.h"
#include <string.h>
#include <time.h>
#include <map>
#include <string>
#include <vector>
#include <iostream>

class M202MD28A
{
private:
    uart_port_t uart_num;
    int tx_pin;
    int rx_pin;
    int baud_rate;

    static constexpr size_t BUF_SIZE = 1024;

    void send_bytes(const uint8_t* data, size_t length);
    void send_command(const uint8_t* command, size_t length);
    void set_cursor(uint8_t x, uint8_t y);
    void send_string(const char* str);
    void delay_ms(int ms);


public:
    M202MD28A(uart_port_t uart_num, int tx, int rx, int baudrate = 9600);
    ~M202MD28A();

    void init();
    void clear_screen();
    void cursor_home();
    void display_time_loop();
    void test_display();
    void display_test();
    bool str_big_number(const std::string& input, \
                    std::vector<uint8_t>& upper, std::vector<uint8_t>& lower); 
    void draw_string(const char* str);
};

#endif
