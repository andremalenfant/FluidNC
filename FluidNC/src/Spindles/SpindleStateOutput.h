#pragma once

#include "Config.h"
#include "Module.h"
#include "Configuration/Configurable.h"
#include "Channel.h"

class SpindleStateOutput : public Module, public ConfigurableModule {
    Pin _output_pin; 

public:
    SpindleStateOutput(const char* name) : ConfigurableModule(name) {}

    ~SpindleStateOutput();
  
    void deinit() override {}
    void poll() override;

    void status_report(Channel& out) override {}
    void build_info(Channel& out) override {}
    void wifi_stats(JSONencoder& j) override {}
    bool is_radio() override { return false; }    

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
    
    TaskHandle_t _daemon_task_handle = nullptr;
    int32_t _report_interval_ms = 500;

    static void daemonTask(void* arg);
};
