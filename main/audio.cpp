#include "driver/i2s.h"
#include <stdio.h>
#include <sys/stat.h>
#include <math.h>
#include "synth.h"
#include "mch2022_badge.h"

#include "config.h"

#define I2S_NUM ((i2s_port_t)0)

#define FRAMES_PER_BUF 1024
#define DMA_BUF_COUNT 8
#define DMA_BUF_LEN 256

static int16_t readBuffer[2*FRAMES_PER_BUF];

static const i2s_config_t i2sConfig = {
	.mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
	.sample_rate = SAMPLE_RATE,
	.bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
	.channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
	.communication_format = I2S_COMM_FORMAT_STAND_I2S,
	.intr_alloc_flags = 0, // default interrupt priority
	.dma_buf_count = DMA_BUF_COUNT,
	.dma_buf_len = DMA_BUF_LEN,
	.use_apll = false,
  .tx_desc_auto_clear = true,                         // auto clear tx descriptor on underflow
  .fixed_mclk = false,
  .mclk_multiple = I2S_MCLK_MULTIPLE_256,
  .bits_per_chan = I2S_BITS_PER_CHAN_16BIT,
};

static const i2s_pin_config_t i2sPinConfig = {
  .mck_io_num = GPIO_I2S_MCLK,
  .bck_io_num = 4, // should be GPIO_I2S_CLK
  .ws_io_num = 12, // should be GPIO_I2S_LR
  .data_out_num = GPIO_I2S_DATA,
  .data_in_num = -1      
};

void audio_init() {
	//initialize I2S
    i2s_driver_install(I2S_NUM, &i2sConfig, 0, NULL);
    i2s_set_pin(I2S_NUM, &i2sPinConfig);
    i2s_set_clk(I2S_NUM, i2sConfig.sample_rate, i2sConfig.bits_per_sample, (i2s_channel_t)2);

	setup(SAMPLE_RATE, FRAMES_PER_BUF);
}

void playSynthTask(void *param) {
	i2s_start(I2S_NUM);
	while (1) {
//		int64_t start = esp_timer_get_time();
		render2I16(readBuffer, FRAMES_PER_BUF);
//		int64_t end = esp_timer_get_time();
		uint32_t bytesWritten = 0; 
		i2s_write(I2S_NUM, readBuffer, 2 * sizeof(int16_t) * FRAMES_PER_BUF, &bytesWritten, portMAX_DELAY);
	}
}

TaskHandle_t synthTaskHandle = 0;

void playSynth() {
	xTaskCreatePinnedToCore(playSynthTask, "Synth task", 8192, NULL, 2 | portPRIVILEGE_BIT , &synthTaskHandle, 1/*tskNO_AFFINITY*/);
}

