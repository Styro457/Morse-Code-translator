typedef enum {
    MESSAGE_RECEIVED,
    MESSAGE_SENT,
    MUSIC,
    DOT_SOUND,
    LINE_SOUND,
    MENU_SOUND,
    ERROR_SOUND
} Sound;

void buzzer_task(void *arg);
void play_sound(Sound sound);