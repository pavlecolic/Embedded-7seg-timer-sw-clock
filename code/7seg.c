#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <sys/time.h>
#include <time.h>
#include <linux/input.h>
#include <sys/mman.h>
#include <stdlib.h>

#define LW_BRIDGE_BASE 0xFF200000
#define LW_BRIDGE_SPAN 0x00200000
#define GPIO_HEX0_BASE 0x60
#define GPIO_HEX1_BASE 0x50
#define GPIO_HEX2_BASE 0x40
#define GPIO_HEX3_BASE 0x30
#define GPIO_HEX4_BASE 0x20
#define GPIO_HEX5_BASE 0x10


unsigned char seg7[10] = {~0x3F, ~0x06, ~0x5B, ~0x4F, ~0x66, ~0x6D, ~0x7D, ~0x07, ~0x7F, ~0x6F};

// Postavljanje 7-seg displeja
void set_7seg_display(volatile unsigned int *gpio_base, unsigned char value) {
    *gpio_base = value;
}

volatile unsigned int *gpio_hex0, *gpio_hex1, *gpio_hex2, *gpio_hex3, *gpio_hex4, *gpio_hex5;

unsigned int sw0_on = 0;
unsigned int sw1_on = 0;

int mode = 0; // 0 for off, 1 for stopwatch, 2 for current time, 3 for countdown timer
int hundredths = 0;
int seconds = 0;
int minutes = 0;
int hours = 0;
int countdown_active = 0;
int stopwatch_paused = 0;
int timer_paused = 1;

void reset_timer() {
    hundredths = 0;
    seconds = 0;
    minutes = 0;
    hours = 0;
}


void display_stopwatch(int m, int s, int hs) {
    set_7seg_display(gpio_hex0, seg7[hs % 10]); // hundredths of a second units
    set_7seg_display(gpio_hex1, seg7[hs / 10]); // hundredths of a second tens
    set_7seg_display(gpio_hex2, seg7[s % 10]); // seconds units
    set_7seg_display(gpio_hex3, seg7[s / 10]); // seconds tens
    set_7seg_display(gpio_hex4, seg7[m % 10]); // minutes units
    set_7seg_display(gpio_hex5, seg7[m / 10]); // minutes tens
}

void display_current_time(int h, int m, int s) {
    set_7seg_display(gpio_hex0, seg7[s % 10]); // seconds units
    set_7seg_display(gpio_hex1, seg7[s / 10]); // seconds tens
    set_7seg_display(gpio_hex2, seg7[m % 10]); // minutes units
    set_7seg_display(gpio_hex3, seg7[m / 10]); // minutes tens
    set_7seg_display(gpio_hex4, seg7[h % 10]); // hours units
    set_7seg_display(gpio_hex5, seg7[h / 10]); // hours tens
}

void display_countdown_timer(int h, int m, int s) {
    set_7seg_display(gpio_hex0, seg7[s % 10]); 
    set_7seg_display(gpio_hex1, seg7[s / 10]); 
    set_7seg_display(gpio_hex2, seg7[m % 10]);
    set_7seg_display(gpio_hex3, seg7[m / 10]);
    set_7seg_display(gpio_hex4, seg7[h % 10]); 
    set_7seg_display(gpio_hex5, seg7[h / 10]); 
}

void timer_handler(int signum) {
    if(mode == 0) {
        reset_timer();
        display_current_time(hours, minutes, seconds);
    }
    if (mode == 1 && !stopwatch_paused) {
        // Stopwatch
        hundredths += 1;

        if (hundredths >= 100) {
            hundredths = 0;
            seconds += 1;
        }

        if (seconds >= 60) {
            seconds = 0;
            minutes += 1;
        }

        if (minutes >= 60) {
            minutes = 0;
        }

        display_stopwatch(minutes, seconds, hundredths);
    } else if (mode == 2) {
        // Trenutno vrijeme
        time_t now;
        struct tm *now_tm;
        int cur_hours, cur_minutes, cur_seconds;

        now = time(NULL);
        now_tm = localtime(&now);

        cur_hours = now_tm->tm_hour;
        cur_minutes = now_tm->tm_min;
        cur_seconds = now_tm->tm_sec;

        display_current_time(cur_hours, cur_minutes, cur_seconds);
    } else if (mode == 3 && !timer_paused) {
        // TAJMER
        if (hundredths == 0) {
            if (seconds == 0) {
                if (minutes == 0) {
                    if (hours == 0) {
                        // Timer reached zero, stop countdown
                        timer_paused = 1;
                    } else {
                        hours -= 1;
                        minutes = 59;
                        seconds = 59;
                        hundredths = 99;
                    }
                } else {
                    minutes -= 1;
                    seconds = 59;
                    hundredths = 99;
                }
            } else {
                seconds -= 1;
                hundredths = 99;
            }
        } else {
            hundredths -= 1;
        }

        display_countdown_timer(hours, minutes, seconds);
    }
}

