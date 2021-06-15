#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <signal.h>

#include "comm.h"
#include "database.h"

#include "server.h"

static void send_success(int sock_fd, struct sockaddr *send_addr, socklen_t addr_len)
{
    ProtocolData data;
    size_t rv;
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
    size_t rv;
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
    size_t rv;
    ProtocolData data;
    data.op = htonl(SUCCESS);
    data.profiles_num = htonl(n_profs);

    rv = sendto(sock_fd, &data, sizeof(ProtocolData), 0, send_addr, addr_len);
    if (rv == -1)
        perror("send_profiles, data");

    for (int i = 0; i < n_profs; i++)
    {
        rv = sendto(sock_fd, &profs[i], sizeof(UserProfile), 0, send_addr, addr_len);
        if (rv == -1)
            perror("send_profiles");
    }

    printf("sent %d profiles!\n", n_profs);
}

void handle_request(int sock_fd, uint8_t *recvd_datagram, size_t numBytes, struct sockaddr *send_addr, socklen_t addr_len)
{
    int n_profs;
    ProtocolData *req_data;
    UserProfile *profile_buf;

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