#include "imu.h"

#include <pico/stdlib.h>
#include <stdio.h>
#include <math.h>

#include <FreeRTOS.h>
#include <task.h>

#include "tkjhat/sdk.h"
#include "buzzer.h"
#include "state.h"
#include "interface.h"
#include "uart.h"

#define BUFFER_SIZE        100
#define TASK_DELAY         200

#define MOTION_MAGNITUDE   40
#define MOTION_TIME_MS     50
#define COOLDOWN_MS        700

#define DOMINANCE_F        1.3f
#define ALPHA              0.25f


static void add_char_to_message(char character) {
    g_state.currentMessage[g_state.currentMessageSize] = character;
    g_state.currentMessageSize++;
    g_state.currentMessage[g_state.currentMessageSize] = 0;
    update_interface();
}

static int get_dominant_axis(float *data) {
    float ax = fabsf(data[0]), ay = fabsf(data[1]), az = fabsf(data[2]);

    if (ax > ay * DOMINANCE_F && ax > az * DOMINANCE_F) return 0;
    else if (ay > ax * DOMINANCE_F && ay > az * DOMINANCE_F) return 1;
    else if (az > ay * DOMINANCE_F && az > ax * DOMINANCE_F) return 2;

    return -1;
}

static void gesture(int8_t *command) {
    printf("GESTURE %d\n", *command);
    switch(*command) {
        case 1:
            add_char_to_message('-');
            play_sound(LINE_SOUND);
            break;
        case -1:
            add_char_to_message('.');
            play_sound(DOT_SOUND);
            break;
        case 3:
            add_char_to_message(' ');
            break;
        case -3:
            send_message();
            play_sound(MESSAGE_SENT);
            break;
        default:
            break;
    }
}

void imu_task(void *pvParameters) {
    (void)pvParameters;

    float temp;

    float accel[3] = {0}, filt_accel[3] = {0};

    float gyro[3] = {0}, filt_gyro[3] = {0};

    float delta_accel[3] = {0};

    // Setting up the sensor. 

    // Start collection data here. Infinite loop. 
    TickType_t cooldown = 0;
    int8_t command = 0;
    while (1) {
        if(get_status() != INPUT || g_state.playing_music) {
            vTaskDelay(300);
            continue;
        }
        if (ICM42670_read_sensor_data(&accel[0], &accel[1], &accel[2], &gyro[0], &gyro[1], &gyro[2], &temp) != 0) {
            printf("Failed to read imu data\n");
            vTaskDelay(pdMS_TO_TICKS(TASK_DELAY));
            continue;
        }

        // Apply Exponential Moving Average (EMA) to remove noise
        for(int i = 0; i < 3; i++) {
            delta_accel[i] = filt_accel[i];
            filt_accel[i] = ALPHA * accel[i] + (1.0f - ALPHA) * filt_accel[i];
            delta_accel[i] = filt_accel[i] - delta_accel[i];

            filt_gyro[i] = ALPHA * gyro[i] + (1.0f - ALPHA) * filt_gyro[i];
        }


        if(g_state.settings.debug) {
        printf("Accel: X=%f, Y=%f, Z=%f | Gyro: X=%f, Y=%f, Z=%f, temp:%f\n",
            filt_accel[0], filt_accel[1], filt_accel[2], filt_gyro[0], filt_gyro[1], filt_gyro[2], temp);
        }

        int axis = get_dominant_axis(filt_gyro);
        TickType_t now = xTaskGetTickCount();

        if(cooldown != 0) {
            if(now - cooldown >= pdMS_TO_TICKS(COOLDOWN_MS))
                cooldown = 0;
            else {
                vTaskDelay(pdMS_TO_TICKS(TASK_DELAY));
                continue;
            }
        }
        
        if(axis != -1) {
            if(fabsf(filt_gyro[axis]) > MOTION_MAGNITUDE) {
                command = (axis+1) * (filt_gyro[axis] < 0 ? -1 : 1);
                cooldown = now;
                gesture(&command);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(TASK_DELAY));
    }
}