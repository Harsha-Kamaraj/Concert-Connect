#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "users.h"
#include "utils.h"

#define INITIAL_USER_CAP 128

User *users = NULL;
int user_count = 0;
int user_capacity = 0;
UserHashTable *user_hash_table = NULL;

/* ============= HASH TABLE IMPLEMENTATION ============= */

UserHashTable* create_user_hash_table(int size) {
    UserHashTable *ht = (UserHashTable *)malloc(sizeof(UserHashTable));
    if (!ht) return NULL;
    ht->buckets = (UserHashNode **)calloc(size, sizeof(UserHashNode *));
    if (!ht->buckets) { free(ht); return NULL; }
    ht->size = size;
    return ht;
}

void free_user_hash_table(UserHashTable *ht) {
    if (!ht) return;
    for (int i = 0; i < ht->size; ++i) {
        UserHashNode *node = ht->buckets[i];
        while (node) {
            UserHashNode *temp = node;
            node = node->next;
            free(temp);
        }
    }
    free(ht->buckets);
    free(ht);
}

/* Simple hash function using djb2 algorithm */
unsigned int hash_username(const char *username) {
    unsigned int hash = 5381;
    int c;
    while ((c = *username++)) {
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    }
    return hash;
}

void hash_insert_user(UserHashTable *ht, const char *username, int user_index) {
    if (!ht) return;
    unsigned int bucket = hash_username(username) % ht->size;
    
    /* Check if already exists (update) */
    UserHashNode *node = ht->buckets[bucket];
    while (node) {
        if (strcmp(users[node->user_index].username, username) == 0) {
            node->user_index = user_index;  /* Update */
            return;
        }
        node = node->next;
    }
    
    /* Insert new node at head of chain */
    UserHashNode *new_node = (UserHashNode *)malloc(sizeof(UserHashNode));
    if (!new_node) return;
    new_node->user_index = user_index;
    new_node->next = ht->buckets[bucket];
    ht->buckets[bucket] = new_node;
}

int hash_find_user(UserHashTable *ht, const char *username) {
    if (!ht) return -1;
    unsigned int bucket = hash_username(username) % ht->size;
    
    UserHashNode *node = ht->buckets[bucket];
    while (node) {
        if (strcmp(users[node->user_index].username, username) == 0) {
            return node->user_index;  /* Found! O(1) average case */
        }
        node = node->next;
    }
    return -1;  /* Not found */
}

void rebuild_hash_table(void) {
    if (user_hash_table) free_user_hash_table(user_hash_table);
    user_hash_table = create_user_hash_table(HASH_TABLE_SIZE);
    for (int i = 0; i < user_count; ++i) {
        hash_insert_user(user_hash_table, users[i].username, i);
    }
}

void ensure_user_capacity(void) {
    if (!users) {
        user_capacity = INITIAL_USER_CAP;
        users = (User *)malloc(sizeof(User) * user_capacity);
        if (!users) { fprintf(stderr, "Memory allocation failed\n"); exit(1); }
        /* Initialize hash table */
        if (!user_hash_table) {
            user_hash_table = create_user_hash_table(HASH_TABLE_SIZE);
        }
    } else if (user_count >= user_capacity) {
        user_capacity *= 2;
        User *tmp = (User *)realloc(users, sizeof(User) * user_capacity);
        if (!tmp) { fprintf(stderr, "Memory allocation failed\n"); exit(1); }
        users = tmp;
        /* Rebuild hash table after realloc (pointers may have changed) */
        rebuild_hash_table();
    }
}

int validate_customer_username(const char *u) {
    if (!u) return 0;
    int len = strlen(u);
    if (len < 6) return 0;
    const char *underscore = strchr(u, '_');
    if (!underscore) return 0;
    if (strchr(underscore + 1, '_')) return 0;
    int year_len = strlen(underscore + 1);
    if (year_len != 4) return 0;
    for (int i = 0; i < year_len; ++i) if (!isdigit((unsigned char)underscore[1 + i])) return 0;
    int name_len = underscore - u;
    if (name_len < 1) return 0;
    for (int i = 0; i < name_len; ++i) {
        char ch = u[i];
        if (!(isalpha((unsigned char)ch) || ch == '.' || ch == '-')) return 0;
    }
    return 1;
}

int validate_admin_username(const char *u) {
    if (!u) return 0;
    int len = strlen(u);
    if (len < 6) return 0;
    const char *at = strchr(u, '@');
    if (!at) return 0;
    if (strchr(at + 1, '@')) return 0;
    int year_len = strlen(at + 1);
    if (year_len != 4) return 0;
    for (int i = 0; i < year_len; ++i) if (!isdigit((unsigned char)at[1 + i])) return 0;
    int name_len = at - u;
    if (name_len < 1) return 0;
    for (int i = 0; i < name_len; ++i) {
        char ch = u[i];
        if (!(isalpha((unsigned char)ch) || ch == '.' || ch == '-')) return 0;
    }
    return 1;
}

