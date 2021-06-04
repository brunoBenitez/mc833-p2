#include "utils.h"

#include <stdio.h>
#include <string.h>


char *fgetss(char *dest, int n, FILE *stream)
{
    if (fgets(dest, n, stream))
    {
        int len = strlen(dest);
        if (dest[len-1] == '\n')
            dest[len-1] = '\0';
        return dest;
    }
    return NULL;
}

AccountType login(void)
{
    printf("Bem vindo!\n");

    // TODO: get input user
    // select account type based on user input
    // Q: should the user be already registered to be able to access? 
    //    (request the server for the user or allow anything?)

    char acc_type[7];

    printf("Por favor, insira o tipo de usuario. Opcoes: (admin, user):\n");
    do{
        fgetss(acc_type, 7, stdin);
    } while(strcmp(acc_type,"user") != 0 && strcmp(acc_type,"admin") != 0);
  
    if(strcmp(acc_type,"user") == 0)
        return USER;

    return ADMIN;
}

int showMenu(AccountType accountType)
{
    char command;
    char input[8];
    printf("O que deseja fazer?\n");
    printf("0 - sair\n");
    printf("1 - acrescentar uma nova experiência profissional em um perfil;\n");
    printf("2 - listar todas as pessoas(email e nome) formadas em um determinado curso;\n");
    printf("3 - listar todas as pessoas(email e nome) que possuam uma determinada habilidade;\n");
    printf("4 - listar todas as pessoas(email, nome e curso) formadas em um determinado ano;\n");
    printf("5 - listar todas as informações de todos os perfis;\n");
    printf("6 - dado o email de um perfil, retornar suas informações;\n");
    if (accountType == ADMIN)
    {
        printf("7 - remover um perfil a partir de seu identificador(email).\n");
        printf("8 - cadastrar um novo perfil utilizando o email como identificador;\n");
    }

    fgetss(input, 8, stdin);
    sscanf(input, "%c", &command);
    return command;
}

