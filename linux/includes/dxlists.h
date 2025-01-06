/*
 This module implements basic list objects.
 DeusEx 2023. Mourgis Nikolaos. deus-ex.gr

 List types :

 Simple : A simple list is a regular double linked list with a pointer member to store a DXL_OBJECT.
 You can sort a simple list with the dedicated functions , but then you HAVE to use the appropriate
 functions for the searching and addition of the objects. The list support the sorting of
 strings (PDX_STRING type) , integers (DXLONG64 type) and real types (double type).

 Dictionary : A dictionary is a hash table. The table is created with n buckets
 and can store,remove and find the special object PDXL_OBJECT

*/
#ifndef DXSTRINGS
#include "dxstrings.h"
#endif

#ifndef DXDATATYPES
#include "dxdatatypes.h"
#endif

#define DXLISTS

void dxUtf8TrimRight(char** str, char* utf8characters);


enum dx_sort_key_type {dx_str,dx_int,dx_float} ;
/*#
 This represent the sorting type of the list e.g key , int_key , float_key
#*/


/*
 basic double linked list object
*/
typedef struct dxl_node *PDXL_NODE ;
/*#
 A pointer type for the dxl_node object
#*/
typedef struct dx_list
/*#
 {
    PDXL_NODE  start ;
    PDXL_NODE  end   ;
    DXLONG64  count  ;
 } *PDX_LIST;

 The basic list object
#*/
 {
    PDXL_NODE  start ;
    PDXL_NODE  end   ;
    DXLONG64  count  ;
	DXLONG64  curr_indx ;
	PDXL_NODE curr_node ;
 } *PDX_LIST;

struct dxl_node
/*#
{
	void *object           ;
    PDXL_NODE left , right ;
    PDX_LIST lst		   ; //the list that the node is a member
}

 The basic list node object
#*/
 {
    PDXL_OBJECT object     ;
    PDXL_NODE left , right ;
    PDX_LIST lst		   ; //the list that the node is a member
 };


/*-------------------------------hash table structs*/

#define HASH_KEY 125668954

typedef struct dx_hash_table
/*#
{
  	PDX_LIST *buckets ; // this will be malloced as an array of lists
	uint32_t length   ; // how many buckets the table has
	DXLONG64 count    ; // how many items in the table
}*PDX_HASH_TABLE;
 The basic hash table structure.
 Remarks : You MUST use strings as keys of the same encoding (eg UTF8 , ANSI ,WIDESTRING) , else the hashes
 that will be produced will be different.
#*/
{
  	PDX_LIST *buckets ; // this will be malloced as an array of lists
	uint32_t length   ; // how many buckets the table has
	DXLONG64 count	  ; // how many items in the table 
}*PDX_HASH_TABLE;


/*---------------------------------------*/


//======dx_list==========================================================================|

PDXL_OBJECT dx_list_create_object(PDX_STRING strkey)		;
/*#
  Creates and returns a node object initialized with its object to NULL
  and its key to the strkey. The strkey can be ""
#*/

PDX_LIST  dx_list_create_list()							;
/*#
  Creates and returns a list with count = 0
#*/
PDXL_NODE dx_list_create_node(PDXL_OBJECT obj)			;
/*#
 Creates and returns a node and set its object to obj
#*/
PDXL_NODE dx_list_add_node(PDX_LIST list , PDXL_NODE node )			;
/*#
 Adds at the end of the list the node. Returns the node
#*/
PDXL_NODE dx_list_add_node_direct( PDX_LIST list , PDXL_OBJECT obj)		;
/*#
  Creates a node , sets its object to obj and insert it to the end of the list. Returns the node
#*/
PDXL_NODE dx_list_delete_node( PDXL_NODE node )						;
/*#
   Deletes the node from the list ( not the object just the node ). Returns NULL
#*/
PDXL_NODE dx_list_insert_node( PDX_LIST list ,  PDXL_NODE node , PDXL_NODE newnode );
/*#
 The [newnode] will take the position of the [node] and the [node] will be pushed 1 position to the right. Returns the newnode
#*/
void dx_list_swap_nodes(PDXL_NODE node1 , PDXL_NODE node2)			;
/*#
Swap the objects of the two objects in the nodes , works even if the nodes are in different lists
#*/
PDXL_NODE dx_list_go_to_pos( PDX_LIST list , DXLONG64 index )		;
/*#
  Starts from 0 return the [index] node in the list
  The function has been revised in 21-10-2024 to be more efficient.
  It does not starts from the start of the list until find the indx ,
  it use the internal state of the list to retrieve the index with the fastest way
  that the algorithm permits.

  REMARKS : Any change in the list resets the internal state of the list.
  for example adding a node or deleting a node will reset the internal state. 

#*/
PDXL_NODE dx_list_next( PDXL_NODE node)							    ;
/*#
Return the next node if node is NULL the last node is returned
#*/
PDXL_NODE dx_list_prev( PDXL_NODE node)							    ;
/*#
Return the previous node if node = NULL the first node is returned
#*/
PDXL_NODE dx_list_find_object( PDX_LIST list , PDXL_OBJECT obj )			;
/*#
Search the list to find the obj->obj. If found returns the FIRST node with the object else returns NULL. The obj->obj can be NULL.
#*/
PDX_LIST dx_list_empty_list( PDX_LIST list );
/*#
Deletes all the nodes NOT the objects in it. Return the [list]
#*/
PDX_LIST dx_list_delete_list( PDX_LIST list );
/*#
Deletes all the nodes and delete the list itself. Returns NULL
#*/
PDXL_NODE dx_list_safe_set_object( PDXL_NODE node , PDXL_OBJECT obj)		;
/*#
Sets the object of the [node] to obj , only if the object is NULL.
If the [node] object is already set then the function returns NULL ,
else it returns the [node]
#*/

PDX_LIST dx_list_clone_list( PDX_LIST list , DXLONG64 frompos )		;
/*#
Creates and returns an exact copy of the list if frompos = 0 else the new list is identical with the list starting frompos and right.
the objects in the new list is just references , so beware when free the memory of the objects to not be referenced
to multiple lists.
#*/
enum dx_compare_res dx_list_compare( PDX_LIST list1 , PDX_LIST list2 );
/*#
Compares the two lists (the node->object->obj) and returns the result as a dx_compare_res
#*/


DXLONG64 dx_list_find_key(PDX_LIST list , PDX_STRING key);
/*#
 The function do a linear search to find the give key. Is the only
 way for a non sorted list.
#*/

DXLONG64 dx_list_find_int_key(PDX_LIST list , DXLONG64 int_key);
/*#
 The function do a linear search to find the give key. Is the only
 way for a non sorted list.
#*/

DXLONG64 dx_list_find_float_key(PDX_LIST list , double float_key);
/*#
 The function do a linear search to find the give key. Is the only
 way for a non sorted list.
#*/
/*---------------- sorting routines ------------------*/

DXLONG64 dx_list_sort_find_key_asc(PDX_LIST list , PDX_STRING key);
/*#
 Find the position of the key IF the list is sorted by an asceding fashion. If the key does not exists -1 is returned
 Remarks : If an element is more than once inside the list then the function returns the position of the first key that it will find
#*/

DXLONG64 dx_list_sort_find_key_desc(PDX_LIST list , PDX_STRING key);
/*#
 Find the position of the key IF the list is sorted by a desceding fashion. If the key does not exists -1 is returned
 Remarks : If an element is more than once inside the list then the function returns the position of the first key that it will find
#*/

DXLONG64 dx_list_sort_find_int_key_asc(PDX_LIST list , DXLONG64 int_key);
/*#
 Find the position of the key IF the list is sorted by an asceding fashion. If the key does not exists -1 is returned
 Remarks : If an element is more than once inside the list then the function returns the position of the first key that it will find
#*/

DXLONG64 dx_list_sort_find_int_key_desc(PDX_LIST list , DXLONG64 int_key);
/*#
 Find the position of the key IF the list is sorted by a desceding fashion. If the key does not exists -1 is returned
 Remarks : If an element is more than once inside the list then the function returns the position of the first key that it will find
#*/

DXLONG64 dx_list_sort_find_float_key_asc(PDX_LIST list , double float_key);
/*#
 Find the position of the key IF the list is sorted by an asceding fashion. If the key does not exists -1 is returned
 Remarks : If an element is more than once inside the list then the function returns the position of the first key that it will find
#*/

DXLONG64 dx_list_sort_find_float_key_desc(PDX_LIST list , double float_key);
/*#
 Find the position of the key IF the list is sorted by a desceding fashion. If the key does not exists -1 is returned
 Remarks : If an element is more than once inside the list then the function returns the position of the first key that it will find
#*/

void dx_list_sort_keys(PDX_LIST list, bool asc) ;
/*#
 The function will sort the nodes in the list based of the node->object->key
 if asc is true then the sort will be asceding else desceding
#*/

void dx_list_sort_int_keys(PDX_LIST list, bool asc) ;
/*#
 The function will sort the nodes in the list based of the node->object->int_key
 if asc is true then the sort will be asceding else desceding
#*/

void dx_list_sort_float_keys(PDX_LIST list, bool asc) ;
/*#
 The function will sort the nodes in the list based of the node->object->float_key
 if asc is true then the sort will be asceding else desceding
#*/

PDXL_NODE dx_list_add_node_direct_sort( PDX_LIST list , PDXL_OBJECT obj , bool asc,enum dx_sort_key_type type)	;
/*
 The function will add a new object in a SORTED list in the correct position (sorting position)
 based on the obj->key.
 The calling application has the responsibility to know if the list is sorted or not.
 If the asc is true then the list is handled as a asceding sorted list ,
 else the list is handled as an descending sorted list
*/
/*--------------- end of sorting routines -----------*/


/************** HASH ROUTINES ************************/

uint32_t dx_CreateHash(const void *key , int key_len , uint32_t limit_to);
/*#
 The function creates a 4 byte unsigned integer number from the [key] string.
 The limit_to is the maximum number that the number can be. For example if the
 hash table size is 1000 then the limit_to must be 999 .
 Remarks : No need to use this function directly , the appropriate functions use this
 internally.
#*/

PDX_HASH_TABLE dx_HashCreateTable(uint32_t max_buckets);
/*#
 The function creates a Hash Table witn max_buckets bucket count.
 The Hash Table is a dynamic allocated array of PDX_LIST.
#*/

typedef void(*hash_free_custom_object)(PDXL_OBJECT obj);
PDX_HASH_TABLE dx_HashDestroyTable(PDX_HASH_TABLE table,hash_free_custom_object free_func);
/*#
 This function destroys the table.
 Remarks : The function hash_free_custom_object is called with the PDXL_OBJECT of the node in the buckets
		   as a parameter so the application can free the memory of the obj member of it it.
		   The actual PDXL_OBJECT of the node is freed inside the dx_HashDestroyTable. 
#*/

uint32_t dx_HashInsertObject(PDX_HASH_TABLE table ,PDXL_OBJECT object);
/*#
 The function inserts a new object in the hash table.
 The object->key member will be used , so it must be set correctly.
 Returns the index (hash number) of the bucket .
#*/

bool dx_HashBucketEmpty(PDX_HASH_TABLE table , PDX_STRING str);
/*#
 The function return true if the bucket that the key resolve to
 is empty and false if the backet has any elements in it.
#*/

PDXL_OBJECT dx_HashReturnItem(PDX_HASH_TABLE table , PDX_STRING str,bool compare_binary);
/*#
 Return the item that correspond to this particular key.
 If there is more items with the same key , then the first in the bucket
 will be retrieved.
 Returns NULL if the str does not match any item

 if the compare_binary is true then the function does not do any checks and conversions
 from different encodings and compare byte to byte and not character to character. This make the function faster(2 to 3 times faster).

#*/

PDXL_OBJECT dx_HashRemoveItem(PDX_HASH_TABLE table , PDX_STRING str);
/*#
 Removes from the table and returns the item that correspond to this particular key.
 If there is more items with the same key , then the first in the bucket
 will be removed .
 Returns NULL if the str is not present
#*/

DXLONG64 dx_HashItemCount(PDX_HASH_TABLE table);
/*#
 Returns the total items 
#*/

PDXL_OBJECT dx_HashItemByIndex(PDX_HASH_TABLE table ,DXLONG64 indx) ;
/*#
 Returns the object that finds after passes indx-1 objects 
#*/


/*--------------------------------------------------*/

/*A simple STRING LIST ONLY for UTF8 strings*/

typedef PDX_LIST PDX_STRINGLIST ;

PDX_STRINGLIST dx_stringlist_create();
/*#
 Creates a new string list. If the malloc fails NULL is returned
#*/
void *dx_stringlist_free(PDX_STRINGLIST list);
/*#
 Frees a string list with all its strings, returns NULL
#*/
void dx_stringlist_add_raw_string(PDX_STRINGLIST list,char *str) ;
/*#
 Copy the str in a PDX_STRING and add it to the list
#*/

