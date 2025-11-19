#ifndef STATE_H
#define STATE_H

#include <stdint.h>
#include <stdbool.h>

#define MSG_BUFFER_SIZE 256
#define MSG_LIST_SIZE 16

typedef enum {
    MAIN_MENU,
    UART,
    WIFI
} status;

typedef struct {
    uint8_t sender;
    int message_size;
    char message[MSG_BUFFER_SIZE];
} Message;

typedef struct {
    status g_status;
    Message currentMessage;
    Message messageHistory[MSG_LIST_SIZE];
} state;

// Global variable for the state
extern state g_state;

void state_init();

void set_status(status new_status);

status get_status();

#endif
