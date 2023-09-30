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

#define __STR(x) #x
#define STR(x) __STR(x)

extern TaskHandle_t https_Handle;
extern xQueueHandle queue;

typedef enum {
  HTTP_POST,
  HTTP_GET,
  HTTP_UNKNOWN,
} HtmlRequestMethod_TypeDef;

struct HttpReq {
  HtmlRequestMethod_TypeDef type;
  const char *url;
};

static void genHttpReq(char *reqstr, const struct HttpReq *req) {
  sprintf(reqstr,
          "GET %s HTTP/1.1\r\n"
          "Host: " FEILONG_JELLYFIN_SERVER_ADDR
          ":" STR(FEILONG_JELLYFIN_SERVER_PORT) "\r\n"
                                                "User-Agent:"
                                                " " FEILONG_JELLYFIN_USER_AGENT
                                                "\r\n"
                                                "X-Emby-Authorization:"
                                                " " FEILONG_JELLYFIN_X_EMBY
                                                "\r\n"
                                                "accept: application/json\r\n"
                                                "\r\n",
          req->url);
}

enum JELLYFIN_REQ {
  JELLYFIN_REQ_QuickConnect_Initiate,

  JELLYFIN_REQ_MAX
};

static void JELLYFIN_REQ_QuickConnect_Initiate_cb(char *resp, u16_t len) {
  printf("JELLYFIN_REQ_QuickConnect_Initiate_cb[%d]: %s\r\n", len, resp);
}

static const struct {
  const struct HttpReq req;
  void (*cb)(char *, u16_t);
} JellfinTable[JELLYFIN_REQ_MAX] = {
    [JELLYFIN_REQ_QuickConnect_Initiate] =
        {.cb = JELLYFIN_REQ_QuickConnect_Initiate_cb,
         .req =
             {
                 .type = HTTP_GET,
                 .url = "/QuickConnect/Initiate",
             }},
};

static int http_get(enum JELLYFIN_REQ jreq) {
  static char *host_ip = NULL;
  static char req[1024];
  static struct netbuf *buffer;
  struct netconn *conn;
  static ip_addr_t remote_ip;
  volatile err_t res;

  if (!host_ip) {
#ifdef FEILONG_JELLYFIN_SERVER_USE_DNS
    ip4_addr_t dns_ip;
    netconn_gethostbyname(FEILONG_JELLYFIN_SERVER_ADDR, &dns_ip);
    host_ip = ip_ntoa(&dns_ip);
    PRINT_DEBUG("host name : %s , host_ip : %s\r\n",
                FEILONG_JELLYFIN_SERVER_ADDR, host_ip);
#else
    host_ip = FEILONG_JELLYFIN_SERVER_ADDR;
#endif
    ip4addr_aton(host_ip, &remote_ip);
  }

  conn = netconn_new(NETCONN_TCP);
  res = netconn_connect(conn, &remote_ip, FEILONG_JELLYFIN_SERVER_PORT);
  if (res != ERR_OK) {
    printf("http_get failed %d!\r\n", res);
    return res;
  }
  printf("Connected!\r\n");

  genHttpReq(req, &JellfinTable[jreq].req);
  if (netconn_write(conn, req, strlen(req), NETCONN_COPY) != ERR_OK) {
    printf("netconn_write failed %d!\r\n", res);
    return res;
  }
  printf("netconn_write!\r\n");

  res = netconn_recv(conn, &buffer);
  if (res != ERR_OK) {
    printf("netconn_recv failed %d!\r\n", res);
  }

  char *buf;
  u16_t buflen;

  netbuf_data(buffer, (void **)&buf, &buflen);
  if (buflen > 0) {
    // printf("netconn_recv len: %d\r\n", buflen);
    // printf("netconn_recv p: %s\r\n", buf);
    JellfinTable[jreq].cb(buf, buflen);
  }

  if (buffer != NULL) {
    netbuf_delete(buffer);
  }

  return ERR_OK;
}

void https_jellyfin_task(void *arg) {
  http_get(JELLYFIN_REQ_QuickConnect_Initiate);
  while (1) {
    vTaskDelay(1000);
  }
}