void dx_stringlist_add_string(PDX_STRINGLIST list,PDX_STRING str) ;
/*#
 Add the PDX_STRING in the list. (does not copy it , it insert the actual object)
#*/

void dx_stringlist_remove_str(PDX_STRINGLIST list ,DXLONG64 indx);
/*#
 Removes the string in the indx position from the list
#*/

void dx_stringlist_clear(PDX_STRINGLIST list);
/*#
 Clears (and free the memory of) all the string of the list
#*/

PDX_STRING dx_stringlist_text(PDX_STRINGLIST list,bool unix_style) ;
/*#
 Concatenate all the strings in the list as a text.
 Every string is separated by an end of line. If unix_style is true then
 the lines are separated by a \n else by an \r\n
#*/

PDX_STRING dx_stringlist_raw_text(PDX_STRINGLIST list) ;
/*#
 The function return all the string list lines concatenated.
#*/

PDX_STRING dx_stringlist_string(PDX_STRINGLIST lst , DXLONG64 pos);
/*#
 The function return the PDX_STRING of the pos(ition).
 The PDX_STRING is not a copy , is the object in the list so do not free it
 casually.
#*/

PDX_STRING dx_stringlist_find(PDX_STRINGLIST lst , PDX_STRING str);
/*#
 The function search to find the str in the lst. This is a
 simple inefficient serial search
#*/

PDX_STRINGLIST dx_stringlist_concat(PDX_STRINGLIST lst, PDX_STRINGLIST add_list);
/*#
 The function deep copies the elements of the add_list and add it in the end of the lst.
 Returns the lst
#*/
/*---*/
DXLONG64 dx_stringlist_strlen(PDX_STRINGLIST list);
/*#
 The function returns the string length of all the elements in the list.
 return -1 if an error occurs
#*/

DXLONG64 dx_stringlist_bytelen(PDX_STRINGLIST list);
/*#
 The function returns the byte count of all the strings in the list.
 return -1 if an error occurs
#*/

PDX_STRINGLIST dx_stringlist_load_text_ex(PDX_STRINGLIST list , PDX_STRING str , int break_every);
/*#
 The function adds the str to the list BUT it does not add it as one singular string. 
 The function counts bytes and when the characters reach the break_every then a dx_string is
 saved as a new one is created for the remaining characters.
 This is usefull in operations like character or words replace , so we avoid to alloc and free large memory buffers. 
#*/

PDX_STRINGLIST dx_stringlist_remove_char(PDX_STRINGLIST list,char *utf8char);
/*#
 The function remove the character from all the strings from the list
#*/

PDX_STRINGLIST dx_stringlist_replace_char(PDX_STRINGLIST list , char *utf8char , char *replacewith);
/*#
 The function remove the utf8char from all the strings from the list
#*/

DXLONG64 dx_stringlist_find_word(PDX_STRINGLIST list,char *word,DXLONG64 from_indx,DXLONG64 *line,DXLONG64 *line_ch_indx) ;
/*#
 The function is starting from_indx . 
 The function return the indx as the absolute index that the first character of the word resides ,
 the line is set to the line the word is found , the line_ch_indx is the index of the first character of the word in the particular line.
 if the word is not found then all the out parameters (except the word) are returned as -1
#*/

DXLONG64 dx_stringlist_find_word_binary(PDX_STRINGLIST list,char *word,DXLONG64 from_indx,DXLONG64 *line,DXLONG64 *line_ch_indx) ;
/*#
 The function is starting from_indx (byte indx) . 
 The function return the indx as the absolute byte index that the first byte of the word resides ,
 the line is set to the line the word is found , the line_ch_indx is the byte index of the character in the particular line.
 if the word is not found then all the out parameters (except the word) are returned as -1
#*/


DXLONG64 dx_utf8_replace_word(PDX_STRING source,DXLONG64 from_index,PDX_STRING word,PDX_STRING replace_with,bool replace_all);
/*#
 The function finds the word in the string and replace it with the replace_with. If the word is empty the function do nothing.
 If the replace_all is true then all the word instances in the string will be replaced, else only the first occurence will be replaced.
 The function returns -1 if no replacement was done or the position of the first character of the last word replaced in the string.
 Keep in mind that the returned position is in utf8 characters and not in bytes. 

 WARNING : The function alters the source.
#*/

DXLONG64 dx_replace_word_binary(PDX_STRING source,DXLONG64 from_index,PDX_STRING word,PDX_STRING replace_with,bool replace_all);
/*#
 The function finds the word in the string and replace it with the replace_with. If the word is empty the function do nothing.
 If the replace_all is true then all the word instances in the string will be replaced, else only the first occurence will be replaced.
 The function returns -1 if no replacement was done or the position of the first byte of the last word replaced in the string.
 Keep in mind that the returned position is the index of the first byte of the first character

 WARNING : The function alters the source.
#*/


DXLONG64 dx_replace_word_unicode(PDX_STRING source,DXLONG64 from_index,PDX_STRING word,PDX_STRING replace_with,bool replace_all);
/*#
 The function finds the word in the string and replace it with the replace_with. If the word is empty the function do nothing.
 If the replace_all is true then all the word instances in the string will be replaced, else only the first occurence will be replaced.
 The function returns -1 if no replacement was done or the position of the first byte of the last word replaced in the string.
 Keep in mind that the returned position is the index of the first byte of the first character

 WARNING : The function alters the source.
#*/


//=================IMPLEMENTATION=============================


/************** Quick Sort **********************/

void dx_list_array_swap(PDXL_OBJECT *obj1 , PDXL_OBJECT *obj2)
{
	PDXL_OBJECT temp = *obj1;
	*obj1 = *obj2 ;
	*obj2 = temp ;
}

/***************** SORTING STRING **********************************/

DXLONG64 dx_list_partition( PDXL_OBJECT *arr, DXLONG64 low , DXLONG64 high,bool asc)
{
  //choose the pivot
  PDXL_OBJECT pivot = arr[high];
  //Index of smaller element and Indicate
  //the right position of pivot found so far
  DXLONG64 i=(low-1);
  enum dx_compare_res comp_type;
  if(asc == true) comp_type = dx_right_bigger;
  else
	 comp_type = dx_left_bigger;

  for(int j=low;j<=high;j++)
  {
    //If current element is smaller than the pivot
    if(dx_string_lex_cmp(arr[j]->key,pivot->key) == comp_type)
    {
      //Increment index of smaller element
      i++;
      dx_list_array_swap(&arr[i],&arr[j]);
    }
  }
  dx_list_array_swap(&arr[i+1],&arr[high]);
  return (i+1);
}

void dx_list_quickSort(PDXL_OBJECT *arr, DXLONG64 low, DXLONG64 high,bool asc)
{
  // when low is less than high
  if(low<high)
  {
    // pi is the partition return index of pivot

    DXLONG64 pi=dx_list_partition(arr,low,high,asc);

    //Recursion Call
    //smaller element than pivot goes left and
    //higher element goes right
    dx_list_quickSort(arr,low,pi-1,asc);
    dx_list_quickSort(arr,pi+1,high,asc);
  }
}
/*****************************************************************/

/******************************* INTEGER SORT****************************************/
DXLONG64 dx_list_partition_int( PDXL_OBJECT *arr, DXLONG64 low , DXLONG64 high,bool asc)
{
  //choose the pivot
  PDXL_OBJECT pivot = arr[high];
  //Index of smaller element and Indicate
  //the right position of pivot found so far
  DXLONG64 i=(low-1);
  enum dx_compare_res comp_type;
  if(asc == true) comp_type = dx_right_bigger;
  else
	 comp_type = dx_left_bigger;

  for(int j=low;j<=high;j++)
  {
    //If current element is smaller than the pivot
    if(dx_int_compare(arr[j]->int_key,pivot->int_key) == comp_type)
    {
      //Increment index of smaller element
      i++;
      dx_list_array_swap(&arr[i],&arr[j]);
    }
  }
  dx_list_array_swap(&arr[i+1],&arr[high]);
  return (i+1);
}

void dx_list_quickSort_int(PDXL_OBJECT *arr, DXLONG64 low, DXLONG64 high,bool asc)
{
  // when low is less than high
  if(low<high)
  {
    // pi is the partition return index of pivot

    DXLONG64 pi=dx_list_partition_int(arr,low,high,asc);

    //Recursion Call
    //smaller element than pivot goes left and
    //higher element goes right
    dx_list_quickSort_int(arr,low,pi-1,asc);
    dx_list_quickSort_int(arr,pi+1,high,asc);
  }
}
/************************************************************************/


/******************************* FLOAT SORT****************************************/
DXLONG64 dx_list_partition_float( PDXL_OBJECT *arr, DXLONG64 low , DXLONG64 high,bool asc)
{
  //choose the pivot
  PDXL_OBJECT pivot = arr[high];
  //Index of smaller element and Indicate
  //the right position of pivot found so far
  DXLONG64 i=(low-1);
  enum dx_compare_res comp_type;
  if(asc == true) comp_type = dx_right_bigger;
  else
	 comp_type = dx_left_bigger;

  for(int j=low;j<=high;j++)
  {
    //If current element is smaller than the pivot
    if(dx_float_compare(arr[j]->float_key,pivot->float_key) == comp_type)
    {
      //Increment index of smaller element
      i++;
      dx_list_array_swap(&arr[i],&arr[j]);
    }
  }
  dx_list_array_swap(&arr[i+1],&arr[high]);
  return (i+1);
}

void dx_list_quickSort_float(PDXL_OBJECT *arr, DXLONG64 low, DXLONG64 high,bool asc)
{
  // when low is less than high
  if(low<high)
  {
    // pi is the partition return index of pivot

    DXLONG64 pi=dx_list_partition_float(arr,low,high,asc);

    //Recursion Call
    //smaller element than pivot goes left and
    //higher element goes right
    dx_list_quickSort_float(arr,low,pi-1,asc);
    dx_list_quickSort_float(arr,pi+1,high,asc);
  }
}
/************************************************************************/

/************** Quick Sort *********************/


//+++PDX_LIST+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++|

PDXL_OBJECT dx_list_create_object(PDX_STRING strkey)
{
	PDXL_OBJECT obj =dxl_object_create();
	if (obj == NULL) return NULL;
	obj->obj = NULL ;
	obj->key = dx_string_clone(strkey);
	return obj ;
}

PDX_LIST dx_list_create_list()
{
    PDX_LIST list = (PDX_LIST)malloc(sizeof(struct dx_list));
	if (list == NULL) return NULL;
    list->count   = 0 ;
    list->end     = NULL ;
    list->start   = NULL ;
    list->curr_indx = 0  ;
	list->curr_node = list->start ;
	return list ;
}

PDXL_NODE dx_list_create_node(PDXL_OBJECT obj)
{
    PDXL_NODE node = (PDXL_NODE)malloc(sizeof(struct dxl_node));
	if (node == NULL) return NULL;
    node->left = NULL ;
    node->right = NULL ;
    node->object = obj ;
    return node ;
}

PDXL_NODE dx_list_add_node_direct( PDX_LIST list , PDXL_OBJECT obj)
{
	if ( list == NULL ) return NULL ;
    PDXL_NODE node = dx_list_create_node( obj );
    dx_list_add_node( list , node );
    return node ;
}

PDXL_NODE dx_list_add_node(PDX_LIST list , PDXL_NODE node )
{
    if ( (node == NULL) || ( list == NULL ) ) return NULL ;


    node->lst = list;
    if ( list->start == NULL )
    {
        // the first element
       list->start = node ;
       list->end = node ;
       list->count = 1 ;

	   	//reset internal state 
	   list->curr_indx = 0  ;
	   list->curr_node = list->start ;

       return node ;
    }
    // if the list has not a defined start and end
    if ( list->count == 1 )
    {
        list->end = node ;
        list->end->left = list->start ;
        list->start->right = list->end ;
        list->count++ ;
       
		//reset internal state 
	    list->curr_indx = 0  ;
	    list->curr_node = list->start ;
		
		return node ;
    }

    // add other
    node->left = list->end;
    list->end->right = node ;
    list->end = node;
    list->count++ ;

	//reset internal state 
	list->curr_indx = 0  ;
	list->curr_node = list->start ;

    return node ;
}

PDXL_NODE dx_list_delete_node( PDXL_NODE node )
{
    if (node == NULL)  return NULL ;
	
	if (node->lst->count == 1)
    {
        PDX_LIST list = node->lst;
        list->count = 0;
        list->end   = NULL;
        list->start = NULL ;
        free(node);

		//reset internal state 
	    list->curr_indx = 0  ;
	    list->curr_node = list->start ;

		//reset internal state 
		list->curr_indx = 0  ;
		list->curr_node = list->start ;

        return NULL ;
    }

    //check if its the start or end
	PDX_LIST list = node->lst ;
    if ( node == node->lst->start ) node->lst->start = node->right ;
    if ( node == node->lst->end ) node->lst->end = node->left;
    // make the reassigment
    if ( node->left != NULL ) node->left->right = node->right;
    if ( node->right != NULL ) node->right->left = node->left ;
    // delete the memory
    node->lst->count--;
    free( node );

	//reset internal state 
	list->curr_indx = 0  ;
	list->curr_node = list->start ;

    return NULL;
}

