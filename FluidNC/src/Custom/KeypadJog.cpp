#include <Wire.h>
#include "KeypadJog.h"
#include "..\GCode.h"

KeypadChannel keypad;
String        buffer = "";

void KeypadChannel::receiveEvent(int howMany) {
    buffer.clear();
    while (0 < Wire1.available()) {
        char c = Wire1.read();
        if (c == 0x85) {
            keypad.pushRT(c);
            log_info("KeypadJog cancel");
        } else if (c != '\n') {
            buffer += c;
        } else {
            log_info("KeypadJog: " << buffer);
            keypad.push(buffer + '\n');            
        }
    }
}

KeypadChannel::KeypadChannel() :
    Channel("keypad") {
        allChannels.registration(this);
        Wire1.begin(8, GPIO_NUM_15, GPIO_NUM_12, 400000);
        Wire1.onReceive(KeypadChannel::receiveEvent);
    }

int KeypadChannel::read() {
    if (_rtchar == -1) {
        return -1;
    } else {
        auto ret = _rtchar;
        _rtchar  = -1;
        return ret;
    }
}

void KeypadChannel::pushRT(char ch) {
    _rtchar = ch;
}

bool KeypadChannel::push(const uint8_t* data, size_t length) {
    char c;
    while ((c = *data++) != '\0') {
        _queue.push(c);
    }
    return true;
}

bool KeypadChannel::push(String& s) {
    return push((uint8_t*)s.c_str(), s.length());
}

void KeypadChannel::handle() {}

void KeypadChannel::flush(void) {}

KeypadChannel::~KeypadChannel() {
    Wire1.end();
}
