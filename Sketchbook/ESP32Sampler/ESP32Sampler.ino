#include <Arduino.h>
#include "I2SMEMSSampler.h"
#include "ADCSampler.h"
#include <WiFi.h>
#include <HTTPClient.h>

WiFiClient *wifiClientADC = NULL;
HTTPClient *httpClientADC = NULL;
WiFiClient *wifiClientI2S = NULL;
HTTPClient *httpClientI2S = NULL;
ADCSampler *adcSampler = NULL;
I2SSampler *i2sSampler = NULL;

// replace this with your machines IP Address
#define ADC_SERVER_URL "http://192.168.1.72:5003/adc_samples"
#define I2S_SERVER_URL "http://192.168.1.72:5003/i2s_samples"

// i2s config for using the internal ADC
i2s_config_t adcI2SConfig = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX | I2S_MODE_ADC_BUILT_IN),
    .sample_rate = 32000,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = I2S_COMM_FORMAT_I2S_LSB,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 4,
    .dma_buf_len = 1024,
    .use_apll = false,
    .tx_desc_auto_clear = false,
    .fixed_mclk = 0};

// i2s config for reading from left channel of I2S
i2s_config_t i2sMemsConfigLeftChannel = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
    .sample_rate = 44000,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = i2s_comm_format_t(I2S_COMM_FORMAT_I2S),
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 4,
    .dma_buf_len = 1024,
    .use_apll = false,
    .tx_desc_auto_clear = false,
    .fixed_mclk = 0};

// i2s pins
i2s_pin_config_t i2sPins = {
    .bck_io_num = GPIO_NUM_32,
    .ws_io_num = GPIO_NUM_25,
    .data_out_num = I2S_PIN_NO_CHANGE,
    .data_in_num = GPIO_NUM_33};

// how many samples to read at once
const int SAMPLE_SIZE = 32; //1 ms interval

float avgAccu = 0;
uint16_t avgCtr = 0;
uint16_t avgIntv = 50; //ms

// send data to a remote address
void sendData(WiFiClient *wifiClient, HTTPClient *httpClient, const char *url, uint8_t *bytes, size_t count)
{
  uint16_t * adcData = (uint16_t*)&bytes[0];
  // send them off to the server
  float sqrVal = 0;
  for (int i = 0; i < (count>>1); i++)
    sqrVal += sq(adcData[i]);

  avgAccu += sqrVal;
  
  sqrVal = sqrt(sqrVal/(count>>1)); //prog pulse
  avgCtr++;
  if (avgCtr >= avgIntv)
  {
    digitalWrite(2, !digitalRead(2));
    Serial.printf("%i %f\n", avgCtr, sqrt(avgAccu/(avgIntv * (count>>1))));
//    digitalWrite(2, LOW);
    avgAccu = 0;
    avgCtr = 0;
  }
}

// Task to write samples from ADC to our server
void adcWriterTask(void *param)
{
  I2SSampler *sampler = (I2SSampler *)param;
  int16_t *samples = (int16_t *)malloc(sizeof(uint16_t) * SAMPLE_SIZE);
  if (!samples)
  {
    Serial.println("Failed to allocate memory for samples");
    return;
  }
  while (true)
  {
    int samples_read = sampler->read(samples, SAMPLE_SIZE);
    sendData(wifiClientADC, httpClientADC, ADC_SERVER_URL, (uint8_t *)samples, samples_read * sizeof(uint16_t));
  }
}

// Task to write samples to our server
void i2sMemsWriterTask(void *param)
{
  I2SSampler *sampler = (I2SSampler *)param;
  int16_t *samples = (int16_t *)malloc(sizeof(uint16_t) * SAMPLE_SIZE);
  if (!samples)
  {
    Serial.println("Failed to allocate memory for samples");
    return;
  }
  while (true)
  {
    int samples_read = sampler->read(samples, SAMPLE_SIZE);
    sendData(wifiClientI2S, httpClientI2S, I2S_SERVER_URL, (uint8_t *)samples, samples_read * sizeof(uint16_t));
  }
}

void setup()
{
  Serial.begin(115200);
  delay(500);
  // launch WiFi
  Serial.println("Started up");
  // indicator LED
  pinMode(2, OUTPUT);
  // setup the HTTP Client
//  wifiClientADC = new WiFiClient();
//  httpClientADC = new HTTPClient();

//  wifiClientI2S = new WiFiClient();
//  httpClientI2S = new HTTPClient();

  // input from analog microphones such as the MAX9814 or MAX4466
  // internal analog to digital converter sampling using i2s
  // create our samplers
  adcSampler = new ADCSampler(ADC_UNIT_1, ADC1_CHANNEL_7, adcI2SConfig); //35

  // set up the adc sample writer task
  TaskHandle_t adcWriterTaskHandle;
  adcSampler->start();
  xTaskCreatePinnedToCore(adcWriterTask, "ADC Writer Task", 4096, adcSampler, 1, &adcWriterTaskHandle, 1);

  // Direct i2s input from INMP441 or the SPH0645
  //i2sSampler = new I2SMEMSSampler(I2S_NUM_0, i2sPins, i2sMemsConfigLeftChannel, false);
  //i2sSampler->start();
  // set up the i2s sample writer task
  //TaskHandle_t i2sMemsWriterTaskHandle;
  //xTaskCreatePinnedToCore(i2sMemsWriterTask, "I2S Writer Task", 4096, i2sSampler, 1, &i2sMemsWriterTaskHandle, 1);

  // // start sampling from i2s device
}

void loop() {
  // put your main code here, to run repeatedly:

}
