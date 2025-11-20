#include "uart.h"
#include "buzzer.h"
#include "interface.h"

#include <string.h>
#include <pico/stdlib.h>
#include <stdio.h>

#include <FreeRTOS.h>
#include <task.h>

#include "tkjhat/sdk.h"
#include "state.h"

#define DEFAULT_STACK_SIZE 2048

#define INPUT_BUFFER_SIZE 256

void send_message() {
  printf("MSG: %s", g_state.currentMessage);
  strcpy(g_state.messageHistory[g_state.messageHistorySize].message, g_state.currentMessage);
  g_state.messageHistory[g_state.messageHistorySize].sender = 0;
  g_state.messageHistory[g_state.messageHistorySize].message_size = strlen(g_state.currentMessage);
  g_state.messageHistorySize++;
  g_state.currentMessage[0] = 0;
  g_state.currentMessageSize = 0;
  play_sound(MESSAGE_RECEIVED);
}

void receive_task(void *arg) {
  (void)arg;
  char line[INPUT_BUFFER_SIZE];
  size_t index = 0;

  while (1) {
        int c = getchar_timeout_us(0);
        if (c != PICO_ERROR_TIMEOUT){// I have received a character
            set_status(RECEIVING);
            if (c == '\r') continue; // ignore CR, wait for LF if (ch == '\n') { line[len] = '\0';
            if (c == '\n'){
                // terminate and process the collected line
                line[index] = '\0'; 
                printf("__[RX]:\"%s\"__\n", line); //Print as debug in the output
                index = 0;
                
                strcpy(g_state.messageHistory[g_state.messageHistorySize].message, line);
                g_state.messageHistory[g_state.messageHistorySize].sender = 1;
                g_state.messageHistory[g_state.messageHistorySize].message_size = strlen(line);
                g_state.messageHistorySize++;
                update_interface();
                play_sound(MESSAGE_RECEIVED);
                vTaskDelay(pdMS_TO_TICKS(100)); // Wait for new message
                set_status(INPUT);
            }
            else if(index < INPUT_BUFFER_SIZE - 1){
                line[index++] = (char)c;
                vTaskDelay(pdMS_TO_TICKS(100));
            }
            else { //Overflow: print and restart the buffer with the new character. 
                line[INPUT_BUFFER_SIZE - 1] = '\0';
                printf("__[RX]:\"%s\"__\n", line);
                index = 0; 
                line[index++] = (char)c; 
                vTaskDelay(pdMS_TO_TICKS(100));
            }
        }
        else {
            vTaskDelay(pdMS_TO_TICKS(100)); // Wait for new message
        }
  }
}
