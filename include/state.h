// Used to ensure this header file is only defined once
#ifndef STATE_H
#define STATE_H

#include <stdint.h>
#include <stdbool.h>

// The character length of a message
#define MSG_BUFFER_SIZE 256
// How many messages the history saves
#define MSG_LIST_SIZE 16

// States for the Global State Machine
typedef enum {
    MENU,
    INPUT,
    OUTPUT,
    RECEIVING
} Status;

typedef struct {
    // 0 = morse, 1 = text
    bool display_type;
    bool debug;
} Settings;

typedef struct {
    // 0 is for self, 1 for the connected program / device
    bool sender;
    int message_size;
    char message[MSG_BUFFER_SIZE];
    char translated_message[MSG_BUFFER_SIZE/2 + 1];
} Message;

// Global structure used for variables 
typedef struct {
    Status status;
    char currentMessage[MSG_BUFFER_SIZE];
    int currentMessageSize;
    Message messageHistory[MSG_LIST_SIZE];
    int messageHistorySize;
    // Only changed from Display task
    bool useUART;
    // Only changed from Buzzer Task
    bool playing_music;
    // Only changed from Display Task
    Settings settings;
} State;

// Global variable for the state
extern State g_state;

/**
 * Print a message using `printf` only if the `debug` option is enabled in the settings
 * @param message The message to print
 */
void debug(char* message);

// Initate the default values for the global state
void state_init();

// Change the status
void set_status(Status new_status);

// Get the current status
Status get_status();

/**
 * Add a message into message history
 * @param message Array of characters representing the message
 * @param sender Who sent this message. `0` for self, `1` for connected program / device
 */
void add_message_to_history(char *message, bool sender);

// Clear the message history.
// Used when changing from USB to UART
void clear_message_history();

/**
 * Convert morse code messages into normal text.
 * @param morseInput the original message in morse code
 * @param result the array to save the normal text result in
 */
void morse_to_text(const char *morseInput, char *result);

#endif
