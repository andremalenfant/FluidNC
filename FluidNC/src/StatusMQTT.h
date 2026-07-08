#pragma once

#include "Config.h"
#include "Module.h"
#include "Configuration/Configurable.h"
#include "Channel.h"
#include <WiFi.h>
#include <PubSubClient.h>

class StatusMQTT : public ConfigurableModule {
    Pin _output_pin; 

public:
    StatusMQTT(const char* name) : ConfigurableModule(name) {}

    ~StatusMQTT();

    void init() override;
    void validate() override {}
    void afterParse() override {};

    std::string _serverHost;
    uint32_t _serverPort;
    std::string _topic;
    std::string _dusty_id;      
    std::string _user_id;
    std::string _password;  

    void group(Configuration::HandlerBase& handler) override {
        handler.item("report_interval_ms", _report_interval_ms, 100, 5000);
        handler.item("server_host", _serverHost);
        handler.item("server_port", _serverPort);
        handler.item("dusty_id", _dusty_id);
        handler.item("topic", _topic);
        handler.item("output_pin", _output_pin);
        handler.item("user_id", _user_id);
        handler.item("password", _password);
    }

private:
    std::string _report;
    std::string _spindle_status;
    bool _started = false;
    
    TaskHandle_t _daemon_task_handle = nullptr;
    int32_t _report_interval_ms = 500;
    int8_t _off_counter = 0;

    WiFiClient* _espClient;
    PubSubClient* _client;
 
    std::string _messageTemplate = "{\"ID\":\"%s\",\"EVENT\":\"%s\"}";
    char _messageBuffer[35];

    static void daemonTask(void* arg);
    void sendCommand(bool);
};