PDXL_NODE dx_list_insert_node( PDX_LIST list ,  PDXL_NODE node , PDXL_NODE newnode )
{
    if ( ( node == NULL )||( newnode == NULL )||( list == NULL ) ) return NULL ;

	newnode->lst   = list ;
    newnode->left  = node->left ;
    newnode->right = node;
    if ( node->left != NULL )node->left->right = newnode;
    node->left = newnode;
    if ( newnode->left == NULL ) list->start = newnode;

    list->count++;

	//set internal state to the new node 
	list->curr_indx = 0  ;
	list->curr_node = list->start ;

    return newnode ;
}

void dx_list_swap_nodes(PDXL_NODE node1 , PDXL_NODE node2)
{
	/*we do not reset the internal indexing state as the list does not really change for indexing puproses*/
    if ( (node1 == NULL) || (node2 == NULL) ) return ;

    PDXL_OBJECT tmpo = node1->object    ;
    node1->object = node2->object ;
    node2->object = tmpo  		  ;
    return;
}

/*the following is obsolete*/
PDXL_NODE dx_list_go_oto_pos( PDX_LIST list , DXLONG64 index )
{
    if ( list == NULL ) return NULL ;
    if ( index > list->count -1 ) return NULL ;
	if(index < 0 ) return NULL ;
    if ( index == 0 ) return list->start;
    if ( index == list->count - 1) return list->end;
    //parse the list,  this is the next best, the first best would be a
	//static memory table e.g an array
    PDXL_NODE node = list->start ;
    for ( DXLONG64 indx = 0 ; indx < index ; indx++ )
    {
     node = node->right ;
    }

    return node;
}

PDXL_NODE dx_list_go_to_pos( PDX_LIST list , DXLONG64 index )
{

	/*
	 Algorithm ->  Check the index distance with start , with end and with current index.
	 The smaller distance is the most efficient , The node will be used to find the correct node.
	 when the correct node is found the current index will be set to the index and the current node will be set to the node.
	 The most efficient case (for concured readings) is when the function is called in a loop that only reads the list , as the 
	 function returns the next node every time and no internal state reset is being done
	*/

    if ( list == NULL ) return NULL ;
    if ( index > list->count -1 ) return NULL ;
	if(index < 0 ) return NULL ;
    if ( index == 0 ) return list->start;
    if ( index == list->count - 1) return list->end;
	if (index == list->curr_indx) return list->curr_node ;
    
	//find the best way to access the list,  this is the next best, the first best would be a
	//static memory table e.g an array

	DXLONG64 sdelta = index ;
	DXLONG64 edelta = (list->count - 1) - index ;
	DXLONG64 cdelta = (list->curr_indx - index)  ;

	//find the smallest distance
	int d_type = 0 ; // 0 start 1 end 2 current

	if(sdelta>edelta) d_type  = 1 ;
	if(edelta>abs(cdelta)) d_type  = 2 ;

	PDXL_NODE node = NULL ;
	if(d_type == 0)
	{
		/*simple accessing the list from the beginning*/
		node = list->start ;
		for ( DXLONG64 indx = 0 ; indx < index ; indx++ )
		{
		 node = node->right ;
		}
	}
	else
	 if(d_type == 1)
	 {
	   /*simple accessing the list from the end*/
		node = list->end ;
		for ( DXLONG64 indx = list->count-1 ; indx > index ; indx-- )
		{
		 node = node->left ;
		}
	 }
	 else
	 {
	 
		 if(cdelta > 0)
		 {
		   /*we will backtrack from the current index*/
		   node = list->curr_node ;
		   for ( DXLONG64 indx = list->curr_indx ; indx > index ; indx-- )
		   {
		     node = node->left ;
		   }
		 }
		 else
		 {
		    node = list->curr_node ;
		    for ( DXLONG64 indx = list->curr_indx ; indx < index ; indx++ )
		    {
		     node = node->right ;
		    }
		 }


	 }

	/*update the internal state*/
	list->curr_indx = index ;
	list->curr_node = node ;
    return node;
}

PDXL_NODE dx_list_next( PDXL_NODE node)
{
	//this is not bound to the indexing internal scheme
	if(node == NULL) return node->lst->end ;
	return node->right ;
}

PDXL_NODE dx_list_prev( PDXL_NODE node)
{
	//this is not bound to the indexing internal scheme
	if(node == NULL) return node->lst->start ;
	return node->left ;
}

PDXL_NODE dx_list_find_object( PDX_LIST list , PDXL_OBJECT obj )
{
   if ( list == NULL ) return NULL ;

   PDXL_NODE node = list->start ;
   while ( node != NULL )
   {
       if ( node->object->obj == obj->obj ) return node;
       node = node->right ;
   }

   return NULL;
}

PDX_LIST dx_list_empty_list( PDX_LIST list)
{
   if ( (list == NULL) || ( list->start == NULL ) ) return list ;

   PDXL_NODE node = list->end ;
   while ( node != list->start )
   {
       node = node->left ;
       free( node->right );
   }
   // free last one
   free( node );
   list->end = NULL ;
   list->start = NULL ;
   list->count = 0 ;

   	//reset internal state 
	list->curr_indx = 0  ;
	list->curr_node = list->start ;

   return list ;
}

PDX_LIST dx_list_delete_list( PDX_LIST list )
{
    if ( list == NULL ) return NULL ;
    dx_list_empty_list( list );
    free( list );
    return NULL ;
}

PDXL_NODE dx_list_safe_set_object( PDXL_NODE node , PDXL_OBJECT obj)
{
	if ( node == NULL ) return NULL ;
    if ( node->object == NULL )
	{
		node->object = obj ;
		return node ;
	}

    return NULL ;
}

PDX_LIST dx_list_clone_list( PDX_LIST list , DXLONG64 frompos )
{
	if ( list == NULL ) return NULL ;
    PDX_LIST newlist = dx_list_create_list();
    if ( newlist == NULL ) return NULL ;

    PDXL_NODE node = dx_list_go_to_pos(list,frompos) ;
    while( node->right != NULL )
    {
      dx_list_add_node_direct( newlist , node->object );
      node = node->left ;
    }

   return newlist ;
}

enum dx_compare_res dx_list_compare( PDX_LIST list1 , PDX_LIST list2 )
{

    if ( ( list1 == NULL )||( list2 == NULL ) ) return dx_error ;
    if ( list1->count != list2->count ) return dx_not_equal ;
    PDXL_NODE n1 , n2 ;
    n1 = list1->start ;
    n2 = list2->start ;
    for ( DXLONG64 i = 0 ; i < list1->count ; i++ )
    {
        if ( n1->object != n2->object  ) return dx_not_equal ;
        n1 = n1->right;
        n2 = n2->right;
    }

    return dx_equal;
}



DXLONG64 dx_list_find_key(PDX_LIST list , PDX_STRING key)
{
	if (list->count == 0) return -1 ;
	PDXL_NODE node = list->start ;
	DXLONG64 indx = 0 ;
	while(node != NULL)
	{
		PDXL_OBJECT object = node->object ;
		if (dx_string_compare(object->key,key) == dx_equal ) return indx ;
		node = node->right ;
		indx++ ;
	}

	return  -1 ;
}

DXLONG64 dx_list_find_int_key(PDX_LIST list , DXLONG64 int_key)
{
	if (list->count == 0) return -1 ;
	PDXL_NODE node = list->start ;
	DXLONG64 indx = 0 ;
	while(node != NULL)
	{
		PDXL_OBJECT object = node->object ;
		if (dx_int_compare(object->int_key,int_key) == dx_equal ) return indx ;
		node = node->right ;
		indx++ ;
	}

	return  -1 ;
}


DXLONG64 dx_list_find_float_key(PDX_LIST list , double float_key)
{
	if (list->count == 0) return -1 ;
	PDXL_NODE node = list->start ;
	DXLONG64 indx = 0 ;
	while(node != NULL)
	{
		PDXL_OBJECT object = node->object ;
		if (dx_float_compare(object->float_key,float_key) == dx_equal ) return indx ;
		node = node->right ;
		indx++ ;
	}

	return  -1 ;
}



void dx_list_sort_keys(PDX_LIST list, bool asc)
{
	/*
	 The function will use the quicksort algorithm but with a twist.
	 An array will be created from the list and the array elements will be
	 sorted. After the sorting the list will be rewrited from the array.
	 This have the benefit of the speed of the quicksort in a memory array,
	 and realistically the extra memory that we will use for the sorting is
 	 very little , as we will store pointers to the objects. So for 1000 elements
	 we will use (for 64bit systems typicaly the pointer size is 8 bytes ) 8000 bytes
	 that for the modern systems are not even considered.
	 I do not have benchmarked the two approaches i.e the quicksort in the
	 list itsef and this technique. In some bibliography i found this technique as preferable,
	 well i will contact some experiments later.
	 The comparison will be made with the dx_string_lex_cmp function.
	*/


	if( list == NULL )  return  ;
	if(list->count < 2) return  ;

	/*
	 create the array
	*/
	//reserve memory for the pointers that will point to the actual list objects
	PDXL_OBJECT *sortbuffer = (PDXL_OBJECT*)malloc(sizeof(PDXL_OBJECT)*list->count) ;
	//copy all the pointers to the array
	PDXL_NODE node = list->start ;
	PDXL_OBJECT *bind = sortbuffer ;
	while(node != NULL)
	{
	  *bind = node->object ;
	  bind++ ;
      node = node->right ;
	}

	//apply the quicksort to the array
	dx_list_quickSort(sortbuffer, 0 , list->count-1,asc);
	/*now we will rewrite the pointers in the list in the ordered form*/

	node = list->start ;
	bind = sortbuffer ;
	while(node != NULL)
	{
	  node->object = *bind ;
	  bind++ ;
      node = node->right ;
	}

	return ;
}


void dx_list_sort_int_keys(PDX_LIST list, bool asc)
{
/*
	 The function will use the quicksort algorithm but with a twist.
	 An array will be created from the list and the array elements will be
	 sorted. After the sorting the list will be rewrited from the array.
	 This have the benefit of the speed of the quicksort in a memory array,
	 and realistically the extra memory that we will use for the sorting is
 	 very little , as we will store pointers to the objects. So for 1000 elements
	 we will use (for 64bit systems typicaly the pointer size is 8 bytes ) 8000 bytes
	 that for the modern systems are not even considered.
	 I do not have benchmarked the two approaches i.e the quicksort in the
	 list itsef and this technique. In some bibliography i found this technique as preferable,
	 well i will contact some expiraments later.
	*/


	if( list == NULL )  return  ;
	if(list->count < 2) return  ;

	/*
	 create the array
	*/
	//reserve memory for the pointers that will point to the actual list objects
	PDXL_OBJECT *sortbuffer = (PDXL_OBJECT*)malloc(sizeof(PDXL_OBJECT)*list->count) ;
	//copy all the pointers to the array
	PDXL_NODE node = list->start ;
	PDXL_OBJECT *bind = sortbuffer ;
	while(node != NULL)
	{
	  *bind = node->object ;
	  bind++ ;
      node = node->right ;
	}

	//apply the quicksort to the array
	dx_list_quickSort_int(sortbuffer, 0 , list->count-1,asc);
	/*now we will rewrite the pointers in the list in the ordered form*/

	node = list->start ;
	bind = sortbuffer ;
	while(node != NULL)
	{
	  node->object = *bind ;
	  bind++ ;
      node = node->right ;
	}

	return ;
}

void dx_list_sort_float_keys(PDX_LIST list, bool asc)
{
/*
	 The function will use the quicksort algorithm but with a twist.
	 An array will be created from the list and the array elements will be
	 sorted. After the sorting the list will be rewrited from the array.
	 This have the benefit of the speed of the quicksort in a memory array,
	 and realistically the extra memory that we will use for the sorting is
 	 very little , as we will store pointers to the objects. So for 1000 elements
	 we will use (for 64bit systems typicaly the pointer size is 8 bytes ) 8000 bytes
	 that for the modern systems are not even considered.
	 I do not have benchmarked the two approaches i.e the quicksort in the
	 list itsef and this technique. In some bibliography i found this technique as preferable,
	 well i will contact some expiraments later.
	*/


	if( list == NULL )  return  ;
	if(list->count < 2) return  ;

	/*
	 create the array
	*/
	//reserve memory for the pointers that will point to the actual list objects
	PDXL_OBJECT *sortbuffer = (PDXL_OBJECT*)malloc(sizeof(PDXL_OBJECT)*list->count) ;
	//copy all the pointers to the array
	PDXL_NODE node = list->start ;
	PDXL_OBJECT *bind = sortbuffer ;
	while(node != NULL)
	{
	  *bind = node->object ;
	  bind++ ;
      node = node->right ;
	}

	//apply the quicksort to the array
	dx_list_quickSort_float(sortbuffer, 0 , list->count-1,asc);
	/*now we will rewrite the pointers in the list in the ordered form*/

	node = list->start ;
	bind = sortbuffer ;
	while(node != NULL)
	{
	  node->object = *bind ;
	  bind++ ;
      node = node->right ;
	}

	return ;
}

