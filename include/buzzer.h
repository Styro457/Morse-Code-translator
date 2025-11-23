// Sounds that the buzzer can play
typedef enum {
    MESSAGE_RECEIVED,
    MESSAGE_SENT,
    MUSIC,
    UP_MUSIC,
    DOWN_MUSIC,
    DOT_SOUND,
    LINE_SOUND,
    SPACE_SOUND,
    MENU_SOUND,
    ERROR_SOUND
} Sound;

/**
 * Play a sound on the buzzer. Use the Sound enum for it.
 * This function adds the sound in a queue
 */
void play_sound(Sound sound);

// Main function for the Buzzer Task.
// Includes the logic for all the sound enums which are played from a queue.
void buzzer_task(void *arg);