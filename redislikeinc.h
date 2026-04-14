#ifndef MINI_REDIS_H
#define MINI_REDIS_H

#include <stddef.h>

/* ================================
   VALUE TYPES
   ================================ */

typedef enum {
    TYPE_STRING
    /* future: TYPE_INT, TYPE_LIST, TYPE_HASH */
} ValueType;

/* ================================
   VALUE OBJECT
   ================================ */

typedef struct {
    ValueType type;
    char *data;          /* dynamically allocated */
} Value;

/* ================================
   HASH TABLE ENTRY
   ================================ */

typedef struct Entry {
    char *key;           /* dynamically allocated */
    Value value;
    struct Entry *next;  /* collision handling (chaining) */
} Entry;

/* ================================
   HASH TABLE
   ================================ */

#define TABLE_SIZE 1024

typedef struct {
    Entry *buckets[TABLE_SIZE];
} HashTable;

/* ================================c(size
   DATABASE OBJECT
   ================================ */

typedef struct {
    HashTable *table;
    size_t count;        /* number of keys */
} RedisDB;

/* ================================
   HASH TABLE API
   ================================ */

/* Hash function */
unsigned int hash(const char *key);

/* Create / destroy */
HashTable *ht_create(void);
void ht_free(HashTable *ht);


/* CRUD operations */
int  ht_set(HashTable *ht, const char *key, const char *value);
char *ht_get(HashTable *ht, const char *key);
int  ht_delete(HashTable *ht, const char *key);

/* ================================
   DATABASE API (HIGH LEVEL)
   ================================ */

RedisDB *db_create(void);
void     db_free(RedisDB *db);

int      db_set(RedisDB *db, const char *key, const char *value);
char    *db_get(RedisDB *db, const char *key);
int      db_del(RedisDB *db, const char *key);

/* ================================
   COMMAND PARSER
   ================================ */

typedef enum {
    CMD_SET,
    CMD_GET,
    CMD_DEL,
    CMD_EXIT,
    CMD_UNKNOWN
} CommandType;

typedef struct {
    CommandType type;
    char *key;      /* dynamically allocated */
    char *value;    /* NULL for GET/DEL */
} Command;

/* Parse user input like: "SET name Alice" */
Command parse_command(char *input);

/* Free memory allocated inside Command */
void free_command(Command *cmd);

/* ================================
   COMMAND EXECUTION
   ================================ */

void execute_command(RedisDB *db, Command cmd);
void execute_command_for_server(RedisDB *db,Command cmd,char *responseBuffer);

/* ================================
   PERSISTENCE 
   ================================ */

/* Save database to disk (key=value format) */
int db_save(RedisDB *db, const char *filename);

/* Load database from disk */
int db_load(RedisDB *db, const char *filename);

/* ================================
   MAIN LOOP 
   ================================ */

void redis_loop(RedisDB *db);
void tcpServer(RedisDB *db);

#endif /* MINI_REDIS_H */
