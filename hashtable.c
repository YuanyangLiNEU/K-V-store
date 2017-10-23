#include "hashtable.h"

Row** initDB() {
   Row** hashtable = (Row **) malloc(sizeof(Row*) * TABLE_SIZE);
   for (int i = 0; i < TABLE_SIZE; ++i) {
      hashtable[i] = NULL;
   }

   return hashtable;
}

Row* getRow(Row** hashtable, char *key) {
   int index = computeHash(key) % TABLE_SIZE;
   Row *ele = hashtable[index];
   while (ele != NULL) {
      if (strcmp(ele->key, key) == 0) {
         return ele;
      }

      ele = ele->next;
   }

   return NULL;
}

void setRow(Row** hashtable, char *key, char *value) {
   Row* res = getRow(hashtable, key);
   if (res != NULL) {
      strcpy(res->value, value);
   } else {
      int index = computeHash(key) % TABLE_SIZE;
      Row *row = malloc(sizeof(Row));
      strcpy(row->key, key);
      strcpy(row->value, value);
      row->next = hashtable[index];
      hashtable[index] = row;
   }
}

void closeDB(Row** hashtable) {
   for (int i = 0; i < TABLE_SIZE; ++i) {
      Row *row = hashtable[i];
      while (row != NULL) {
         Row *tmp = row;
         row = row->next;
         free(tmp);
      }
   }

   free(hashtable);
}
