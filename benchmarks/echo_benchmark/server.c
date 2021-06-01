#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <include/messagefirst_api.h>

void error_cb(int socket, struct mf_msg *msg, mf_error_t err) {
    mf_error_print(err);
}

struct mf_msg poll_resp(struct mf_msg msg) {
    return msg;
}

void timeout_cb(int socket, struct mf_msg *msg) {
    fprintf(stderr, "MessageFirst timeout on socket %d with msg content %s", socket, msg->data);
}

struct mf_ctx ctx;
int listen_sock;

void term(int signum) {
    printf("sigterm\n");
    close(listen_sock);
    mf_ctx_cleanup(&ctx);
}

int main(int argc, char *argv[]) {
    assert(argc == 2);

    struct sigaction action;
    memset(&action, 0, sizeof(action));
    action.sa_handler = term;
    sigaction(SIGTERM, &action, NULL);

    int SERVER_PORT = atoi(argv[1]);

    struct sockaddr_in server_address;
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;

    server_address.sin_port = htons(SERVER_PORT);

    server_address.sin_addr.s_addr = htonl(INADDR_ANY);

    if ((listen_sock = socket(PF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0)) < 0) {
        perror("socket()");
        return 1;
    }

    if ((bind(listen_sock, (struct sockaddr *)&server_address,
              sizeof(server_address))) < 0) {
        perror("bind()");
        return 1;
    }

    int wait_size = 32;
    if (listen(listen_sock, wait_size) < 0) {
        perror("listen()");
        return 1;
    }

    int timeout = -1;
    int num_threads = 1;
    if (mf_ctx_poll_init(&ctx, timeout, error_cb, timeout_cb, poll_resp, num_threads) != 0) {
        return -1;
    }

    int res = mf_poll(listen_sock, &ctx);

    close(listen_sock);
    mf_ctx_cleanup(&ctx);

    return res;
}
