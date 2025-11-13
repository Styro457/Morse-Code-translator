typedef enum {
    MESSAGE_RECEIVED,
    MESSAGE_SENT,
    JEEEEEE,
    MUSIC,
    WOOOOOHOOOOOO
} sound;

void buzzer_task(void *arg);
void play_sound(sound music);