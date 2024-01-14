#include <esp32audio.h>
#include "const.h"

BluetoothA2DPSink a2dp_sink;
Preferences preferences;
BluetoothSerial SerialBT;
Adafruit_NeoPixel pixels(NUM_PIXELS, PIN_WS2812B, NEO_RGB + NEO_KHZ800);

#include "duallogger.h"


int isConnected = 0;
int volume = 0;





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

void volumeChange(int newVolume)
{
  pixels.setBrightness(newVolume * 2);
  pixels.show();
  log("update: ");
  log(newVolume);
  log(", scaled:");
  logln(map(newVolume, 0, 127, 0, 150));
}

void setup()
{
  preferences.begin("main");
  Serial.begin(115200);

  pixels.fill(0);
  pixels.show();

  String name = preferences.getString("name", "HBL partybox 3");
  SerialBT.begin(name.c_str());
  Serial.println("The device started, now you can pair it with bluetooth as " + name);

  const i2s_config_t i2s_config = {
      .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX | I2S_MODE_DAC_BUILT_IN),
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

  a2dp_sink.start(name.c_str());
}


void loop()
{
  if (isConnected != a2dp_sink.get_connection_state())
  {
    isConnected = a2dp_sink.get_connection_state();

    switch(isConnected){
      case 0:
      logln("Disconnected!");
      break;
      
      case 1:
      logln("State change?");
      break;
      
      case 2:
      logln("Connected!");
      break;
    }
  }
  if (Serial.available())
  {
    SerialBT.write(Serial.read());
  }

  if (SerialBT.available())
  {
    byte content = SerialBT.read();
    Serial.write(content);
    if (content == 0x0A || content == 0x0D)
      return;

    switch (content)
    {

    case 's':
      pixels.setPixelColor(0, pixels.Color((uint8_t)SerialBT.parseInt(), (uint8_t)SerialBT.parseInt(), (uint8_t)SerialBT.parseInt()));
      break;

    case 'c':
      String newName = SerialBT.readString();
      logln("Changing name to " + newName);
      preferences.putString("name", newName);
      delay(1000);
      ESP.restart();
      break;
    }
    SerialBT.readString();
    pixels.show();
  }
}