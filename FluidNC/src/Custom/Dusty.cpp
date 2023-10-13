#include <sys/time.h>
#include <Arduino.h>
#include "Dusty.h"
#include "../Main.h"  // display_init()
#include "../Logging.h"
#include "../Spindles/Spindle.h"
#include "../SpindleDatatypes.h"

#define REPEAT_INTERVAL 2000
#define REPEAT_OFF_COUNT 5

#ifdef MQTT_KEEPALIVE
#    undef MQTT_KEEPALIVE
#endif
#define MQTT_KEEPALIVE 2

String topic = "dusty";

unsigned long closeCount  = 0;

WiFiClient   wifiClient;
PubSubClient mqttclient(wifiClient);

static TaskHandle_t dustyTaskHandle = 0;

boolean connect() {
    if (!mqttclient.connected()) {
        log_debug("MQTT: Connecting");
        mqttclient.setServer("mediaman", 1883);
        if (!mqttclient.connect("fluidbee")) {
            log_debug("failed, rc=" + mqttclient.state());
        }
    }
    return mqttclient.connected();
}

void sendMessage(bool on) {
    log_debug("MQTT: Sending message");
    String data = "{\"ID\":\"0\", \"EVENT\":";
    if (on) {
        data += "\"TOOL_ON\"}";
    } else {
        data += "\"TOOL_OFF\"}";
    } 
    mqttclient.publish("dusty", data.c_str());
    log_debug("MQTT: Message sent"); //: " << data);
}

static void sense(void* pvParameters) {
    TickType_t xTaskInterval = REPEAT_INTERVAL / portTICK_PERIOD_MS;  // in ticks (typically ms)

     while (true) {
        if (connect() && spindle) {
            log_debug("MQTT: checking spindle status");
            bool toolOn = (spindle->get_state() == SpindleState::Cw || spindle->get_state() == SpindleState::Ccw);
            if (toolOn) {
                sendMessage(true);
                closeCount = REPEAT_OFF_COUNT;
            } else {
                if (closeCount > 0) {
                    sendMessage(false);
                    closeCount--;
                }
            }
        }
        mqttclient.loop();
        log_debug("MQTT: task sleeping stack:" << uxTaskGetStackHighWaterMark(NULL));
        vTaskDelay(xTaskInterval);
    }
}

void dusty_init() {
    log_info("MQTT: initialized");
    xTaskCreatePinnedToCore(sense,        // task
                            "dustyTask",  // name for task
                            2048,              // size of task stack
                            NULL,              // parameters
                            1,                 // priority
                            &dustyTaskHandle,
                            SUPPORT_TASK_CORE  // core
    );
}


