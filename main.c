#include<stdio.h>
#include "redislikeinc.h"

int main(){

    RedisDB* db = db_create();

    tcpServer(db);

    db_free(db);

    return 0;
}