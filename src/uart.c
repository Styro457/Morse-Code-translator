
#include "state.h"
#include "uart.h"
#include <pico/stdlib.h>
#include <stdio.h>

#include <FreeRTOS.h>
#include <task.h>

#include "tkjhat/sdk.h"

#define DEFAULT_STACK_SIZE 2048

#define INPUT_BUFFER_SIZE 256

void send_message() {
  printf("%s", g_state.currentMessage);
  strcpy(g_state.messageHistory[g_state.messageHistorySize].message, g_state.currentMessage);
  g_state.currentMessage[0] = 0;
}

void receive_task(void *arg) {
  (void)arg;
  char line[INPUT_BUFFER_SIZE];
  size_t index = 0;

  while (1) {
    // OPTION 1
    //  Using getchar_timeout_us
    //  https://www.raspberrypi.com/documentation/pico-sdk/runtime.html#group_pico_stdio_1ga5d24f1a711eba3e0084b6310f6478c1a
    //  take one char per time and store it in line array, until reeceived the
    //  \n The application should instead play a sound, or blink a LED.
    int c = getchar_timeout_us(0);
    if (c != PICO_ERROR_TIMEOUT) { // I have received a character
      if (c == '\r')
        continue; // ignore CR, wait for LF if (ch == '\n') { line[len] = '\0';
      if (c == '\n') {
        // terminate and process the collected line
        line[index] = '\0';
        printf("__[RX]:\"%s\"__\n", line); // Print as debug in the output
        index = 0;
        vTaskDelay(pdMS_TO_TICKS(100)); // Wait for new message
      } else if (index < INPUT_BUFFER_SIZE - 1) {
        line[index++] = (char)c;
      } else { // Overflow: print and restart the buffer with the new character.
        line[INPUT_BUFFER_SIZE - 1] = '\0';
        printf("__[RX]:\"%s\"__\n", line);
        index = 0;
        line[index++] = (char)c;
      }
    } else {
      vTaskDelay(pdMS_TO_TICKS(100)); // Wait for new message
    }
    // OPTION 2. Use the whole buffer.
    /*absolute_time_t next = delayed_by_us(get_absolute_time,500);//Wait 500 us
    int read = stdio_get_until(line,INPUT_BUFFER_SIZE,next);
    if (read == PICO_ERROR_TIMEOUT){
        vTaskDelay(pdMS_TO_TICKS(100)); // Wait for new message
    }
    else {
        line[read] = '\0'; //Last character is 0
        printf("__[RX] \"%s\"\n__", line);
        vTaskDelay(pdMS_TO_TICKS(50));
    }*/
  }
}
