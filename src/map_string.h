#ifndef MAP_STRING_H
#define MAP_STRING_H

#include "utillib.h"
#include <string.h>

/* 
 * Map for working with strings. 
 */

#define INIT_MAP_STRUCT( name )                                               \
typedef struct name##_t {                                                     \
   struct name##_t* next;                                                     \
   char* key;                                                                 \
   char* value;                                                               \
}  name##_t;                                                                  \

/*
 * 1. name_t* init_name_map( char* key, char* key )
 *     
 * 2. name_t* add_back_name_map( name_t* map, char* key, char* value)
 *    If map == NULL ret new map.
 * 3. static name_t* destroy_name_node( name_t node )
 *    Destroy node except next;
 * 4. void destroy_name_map( name_t* map ) 
 *    Destroy map.
 * 5. size_t name_map_length( const name_t* map ) 
 *    
 * 6. name_t* get_name_node( const name_t* map, const char* key) 
 *    Return one record.
 * 7. const char*  get_name_value( const name_t* map, const char* key )
 *    Return value from found record by the key.
 * 8. int set_name_value( const name_t* map, const char* key, char* value)
 *    Return OK success set or ENOMATCH (look utillib.h) on fail.
 * 9. char** name_map_to_array( const name_t* map ) 
 *    Return new array from the map. 
 * 10. name_t* name_array_to_map( const char** array )
 *    Return new map from the array.
 * 11. void destroy_name_array( char** array )
 *    Destroy array;
 */

#define INIT_MAP_FUNC( name )                                                 \
name##_t* init_##name##_map( char* key, char* value) {                        \
   name##_t* record = ALLOCATE( name##_t, 1);                                 \
   record->key   = key == NULL ? NULL : strdup(key);                          \
   record->value = value == NULL ? NULL : strdup(value);                      \
   record->next  = NULL;                                                      \
   return record;                                                             \
}                                                                             \
name##_t* add_back_##name##_map( name##_t* map, char* key, char* value)  {    \
   name##_t* tmp_map = init_##name##_map( key, value);                        \
   if(map) map->next = tmp_map;                                               \
   else  map = tmp_map;                                                       \
   return map;                                                                \
}                                                                             \
static name##_t* destroy_##name##_node( name##_t* node ) {                    \
   if( node ) {                                                               \
      name##_t* tmp = node->next;                                             \
      free(node->key); free(node->value); free(node);                         \
      return tmp;                                                             \
   }                                                                          \
   else                                                                       \
      return NULL;                                                            \
}                                                                             \
void destroy_##name##_map( name##_t* map ) {                                  \
   while((map = destroy_##name##_node(map)));                                 \
}                                                                             \
size_t name##_map_length( const name##_t* map ) {                             \
   size_t acc = 0;                                                            \
   name##_t* tmp = (name##_t*)map;                                            \
   while(tmp) { acc++; tmp = tmp->next; }                                     \
   return acc;                                                                \
}                                                                             \
name##_t* get_##name##_node( const name##_t* map, const char* key) {          \
   name##_t* tmp = (name##_t*)map;                                            \
   while(tmp){                                                                \
      if(strcmp(key, tmp->key) == 0 ) return tmp;                             \
      tmp = tmp->next;                                                        \
   }                                                                          \
   return NULL;                                                               \
}                                                                             \
const char*  get_##name##_value( const name##_t* map, const char* key ) {     \
   name##_t* tmp = get_##name##_node(map,key);                                \
   return tmp == NULL ? NULL : tmp->value;                                    \
}                                                                             \
int set_##name##_value( const name##_t* map, const char* key, char* value) {  \
   name##_t* tmp = get_##name##_node(map,key);                                \
   if(tmp->value) {                                                           \
      free(tmp->value);                                                       \
      tmp->value = strdup(value);                                             \
      return OK;                                                              \
   }                                                                          \
   return ENOMATCH;                                                           \
}                                                                             \
char** name##_map_to_array( const name##_t* map ) {                           \
   name##_t* tmp = (name##_t*)map;                                            \
   if(tmp == NULL) return NULL;                                               \
   size_t i = 0, length = name##_map_length(map);                             \
   char** array = ALLOCATE( char*, (length+1));                               \
   while(tmp) {                                                               \
      if(tmp->key == NULL ||  tmp->value == NULL) return NULL;                \
      char* tmp_str =ALLOCATE(char,(strlen(tmp->key)+strlen(tmp->value)+2));  \
      strcat(tmp_str,tmp->key); strcat(tmp_str,"=");                          \
      strcat(tmp_str,tmp->value);                                             \
      array[i++] = tmp_str;                                                   \
      tmp = tmp->next;                                                        \
  }                                                                           \
  array[i] = NULL;                                                            \
  return array;                                                               \
}                                                                             \
name##_t* name##_array_to_map( char** array ) {                               \
   name##_t* map = NULL;                                                      \
   size_t i = 0;                                                              \
   char* save, *cpy,*tmp;                                                     \
   while(array[i]) {                                                          \
      cpy = strdup(array[i++]);                                               \
      tmp = strtok_r( cpy, "=", &save);                                       \
      map = add_back_##name##_map( map, tmp, save);                           \
      free(cpy);                                                              \
   }                                                                          \
   return map;                                                                \
}                                                                             \
void destroy_##name##_array( char** array ) {                                 \
   size_t i = 0;                                                              \
   while(array[i]) free(array[i++]);                                          \
   free(array);                                                               \
}

#endif /* MAP_STRING_H */
