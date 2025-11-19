#include "imu.h"

#include <pico/stdlib.h>
#include <stdio.h>

#include <FreeRTOS.h>
#include <task.h>

#include "tkjhat/sdk.h"
#include "buzzer.h"
#include "state.h"
#include "interface.h"
#include "uart.h"

#define BUFFER_SIZE 100
#define MOTION_TIME_MS 100
#define MOTION_MAGNITUDE 100
#define MOTION_STILL 50


static void add_char_to_message(char character) {
    g_state.currentMessage[g_state.currentMessageSize] = character;
    g_state.currentMessageSize++;
    g_state.currentMessage[g_state.currentMessageSize] = 0;
    update_interface();
}

void imu_task(void *pvParameters) {
    (void)pvParameters;

    float ax, ay, az, gx, gy, gz, t;
    // Setting up the sensor. 
    if (init_ICM42670() == 0) {
        printf("ICM-42670P initialized successfully!\n");
        if (ICM42670_start_with_default_values() != 0){
            printf("ICM-42670P could not initialize accelerometer or gyroscope");
        }
    } else {
        printf("Failed to initialize ICM-42670P.\n");
    }
    // Start collection data here. Infinite loop. 
    TickType_t motion_time[2] = {0, 0};
    uint8_t command[2] = {0, 0};

    bool x_cooldown = false, y_cooldown = false;
    while (1)
    {
        if (ICM42670_read_sensor_data(&ax, &ay, &az, &gx, &gy, &gz, &t) == 0) {
            TickType_t now = xTaskGetTickCount();
            printf("Accel: X=%f, Y=%f, Z=%f | Gyro: X=%f, Y=%f, Z=%f| Temp: %2.2f°C\n", ax, ay, az, gx, gy, gz, t);

            // Not proud of this code but it works
            if(gx > MOTION_MAGNITUDE) {
                motion_time[0] = now;
                command[0] = 1;
            }
            else if(gx < -MOTION_MAGNITUDE) {
                motion_time[0] = now;
                command[0] = 2;
            }
            else if (-MOTION_STILL < gx && gx < MOTION_STILL) {

                // Check if a movememnt was detected and if its longer than MOTION_TIME_MS
                if(motion_time[0] != 0 && now - motion_time[0] >= pdMS_TO_TICKS(MOTION_TIME_MS)) {
                    if(command[0] == 1) {
                        add_char_to_message('.');
                        play_sound(DOT_SOUND);
                    }
                    else {
                        add_char_to_message('-');
                        play_sound(LINE_SOUND);
                    }
                }
                motion_time[0] = 0;

                // Same checks for z axis
                if(gz > MOTION_MAGNITUDE) {
                    motion_time[1] = now;
                    command[1] = 1;
                }
                else if(gz < -MOTION_MAGNITUDE) {
                    motion_time[1] = now;
                    command[1] = 2;
                }
                else if (-MOTION_STILL < gz && gz < MOTION_STILL) {

                    // Check if a movememnt was detected and if its longer than MOTION_TIME_MS
                    if(motion_time[1] != 0 && now - motion_time[1] >= pdMS_TO_TICKS(MOTION_TIME_MS)) {
                        if(command[1] == 2) {
                            add_char_to_message(' ');
                        }
                        else {
                            send_message();
                            update_interface();
                        }
                    }
                    motion_time[1] = 0;
                }
            }

        } else {
            printf("Failed to read imu data\n");
        }
        vTaskDelay(pdMS_TO_TICKS(250));
    }
}