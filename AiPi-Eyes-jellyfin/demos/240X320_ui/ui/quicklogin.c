#include "cJSON.h"
#include "lvgl.h"

static lv_obj_t *lv_P_login_O_logincode;

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

void libBrowser(cJSON *lib) {
  lv_obj_t *page = lv_scr_act();
  lv_obj_clean(page);

  lv_obj_set_layout(page, LV_LAYOUT_FLEX);
  lv_obj_set_flex_flow(page, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_flex_align(page, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START,
                        LV_FLEX_ALIGN_START);

  lv_obj_t *logincode_msg = lv_label_create(page);
  lv_label_set_text(logincode_msg, "Lib:");
}
