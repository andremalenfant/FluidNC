// Copyright 2026 Andre Malenfant
// Use of this source code is governed by a GPLv3 license that can be found in the LICENSE file.

#include "usb/usb_host.h"
#include "hid_host.h"
#include "hid_usage_keyboard.h"
#include <freertos/queue.h>
#include "USBKeyboard.h"
#include "USBKeyboardChannel.h"
#include "RealtimeCmd.h"

void USBKeyboardChannel::init() {
    log_debug("USBKeyboardChannel start");
    keyEventQueue = xQueueCreate(10, sizeof(key_event_t));
    init_hid_host(keyEventQueue);
    allChannels.registration(this);
}

Error USBKeyboardChannel::pollLine(char* line) {
  if (xQueueReceive(keyEventQueue, &keyEvent, pdMS_TO_TICKS(50))) {
      std::stringstream ss;
      ss << std::hex << keyEvent.key_code; // Formats to "2a"
      std::string hexStr = ss.str();
      log_debug("received even from queue state: " << keyEvent.state << " code: " << keyEvent.key_code_hex);
      if (keyEvent.state == keyEvent.KEY_STATE_RELEASED && jogging) {
        jogging = false;  
        if (continuous) {
          log_debug("Cancelling jog");
          execute_realtime_command(Cmd::JogCancel, *this);  
        }
        return Error::NoData;
      } else {
        switch(keyEvent.key_code) {
          case HID_KEY_KEYPAD_4: //Arrow left
            return prepareJogCommand(line, continuous ? "X-1000" : "X-" + steps[step]);
          case HID_KEY_KEYPAD_6: //Arrow right
            return prepareJogCommand(line, continuous ? "X1000" : "X" + steps[step]);
          case HID_KEY_KEYPAD_8: //Arrow up
            return prepareJogCommand(line, continuous ? "Y1000" : "Y" + steps[step]);
          case HID_KEY_KEYPAD_2: //Arrow down
            return prepareJogCommand(line, continuous ? "Y-1000" : "Y-" + steps[step]);
          case HID_KEY_KEYPAD_9: //Page Up
            return prepareJogCommand(line, continuous ? "Z100" : "Z" + steps[step]);
          case HID_KEY_KEYPAD_3: //Page down
            return prepareJogCommand(line, continuous ? "Z-100" : "Z-" + steps[step]);
          case HID_KEY_KEYPAD_DIV: // /
            continuous = false;
            break;
          case HID_KEY_KEYPAD_MUL: // *
            continuous = true;
            break;
          case HID_KEY_KEYPAD_ADD: // +
            step = (step + 1 < 3) ? step + 1 : step;
            break;
          case HID_KEY_KEYPAD_SUB: // -
            step = (step - 1 >= 0) ? step - 1 : step;
            break;
        }  
      }
  }
  return Error::NoData;
}

Error USBKeyboardChannel::prepareJogCommand(char* line, String cmd) {
    jogging = true;
    cmd = "$J=G91 G21 F3000 " + cmd + "\r";
    cmd.toCharArray(line, cmd.length());
    log_debug("prepared command: " << line);
    return Error::Ok;
}

USBKeyboardChannel::~USBKeyboardChannel() {}

ConfigurableModuleFactory::InstanceBuilder<USBKeyboardChannel> usbkeyboard_module __attribute__((init_priority(104))) ("usbkeyboard");