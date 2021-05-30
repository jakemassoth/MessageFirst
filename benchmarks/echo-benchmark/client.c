#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <include/messagefirst_api.h>

int main(int argc, char *argv[]) {
    assert(argc == 3);
    const char* server_name = argv[1];
    const int server_port = atoi(argv[2]);

    struct sockaddr_in server_address;
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;

    inet_pton(AF_INET, server_name, &server_address.sin_addr);

    server_address.sin_port = htons(server_port);

    int sock;
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
    msg.len = strlen("12345678");
    strcpy(msg.data, "12345678");

    for (int i = 0; i < 1000; i++) {
        memset(&msg_recv, 0, sizeof(struct mf_msg));
        int res = mf_send_msg(sock, &msg, &msg_recv, timeout);

        if (res != 0) {
            return 1;
        }

        assert(strncmp(msg_recv.data, msg.data, msg.len) == 0);
    }

    close(sock);

    return 0;
}
