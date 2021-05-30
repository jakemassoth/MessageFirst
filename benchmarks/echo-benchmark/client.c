#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <include/messagefirst_api.h>
#include <sys/time.h>
#include <signal.h>

int total;
int sock;

void timer_handler(int signum) {
    printf("%d\n", total);
    close(sock);
    exit(0);
}

int main(int argc, char *argv[]) {
    assert(argc == 5);
    const char* server_name = argv[1];
    const int server_port = atoi(argv[2]);
    const int data_len = atoi(argv[3]);
    const char *data = argv[4];


    struct sockaddr_in server_address;
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;

    inet_pton(AF_INET, server_name, &server_address.sin_addr);

    server_address.sin_port = htons(server_port);

    if ((sock = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket()");
        return 1;
    }

    if (connect(sock, (struct sockaddr*)&server_address, sizeof(server_address)) < 0) {
        perror("connect()");
        return 1;
    }

    int timeout = -1;

    struct mf_msg msg;
    struct mf_msg msg_recv;

    memset(&msg, 0, sizeof(struct mf_msg));
    msg.len = data_len;
    memcpy(msg.data, data, data_len);

    struct itimerval timer;
    struct sigaction sa;

    memset(&sa, 0, sizeof(sa));

    sa.sa_handler = &timer_handler;

    sigaction(SIGALRM, &sa, NULL);

    timer.it_value.tv_sec = 30;
    timer.it_value.tv_usec = 0;

    timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_usec = 0;

    setitimer(ITIMER_REAL, &timer, NULL);
    int res;

    for (;;) {
        memset(&msg_recv, 0, sizeof(struct mf_msg));
        res = mf_send_msg(sock, &msg, &msg_recv, timeout);

        if (res != 0) {
           goto cleanup;
        }
        // sanity checks
//        fprintf(stderr, "%d\n", msg_recv.len);
        assert(msg_recv.len == msg.len);
        assert(strncmp(msg_recv.data, msg.data, msg.len) == 0);

        total++;
    }

    cleanup:

    close(sock);

    return res;
}
