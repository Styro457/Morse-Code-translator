
#include <stdio.h>
#include <string.h>

#include <pico/stdlib.h>

#include <FreeRTOS.h>
#include <queue.h>
#include <task.h>

#include "buzzer.h"
#include "tkjhat/sdk.h"
#include "uart.h"

#include "buttons.h"
#include "interface.h"
#include "imu.h"
#include "state.h"

// Default stack size for the tasks. It can be reduced to 1024 if task is not
// using lot of memory.
#define DEFAULT_STACK_SIZE 2048

static void example_task(void *arg) {
  (void)arg;

  for (;;) {
    tight_loop_contents(); // Modify with application code here.
    vTaskDelay(pdMS_TO_TICKS(2000));
  }
}

int main() {
  stdio_init_all();
  // Uncomment this lines if you want to wait till the serial monitor is
  // connected
  /*while (!stdio_usb_connected()){
      sleep_ms(10);
  }*/
  while (!stdio_usb_connected()){
    sleep_ms(10);
  }
  init_hat_sdk();
  

  state_init();

      // Setting up the sensor. 
    if (init_ICM42670() == 0) {
        printf("ICM-42670P initialized successfully!\n");
        if (ICM42670_start_with_default_values() != 0){
            printf("ICM-42670P could not initialize accelerometer or gyroscope");
        }
    } else {
        printf("Failed to initialize ICM-42670P.\n");
    }

  sleep_ms(300); // Wait some time so initialization of USB and hat is done.

  TaskHandle_t displayTask, buzzerTask, receiveTask, imuTask;
  // Create the tasks with xTaskCreate

  // Buzzer task
  BaseType_t result = xTaskCreate(buzzer_task, // (en) Task function
                       "buzzer",    // (en) Name of the task
                       1024, // (en) Size of the stack for this task (in words).
                             // Generally 1024 or 2048
                       NULL, // (en) Arguments of the task
                       2,    // (en) Priority of this task
                       &buzzerTask);

  // Display task
  result = xTaskCreate(display_task, "display", DEFAULT_STACK_SIZE, NULL, 2,
                       &displayTask);

  if (result != pdPASS) {
    printf("Display Task creation failed \n");
    return 0;
  }

  result = xTaskCreate(receive_task, "receiveTask", DEFAULT_STACK_SIZE, NULL, 5,
                       &receiveTask);

  if (result != pdPASS) {
    printf("Receive Task creation failed \n");
    return 0;
  }

  result = xTaskCreate(imu_task, "imuTask", DEFAULT_STACK_SIZE, NULL, 4,
                       &imuTask);

  if (result != pdPASS) {
    printf("IMU Task creation failed \n");
    return 0;
  }

  // Start the scheduler (never returns)
  vTaskStartScheduler();

  // Never reach this line.
  return 0;
}
