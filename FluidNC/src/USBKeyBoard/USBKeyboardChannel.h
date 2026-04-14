// Copyright (c) 2026 Andre Malenfant
// Use of this source code is governed by a GPLv3 license that can be found in the LICENSE file.

#pragma once

#include "Config.h"
#include "Module.h"
#include "Configuration/Configurable.h"
#include "Channel.h"
#include "hid_host.h"

class USBKeyboardChannel : public Channel, public ConfigurableModule {

    int32_t _state = 0;

public:
    USBKeyboardChannel(const char* name) : Channel(name), ConfigurableModule(name) {}    

    int    rx_buffer_available() override {return 0;}
    size_t write(uint8_t data) override {return 0;}
    size_t write(const uint8_t* buffer, size_t size) override {return 0;}
    int    read(void) override {return -1;}
    int    peek(void) override {return -1;}
    int    available() override {return 0;}
    void   flush() override {}
    virtual Error pollLine(char* line) override;

    void init()  override;

    ~USBKeyboardChannel();

    // Configuration handlers:
    void validate() override {}

    void afterParse() override {}

    void group(Configuration::HandlerBase& handler) override {}    

private:           
    bool continuous = true;
    bool jogging = false;
    uint8_t step = 1;
    String steps[3] = {"0.1", "1", "10"};      
    key_event_t keyEvent;
    QueueHandle_t keyEventQueue;

    Error prepareJogCommand(char* line, String cmd);
};

