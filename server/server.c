#include "server.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "comm.h"
#include "database.h"

static void send_success(int sock_fd, struct sockaddr *send_addr, socklen_t addr_len)
{
    ProtocolData data;
    size_t rv, sent_size = 0;
    data.op = htonl(SUCCESS);
    data.profiles_num = 0;
    rv = sendto(sock_fd, &data, sizeof(ProtocolData), 0, send_addr, addr_len);
    if (rv == -1) {
        perror("send_success");
        return;
    }
    printf("SUCCESS sent\n");
}

static void send_error(int sock_fd, struct sockaddr *send_addr, socklen_t addr_len)
{
    ProtocolData data;
    size_t rv, sent_size = 0;
    data.op = htonl(ERROR);
    data.profiles_num = 0;
    rv = sendto(sock_fd, &data, sizeof(ProtocolData), 0, send_addr, addr_len);
    if (rv == -1) {
        perror("send_error");
        return;
    }
    printf("ERROR sent\n");
}

static void send_profiles(int sock_fd, UserProfile *profs, int n_profs, struct sockaddr *send_addr, socklen_t addr_len)
{
    size_t send_size = sizeof(ProtocolData) + n_profs * sizeof(UserProfile);
    uint8_t *send_buf = malloc(send_size);
    ProtocolData data;
    data.op = htonl(SUCCESS);
    data.profiles_num = htonl(n_profs);

    memcpy(send_buf, &data, sizeof(ProtocolData));
    memcpy(send_buf + sizeof(ProtocolData), profs, n_profs * sizeof(UserProfile));

    size_t rv, sent_size = 0;
    rv = sendto(sock_fd, send_buf, send_size, 0, send_addr, addr_len);
    if (rv == -1)
        perror("send_profiles");

    if (rv == send_size)
        printf("%u profiles sent successfully!\n", n_profs);

    free(send_buf);
}

void handle_request(int sock_fd, void *recvd_datagram, size_t numBytes, struct sockaddr *send_addr, socklen_t addr_len);
{
    size_t recv_size, recvd_size, rv;
    int n_profs;
    ProtocolData *req_data;
    UserProfile *profile_buf;

    recv_size = sizeof(ProtocolData) + sizeof(UserProfile);

    req_data = (ProtocolData *)recvd_datagram;
    profile_buf = (UserProfile *)(recvd_datagram + sizeof(ProtocolData));

    // Should all cases send response?
    printf("Received operation: %u\n", ntohl(req_data->op));
    switch (ntohl(req_data->op))
    {
    case CREATE:
        if (create_user(profile_buf))
            send_success(sock_fd, send_addr, addr_len);
        else
            send_error(sock_fd, send_addr, addr_len);
        break;
    case READ:
        n_profs = read_db(&profile_buf);
        if (n_profs > 0)
            send_profiles(sock_fd, profile_buf, n_profs, send_addr, addr_len);
        else
            send_error(sock_fd, send_addr, addr_len);
        break;
    case UPDATE:
        if (update_user(profile_buf))
            send_success(sock_fd, send_addr, addr_len);
        else
            send_error(sock_fd, send_addr, addr_len);
        break;
    case DELETE:
        if (delete_user(profile_buf))
            send_success(sock_fd, send_addr, addr_len);
        else
            send_error(sock_fd, send_addr, addr_len);
        break;

    default:
        fprintf(stderr, "Operação inválida recebida: %u\n", ntohl(req_data->op));
        break;
    }
}