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

void buzzer_task(void *arg);
void play_sound(Sound sound);