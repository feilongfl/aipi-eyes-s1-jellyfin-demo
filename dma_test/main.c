/****************************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.  The
 * ASF licenses this file to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance with the
 * License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"

#include "sntp.h"
#include <lwip/netdb.h>
#include <lwip/sockets.h>
#include <lwip/tcpip.h>

#include "bl_fw_api.h"
#include "wifi_mgmr.h"
#include "wifi_mgmr_ext.h"

#include "bflb_irq.h"
#include "bflb_l1c.h"
#include "bflb_mtimer.h"
#include "bflb_uart.h"

#include "bl616_glb.h"
#include "rfparam_adapter.h"

#include "board.h"

#define DBG_TAG "MAIN"
#include "log.h"

#include "lv_conf.h"
#include "lvgl.h"

#include "lv_port_disp.h"
#include "lv_port_indev.h"

#include "lcd.h"
#include "portable.h"
// #include "fhost.h"
// #include "gui_guider.h"
#include "bflb_dma.h"
#include "bflb_mtd.h"
#include "easyflash.h"

#define WIFI_STACK_SIZE (1536)
#define TASK_PRIORITY_FW (16)
static TaskHandle_t wifi_fw_task;

static wifi_conf_t conf = {
    .country_code = "CN",
};

static uint32_t sta_ConnectStatus = 0;

extern void shell_init_with_task(struct bflb_device_s *shell);
extern int es8388_voice_init(void);
extern int audio_init(void);
extern int audio_pcm_init(void);
extern void play_voice_open(void);
extern void play_voice_close(void);
int wifi_start_firmware_task(void) {
  LOG_I("Starting wifi ...\r\n");

  /* enable wifi clock */

  GLB_PER_Clock_UnGate(GLB_AHB_CLOCK_IP_WIFI_PHY |
                       GLB_AHB_CLOCK_IP_WIFI_MAC_PHY |
                       GLB_AHB_CLOCK_IP_WIFI_PLATFORM);
  GLB_AHB_MCU_Software_Reset(GLB_AHB_MCU_SW_WIFI);

  /* set ble controller EM Size */

  GLB_Set_EM_Sel(GLB_WRAM160KB_EM0KB);

  if (0 != rfparam_init(0, NULL, 0)) {
    LOG_I("PHY RF init failed!\r\n");
    return 0;
  }

  LOG_I("PHY RF init success!\r\n");

  /* Enable wifi irq */

  extern void interrupt0_handler(void);
  bflb_irq_attach(WIFI_IRQn, (irq_callback)interrupt0_handler, NULL);
  bflb_irq_enable(WIFI_IRQn);

  xTaskCreate(wifi_main, (char *)"fw", WIFI_STACK_SIZE, NULL, TASK_PRIORITY_FW,
              &wifi_fw_task);

  return 0;
}

void wifi_event_handler(uint32_t code) {

  sta_ConnectStatus = code;
  switch (code) {
  case CODE_WIFI_ON_INIT_DONE: {
    LOG_I("[APP] [EVT] %s, CODE_WIFI_ON_INIT_DONE\r\n", __func__);
    wifi_mgmr_init(&conf);
  } break;
  case CODE_WIFI_ON_MGMR_DONE: {
    LOG_I("[APP] [EVT] %s, CODE_WIFI_ON_MGMR_DONE\r\n", __func__);
  } break;
  case CODE_WIFI_ON_SCAN_DONE: {
    LOG_I("[APP] [EVT] %s, CODE_WIFI_ON_SCAN_DONE\r\n", __func__);
    wifi_mgmr_sta_scanlist();
  } break;
  case CODE_WIFI_ON_CONNECTED: {
    LOG_I("[APP] [EVT] %s, CODE_WIFI_ON_CONNECTED\r\n", __func__);
    void mm_sec_keydump();
    mm_sec_keydump();
  } break;
  case CODE_WIFI_ON_GOT_IP: {

    LOG_I("[APP] [EVT] %s, CODE_WIFI_ON_GOT_IP\r\n", __func__);
  } break;
  case CODE_WIFI_ON_DISCONNECT: {
    LOG_I("[APP] [EVT] %s, CODE_WIFI_ON_DISCONNECT\r\n", __func__);
  } break;
  case CODE_WIFI_ON_AP_STARTED: {
    LOG_I("[APP] [EVT] %s, CODE_WIFI_ON_AP_STARTED\r\n", __func__);
  } break;
  case CODE_WIFI_ON_AP_STOPPED: {
    LOG_I("[APP] [EVT] %s, CODE_WIFI_ON_AP_STOPPED\r\n", __func__);
  } break;
  case CODE_WIFI_ON_AP_STA_ADD: {
    LOG_I("[APP] [EVT] [AP] [ADD] %lld\r\n", xTaskGetTickCount());
  } break;
  case CODE_WIFI_ON_AP_STA_DEL: {
    LOG_I("[APP] [EVT] [AP] [DEL] %lld\r\n", xTaskGetTickCount());
  } break;
  default: {
    LOG_I("[APP] [EVT] Unknown code %u \r\n", code);
  }
  }
}

