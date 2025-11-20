#include <FreeRTOS.h>
#include <semphr.h>
#include <stdio.h>
#include <string.h>


#include "state.h"
#include "interface.h"

State g_state;

static SemaphoreHandle_t state_mutex = NULL;

void state_init() {
    //state_mutex = xSemaphoreCreateMutex();
    g_state.status = MAIN_MENU;
    g_state.messageHistorySize = 0;
    g_state.settings.debug = true;
    g_state.settings.display_type = 1;
}


void set_status(Status new_status) {
    //xSemaphoreTake(state_mutex, portMAX_DELAY);
    g_state.status = new_status;
    //xSemaphoreGive(state_mutex);
}

Status get_status() {
    Status current_status;
    //xSemaphoreTake(state_mutex, portMAX_DELAY);
    current_status = g_state.status;
    //xSemaphoreGive(state_mutex);
    return current_status;
}

const char* morse_table[] = {
    ".-", "-...", "-.-.", "-..", ".", 
    "..-.", "--.", "....", "..", ".---",
    "-.-", ".-..", "--", "-.", "---",
    ".--.", "--.-", ".-.", "...", "-",
    "..-", "...-", ".--", "-..-", "-.--",
    "--..",

    "-----", ".----", "..---", "...--", "....-",
    ".....", "-....", "--...", "---..", "----.",
};

void add_message_to_history(char *message, uint8_t sender) {
    // Check if the message history is full.
    // If it is full delete the first message and move everything back 1 index.
    if(g_state.messageHistorySize == MSG_LIST_SIZE) {
        for(int i = 0; i < MSG_LIST_SIZE-1; i++) {
            g_state.messageHistory[i] = g_state.messageHistory[i+1];
        }
        g_state.messageHistorySize--;
    }

    // Copy the message to messageHistory in the global state
    strcpy(g_state.messageHistory[g_state.messageHistorySize].message, message);
    g_state.messageHistory[g_state.messageHistorySize].sender = sender;
    g_state.messageHistory[g_state.messageHistorySize].message_size = strlen(g_state.currentMessage);
    g_state.messageHistorySize++;
    update_interface();
}


char morse_to_char(const char *morse) {
    for (int i = 0; i < 36; i++) {
        if (strcmp(morse, morse_table[i]) == 0) {
            if (i < 26) return 'A' + i; 
            else return '0' + (i - 26);
        }
    }
    return '?';
}

void morse_to_text(const char *morse_input, char *result) {
    result[0] = '\0';
    char buffer[500];
    strcpy(buffer, morse_input);

    char *token = strtok(buffer, " ");
    while (token != NULL) {
        if (strcmp(token, "/") == 0) {
            strcat(result, " ");
        } else {
            char ch = morse_to_char(token);
            int len = strlen(result);
            result[len] = ch;
            result[len + 1] = '\0';
        }
        token = strtok(NULL, " ");
    }
}

