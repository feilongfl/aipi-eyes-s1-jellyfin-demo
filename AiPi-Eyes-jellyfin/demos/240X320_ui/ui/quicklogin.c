#include "cJSON.h"
#include "lvgl.h"

static lv_obj_t *lv_P_login_O_logincode;

// 懒得queue
extern char *HTTP_JELLYFIN_Lib;
extern char *HTTP_JELLYFIN_MUSIC;

void setQuickLoginCode(char *code) {
  lv_label_set_text(lv_P_login_O_logincode, code);
}

void quickLogin() {
  // use default page
  lv_obj_t *page = lv_scr_act();

  //   lv_obj_t *page_box = lv_obj_create(page->_page);
  lv_obj_set_layout(page, LV_LAYOUT_FLEX);
  lv_obj_set_flex_flow(page, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_flex_align(page, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER,
                        LV_FLEX_ALIGN_CENTER);

  lv_obj_t *logincode_msg = lv_label_create(page);
  lv_label_set_text(logincode_msg, "Jellyfin Box");
  lv_P_login_O_logincode = lv_label_create(page);
  lv_label_set_text(lv_P_login_O_logincode, "Booting...");
}

static void libList_create_event(lv_event_t *e) {
  char *data = lv_event_get_user_data(e);
  HTTP_JELLYFIN_Lib = data;
  printf("button click");
}

static void libList_create_item_event(lv_event_t *e) {
  char *data = lv_event_get_user_data(e);
  HTTP_JELLYFIN_MUSIC = data;
  printf("button click");
}

static void libList_create(lv_obj_t *parent, char *name, char *id,
                           unsigned char isFolder) {
  // char buff[50];
  // sprintf(buff, "%s - %s", name, id);

  char *idbuf = malloc(33);
  memcpy(idbuf, id, 32); // 懒得free
  idbuf[32] = 0;

  lv_obj_t *btn = lv_btn_create(parent);
  if (isFolder) {
    lv_obj_add_event_cb(btn, libList_create_event, LV_EVENT_CLICKED, idbuf);
  } else {
    lv_obj_add_event_cb(btn, libList_create_item_event, LV_EVENT_CLICKED,
                        idbuf);
  }
  lv_obj_t *label = lv_label_create(btn);

  // lv_label_set_text(label, buff);
  lv_label_set_text(label, name);
}

void libBrowser(cJSON *lib) {
  lv_obj_t *page = lv_scr_act();
  lv_obj_clean(page);

  lv_obj_set_layout(page, LV_LAYOUT_FLEX);
  lv_obj_set_flex_flow(page, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_flex_align(page, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START,
                        LV_FLEX_ALIGN_START);

  lv_obj_t *logincode_msg = lv_label_create(page);
  lv_label_set_text(logincode_msg, "Lib:");

  printf("libBrowser: process json\n");
  cJSON *items = cJSON_GetObjectItem(lib, "Items");
  if (items == NULL) {
    printf("items == NULL\n");
    goto err;
  }
  for (uint8_t i = 0; i < cJSON_GetArraySize(items); i++) {
    cJSON *item = cJSON_GetArrayItem(items, i);
    if (item == NULL) {
      printf("item[%d] == NULL\n", i);
      goto err;
    }
    cJSON *name = cJSON_GetObjectItem(item, "Name");
    if (name == NULL) {
      printf("name[%d] == NULL\n", i);
      goto err;
    }
    cJSON *id = cJSON_GetObjectItem(item, "Id");
    if (id == NULL) {
      printf("id[%d] == NULL\n", i);
      goto err;
    }
    cJSON *isFolder = cJSON_GetObjectItem(item, "IsFolder");
    if (isFolder == NULL) {
      printf("isFolder[%d] == NULL\n", i);
      goto err;
    }
    libList_create(page, name->valuestring, id->valuestring,
                   isFolder->valueint);
  }
  printf("create done\n");

  return;

err:
  return;
}
