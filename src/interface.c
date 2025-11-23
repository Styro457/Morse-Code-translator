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

#define MENU_ITEM_NUM 3
#define SETTINGS_ITEM_NUM 3

#define CHAT_LINES_MAX       6
#define CHAT_CUR_MESSAGE_MAX   20
#define CHAT_MESSAGE_MAX       18

typedef enum {
    MAIN_MENU,
    SETTINGS,
    CHAT
} Menu;

static char main_menu[3][8] = {
"USB",
"UART",
"Settings"
};

static char settings[3][14] = {
"DISPLAY TYPE:",
"DEBUG:",
"Exit"
};

static volatile Menu menu;
static volatile uint8_t interface_index = 0;

// Store the total amount of lines for the displayed messages.
// Used for scrolling up in the chat interface
static volatile uint8_t history_lines = 0;
// How many lines each message has
static uint8_t message_lines[MSG_LIST_SIZE];

static bool update = true;

static char translationBuffer[MSG_BUFFER_SIZE];


static void display_menu();
static void display_chat();
static void display_settings();
static bool exit_to_main_menu();

void display_task(void *arg) {
    (void)arg;

    init_display();
    button_init();
    printf("__Initializing display__\n");

    menu = MAIN_MENU;

    while(1) {
        // button_check will block the task for 500ms if no button is pressed.
        button_check();
        if(update) {
            update = false;
            switch(menu) {
                case MAIN_MENU:
                    display_menu();
                    break;
                case CHAT:
                    // Stop input while updating the screen by setting status to MENU
                    if(get_status() == INPUT) {
                        set_status(MENU);
                        display_chat();
                        set_status(INPUT);
                    }
                    else {
                        display_chat();
                    }
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
        if(i == interface_index) {
            strcat(message, "> ");
        }
        strcat(message, main_menu[i]);
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
        if(i == interface_index) {
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

    char sender[3] = {'?', ':', '\0'};
    int empty_lines = MIN(history_lines, CHAT_LINES_MAX)+1;

    // Print Message History in reverse order in order to make sure they fit on the screen
    for(int i = g_state.messageHistorySize-1 - interface_index; i >= 0; i--) {

        empty_lines -= message_lines[i];
        if(empty_lines <= 0)
        break;

        // Get message from history
        char *message = g_state.messageHistory[i].message;

        // Get translated version if its enabled in settings
        if(g_state.settings.display_type == 1) {
            message = g_state.messageHistory[i].translated_message;
        }

        // Calculate y coordinate
        int y = (empty_lines-1) * TEXT_SMALL_Y_MUT;

        // Print the sender id
        sender[0] = '0'+g_state.messageHistory[i].sender;
        ssd1306_draw_string(get_display(), 0, y, 1, sender);
        
        // Print the message on multiple lines if its too long
        if(message_lines[i] != 1) {
            char line[CHAT_MESSAGE_MAX+1];
            for(int j = 0; j < message_lines[i]; j++) {
                int start = j * CHAT_MESSAGE_MAX;
                int len = j == message_lines[i]-1 ? strlen(message) - start : CHAT_MESSAGE_MAX;
                memcpy(line, &message[start], len);
                line[len] = '\0';
                ssd1306_draw_string(get_display(), 16, y+(j*TEXT_SMALL_Y_MUT), 1, line);
            }
        }
        // Print the message normally if its short enough
        else {
            ssd1306_draw_string(get_display(), 16, y, 1, message);
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
    ssd1306_draw_string(get_display(), 3, 52, 1, current_message);

    ssd1306_show(get_display());
}

void button_press(uint8_t button, bool hold) {
    // Main Menu
    if(menu == MAIN_MENU) {
        if(button == 1) {
           interface_index = (interface_index+1)%MENU_ITEM_NUM;
        }
        else {
            switch(interface_index) {
                case 0:
                    // Delete chat history if mode is switched
                    if(g_state.useUART != false) {
                        g_state.useUART = false;
                        g_state.messageHistorySize = 0;
                        clear_message_history();
                    }
                    play_sound(MENU_SOUND);
                    menu = CHAT;
                    set_status(INPUT);
                    break;
                case 1:
                    // Delete chat history if mode is switched
                    if(g_state.useUART != true) {
                        g_state.useUART = true;
                        clear_message_history();
                    }
                    play_sound(MENU_SOUND);
                    menu = CHAT;
                    set_status(INPUT);
                    break;
                case 2:
                    play_sound(MENU_SOUND);
                    interface_index = 0;
                    menu = SETTINGS;
                    break;
                default:
                    play_sound(ERROR_SOUND);
                    break;
            }
            // Reset interface index for the next menu
            interface_index = 0;
        }
    }


    // Settings Menu
    else if(menu == SETTINGS) {
        if(button == 1) {
            if(hold) {
                play_sound(MENU_SOUND);
                interface_index = 0;
                menu = MAIN_MENU;
            }
            else {
                interface_index = (interface_index+1)%SETTINGS_ITEM_NUM;
            }
        }
        else {
            switch(interface_index) {
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
                    interface_index = 2;
                    menu = MAIN_MENU;
                    break;
            }
        }
    }


    // Chat Interface
    else if(menu == CHAT) {
        // Button 1 logic
        if(button == 1) {
            // Check if the current message is empty or the input is a button hold
            // If it is, clear current message and exit to main menu
            if(g_state.currentMessageSize == 0 || hold) {
                exit_to_main_menu();
            }
            else {
                // Delete one character from the current message
                g_state.currentMessageSize--;
                g_state.currentMessage[g_state.currentMessageSize] = '\0';
            }
        }
        // Button 2 logic
        else {
            // If button 2 is held, send the current message
            if(hold) {
                send_message();
                // Scroll back down
                interface_index = 0;
            }
            // If button 2 is pressed, scroll up in chat history if there are any hidden lines. Once the top is reached, go back to 0
            else {
                if(history_lines > CHAT_LINES_MAX) {
                    interface_index = (interface_index+1);
                    if(interface_index > history_lines-CHAT_LINES_MAX)
                        interface_index = 0;
                }
            }
        }
    }

    // Update interface
    update = true;
}

void update_interface() {
    update = true;
}

void update_interface_message_history() {
    history_lines = 0;
    for(int i = 0; i < g_state.messageHistorySize; i++) {

        // Get message from history
        char *message = g_state.settings.display_type == 0 ? g_state.messageHistory[i].message : g_state.messageHistory[i].translated_message;
        int message_size = strlen(message);
        
        // Message is longer than one line
        if(message_size > CHAT_MESSAGE_MAX) {
            message_lines[i] = message_size/(float)CHAT_MESSAGE_MAX;
            if(message_size % CHAT_MESSAGE_MAX != 0) message_lines[i]++;
        }
        // Message is only one line
        else
            message_lines[i] = 1;
        
        history_lines += message_lines[i];
    }
    interface_index = 0;
}

static bool exit_to_main_menu() {
    if(get_status() == RECEIVING)
        return false;
    interface_index = 0;
    g_state.currentMessageSize = 0;
    g_state.currentMessage[0] = '\0';
    set_status(MENU);
    menu = MAIN_MENU;
    return true;
}