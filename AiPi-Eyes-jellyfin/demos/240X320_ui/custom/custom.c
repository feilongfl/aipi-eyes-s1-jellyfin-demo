// SPDX-License-Identifier: MIT
// Copyright 2020 NXP

/**
 * @file custom.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "custom.h"
#include "FreeRTOS.h"
#include "bflb_timestamp.h"
#include "cJSON.h"
#include "easyflash.h"
#include "https_client.h"
#include "lvgl.h"
#include "queue.h"
#include "sntp.h"
#include "task.h"
#include "timers.h"
#include <stdio.h>
/*********************
 *      DEFINES
 *********************/
#define UTC_CHINA 8
uint8_t wifi_connect(char* ssid, char* passwd);
TaskHandle_t https_Handle;

void sntp_set_time(uint32_t sntp_time, uint32_t fac);
/**********************
 *      TYPEDEFS
 **********************/
xQueueHandle queue;

/**********************
 *  STATIC PROTOTYPES
 **********************/

static int parse_config_quick_login(char *json_data) { return 0; }

static int (*parse_config_table[QUEUE_CONFIG_MAX])(char *json_data) = {
    [QUEUE_CONFIG_QUICKLOGIN] = parse_config_quick_login,
};

static int parse_config(char *json_data) {
  struct queue_config_header *header = (struct queue_config_header *)json_data;
  if (header->type >= QUEUE_CONFIG_MAX)
    return 0;

  return parse_config_table[header->type];
}

/**
 * @brief 解析json 类型
 *
 * @param json_data
 * @return int
 */
static int cjson__analysis_type(char *json_data) {

  cJSON *root = cJSON_Parse(json_data);

  if (root == NULL) {
    printf("[%s] is not json\r\n", __func__);

    return parse_config(json_data);
  }
  cJSON *wifi = cJSON_GetObjectItem(root, "WiFi");
  if (wifi) {
    cJSON_Delete(root);
    return 1;
  }
  cJSON *ip = cJSON_GetObjectItem(root, "ip");
  if (ip) {
    cJSON_Delete(root);
    return 2;
  }

  cJSON_Delete(root);
  return 0;
}

/**
 * @brief cjson_analysis_ssid
 *
 * @param json_data
 * @return char*
*/
static char* cjson_analysis_ssid(char* json_data)
{
    static char* ssid_str;
    ssid_str = pvPortMalloc(32);
    memset(ssid_str, 0, 32);
    cJSON* root = cJSON_Parse(json_data);
    if (root==NULL) {
        printf("[%s] is not json\r\n");
        return NULL;
    }
    cJSON* wifi = cJSON_GetObjectItem(root, "WiFi");
    if (wifi==NULL)return NULL;
    cJSON* ssid = cJSON_GetObjectItem(wifi, "ssid");
    memcpy(ssid_str, ssid->valuestring, sizeof(ssid->valuestring)*8);
    cJSON_Delete(root);
    return ssid_str;
}
/**
 * @brief cjson_analysis_password
 *
 * @param json_data
 * @return char*
*/
static char* cjson_analysis_password(char* json_data)
{
    static char* pass_str;
    pass_str = pvPortMalloc(32);
    memset(pass_str, 0, 32);
    cJSON* root = cJSON_Parse(json_data);
    if (root==NULL) {
        printf("[%s] is not json\r\n");
        return NULL;
    }
    cJSON* wifi = cJSON_GetObjectItem(root, "WiFi");
    if (wifi==NULL)return NULL;
    cJSON* password = cJSON_GetObjectItem(wifi, "password");
    memcpy(pass_str, password->valuestring, sizeof(password->valuestring)*8);
    cJSON_Delete(root);
    return pass_str;
}
/**
 * @brief cjson__analysis_ip
 *      解析 IP地址
 * @param cjson_data
 * @return char*
*/
static char* cjson__analysis_ip(char* cjson_data)
{
    static char* IP_str;

    IP_str = pvPortMalloc(64);
    memset(IP_str, 0, 64);

    cJSON* root = cJSON_Parse(cjson_data);
    if (root==NULL) {
        printf("[%s] is not json\r\n", __func__);
        return NULL;
    }
    cJSON* ip_s = cJSON_GetObjectItem(root, "ip");
    cJSON* ip = cJSON_GetObjectItem(ip_s, "IP");
    memcpy(IP_str, ip->valuestring, sizeof(ip->valuestring)*16);
    cJSON_Delete(root);
    return IP_str;
}
char* compare_wea_output_img_100x100(const char* weather_data)
{}
char* compare_wea_output_img_20x20(const char* weather_data)
{}
/**
 * @brief  void queue_task(void* arg)
 * 消息队列循环读取
 * @param arg
 */
