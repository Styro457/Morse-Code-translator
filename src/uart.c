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

#define INPUT_BUFFER_SIZE 256

void init_uart() {
  uart_init(uart0, 115200);

  // Configure GPIO pins for UART0
  gpio_set_function(0, GPIO_FUNC_UART); // GP0 = TX
  gpio_set_function(1, GPIO_FUNC_UART); // GP1 = RX
}

void send_message() {
  if(get_status() != INPUT){
    return;
  }
  set_status(OUTPUT);
  if(g_state.currentMessageSize == 0) {
    play_sound(ERROR_SOUND);
    return;
  }
  
  play_sound(MESSAGE_SENT);

  if(g_state.useUART) {
    for(int i = 0; i < g_state.currentMessageSize; i++) {
      uart_putc(uart0, g_state.currentMessage[i]);
    }
    uart_putc(uart0, ' ');
    uart_putc(uart0, ' ');
    uart_putc(uart0, '\n');
  }
  else {
    printf("%s  \n", g_state.currentMessage);
  }
  add_message_to_history(g_state.currentMessage, 0);
  g_state.currentMessage[0] = 0;
  g_state.currentMessageSize = 0;
  set_status(INPUT);
}

void receive_task(void *arg) {
  (void)arg;
  char line[INPUT_BUFFER_SIZE];
  size_t index = 0;

  init_uart();
  int c;
  bool receive;
  while (1) {
    receive = false;

    // UART logic
    if(g_state.useUART) {
      if(uart_is_readable(uart0)) {
        c = uart_getc(uart0);
        receive = true;
      }
    }

    // USB Logic
    else {
      c = getchar_timeout_us(0);
      if(c != PICO_ERROR_TIMEOUT) {
        receive = true;
      }
    }
    if (receive){ // I have received a character
      
      // If this is the first character, set status to RECEIVING to stop IMU task and play a sound
      if(get_status() != RECEIVING) {
        play_sound(MESSAGE_RECEIVED);
        set_status(RECEIVING);
      }

      if (c == '\r') continue; // ignore CR, wait for LF if (ch == '\n') { line[len] = '\0';
      if (c == '\n'){
          // terminate and process the collected line
          line[index] = '\0'; 
          //printf("__[RX]:\"%s\"__\n", line); //Print as debug in the output
          index = 0;
                
          add_message_to_history(line, 1);
          // Set status back to INPUT
          set_status(INPUT);

      }
      else if(index < INPUT_BUFFER_SIZE - 1){
          line[index++] = (char)c;
          if ((char)c == '.'){
            play_sound(DOT_SOUND);
          }
          else if ((char)c == '-'){
            play_sound(LINE_SOUND);
          }
          else if ((char)c == ' '){
            play_sound(SPACE_SOUND);
          }
          //vTaskDelay(400);
      }
      else { //Overflow: print and restart the buffer with the new character. 
          line[INPUT_BUFFER_SIZE - 1] = '\0';
          printf("__[RX]:\"%s\"__\n", line);
          add_message_to_history(line, 1);
          index = 0; 
      }
    }
    vTaskDelay(pdMS_TO_TICKS(200)); // Wait for new message
  }
}
