#include "buttons.h"

#include <stdio.h>
#include <string.h>

#include <pico/stdlib.h>
#include <FreeRTOS.h>
#include <semphr.h>
#include <task.h>

#include "tkjhat/sdk.h"
#include "interface.h"

#define BUTTONS            2

#define BTN_TASK_DELAY     20
#define BTN_HOLD_MS 200

static SemaphoreHandle_t button_sem[BUTTONS];
static TickType_t last_press[BUTTONS] = {0};

void button_isr(unsigned int gpio, long unsigned int events);

void button_init() {
    for(int i = 0; i < BUTTONS; i++) {
        button_sem[i] = xSemaphoreCreateBinary();
    }

      gpio_set_irq_enabled_with_callback(SW1_PIN, GPIO_IRQ_EDGE_RISE, true, &button_isr);
    gpio_set_irq_enabled_with_callback(SW2_PIN, GPIO_IRQ_EDGE_RISE, true, &button_isr);

}

void button_isr(unsigned int gpio, long unsigned int events) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    switch (gpio) {
        case SW1_PIN:
            xSemaphoreGiveFromISR(button_sem[0], &xHigherPriorityTaskWoken);
            break;
        case SW2_PIN:
            xSemaphoreGiveFromISR(button_sem[1], &xHigherPriorityTaskWoken);
            break;
        default:
            break;
    }

    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void button_task(void *arg) {
    (void)arg;
    button_init();

    while (1) {
        for(int i = 0; i < BUTTONS; i++) {
            // Count holding the button as 1 press

            if (xSemaphoreTake(button_sem[i], 0) == pdTRUE) {
                TickType_t now = xTaskGetTickCount();
                if (now - last_press[i] > pdMS_TO_TICKS(BTN_HOLD_MS)) {
                    button_press(i);
                    last_press[i] = now;
                }
            } 

        }

        vTaskDelay(pdMS_TO_TICKS(BTN_TASK_DELAY));
    }
}