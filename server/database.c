#include "database.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <error.h>
#include <errno.h>
#include <dirent.h>
#include <assert.h>
#include <netinet/in.h>

#define GET_FILENAME(str, email) sprintf(str, "data/%s.db", email);

// lists the filenames inside directory with dirname, the user need to release memory from namelist
// returns the number of elements in namelist if succeeded, -1 if not
static size_t get_dir_filenames(char *dirname, char ***namelist)
{
    DIR *data_dir;
    struct dirent *dir_entry;
    char **filenames;
    size_t cap = 16;
    size_t n_files = -1;

    assert(dirname);
    assert(namelist);

    data_dir = opendir(dirname);
    if (data_dir)
    {
        n_files = 0;
        filenames = malloc(cap * sizeof(char*));
        // read ./data directory and store entries names on filename_list.
        while ((dir_entry = readdir(data_dir)) != NULL)
        {
            if (dir_entry->d_name[0] == '.')
                continue;

            if (n_files >= cap)
            {
                size_t new_size = cap * 2;
                char **aux_buf = malloc(sizeof(char *) * new_size);
                memcpy(aux_buf, filenames, cap);
                cap = new_size;
                free(filenames);
                filenames = aux_buf;
            }

            char *cur_name = malloc(256 * sizeof(char));
            strcpy(cur_name, dir_entry->d_name);
            filenames[n_files] = cur_name;
            n_files++;
        }
        closedir(data_dir);
    }
    *namelist = filenames;
    return n_files;
}

// gets the email from the filename and place it in dest.
// basically just remove the .db extension
static void get_email_from_filename(char *dest, char *filename)
{
    assert(dest);
    assert(filename);

    char *last_point = strrchr(filename, '.');
    strcpy(dest, filename);
    dest[last_point - filename] = '\0';
}

static void fget_uint(FILE *in, uint32_t *dest)
{
    char input[32];
    fgets(input, 32, in);
    sscanf(input, "%u", dest);
}

static char *fgetss(char *dest, int n, FILE *stream)
{
    if (fgets(dest, n, stream))
    {
        int len = strlen(dest);
        if (dest[len - 1] == '\n')
            dest[len - 1] = '\0';
        return dest;
    }
    return NULL;
}

// creates an entry for profile in database
// parameters:
//  profile: the profile to add to database
// return:
//  1 if succeeded, 0 if not
int create_user(UserProfile *profile)
{
    char filename[256];
    FILE *db_file;
    int success = 1;

    if (profile)
    {
        GET_FILENAME(filename, profile->email);
        db_file = fopen(filename, "w");

        if (db_file)
        {
            fprintf(db_file, "%s\n", profile->nome);
            fprintf(db_file, "%s\n", profile->sobrenome);
            fprintf(db_file, "%s\n", profile->residencia);
            fprintf(db_file, "%s\n", profile->formacao);
            fprintf(db_file, "%u\n", ntohl(profile->ano_formatura));
            fprintf(db_file, "%s\n", profile->habilidades);
            for (int32_t i = 0; i < ntohl(profile->n_experiencia); i++)
            {
                fprintf(db_file, "%s\n", profile->experiencia[i]);
            }

            fclose(db_file);
        }
        else
        {
            success = 0;
            fprintf(stderr, "failed to create file at: create_user\n");
        }
    }
    else
    {
        success = 0;
        fprintf(stderr, "invalid profile received at: create_user\n");
    }

    return success;
}

// reads database
// parameters:
//  profile: receives an array of the user profiles stored in database. Memory should be released by user.
// return:
//  number of entries in array if succeeded, -1 if failed.
int read_db(UserProfile **profile)
{
    FILE *current_file;
    UserProfile *users;
    size_t n_files, u_idx;
    char **filenames;
    char cur_filename[MAX_CHARS];
    int32_t n_exp;

    n_files = get_dir_filenames("./data", &filenames);
    if (n_files > 0)
    {
        users = malloc(n_files * sizeof(UserProfile));
        *profile = users;
        u_idx = 0;
        for (size_t i = 0; i < n_files; i++, u_idx++)
        {
            sprintf(cur_filename, "data/%s", filenames[i]);
            current_file = fopen(cur_filename, "r");
            if (current_file)
            {
                get_email_from_filename(users[u_idx].email, filenames[i]);

                fgetss(users[u_idx].nome, sizeof users[u_idx].nome, current_file);
                fgetss(users[u_idx].sobrenome, sizeof users[u_idx].sobrenome, current_file);
                fgetss(users[u_idx].residencia, sizeof users[u_idx].residencia, current_file);
                fgetss(users[u_idx].formacao, sizeof users[u_idx].formacao, current_file);

                // get uint32_t from file
                fget_uint(current_file, &(users[u_idx].ano_formatura));
                users[u_idx].ano_formatura = htonl(users[u_idx].ano_formatura);

                fgetss(users[u_idx].habilidades, sizeof users[u_idx].habilidades, current_file);

                // get the subsequent line as work experience
                for (n_exp = 0; fgetss(users[u_idx].experiencia[n_exp], MAX_CHARS, current_file) && n_exp < MAX_EXP; n_exp++);
                users[u_idx].n_experiencia = htonl(n_exp);

                fclose(current_file);
            }
            else
            {
                u_idx--;
                fprintf(stderr, "db file not found while reading: %s\n", filenames[i]);
            }

            free(filenames[i]);
            cur_filename[0] = '\0';
        }
        free(filenames);
    }

    return u_idx;
}

// updates a profile entry in database
// parameters:
//  profile: the profile to update, identified by its email which has the updated values
// return:
//  1 if succeded, 0 if not
int update_user(UserProfile *profile)
{
    char filename[256];
    FILE *db_file;
    int success = 1;
    if (profile)
    {
        GET_FILENAME(filename, profile->email);
        db_file = fopen(filename, "r");
        if (db_file)
        {
            db_file = freopen(NULL, "a", db_file);
            if (db_file)
            {
                fputs(profile->experiencia[0], db_file);
                fclose(db_file);
            }
            else
            {
                success = 0;
                perror("error trying to reopen file to append");
            }
        }
        else
        {
            success = 0;
            fprintf(stderr, "file %s not found\n", filename);
        }
    }
    else
    {
        success = 0;
        fprintf(stderr, "invalid profile received at: update_user\n");
    }
    return success;
}

// deletes a profile entry from database
// paramenters:
//  profile: the profile to delete, identified by its email
// return:
//  1 if succeeded, 0 if not.
int delete_user(UserProfile *profile)
{
    char filename[256];
    int success = 1;
    if (profile)
    {
        GET_FILENAME(filename, profile->email);
        if (remove(filename) != 0)
        {
            success = 0;
            fprintf(stderr, "failed to remove file for %s\n", profile->email);
        }
    }
    else
    {
        success = 0;
        fprintf(stderr, "invalid profile received at: delete_user\n");
    }
    return success;
}
