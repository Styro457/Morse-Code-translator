#include <stdio.h>
#include <string.h>

#include <FreeRTOS.h>
#include <semphr.h>
#include <task.h>

#include "tkjhat/sdk.h"
#include "interface.h"
#include "buttons.h"
#include "state.h"
#include "buzzer.h"

#define DEFAULT_I2C_SDA_PIN   12
#define DEFAULT_I2C_SCL_PIN   13

#define TEXT_X                 8
#define TEXT_SELECTED_X        36
#define TEXT_Y_MUT             16
#define TEXT_SMALL_Y_MUT       8
#define TEXT_SCALE             2

static char menu[3][12] = {
"USB",
"UART",
"Settings"
};

static char settings[2][14] = {
"DISPLAY TYPE:",
"DEBUG:"
};

static volatile uint8_t selected_menu = 0;

static bool update = true;

static char translationBuffer[MSG_BUFFER_SIZE];


static void display_menu();
static void display_chat();
static void display_settings();

void display_task(void *arg) {
    (void)arg;

    init_display();
    button_init();
    printf("Initializing display\n");

    while(1) {
        // button_check will block the task for 500ms if no button is pressed.
        button_check();
        if(update) {
            update = false;
            switch(get_status()) {
                case MAIN_MENU:
                    display_menu();
                    break;
                case INPUT:
                    // Stop input while updating the screen by setting status to RECEIVING
                    set_status(RECEIVING);
                    display_chat();
                    set_status(INPUT);
                    break;
                case RECEIVING:
                    display_chat();
                    break;
                case SETTINGS:
                    display_settings();
                    break;
                default:
                    clear_display();
                    break;
            }
        }
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

static void display_menu() {
    clear_display();
    uint32_t x = TEXT_X;
    char message[16];
    for(uint8_t i = 0; i < 3; i++) {
        message[0] = '\0';
        if(i == selected_menu) {
            strcat(message, "> ");
        }
        strcat(message, menu[i]);
        ssd1306_draw_string(get_display(), 0, i*TEXT_Y_MUT, TEXT_SCALE, message);
    }
    ssd1306_show(get_display());
}

static void display_settings() {
    clear_display();
    uint32_t x = TEXT_X;
    char setting[16];
    for(uint8_t i = 0; i < 2; i++) {
        setting[0] = '\0';
        if(i == selected_menu) {
            strcat(setting, "> ");
        }
        strcat(setting, settings[i]);
        if(i == 0) {
            strcat(setting, g_state.settings.display_type ? " MORSE" : " TEXT");
        }
        else {
            strcat(setting, g_state.settings.debug ? " on" : " off");
        }
        ssd1306_draw_string(get_display(), 0, i*TEXT_Y_MUT, 1, setting);
    }
    ssd1306_show(get_display());
}

static void display_chat() {
    clear_display();
    for(int i = 0; i < g_state.messageHistorySize; i++) {
    char *message = g_state.messageHistory[i].message;
    if(g_state.settings.display_type == 1) {
        morse_to_text(message, translationBuffer);
        message = translationBuffer;
    }
    if(g_state.messageHistory[i].sender == 0) {
        ssd1306_draw_string(get_display(), 16, i*TEXT_SMALL_Y_MUT, 1, message);
    }
    else {
        ssd1306_draw_string(get_display(), 0, i*TEXT_SMALL_Y_MUT, 1, message);
    }
    }
    ssd1306_draw_empty_square(get_display(), 0, 50, 127, 13);
    ssd1306_draw_string(get_display(), 0, 52, 1, g_state.currentMessage);
    ssd1306_show(get_display());
}

void button_press(uint8_t button, bool hold) {
    // Main Menu
    if(get_status() == MAIN_MENU) {
        if(button == 1) {
           selected_menu = (selected_menu+1)%3;
        }
        else {
            switch(selected_menu) {
                case 0:
                    g_state.useUART = false;
                    play_sound(MENU_SOUND);
                    set_status(INPUT);
                    break;
                case 1:
                    g_state.useUART = true;
                    play_sound(MENU_SOUND);
                    set_status(INPUT);
                    break;
                case 2:
                    play_sound(MENU_SOUND);
                    selected_menu = 0;
                    set_status(SETTINGS);
                    break;
                default:
                    //set_status(MAIN_MENU);
                    play_sound(MUSIC);
                    break;
            }
        }
    }


    // Settings Menu
    else if(get_status() == SETTINGS) {
        if(button == 1) {
            if(hold) {
                play_sound(MENU_SOUND);
                set_status(MAIN_MENU);
            }
            else {
                selected_menu = (selected_menu+1)%SETTINGS_ITEM_NUM;
            }
        }
        else {
            switch(selected_menu) {
                case 0:
                    play_sound(MENU_SOUND);
                    g_state.settings.display_type = !g_state.settings.display_type;
                    break;
                case 1:
                    play_sound(MENU_SOUND);
                    g_state.settings.debug = !g_state.settings.debug;
            }
        }
    }


    // Default Logic
    else {
        if(button == 1) {
            set_status(MAIN_MENU);
        }
    }

    // Update interface
    update = true;
}

void update_interface() {
update = true;
}
