// Copyright (c) 2023 -	Bart Dring
// Use of this source code is governed by a GPLv3 license that can be found in the LICENSE file.

/*
    This is a class for status "Idle,Run,Hold,Alarm" pins.

    This can be used for Tower lights,etc.
*/
#include "StatusMQTT.h"
#include "Machine/MachineConfig.h"
#include "Spindles/Spindle.h"

void StatusMQTT::init() {
    log_info("MQTT" << " Server:" << _serverHost << ":" << _serverPort << " topic:" << _topic) ;

    if (_output_pin.defined()) {
        _output_pin.setAttr(Pin::Attr::Output);
    }

    _espClient = new WiFiClient();
    _client = new PubSubClient(_serverHost.c_str(), _serverPort, *_espClient);

    xTaskCreatePinnedToCore(daemonTask, "mqtt_daemon", 2048, this, 2, &_daemon_task_handle, 1);
}


void StatusMQTT::sendCommand(bool on) {
    if (!_client->connected()) {
        log_debug("MQTT" << " Connecting");
        bool connected = _client->connect("fluidnc");
        log_debug("MQTT" << " connected=" << connected);
    }     
    if (_client->connected()) {
        if (on) {
            _off_counter = 0;
        } else if (_off_counter < 4) {
            _off_counter++;
        }
        if (_off_counter < 4) {
            sprintf(_messageBuffer, _messageTemplate.c_str(), _dusty_id.c_str(), on ? "TOOL_ON" : "TOOL_OFF");
            log_debug("MQTT" << " publish" << _messageBuffer);
            _client->publish(_topic.c_str(), _messageBuffer);        
        }
        //log_verbose("MQTT" << " module size is " << (2048 - uxTaskGetStackHighWaterMark(NULL)) << " bytes");
    } 
    //if (_output_pin.defined()) {
        _output_pin.write(on);
    //}
}

void StatusMQTT::daemonTask(void* arg) {
    auto* self = static_cast<StatusMQTT*>(arg);
    if (!self->_started) {
        vTaskDelay(pdMS_TO_TICKS(5000));
        self->_started = true;
    }   
    while (true) {
        self->_client->loop();   
        switch (spindle->get_state()) {
            case SpindleState::Disable:                
                self->sendCommand(false);    
            break;
            case SpindleState::Cw:
                self->sendCommand(true);
                break;
            case SpindleState::Ccw:
                self->sendCommand(true);
                break;
            case SpindleState::Unknown:
                self->sendCommand(false);
        }       
#ifdef DEBUG_TASK_STACK
            static UBaseType_t uxHighWaterMark = 0;
            reportTaskStackSize(uxHighWaterMark);
#endif        
        vTaskDelay(pdMS_TO_TICKS(self->_report_interval_ms));
    }
}


StatusMQTT::~StatusMQTT() {
    vTaskDelete(_daemon_task_handle);
}

ConfigurableModuleFactory::InstanceBuilder<StatusMQTT> statusmqtt_module __attribute__((init_priority(104))) ("statusmqtt");