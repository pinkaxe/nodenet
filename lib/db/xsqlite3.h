#ifndef DB_XSQLITE3_H__
#define DB_XSQLITE3_H__

struct db {
    sqlite3 *h;
};

void db_close(struct db *db);

struct db *db_open(const char *file, int *err)

#endif
