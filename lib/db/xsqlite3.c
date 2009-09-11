

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

int db_exec(struct db *db, const char *q)
{
    int r;
    sqlite3_stmt *stmt;

    r = sqlite3_prepare(db->h, q, -1, &stmt, NULL);
    if(r != SQLITE_OK){
        r = -1;
        printf("Error: %s\n", sqlite3_errmsg(db->h));
        //log1(LERR, "Error: %s\n", sqlite3_errmsg(db->h));
        goto err;
    }

    r = sqlite3_step(stmt);
    while(r == SQLITE_ROW ){
        printf("!!\n");
        printf("%s;\n", sqlite3_column_text(stmt, 0));
        printf("%s;\n", sqlite3_column_text(stmt, 1));
        r = sqlite3_step(stmt);
    }
    sqlite3_finalize(stmt);

err:
    return r;
}

int main(int argc, char **argv)
{
    int err;
    struct db *dbh;

    dbh = db_open("my.db", &err);
    if(!dbh){
        return err;
    }
    db_exec(dbh, "create table x(id, val)");
    db_exec(dbh, "insert into x values('bla', '2')");
    db_exec(dbh, "select * from x");
    db_exec(dbh, "create table map (name varchar(20) NOT NULL PRIMARY KEY, \
        image_file blob NOT NULL);");
    db_exec(dbh, "insert into map values('map0', 'xxx')");



    db_close(dbh);

    return 0;
}

