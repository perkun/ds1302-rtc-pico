#include "ds1302.h"

// see ds1302.h

// KSW added names from Dallas Semiconductor spec, as comments
// REGISTER ADDRESS
// A. CLOCK
// Note: bit 0 is RD/W  RD=1 W=0
// Note: bit 6 is 0
// bit order: 7 6 5 4 3 2 1 0
#define REG_SECONDS 0x80  // SEC
#define REG_MINUTES 0x82  // MIN
#define REG_HOUR 0x84     // HR
#define REG_DATE 0x86     // DATE
#define REG_MONTH 0x88    // MONTH
#define REG_DAY 0x8A      // DAY
#define REG_YEAR 0x8C     // YEAR
#define REG_WP 0x8E       // CONTROL  (msparks kWriteProtectReg 7)
// TRICKLE CHARGER 1001000_ 0x90
#define REG_BURST 0xBE  // CLOCK BURST

// B: RAM  (from KSW, Dallas Semiconductor spec)
// Note: bit 6 is 1
#define REG_RAM00 0xC0
#define REG_RAM01 0xC2
#define REG_RAM29 0xFA
#define REG_RAM30 0xFC
#define REG_RAM_BURST 0xFE  //(msparks kRamBurstWrite)
// msparks kRamBurstRead 0xFF)
//
// the following defines are from Arduino.h
// these are needed in Ds1302.cpp
#define HIGH 0x1
#define LOW 0x0

namespace DS1302 {

Clock::Clock(uint8_t pin_rst, uint8_t pin_clk, uint8_t pin_dat)
    : _pin_rst(pin_rst), _pin_clk(pin_clk), _pin_dat(pin_dat)
{
    gpio_init(_pin_rst);
    gpio_set_dir(_pin_rst, GPIO_OUT);
    gpio_set_function(_pin_rst, GPIO_FUNC_SIO);
    gpio_put(_pin_rst, LOW);

    gpio_init(_pin_clk);
    gpio_set_dir(_pin_clk, GPIO_OUT);
    gpio_set_function(_pin_clk, GPIO_FUNC_SIO);
    gpio_put(_pin_clk, LOW);

    gpio_init(_pin_dat);
    gpio_set_function(_pin_dat, GPIO_FUNC_SIO);
    gpio_set_dir(_pin_dat, _dat_direction);
}

DateTime Clock::getDateTime()
{
    DateTime dt;
    prepareRead(REG_BURST);
    dt.second = bcd2dec(readByte() & 0b01111111);
    dt.minute = bcd2dec(readByte() & 0b01111111);
    dt.hour = bcd2dec(readByte() & 0b00111111);
    dt.day = bcd2dec(readByte() & 0b00111111);
    dt.month = bcd2dec(readByte() & 0b00011111);
    dt.day_of_week = bcd2dec(readByte() & 0b00000111);
    dt.year = bcd2dec(readByte() & 0b01111111);
    end();
    return dt;
}
void Clock::setDateTime(const DateTime &dt)
{
    //     // disable write protection
    prepareWrite(REG_WP);
    writeByte(0b00000000);  // 0, false
    end();
    //     // OR call writeProtect(0)

    prepareWrite(REG_BURST);
    writeByte(dec2bcd(dt.second % 60));
    writeByte(dec2bcd(dt.minute % 60));
    writeByte(dec2bcd(dt.hour % 24));
    writeByte(dec2bcd(dt.day % 32));
    writeByte(dec2bcd(dt.month % 13));
    writeByte(dec2bcd(dt.day_of_week % 8));
    writeByte(dec2bcd(dt.year % 100));
    writeByte(0b10000000);
    end();
}

bool Clock::isHalted()
{
    prepareRead(REG_SECONDS);
    uint8_t seconds = readByte();
    end();
    return (seconds & 0b10000000);
}
void Clock::halt()
{
    prepareWrite(REG_SECONDS);
    writeByte(0b10000000);
    end();
}
void Clock::run()
{
    prepareWrite(REG_SECONDS);
    writeByte(0b00000000);
    end();
}

void Clock::disableWrite()
{
    // setting write-protect bit to 1
    prepareWrite(REG_WP);
    writeByte(1);
    end();
}

void Clock::enableWrite()
{
    // setting write-protect bit to 0
    prepareWrite(REG_WP);
    writeByte(0);
    end();
}

void Clock::prepareRead(uint8_t address)
{
    setDirection(GPIO_OUT);
    gpio_put(_pin_rst, HIGH);
    uint8_t command = 0b10000001 | address;
    writeByte(command);
    setDirection(GPIO_IN);
}

void Clock::prepareWrite(uint8_t address)
{
    setDirection(GPIO_OUT);
    gpio_put(_pin_rst, HIGH);
    uint8_t command = 0b10000000 | address;
    writeByte(command);
}

void Clock::setDirection(int direction)
{
    if (_dat_direction != direction) {
        _dat_direction = direction;
        gpio_set_dir(_pin_dat, direction);
    }
}

void Clock::end()
{
    gpio_put(_pin_rst, LOW);
}

uint8_t Clock::readByte()
{
    uint8_t byte = 0;
    for (uint8_t b = 0; b < 8; b++) {
        if (gpio_get(_pin_dat) == HIGH)
            byte |= 0x01 << b;
        nextBit();
    }
    return byte;
}
void Clock::writeByte(uint8_t value)
{
    for (uint8_t b = 0; b < 8; b++) {
        gpio_put(_pin_dat, (value & 0x01) ? HIGH : LOW);
        nextBit();
        value >>= 1;
    }
}
void Clock::nextBit()
{
    gpio_put(_pin_clk, HIGH);
    sleep_us(1);
    gpio_put(_pin_clk, LOW);
    sleep_us(1);
}

uint8_t Clock::dec2bcd(uint8_t dec)
{
    return ((dec / 10 * 16) + (dec % 10));
}
uint8_t Clock::bcd2dec(uint8_t bcd)
{
    return ((bcd / 16 * 10) + (bcd % 16));
}

void Clock::getRamByte(uint8_t addr, uint8_t *data)
{
    prepareRead(addr);
    *data = readByte();
    end();
}
void Clock::setRamByte(uint8_t addr, uint8_t *data)
{
    prepareWrite(addr);
    writeByte(*data);
    end();
}

void Clock::testRam()
{
    uint8_t testaddrSET = 0xC0;  // 0xFC;
    uint8_t testdataSET = 0x30;  // 0
    uint8_t testaddrGET = 0xC1;  // 0xFD;
    uint8_t testdataGET = 0x01;
    // get and display previously stored data (byte)
    getRamByte(testaddrGET, &testdataGET);
    uint8_t prevdata = testdataGET;
    testdataSET = prevdata + 1;
    setRamByte(testaddrSET, &testdataSET);
    getRamByte(testaddrGET, &testdataGET);
    printf("ds1302testRam prev:0x%x new:0x%x\n", prevdata, testdataGET);
}

void Clock::writeRamBulk(const uint8_t *data, int len)
{
    if (len <= 0) {
        return;
    }
    if (len > _kRamSize) {
        len = _kRamSize;
    }
    prepareWrite(REG_RAM_BURST);  // 0xFE
    for (int i = 0; i < len; i++) {
        writeByte(data[i]);
    }
    end();
}
void Clock::readRamBulk(uint8_t *data, int len)
{
    if (len <= 0) {
        return;
    }
    if (len > _kRamSize) {
        len = _kRamSize;
    }
    prepareRead(REG_RAM_BURST);  // TODO should be 0xFF ???
    for (int i = 0; i < len; i++) {
        data[i] = readByte();
    }
    end();
}
}
