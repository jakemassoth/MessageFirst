#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "include/messagefirst_api.h"

void error_cb(int socket, struct mf_msg *msg, mf_error_t err) {
    fprintf(stderr, "This is from the callback!\n");
    mf_error_print(err);
}

mf_error_t success_cb(int socket, struct mf_msg *msg) {
    return mf_send_msg_response(socket, msg);
}

int main(void) {
    int SERVER_PORT = 8877;

    struct sockaddr_in server_address;
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;

    server_address.sin_port = htons(SERVER_PORT);

    server_address.sin_addr.s_addr = htonl(INADDR_ANY);

    int listen_sock;
    if ((listen_sock = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        printf("could not create listen socket\n");
        return 1;
    }

    if ((bind(listen_sock, (struct sockaddr *)&server_address,
              sizeof(server_address))) < 0) {
        printf("could not bind socket\n");
        return 1;
    }

    int wait_size = 16;
    if (listen(listen_sock, wait_size) < 0) {
        printf("could not open socket for listening\n");
        return 1;
    }

    struct sockaddr_in client_address;
    int client_address_len = 0;
    struct mf_ctx ctx;
    ctx.error_cb = error_cb;
    ctx.success_cb = success_cb;

    int sock;
    if ((sock = accept(listen_sock, (struct sockaddr *)&client_address, (socklen_t *) &client_address_len)) < 0) {
        printf("could not open a socket to accept data\n");
        return 1;
    }

    int res = mf_poll(sock, &ctx);

    close(sock);

    close(listen_sock);
    return res;
}
