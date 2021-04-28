#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <include/messagefirst_api.h>

void error_cb(int socket, struct mf_msg *msg, mf_error_t err) {
    fprintf(stderr, "This is from the callback!\n");
    mf_error_print(err);
}

mf_error_t recv_cb(int socket, struct mf_msg *msg) {
    assert(strcmp(msg->data, "12345678") == 0);
    return MF_ERROR_OK;
}

int main(void) {
    const char* server_name = "localhost";
    const int server_port = 8877;

    struct sockaddr_in server_address;
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;

    inet_pton(AF_INET, server_name, &server_address.sin_addr);

    server_address.sin_port = htons(server_port);

    int sock;
    if ((sock = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        printf("could not create socket\n");
        return 1;
    }

    if (connect(sock, (struct sockaddr*)&server_address,
                sizeof(server_address)) < 0) {
        printf("could not connect to server\n");
        return 1;
    }

    struct mf_ctx ctx;
    struct mf_msg msg;
    msg.len = strlen("12345678");
    strcpy(msg.data, "12345678");
    ctx.error_cb = error_cb;
    ctx.recv_cb = recv_cb;
    int res = -1;

    for (int i = 0; i < 1000; i++) {
        res = mf_send_msg(sock, &msg, &ctx);
    }
    close(sock);
    return res;
}
