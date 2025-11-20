#ifndef STATE_H
#define STATE_H

#include <stdint.h>
#include <stdbool.h>

#define MSG_BUFFER_SIZE 256
#define MSG_LIST_SIZE 16

typedef enum {
    MAIN_MENU,
    INPUT,
    RECEIVING,
    SETTINGS
} Status;

typedef struct {
    // 0 = morse, 1 = text
    bool display_type;
    bool debug;
} Settings;

typedef struct {
    // 0 you 1 other person
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
    bool useUART;
    bool playing_music;
    Settings settings;
} State;

// Global variable for the state
extern State g_state;

void state_init();

void set_status(Status new_status);

Status get_status();

void add_message_to_history(char *message, uint8_t sender);

void morse_to_text(const char *morseInput, char *result);

#endif