DXLONG64 dx_list_sort_find_key_asc(PDX_LIST list , PDX_STRING key)
{
  if (list->count == 0) return -1 ;
  /*binary search for the key in the list*/
  DXLONG64 top	  = 0 ;
  DXLONG64 bottom = list->count -1 ;
  DXLONG64 pos = -1   ;
  /*check first and last nodes*/
  PDXL_NODE    cnode = list->start   ;
  PDXL_OBJECT cobj	= cnode->object ;
  enum dx_compare_res cres = dx_string_lex_cmp(cobj->key,key) ;
  if (cres == dx_equal) return 0 ;

  cnode = list->end   ;
  cobj	= cnode->object ;
  cres = dx_string_lex_cmp(cobj->key,key) ;
  if (cres == dx_equal) return list->count-1 ;

  if (list->count <= 2) return -1 ;
  /*no luck! search for the key*/
  while(true)
  {
	 DXLONG64 mid = (bottom - top) / 2 ;
	 cnode = dx_list_go_to_pos(list,top + mid);
	 cobj = cnode->object ;

	 enum dx_compare_res cres = dx_string_lex_cmp(cobj->key,key) ;
     if( cres == dx_equal )
	 {
		pos = top + mid ;
		return pos ;
	 }
	  else
	 if( cres == dx_left_bigger)
	 {
		 bottom = bottom - mid ;
		 if ((((top + mid) == bottom)&&(mid == 1))||(mid == 0))
		 {

			 return -1 ;
		 }
	 }
	 else
	 {
		 top = top + mid ;
		 if ((((bottom + mid) == top)&&(mid == 1))||(mid == 0) )
		 {
		   return -1 ;
		 }
	 }

  }

  return -1 ;
}

DXLONG64 dx_list_sort_find_key_desc(PDX_LIST list , PDX_STRING key)
{
  if (list->count == 0) return -1 ;
  /*binary search for the key in the list*/
  DXLONG64 top	  = 0 ;
  DXLONG64 bottom = list->count -1 ;
  DXLONG   pos = -1   ;
  /*check first and last nodes*/
  PDXL_NODE    cnode = list->start   ;
  PDXL_OBJECT cobj	= cnode->object ;
  enum dx_compare_res cres = dx_string_lex_cmp(cobj->key,key) ;
  if (cres == dx_equal) return 0 ;

  cnode = list->end   ;
  cobj	= cnode->object ;
  cres = dx_string_lex_cmp(cobj->key,key) ;
  if (cres == dx_equal) return list->count-1 ;

  if (list->count <= 2) return -1 ;
  /*no luck! search for the key*/
  while(true)
  {
	 DXLONG64 mid = (bottom - top) / 2 ;
	 cnode = dx_list_go_to_pos(list,top + mid);
	 cobj = cnode->object ;

	 enum dx_compare_res cres = dx_string_lex_cmp(cobj->key,key) ;

     if( cres == dx_equal )
	 {
		pos = top + mid ;
		return pos ;
	 }
	  else
	 if( cres == dx_right_bigger)
	 {
		 bottom = bottom - mid ;
		 if ((((top + mid) == bottom)&&(mid == 1))||(mid == 0))
		 {

			 return -1 ;
		 }
	 }
	 else
	 {
		 top = top + mid ;
		 if ((((bottom + mid) == top)&&(mid == 1))||(mid == 0) )
		 {
		   return -1 ;
		 }
	 }

  }

  return -1 ;
}


DXLONG64 dx_list_sort_find_int_key_asc(PDX_LIST list , DXLONG64 int_key)
{
  if (list->count == 0) return -1 ;
  /*binary search for the key in the list*/
  DXLONG64 top	  = 0 ;
  DXLONG64 bottom = list->count -1 ;
  DXLONG   pos = -1   ;
  /*check first and last nodes*/
  PDXL_NODE    cnode = list->start   ;
  PDXL_OBJECT cobj	= cnode->object ;
  enum dx_compare_res cres = dx_int_compare(cobj->int_key,int_key) ;
  if (cres == dx_equal) return 0 ;

  cnode = list->end   ;
  cobj	= cnode->object ;
  cres = dx_int_compare(cobj->int_key,int_key) ;
  if (cres == dx_equal) return list->count-1 ;

  if (list->count <= 2) return -1 ;
  /*no luck! search for the key*/
  while(true)
  {
	 DXLONG64 mid = (bottom - top) / 2 ;
	 cnode = dx_list_go_to_pos(list,top + mid);
	 cobj = cnode->object ;

	 enum dx_compare_res cres = dx_int_compare(cobj->int_key,int_key) ;
     if( cres == dx_equal )
	 {
		pos = top + mid ;
		return pos ;
	 }
	  else
	 if( cres == dx_left_bigger)
	 {
		 bottom = bottom - mid ;
		 if ((((top + mid) == bottom)&&(mid == 1))||(mid == 0))
		 {
			 return -1 ;
		 }
	 }
	 else
	 {
		 top = top + mid ;
		 if ((((bottom + mid) == top)&&(mid == 1))||(mid == 0) )
		 {
		   return -1 ;
		 }
	 }

  }

  return -1 ;
}

DXLONG64 dx_list_sort_find_int_key_desc(PDX_LIST list , DXLONG64 int_key)
{
  if (list->count == 0) return -1 ;
  /*binary search for the key in the list*/
  DXLONG64 top	  = 0 ;
  DXLONG64 bottom = list->count -1 ;
  DXLONG   pos = -1   ;
  /*check first and last nodes*/
  PDXL_NODE    cnode = list->start   ;
  PDXL_OBJECT cobj	= cnode->object ;
  enum dx_compare_res cres = dx_int_compare(cobj->int_key,int_key) ;
  if (cres == dx_equal) return 0 ;

  cnode = list->end   ;
  cobj	= cnode->object ;
  cres = dx_int_compare(cobj->int_key,int_key) ;
  if (cres == dx_equal) return list->count-1 ;

  if (list->count <= 2) return -1 ;
  /*no luck! search for the key*/
  while(true)
  {
	 DXLONG64 mid = (bottom - top) / 2 ;
	 cnode = dx_list_go_to_pos(list,top + mid);
	 cobj = cnode->object ;

	 enum dx_compare_res cres = dx_int_compare(cobj->int_key,int_key) ;

     if( cres == dx_equal )
	 {
		pos = top + mid ;
		return pos ;
	 }
	  else
	 if( cres == dx_right_bigger)
	 {
		 bottom = bottom - mid ;
		 if ((((top + mid) == bottom)&&(mid == 1))||(mid == 0))
		 {

			 return -1 ;
		 }
	 }
	 else
	 {
		 top = top + mid ;
		 if ((((bottom + mid) == top)&&(mid == 1))||(mid == 0) )
		 {
		   return -1 ;
		 }
	 }

  }

  return -1 ;
}

DXLONG64 dx_list_sort_find_float_key_asc(PDX_LIST list , double float_key)
{
  if (list->count == 0) return -1 ;
  /*binary search for the key in the list*/
  DXLONG64 top	  = 0 ;
  DXLONG64 bottom = list->count -1 ;
  DXLONG   pos = -1   ;
  /*check first and last nodes*/
  PDXL_NODE    cnode = list->start   ;
  PDXL_OBJECT cobj	= cnode->object ;
  enum dx_compare_res cres = dx_float_compare(cobj->float_key,float_key) ;
  if (cres == dx_equal) return 0 ;

  cnode = list->end   ;
  cobj	= cnode->object ;
  cres = dx_float_compare(cobj->float_key,float_key) ;
  if (cres == dx_equal) return list->count-1 ;

  if (list->count <= 2) return -1 ;
  /*no luck! search for the key*/
  while(true)
  {
	 DXLONG64 mid = (bottom - top) / 2 ;
	 cnode = dx_list_go_to_pos(list,top + mid);
	 cobj = cnode->object ;

	 enum dx_compare_res cres = dx_float_compare(cobj->float_key,float_key) ;
     if( cres == dx_equal )
	 {
		pos = top + mid ;
		return pos ;
	 }
	  else
	 if( cres == dx_left_bigger)
	 {
		 bottom = bottom - mid ;
		 if ((((top + mid) == bottom)&&(mid == 1))||(mid == 0))
		 {

			 return -1 ;
		 }
	 }
	 else
	 {
		 top = top + mid ;
		 if ((((bottom + mid) == top)&&(mid == 1))||(mid == 0) )
		 {
		   return -1 ;
		 }
	 }

  }

  return -1 ;
}

DXLONG64 dx_list_sort_find_float_key_desc(PDX_LIST list , double float_key)
{
  if (list->count == 0) return -1 ;
  /*binary search for the key in the list*/
  DXLONG64 top	  = 0 ;
  DXLONG64 bottom = list->count -1 ;
  DXLONG   pos = -1   ;
  /*check first and last nodes*/
  PDXL_NODE    cnode = list->start   ;
  PDXL_OBJECT cobj	= cnode->object ;
  enum dx_compare_res cres = dx_float_compare(cobj->float_key,float_key) ;
  if (cres == dx_equal) return 0 ;

  cnode = list->end   ;
  cobj	= cnode->object ;
  cres = dx_float_compare(cobj->float_key,float_key) ;
  if (cres == dx_equal) return list->count-1 ;

  if (list->count <= 2) return -1 ;
  /*no luck! search for the key*/
  while(true)
  {
	 DXLONG64 mid = (bottom - top) / 2 ;
	 cnode = dx_list_go_to_pos(list,top + mid);
	 cobj = cnode->object ;

	 enum dx_compare_res cres = dx_float_compare(cobj->float_key,float_key) ;

     if( cres == dx_equal )
	 {
		pos = top + mid ;
		return pos ;
	 }
	  else
	 if( cres == dx_right_bigger)
	 {
		 bottom = bottom - mid ;
		 if ((((top + mid) == bottom)&&(mid == 1))||(mid == 0))
		 {

			 return -1 ;
		 }
	 }
	 else
	 {
		 top = top + mid ;
		 if ((((bottom + mid) == top)&&(mid == 1))||(mid == 0) )
		 {
		   return -1 ;
		 }
	 }

  }

  return -1 ;
}


/*************dx_list_add_node_direct_sort helper functions*/

void dx_check_and_add_node_asc(PDX_LIST list,PDXL_NODE node,enum dx_sort_key_type type)
{

 /*
   This function do all the logistics and insert a node in the right place
   If the list is sorted by the node->object->key in a asceding fashion.
 */

 DXLONG64 top     = 0 ;
 DXLONG64 bottom  = list->count - 1 ;
 DXLONG64 pos	  = 0 ;

 /*check first the first and last node*/

 //first node
 PDXL_NODE cnode  = list ->start;
 PDXL_OBJECT cobj = cnode->object ;
 PDXL_OBJECT obj  = node ->object  ;

 if ((cnode == NULL)||(node == NULL)) return ;

 enum dx_compare_res cres = dx_not_equal ;
 if (type == dx_str) cres = dx_string_lex_cmp(cobj->key,obj->key);
 else
   if (type == dx_int) cres = dx_int_compare(cobj->int_key,obj->int_key);
    else
		if (type == dx_float) cres = dx_float_compare(cobj->float_key,obj->float_key);


 if ((cres == dx_left_bigger)||(cres == dx_equal))
 {
	dx_list_insert_node(list,cnode,node);
	return ;
 }

 //last node
 cnode  = list ->end;
 cobj   = cnode->object ;
 obj    = node ->object  ;

 if ((cnode == NULL)||(node == NULL)) return ;

if (type == dx_str) cres = dx_string_lex_cmp(cobj->key,obj->key);
 else
   if (type == dx_int) cres = dx_int_compare(cobj->int_key,obj->int_key);
    else
		if (type == dx_float) cres = dx_float_compare(cobj->float_key,obj->float_key);

 if ((cres == dx_right_bigger)||(cres == dx_equal))
 {
	dx_list_add_node(list,node);
	return ;
 }

 /******** Binary search ***********/

 while(true)
 {
	 DXLONG64 mid = (bottom - top) / 2 ;

	 cnode = dx_list_go_to_pos(list,top + mid);
	 cobj = cnode->object ;
	 obj  = node->object  ;

	 enum dx_compare_res cres = dx_not_equal;
	 if (type == dx_str) cres = dx_string_lex_cmp(cobj->key,obj->key);
	 else
	  if (type == dx_int) cres = dx_int_compare(cobj->int_key,obj->int_key);
	   else
		if (type == dx_float) cres = dx_float_compare(cobj->float_key,obj->float_key);

     if( cres == dx_equal )
	 {
		pos = top + mid ;
		break ;
	 }
	  else
	 if( cres == dx_left_bigger)
	 {
		 bottom = bottom - mid ;
		 if ((((top + mid) == bottom)&&(mid == 1))||(mid == 0))
		 {
			 if (mid == 0) pos = top ;
			 else
			 pos = bottom ;
			 break ;
		 }
	 }
	 else
	 {
		 top = top + mid ;
		 if ((((bottom + mid) == top)&&(mid == 1))||(mid == 0) )
		 {
		   if (mid == 0) pos = bottom ;
		   else
		   pos = top ;
		   break ;
		 }
	 }

 }
  //insert the node and exit
  cnode = dx_list_go_to_pos(list,pos) ;
  if(cnode != NULL)
  dx_list_insert_node(list,cnode,node) ;
  else
	 dx_list_add_node(list,node);
	return ;
}


