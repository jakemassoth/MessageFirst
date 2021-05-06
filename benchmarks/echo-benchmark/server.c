#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <include/messagefirst_api.h>

int num = 0;

void error_cb(int socket, struct mf_msg *msg, mf_error_t err) {
    mf_error_print(err);
}

mf_error_t recv_cb(int socket, struct mf_msg *msg) {
    num++;
    assert(strcmp(msg->data, "12345678") == 0);
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
    if ((listen_sock = socket(PF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0)) < 0) {
        printf("could not create listen socket\n");
        return 1;
    }

    if ((bind(listen_sock, (struct sockaddr *)&server_address,
              sizeof(server_address))) < 0) {
        printf("could not bind socket\n");
        return 1;
    }

    int wait_size = 32;
    if (listen(listen_sock, wait_size) < 0) {
        printf("could not open socket for listening\n");
        return 1;
    }

    struct mf_ctx ctx;
    ctx.error_cb = error_cb;
    ctx.recv_cb = recv_cb;
    ctx.timeout = 10;
    ctx.tp = NULL;

    int res = mf_poll(listen_sock, &ctx);

    close(listen_sock);
    mf_ctx_cleanup(&ctx);

    printf("messages sent: %d\n", num);
    return res;
}
