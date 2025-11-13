#include <stdio.h>
#include <string.h>

#include <FreeRTOS.h>
#include <semphr.h>
#include <task.h>

#include "tkjhat/sdk.h"
#include "interface.h"
#include "state.h"
#include "buzzer.h"

#define DEFAULT_I2C_SDA_PIN   12
#define DEFAULT_I2C_SCL_PIN   13

#define TEXT_X                 8
#define TEXT_SELECTED_X       36
#define TEXT_Y_MUT             16
#define TEXT_SCALE             2

static char menu[3][12] = {
    "UART",
    "WiFi",
    "Settings"
};

static volatile uint8_t selected_menu = 0;

static bool update = true;

static void display_menu();

void display_task(void *arg) {
    (void)arg;

    init_display();
    printf("Initializing display\n");

    while(1) {
        if(update) {
            update = false;
            if(get_status() == MAIN_MENU) {
                display_menu();
            }
            else {
                clear_display();
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

    // Update the display
    ssd1306_show(get_display());
}

void button_press(uint8_t button) {
    if(button == 1) {
        if(get_status() == MAIN_MENU) {
            selected_menu = (selected_menu+1)%3;
        }
        else {
            set_status(MAIN_MENU);
        }
    }
    else {
        switch(selected_menu) {
            case 0:
                //set_status(RECEIVING_DATA);
                play_sound(MESSAGE_RECEIVED);
                break;
            case 1:
                //set_status(WAITING_INPUT);
                play_sound(MESSAGE_SENT);
                break;
            default:
                //set_status(MAIN_MENU);
                play_sound(MUSIC);
                break;
        }
    }
    update = true;
}