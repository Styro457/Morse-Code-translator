#include <pico/stdlib.h>

/**
 * Main function for the Display Task.
 * 
 * Handles optimized updates and button presses and calls the appropiate functions for rendering the user interface.
 */
void display_task(void *arg);

/**
 * Called when a button action is detected.
 * @param button The button pressed, `1` for left, `2` for right
 * @param hold `true` if the button was held, `false` if it was a simple press
 */
void button_press(uint8_t button, bool hold);

// Trigger an update of the interface.
void update_interface();

// Update the local variables used for showing the message history.
// Needs to be called everytime the history is updated.
void update_interface_message_history();