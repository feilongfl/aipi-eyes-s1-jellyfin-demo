#include "FreeRTOS.h"
#include "cJSON.h"
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
  void (*url_fix)(char *newurl, char *url);
};

static struct {
  char code[7];
  char secret[65];
} JellyfinData;

static void genHttpReq(char *reqstr, const struct HttpReq *req) {
  static char url_fix_buffer[256];
  char *url = req->url;
  if (req->url_fix != NULL) {
    req->url_fix(url_fix_buffer, req->url);
    url = url_fix_buffer;
  }

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
          url);
}

enum JELLYFIN_REQ {
  JELLYFIN_REQ_QuickConnect_Initiate,
  JELLYFIN_REQ_QuickConnect_Connect,

  JELLYFIN_REQ_MAX
};

static void JELLYFIN_REQ_debug_cb(cJSON *jsonObject) {
  char *jsonString = cJSON_Print(jsonObject);
  printf("JELLYFIN_REQ_debug_cb:\r\n%s\r\n", jsonString);

  free(jsonString);
}

static void JELLYFIN_REQ_QuickConnect_Initiate_cb(cJSON *jsonObject) {
  cJSON *secret = cJSON_GetObjectItem(jsonObject, "Secret");
  if (secret == NULL) {
    printf("secret is NULL");
    return;
  }
  memcpy(JellyfinData.secret, secret->valuestring, 64);
  // cJSON_Delete(secret);
  JellyfinData.secret[64] = 0;
  printf("JELLYFIN_REQ_QuickConnect_Initiate_cb: Secret: %s\r\n",
         JellyfinData.secret);

  cJSON *qcode = cJSON_GetObjectItem(jsonObject, "Code");
  if (qcode == NULL) {
    printf("qcode is NULL");
    return;
  }
  memcpy(JellyfinData.code, qcode->valuestring, 6);
  // cJSON_Delete(qcode);
  JellyfinData.code[6] = 0;
  printf("JELLYFIN_REQ_QuickConnect_Initiate_cb: qcode: %s\r\n",
         JellyfinData.code);

  cJSON *auth = cJSON_GetObjectItem(jsonObject, "Authenticated");
  if (auth == NULL) {
    printf("auth is NULL");
    return;
  }
  printf("JELLYFIN_REQ_QuickConnect_Initiate_cb: auth: %d: %d\r\n", auth->type,
         auth->valueint);
  // cJSON_Delete(auth);
}

static void JELLYFIN_REQ_QuickConnect_Connect_url_fix(char *newurl, char *url) {
  sprintf(newurl, "%s?Secret=%s", url, JellyfinData.secret);
}

static const struct {
  const struct HttpReq req;
  void (*cb)(cJSON *jsonObject);
} JellfinTable[JELLYFIN_REQ_MAX] = {
    [JELLYFIN_REQ_QuickConnect_Initiate] =
        {.cb = JELLYFIN_REQ_QuickConnect_Initiate_cb,
         .req =
             {
                 .type = HTTP_GET,
                 .url = "/QuickConnect/Initiate",
             }},
    [JELLYFIN_REQ_QuickConnect_Connect] =
        {.cb = JELLYFIN_REQ_QuickConnect_Initiate_cb,
         .req =
             {
                 .type = HTTP_GET,
                 .url_fix = JELLYFIN_REQ_QuickConnect_Connect_url_fix,
                 .url = "/QuickConnect/Connect",
             }},
};

static err_t http_resp_parse(enum JELLYFIN_REQ jreq, char *resp, u16_t len) {
  err_t ret = ERR_OK;

  if (JellfinTable[jreq].cb == NULL)
    return ret; // ignore process

  const char *jsonStart = strstr(resp, "{");
  if (!jsonStart)
    return ERR_CONN;

  cJSON *jsonObject = cJSON_Parse(jsonStart);
  if (!jsonObject)
    return ERR_CONN;

  JellfinTable[jreq].cb(jsonObject);

  cJSON_Delete(jsonObject);
  return ret;
}

static int http_get(enum JELLYFIN_REQ jreq) {
  static char *host_ip = NULL;
  static char req[1024];
  static struct netbuf *buffer;
  static struct netconn *conn;
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

  if (conn == NULL) {
    conn = netconn_new(NETCONN_TCP);
    res = netconn_connect(conn, &remote_ip, FEILONG_JELLYFIN_SERVER_PORT);
    if (res != ERR_OK) {
      printf("http_get failed %d!\r\n", res);
      return res;
    }
    printf("Connected!\r\n");
  }

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
    http_resp_parse(jreq, buf, buflen);
  }

  if (buffer != NULL) {
    netbuf_delete(buffer);
  }

  // netconn_disconnect(conn);
  // netconn_close(conn);

  return ERR_OK;
}

// static cJSON_Hooks cjson_hook = {
//     .malloc_fn = pvPortMalloc,
//     .free_fn = vPortFree,
// };

void https_jellyfin_task(void *arg) {
  // cJSON_InitHooks(&cjson_hook);

  http_get(JELLYFIN_REQ_QuickConnect_Initiate);
  while (1) {
    http_get(JELLYFIN_REQ_QuickConnect_Connect);
    vTaskDelay(1000);
  }
}