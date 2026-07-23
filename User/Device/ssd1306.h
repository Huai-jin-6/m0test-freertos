/**
 * @file ssd1306.h
 * @brief SSD1306 OLED 128x64 I2C 驱动（器件层）
 *
 * 依赖：BspI2c
 */

#ifndef __SSD1306_HPP__
#define __SSD1306_HPP__

#include <stdint.h>

class BspI2c;

class Ssd1306
{
public:
    struct Config
    {
        BspI2c *i2c     = nullptr;
        uint8_t dev_addr = 0x3C;
    };

    bool init(const Config &cfg);
    void clear();
    void set_cursor(uint8_t x, uint8_t y);
    void putc(char c);
    void puts(const char *s);
    void printf(uint8_t x, uint8_t y, const char *fmt, ...);

private:
    BspI2c *_i2c      = nullptr;
    uint8_t _dev_addr = 0x3C;
    uint8_t _cursor_x  = 0;
    uint8_t _cursor_y  = 0;

    void _wr_byte(uint8_t dat, uint8_t mode);
    void _cmd(uint8_t cmd);
    void _data(uint8_t dat);
    void _data_n(const uint8_t *d, uint8_t n);
};

#endif
