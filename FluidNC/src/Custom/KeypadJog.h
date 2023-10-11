#pragma once
#include <Wire.h>
#include "../Channel.h"
#include "../serial.h"

class KeypadChannel : public Channel {

public:
    KeypadChannel();

    inline size_t write(uint8_t) { return 0; }
    inline size_t write(const char* s) { return write((uint8_t*)s, ::strlen(s)); }
    inline size_t write(unsigned long n) { return write((uint8_t)n); }
    inline size_t write(long n) { return write((uint8_t)n); }
    inline size_t write(unsigned int n) { return write((uint8_t)n); }
    inline size_t write(int n) { return write((uint8_t)n); }
    inline size_t write(const uint8_t* buffer, size_t size) { return 0; }

    void handle();

    bool push(const uint8_t* data, size_t length);
    bool push(String& s);
    void pushRT(char ch);

    void flush(void);

    int rx_buffer_available() override { return 0; }

    operator bool() const { return true; };

    ~KeypadChannel();

    int read() override;
    int available() override { return _rtchar == -1 ? 0 : 1; }

private:
    // Instead of queueing realtime characters, we put them here
    // so they can be processed immediately during operations like
    // homing where GCode handling is blocked.
    int _rtchar = -1;
    static void receiveEvent(int howMany);
};
