#include "utils.h"
#include "commands.h"
#include "comm.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

static int get_uint(uint32_t *dest)
{
    char input[32];
    fgets(input, 32, stdin);
    return sscanf(input, "%u", dest);
}

void addExperience(void)
{
    // - get input:
    //      - email
    //      - experience
    // - send update to server database
    UserProfile input;
    ProtocolData comando;
    int n_profiles;

    comando.op = UPDATE;
    comando.profiles_num = 1;

    printf("Voce escolheu *ADICIONAR EXPERIENCIA*. Por favor, forneca um email seguido de apenas uma experiencia\n\n");
    printf("Email:\n");
    fgetss(input.email, MAX_CHARS, stdin);
    printf("Experiencia:\n");
    fgetss(input.experiencia[0], MAX_CHARS, stdin);
    client_connect(comando, input, &n_profiles);

    if (n_profiles == SUCCESS) 
    {
        printf("Sucesso!\n\n");
    } else{
        printf("Falha!\n\n");
    }
}

void listPeopleByMajor(void)
{
    // - get input: major 
    // - queue server for database
    // - filter database by major
    // - show profiles found with Email and Name
    UserProfile input;
    ProtocolData comando;
    UserProfile *output;
    int count = 0;
    int n_profiles;

    printf("Voce escolheu *LISTAR POR FORMACAO*. Por favor, forneca uma formacao\n\n");
    comando.op = READ;
    fgetss(input.formacao, MAX_CHARS, stdin);
    output = client_connect(comando, input, &n_profiles);

    for (int i = 0; i < n_profiles; i++)
    {
        if (strcmp(output[i].formacao, input.formacao) == 0)
        {
            printf("*****\nEmail: %s\nNome: %s\n", output[i].email, output[i].nome);
            count++;
        }        
    }
    
    printf("Encontrados %d perfis no total!\n", count);
}

void listPeopleBySkill(void)
{
    // - get input: skill
    // - queue server for database
    // - filter database by skill
    // - show profiles found with Email and Name
    UserProfile input;
    ProtocolData comando;
    UserProfile *output;
    int count = 0;
    int n_profiles;

    printf("Voce escolheu *LISTAR POR HABILIDADE*. Por favor, forneca uma habilidade\n\n");
    comando.op = READ;
    fgetss(input.habilidades, MAX_CHARS, stdin);
    output = client_connect(comando, input, &n_profiles);

    for (int i = 0; i < n_profiles; i++)
    {
        if (strstr(output[i].habilidades, input.habilidades) != NULL) // Procura substring
        {
            printf("*****\nEmail: %s\nNome: %s\n", output[i].email, output[i].nome);
            count++;
        }
    }
    
    printf("Encontrados %d perfis no total!\n", count);
}

void listPeopleByGradYear(void)
{
    // - get input: major
    // - queue server for database
    // - filter database by major
    // - show profiles found with Email, Name and Major
    UserProfile input;
    ProtocolData comando;
    UserProfile *output;
    int count = 0;
    int n_profiles;

    printf("Voce escolheu *LISTAR POR ANO DE FORMATURA*. Por favor, forneca um ano\n\n");
    comando.op = READ;
    if (!get_uint(&input.ano_formatura))
    {
        printf("Ano invalido!\n");
        return;
    }

    output = client_connect(comando, input, &n_profiles);

    for (int i = 0; i < n_profiles; i++)
    {
        if (output[i].ano_formatura == input.ano_formatura) 
        {
            printf("*****\nEmail: %s\nNome: %s\nCurso: %s\n", output[i].email, output[i].nome, 
            output[i].formacao);
            count++;
        }
    }
    
    printf("Encontrados %d perfis no total!\n", count);
    
    if (output)
        free(output);
}

void listAll(void)
{
    // - queue server for whole database.
    // - show profiles with all info
    // 
    // example:
    //
    // Email: maria_silva@gmail.com
    // Nome: Maria
    // Sobrenome: Silva
    // Residência: Campinas
    // Formação Acadêmica: Ciência da Computação
    // Ano de Formatura: 2015
    // Habilidades: Análise de Dados, Internet das Coisas, Computação em Nuvem
    // Experiência: (1) Estágio de 1 ano na Empresa X, onde trabalhei como analista de dados
    //              (2) Trabalhei com IoT e Computação em Nuvem por 5 anos na Empresa Y
    UserProfile input;
    ProtocolData comando;
    UserProfile *output;
    int count = 0;
    int n_profiles;

    printf("Voce escolheu *LISTAR TODOS PERFIS*\n\n");
    comando.op = READ;
    output = client_connect(comando, input, &n_profiles);

    for (int i = 0; i < n_profiles; i++)
    {
        count++;
        printf("*****\nEmail: %s\n", output[i].email);
        printf("Nome: %s\n", output[i].nome);
        printf("Sobrenome: %s\n", output[i].sobrenome);
        printf("Residencia: %s\n", output[i].residencia);
        printf("Formacao: %s\n", output[i].formacao);
        printf("Ano de formatura: %u\n", output[i].ano_formatura);
        printf("Habilidades: %s\n", output[i].habilidades);
        printf("Experiências:\n");
        if (output[i].n_experiencia > 0)
        {
            for (int j = 0; j < output[i].n_experiencia; j++)
            {
                printf("\t(%d) %s\n", j + 1, output[i].experiencia[j]);
            }
        }
        else
        {
            printf("Nenhuma!\n\n");
        }
    }
    
    printf("Foram listados %d perfis no total!\n", count);
    
    if (output)
        free(output);
}

