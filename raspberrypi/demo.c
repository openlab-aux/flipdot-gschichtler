#include "flipdot.h"
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

void signal_handler(int signum) {
    flipdot_deinit();
    exit(EXIT_SUCCESS);
}

int main(void)
{
    flipdot_init();
   
    signal(SIGKILL, signal_handler);
    signal(SIGQUIT, signal_handler);
    signal(SIGTERM, signal_handler);
    signal(SIGINT,  signal_handler);

    uint8_t data[80*16/8];
    uint8_t d = 0; 
    while (1) {
        int i;
        for (i = 0; i < sizeof(data); i++) {
            data[i] = d;
        }
        //d = 255 - d;
        d++;
        flipdot_data(data, sizeof(data));
        //volatile uint32_t x;
        //for(x=0; x<10000000; x++);
        //for(x=0; x<10000000; x++);
        //for(x=0; x<10000000; x++);
        //for(x=0; x<10000000; x++);
    }
    exit(EXIT_SUCCESS);
}
