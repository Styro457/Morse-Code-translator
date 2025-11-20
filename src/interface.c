#include "interface.h"

#include <stdio.h>
#include <string.h>

#include <FreeRTOS.h>
#include <semphr.h>
#include <task.h>

#include "tkjhat/sdk.h"
#include "buttons.h"
#include "state.h"
#include "buzzer.h"
#include "uart.h"

#define TEXT_X                 8
#define TEXT_SELECTED_X        36
#define TEXT_Y_MUT             16
#define TEXT_SMALL_Y_MUT       8

static char menu[3][12] = {
"USB",
"UART",
"Settings"
};

static char settings[3][14] = {
"DISPLAY TYPE:",
"DEBUG:",
"Exit"
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
    for(uint8_t i = 0; i < MENU_ITEM_NUM; i++) {
        message[0] = '\0';
        if(i == selected_menu) {
            strcat(message, "> ");
        }
        strcat(message, menu[i]);
        ssd1306_draw_string(get_display(), 0, i*TEXT_Y_MUT, 2, message);
    }
    ssd1306_show(get_display());
}

static void display_settings() {
    clear_display();
    uint32_t x = TEXT_X;
    char setting[16];
    for(uint8_t i = 0; i < SETTINGS_ITEM_NUM; i++) {
        setting[0] = '\0';
        if(i == selected_menu) {
            strcat(setting, "> ");
        }
        strcat(setting, settings[i]);
        if(i == 0) {
            strcat(setting, g_state.settings.display_type ? " TEXT" : " MORSE");
        }
        else if(i == 1) {
            strcat(setting, g_state.settings.debug ? " on" : " off");
        }
        ssd1306_draw_string(get_display(), 0, i*TEXT_Y_MUT, 1, setting);
    }
    ssd1306_show(get_display());
}

static void display_chat() {
    clear_display();

    // Print Message History
    int history_lines = 0;
    for(int i = 0; i < g_state.messageHistorySize; i++) {
        // Get message from history
        char *message = g_state.messageHistory[i].message;
        int message_size = g_state.messageHistory[i].message_size;

        // Translate the message if its enabled in settings
        if(g_state.settings.display_type == 1) {
            morse_to_text(message, translationBuffer);
            message = translationBuffer;
            message_size = strlen(message);
        }

        int y = history_lines*TEXT_SMALL_Y_MUT;

        // Print the sender id
        char sender[3] = {'0'+g_state.messageHistory[i].sender, ':', '\0'};
        ssd1306_draw_string(get_display(), 0, y, 1, sender);
        
        // Print the message on multiple lines if its too long
        if(message_size > CHAT_MESSAGE_MAX) {
            char line[CHAT_MESSAGE_MAX+1];

            uint8_t lines = message_size/(float)CHAT_MESSAGE_MAX;
            if(message_size % CHAT_MESSAGE_MAX != 0) lines++;

            history_lines += lines;

            for(int j = 0; j < lines; j++) {
                int start = j * CHAT_MESSAGE_MAX;
                int len = j == lines-1 ? message_size - start : CHAT_MESSAGE_MAX;
                memcpy(line, &message[start], len);
                line[len] = '\0';
                ssd1306_draw_string(get_display(), 16, y+(j*TEXT_SMALL_Y_MUT), 1, line);
            }
        }
        // Print the message normally if its short enough
        else {
            ssd1306_draw_string(get_display(), 16, y, 1, message);
            history_lines++;
        }
    }

    // Draw chat box
    ssd1306_draw_empty_square(get_display(), 0, 50, 127, 13);

    // Only display the last characters in the currentMessage if it exceeds the screen size
    char *current_message = g_state.currentMessage;
    if(g_state.currentMessageSize > CHAT_CUR_MESSAGE_MAX) {
        current_message += g_state.currentMessageSize-CHAT_CUR_MESSAGE_MAX;
    }

    // Print current message
    ssd1306_draw_string(get_display(), 0, 52, 1, current_message);

    ssd1306_show(get_display());
}

void button_press(uint8_t button, bool hold) {
    // Main Menu
    if(get_status() == MAIN_MENU) {
        if(button == 1) {
           selected_menu = (selected_menu+1)%MENU_ITEM_NUM;
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
                    break;
                case 2:
                    play_sound(MENU_SOUND);
                    selected_menu = 2;
                    set_status(MAIN_MENU);
                    break;
            }
        }
    }


    // Chat Interface
    else if(get_status() == RECEIVING || get_status() == INPUT) {
        // Button 1 logic
        if(button == 1) {
            // Check if the current message is empty or the input is a button hold
            // If it is, exit to main menu
            if(g_state.currentMessageSize == 0 || hold) {
                set_status(MAIN_MENU);
                // Delete message history
                g_state.messageHistorySize = 0;
            }
            else {
                // Delete one character from the current message
                g_state.currentMessageSize--;
                g_state.currentMessage[g_state.currentMessageSize] = '\0';
            }
        }
        // Button 2 logic
        else {
            send_message();
        }
    }

    // Update interface
    update = true;
}

void update_interface() {
update = true;
}