int validate_password(const char *p) {
    if (!p) return 0;
    int len = strlen(p);
    if (len <= 8) return 0;
    int has_upper = 0, has_digit = 0, has_special = 0;
    for (int i = 0; i < len; ++i) {
        unsigned char ch = (unsigned char)p[i];
        if (isupper(ch)) has_upper = 1;
        else if (isdigit(ch)) has_digit = 1;
        else if (ispunct(ch) || (!isalnum(ch) && !isspace(ch))) has_special = 1;
    }
    return has_upper && has_digit && has_special;
}

int validate_phone(const char *ph) {
    if (!ph) return 0;
    int len = strlen(ph);
    if (len != 10) return 0;
    for (int i = 0; i < 10; ++i) if (!isdigit((unsigned char)ph[i])) return 0;
    return 1;
}

int validate_email(const char *em) {
    if (!em) return 0;
    const char *at = strchr(em, '@');
    if (!at) return 0;
    const char *dot = strchr(at + 1, '.');
    if (!dot) return 0;
    if (at == em) return 0;
    if (*(dot + 1) == '\0') return 0;
    return 1;
}

int find_user_index(const char *username) {
    /* Use hash table for O(1) average lookup instead of O(n) */
    if (user_hash_table) {
        return hash_find_user(user_hash_table, username);
    }
    
    /* Fallback to linear search if hash table not initialized */
    for (int i = 0; i < user_count; ++i) {
        if (strcmp(users[i].username, username) == 0) return i;
    }
    return -1;
}

int signup_customer(void) {
    char username[MAX_USERNAME], password[MAX_PASS], phone[MAX_PHONE], email[MAX_EMAIL];
    printf("\n== Customer Sign Up ==\n");
    printf("Enter username (name_birthyear): ");
    read_line(username, sizeof(username));
    if (!validate_customer_username(username)) { printf("Invalid username format.\n"); return -1; }
    if (find_user_index(username) != -1) { printf("Username exists.\n"); return -1; }

    printf("Enter password (Min 8 chars with 1 uppercase, 1 digit, 1 special): ");
    read_line(password, sizeof(password));
    if (!validate_password(password)) { printf("Weak password.\n"); return -1; }

    printf("Enter phone (10 digits): ");
    read_line(phone, sizeof(phone));
    if (!validate_phone(phone)) { printf("Invalid phone.\n"); return -1; }

    printf("Enter email: ");
    read_line(email, sizeof(email));
    if (!validate_email(email)) { printf("Invalid email.\n"); return -1; }

    ensure_user_capacity();
    strncpy(users[user_count].username, username, MAX_USERNAME - 1);
    users[user_count].username[MAX_USERNAME - 1] = '\0';
    strncpy(users[user_count].password, password, MAX_PASS - 1);
    users[user_count].password[MAX_PASS - 1] = '\0';
    users[user_count].role = ROLE_CUSTOMER;
    strncpy(users[user_count].phone, phone, MAX_PHONE - 1);
    users[user_count].phone[MAX_PHONE - 1] = '\0';
    strncpy(users[user_count].email, email, MAX_EMAIL - 1);
    users[user_count].email[MAX_EMAIL - 1] = '\0';
    
    /* Add to hash table for O(1) lookup */
    hash_insert_user(user_hash_table, username, user_count);
    
    user_count++;
    
    /* Save to file */
    save_users_to_file("users.txt");
    
    printf("Customer signup successful. You can now Sign In.\n");
    return user_count - 1;
}

int signin_customer(int *out_idx) {
    char username[MAX_USERNAME], password[MAX_PASS];
    printf("\n== Customer Sign In ==\nUsername: ");
    read_line(username, sizeof(username));
    printf("Password: ");
    read_line(password, sizeof(password));
    int idx = find_user_index(username);
    if (idx == -1) { printf("No such user.\n"); return 0; }
    if (users[idx].role != ROLE_CUSTOMER) { printf("Not a customer account.\n"); return 0; }
    if (strcmp(users[idx].password, password) != 0) { printf("Incorrect password.\n"); return 0; }
    *out_idx = idx;
    printf("Signed in. Phone: %s | Email: %s\n", users[idx].phone, users[idx].email);
    return 1;
}

int signup_admin(void) {
    char username[MAX_USERNAME], password[MAX_PASS], phone[MAX_PHONE], email[MAX_EMAIL];
    printf("\n== Admin Sign Up ==\n");
    printf("Enter username (name@birthyear): ");
    read_line(username, sizeof(username));
    if (!validate_admin_username(username)) { printf("Invalid admin username.\n"); return -1; }
    if (find_user_index(username) != -1) { printf("Username exists.\n"); return -1; }

    printf("Enter password (Min 8 chars with 1 uppercase, 1 digit, 1 special): ");
    read_line(password, sizeof(password));
    if (!validate_password(password)) { printf("Weak password.\n"); return -1; }

    printf("Enter phone (10 digits): ");
    read_line(phone, sizeof(phone));
    if (!validate_phone(phone)) { printf("Invalid phone.\n"); return -1; }

    printf("Enter email: ");
    read_line(email, sizeof(email));
    if (!validate_email(email)) { printf("Invalid email.\n"); return -1; }

    ensure_user_capacity();
    strncpy(users[user_count].username, username, MAX_USERNAME - 1);
    users[user_count].username[MAX_USERNAME - 1] = '\0';
    strncpy(users[user_count].password, password, MAX_PASS - 1);
    users[user_count].password[MAX_PASS - 1] = '\0';
    users[user_count].role = ROLE_ADMIN;
    strncpy(users[user_count].phone, phone, MAX_PHONE - 1);
    users[user_count].phone[MAX_PHONE - 1] = '\0';
    strncpy(users[user_count].email, email, MAX_EMAIL - 1);
    users[user_count].email[MAX_EMAIL - 1] = '\0';
    
    /* Add to hash table for O(1) lookup */
    hash_insert_user(user_hash_table, username, user_count);
    
    user_count++;
    
    /* Save to file */
    save_users_to_file("users.txt");
    
    printf("Admin signup successful. You can now Sign In.\n");
    return user_count - 1;
}

