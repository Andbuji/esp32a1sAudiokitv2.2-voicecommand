#include "I2S.h"
#define SAMPLE_RATE (16000)
#define PIN_I2S_BCLK 27
#define PIN_I2S_LRC 25
#define PIN_I2S_DIN 35
#define PIN_I2S_DOUT 26

// This I2S specification :
//  -   LRC high is channel 2 (right).
//  -   LRC signal transitions once each word.
//  -   DATA is valid on the CLOCK rising edge.
//  -   Data bits are MSB first.
//  -   DATA bits are left-aligned with respect to LRC edge.
//  -   DATA bits are right-shifted by one with respect to LRC edges.
I2S::I2S(MicType micType)
{
  if (micType == M5GO || micType == M5STACKFIRE)
  {
    BITS_PER_SAMPLE = I2S_BITS_PER_SAMPLE_16BIT;
    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX | I2S_MODE_TX | I2S_MODE_DAC_BUILT_IN | I2S_MODE_ADC_BUILT_IN),
        .sample_rate = SAMPLE_RATE,
        .bits_per_sample = BITS_PER_SAMPLE,
        .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
        .communication_format = (i2s_comm_format_t)(I2S_COMM_FORMAT_I2S_MSB),
        .intr_alloc_flags = 0,
        .dma_buf_count = 2,
        .dma_buf_len = 1024};
    i2s_driver_install((i2s_port_t)I2S_NUM_0, &i2s_config, 0, NULL);
    i2s_set_adc_mode(ADC_UNIT_1, ADC1_CHANNEL_6);
    i2s_set_clk((i2s_port_t)I2S_NUM_0, SAMPLE_RATE, BITS_PER_SAMPLE, I2S_CHANNEL_STEREO);
    i2s_adc_enable((i2s_port_t)I2S_NUM_0);
  }
  else if (micType == ADMP441 || micType == ICS43434)
  {
  }
}

int I2S::Read(char *data, int numData)
{
  TickType_t ticks_to_wait = portMAX_DELAY;
  size_t bytes_read = 0;
  return i2s_read((i2s_port_t)I2S_NUM_0, (char *)data, numData, &bytes_read, ticks_to_wait);
}

int I2S::Write(char *data, int numData)
{
  TickType_t ticks_to_wait = portMAX_DELAY;
  size_t bytes_written = 0;
  return i2s_write((i2s_port_t)I2S_NUM_0, (char *)data, numData, &bytes_written, ticks_to_wait);
  
}

int I2S::GetBitPerSample()
{
  return (int)BITS_PER_SAMPLE;
}
