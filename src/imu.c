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

// Default task delay for the IMU task
#define TASK_DELAY         200

// Magnitude needed on the gyroscope dominant axis for it to be detected as a tilting motion
#define MOTION_MAGNITUDE   40

// How long in ms the tilting motion should be detected for so that it counts as a gesture
#define MOTION_TIME_MS     50

// How much time to pause detection after running one gesture
#define COOLDOWN_MS        800

// How much time an orientation needs to be held for the program to run the specific music for it
#define ORIENT_DURATION_MS 500

// Value to multiply other axis by when getting the dominant axis
#define DOMINANCE_F        1.3f

// Alpha used for EMA when filtering IMU values
#define ALPHA              0.25f


void init_imu() {
    if (init_ICM42670() == 0) {
        debug("__ICM-42670P initialized successfully!__\n");
        if (ICM42670_start_with_default_values() != 0){
            debug("__ICM-42670P could not initialize accelerometer or gyroscope__");
        }
    }
    else {
        debug("__Failed to initialize ICM-42670P__\n");
    }
}

/**
 * Funtion for adding a charater to the current message.
 * 
 * It is used by the IMU sensor to write the characters from gestures
 * @param character The character to be added to a message
 */
static void add_char_to_message(char character) {

    // Check if there is no more space in the currentMessage buffer
    // If there is not, return
    if(g_state.currentMessageSize == MSG_BUFFER_SIZE) {
        play_sound(ERROR_SOUND);
        return;
    }

    g_state.currentMessage[g_state.currentMessageSize] = character;
    g_state.currentMessageSize++;
    g_state.currentMessage[g_state.currentMessageSize] = 0;

    // Update the interface so it shows the updates message
    update_interface();
}

/**
  * Returns the dominant axis with the highest magnitude.
  * DOMINANCE_F is used to check that the difference between the dominant axis and other axis is significant.
  * 
  * If there is no significantlly larger axis, return -1
  * @param data Float array of size 3 representing the 3 spacial axis for the data
*/
static int get_dominant_axis(float *data) {
    float ax = fabsf(data[0]), ay = fabsf(data[1]), az = fabsf(data[2]);

    if (ax > ay * DOMINANCE_F && ax > az * DOMINANCE_F) return 0;
    else if (ay > ax * DOMINANCE_F && ay > az * DOMINANCE_F) return 1;
    else if (az > ay * DOMINANCE_F && az > ax * DOMINANCE_F) return 2;

    return -1;
}

/**
 * Handle the code for the gestures detected
 * @param command -1 for up, 1 for up, 3 for left, -3 for right
 */
static void gesture(int8_t *command) {
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
            play_sound(SPACE_SOUND);
            break;
        case -3:
            if(g_state.currentMessageSize != 0) {
                send_message();
                play_sound(MESSAGE_SENT);
            }
            break;
        default:
            break;
    }
}

void imu_task(void *pvParameters) {
    (void)pvParameters;

    // Setting up the sensor. 
    init_imu();

    //Waiting for everything to be set up
    vTaskDelay(pdMS_TO_TICKS(200));

    float temp;

    float accel[3] = {0}, filt_accel[3] = {0};
    float gyro[3] = {0}, filt_gyro[3] = {0};

    // Cooldown and direction for the gyroscope gesture detection
    TickType_t cooldown = 0;
    int8_t command = 0;

    // Cooldown and orientation for the position detection
    int8_t last_orientation = 0;
    TickType_t orientation_cooldown = 0;

    while (1) {
        /* Make sure to only run the task while in the INPUT state and when theres no music playing
         to not interfeer with other sensors or corrupting the I2C bus 

         Detection while music is playing might work, but needs further testing.
        */
        if(get_status() != INPUT || g_state.playing_music) {
            vTaskDelay(300);
            continue;
        }
        
        // Read the raw data from the IMU
        if (ICM42670_read_sensor_data(&accel[0], &accel[1], &accel[2], &gyro[0], &gyro[1], &gyro[2], &temp) != 0) {
            debug("__Failed to read imu data__\n");
            vTaskDelay(pdMS_TO_TICKS(TASK_DELAY));
            continue;
        }

        // Apply Exponential Moving Average (EMA) to remove noise
        for(int i = 0; i < 3; i++) {
            filt_accel[i] = ALPHA * accel[i] + (1.0f - ALPHA) * filt_accel[i];

            filt_gyro[i] = ALPHA * gyro[i] + (1.0f - ALPHA) * filt_gyro[i];
        }

        TickType_t now = xTaskGetTickCount();

        /* Check device orientation using accelerometer data. Y axis will be 1 when pointed down and -1 when pointed up
           The cooldown is used to ensure that a change in the orientation is detected only once and after holding the
           device in that position for enough time (ORIENT_COOLDOWN_MS) */
        if(accel[1] > 0.96) {
            if(last_orientation != 1) {
                last_orientation = 1;
                cooldown = now;
            }
            else if(cooldown != 0 && now - cooldown > pdMS_TO_TICKS(ORIENT_DURATION_MS)) {
                cooldown = 0;
                play_sound(DOWN_MUSIC);
            }
        }
        else if(accel[1] < -0.96) {
            if(last_orientation != -1) {
                last_orientation = -1;
                cooldown = now;
            }
            else if(cooldown != 0 && now - cooldown > pdMS_TO_TICKS(ORIENT_DURATION_MS)) {
                cooldown = 0;
                play_sound(UP_MUSIC);
            }
        }
        else if(last_orientation != 0) {
            last_orientation = 0;
            orientation_cooldown = 0;
        }

        // If the debug option is enabled in the settings, print the filtered data from the IMU
        if(g_state.settings.debug) {
            printf("__Accel: X=%f, Y=%f, Z=%f | Gyro: X=%f, Y=%f, Z=%f, temp:%f__\n",
                    filt_accel[0], filt_accel[1], filt_accel[2], filt_gyro[0], filt_gyro[1], filt_gyro[2], temp);
        }

        int axis = get_dominant_axis(filt_gyro);

        // Check the cooldown for gestures and ensure no other gestures are ran while the cooldown is active
        if(cooldown != 0) {
            if(now - cooldown >= pdMS_TO_TICKS(COOLDOWN_MS))
                cooldown = 0;
            else {
                vTaskDelay(pdMS_TO_TICKS(TASK_DELAY));
                continue;
            }
        }
        
        // Detect if a tilting motion happened by checking if the gyroscope value is above a set threshold in the dominant axis
        // The graphs used for determening the magnitude and other parameters are found in the /assets folder
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