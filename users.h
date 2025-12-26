#ifndef USERS_H
#define USERS_H

#define MAX_USERNAME 64
#define MAX_PASS 64
#define MAX_NAME 100
#define MAX_PHONE 20
#define MAX_EMAIL 100
#define HASH_TABLE_SIZE 128  /* power of 2 for better distribution */

typedef enum { ROLE_NONE, ROLE_ADMIN, ROLE_CUSTOMER } Role;

typedef struct User {
    char username[MAX_USERNAME];
    char password[MAX_PASS];
    Role role;
    char phone[MAX_PHONE];
    char email[MAX_EMAIL];
} User;

/* Hash table node for chaining */
typedef struct UserHashNode {
    int user_index;  /* index in users array */
    struct UserHashNode *next;
} UserHashNode;

/* Hash table for O(1) user lookup */
typedef struct UserHashTable {
    UserHashNode **buckets;  /* array of linked lists */
    int size;
} UserHashTable;

/* Globals for users */
extern User *users;
extern int user_count;
extern int user_capacity;
extern UserHashTable *user_hash_table;

void ensure_user_capacity(void);

/* Hash table operations */
UserHashTable* create_user_hash_table(int size);
void free_user_hash_table(UserHashTable *ht);
unsigned int hash_username(const char *username);
void hash_insert_user(UserHashTable *ht, const char *username, int user_index);
int hash_find_user(UserHashTable *ht, const char *username);
void rebuild_hash_table(void);

/* Validation */
int validate_customer_username(const char *u);
int validate_admin_username(const char *u);
int validate_password(const char *p);
int validate_phone(const char *ph);
int validate_email(const char *em);

/* User management */
int find_user_index(const char *username);
int signup_customer(void);
int signin_customer(int *out_idx);
int signup_admin(void);
int signin_admin(int *out_idx);

/* User persistence */
void save_users_to_file(const char *path);
void load_users_from_file(const char *path);

#endif /* USERS_H */