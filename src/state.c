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
    g_state.status = MENU;
    g_state.messageHistorySize = 0;
    g_state.settings.debug = false;
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
    Message *message_struct = &g_state.messageHistory[g_state.messageHistorySize];
    strcpy(message_struct->message, message);
    message_struct->sender = sender;

    // Save the translated message in the struct so that it doesnt have to be calculated each time
    morse_to_text(message, message_struct->translated_message);

    // Set message length
    message_struct->message_size = strlen(message);

    g_state.messageHistorySize++;
    
    update_interface_message_history();
    update_interface();
}

void clear_message_history() {
    g_state.messageHistorySize = 0;
    update_interface_message_history();
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
    
    int i = 0;
    char current_morse[10];
    int morse_index = 0;
    int prev_was_space = 0;
    bool wrong_message_format = false;

    while (morse_input[i] != '\0') {
        if (morse_input[i] == ' ' || wrong_message_format) {
            // Add one space if there are 2 spaces and we are not at the end of the message
            if(prev_was_space && morse_input[i+1] != '\0') {
                strcat(result, " ");
            }
            else {
                // Process completed morse sequence
                if (morse_index > 0) {
                    current_morse[morse_index] = '\0';
                    char ch = morse_to_char(current_morse);
                    
                    strcat(result, (char[]){ch, '\0'});
                    prev_was_space = 0;
                    morse_index = 0;
                }
                prev_was_space = 1;
            }
        }
        // Morse character
        else {
            prev_was_space = 0;
            current_morse[morse_index++] = morse_input[i];

            // Check for messages that do not follow the proper message structure (ending with '  \n')
            if(morse_input[i+1] == '\0') {
                i--;
                wrong_message_format = true;
            }
        }
        i++;
    }
}