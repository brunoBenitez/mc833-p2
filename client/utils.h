#pragma once
#include "comm.h"

#include <stdio.h>

typedef enum _AccountType
{
    ADMIN,
    USER
} AccountType;

char *fgetss(char *dest, int n, FILE *stream);

// show an option menu with choice by number and returns a char with the user input
int showMenu(AccountType accountType);

// handles user login and return its account type (admin or user)
AccountType login(void);

// Makes a connection to the server, sending a request and receiving its response
// Parameters:
//  comando: the basic data to request to server
//  prof_buf: profile to send to buffer when comando.op == CREATE or comando.op == UPDATE
//  n_profile: receive the number of profiles returned by this function when comando.op == READ
// Return:
//  a vector of UserProfiles that were read from the server when comando.op == READ with success. 
//  otherwise returns NULL.
UserProfile *client_connect(ProtocolData comando, UserProfile prof_buf, int *n_profiles);

// Set the Server IP
void set_server_ip(void);