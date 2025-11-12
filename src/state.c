#include <FreeRTOS.h>
#include <semphr.h>

#include "state.h"

state g_state;

static SemaphoreHandle_t state_mutex = NULL;

void state_init() {
    //state_mutex = xSemaphoreCreateMutex();
    g_state.g_status = MAIN_MENU;
}


void set_status(status new_status) {
    //xSemaphoreTake(state_mutex, portMAX_DELAY);
    g_state.g_status = new_status;
    //xSemaphoreGive(state_mutex);
}

status get_status() {
    status current_status;
    //xSemaphoreTake(state_mutex, portMAX_DELAY);
    current_status = g_state.g_status;
    //xSemaphoreGive(state_mutex);
    return current_status;
}