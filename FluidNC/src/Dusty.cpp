// Copyright (c) 2023 -	Bart Dring
// Use of this source code is governed by a GPLv3 license that can be found in the LICENSE file.

/*
    This is a class for status "Idle,Run,Hold,Alarm" pins.

    This can be used for Tower lights,etc.
*/
#include "Dusty.h"
#include "Machine/MachineConfig.h"
#include "Spindles/Spindle.h"

void Dusty::afterParse() {
    if (_output_pin.undefined()) {
        log_error("must define output pin");
    }
}

void Dusty::init() {
    log_info("Dusty starting") ;

    if (_output_pin.defined()) {
        _output_pin.setAttr(Pin::Attr::Output);
    }
    
    xTaskCreatePinnedToCore(daemonTask, "dusty_daemon", 2048, this, 2, &_daemon_task_handle, 1);
}

void Dusty::sendCommand(bool on) {
    _output_pin.write(on);
}

void Dusty::daemonTask(void* arg) {
    auto* self = static_cast<Dusty*>(arg);
    if (!self->_started) {
        vTaskDelay(pdMS_TO_TICKS(5000));
        self->_started = true;
    }   
    while (true) {
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


Dusty::~Dusty() {
    vTaskDelete(_daemon_task_handle);
}

ConfigurableModuleFactory::InstanceBuilder<Dusty> dusty_module __attribute__((init_priority(104))) ("dusty");