void dx_check_and_add_node_desc(PDX_LIST list,PDXL_NODE node, enum dx_sort_key_type type)
{

 /*
   This function do all the logistics and insert a node in the right place
   If the list is sorted by the node->object->key in a desceding fashion.
 */

 DXLONG64 top     = 0 ;
 DXLONG64 bottom  = list->count - 1 ;
 DXLONG64 pos	  = 0 ;

 /*check first the first and last node*/

 //first node
 PDXL_NODE cnode  = list ->start  ;
 PDXL_OBJECT cobj = cnode->object ;
 PDXL_OBJECT obj  = node ->object ;

 if ((cnode == NULL)||(node == NULL)) return ;

 enum dx_compare_res cres = dx_error;

 if (type == dx_str) cres = dx_string_lex_cmp(cobj->key,obj->key);
 else
   if (type == dx_int) cres = dx_int_compare(cobj->int_key,obj->int_key);
    else
		if (type == dx_float) cres = dx_float_compare(cobj->float_key,obj->float_key);

 if ((cres == dx_right_bigger)||(cres == dx_equal))
 {
	dx_list_insert_node(list,cnode,node);
	return ;
 }

 //last node
 cnode  = list ->end;
 cobj   = cnode->object ;
 obj    = node ->object  ;

 if ((cnode == NULL)||(node == NULL)) return ;

if (type == dx_str) cres = dx_string_lex_cmp(cobj->key,obj->key);
 else
   if (type == dx_int) cres = dx_int_compare(cobj->int_key,obj->int_key);
    else
		if (type == dx_float) cres = dx_float_compare(cobj->float_key,obj->float_key);

 if ((cres == dx_left_bigger)||(cres == dx_equal))
 {
	dx_list_add_node(list,node);
	return ;
 }

 /******** Binary search ***********/

 while(true)
 {
	 DXLONG64 mid = (bottom - top) / 2 ;
	// printf("bottom: %d ",bottom);
	// printf("top: %d ",top);
	// printf("mid: %d ",mid);
	 cnode = dx_list_go_to_pos(list,top + mid);
	 cobj = cnode->object ;
	 obj  = node->object  ;

	 enum dx_compare_res cres = dx_error;

if (type == dx_str) cres = dx_string_lex_cmp(cobj->key,obj->key);
 else
   if (type == dx_int) cres = dx_int_compare(cobj->int_key,obj->int_key);
    else
		if (type == dx_float) cres = dx_float_compare(cobj->float_key,obj->float_key);


     if( cres == dx_equal )
	 {
		pos = top + mid ;
		break ;
	 }
	  else
	 if( cres == dx_right_bigger)
	 {
		 bottom = bottom - mid ;
		 if ((((top + mid) == bottom)&&(mid == 1))||(mid == 0))
		 {
			 if (mid == 0) pos = top ;
			 else
			 pos = bottom ;
			 break ;
		 }
	 }
	 else
	 {
		 top = top + mid ;
		 if ((((bottom + mid) == top)&&(mid == 1))||(mid == 0) )
		 {
		   if (mid == 0) pos = bottom ;
		   else
		   pos = top ;
		   break ;
		 }
	 }

 }
  //insert the node and exit
  cnode = dx_list_go_to_pos(list,pos) ;
  if(cnode != NULL)
  dx_list_insert_node(list,cnode,node) ;
  else
	 dx_list_add_node(list,node);
	return ;
}


/*******************/

PDXL_NODE dx_list_add_node_direct_sort( PDX_LIST list , PDXL_OBJECT obj, bool asc,enum dx_sort_key_type type)
{
	/*
	 to find the right position for the string we will
	 "split" the list and we will check if the obj is bigger , smaller , or equal to the last element on the split,
	 and we will proceed until fount the position.
	*/
	if ( list == NULL ) return NULL ;
	if (list->count == 0)
	{
	 return dx_list_add_node_direct(list,obj);
	}

    PDXL_NODE node = dx_list_create_node(obj);
	if (asc == true)
    dx_check_and_add_node_asc(list,node,type);
    else
		dx_check_and_add_node_desc(list,node,type);

    //reset internal state 
	list->curr_indx = 0  ;
	list->curr_node = list->start ;
 

return node ;
}


/********** HASH_TABLE ***** ----------------------------------------------------------- */

//-----------------------------------------------------------------------------
// MurmurHash2, by Austin Appleby

// Note - This code makes a few assumptions about how your machine behaves -

// 1. We can read a 4-byte value from any address without crashing
// 2. sizeof(int) == 4

// And it has a few limitations -

// 1. It will not work incrementally.
// 2. It will not produce the same results on little-endian and big-endian
//    machines.

uint32_t MurmurHash2 ( const void * key, int32_t len, uint32_t seed )
{
	// 'm' and 'r' are mixing constants generated offline.
	// They're not really 'magic', they just happen to work well.

	const uint32_t m = 0x5bd1e995;
	const int32_t r = 24;

	// Initialize the hash to a 'random' value

	uint32_t h = seed ^ len;

	// Mix 4 bytes at a time into the hash

	const unsigned char * data = (const unsigned char *)key;

	while(len >= 4)
	{
		unsigned int k = *(unsigned int *)data;

		k *= m;
		k ^= k >> r;
		k *= m;

		h *= m;
		h ^= k;

		data += 4;
		len -= 4;
	}

	// Handle the last few bytes of the input array

	switch(len)
	{
	case 3: h ^= data[2] << 16;
	case 2: h ^= data[1] << 8;
	case 1: h ^= data[0];
	        h *= m;
	};

	// Do a few final mixes of the hash to ensure the last few
	// bytes are well-incorporated.

	h ^= h >> 13;
	h *= m;
	h ^= h >> 15;

	return h;
}

uint32_t dx_CreateHash(const void *key , int key_len , uint32_t limit_to)
{
  return MurmurHash2 ( key, key_len , HASH_KEY ) % limit_to ;
}

PDX_HASH_TABLE dx_HashCreateTable(uint32_t max_buckets)
{
	//create the hash as an array of PDXL_OBJECT
	PDX_HASH_TABLE table = (PDX_HASH_TABLE)malloc(sizeof(struct dx_hash_table)) ;
	if (table == NULL)
	{
		printf("Table was not created. Memory Allocation Error\n");
		return NULL ;
	}
	table->count = 0 ;
	//create the buckets
	table->buckets = (PDX_LIST*)malloc(sizeof(PDX_LIST)*max_buckets);
	if (table->buckets == NULL)
	{
		printf("Buckets was not created. Memory Allocation Error\n");
		free(table);
		return NULL ;
	}

	table->length  = max_buckets ;

	//create the dynamic lists
	for(uint32_t i = 0; i < table->length; i++)
	{
		table->buckets[i] = dx_list_create_list();
		if (table->buckets[i] == NULL)
		{
			printf("List in bucket was not created.Some memory may not have been released. Memory Allocation Error\n");
			free(table->buckets);
			free(table);
			return NULL ;
		}
	}
	//all buckets are created
	return table ;
}

PDX_HASH_TABLE dx_HashDestroyTable(PDX_HASH_TABLE table, hash_free_custom_object free_func)
{
	//destroy the table , NOT the object inside the PDXL_OBJECT buckets , these must be destroyed by the calling program
	//return always NULL
	if(table == NULL) return NULL ;
	for(uint32_t i = 0; i < table->length;i++)
	{
		
		PDXL_NODE node = table->buckets[i]->start ;
		while(node != NULL)
		{
			PDXL_OBJECT obj = node->object ;
			if(free_func != NULL)free_func(obj);
			if (obj->key != NULL) dx_string_free(obj->key); // free the obj key
			free(obj) ;
			node = node->right ;
		}
		
		dx_list_delete_list(table->buckets[i]);
		
	}

	//free the buckets (the pointers)
	free(table->buckets);
	//free the object
	free(table);
	return NULL ;
}

uint32_t dx_HashInsertObject(PDX_HASH_TABLE table , PDXL_OBJECT object)
{
	//add the object in the bucket of this key and return the key
	uint32_t key_len = object->key->bcount;
	void     *key    = NULL ;
    //pass the right data pointer to the function
	if (object->key->type == dx_wide)
    key = (void*)object->key->stringw ;
	 else
		key = (void*)object->key->stringa ;


	uint32_t hsh = dx_CreateHash( key , key_len , table->length ) ;
	//the hash number its ready, in actuality this is the table index! , go to the bucket !
	//add the object in the backet

	dx_list_add_node_direct(table->buckets[hsh] , object);
	table->count++;
	return hsh ;
}

bool dx_HashBucketEmpty(PDX_HASH_TABLE table , PDX_STRING str)
{
	//returns true if the bucket is empty
	//or false if there is even one item in it
	uint32_t key_len = str->bcount ;
	void *key ;

	if (str->type == dx_wide)
    key = (void*)str->stringw ;
	 else
		key = (void*)str->stringa ;

	uint32_t hsh 	 = dx_CreateHash( key , key_len , table->length ) ;
	if(table->buckets[hsh]->count > 0) return false;
	else
		return true ;
}


enum dx_compare_res dx_compare_utf8_binary(PDX_STRING str1 , PDX_STRING str2)
{
  if(str1->len != str2->len) return dx_not_equal ;
  if((str1 == NULL)||(str2 == NULL)) return dx_not_equal ;

  char *str1indx = str1->stringa ;
  char *str2indx = str2->stringa ;

  while( *str1indx != 0 )
  {
      if(*str1indx != *str2indx) return dx_not_equal ;
	  str1indx++;
	  str2indx++;
  }

  return dx_equal ;
}


PDXL_OBJECT dx_HashReturnItem(PDX_HASH_TABLE table , PDX_STRING str , bool compare_binary)
{
	/*
		returns the dxl_object that resides in this particular key
	*/

	uint32_t key_len = str->bcount ;
	void * key = NULL ;
	if (str->type == dx_wide)
    key = (void*)str->stringw ;
	 else
		key = (void*)str->stringa ;

	uint32_t hsh  = dx_CreateHash( key , key_len , table->length ) ;

	if (table->buckets[hsh]->count == 1)
	{
		//only one entry so return imediately
		PDXL_NODE node = table->buckets[hsh]->start ;
		if (node == NULL) return NULL ;
		PDXL_OBJECT obj = node->object ;
		/*do the compare*/
		
		if(compare_binary == true)
		{
		  if (dx_compare_utf8_binary(obj->key,str) == dx_equal) return obj;
		}
		else
			if (dx_string_compare(obj->key, str) == dx_equal) return obj;
		return NULL ; // no luck xD
	}

	//the list has more than one items , we will do a search
	PDXL_NODE node = table->buckets[hsh]->start ;
	while(node != NULL)
	{
		PDXL_OBJECT obj = node->object ;
		//check for the key
		if(compare_binary == true)
		{
		 if (dx_compare_utf8_binary(obj->key, str) == dx_equal) return obj;
		}
		else
			if (dx_string_compare(obj->key, str) == dx_equal) return obj;
		node = node->right ;
	}

	return NULL ; // no luck !
}


PDXL_OBJECT dx_HashRemoveItem(PDX_HASH_TABLE table , PDX_STRING str)
{
	uint32_t key_len = str->bcount ;
	void * key       = NULL ;
	if (str->type == dx_wide)
	key = (void*)str->stringw ;
	 else
		key = (void*)str->stringa ;

	uint32_t hsh  = dx_CreateHash( key , key_len , table->length ) ;

	if (table->buckets[hsh]->count == 1)
	{
		//only one entry so return imediately
		PDXL_NODE node = table->buckets[hsh]->start;
		if (node == NULL) return NULL ;
		PDXL_OBJECT obj = node->object ;
		/*do the compare*/
		if (dx_string_native_compare(obj->key,str) == dx_equal)
		{
		  dx_list_delete_node(node) ;
		  table->count--;
		  return (PDXL_OBJECT)obj ;
		}
		return NULL ; // no luck xD
	}

	//the list has more than one items , we will do a search
	PDXL_NODE node = table->buckets[hsh]->start ;
	while(node != NULL)
	{
		PDXL_OBJECT obj = node->object ;
		//check for the key
		if (dx_string_native_compare(obj->key,str) == dx_equal)
		{
		 dx_list_delete_node(node) ;
		 table->count-- ;
		 return (PDXL_OBJECT)obj ;
		}
		node = node->right ;
	}

	return NULL ; // no luck !
}

