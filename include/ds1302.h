#pragma once

#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include <stdint.h>
#include <string>

namespace DS1302 {
/**
 * Months of year
 */
enum MONTH
{
    MONTH_JAN = 1,
    MONTH_FEB = 2,
    MONTH_MAR = 3,
    MONTH_APR = 4,
    MONTH_MAY = 5,
    MONTH_JUN = 6,
    MONTH_JUL = 7,
    MONTH_AUG = 8,
    MONTH_SET = 9,
    MONTH_OCT = 10,
    MONTH_NOV = 11,
    MONTH_DEC = 12
};

/**
 * Days of week
 */
enum DOW
{
    DOW_MON = 1,
    DOW_TUE = 2,
    DOW_WED = 3,
    DOW_THU = 4,
    DOW_FRI = 5,
    DOW_SAT = 6,
    DOW_SUN = 7
};

struct DateTime
{
    uint8_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
    uint8_t day_of_week;

    std::string getDayOfWeek()
    {
        if (day_of_week == 1)
            return "Monday";
        if (day_of_week == 2)
            return "Tuesday";
        if (day_of_week == 3)
            return "Wednesday";
        if (day_of_week == 4)
            return "Thursday";
        if (day_of_week == 5)
            return "Friday";
        if (day_of_week == 6)
            return "Saturday";
        if (day_of_week == 7)
            return "Sunday";
        return "Unknown";
    }

    std::string getString()
    {
        char buff[40];
        sprintf(buff,
                "%s\n%02d/%02d/20%d\n%02d:%02d:%02d\n",
                getDayOfWeek().c_str(),
                day,
                month,
                year,
                hour,
                minute,
                second);

        return {buff};
    }
};

class Clock
{
public:
    Clock(uint8_t pin_rst, uint8_t pin_clk, uint8_t pin_dat);

    DateTime getDateTime();
    void setDateTime(const DateTime &dt);

    bool isHalted();
    void halt();
    void run();

    void disableWrite();
    void enableWrite();

private:
    void prepareRead(uint8_t address);
    void prepareWrite(uint8_t address);
    void end();

    uint8_t readByte();
    void writeByte(uint8_t value);
    void nextBit();

    uint8_t dec2bcd(uint8_t dec);
    uint8_t bcd2dec(uint8_t bcd);

    void setDirection(int direction);

    void getRamByte(uint8_t addr, uint8_t *data);
    void setRamByte(uint8_t addr, uint8_t *data);
    void testRam();

    void writeRamBulk(const uint8_t *data, int len);
    void readRamBulk(uint8_t *data, int len);

    static const int _kRamSize = 31;

    uint8_t _pin_rst;
    uint8_t _pin_clk;
    uint8_t _pin_dat;
    uint8_t _dat_direction = GPIO_IN;
};
}  // namespace DS1302
