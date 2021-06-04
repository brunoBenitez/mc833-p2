#include "server.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "comm.h"
#include "database.h"

static void send_success(int sock_fd)
{
    ProtocolData data;
    size_t rv, sent_size = 0;
    data.op = htonl(SUCCESS);
    data.profiles_num = 0;
    do {
        rv = send(sock_fd, &data + sent_size, sizeof(ProtocolData) - sent_size, 0);
        if (rv == -1) {
            perror("send_success");
            return;
        } else {
            sent_size += rv;
        }
    } while (sent_size < sizeof(ProtocolData));
    printf("SUCCESS sent\n");
}

static void send_error(int sock_fd)
{
    ProtocolData data;
    size_t rv, sent_size = 0;
    data.op = htonl(ERROR);
    data.profiles_num = 0;
    do {
        rv = send(sock_fd, &data + sent_size, sizeof(ProtocolData) - sent_size, 0);
        if (rv == -1) {
            perror("send_error");
            return;
        } else {
            sent_size += rv;
        }
    } while (sent_size < sizeof(ProtocolData));
    printf("ERROR sent\n");
}

static void send_profiles(int sock_fd, UserProfile *profs, int n_profs)
{
    size_t send_size = sizeof(ProtocolData) + n_profs * sizeof(UserProfile);
    uint8_t *send_buf = malloc(send_size);
    ProtocolData data;
    data.op = htonl(SUCCESS);
    data.profiles_num = htonl(n_profs);

    memcpy(send_buf, &data, sizeof(ProtocolData));
    memcpy(send_buf + sizeof(ProtocolData), profs, n_profs * sizeof(UserProfile));

    size_t rv, sent_size = 0;
    do {
        rv = send(sock_fd, send_buf + sent_size, send_size - sent_size, 0);
        if (rv == -1) {
            perror("send_profiles");
            break;
        } else {
            sent_size += rv;
        }
    } while (sent_size < send_size);

    if (sent_size == send_size)
        printf("%u profiles sent successfully!\n", n_profs);

    free(send_buf);
}

void handle_request(int sock_fd)
{
    size_t recv_size, recvd_size, rv;
    int n_profs;
    uint8_t *recv_buffer;
    ProtocolData *req_data;
    UserProfile *profile_buf;

    recv_size = sizeof(ProtocolData) + sizeof(UserProfile);
    recv_buffer = malloc(recv_size);
    recvd_size = 0;
    do {
        rv = recv(sock_fd, recv_buffer + recvd_size, recv_size - recvd_size, 0);
        if (rv == -1) {
            perror("recv");
            return;
        } else {
            recvd_size += rv;
        }
    } while (recvd_size < recv_size);

    req_data = (ProtocolData *)recv_buffer;
    profile_buf = (UserProfile *)(recv_buffer + sizeof(ProtocolData));

    // Should all cases send response?
    printf("Received operation: %u\n", ntohl(req_data->op));
    switch (ntohl(req_data->op))
    {
    case CREATE:
        if (create_user(profile_buf))
            send_success(sock_fd);
        else
            send_error(sock_fd);
        break;
    case READ:
        n_profs = read_db(&profile_buf);
        if (n_profs > 0)
            send_profiles(sock_fd, profile_buf, n_profs);
        else
            send_error(sock_fd);
        break;
    case UPDATE:
        if (update_user(profile_buf))
            send_success(sock_fd);
        else
            send_error(sock_fd);
        break;
    case DELETE:
        if (delete_user(profile_buf))
            send_success(sock_fd);
        else
            send_error(sock_fd);
        break;

    default:
        fprintf(stderr, "Operação inválida recebida: %u\n", ntohl(req_data->op));
        break;
    }
}