void reset_stopwatch(int on) {
    hundredths = 0;
    seconds = 0;
    minutes = 0;
    if(on)
        stopwatch_paused = 0;
    else
        stopwatch_paused = 1;
    display_stopwatch(minutes, seconds, hundredths);
}

void change_mode() {
    if(!(sw0_on || sw1_on)) {
        mode = 0;
    } else if(sw0_on && !sw1_on) {
        mode = 1;
        reset_stopwatch(1);
    } else if(!sw0_on && sw1_on) {
        mode = 2;
    } else if(sw0_on && sw1_on) {
        mode = 3;
    }
}



void handle_event(struct input_event *ev) {
    if(ev->type == EV_KEY && ev->value == 1) {
        if(ev->code == KEY_A) {
            sw0_on = 1;
            change_mode();
        } else if(ev->code == KEY_S) {
            sw1_on = 1;
            change_mode();
        } else if (mode == 1) {
            if(ev->code == KEY_W)
                stopwatch_paused = !stopwatch_paused;
            else if(ev->code == KEY_E)
                reset_stopwatch(0);
        
        } else if(mode == 3) {
            if(timer_paused) {
                if(ev->code == KEY_W) {
                    timer_paused = 0;
                }
                else if (ev->code == KEY_T) {
                    hours = (hours + 1) % 24;
                } else if (ev->code == KEY_R) {
                    minutes = (minutes + 1) % 60;
                } else if (ev->code == KEY_E) {
                    seconds = (seconds + 1) % 60;
                }
                display_current_time(hours, minutes, seconds);
            } else {
                if(ev->code == KEY_W) {
                    timer_paused = 1;
                }
            }
        }
    } else if(ev->value == 0) {
        if(ev->code == KEY_A) {
            sw0_on = 0;
            change_mode();
        } else if(ev->code == KEY_S) {
            sw1_on = 0;
            change_mode();
        } 
    }
}

int main() {
    int fd;
    void *lw_bridge_base;
    struct itimerval timer;
    struct input_event ev;
    int ev_fd;

    if ((fd = open("/dev/mem", (O_RDWR | O_SYNC))) == -1) {
        perror("open /dev/mem");
        printf("Can't access phy");
        return 1;
    }

    lw_bridge_base = mmap(NULL, LW_BRIDGE_SPAN, (PROT_READ | PROT_WRITE), MAP_SHARED, fd, LW_BRIDGE_BASE);
    if (lw_bridge_base == MAP_FAILED) {
        perror("mmap");
        printf("Can't map HPS to FPGA");

        close(fd);
        return 1;
    }

    // GPIO adrese
    gpio_hex0 = (unsigned int *)(lw_bridge_base + GPIO_HEX0_BASE);
    gpio_hex1 = (unsigned int *)(lw_bridge_base + GPIO_HEX1_BASE);
    gpio_hex2 = (unsigned int *)(lw_bridge_base + GPIO_HEX2_BASE);
    gpio_hex3 = (unsigned int *)(lw_bridge_base + GPIO_HEX3_BASE);
    gpio_hex4 = (unsigned int *)(lw_bridge_base + GPIO_HEX4_BASE);
    gpio_hex5 = (unsigned int *)(lw_bridge_base + GPIO_HEX5_BASE);

    // Event interafece za gpio-keys-polled
    if ((ev_fd = open("/dev/input/event0", O_RDONLY)) == -1) {
        perror("open /dev/input/event0");
        if (munmap(lw_bridge_base, LW_BRIDGE_SPAN) != 0) {
            printf("Can't access event0");
            perror("munmap");
        }
        close(fd);
        return 1;
    }

    // Timer handler
    signal(SIGALRM, timer_handler);

    timer.it_value.tv_sec = 0;
    timer.it_value.tv_usec = 10000;
    timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_usec = 10000;

    // Start the timer
    if (setitimer(ITIMER_REAL, &timer, NULL) == -1) {
        perror("setitimer");
        if (munmap(lw_bridge_base, LW_BRIDGE_SPAN) != 0) {
            perror("munmap");
            printf("Can't access timer");


        }
        close(fd);
        return 1;
    }

    // Main loop
    while (1) {
        if (read(ev_fd, &ev, sizeof(struct input_event)) != -1) {
            handle_event(&ev);
        }
        pause();  // Wait for signals
    }

    if (munmap(lw_bridge_base, LW_BRIDGE_SPAN) != 0) {
        perror("munmap");
    }
    close(fd);
    close(ev_fd);
    return 0;
}

