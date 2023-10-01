// SPDX-License-Identifier: MIT
// Copyright 2020 NXP

/*
 * custom.h
 *
 *  Created on: July 29, 2020
 *      Author: nxf53801
 */

#ifndef __CUSTOM_H_
#define __CUSTOM_H_
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

// #include "gui_guider.h"
#define SSID_KEY "SSID"
#define PASS_KEY "PASS"

struct queue_config_header {
  unsigned char type;
};

struct queue_config_quick_login {
  struct queue_config_header header;

  unsigned char auth;
  char *code;
};

enum queue_config_index {
  QUEUE_CONFIG_QUICKLOGIN,

  QUEUE_CONFIG_MAX
};

void custom_init(void);
void flash_erase_set(char* key, char* value);
char* flash_get_data(char* key, uint32_t len);
#ifdef __cplusplus
}
#endif
#endif /* EVENT_CB_H_ */