DXLONG64 dx_HashItemCount(PDX_HASH_TABLE table)
{
	return table->count;
}

PDXL_OBJECT dx_HashItemByIndex(PDX_HASH_TABLE table ,DXLONG64 indx)
{
	/*This is not very efficient but it does its job :D*/
	if((indx < 0)||(indx>table->count-1)) return NULL ;

	PDX_LIST *buckets = table->buckets ;
	DXLONG64 cindx = 0 ;
	for(DXLONG64 i = 0 ; i < table->length;i++)
	{
	 PDX_LIST bucket = buckets[i] ;
	 PDXL_NODE node = bucket->start ;
	 while(node!=NULL)
	 {
	     if (cindx == indx) return node->object ;
		 cindx++ ;
		 node = node->right ;
	 }
	
	}
	return NULL ; // no luck !
}


/*-------- STRING LIST IMPLEMENTATION ------------*/

PDX_STRINGLIST dx_stringlist_create()
{

    return dx_list_create_list();

}
void *dx_stringlist_free(PDX_STRINGLIST list)
{
    if (list == NULL) return NULL ;

    PDXL_NODE node = list->start ;
    while(node != NULL)
    {

        PDXL_OBJECT obj = node->object ;
        dx_string_free(obj->key) ;
        free(obj) ;
        node = node->right ;
    }

    dx_list_delete_list(list) ;

    return NULL ;
}

void dx_stringlist_add_raw_string(PDX_STRINGLIST list,char *str)
{
    if (list == NULL) return ;
    if (str == NULL)  return ;

    PDX_STRING dxstr =  dx_string_createU(NULL,str) ;

    PDXL_OBJECT obj = dxl_object_create();
    obj->key        = dxstr ;
    dx_list_add_node_direct(list,obj) ;

    return ;
}

void dx_stringlist_add_string(PDX_STRINGLIST list,PDX_STRING str)
{
    if (list == NULL) return ;
    if (str == NULL)  return ;

    PDXL_OBJECT obj = dxl_object_create();
    obj->key        = str ;
    dx_list_add_node_direct(list,obj) ;

    return ;

}

void dx_stringlist_remove_str(PDX_STRINGLIST list ,DXLONG64 indx)
{
    PDXL_NODE node = dx_list_go_to_pos(list,indx) ;
    if(node == NULL) return ;
    PDXL_OBJECT obj = node->object ;
    dx_string_free(obj->key);
    free(obj) ;
    dx_list_delete_node(node) ;
    return ;
}

void dx_stringlist_clear(PDX_STRINGLIST list)
{
    if (list == NULL) return ;
    PDXL_NODE node = list->start ;
    while(node != NULL)
    {
        PDXL_OBJECT obj = node->object ;
        dx_string_free(obj->key) ;
        free(obj);
        node = node->right ;
    }

    dx_list_empty_list(list);

}

PDX_STRING dx_stringlist_text(PDX_STRINGLIST list,bool unix_style)
{
  if(list == NULL) return NULL ;
  if(list->count == 0) return dx_string_createU(NULL,"") ; 
  PDXL_NODE node = list->start ;
  /*get the length of all the text*/
  DXLONG64 tsize = 0 ;
  while(node != NULL)
  {
    PDXL_OBJECT obj = node->object   ;
    tsize = tsize + obj->key->bcount    ;
    if(unix_style == true) tsize++;
    else
        tsize = tsize + 2 ;  // 2 bytes for win 1 for unix style (crlf or lf)

    node = node->right ;
  }

  tsize++ ; // for terminating zero

  /*fill a new buffer*/
  char *text = (char*)malloc(tsize);
  char *tindx = text ;
  char *cindx = NULL ;
  node = list->start ;
  while(node != NULL)
  {
    PDXL_OBJECT obj = node->object   ;
    cindx = obj->key->stringa        ;
    for(DXLONG64 i=0 ; i < obj->key->bcount;i++)
    {
      *tindx = *cindx ;
      tindx++ ;
      cindx++ ;
    }
    /*add the line feed*/

    if(unix_style == true)
    *tindx = '\n';
    else
    {
        *tindx = '\r';
        tindx++ ;
        *tindx = '\n';
        tindx++;
    }

    node = node->right ;
  }

  text[tsize-1] = 0 ; 

  PDX_STRING dxstr = dx_string_createU(NULL,"") ;
  
  /*trim the last \n or \r\n*/
  dxUtf8TrimRight(&text,"\r\n") ;
  dx_string_setU(dxstr,text)    ;

  return dxstr ;
}

PDX_STRING dx_stringlist_raw_text(PDX_STRINGLIST list)
{
  if(list == NULL) return NULL ;
  if(list->count == 0) return dx_string_createU(NULL,"") ; 
  PDXL_NODE node = list->start ;
  /*get the length of all the text*/
  DXLONG64 tsize = 0 ;
  while(node != NULL)
  {
    PDXL_OBJECT obj = node->object   ;
    tsize = tsize + obj->key->bcount    ;
    node = node->right ;
  }

  tsize++ ; // for terminating zero

  /*fill a new buffer*/
  char *text = (char*)malloc(tsize);
  char *tindx = text ;
  char *cindx = NULL ;
  node = list->start ;
  while(node != NULL)
  {
    PDXL_OBJECT obj = node->object   ;
    cindx = obj->key->stringa        ;
    for(DXLONG64 i=0 ; i < obj->key->bcount;i++)
    {
      *tindx = *cindx ;
      tindx++ ;
      cindx++ ;
    }

    node = node->right ;
  }

  text[tsize-1] = 0 ;

  PDX_STRING dxstr = dx_string_createU(NULL,"") ;
  dx_string_setU(dxstr,text) ;
  return dxstr ;

}


PDX_STRING dx_stringlist_string(PDX_STRINGLIST lst , DXLONG64 pos)
{
  PDXL_NODE node = dx_list_go_to_pos(lst,pos) ;
  return node->object->key ;
}

PDX_STRING dx_stringlist_find(PDX_STRINGLIST lst , PDX_STRING str)
{

    PDXL_NODE node = lst->start ;
    while(node != NULL)
    {
        PDXL_OBJECT obj = node->object ;
        if (dx_string_native_compare(obj->key,str) == dx_equal) return obj->key ;
        node = node->right ;
    }
    return NULL ;
}

PDX_STRINGLIST dx_stringlist_concat(PDX_STRINGLIST lst, PDX_STRINGLIST add_list)
{
	/*we have to account for concating its self */
	PDX_LIST temp_l = dx_list_create_list();


	PDXL_NODE node = add_list->start;
	while (node != NULL)
	{
		PDXL_OBJECT obj = node->object;
		/*create a new dx_string and copy the string*/
		PDX_STRING str = dx_string_createU(NULL, obj->key->stringa) ;
		PDXL_OBJECT tobj = dxl_object_create()   ;
		tobj->key = str;
		dx_list_add_node_direct(temp_l, tobj);
		node = node->right;
	}

	/*now concat*/
	node = temp_l->start;
	while(node != NULL)
	{
		PDXL_OBJECT obj = node->object;
		dx_stringlist_add_string(lst, obj->key) ;
		free(obj); /*free this is not need it anymore*/
		node = node->right;
	}
	
	/*
	 free the memory of the temporary list 
	*/
	dx_list_delete_list(temp_l) ;/*release the memory for the nodes and the list itself */

	return lst;
}


DXLONG64 dx_stringlist_strlen(PDX_STRINGLIST list)
{
	DXLONG64 lencnt = 0 ;
	if(list == NULL) return -1 ;
	PDXL_NODE node = list->start ;
	while(node != NULL)
	{
		PDXL_OBJECT obj = node->object ;
		lencnt = lencnt + obj->key->len ;
		node = node->right ;
	}

	return lencnt ;
}

DXLONG64 dx_stringlist_bytelen(PDX_STRINGLIST list)
{
	DXLONG64 bytecnt = 0 ;
	if(list == NULL) return -1 ;
	PDXL_NODE node = list->start ;
	while(node != NULL)
	{
		PDXL_OBJECT obj = node->object       ;
		bytecnt = bytecnt + obj->key->bcount ;
		node = node->right ;
	}

	return bytecnt ;
}

PDX_STRINGLIST dx_stringlist_load_text_ex(PDX_STRINGLIST list , PDX_STRING str , int break_every)
{
	 if(list == NULL) return NULL ;
	 if(str == NULL) return NULL  ;
	 if(break_every < 1 ) return NULL ;

	 char * buff = (char*)malloc(break_every+1); /*the larger line in bytes*/
	 if(buff == NULL) return NULL ;
	 char * buff_indx = buff ;
	 *buff_indx = 0 ;
	 char *string = str->stringa ;
	 char *curr_char ;
	 int bytes_cnt = 0 ;
	 while(*string != 0)
	 {
       DXCHAR uchar = dx_get_utf8_char_ex(&string,&curr_char) ;
	   /*get the bytes of the utf8 char*/
	   int csize = dx_utf8_char_byte_count(*curr_char) ;
	   /*check if the character fits in the buffer*/
	   if((bytes_cnt+csize) <= break_every)
	   {
		   for(int i = 0;i<csize;i++)
		   {
			   *buff_indx = *curr_char ;
			   buff_indx++;
			   curr_char++;
		   }
		   bytes_cnt = bytes_cnt+csize ;
		   continue ;/*the character was inserted , proceed to the next one*/
	   }
	   else
	   {
		/*the character does not fit the buffer , save the accumulated buffer*/
	    *buff_indx = 0 ;
		dx_stringlist_add_raw_string(list,buff);
		bytes_cnt = 0 ;
		buff_indx = buff ;
		*buff_indx = 0   ;
		/*reset the last character as we did not handle it*/
		string = curr_char ;
		continue ;
	   }

	   if(bytes_cnt == break_every)
	   {
		 *buff_indx=0 ;/*terminate and insert*/
		 dx_stringlist_add_raw_string(list,buff);
		 buff_indx = buff ; /*reset buffer*/
		 *buff_indx = 0   ;
		 bytes_cnt  = 0   ;
	   }

	 }
	 /*add the remainder*/
	 if(bytes_cnt !=0)
	 {
	     *buff_indx=0 ;/*terminate and insert*/
		 dx_stringlist_add_raw_string(list,buff);
	 }

	 free(buff);

	 return list ;

}


PDX_STRINGLIST dx_stringlist_remove_char(PDX_STRINGLIST list,char *utf8char)
{
	if((list == NULL)||(utf8char==NULL)||(*utf8char==0) ) return NULL ;
	/*a character exists only in one line and cannpot be split in a second line , this makes our life easier!*/
	PDXL_NODE node = list->start ;
	while(node != NULL)
	{
	 PDX_STRING str = node->object->key ;
	 dx_utf8_remove_char(str,utf8char) ;
	 node = node->right ;
	}

	return list ;
}

PDX_STRINGLIST dx_stringlist_replace_char(PDX_STRINGLIST list , char *utf8char , char *replacewith)
{
	if((list == NULL)||(utf8char==NULL)||(*utf8char==0) ) return NULL ;
	/*a character exists only in one line and cannpot be split in a second line , this makes our life easier!*/
	PDXL_NODE node = list->start ;
	while(node != NULL)
	{
	 PDX_STRING str = node->object->key ;
	 dx_utf8_replace_char(str,utf8char, replacewith) ;
	 node = node->right ;
	}

	return list ;
}


DXLONG64 dx_string_list_h_find_word(PDX_STRING line,char *word,DXLONG64 from_indx , char **word_indx)
{
  /*ok this is tricky. We will search for the word in the string as usuall,
   if we found the word , we will return the index (in the line) of the first character.
   The word_indx will be invalid.
   If we do not found the word and there is not any partial word pending then the return value will be -1.
   If we do not found the word but there is a partial word pending  , then we return the index of the first 
   character of the partial word , and the word_indx is different of the word (word_indx != word)
  */


	if(line == NULL) return -1  ;
	if(word == NULL) return -1  ;

	if((from_indx < 0 )||(from_indx>=line->len)) return -1 ;
	
	/*go to the rignt index*/
	char *str_indx  = line->stringa ;
	for(DXLONG64 i = 0 ; i < from_indx ;i++ ) dx_get_utf8_char(&str_indx) ;

	DXLONG64 indx	    = 0    ;
	DXLONG64 first_char = -1   ; 
	char *cur_char = NULL      ;
	int char_len   = 0		   ;
	while(*str_indx != 0)
	{
		DXCHAR word_char = dx_get_utf8_char(word_indx) ;
		/*check if the word has reached its end*/
		if(word_char == 0)
		{
		 /*found it*/
		  return first_char ;
		}

		DXCHAR base_char = dx_get_utf8_char_ex2(&str_indx,&cur_char,&char_len) ;
		/*check if the character is valid*/
		if(base_char != word_char)
		{
			if(first_char != -1) first_char = -1;
			*word_indx = word ;/*reset*/
		}
		else
		{
			/*the character is in the right order for the word*/
			if(first_char == -1) first_char = indx + from_indx ;
			/*the next character is being in line by the dx_get_utf8_char*/
		}

		indx++;
	}

	/*check if there is a partial word in place*/
	if(*word_indx != word )
	{
	 return first_char ;
	}
	return -1     ;

}

