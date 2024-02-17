#include <Arduino.h>

#include <HardwareSerial.h> // ensure we have the correct "Serial" on new MCUs (depends on ARDUINO_USB_MODE and ARDUINO_USB_CDC_ON_BOOT)

#include <WiFi.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

#include <BluetoothA2DPSink.h>
#include <Preferences.h>
#include "BluetoothSerial.h"

BluetoothA2DPSink a2dp_sink;
Preferences preferences;
BluetoothSerial SerialBT;
// Adafruit_NeoPixel pixels(NUM_PIXELS, PIN_WS2812B, NEO_RGB + NEO_KHZ800);

#include "duallogger.h"

int isConnected = 0;
int volume = 0;

uint8_t r;
uint8_t g;
uint8_t b;

#define RXD2 16
#define TXD2 17

void avrc_metadata_callback(uint8_t id, const uint8_t *text)
{
  switch (id)
  {
  case 1:
    // title
    log("{\"");           //{"
    log("title\": ");     // title":
    logf("\"%s\"", text); //"asd"
    log(",");             //,
    break;
  case 2:
    // artist
    log("\"artist\": ");  //"artist":
    logf("\"%s\"", text); //"asd"
    log(",");             //,

    break;
  case 4:
    // album
    log("\"album\": ");   //"album":
    logf("\"%s\"", text); //"asd"
    // Serial.print(",");
    logln("}"); //}

    break;
  }
}

#define pairBtn 15
unsigned long lastSend = 0;

void volumeChange(int newVolume)
{
  while(millis() - lastSend < 10) yield();
  lastSend = millis();
  // pixels.setBrightness(newVolume * 2);
  // pixels.show();
  log("update: ");
  log(newVolume);
  log(", scaled:");
  logln(map(newVolume, 0, 127, 0, 150));
  if (abs(newVolume - volume) > 10 || newVolume > 120)
  {
    Serial2.println("6" + String(newVolume));
  }
  volume = newVolume;
}

void setup()
{
  preferences.begin("main");
  Serial.begin(115200);
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);
  Serial.setTimeout(150);

  pinMode(pairBtn, INPUT_PULLUP);

  // pixels.fill(0);
  // pixels.show();

  String name = preferences.getString("name", "HBL partybox 3");
  SerialBT.begin(name.c_str());
  Serial.println("The device started, now you can pair it with bluetooth as " + name);

  const i2s_config_t i2s_config = {
      .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
      .sample_rate = 44100,                         // corrected by info from bluetooth
      .bits_per_sample = (i2s_bits_per_sample_t)16, // the DAC module will only take the 8bits from MSB
      .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
      .communication_format = (i2s_comm_format_t)I2S_COMM_FORMAT_STAND_MSB,
      .intr_alloc_flags = 0, // default interrupt priority
      .dma_buf_count = 8,
      .dma_buf_len = 64,
      .use_apll = false,
      .tx_desc_auto_clear = false,
      .fixed_mclk = false,
  };

  const i2s_pin_config_t my_pin_config = {
      .mck_io_num = I2S_PIN_NO_CHANGE,
      .bck_io_num = 26,
      .ws_io_num = 25,
      .data_out_num = 22,
      .data_in_num = I2S_PIN_NO_CHANGE};

  a2dp_sink.set_pin_config(my_pin_config);
  a2dp_sink.set_i2s_config(i2s_config);

  a2dp_sink.set_avrc_metadata_callback(avrc_metadata_callback);
  a2dp_sink.set_avrc_rn_volumechange(volumeChange);
  a2dp_sink.set_avrc_rn_volumechange_completed(volumeChange);
  a2dp_sink.set_on_volumechange(volumeChange);
  a2dp_sink.set_auto_reconnect(true);

  // ESP_BT_NON_DISCOVERABLE,            //Non-discoverable
  // ESP_BT_LIMITED_DISCOVERABLE,        //Limited Discoverable
  // ESP_BT_GENERAL_DISCOVERABLE,        //General Discoverable
  a2dp_sink.set_discoverability(ESP_BT_LIMITED_DISCOVERABLE);

  a2dp_sink.start(name.c_str());
}

unsigned long lastBlink = 0;
unsigned long now = 0;
bool statusLedState = 0;
bool isDiscoverable = 0;
unsigned long pairStart = 0;
String newName;

void loop()
{
  now = millis();

  if (!digitalRead(pairBtn) > isDiscoverable)
  {
    isDiscoverable = !digitalRead(pairBtn);
    a2dp_sink.set_discoverability(ESP_BT_GENERAL_DISCOVERABLE);
    pairStart = now;
    Serial2.println("12");
    Serial.println("Pairing!");
  }

  if (now - pairStart > 15000 && isDiscoverable)
  {
    isDiscoverable = false;
    Serial.println("Stopped pairing!");
  }

  if (isConnected != a2dp_sink.get_connection_state())
  {
    isConnected = a2dp_sink.get_connection_state();

    switch (isConnected)
    {
    case ESP_A2D_CONNECTION_STATE_DISCONNECTED:
      logln("Disconnected!");
      Serial2.println("9");
      statusLedState = 0;
      break;

    case ESP_A2D_CONNECTION_STATE_CONNECTING:
      Serial2.println("4");

      logln("Connecting");
      break;

    case ESP_A2D_CONNECTION_STATE_CONNECTED:
      Serial2.println("8");

      a2dp_sink.set_discoverability(ESP_BT_NON_DISCOVERABLE);

      logln("Connected!");
      statusLedState = 0;
      break;

    case ESP_A2D_CONNECTION_STATE_DISCONNECTING:
      Serial2.println("7");
      break;
    }
  }
  if (Serial.available())
  {
    SerialBT.write(Serial.read());
  }

  if (SerialBT.available())
  {
    Serial2.print("5");

    while (SerialBT.available())
    {
      byte content = SerialBT.read();
      Serial2.write(content);
      Serial.write(content);
      if (content == 0x0A || content == 0x0D)
        return;

      switch (content)
      {
      case 'c':
        newName = SerialBT.readStringUntil('\n');
        logln("Changing name to " + newName);
        preferences.putString("name", newName);
        delay(1000);
        ESP.restart();
        break;
      case 's':
        String input = SerialBT.readStringUntil('\n');
        Serial2.println(input);
        logln(input);
        break;
      }

      SerialBT.readString();
    }

    Serial2.println();
  }
}