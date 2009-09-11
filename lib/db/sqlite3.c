
#include<stdlib.h>
#include<stdio.h>
#include<stdint.h>
#include<stdbool.h>
#include<stdlib.h>
#include<errno.h>

#include "util/log.h"

#include<sqlite3.h>

struct db {
    sqlite3 *h;
};

void db_close(struct db *db);

struct db *db_open(const char *file, int *err)
{
    struct db *db;

    db = malloc(sizeof(struct db));
    if(!db){
        *err = errno;
        goto err;
    }

    sqlite3_open(file, &db->h);
    if(!db->h || sqlite3_errcode(db->h) != SQLITE_OK){
        //log1(LERR, "Error: %s\n", sqlite3_errmsg(db->h));
        db_close(db);
        *err = -1;
    }

err:
    return db;
}

void db_close(struct db *db)
{
    if(db->h){
        sqlite3_close(db->h);
    }

    if(db){
        free(db);
    }
}


int main(int argc, char **argv)
{
    int err;
    struct db *dbh;

    dbh = db_open("my.db", &err);
    if(!dbh){
        return err;
    }

    db_close(dbh);

    return 0;
}

