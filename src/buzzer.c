#include "buzzer.h"
#include <stdio.h>

#include <pico/stdlib.h>

#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>

#include "tkjhat/sdk.h"
#include "state.h"

// Queue to add sounds to be playeed
QueueHandle_t queue;

int up_melody[] = {500, 600, 700, 800, 900, 1000, 1100, 1200};
int down_melody[] = {1200, 1100, 1000, 900, 800, 700, 600, 500};

int melody[] = {1318, 1318, 1318, 1100, 1318, 1500, 1000};
float durations[] = {250, 250, 250, 250, 250, 300, 300};

void play_sound(Sound sound){
    xQueueSend(queue, &sound, portMAX_DELAY);
}

void buzzer_task(void *arg) {
    (void)arg;

    //Initialize the buzzer
    init_buzzer();
    debug("Initializing buzzer\n");
    queue = xQueueCreate(5,sizeof(Sound));

    play_sound(MUSIC);

    while(1){

        Sound nextSound;
        if (xQueueReceive(queue, &nextSound, portMAX_DELAY) == pdPASS) {
            g_state.playing_music = true;
            switch (nextSound) {
                case MESSAGE_RECEIVED:
                    buzzer_play_tone (50, 50);
                    vTaskDelay(pdMS_TO_TICKS(60));
                    break;
                case MESSAGE_SENT:
                    buzzer_play_tone (800, 50);
                    vTaskDelay(pdMS_TO_TICKS(80));
                    buzzer_play_tone (300, 100);
                    vTaskDelay(pdMS_TO_TICKS(100));
                    break;
                case MUSIC:
                    for (int i=0 ; i<4; i++){
                        buzzer_play_tone (melody[i], durations[i]);
                        vTaskDelay(pdMS_TO_TICKS(durations[i]));
                    }
                    break;
                case UP_MUSIC:
                    for (int i=0 ; i<7; i++){
                        buzzer_play_tone (up_melody[i], 200);
                        vTaskDelay(pdMS_TO_TICKS(200));
                    }
                    break;
                case DOWN_MUSIC:
                    for (int i=0 ; i<7; i++){
                        buzzer_play_tone (down_melody[i], 200);
                        vTaskDelay(pdMS_TO_TICKS(200));
                    }
                    break;
                case DOT_SOUND:
                    buzzer_play_tone (800, 50);
                    vTaskDelay(pdMS_TO_TICKS(50));
                    break;
                case LINE_SOUND:
                    buzzer_play_tone (500, 200);
                    vTaskDelay(pdMS_TO_TICKS(200));
                    break;
                case SPACE_SOUND:
                    buzzer_play_tone (100, 100);
                    vTaskDelay(pdMS_TO_TICKS(100));
                    break;
                case MENU_SOUND:
                    buzzer_play_tone (800, 50);
                    vTaskDelay(pdMS_TO_TICKS(50));
                    break;
                case ERROR_SOUND:
                    buzzer_play_tone (50, 1000);
                    vTaskDelay(pdMS_TO_TICKS(1000));
                    break;
                default:
                    break;
            }
            g_state.playing_music = false;
        }
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}

