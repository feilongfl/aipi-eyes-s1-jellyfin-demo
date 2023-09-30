
#include "FreeRTOS.h"
#include "task.h"

#include "lvgl.h"

static TaskHandle_t lvgl_TaskHandle;

/* extern val */
lv_obj_t *lv_P_login_O_logincode;
/* extern val */

typedef void (*lv_fn)(struct lv_page *);

struct lv_page {
  lv_fn create;
  lv_fn destroy;
  lv_fn refersh;

  lv_obj_t *_page;
};

void lv_page_login_create(struct lv_page *page) {
  // use default page
  page->_page = lv_scr_act();

  //   lv_obj_t *page_box = lv_obj_create(page->_page);
  lv_obj_set_layout(page->_page, LV_LAYOUT_FLEX);
  lv_obj_set_flex_flow(page->_page, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_flex_align(page->_page, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER,
                        LV_FLEX_ALIGN_CENTER);

  lv_obj_t *logincode_msg = lv_label_create(page->_page);
  lv_label_set_text(logincode_msg, "Quick Login Code:");
  lv_P_login_O_logincode = lv_label_create(page->_page);
  lv_label_set_text(lv_P_login_O_logincode, "Loading...");

  return;
}
void lv_page_destroy(struct lv_page *page) { lv_obj_clean(page->_page); }

static struct lv_ui {
  struct lv_page login;
  struct lv_page music_player;
  struct lv_page music_list;
} lv_ui = {.login = {
               .create = lv_page_login_create,
           }};

static void ui_loop(struct lv_ui *ui) {}

static void ui_setup(struct lv_ui *ui) { ui->login.create(&ui->login); }

static void ui_task() {
  ui_setup(&lv_ui);
  while (1) {
    ui_loop(&lv_ui);

    lv_task_handler();
    vTaskDelay(1);
  }
}

#define LVGL_STACK_SIZE 1024
#define LVGL_TASK_PRIORITY 15
void ui_create() {
  xTaskCreate(ui_task, (char *)"lvgl", LVGL_STACK_SIZE, NULL,
              LVGL_TASK_PRIORITY, &lvgl_TaskHandle);
}