int signin_admin(int *out_idx) {
    char username[MAX_USERNAME], password[MAX_PASS];
    printf("\n== Admin Sign In ==\nUsername: ");
    read_line(username, sizeof(username));
    printf("Password: ");
    read_line(password, sizeof(password));
    int idx = find_user_index(username);
    if (idx == -1) { printf("No such user.\n"); return 0; }
    if (users[idx].role != ROLE_ADMIN) { printf("Not an admin account.\n"); return 0; }
    if (strcmp(users[idx].password, password) != 0) { printf("Incorrect password.\n"); return 0; }
    *out_idx = idx;
    printf("Signed in. Phone: %s | Email: %s\n", users[idx].phone, users[idx].email);
    return 1;
}

/* ============= USER PERSISTENCE ============= */

void save_users_to_file(const char *path) {
    FILE *fp = fopen(path, "w");
    if (!fp) { 
        printf("Warning: Failed to save users to %s\n", path); 
        return; 
    }
    
    for (int i = 0; i < user_count; ++i) {
        User *u = &users[i];
        const char *role_str = (u->role == ROLE_ADMIN) ? "ADMIN" : 
                               (u->role == ROLE_CUSTOMER) ? "CUSTOMER" : "NONE";
        /* Format: username|password|role|phone|email */
        fprintf(fp, "%s|%s|%s|%s|%s\n", 
                u->username, u->password, role_str, u->phone, u->email);
    }
    
    fclose(fp);
}

void load_users_from_file(const char *path) {
    FILE *fp = fopen(path, "r");
    if (!fp) return; /* OK if file doesn't exist yet */
    
    char line[512];
    while (fgets(line, sizeof(line), fp)) {
        /* Remove trailing newline */
        size_t len = strlen(line);
        while (len > 0 && (line[len-1] == '\n' || line[len-1] == '\r')) {
            line[--len] = '\0';
        }
        
        if (!line[0]) continue;
        
        char username[MAX_USERNAME], password[MAX_PASS], role_str[16];
        char phone[MAX_PHONE], email[MAX_EMAIL];
        
        /* Parse: username|password|role|phone|email */
        char *tok;
        char *rest = line;
        
        tok = strtok(rest, "|"); 
        if (!tok) continue; 
        strncpy(username, tok, sizeof(username)-1); 
        username[sizeof(username)-1] = '\0';
        
        tok = strtok(NULL, "|"); 
        if (!tok) continue; 
        strncpy(password, tok, sizeof(password)-1); 
        password[sizeof(password)-1] = '\0';
        
        tok = strtok(NULL, "|"); 
        if (!tok) continue; 
        strncpy(role_str, tok, sizeof(role_str)-1); 
        role_str[sizeof(role_str)-1] = '\0';
        
        tok = strtok(NULL, "|"); 
        if (!tok) continue; 
        strncpy(phone, tok, sizeof(phone)-1); 
        phone[sizeof(phone)-1] = '\0';
        
        tok = strtok(NULL, "|"); 
        if (!tok) continue; 
        strncpy(email, tok, sizeof(email)-1); 
        email[sizeof(email)-1] = '\0';
        
        /* Determine role */
        Role role = ROLE_NONE;
        if (strcmp(role_str, "ADMIN") == 0) role = ROLE_ADMIN;
        else if (strcmp(role_str, "CUSTOMER") == 0) role = ROLE_CUSTOMER;
        
        /* Add user */
        ensure_user_capacity();
        strncpy(users[user_count].username, username, MAX_USERNAME - 1);
        users[user_count].username[MAX_USERNAME - 1] = '\0';
        strncpy(users[user_count].password, password, MAX_PASS - 1);
        users[user_count].password[MAX_PASS - 1] = '\0';
        users[user_count].role = role;
        strncpy(users[user_count].phone, phone, MAX_PHONE - 1);
        users[user_count].phone[MAX_PHONE - 1] = '\0';
        strncpy(users[user_count].email, email, MAX_EMAIL - 1);
        users[user_count].email[MAX_EMAIL - 1] = '\0';
        
        /* Add to hash table */
        hash_insert_user(user_hash_table, username, user_count);
        
        user_count++;
    }
    
    fclose(fp);
}