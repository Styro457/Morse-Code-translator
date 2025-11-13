
#include <stdio.h>
#include <string.h>

#include <pico/stdlib.h>

#include <FreeRTOS.h>
#include <queue.h>
#include <task.h>

#include "tkjhat/sdk.h"
#include "buzzer.h"

#include "state.h"
#include "interface.h"
#include "buttons.h"

// Default stack size for the tasks. It can be reduced to 1024 if task is not using lot of memory.
#define DEFAULT_STACK_SIZE 2048 

//Add here necessary states
enum state { IDLE=1 };
enum state programState = IDLE;

static void example_task(void *arg){
    (void)arg;

    for(;;){
        tight_loop_contents(); // Modify with application code here.
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

int main() {
    stdio_init_all();
    // Uncomment this lines if you want to wait till the serial monitor is connected
    /*while (!stdio_usb_connected()){
        sleep_ms(10);
    }*/ 
    init_hat_sdk();

    state_init();

    sleep_ms(300); //Wait some time so initialization of USB and hat is done.


    TaskHandle_t myExampleTask, displayTask, buzzerTask;
    // Create the tasks with xTaskCreate
    BaseType_t result = xTaskCreate(example_task,       // (en) Task function
                "example",              // (en) Name of the task 
                DEFAULT_STACK_SIZE, // (en) Size of the stack for this task (in words). Generally 1024 or 2048
                NULL,               // (en) Arguments of the task 
                2,                  // (en) Priority of this task
                &myExampleTask);    // (en) A handle to control the execution of this task

    if(result != pdPASS) {
        printf("Example Task creation failed\n");
        return 0;
    }

    // Buzzer task
    result = xTaskCreate(buzzer_task,   // (en) Task function
            "buzzer",                   // (en) Name of the task 
                1024,                   // (en) Size of the stack for this task (in words). Generally 1024 or 2048
                NULL,                   // (en) Arguments of the task 
                2,                      // (en) Priority of this task
                &buzzerTask);
  
    // Display task
    result = xTaskCreate(display_task,
        "display",
        DEFAULT_STACK_SIZE,
        NULL,
        2,
        &displayTask
    );

    if (result != pdPASS) {
        printf("Display Task creation failed \n");
        return 0;
    }

    // Button task
    result = xTaskCreate(button_task,
        "button",
        DEFAULT_STACK_SIZE,
        NULL,
        1,
        &displayTask
    );

    if (result != pdPASS) {
        printf("Button Task creation failed \n");
        return 0;
    }

    // Start the scheduler (never returns)
    vTaskStartScheduler();

    // Never reach this line.
    return 0;
}

