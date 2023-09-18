#ifndef HASHMAP_H_
#define HASHMAP_H_

#define SIZE 20

struct DataItem {
   char *data;   
   char key;
};

int hashCode(char key);
struct DataItem *search(int key);
void insert(int key, char *data);
struct DataItem* delete(struct DataItem* item);
void display();

#endif
