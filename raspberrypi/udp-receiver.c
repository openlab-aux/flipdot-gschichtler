#include "flipdot.h"
#include "flipdot_net.h"
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <signal.h>

void signal_handler(int signum) {
    flipdot_deinit();
    exit(EXIT_SUCCESS);
}

int main(void)
{

    flipdot_net_init();
    flipdot_init();

    signal(SIGKILL, signal_handler);
    signal(SIGQUIT, signal_handler);
    signal(SIGTERM, signal_handler);
    signal(SIGINT,  signal_handler);

    uint8_t data[(16*80)/8];
    while (1) {
        int n = flipdot_net_recv_frame((uint8_t *)data, sizeof(data));

        /*printf("got %u bytes\n", n);*/
        if(n >= sizeof(data)) {
            flipdot_data(data, sizeof(data));
            n -=  sizeof(data);
        } else {
            flipdot_data(data, n);
        }
    }
    return 0;
}
