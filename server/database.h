#pragma once

#include "comm.h"

// creates an entry for profile in database
// parameters:
//  profile: the profile to add to database
// return:
//  1 if succeeded, 0 if not
int create_user(UserProfile *profile);

// reads database
// parameters:
//  profile: receives an array of the user profiles stored in database. Memory should be released by user.
// return:
//  number of entries in array
int read_db(UserProfile **profile);

// updates a profile entry in database
// parameters:
//  profile: the profile to update, identified by its email which has the updated values
// return:
//  1 if succeded, 0 if not
int update_user(UserProfile *profile);

// deletes a profile entry from database
// paramenters:
//  profile: the profile to delete, identified by its email
// return:
//  1 if succeeded, 0 if not.
int delete_user(UserProfile *profile);