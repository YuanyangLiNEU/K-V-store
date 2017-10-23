#ifndef __hashtable_h__
#define __hashtable_h__

#include "util.h"

#ifndef TABLE_SIZE
#define TABLE_SIZE 997
#endif

typedef struct _Row Row;
struct _Row
{
   char key[128];
   char value[128];
   struct _Row *next;
};

// db - hashtable
Row** initDB();
Row* getRow(Row** hashtable, char *key);
void setRow(Row** hashtable, char *key, char *value);
void closeDB(Row** hashtable);

#endif