#pragma once

#include <Arduino.h>

#include <HardwareSerial.h> // ensure we have the correct "Serial" on new MCUs (depends on ARDUINO_USB_MODE and ARDUINO_USB_CDC_ON_BOOT)

#include <WiFi.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

#include <BluetoothA2DPSink.h>
#include <Preferences.h>
#include "BluetoothSerial.h"
#include <Adafruit_NeoPixel.h>

#include "secrets.h"