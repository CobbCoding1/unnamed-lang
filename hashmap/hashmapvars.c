// https://www.tutorialspoint.com/data_structures_algorithms/hash_table_program_in_c.htm
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>

#define SIZEVAR 20

struct DataItemVar {
  size_t data;   
  char *key;
};

struct DataItemVar* hashArrayVar[SIZEVAR]; 
struct DataItemVar* dummyItemVar;
struct DataItemVar* itemVar;

int hashCodeVar(char *key) {
  int hash, i;
  for(hash = i = 0; i < SIZEVAR; ++i)
  {
      hash += key[i];
      hash += (hash << 10);
      hash ^= (hash >> 6);
  }
  hash += (hash << 3);
  hash ^= (hash >> 11);
  hash += (hash << 15);
  return hash;
}

struct DataItemVar *search_var(char *key) {
   //get the hash 
   int hashIndex = hashCodeVar(key);  
	

   printf("THE KEY IS %s and the hashidnex is %d and the current key is \n", key, hashIndex);
   //move in array until an empty 
   while(hashArrayVar[hashIndex] != NULL) {
     printf("THE KEY IS %s and the hashidnex is %d and the current key is %s\n", key, hashIndex, hashArrayVar[hashIndex]->key);
	
      if(strcmp(hashArrayVar[hashIndex]->key, key) == 0)
         return hashArrayVar[hashIndex]; 
			
      //go to next cell
      ++hashIndex;
		
      //wrap around the table
      hashIndex %= SIZEVAR;
   }        
   printf("ERROR: %s NOT FOUND IN HASHMAP\n", key);
   exit(1);
   return NULL;        
}

void insert_var(char *key, size_t data) {

   struct DataItemVar *item = (struct DataItemVar*) malloc(sizeof(struct DataItemVar));
   item->data = data;  
   item->key = key;

   //get the hash 
   int hashIndex = hashCodeVar(key);
   printf("%d\n", hashIndex);

   //move in array until an empty or deleted cell
   while(hashArrayVar[hashIndex] != NULL) {
      //go to next cell
      ++hashIndex;
		
      //wrap around the table
      hashIndex %= SIZEVAR;
   }
	
   hashArrayVar[hashIndex] = item;
}

struct DataItemVar* delete_var(struct DataItemVar* item) {
   char *key = item->key;

   //get the hash 
   int hashIndex = hashCodeVar(key);

   //move in array until an empty
   while(hashArrayVar[hashIndex] != NULL) {
	
      if(hashArrayVar[hashIndex]->key == key) {
         struct DataItemVar* temp = hashArrayVar[hashIndex]; 
			
         //assign a dummy item at deleted position
         hashArrayVar[hashIndex] = dummyItemVar; 
         return temp;
      }
		
      //go to next cell
      ++hashIndex;
		
      //wrap around the table
      hashIndex %= SIZEVAR;
   }      
	
   return NULL;        
}

void display_vars() {
   int i = 0;
	
   for(i = 0; i<SIZEVAR; i++) {
	
      if(hashArrayVar[i] != NULL)
         printf(" (%s,%zu)",hashArrayVar[i]->key,hashArrayVar[i]->data);
      else
         printf(" ~~ ");
   }
	
   printf("\n");
}