#define TEST_SIZE 1000
static char test_data[TEST_SIZE];
static char test_area[TEST_SIZE];

struct bflb_device_s *dma_dev;

void dma_dev_isr(void *arg) {
  bflb_dma_channel_stop(dma_dev);
  printf("isr: %02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x\n", test_area[0],
         test_area[1], test_area[2], test_area[3], test_area[4], test_area[5],
         test_area[6], test_area[7]);
}

static void test_dma_init() {
  struct bflb_dma_channel_config_s tx_config = {
      .direction = DMA_MEMORY_TO_MEMORY,
      .src_req = DMA_REQUEST_NONE,
      .dst_req = DMA_REQUEST_NONE,
      .src_addr_inc = DMA_ADDR_INCREMENT_ENABLE,
      .dst_addr_inc = DMA_ADDR_INCREMENT_ENABLE,
      .src_burst_count = DMA_BURST_INCR1,
      .dst_burst_count = DMA_BURST_INCR1,
      .src_width = DMA_DATA_WIDTH_8BIT,
      .dst_width = DMA_DATA_WIDTH_8BIT,
  };

  printf("dma0_ch0 init\r\n");
  dma_dev = bflb_device_get_by_name("dma0_ch0");
  bflb_dma_channel_init(dma_dev, &tx_config);
  bflb_dma_channel_irq_attach(dma_dev, dma_dev_isr, NULL);
}

static void test_dma_start(char *dst, char *src, uint32_t size) {
  static struct bflb_dma_channel_lli_transfer_s tx_transfers[1];
  static struct bflb_dma_channel_lli_pool_s tx_llipool[20];

  tx_transfers[0].src_addr = (uint32_t)src;
  tx_transfers[0].dst_addr = (uint32_t)dst;
  tx_transfers[0].nbytes = size;
  bflb_dma_channel_lli_reload(dma_dev, tx_llipool, 20, tx_transfers, 1);
  bflb_dma_channel_start(dma_dev);
}

void test_task(void *param) {
  while (1) {
    // printf("DMA_C3Control: %08x\n", *(unsigned int *)0x2000c40c);
    // printf("DMA_C3Config : %08x\n", *(unsigned int *)0x2000c410);
    printf("dat: %02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x\n", test_data[0],
           test_data[1], test_data[2], test_data[3], test_data[4], test_data[5],
           test_data[6], test_data[7]);
    // printf("tst: %02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x\n", test_area[0],
    //        test_area[1], test_area[2], test_area[3], test_area[4],
    //        test_area[5], test_area[6], test_area[7]);
    test_dma_start(test_area, test_data, TEST_SIZE);
    bflb_mtimer_delay_ms(1000);
  }
}

static TaskHandle_t test_TaskHandle;
int main(void) {
  board_init();

  tcpip_init(NULL, NULL);
  wifi_start_firmware_task();

  bflb_mtimer_delay_ms(1000);
  test_dma_init();

  for (int i = 0; i < TEST_SIZE; i++) {
    test_data[i] = i + 1;
    test_area[i] = 0x55;
  }

  xTaskCreate(test_task, (char *)"lvgl", 1024, NULL, 15, &test_TaskHandle);
  vTaskStartScheduler();
  while (1)
    ;
}
