#include "FreeRTOS.h"
#include "custom.h"
#include "queue.h"
#include "task.h"
#include <lwip/api.h>
#include <lwip/arch.h>
#include <lwip/inet.h>
#include <lwip/opt.h>
#include <lwip/sockets.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern TaskHandle_t https_Handle;
extern xQueueHandle queue;

// #define BUFFER_SIZE 1024
// char *buffer;
struct netbuf *buffer;

static int http_get() {
  int sock = -1, rece;
  struct sockaddr_in client_addr;
  char *host_ip;

#ifdef FEILONG_JELLYFIN_SERVER_USE_DNS
  ip4_addr_t dns_ip;
  netconn_gethostbyname(FEILONG_JELLYFIN_SERVER_ADDR, &dns_ip);
  host_ip = ip_ntoa(&dns_ip);
  PRINT_DEBUG("host name : %s , host_ip : %s\r\n", FEILONG_JELLYFIN_SERVER_ADDR,
              host_ip);
#else
  host_ip = FEILONG_JELLYFIN_SERVER_ADDR;
#endif

  struct netconn *nc;
  ip_addr_t remote_ip;
  err_t err;

  ip4addr_aton(host_ip, &remote_ip);
  nc = netconn_new(NETCONN_TCP);
  volatile err_t res;
  res = netconn_connect(nc, &remote_ip, 8096);
  if (res != ERR_OK) {
    printf("Connect failed %d!\r\n", res);
    return res;
  }
  printf("Connected!\r\n");

  const char *http_request =
      "GET /QuickConnect/Initiate HTTP/1.1\r\n"
      "Host: 192.168.10.100:8096\r\n"
      "User-Agent: feilong JellyfinBox\r\n"
      "X-Emby-Authorization: MediaBrowser Client=\"Jellyfin MCU\", "
      "Device=\"JellyfinBox\", "
      "DeviceId="
      "\"TW96aWxsYS81LjAgKFgxMTsgTGludXggeDg2XzY0KSBBcHBsZVdlYktpdC81MzcuMzYgKE"
      "tIVE1MLCBsaWtlIEdlY2tvKSBDaHJvbWUvMTE2LjAuMC4wIFNhZmFyaS81MzcuMzZ8MTY5ND"
      "g2MTkwNDQ1NQ11\", Version=\"10.8.10\"\r\n"
      "accept: application/json\r\n"
      "\r\n";

  if (netconn_write(nc, http_request, strlen(http_request), NETCONN_COPY) !=
      ERR_OK) {
    printf("netconn_write failed %d!\r\n", res);
    return res;
  }
  printf("netconn_write!\r\n");

  err = netconn_recv(nc, &buffer);
  if (err != ERR_OK) {
    printf("netconn_recv failed %d!\r\n", res);
  }

  char *buf;
  u16_t buflen;

  netbuf_data(buffer, (void **)&buf, &buflen);
  if (buflen > 0) {
    printf("netconn_recv len: %d\r\n", buflen);
    printf("netconn_recv p: %s\r\n", buf);
  }

  if (buffer != NULL) {
    netbuf_delete(buffer);
  }
}

void https_jellyfin_task(void *arg) {
  http_get();
  while (1) {
    vTaskDelay(1000);
  }
}