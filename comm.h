#pragma once

#include <inttypes.h>

#define MAX_CHARS 128
#define MAX_EXP 16

typedef enum _operation {
    ERROR = 0,
    SUCCESS,
    CREATE,
    READ,
    UPDATE,
    DELETE
} Operation;

typedef struct _userProfile {
    char email[MAX_CHARS];
    char nome[MAX_CHARS];
    char sobrenome[MAX_CHARS];
    char residencia[MAX_CHARS];
    char formacao[MAX_CHARS];
    uint32_t ano_formatura;
    char habilidades[MAX_CHARS * 2];
    char experiencia[MAX_EXP][MAX_CHARS];
    uint32_t n_experiencia;
} UserProfile;

typedef struct _protocolData {
    Operation op;
    uint32_t profiles_num;
    // UserProfile* profiles_buf; // subsequent data is a user profile buffer
} ProtocolData;

// 4 / 4 / ***********************************************
//    void* buf;
//    ProtocolData* data = (ProtocolData*)buf;
//    data->op;           // 4 bytes
//    data->profiles_num; // 4 bytes
//    UserProfile* prof_buf = (UserProfile*) (buf + sizeof(ProtocolData));