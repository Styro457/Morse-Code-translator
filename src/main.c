/* Module 2 Final Project, Tier 3

Project made by:

Ke Wu
Radu Ursache
Sultan Alshehry
*/

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

int main() {
  stdio_init_all();
  // Uncomment this lines if you want to wait till the serial monitor is
  // connected
  /*while (!stdio_usb_connected()){
      sleep_ms(10);
  }*/
  init_hat_sdk();
  
  state_init();

  sleep_ms(300); // Wait some time so initialization of USB and hat is done.

  TaskHandle_t displayTask, buzzerTask, receiveTask, imuTask;
  // Create the tasks with xTaskCreate
  BaseType_t result;

  // Buzzer task
  result = xTaskCreate(buzzer_task, // (en) Task function
                       "buzzer",    // (en) Name of the task
                       1024, // (en) Size of the stack for this task (in words).
                             // Generally 1024 or 2048
                       NULL, // (en) Arguments of the task
                       6,    // (en) Priority of this task
                       &buzzerTask);

  // Display task
  result = xTaskCreate(display_task, "display", DEFAULT_STACK_SIZE, NULL, 2,
                       &displayTask);

  if (result != pdPASS) {
    debug("__Display Task creation failed__\n");
    return 0;
  }

  // Receive task
  result = xTaskCreate(receive_task, "receiveTask", DEFAULT_STACK_SIZE, NULL, 4,
                       &receiveTask);

  if (result != pdPASS) {
    debug("__Receive Task creation failed__\n");
    return 0;
  }

  // IMU task
  result = xTaskCreate(imu_task, "imuTask", DEFAULT_STACK_SIZE, NULL, 3,
                       &imuTask);

  if (result != pdPASS) {
    debug("IMU Task creation failed \n");
    return 0;
  }

  // Start the scheduler (never returns)
  vTaskStartScheduler();

  // Never reach this line.
  return 0;
}
