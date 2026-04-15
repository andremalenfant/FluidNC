// Copyright (c) 2026 Andre Malenfant
// Use of this source code is governed by a GPLv3 license that can be found in the LICENSE file.

#include <sstream>
#include "hid_host.h"
#include <freertos/queue.h>

typedef struct {
  hid_host_device_handle_t hid_device_handle;
  hid_host_driver_event_t event;
  void *arg;
} hid_host_event_queue_t;

/**
 * @brief HID Protocol string names
 */
static const char *hid_proto_name_str[] = {"NONE", "KEYBOARD", "MOUSE"};

/**
 * @brief Key event
 */
typedef struct {
  enum key_state { KEY_STATE_PRESSED = 0x00, KEY_STATE_RELEASED = 0x01 } state;
  uint8_t modifier;
  uint8_t key_code;
  char key_code_hex[4];
  QueueHandle_t keyEventQueue;
} key_event_t;

/* Main char symbol for ENTER key */
#define KEYBOARD_ENTER_MAIN_CHAR '\r'
/* When set to 1 pressing ENTER will be extending with LineFeed during serial
 * debug output */
#define KEYBOARD_ENTER_LF_EXTEND 1

static QueueHandle_t hid_host_event_queue;

void hid_host_device_callback(hid_host_device_handle_t hid_device_handle, const hid_host_driver_event_t event, void *arg);
void hid_host_task(void *pvParameters);
void hid_host_device_event(hid_host_device_handle_t hid_device_handle, const hid_host_driver_event_t event, void *arg);
void hid_host_interface_callback(hid_host_device_handle_t hid_device_handle, const hid_host_interface_event_t event, void *arg);
void hid_host_keyboard_report_callback(const uint8_t *const data, const int length);
void key_event_callback(key_event_t key_event, QueueHandle_t keyEventQueue);
bool key_found(const uint8_t *const src, uint8_t key, unsigned int length);
void init_hid_host(QueueHandle_t keyEventQueue);