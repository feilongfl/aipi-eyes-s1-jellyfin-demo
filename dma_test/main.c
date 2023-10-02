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
#include "bflb_mtd.h"
#include "easyflash.h"

void wifi_event_handler() {}

int main(void) {
  board_init();

  while (1) {
    printf("test\n");
  }
}