DXLONG64 dx_stringlist_find_word(PDX_STRINGLIST list,char *word,DXLONG64 from_indx,DXLONG64 *line,DXLONG64 *line_ch_indx) 
{
	/*
	 There is a problem with finding a word in the list. As the list 
	 has "break" the string in multiple lines , its very possible that a word is wraped in more that one line.
	*/
	if(list == NULL) return -1 ;
	if(from_indx < 0 ) return -1 ;
	char* word_indx = word ;

	/*find the relative index form the absolute index*/
	DXLONG64  relative_indx = -1 ;
	DXLONG64  abs_indx		= 0 ;
	PDXL_NODE node = list->start ;
	while(node!=NULL)
	{
	  PDXL_OBJECT obj = node->object   ;
	  PDX_STRING str  = obj->key	   ;
	  abs_indx = abs_indx + str->len;
	
	  if(abs_indx >= from_indx)
	  {
		  /*fine tuning*/
		  if (abs_indx == from_indx) {relative_indx = str->len;break;}
		  /*
		    if the abs_indx is bigger than from indx we will return the remainder from the substraction
		    as this is the index in the current line
		  */
		  relative_indx = str->len - (abs_indx - from_indx) ;
		  abs_indx = abs_indx - str->len ; /*the index in the current line will be added later so we have to subtrack the line length*/
		  break ;
	  }

	  node = node->right ;
	}

	if(node == NULL) return -1 ; /*the index is bigger than the string length*/
	DXLONG64 line_num = 0 ;

	while(node!=NULL)
	{
		PDXL_OBJECT obj = node->object ;
		PDX_STRING str  = obj->key	   ;
		DXLONG64 ret_indx = dx_string_list_h_find_word(str,word,relative_indx ,&word_indx) ;
		/*check if we found the word*/
		if((ret_indx!=-1)&&(*word_indx == 0))
		{
		 *line = line_num ;
		 *line_ch_indx = ret_indx ;
		 return abs_indx + ret_indx ;
		}

		if((ret_indx!=-1)&&(*word_indx != 0 ))
		{
			/*we do not find the word but we have part of the word*/
			abs_indx = abs_indx + ret_indx ;  
		}
		else 
			abs_indx = abs_indx + str->len ;

			line_num++;
			relative_indx = 0 ;/*next line will start from the start*/
		
		node = node->right ;
	}

	return -1 ;
}

DXLONG64 dx_string_list_h_find_word_binary(PDX_STRING line,char *word,DXLONG64 from_indx , char **word_indx)
{
  /*ok this is tricky. We will search for the word in the string as usuall,
   if we found the word , we will return the index (in the line) of the first byte of the word.
   The word_indx will be invalid.
   If we do not found the word and there is not any partial word pending then the return value will be -1.
   If we do not found the word but there is a partial word pending  , then we return the index of the first 
   byte of the partial word , and the word_indx is different of the word (word_indx != word)
  */


	if(line == NULL) return -1  ;
	if(word == NULL) return -1  ;

	if((from_indx < 0 )||(from_indx>=line->bcount)) return -1 ;
	
	/*go to the rignt index*/
	char *str_indx  = &(line->stringa[from_indx]) ;
	

	DXLONG64 indx	    = 0    ;
	DXLONG64 first_char = -1   ; 

	while(*str_indx != 0)
	{
		/*check if the word has reached its end*/
		if(*(*word_indx)==0)
		{
		 /*found it*/
		  return first_char ;
		}

		
		/*check if the character is valid*/
		if(*str_indx != *(*word_indx))
		{
			if(first_char != -1) first_char = -1;
			*word_indx = word ;/*reset*/
			str_indx++ ;
			indx++ ;
			continue ;
		}
		else
		{
			/*the character is in the right order for the word*/
			if(first_char == -1) first_char = indx + from_indx ;
			/*the next character is being in line by the dx_get_utf8_char*/
		}

		indx++;
		(*word_indx)++ ;
		str_indx++ ;
	}

	/*check if there is a partial word in place*/
	if((*word_indx) != word )
	{
	 return first_char ;
	}
	return -1     ;
}


DXLONG64 dx_stringlist_find_word_binary(PDX_STRINGLIST list,char *word,DXLONG64 from_indx,DXLONG64 *line,DXLONG64 *line_ch_indx)
{
	/*
	 There is a problem with finding a word in the list. As the list 
	 has "break" the string in multiple lines , its very possible that a word is wraped in more that one line.
	*/
	if(list == NULL) return -1 ;
	if(from_indx < 0 ) return -1 ;
	char* word_indx = word ;

	/*find the relative index form the absolute index*/
	DXLONG64  relative_indx = -1 ;
	DXLONG64  abs_indx		= from_indx / 512 ;
	PDXL_NODE node = list->start ;
	while(node!=NULL)
	{
	  PDXL_OBJECT obj = node->object   ;
	  PDX_STRING str  = obj->key	   ;
	  abs_indx = abs_indx + str->bcount   ;
	
	  if(abs_indx >= from_indx)
	  {
		  /*fine tuning*/
		  if (abs_indx == from_indx) {relative_indx = str->bcount;break;}
		  /*
		    if the abs_indx is bigger than from indx we will return the remainder from the substraction
		    as this is the index in the current line
		  */
		  relative_indx = str->bcount - (abs_indx - from_indx) ;
		  abs_indx = abs_indx - str->bcount ; /*the index in the current line will be added later so we have to subtrack the line length*/
		  break ;
	  }

	  node = node->right ;
	}

	if(node == NULL) return -1 ; /*the index is bigger than the string length*/
	DXLONG64 line_num = 0 ;

	while(node!=NULL)
	{
		PDXL_OBJECT obj = node->object ;
		PDX_STRING str  = obj->key	   ;
		DXLONG64 ret_indx = dx_string_list_h_find_word_binary(str,word,relative_indx ,&word_indx) ;
		/*check if we found the word*/
		if((ret_indx!=-1)&&(*word_indx == 0))
		{
		 *line = line_num ;
		 *line_ch_indx = ret_indx ;
		 return abs_indx + ret_indx ;
		}

		if((ret_indx!=-1)&&(*word_indx != 0 ))
		{
			/*we do not find the word but we have part of the word*/
			abs_indx = abs_indx + ret_indx ;  
		}
		else 
			abs_indx = abs_indx + str->len ;

			line_num++;
			relative_indx = 0 ;/*next line will start from the start*/

		node = node->right ;
	}

	return -1 ;
}


/********************** SPECIAL STRING FUNCTIONS **************************/


DXLONG64 dx_utf8_search_h_word(char **str_indx,char *word,char **word_start )
{
	/*
	 fast searching for a word in a utf8 string
	 returns the character position in the string and the word_start will be set to the first byte of the word in the string.
	 If the function does not find the word returns -1 and the word_start is invalid
	*/
	if(str_indx == NULL)  return -1 ;
	if(word == NULL)	  return -1 ;

	char *word_indx     = word  ;
	DXLONG64 indx	    = 0  ;
	DXLONG64 first_char = -1 ; 
	char *cur_char = NULL    ;
	int char_len   = 0		 ;
	*word_start = NULL       ;
	while(*(*str_indx) != 0)
	{
		DXCHAR word_char = dx_get_utf8_char(&word_indx) ;
		/*check if the character is valid*/
		/*check if the word has reached its end*/
		if(word_char == 0)
		{
		 /*found it*/
		  return first_char ;
		}

		DXCHAR base_char = dx_get_utf8_char_ex2(str_indx,&cur_char,&char_len) ;

		if(base_char != word_char)
		{
			if(first_char != -1) first_char = -1;
			word_indx = word ;/*reset*/
		}
		else
		{
			if(first_char == -1) 
			{
				first_char = indx ;
				*word_start = cur_char ; 
			}
			/*the next character is being in line by the dx_get_utf8_char*/
		}

		indx++;
	}

	DXCHAR word_char = dx_get_utf8_char(&word_indx) ;
	/*check if we found the character in the end of the string*/
	if((word_char == 0)&&(first_char!=-1))
	{
		/*found it*/
		return first_char ;
	}

	return -1     ;
}

DXLONG64 dx_utf8_replace_word(PDX_STRING source,DXLONG64 from_index,PDX_STRING word,PDX_STRING replace_with,bool replace_all)
{
	/*
	 This function is complex. For efficency it tries to create a list of the words for replacing in the string,
	 it creates a string list with the strings and the replaced words , and then constructs and returns a 
	 PDX_STRING 
	*/
	
	if( word->len == 0 ) return -1 ;
	if((from_index < 0)||(from_index > source->len) ) return -1 ;
	char *str_indx     = source->stringa ;
	char *word_indx	   = word->stringa   ; 
	char *word_start   = NULL			 ;

	/*go to the rignt index*/
	for(DXLONG64 i = 0 ; i < from_index ;i++ ) dx_get_utf8_char(&str_indx) ;
	if(str_indx == 0 ) return - 1  ;

	/*create the list with the position of the words for replacing*/
	char * cur_indx = str_indx ;
	PDX_LIST replace_words = dx_list_create_list();
	DXLONG   last_word_start = 0 ;
	while(true)
	{
	
      DXLONG64 indx = dx_utf8_search_h_word(&str_indx , word_indx , &word_start ) ;
	  if(indx == -1) break ;
	  PDXL_OBJECT obj = dxl_object_create() ;
	  obj->obj     = (void*)word_start  ; /*the first byte*/
	  last_word_start =  last_word_start + word->len + indx			 	; /*the position of the first character*/
	  dx_list_add_node_direct(replace_words,obj) ;
	  if(replace_all == false) break    ;
	
	}
	last_word_start = last_word_start - word->len ;/*set the indexx as the first character of the last word to replace*/
	if(replace_words->count == 0) 
	{
		dx_list_delete_list(replace_words) ;
		return -1 ; /*no word was found*/
	}
	/*create a special list with the info about the strings to be copied*/
	PDX_LIST strlist = dx_list_create_list() ;
	DXLONG64 buf_len = 0 ;
	PDXL_NODE node = replace_words->start ;
	while(node != NULL) /*for all the words to be replaced*/
	{
	    PDXL_OBJECT obj = node->object ;

		PDXL_OBJECT nobj = dxl_object_create() ;
		
		nobj->obj     = (void*)cur_indx ; /*the index for the start for the substring to copy */
		nobj->int_key = (((char*)obj->obj) - cur_indx); /*the bytes that we need to copy until the word to replace*/
		buf_len = buf_len + (nobj->int_key) ; /*create the right buffer length*/
		nobj->flags   = 0				    ; /*its a substring from the source string*/

		if (nobj->int_key != 0) 
		{
		 cur_indx = (char*)obj->obj + word->bcount ;/*set the current indx to the byte after the old word to ommit it*/ 
		 dx_list_add_node_direct(strlist,nobj) ;
		}
		else
		{
		  cur_indx = (char*)obj->obj + word->bcount ;/*set the current indx to the byte after the old word to ommit it*/ 
		  free(nobj) ;
		  nobj = NULL ;
		}

		if(replace_with->len > 0 )
		{
		  /*check if the word is the very first one!*/
		  if(nobj != NULL)
		  {
		   if (nobj->int_key == 0)  cur_indx = (char*)obj->obj + word->bcount ;/*set the current indx to the byte after the old word to ommit it*/ 
		  }
		   else 
			  cur_indx = (char*)obj->obj + word->bcount ;

		  nobj = dxl_object_create() ;
		  nobj->flags = 1 ;
		  buf_len = buf_len + replace_with->bcount ;
		  dx_list_add_node_direct(strlist,nobj) ;
		}
		node = node->right ;
	}

	/*add the remainder string*/
	buf_len = buf_len +(&(source->stringa[source->bcount-1]) - cur_indx)+1 ;
	/*create a new string with the replaced words*/
	char * new_string     = (char*)malloc(buf_len + 1) ;
	new_string[buf_len]   = 0 ; /*set the terminating zero*/
	char *new_string_indx = new_string ;

	node = strlist->start ;
	while(node != NULL)/*for every substring*/
	{
	  PDXL_OBJECT obj = node->object ;
	  if(obj->flags == 0) /*this is a source substring*/
	  {
		  char *srcbt = (char*)obj->obj ; 
		  for(DXLONG64 i = 0;i<obj->int_key;i++) /*for all the bytes*/
		  {
			*new_string_indx = *srcbt ;	
			srcbt++ ;/*next source byte*/
			new_string_indx++   ;
		  }
	  }
	  else
	  { /*this is a replacement word*/
	      for(DXLONG64 i = 0;i<replace_with->bcount;i++)
		  {
			*new_string_indx = replace_with->stringa[i] ;
			new_string_indx++ ;
		  }
	  }

	  node = node->right ;
	}

	/*add the remainder*/
	while(*cur_indx != 0)
	{
		*new_string_indx = *cur_indx ;
		new_string_indx++;
		cur_indx++ ;
	}
	/*alter the string*/
	dx_string_setU(source,new_string) ;

	node = replace_words->start ;
	while(node != NULL)
	{
	    free(node->object) ;
		node = node->right ;
	}
	dx_list_delete_list(replace_words) ;

    node = strlist->start ;
	while(node != NULL)
	{
	    free(node->object) ;
		node = node->right ;
	}
	dx_list_delete_list(strlist) ;

	return last_word_start ;
}


