#include <FreeRTOS.h>
#include <semphr.h>
#include <stdio.h>
#include <string.h>


#include "state.h"

State g_state;

static SemaphoreHandle_t state_mutex = NULL;

void state_init() {
    //state_mutex = xSemaphoreCreateMutex();
    g_state.status = MAIN_MENU;
    g_state.messageHistorySize = 0;
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

char morse_table[] = {
    {'A', ".-"},    {'B', "-..."},  {'C', "-.-."},  {'D', "-.."},   {'E', "."},
    {'F', "..-."},  {'G', "--."},   {'H', "...."},  {'I', ".."},    {'J', ".---"},
    {'K', "-.-"},   {'L', ".-.."},  {'M', "--"},    {'N', "-."},    {'O', "---"},
    {'P', ".--."},  {'Q', "--.-"},  {'R', ".-."},   {'S', "..."},   {'T', "-"},
    {'U', "..-"},   {'V', "...-"},  {'W', ".--"},   {'X', "-..-"},  {'Y', "-.--"},
    {'Z', "--.."},
    
    {'0', "-----"}, {'1', ".----"}, {'2', "..---"}, {'3', "...--"}, {'4', "....-"},
    {'5', "....."}, {'6', "-...."}, {'7', "--..."}, {'8', "---.."}, {'9', "----."},
    
};


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

