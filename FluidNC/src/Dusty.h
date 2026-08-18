#pragma once

#include "Config.h"
#include "Module.h"
#include "Configuration/Configurable.h"
#include "Channel.h"

class Dusty : public ConfigurableModule {
    Pin _output_pin; 

public:
    Dusty(const char* name) : ConfigurableModule(name) {}

    ~Dusty();

    void init() override;
    void validate() override {}
    void afterParse() override;

    void group(Configuration::HandlerBase& handler) override {
        handler.item("report_interval_ms", _report_interval_ms, 100, 5000);
        handler.item("output_pin", _output_pin);
    }

private:
    std::string _report;
    std::string _spindle_status;
    bool _started = false;
    
    TaskHandle_t _daemon_task_handle = nullptr;
    int32_t _report_interval_ms = 500;
    int8_t _off_counter = 0;

 
    static void daemonTask(void* arg);
    void sendCommand(bool);
};
