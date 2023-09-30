/**
 * @file https_client.c
 * @author your name (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2023-06-21
 *
 * @copyright Copyright (c) 2023
 *
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "custom.h"
#include <sys/socket.h>
#include <lwip/api.h>
#include <lwip/arch.h>
#include <lwip/opt.h>
#include <lwip/inet.h>
#include "mbedtls/platform.h"
#include "mbedtls/net_sockets.h"
#include "mbedtls/ssl.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/error.h"
#include "mbedtls/md5.h"
#include "mbedtls/debug.h"

#define JELLYFIN_BUFFER_SIZE 1024
static char jellyfin_buffer[JELLYFIN_BUFFER_SIZE];

extern TaskHandle_t https_Handle;
extern xQueueHandle queue;
static const char *http_request =
    "GET /QuickConnect/Initiate HTTP/1.1\r\n"
    "Host: jellyfin_test.lan\r\n"
    "User-Agent: feilong JellyfinBox\r\n"
    "X-Emby-Authorization: MediaBrowser Client=\"Jellyfin MCU\", "
    "Device=\"JellyfinBox\", DeviceId=\"" FEILONG_JELLYFIN_DEVICE_ID
    "\", Version=\"10.8.10\"\r\n"
    "accept: application/json\r\n\r\n";

static ip4_addr_t *jellyfin_gethost() {
  static ip4_addr_t dns_ip = {0};
  netconn_gethostbyname(FEILONG_JELLYFIN_SERVER, &dns_ip);
  char *host_ip = ip_ntoa(&dns_ip);
  printf("host name : %s , host_ip : %s\n", FEILONG_JELLYFIN_SERVER, host_ip);
  return &dns_ip;
}

static int jellyfin_get(char *resp, const char *api, const char *req) {
  static char url[100];
  //   ip4_addr_t *jellyfin_host = jellyfin_gethost();
  //   static ip4_addr_t dns_ip = {0};
  //   netconn_gethostbyname(FEILONG_JELLYFIN_SERVER, &dns_ip);
  //   char *host_ip = ip_ntoa(&dns_ip);
  //   printf("host name : %s , host_ip : %s\n", FEILONG_JELLYFIN_SERVER,
  //   host_ip);

  //   sprintf(url, "http://" FEILONG_JELLYFIN_SERVER ":8096%s", api);
  //   printf("jellyfin_get: url: %s\n", url);

  int sock = -1;

  sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock < 0) {
    printf("jellyfin_get: Socket error\n");
    return -1;
  }

  struct sockaddr_in client_addr;
  client_addr.sin_family = AF_INET;
  client_addr.sin_port = 8096;
  client_addr.sin_addr.s_addr = inet_addr("192.168.10.110");
  memset(&(client_addr.sin_zero), 0, sizeof(client_addr.sin_zero));

  printf("jellyfin_get: connect...\n");
  if (tcp_connect(sock, (struct sockaddr *)&client_addr,
                  sizeof(struct sockaddr)) == -1) {
    printf("Connect failed!\n");
    closesocket(sock);
    return -1;
  }

  printf("jellyfin_get: write...\n");
  write(sock, req, sizeof(req));
  printf("jellyfin_get: recv...\n");
  int ret = recv(sock, resp, JELLYFIN_BUFFER_SIZE, 0);
  closesocket(sock);
  return ret;
}

// curl 'http://jellyfin_test.lan/QuickConnect/Initiate'
// -H 'User-Agent: feilong JellyfinBox'
// -H 'X-Emby-Authorization: MediaBrowser Client="Jellyfin MCU",
// Device="JellyfinBox",
// DeviceId="TW96aWxsYS81LjAgKFgxMTsgTGludXggeDg2XzY0KSBBcHBsZVdlYktpdC81MzcuMzYgKEtIVE1MLCBsaWtlIEdlY2tvKSBDaHJvbWUvMTE2LjAuMC4wIFNhZmFyaS81MzcuMzZ8MTY5NDg2MTkwNDQ1NQ11",
// Version="10.8.10"' -H 'accept: application/json'
void jellyfin_quick_login(void *arg) {
  // get quick login code
  int ret =
      jellyfin_get(jellyfin_buffer, "/QuickConnect/Initiate", http_request);
  if (ret < 0) {
    printf("Jellyfin Get: recv err: %d\n", ret);
  }
  printf("Jellyfin Get: recv: %s\n", jellyfin_buffer);

  while (1) {
    // check login status
    vTaskDelay(1);
  }
}
