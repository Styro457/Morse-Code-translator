// Initiate the semaphore for the buttons and the gpio irq callbacks.
void button_init();

// Run the logic for the button detection. It is being ran from the displayTask.
// This function makes the task wait 500ms for a button change.
void button_check();