void retrieveProfileInfo(void)
{
    // - get input: email 
    // - queue server for database
    // - find specified email in data
    // - show profile with all info
    //
    // example:
    //
    // Email: maria_silva@gmail.com
    // Nome: Maria
    // Sobrenome: Silva
    // Residência: Campinas
    // Formação Acadêmica: Ciência da Computação
    // Ano de Formatura: 2015
    // Habilidades: Análise de Dados, Internet das Coisas, Computação em Nuvem
    // Experiência: (1) Estágio de 1 ano na Empresa X, onde trabalhei como analista de dados
    //              (2) Trabalhei com IoT e Computação em Nuvem por 5 anos na Empresa Y
    UserProfile input;
    ProtocolData comando;
    UserProfile *output;
    int n_profiles;

    printf("Voce escolheu *LISTAR UM PERFIL*\n\n");
    printf("Por favor insira o email que deseja buscar.\n");
    fgetss(input.email, MAX_CHARS, stdin);
    comando.op = READ;
    output = client_connect(comando, input, &n_profiles);

    for (int i = 0; i < n_profiles; i++)
    {
        if (strcmp(input.email, output[i].email) == 0) 
        {
            printf("*****\nEmail: %s\n", output[i].email);
            printf("Nome: %s\n", output[i].nome);
            printf("Sobrenome: %s\n", output[i].sobrenome);
            printf("Residencia: %s\n", output[i].residencia);
            printf("Formacao: %s\n", output[i].formacao);
            printf("Ano de formatura: %u\n", output[i].ano_formatura);
            printf("Habilidades: %s\n", output[i].habilidades);
            printf("Experiências:\n");
            if(output[i].n_experiencia > 0){
                for (int j = 0; j < output[i].n_experiencia; j++)
                {
                    printf("\t(%d) %s\n", j+1, output[i].experiencia[j]);
                }
            } else{
                printf("Nenhuma!\n\n");
            }
            goto end;
        }
    }

    printf("Nenhum perfil encontrado com esse email.\n");

end:
    if (output)
        free(output);
}

void deleteProfile(void)
{
    // - get input: email (string?)
    // - send deletion request to server
    UserProfile input;
    ProtocolData comando;
    int n_profiles;

    printf("Voce escolheu *EXCLUIR PERFIL*. Por favor, forneca um email\n\n");
    fgetss(input.email, MAX_CHARS, stdin);
    comando.op = DELETE;
    client_connect(comando, input, &n_profiles);

    if (n_profiles == SUCCESS) 
    {
        printf("Sucesso!\n\n");
    } else{
        printf("Falha!\n\n");
    }
}

void registerProfile(void)
{
    // - get input:
    //      - email (string)
    //      - nome (string)
    //      - sobrenome (string)
    //      - residência (string)
    //      - formação academica (string)
    //      - ano de formatura (int)
    //      - habilidades (string)
    //      - experiência (int+string)
    // 2) send to server
    UserProfile input;
    ProtocolData comando;
    int n_profiles; // Variavel coringa para n de profiles retornados. pode ser usada para erro

    printf("Voce escolheu *ADICIONAR NOVO PERFIL*. Por favor, forneca os dados conforme requisitado:\n");
    printf("Email:\n");
    fgetss(input.email, MAX_CHARS, stdin);
    printf("Nome:\n");
    fgetss(input.nome, MAX_CHARS, stdin);
    printf("Sobrenome:\n");
    fgetss(input.sobrenome, MAX_CHARS, stdin);
    printf("Residencia:\n");
    fgetss(input.residencia, MAX_CHARS, stdin);
    printf("Formacao Academica:\n");
    fgetss(input.formacao, MAX_CHARS, stdin);
    printf("Ano de formatura:\n");
    while(!get_uint(&input.ano_formatura))
        printf("Ano invalido, tente novamente\n");
    printf("Habilidades:\n");
    fgetss(input.habilidades, MAX_CHARS, stdin);
    printf("Quantas experiencias quer inserir? (0-20):\n");
    while(!get_uint(&input.n_experiencia) || input.n_experiencia >= MAX_EXP)
        printf("Numero invalido, por favor insira um numero entre 0 e 20\n");
    input.n_experiencia = input.n_experiencia < MAX_EXP ? input.n_experiencia : MAX_EXP;
    for (int i = 0; i < input.n_experiencia; i++)
    {
        printf("Insira experiencia #%d:", i+1);
        fgetss(input.experiencia[i], MAX_CHARS, stdin);
    }
    
    comando.op = CREATE;
    client_connect(comando, input, &n_profiles);

    if (n_profiles == SUCCESS) 
    {
        printf("Sucesso!\n");
    } else {
        printf("Falha!\n");
    }
}
