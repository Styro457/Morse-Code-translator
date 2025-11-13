#ifndef STATE_H
#define STATE_H

#include <stdint.h>
#include <stdbool.h>

typedef enum {
    MAIN_MENU,
    UART,
    WIFI
} status;

typedef struct {
    status g_status;
    char current_message[10];
} state;

// Global variable for the state
extern state g_state;

void state_init();

void set_status(status new_status);

status get_status();

#endif
