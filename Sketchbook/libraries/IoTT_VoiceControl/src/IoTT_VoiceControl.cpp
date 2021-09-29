/*

TCP Access library to send LocoNet commands to an lbServer or act as lbServer
* 
* The library makes use of two topics to deal with LocoNet commands:
* 
* Broadcast command (default is lnIn) is used for regular message flow. A Loconet device within the MQTT network can send a message using the broadcast topic to send it
* to the gateway. The gateway will send it to LocoNet. 
* 
* Echo topic (default is nlEcho) is used by teh gateway to send confirmation that a received message has been sent to the gateway. This way any application can have positive confirmation that a 
* sent message was indeed sent to Loconet.

Concept and implementation by Hans Tanner. See https://youtu.be/e6NgUr4GQrk for more information
See Digitrax LocoNet PE documentation for more information

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

*/

#include <IoTT_VoiceControl.h>
#include <EmergencyStop_inferencing.h>
#include <driver/i2s.h>

#define PIN_CLK  0
#define PIN_DATA 34
#define GAIN_FACTOR 3 //3


// If your target is limited in memory remove this macro to save 10K RAM
#define EIDSP_QUANTIZE_FILTERBANK   0

txFct vcCallback = NULL;

/**
 * Define the number of slices per model window. E.g. a model window of 1000 ms
 * with slices per model window set to 4. Results in a slice size of 250 ms.
 * For more info: https://docs.edgeimpulse.com/docs/continuous-audio-sampling
 */
#define EI_CLASSIFIER_SLICES_PER_MODEL_WINDOW 3

static inference_t inference;
static bool record_ready = false;
static signed short *sampleBuffer;
static bool debug_nn = false; // Set this to true to see e.g. features generated from the raw signal
static int print_results = -(EI_CLASSIFIER_SLICES_PER_MODEL_WINDOW);

TaskHandle_t taskRecognize = NULL;

void i2sInit()
{
   i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX | I2S_MODE_PDM),
    .sample_rate =  16000,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT, // is fixed at 12bit, stereo, MSB
    .channel_format = I2S_CHANNEL_FMT_ALL_RIGHT,
    .communication_format = I2S_COMM_FORMAT_I2S,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 2,
    .dma_buf_len = 128,
   };

   i2s_pin_config_t pin_config;
   pin_config.bck_io_num   = I2S_PIN_NO_CHANGE;
   pin_config.ws_io_num    = PIN_CLK;
   pin_config.data_out_num = I2S_PIN_NO_CHANGE;
   pin_config.data_in_num  = PIN_DATA;
  
   i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
   i2s_set_pin(I2S_NUM_0, &pin_config);
   i2s_set_clk(I2S_NUM_0, 16000, I2S_BITS_PER_SAMPLE_16BIT, I2S_CHANNEL_MONO);
}

void mic_record_task (void* arg)
{   
  size_t bytesread;
  while(1){
    i2s_read(I2S_NUM_0,(char*) &sampleBuffer[0], inference.n_samples>>1, &bytesread, (100 / portTICK_RATE_MS));
    
    if (record_ready == true) 
    {
        for (int i = 0; i< bytesread >> 1; i++) {
            inference.buffers[inference.buf_select][inference.buf_count++] = sampleBuffer[i];

            if (inference.buf_count >= inference.n_samples) 
            {
//                Serial.printf("send it %i %i\n", inference.buf_count,inference.n_samples);
                inference.buf_select ^= 1;
                inference.buf_count = 0;
                inference.buf_ready = 1;
            }
        }
    }
  }
}

/**
 * Get raw audio signal data if available
 */
static int microphone_audio_signal_get_data(size_t offset, size_t length, float *out_ptr)
{
    numpy::int16_to_float(&inference.buffers[inference.buf_select ^ 1][offset], out_ptr, length);
    return 0;
}

IoTT_VoiceControl::IoTT_VoiceControl()
{
}

IoTT_VoiceControl::~IoTT_VoiceControl()
{
	vTaskDelete(taskRecognize);
	taskRecognize = NULL;
	
    free(inference.buffers[0]);
    free(inference.buffers[1]);
    free(sampleBuffer);
}

void IoTT_VoiceControl::setTxCallback(txFct newCB)
{
	vcCallback = newCB;
}

void IoTT_VoiceControl::beginKeywordRecognition()
{
	uint16_t n_samples = EI_CLASSIFIER_SLICE_SIZE;
    inference.buffers[0] = (signed short *)malloc(n_samples * sizeof(signed short));
    if (inference.buffers[0] == NULL) 
		return;
	else
    {
		inference.buffers[1] = (signed short *)malloc(n_samples * sizeof(signed short));
		if (inference.buffers[1] == NULL) 
		{
			free(inference.buffers[0]);
			return;
		}
		else
		{
			sampleBuffer = (signed short *)malloc((n_samples >> 1) * sizeof(signed short));
			if (sampleBuffer == NULL) 
			{
				free(inference.buffers[0]);
				free(inference.buffers[1]);
				return;
			}
		}
	}

    inference.buf_select = 0;
    inference.buf_count = 0;
    inference.n_samples = n_samples;
    inference.buf_ready = 0;

    i2sInit();
    run_classifier_init();

    xTaskCreate(mic_record_task, "mic_record_task", 2048, NULL, 1, &taskRecognize);
    record_ready = true;
}

void IoTT_VoiceControl::processKeywordRecognition()
{
	if (inference.buf_ready == 1)
//    if (microphone_inference_record())
    {
		inference.buf_ready = 0;
		signal_t signal;
		signal.total_length = EI_CLASSIFIER_SLICE_SIZE;
		signal.get_data = &microphone_audio_signal_get_data;
		ei_impulse_result_t result = {0};

		EI_IMPULSE_ERROR r = run_classifier_continuous(&signal, &result, debug_nn);
		if (r != EI_IMPULSE_OK) 
		{
			Serial.printf("ERR: Failed to run classifier (%d)\n", r);
			return;
		}

		if (++print_results >= (EI_CLASSIFIER_SLICES_PER_MODEL_WINDOW)) 
		{	
			for (size_t ix = 2; ix < EI_CLASSIFIER_LABEL_COUNT; ix++) 
			{
				if (result.classification[ix].value > 0.5) //only stop and go
				{
					Serial.printf("  %i  %s: %.5f\n", ix, result.classification[ix].label, result.classification[ix].value);
					lnTransmitMsg recData;
					if (vcCallback) 
						switch (ix)
						{
							case 2:	//GO
								Serial.println("Heard GO");
								if (sendGoCmd)
								{
									recData.lnData[0] = 0x83;
									recData.lnData[1] = ~recData.lnData[0];
									recData.lnMsgSize = 2;
									vcCallback(recData);
								}
								break;
							case 3:	//STOP
								Serial.println("Heard STOP");
								if (sendStopCmd)
								{
									recData.lnData[0] = 0x85;
									recData.lnData[1] = ~recData.lnData[0];
									recData.lnMsgSize = 2;
									vcCallback(recData);
								}
								break;
						}
				}
			}
			if (result.anomaly > 0.5)
				Serial.printf("    anomaly score: %.3f\n", result.anomaly);
			print_results = 0;
		}

    }
}

void IoTT_VoiceControl::loadKeywordCfgJSON(DynamicJsonDocument doc)
{
	if (doc.containsKey("UseStop"))
		sendStopCmd = doc["UseStop"];
	if (doc.containsKey("UseGo"))
		sendGoCmd = doc["UseGo"];

}