static void queue_task(void* arg)
{
    char* queue_buff = NULL;
    char* ssid = NULL;
    char* password = NULL;
    char *ipv4_addr = NULL;
    ssid = flash_get_data(SSID_KEY, 32);
    password = flash_get_data(PASS_KEY, 32);
    if (ssid!=NULL && strlen(ssid)>0)
    {
        printf("read flash ssid:%s password:%s\r\n", ssid, password);
        wifi_connect(ssid, password);
    }
    else {
        printf("use wifi setting in firmware, ssid:%s\r\n", FEILONG_WIFI_SSID);
        wifi_connect(FEILONG_WIFI_SSID, FEILONG_WIFI_PASS);
    }

    while (1)
    {

        queue_buff = pvPortMalloc(1024*2);
        memset(queue_buff, 0, 1024*2);

        xQueueReceive(queue, queue_buff, portMAX_DELAY);

        switch (cjson__analysis_type(queue_buff))
        {
            case 1:
                ssid = cjson_analysis_ssid(queue_buff);
                password = cjson_analysis_password(queue_buff);
                printf("[%s]:ssid=%s password:%s\r\n", __func__, ssid, password);
                wifi_connect(ssid, password);
                break;
                //接收ip地址
            case 2:
                ipv4_addr = cjson__analysis_ip(queue_buff);
                printf("[%s] ipv4 addr=%s\r\n", __func__, ipv4_addr);
                memset(queue_buff, 0, 1024*2);
                sprintf(queue_buff, "IP:%s", ipv4_addr);

                //识别当前界面

                vPortFree(ssid);
                vPortFree(password);
                vPortFree(ipv4_addr);
                if (https_Handle!=NULL) {
                    vTaskDelete(https_Handle);
                }
                xTaskCreate(https_jellyfin_task, "https task", 1024*10, NULL, 3, &https_Handle);
                break;
                //接收天气情况
            case 3:

                break;
            default:
                break;
        }

        vPortFree(queue_buff);
    }
}
/**********************
 *  STATIC VARIABLES
 **********************/

 /**
  * Create a demo application
  */

void custom_init() {
  quickLogin();
  /* Add your codes here */
  queue = xQueueCreate(1, 1024 * 2);
  xTaskCreate(queue_task, "queue task", 1024 * 6, NULL, 2, NULL);
}
/**
 * @brief 设置时间
 *
 * @param sntp_time
 * @param fac
*/
void sntp_set_time(uint32_t sntp_time, uint32_t fac)
{
    uint32_t stamp;

    stamp = sntp_time - 2208988800;

    bflb_timestamp_t time_s;

    bflb_timestamp_utc2time(stamp, &time_s);

    if (https_Handle!=NULL)
      vTaskResume(https_Handle);
}
/**
 * @brief
 *
 * @param key
 * @param value
*/
void flash_erase_set(char* key, char* value)
{
    size_t len = 0;
    int value_len = strlen(value);
    ef_set_and_save_env(key, value);
    // bflb_flash_read(key, flash_data, strlen(value));
    // printf("writer data:%s\r\n", flash_data);
    memset(value, 0, strlen(value));
    ef_get_env_blob(key, value, value_len, &len);
}
/**
 * @brief
 *
 * @param key
 * @return char*
*/
char* flash_get_data(char* key, uint32_t len)
{
    static char* flash_data;
    flash_data = pvPortMalloc(len);
    memset(flash_data, 0, len);

    ef_get_env_blob(key, flash_data, len, (size_t)&len);

    return flash_data;
}