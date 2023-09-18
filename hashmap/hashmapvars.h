#ifndef HASHMAP_VARS_H_
#define HASHMAP__VARS_H_

#define SIZEVAR 200

struct DataItemVar {
   size_t data;   
   char *key;
};

struct DataItemVar *search_var(char *key);
void insert_var(char *key, size_t data);
struct DataItemVar* delete_vars(struct DataItem* item);
void display_vars();

#endif
