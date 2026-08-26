// Copyright (c) 2026 - Andre Malenfant
// Use of this source code is governed by a GPLv3 license that can be found in the LICENSE file.

/*
   This class changes the level of the pin based on the Spindle state
   Usefull for automatic triggering of dust collection, etc. while using a ModBus spindle that is not controlled by a relay

*/
#include "SpindleStateOutput.h"
#include "Machine/MachineConfig.h"
#include "Spindles/Spindle.h"

void SpindleStateOutput::afterParse() {
    if (_output_pin.undefined()) {
        log_error("must define output pin");
    }
}

void SpindleStateOutput::init() {
    log_info("Spindle State Output Starting") ;

    if (_output_pin.defined()) {
        _output_pin.setAttr(Pin::Attr::Output);
    }
}

// Lazy init the demon task to make sure we are ready to ready the spindle state
// Would have loved to override ready() and use task notifications, but it is not virtual
void SpindleStateOutput::poll() {
    if (_daemon_task_handle == nullptr) {
        //xTaskNotifyGive(_daemon_task_handle);
        xTaskCreatePinnedToCore(daemonTask, "spindle_state_daemon", 2048, this, 2, &_daemon_task_handle, 1);
    }
}

void SpindleStateOutput::daemonTask(void* arg) {
    //ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    auto* self = static_cast<SpindleStateOutput*>(arg);
    while (true) {
        switch (spindle->get_state()) {
            case SpindleState::Cw:
            case SpindleState::Ccw:
                 self->_output_pin.on();
                break;
            case SpindleState::Disable:                
            case SpindleState::Unknown:
                 self->_output_pin.off();
                 break;            
        }       
#ifdef DEBUG_TASK_STACK
            static UBaseType_t uxHighWaterMark = 0;
            reportTaskStackSize(uxHighWaterMark);
#endif        
        vTaskDelay(pdMS_TO_TICKS(self->_report_interval_ms));
    }
}

SpindleStateOutput::~SpindleStateOutput() {
    vTaskDelete(_daemon_task_handle);
}

ConfigurableModuleFactory::InstanceBuilder<SpindleStateOutput> spindle_state_output_module __attribute__((init_priority(104))) ("spindle_state_output");