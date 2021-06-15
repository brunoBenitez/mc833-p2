/*
** client.c -- a stream socket client demo
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include "comm.h"
#include "utils.h"
#include <time.h> # Usar struct timespec

#include <arpa/inet.h>

#define PORT "3490" // the port client will be connecting to 

#define MAXDATASIZE 4098 // max number of bytes we can get at once 

char server_ip[MAX_CHARS];

void set_server_ip(void){
	for(int i = 0; i < MAX_CHARS; i++){
		server_ip[i] = '\0';
	}
	printf("Por favor, insira o endereco IP do servidor antes de iniciar as operacoes:\n");
	fgetss(server_ip, MAX_CHARS, stdin);
	return;
}

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

UserProfile *client_connect(ProtocolData comando, UserProfile prof_buf, int *n_profiles)
{
	int sockfd;
	struct addrinfo hints, *servinfo, *p;
	char buf[MAXDATASIZE];
	int rv;
	int numbytes;
	struct timespec timeout;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET; 
	hints.ai_socktype = SOCK_DGRAM;

	if ((rv = getaddrinfo(server_ip, PORT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and make a socket
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("talker: socket");
			continue;
		}

		break;
	}

	if (p == NULL) {
		fprintf(stderr, "talker: failed to create socket\n");
		return 2;
	}

	/* //Send do Beejs
	if ((numbytes = sendto(sockfd, buf, strlen(buf), 0,
			 p->ai_addr, p->ai_addrlen)) == -1) {
		perror("talker: sendto");
		exit(1);
	}

	freeaddrinfo(servinfo);

	printf("talker: sent %d bytes to %s\n", numbytes, server_ip);
	close(sockfd);*/

	// Definir timeout

	timeout.tv_sec = 0;
  	timeout.tv_nsec = 100000000;
  	setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

	// Envio de comandos

	int protocol_bytes = sizeof(ProtocolData);
	ProtocolData data;
	data.op = htonl(comando.op);
	data.profiles_num = htonl(comando.profiles_num);
	memcpy(buf, &data, protocol_bytes);

	int profile_bytes = sizeof(UserProfile);
	prof_buf.ano_formatura = htonl(prof_buf.ano_formatura);
	prof_buf.n_experiencia = htonl(prof_buf.n_experiencia);
	memcpy(buf+protocol_bytes, &prof_buf, profile_bytes);

	/* // TCP
	int sent_bytes;
	int total_sent = 0;
	do{
		if ((sent_bytes = send(sockfd, buf+total_sent, protocol_bytes + profile_bytes - total_sent, 0)) == -1){
			perror("send");
			return NULL;
		} else{
			total_sent += sent_bytes;
		}
	} while(total_sent < profile_bytes + protocol_bytes);*/

	// Enviar comandos por UDP

	int sent_bytes = 0;
	if ((sent_bytes = sendto(sockfd, (const char *)buf, MAXDATASIZE, MSG_CONFIRM, 
		(struct sockaddr*) NULL, sizeof(struct sockaddr))) == -1) // Arrumar endereco do servidor
  	{
		perror("ERROR: sendto");
		exit(1);
  	}
	else if (sent_bytes == 0)
	{
		printf("ERROR: socket indisponivel\n");
		exit(1);
	}

	// Recebimento da resposta inicial (ProtocolData)
	int total_recv = 0;

	do{
		if ((numbytes = recv(sockfd, buf+total_recv, protocol_bytes-total_recv, 0)) == -1) {
	    	perror("recv");
	    	return NULL;
		} else{
			total_recv += numbytes;
		}
	} while(total_recv < protocol_bytes);
	
	ProtocolData resposta;
	UserProfile *profile_list;

	// Capturando protocol data resposta
	memcpy(&resposta, buf, protocol_bytes);
	resposta.op = ntohl(resposta.op);
	resposta.profiles_num = ntohl(resposta.profiles_num);
	
	// Se nao for READ, capturar se houve sucesso ou erro na operacao, e encerrar
	if (comando.op != READ || resposta.op == ERROR)
	{
		*n_profiles = resposta.op; // ERROR ou SUCCES neste caso
		close(sockfd);
		return NULL;
	}

	// Sabemos quantos profiles vao chegar atraves de resposta.profiles_num
	profile_list = (UserProfile*)malloc(profile_bytes * resposta.profiles_num); // Alocar a lista
	*n_profiles = resposta.profiles_num;

	// Receber N Profiles
	for (size_t i = 0; i < resposta.profiles_num; i++)
	{
		total_recv = 0;
		do{
			if ((numbytes = recv(sockfd, buf+total_recv, profile_bytes-total_recv, 0)) == -1) {
	    		perror("recv");
	    		return NULL;
			} else{
				total_recv += numbytes;
			}
		} while(total_recv < profile_bytes);

		// Transferir para a lista
		memcpy(&(profile_list[i]), buf, profile_bytes);
		profile_list[i].ano_formatura = ntohl(profile_list[i].ano_formatura);
		profile_list[i].n_experiencia = ntohl(profile_list[i].n_experiencia);
	}

	close(sockfd);

	return profile_list;
}

/*Exemplos*/

int receive_data(int socket, char *buffer)
{
  buffer[0] = 'x';
  while (buffer[0] != '\0')
  { // print all messages
    if (read_udp(socket, buffer, servaddr, &len) < 0)
      return -1;
    printf("%s\n", buffer);
  }

  return 0;
}

int read_udp(int socket, char *buffer, sap sender, int *sender_len)
{
  int r_val, total = 0;

  while (total != BUFFLEN)
  {
    if ((r_val = recvfrom(socket, &buffer[total], (BUFFLEN - total), MSG_WAITALL,
                          sender, sender_len)) == -1)
    {
      if (errno != 11)
      {
        printf("ERROR: message might be lost/corrupted.\n");
        return -1;
      }
      else
      {
        printf("\nCLIENT: reached timeout (package might have been lost).\n");
        return -1;
      }
    }
    else if (r_val == 0)
    { // if client not responding
      printf("ERROR: pairing socket is closed\n");
      return -1;
    }
    else
    {
      total += r_val;
    }
  }

  return total;
}