/*replace in binary*/

DXLONG64 dx_binary_search_h_word(char **str_indx,char *word,char **word_start )
{
	/*
	 fast searching for a word in a string
	 returns the first byte of the character position in the string and the word_start will be set to the first byte of the word in the string.
	 If the function does not find the word returns -1 and the word_start is invalid
	*/
	if(str_indx == NULL)  return -1 ;
	if(word == NULL)	  return -1 ;

	char *word_indx     = word  ;
	DXLONG64 indx	    = 0  ;
	DXLONG64 first_byte = -1 ; 
	*word_start = NULL       ;

	while(*(*str_indx) != 0)
	{
		/*check if the character is valid*/

		if(*(*str_indx) != *word_indx)
		{
			if(first_byte != -1) first_byte = -1;
			word_indx = word ;/*reset*/
		}
		else
		{
			if(first_byte == -1) 
			{
				first_byte = indx  ;
				*word_start = *str_indx ; 
			}
			word_indx++;
		}

		indx++;
		(*str_indx)++ ;

		
		/*check if the word has reached its end*/
		if(*word_indx == 0)
		{
		 /*found it*/
		  return first_byte ;
		}
	}

	return -1     ;
}

DXLONG64 dx_replace_word_binary(PDX_STRING source,DXLONG64 from_index,PDX_STRING word,PDX_STRING replace_with,bool replace_all)
{
	/*
	 This function is complex. For efficency it tries to create a list of the words for replacing in the string,
	 it creates a string list with the strings and the replaced words , and then constructs and returns a 
	 PDX_STRING 
	*/
	
	if((from_index < 0 )||(from_index > source->bcount) ) return -1 ;

	if( word->len == 0 ) return -1 ;

	char *str_indx     = &(source->stringa[from_index]) ;
	char *word_indx	   = word->stringa   ; 
	char *word_start   = NULL			 ;

	if(str_indx == 0 ) return - 1  ;

	/*create the list with the position of the words for replacing*/
	char * cur_indx = str_indx ;
	PDX_LIST replace_words = dx_list_create_list();
	DXLONG   last_word_start = 0 ;
	while(true)
	{
	
      DXLONG64 indx = dx_binary_search_h_word(&str_indx , word_indx , &word_start ) ;
	  if(indx == -1) break ;
	  PDXL_OBJECT obj = dxl_object_create() ;
	  obj->obj     = (void*)word_start  ; /*the first byte*/
	  last_word_start =  last_word_start + word->len + indx			 	; /*the position of the first character*/
	  dx_list_add_node_direct(replace_words,obj) ;
	  if(replace_all == false) break    ;
	
	}
	last_word_start = last_word_start - word->len ;/*set the index as the first character of the last word to replace*/
	if(replace_words->count == 0) 
	{
		dx_list_delete_list(replace_words) ;
		return -1 ; /*no word was found*/
	}
	/*create a special list with the info about the strings to be copied*/
	PDX_LIST strlist = dx_list_create_list() ;
	DXLONG64 buf_len = 0 ;
	PDXL_NODE node = replace_words->start ;
	while(node != NULL) /*for all the words to be replaced*/
	{
	    PDXL_OBJECT obj = node->object ;

		PDXL_OBJECT nobj =dxl_object_create() ;
		
		nobj->obj     = (void*)cur_indx ; /*the index for the start for the substring to copy */
		nobj->int_key = (((char*)obj->obj) - cur_indx); /*the bytes that we need to copy until the word to be replaced*/
		buf_len = buf_len + (nobj->int_key) ; /*create the right buffer length*/
		nobj->flags   = 0				  ; /*its a substring from the source string*/

		if (nobj->int_key != 0) 
		{
		 cur_indx = (char*)obj->obj + word->bcount ;/*set the current indx to the byte after the old word to ommit it*/ 
		 dx_list_add_node_direct(strlist,nobj) ;
		}
		else 
		{
			cur_indx = (char*)obj->obj + word->bcount ;/*set the current indx to the byte after the old word to ommit it*/ 
			free(nobj) ;
			nobj = NULL ;
		}
		if(replace_with->len > 0 )
		{
          /*check if the word is the very first one!*/
          if (nobj!= NULL)
		  {
		    if (nobj->int_key == 0)  cur_indx = (char*)obj->obj + word->bcount ;/*set the current indx to the byte after the old word to ommit it*/ 
		  } 
		  else 
			  cur_indx = (char*)obj->obj + word->bcount ;

		  nobj =dxl_object_create();
		  nobj->flags = 1 ;
		  buf_len = buf_len + replace_with->bcount ;
		  dx_list_add_node_direct(strlist,nobj) ;
		}
		node = node->right ;
	}

	/*add the remainder string*/
	buf_len = buf_len +(&(source->stringa[source->bcount-1]) - cur_indx)+1 ;
	/*create a new string with the replaced words*/
	char * new_string     = (char*)malloc(buf_len + 1) ;
	new_string[buf_len]   = 0 ; /*set the terminating zero*/
	char *new_string_indx = new_string ;

	node = strlist->start ;
	while(node != NULL)/*for every substring*/
	{
	  PDXL_OBJECT obj = node->object ;
	  if(obj->flags == 0) /*this is a source substring*/
	  {
		  char *srcbt = (char*)obj->obj ;
		  for(DXLONG64 i = 0;i<obj->int_key;i++) /*for all the bytes*/
		  { 
			*new_string_indx = *srcbt ;
			srcbt++ ;/*next source byte*/
			new_string_indx++   ;
		  }
	  }
	  else
	  { /*this is a replacement word*/
	      for(DXLONG64 i = 0;i<replace_with->bcount;i++)
		  {
			*new_string_indx = replace_with->stringa[i] ;
			new_string_indx++ ;
		  }
	  }

	  node = node->right ;
	}

	/*add the remainder*/
	while(*cur_indx != 0)
	{
		*new_string_indx = *cur_indx ;
		new_string_indx++;
		cur_indx++ ;
	}
	/*alter the string*/
	dx_string_setU(source,new_string) ;

	node = replace_words->start ;
	while(node != NULL)
	{
	    free(node->object) ;
		node = node->right ;
	}
	dx_list_delete_list(replace_words) ;

    node = strlist->start ;
	while(node != NULL)
	{
	    free(node->object) ;
		node = node->right ;
	}
	dx_list_delete_list(strlist) ;

	return last_word_start ;
}




DXLONG64 dx_unicode_search_h_word(DXCHAR **str_indx,DXCHAR *word,DXCHAR **word_start )
{
	/*
	 fast searching for a word in a string
	 returns the first byte of the character position in the string and the word_start will be set to the first byte of the word in the string.
	 If the function does not find the word returns -1 and the word_start is invalid
	*/
	if(str_indx == NULL)  return -1 ;
	if(word == NULL)	  return -1 ;

	DXCHAR *word_indx     = word  ;
	DXLONG64 indx	    = 0  ;
	DXLONG64 first_char = -1 ; 
	DXCHAR *cur_char = NULL    ;
	int char_len   = 0		 ;
	*word_start = NULL       ;

	while(*(*str_indx) != 0)
	{
		/*check if the character is valid*/

		if(*(*str_indx) != *word_indx)
		{
			if(first_char != -1) first_char = -1;
			word_indx = word ;/*reset*/
		}
		else
		{
			if(first_char == -1) 
			{
				first_char = indx  ;
				*word_start = *str_indx ; 
			}
			word_indx++;
		}

		indx++;
		(*str_indx)++ ;

		
		/*check if the word has reached its end*/
		if(*word_indx == 0)
		{
		 /*found it*/
		  return first_char ;
		}
	}

	return -1     ;
}

DXLONG64 dx_replace_word_unicode(PDX_STRING source,DXLONG64 from_index,PDX_STRING word,PDX_STRING replace_with,bool replace_all)
{
	/*
	 This function is complex. For efficency it tries to create a list of the words for replacing in the string,
	 it creates a string list with the strings and the replaced words , and then constructs and returns a 
	 PDX_STRING 
	*/
	
	if((from_index < 0 )||(from_index > source->len) ) return -1 ;

	if( word->len == 0 ) return -1 ;

	DXCHAR *str_indx     = &(source->stringx[from_index]) ;
	DXCHAR *word_indx	  = word->stringx   ; 
	DXCHAR *word_start   = NULL			    ;

	if(str_indx == 0 ) return - 1  ;

	/*create the list with the position of the words for replacing*/
	DXCHAR * cur_indx = str_indx ;
	PDX_LIST replace_words = dx_list_create_list();
	DXLONG   last_word_start = 0 ;
	while(true)
	{
	
      DXLONG64 indx = dx_unicode_search_h_word(&str_indx , word_indx , &word_start ) ;
	  if(indx == -1) break ;
	  PDXL_OBJECT obj =dxl_object_create() ;
	  obj->obj        = (void*)word_start  ; /*the first byte*/
	  last_word_start =  last_word_start + word->len + indx			 	; /*the position of the first character*/
	  dx_list_add_node_direct(replace_words,obj) ;
	  if(replace_all == false) break    ;
	
	}

	last_word_start = last_word_start - word->len ;/*set the index as the first character of the last word to replace*/
	if(replace_words->count == 0) return -1 ; /*no word was found*/
	
	/*create a special list with the info about the strings to be copied*/
	PDX_LIST strlist = dx_list_create_list() ;
	DXLONG64 buf_len = 0 ;
	PDXL_NODE node = replace_words->start ;
	while(node != NULL) /*for all the words to be replaced*/
	{
	    PDXL_OBJECT obj = node->object ;

		PDXL_OBJECT nobj = dxl_object_create() ;
		
		nobj->obj     = (void*)cur_indx ; /*the index for the start for the substring to copy */
		nobj->int_key = (((DXCHAR*)obj->obj) - cur_indx); /*the characters that we need to copy until the word to replace*/
		buf_len		  = buf_len + (nobj->int_key) ; /*create the right buffer length*/
		nobj->flags   = 0				  ; /*its a substring from the source string*/

		if (nobj->int_key == 0) break ;
		cur_indx = (DXCHAR*)obj->obj + word->len ;/*set the current indx to the byte after the old word to ommit it*/ 

		dx_list_add_node_direct(strlist,nobj) ;
		
		if(replace_with->len > 0 )
		{
		  nobj = dxl_object_create() ;
		  nobj->flags = 1 ;
		  buf_len = buf_len + replace_with->len ;
		  dx_list_add_node_direct(strlist,nobj) ;
		}
		node = node->right ;
	}

	/*add the remainder string*/
	buf_len = buf_len +(&(source->stringx[source->len-1]) - cur_indx)+1 ;
	/*create a new string with the replaced words*/
	DXCHAR * new_string     = (DXCHAR*)malloc((buf_len + 1)*sizeof(DXCHAR)) ;
	new_string[buf_len]   = 0 ; /*set the terminating zero*/
	DXCHAR *new_string_indx = new_string ;

	node = strlist->start ;
	while(node != NULL)/*for every substring*/
	{
	  PDXL_OBJECT obj = node->object ;
	  if(obj->flags == 0) /*this is a source substring*/
	  {
		  for(DXLONG64 i = 0;i<obj->int_key;i++) /*for all the bytes*/
		  {
			*new_string_indx = *((DXCHAR*)obj->obj) ;
			DXCHAR *srcbt = (DXCHAR*)obj->obj ; 
			srcbt++ ;/*next source byte*/
			new_string_indx++   ;
		  }
	  }
	  else
	  { /*this is a replacement word*/
	      for(DXLONG64 i = 0;i<replace_with->len;i++)
		  {
			*new_string_indx = replace_with->stringx[i] ;
			new_string_indx++ ;
		  }
	  }

	  node = node->right ;
	}

	/*add the remainder*/
	while(*cur_indx != 0)
	{
		*new_string_indx = *cur_indx ;
		new_string_indx++;
		cur_indx++ ;
	}
	/*alter the string*/
	dx_string_setX(source,new_string) ;

	node = replace_words->start ;
	while(node != NULL)
	{
	    free(node->object) ;
		node = node->right ;
	}
	dx_list_delete_list(replace_words) ;

    node = strlist->start ;
	while(node != NULL)
	{
	    free(node->object) ;
		node = node->right ;
	}
	dx_list_delete_list(strlist) ;

	return last_word_start ;
}





/*************************************************************************/




/*-----------------------------------------------*/

















