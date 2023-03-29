#define START_FLAG 0xF0
#define END_FLAG 0XC5

int rc_serial_init(void);
void rc_serial_read_loop(void);