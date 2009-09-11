#ifndef __DB__
#define __DB__

struct db {
    sqlite3 *h;
};

void db_close(struct db *db);

struct db *db_open(const char *file, int *err)

#endif
