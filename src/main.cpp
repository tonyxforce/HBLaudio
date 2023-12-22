#include <esp32audio.h>


#include "BluetoothA2DPSink.h"

BluetoothA2DPSink a2dp_sink;
bool is_active = true;

void avrc_metadata_callback(uint8_t id, const uint8_t *text)
{
  Serial.printf("==> AVRC metadata rsp: attribute id 0x%x, %s\n", id, text);
}

void volumeChange(int newVolume){
  Serial.println(newVolume);
}

void setup()
{
  Serial.begin(115200);

  const i2s_config_t i2s_config = {
      .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX | I2S_MODE_DAC_BUILT_IN),
      .sample_rate = 44100,                         // corrected by info from bluetooth
      .bits_per_sample = (i2s_bits_per_sample_t)16, // the DAC module will only take the 8bits from MSB
      .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
      .communication_format = (i2s_comm_format_t)I2S_COMM_FORMAT_STAND_MSB,
      .intr_alloc_flags = 0, // default interrupt priority
      .dma_buf_count = 8,
      .dma_buf_len = 64,
      .use_apll = false};

  a2dp_sink.set_i2s_config(i2s_config);

  a2dp_sink.set_avrc_metadata_callback(avrc_metadata_callback);
  a2dp_sink.set_avrc_rn_volumechange(volumeChange);
  a2dp_sink.set_avrc_rn_volumechange_completed(volumeChange);
  a2dp_sink.set_on_volumechange(volumeChange);
  a2dp_sink.set_auto_reconnect(true);
  //a2dp_sink.set_volume(1);

  a2dp_sink.start("MyMusic", true);
}



void loop()
{
  //a2dp_sink.get_connection_state() == ESP_A2D_CONNECTION_STATE_CONNECTED
  //set_volume, get_volume
  //set_avrc_rn_volumechange
  Serial.println(a2dp_sink.get_volume());
  a2dp_sink.set_volume(random(0, 127));
  delay(1000);
  

  // pause / play ever 10 seconds
  if (a2dp_sink.get_audio_state() == ESP_A2D_AUDIO_STATE_STARTED)
  {
    //delay(10000);
    //Serial.println("changing state...");
    //is_active = !is_active;
    //if (is_active)
    //{
    //  Serial.println("play");
    //  a2dp_sink.play();
    //}
    //else
    //{
    //  Serial.println("pause");
    //  a2dp_sink.pause();
    //}
  }
}