#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include "redislikeinc.h"
#define REDIS_DB "database.db"




unsigned int hash(const char *key)
{
    unsigned int hash = 5381;
    int c;

    while (c = *key++)
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return hash % TABLE_SIZE;
}




/* Create / destroy */
HashTable *ht_create(void){
    HashTable *ht = malloc(sizeof(*ht));
    
    for(int i = 0;i<TABLE_SIZE;i++){
        ht->buckets[i] = NULL;
    }

    return ht;
    Command cmd;


}
void ht_free(HashTable *ht){
    free(ht);

}

Entry*  createEntry(const char *key,const char * value){
     Entry *newEntry = malloc(sizeof(*newEntry));
     if(!newEntry) return NULL;

    newEntry->key = strdup(key);
    newEntry->value.type = TYPE_STRING;
    newEntry->value.data = strdup(value);
    newEntry->next = NULL;

    return newEntry;
}

int  ht_set(HashTable *ht, const char *key, const char *value){
    unsigned long hashedKey = hash(key);
//check this
    Entry *curr = ht->buckets[hashedKey];
    
    while(curr){
        if(strcmp(curr->key,key)==0){
            char * toFree = curr->value.data;
            curr->value.data = strdup(value);
            free(toFree);

            return 1;
        }

        curr = curr->next;

    }
    
    Entry *newEntry = createEntry(key,value);

    if(!newEntry) return 0;
    newEntry->next =  ht->buckets[hashedKey];
    ht->buckets[hashedKey] = newEntry;

    return 1;





}
char *ht_get(HashTable *ht, const char *key){
    unsigned long hashedKey = hash(key);

    Entry *curr = ht->buckets[hashedKey];


    while(curr){
        if(strcmp(curr->key,key)==0){
            return (curr->value).data;
        }
        curr = curr->next;
    }

    return NULL;
}
void free_entry(Entry *entry){
    free(entry->key);
    free(entry->value.data);
    free(entry);
}
int  ht_delete(HashTable *ht, const char *key){

    unsigned long hashedKey = hash(key);


    Entry *curr = ht->buckets[hashedKey];
    if(curr!=NULL && strcmp(curr->key,key)==0){
        Entry *tmp = curr;
        ht->buckets[hashedKey] = curr->next;
        free_entry(tmp);
        return 1;

    }
    Entry *prev = NULL;
//    RedisDB *db = db_create();
    while(curr){
        if(strcmp(curr->key,key)==0){

            if(!prev){
                Entry *tmp = curr;
                ht->buckets[hashedKey] = curr->next;
                free_entry(tmp);
                return 1;

            }
            else{
            prev->next = curr->next;
            free_entry(curr);
            return 1;
            }

        }
        prev = curr;
        curr = curr->next;
    }


    return 0;
}



RedisDB *db_create(void){

    RedisDB *db = malloc(sizeof(*db));
    db->table = ht_create();
    db->count = 0;

    return db;
}
void     db_free(RedisDB *db){
    ht_free(db->table);
    free(db);
}  

int      db_set(RedisDB *db, const char *key, const char *value){
    int returnVal = ht_set(db->table,key,value);
    if(returnVal){
        (db->count)++;
        return 1;
    }

    return 0;
}
char    *db_get(RedisDB *db, const char *key){
    return ht_get(db->table,key);
}
int      db_del(RedisDB *db, const char *key){
    int returnVal = ht_delete(db->table,key);
    if(returnVal){
        (db->count)--;
        return 1;
    }

    return 0;
}



Command parse_command(char *input){
    char delimiter[5] = " \n";
    char * token  = strtok(input,delimiter);
    
    Command cmd;
    cmd.key = NULL;
    cmd.value = NULL;

    if(token == NULL){
     cmd.type = CMD_UNKNOWN;
     return cmd;   
    }
    else if(strcmp(token,"SET")== 0){
        cmd.type = CMD_SET;

    }
    else if(strcmp(token,"GET") == 0){
        cmd.type = CMD_GET;

    }
    else if(strcmp(token,"DEL") == 0){
        cmd.type = CMD_DEL;
    }
    else if(strcmp(token,"EXIT") == 0){
        cmd.type = CMD_EXIT;
        return cmd;

    }
    else{
        cmd.type = CMD_UNKNOWN;
        return cmd;
    }



    token = strtok(NULL,delimiter);

    if(token == NULL){
        cmd.type = CMD_UNKNOWN;
        return cmd;
    }
    else{
        cmd.key = strdup(token);




    }
    
    token = strtok(NULL,delimiter);

    if(token == NULL && cmd.type == CMD_SET){
            cmd.type = CMD_UNKNOWN;
        }
    else if (token != NULL) {
        cmd.value = strdup(token);
    }
    
    // printf("%d %s %s\n",cmd.type,cmd.key,cmd.value);

    return cmd;
    




}

