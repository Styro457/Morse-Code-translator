#include "buttons.h"

#include <stdio.h>
#include <string.h>

#include <pico/stdlib.h>
#include <FreeRTOS.h>
#include <semphr.h>
#include <task.h>

#include "tkjhat/sdk.h"
#include "interface.h"

#define BUTTONS          2

#define BTN_HOLD_MS      1000
#define BTN_COOLDOWN_MS  200

static SemaphoreHandle_t button_sem;
static bool was_pressed[BUTTONS] = {0};
static TickType_t last_press[BUTTONS] = {0};
static TickType_t cooldown[BUTTONS] = {0};
static int button_pin[BUTTONS] = {SW1_PIN, SW2_PIN};

void button_isr(unsigned int gpio, long unsigned int events);

void button_init() {
    button_sem = xSemaphoreCreateBinary();

    gpio_set_irq_enabled_with_callback(SW1_PIN, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &button_isr);
    gpio_set_irq_enabled_with_callback(SW2_PIN, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &button_isr);

}

void button_isr(unsigned int gpio, long unsigned int events) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xSemaphoreGiveFromISR(button_sem, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void button_check() {
    BaseType_t button_change = xSemaphoreTake(button_sem, 500);
    TickType_t now = xTaskGetTickCount();

    // The logic to run if there has been a change in the buttons (pressed or released)
    if (button_change == pdTRUE) {

        // Run the button logic for each button individually
        for(int i = 0; i < BUTTONS; i++) {
            bool is_pressed = gpio_get(button_pin[i]);

            if(is_pressed) {
                if(!was_pressed[i]) {
                    last_press[i] = now;
                }
            }

            else {
                if(was_pressed[i]) {
                    if(last_press[i] != 0 && now - cooldown[i] >= pdMS_TO_TICKS(BTN_COOLDOWN_MS)) {
                        button_press(i, false);
                        cooldown[i] = now;
                        last_press[i] = 0;
                    }
                    
                }
            }
            was_pressed[i] = is_pressed;
        }
    }

    else {
        for(int i = 0; i < BUTTONS; i++) {
            if(was_pressed[i] && last_press[i] != 0 && now - last_press[i] >= pdMS_TO_TICKS(BTN_HOLD_MS)) {
                    button_press(i, true);
                    last_press[i] = 0;
                    cooldown[i] = now;
            }
        }
    }
}