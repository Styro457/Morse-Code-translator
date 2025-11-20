#include <pico/stdlib.h>

#define MENU_ITEM_NUM 3
#define SETTINGS_ITEM_NUM 3

#define CHAT_HISTORY_MAX       4
#define CHAT_CUR_MESSAGE_MAX   20
#define CHAT_MESSAGE_MAX       18

void display_task(void *arg);

void button_press(uint8_t button, bool hold);

void update_interface();