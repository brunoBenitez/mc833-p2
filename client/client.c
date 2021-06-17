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
#include <time.h>

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

UserProfile *client_connect(ProtocolData comando, UserProfile prof_buf, int *n_profiles)
{
	int sockfd;
	struct addrinfo hints, *servinfo, *p;
	struct sockaddr_storage their_addr; // sender's address information
    socklen_t addr_len;
	char buf[MAXDATASIZE];
	int rv;
	int numbytes;
	struct timeval timeout;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET; 
	hints.ai_socktype = SOCK_DGRAM;

	if ((rv = getaddrinfo(server_ip, PORT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return NULL;
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
		return NULL;
	}

	// Definir timeout

	timeout.tv_sec = 2;
  	timeout.tv_usec = 0;
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

	// Enviar comandos por UDP

	int sent_bytes = 0;
	if ((sent_bytes = sendto(sockfd, (const char *)buf, MAXDATASIZE, MSG_CONFIRM, 
		p->ai_addr, p->ai_addrlen)) == -1)
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
	if ((numbytes = recvfrom(sockfd, buf, protocol_bytes, MSG_WAITALL, (struct sockaddr *)&their_addr, &addr_len)) == -1) {
		if (errno == EAGAIN)
        {
			printf("Tempo esgotado esperando receber resposta do servidor\n");
            return NULL;
        }
		else
			perror("recv protocol");
		return NULL;
	}

	
	ProtocolData resposta;
	UserProfile *profile_list;

	// Capturando protocol data resposta
	memcpy(&resposta, buf, protocol_bytes);
	resposta.op = ntohl(resposta.op);
	resposta.profiles_num = ntohl(resposta.profiles_num);
	
	// Se nao for READ, capturar se houve sucesso ou erro na operacao, e encerrar
	if (comando.op != READ || resposta.op != SUCCESS)
	{
		*n_profiles = resposta.op; // ERROR ou SUCCES neste caso
		close(sockfd);
		return NULL;
	}

	// Sabemos quantos profiles vao chegar atraves de resposta.profiles_num
	profile_list = (UserProfile*)malloc(profile_bytes * resposta.profiles_num); // Alocar a lista

	// Receber N Profiles
	int total_recvd = 0;
	for (size_t i = 0; i < resposta.profiles_num; i++)
	{
		if ((numbytes = recvfrom(sockfd, buf, profile_bytes, MSG_WAITALL, (struct sockaddr *)&their_addr, &addr_len)) == -1) {
			if (errno != EAGAIN)
				perror("recv users");
			continue;
		}

		// Transferir para a lista
		memcpy(&(profile_list[total_recvd]), buf, profile_bytes);
		profile_list[total_recvd].ano_formatura = ntohl(profile_list[total_recvd].ano_formatura);
		profile_list[total_recvd].n_experiencia = ntohl(profile_list[total_recvd].n_experiencia);
		total_recvd++;
	}

	if (total_recvd < resposta.profiles_num)
		printf("foram perdidos %d perfis, por favor tente novamente\n", resposta.profiles_num - total_recvd);

	*n_profiles = total_recvd;

	close(sockfd);

	return profile_list;
}
