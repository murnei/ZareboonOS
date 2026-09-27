#ifndef TIME_H
#define TIME_H

struct time_now {
    uint8_t seconds;
    uint8_t minutes;
    uint8_t hours;
};

void get_time(struct time_now* t);
void sleep(uint32_t ms);

#endif
