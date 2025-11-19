#include <FreeRTOS.h>
#include <semphr.h>

#include "state.h"

State g_state;

static SemaphoreHandle_t state_mutex = NULL;

void state_init() {
    //state_mutex = xSemaphoreCreateMutex();
    g_state.status = MAIN_MENU;
    g_state.messageHistorySize = 0;
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