/* Free memory allocated inside Command */
void free_command(Command *cmd){
    free(cmd->key);
    free(cmd->value);
}

/* ================================
   COMMAND EXECUTION
   ================================ */

void execute_command(RedisDB *db, Command cmd){
    switch(cmd.type){
        case CMD_EXIT : 
        exit(0);
        break;
        case CMD_SET:
        if(db_set(db,cmd.key,cmd.value)==1) {printf("Done\n"); }
        else {printf("Error: Failed to set\n"); }
        break;
        case CMD_GET:
        char *ans = db_get(db,cmd.key);
        if(ans!=NULL) printf("Result: %s\n",ans );
        else printf("Error: Key does not exist\n");
        break;
        case CMD_DEL:
        if(db_del(db,cmd.key)==1) printf("Deleted\n");
        else printf("Error: Failed to delete\n");
        break;
        case CMD_UNKNOWN:
         printf("unknown command or not enough input\n");
         break;
        default:
         printf("Wrong input\n");
        
    }
    free_command(&cmd);
}

void execute_command_for_server(RedisDB *db,Command cmd,char *responseBuffer){
    switch(cmd.type){
        case CMD_EXIT : 
        strcpy(responseBuffer,"-This is not how it works\n");
        break;
        case CMD_SET:
        if(db_set(db,cmd.key,cmd.value)==1) {sprintf(responseBuffer,"+Done\n"); }
        else {sprintf(responseBuffer,"-Error: Failed to set\n"); }
        break;
        case CMD_GET:
        char *ans = db_get(db,cmd.key);
        if(ans!=NULL) sprintf(responseBuffer,"+Result: %s\n",ans );
        else sprintf(responseBuffer,"-Error: Key does not exist\n");
        break;
        case CMD_DEL:
        if(db_del(db,cmd.key)==1) sprintf(responseBuffer,"+Deleted\n");
        else sprintf(responseBuffer,"-Error: Failed to delete\n");
        break;
        case CMD_UNKNOWN:
         sprintf(responseBuffer,"-Unknown command or Not enough input\n");
         break;
        default:
         sprintf(responseBuffer,"-Wrong input\n");
        
    }
    free_command(&cmd);

}

/* Save database to disk (key=value format) */
int db_save(RedisDB *db, const char *filename){
    FILE* fp = fopen(filename,"wb");
    if(fp == NULL){
        perror("Failed to open file: ");
        return 0;
    }
    int count = db->count;
    fwrite(&count,sizeof(int),1,fp);

    for(int i = 0;i<TABLE_SIZE;i++){
        Entry * curr = db->table->buckets[i];

        while(curr){
            int keyLen = strlen(curr->key);
            fwrite(&keyLen,sizeof(keyLen),1,fp);
            fwrite(curr->key,keyLen,1,fp);
            int valueLen = strlen(curr->value.data);
            fwrite(&valueLen,sizeof(valueLen),1,fp);
            fwrite(curr->value.data,valueLen,1,fp);

            curr = curr->next;
        }


    }

    fclose(fp);
    printf("Database saved\n");
    return 1;

}

/* Load database from disk */
int db_load(RedisDB *db, const char *filename){
    FILE* fp= fopen(filename,"rb");

    if(fp == NULL){
        perror("Error: failed to open file\n");
        return 0;
    }

    int count;
    fread(&count,sizeof(count),1,fp);



    while(!feof(fp)){

        int keyLen;
        fread(&keyLen,sizeof(keyLen),1,fp);
        char * key = malloc(keyLen+1);
        fread(key,keyLen,1,fp);
        key[keyLen] = '\0';
        int valueLen;
        fread(&valueLen,sizeof(valueLen),1,fp);
        char * value = malloc(valueLen+1);
        fread(value,valueLen,1,fp);
        value[valueLen] = '\0';

        if(db_set(db,key,value)==1) printf("loaded values set\n");
        else printf("failed to set loaded\n");
        free(key);
        free(value);
      
    }


    fclose(fp);
    printf("Database loaded\n");

    return 1;
}

/* ================================
   MAIN LOOP 
   ================================ */

void redis_loop(RedisDB *db){

//  db_load(db,REDIS_DB);

    while(1){

    char inputBuffer[100];
    fgets(inputBuffer,99,stdin);
    execute_command(db,parse_command(inputBuffer));
    
  

    }

    // db_save(db,REDIS_DB);
}







