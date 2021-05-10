#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <include/messagefirst_api.h>

#define NUM_CLIENTS 6
#define NUM_MSG 10

void error_cb(int socket, struct mf_msg *msg, mf_error_t err) {
    mf_error_print(err);
}

mf_error_t recv_cb(int socket, struct mf_msg *msg) {
    printf("recv callback!\n");
    assert(strcmp(msg->data, "12345678") == 0);
    return MF_ERROR_OK;
}

void *thread_function(void *dummy) {
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
        return (void *) 1;
    }
    printf("socket fd: %d\n", sock);

    if (connect(sock, (struct sockaddr*)&server_address, sizeof(server_address)) < 0) {
        printf("could not connect to server\n");
        return (void *) 1;
    }

    struct mf_ctx ctx;
    struct mf_msg msg;
    memset(&ctx, 0, sizeof(struct mf_ctx));
    memset(&msg, 0, sizeof(struct mf_msg));
    msg.len = strlen("12345678");
    strcpy(msg.data, "12345678");
    printf("msg len: %d, content %s\n", msg.len, msg.data);
    ctx.error_cb = error_cb;
    ctx.recv_cb = recv_cb;

    for (int i = 0; i < NUM_MSG; i++) {
        printf("message num %d\n", i);
        int res = mf_send_msg(sock, &msg, &ctx);

        if (res != 0) {
            return (void *) 1;
        }
    }

    close(sock);

    return (void *) 0;
}

int main(void) {
    pthread_t thread_id[NUM_CLIENTS];

    for (int i = 0; i < NUM_CLIENTS; i++) {
        pthread_create(&thread_id[i], NULL, thread_function, NULL);
        printf("Created thread %lu\n", thread_id[i]);
    }

    for (int i = 0; i < NUM_CLIENTS; i++) {
        int ret;
        pthread_join(thread_id[i], (void **) &ret);
        printf("thread %lu terminated with exit code %d\n", thread_id[i], ret);
    }
}
