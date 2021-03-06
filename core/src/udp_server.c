#include "udp_server.h"
#include "utils.h"
#include "net_util.h"
#include "udp_util.h"
#include "udp_constants.h"

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

int listening_udp_server;
int udp_server_socket_fd;

void run_udp_server();

void udp_server_start() {
    if (listening_udp_server) return;
    listening_udp_server = 1;
    pthread_t server_thread;
    pthread_create(&server_thread, NULL, run_udp_server, NULL);
}

void run_udp_server() {
    udp_server_socket_fd = udp_listen(UDP_SERVER_PORT);
    if (udp_server_socket_fd > 0) {
        if (udp_join_broadcast_group(udp_server_socket_fd, UDP_BROADCAST_ADDRESS) != -1) {
            while (listening_udp_server) {
                struct udp_recv_result urr = udp_recv(udp_server_socket_fd);
                char *data = NULL, *prefix = NULL;
                if (str_eq(urr.data, STR_REQ_MAC_ADDRESS)) {
                    data = local_mac_address();
                    prefix = STR_RSP_MAC_ADDRESS;
                }
                if (data && prefix) {
                    char* result = (char*)malloc((strlen(prefix) + strlen(data)) * sizeof(char));
                    strcpy(result, prefix);
                    strcat(result, data);
                    udp_send(udp_server_socket_fd, urr.addr, result);
                    free(data);
                    free(result);
                }
            }
            close(udp_server_socket_fd);
        }
    }
}

void udp_server_stop() {
    listening_udp_server = 0;
}
