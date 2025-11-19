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
} Status;

typedef struct {
    uint8_t sender;
    int message_size;
    char message[MSG_BUFFER_SIZE];
} Message;

typedef struct {
    Status status;
    char currentMessage[MSG_BUFFER_SIZE];
    int currentMessageSize;
    Message messageHistory[MSG_LIST_SIZE];
    int messageHistorySize;
} State;

// Global variable for the state
extern State g_state;

void state_init();

void set_status(Status new_status);

Status get_status();

#endif
