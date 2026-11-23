#include "hash_table.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Start with an empty table so every bucket is ready to use.
void initialise_hash_table(HashTable *ht)
{
    if (ht == NULL)
    {
        return;
    }

    for (int i = 0; i < HASH_TABLE_SIZE; i++)
    {
        ht->buckets[i] = NULL;
    }
}

// Use the user ID to work out where their details should be stored.
int get_bucket_for_user(int user_id)
{
    if (user_id < 1 || user_id > 2139)
    {
        return -1;
    }

    return user_id % HASH_TABLE_SIZE;
}

// Search the bucket, including any users linked together in it.
User *find_user(const HashTable *ht, int user_id)
{
    if (ht == NULL)
    {
        return NULL;
    }

    int bucket = get_bucket_for_user(user_id);
    if (bucket == -1)
    {
        return NULL;
    }

    User *current = ht->buckets[bucket];
    while (current != NULL)
    {
        if (current->user_id == user_id)
        {
            return current;
        }
        current = current->next; // Move on to the next user in this bucket.
    }

    return NULL;
}

// Add a user, or replace their password if their ID is already registered.
int register_user(HashTable *ht, int user_id, char password[])
{
    if (ht == NULL || password == NULL)
    {
        return ERROR;
    }

    int bucket = get_bucket_for_user(user_id);
    if (bucket == -1 || strlen(password) > MAX_PASSWORD_LEN)
    {
        return ERROR;
    }

    User *existing = find_user(ht, user_id);
    if (existing != NULL)
    {
        strcpy(existing->password, password);
        return SUCCESS;
    }

    // Make space for a new record before linking it into the table.
    User *new_user = malloc(sizeof(User));
    if (new_user == NULL)
    {
        return ERROR;
    }

    new_user->user_id = user_id;
    strcpy(new_user->password, password);

    // Put the new record at the front, keeping the other users linked behind it.
    new_user->next = ht->buckets[bucket];
    ht->buckets[bucket] = new_user;

    return SUCCESS;
}

// Check whether the supplied password matches the registered user's password.
int is_valid_password(const HashTable *ht, int user_id, char password[])
{
    if (ht == NULL || password == NULL)
    {
        return ERROR;
    }

    User *user = find_user(ht, user_id);
    if (user == NULL)
    {
        return ERROR;
    }

    // strcmp gives zero when the two passwords match.
    if (strcmp(user->password, password) == 0)
    {
        return SUCCESS;
    }

    return ERROR;
}

// Free every user record and leave all the buckets empty again.
void free_buckets(HashTable *ht)
{
    if (ht == NULL)
    {
        return;
    }

    for (int i = 0; i < HASH_TABLE_SIZE; i++)
    {
        User *current = ht->buckets[i];
        while (current != NULL)
        {
            // Save the next pointer before freeing the current record.
            User *next_user = current->next;
            free(current);
            current = next_user;
        }
        ht->buckets[i] = NULL;
    }
}

// Read the users from a text file and register them in the order they appear.
int load_users_from_file(HashTable *ht, char filename[])
{
    if (ht == NULL || filename == NULL)
    {
        return ERROR;
    }

    FILE *file = fopen(filename, "r");
    if (file == NULL)
    {
        return ERROR;
    }

    char line[256];
    while (fgets(line, sizeof(line), file) != NULL)
    {
        int user_id;
        char password[MAX_PASSWORD_LEN + 2];
        char extra;

        // Read the ID and password, and check that there isn't a third value.
        int items = sscanf(line, "%d %41s %c", &user_id, password, &extra);
        if (items != 2)
        {
            fclose(file);
            return ERROR;
        }

        if (register_user(ht, user_id, password) != SUCCESS)
        {
            fclose(file);
            return ERROR;
        }
    }

    // A read error is different from reaching the end of the file normally.
    int read_error = ferror(file);
    fclose(file);

    if (read_error)
    {
        return ERROR;
    }
    return SUCCESS;
}
