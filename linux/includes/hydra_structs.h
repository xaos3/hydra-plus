#ifndef DXLISTS
#include "dxlists.h"
#endif

#include <cthreads/cthreads.h>
/*the support for the SSL is via the wolfSSL third party library*/
#include <wolfssl/ssl.h>

/*include the memory man*/
#include "hydra_mman.h"
/***********************/

PHDR_POINTERS        _POINTERS  = NULL ;
PHDR_POINTERS_LOGGER _LOGGER    = NULL ;
bool                 _ON_MEMORY_DETECT ;
#define HYDRA_STRUCTS

/*
  Hydra+
  Header file module for the internal structures
  Nikos Mourgis deus-ex.gr 2024
*/

/*Error codes******************************************/
/*Include errors*/
#define HDR_ERROR_LOADER_INCLUDE_NO_FILENAME		-200 /*the include directive has not a filename component*/
/***************/
#define HDR_ERROR_LOADER_NOT_TERMINATED				-201 /*the instruction is not terminated with a [;]*/
#define HDR_ERROR_LOADER_STRING_MISMATCH			-202 /*the instruction has a literal string that has not a match pair of [""] or[``]*/
#define HDR_ERROR_LOADER_STRING_STRAY				-203 /*a ["] or [`] with not a pair in the code is found*/

#define HDR_ERROR_LOADER_IS_NULL					-204 /*loader object is NULL*/
#define HDR_ERROR_INCL_SCRIPT_IS_NULL				-205 /*an included script structure is passed as NULL*/ 

#define HDR_ERROR_LOADER_EXPR_MISSING				-206 /*In a state that the loader expected to find an expression , empty entity was returned*/ 
#define HDR_ERROR_LOADER_TOKEN_IS_NULL				-207 /*the token from an entity is NULL. An entity has at least one token*/
#define HDR_ERROR_LOADER_NEW_INSTR_WHILE_PENDING	-208 /*The loader found the start of a new instruction before the previous one completed*/
#define HDR_ERROR_LOADER_SYNTAX_ERROR				-209 /*Generic error message for malformed instruction. The loader will print a detailed error description*/
#define HDR_ERROR_LOADER_INTERNAL_ERROR				-210 /*Generic error message for weird situations that are most likelly bugs in the code*/

#define HDR_VAR_ADD_ERROR							4000000000
#define HDR_CUSTOM_FUNCTION_ADD_ERROR				4000000000
#define HDR_OBJECT_CLASS_ADD_ERROR				    4000000000
#define HDR_VAR_LIST_HASH_BUCKETS					128 /*this value will be used to create the buckets in the hash table (var list)*/
#define HDR_CUSTOM_FUNC_LIST_HASH_BUCKETS			256 /*this value will be used to create the buckets in the hash table (custom functions list)*/
#define HDR_OBJ_CLASS_HASH_BUCKETS					64  /*this value will be used to create the buckets in the hash table (object class list)*/
#define HDR_SUCCESS									0   /*generic success code*/
#define HDR_ERROR_SYNTAX_ERROR						-100
#define HDR_ERROR_TOKEN_NAME_ERROR					-101
#define HDR_ERROR_TOKEN_NULL_P						-102
#define HDR_ERROR_SECTION_MISSMATCH					-103  
#define HDR_ERROR_LITERAL_AS_PART					-104  
#define HDR_ERROR_NAME_NULL							-105
#define HDR_ERROR_NAME_EMPTY						-106
#define HDR_ERROR_EXPR_STR_NULL						-107
#define HDR_ERROR_EXPR_STR_EMPTY					-108
#define	HDR_ERROR_EMPTY_INDEX_BEFORE_CURRENT		-109
#define HDR_ERROR_TOKEN_TYPE_FOR_PART_ROOT			-110
#define HDR_ERROR_TOKEN_FUNCTION_CANNOT_HAVE_PART	-111
#define HDR_ERROR_STRAY_CHARS_BEFORE_INDEX			-112
#define HDR_ERROR_BRACKET_AFTER_DOT					-113
#define HDR_SCRIPT_END								-114
#define HDR_ERROR_SEPARATOR_DOT_	 				107

/****************************************************/

#define HYDRA_MAX_ENTITY_NAME 128 /*a variable , function , object name etc cant be bigger than this*/
#define HYDRA_MAXIMUM_THREADS 40000 /*arbritary number of threads :D*/
#define HDR_FUNCTION_PARAM_ABS_REF 1010 /*set this to function params placeholders to avoid deleting used memory*/ 
/*******************************************************/
DXLONG64 GLID = 0;
DXLONG64 _GET_GLOBAL_LOADER_ID()
{
	GLID++;
	return GLID;
}
/*******************************************************/

/*******************************************************/
typedef struct hdr_block			*PHDR_BLOCK         ;
PHDR_BLOCK hdr_block_create(PHDR_BLOCK parent)			;
PHDR_BLOCK hdr_block_free(PHDR_BLOCK block)				;
/*the function deep copy the block and return the copy*/
PHDR_BLOCK hdr_block_copy(PHDR_BLOCK block,PHDR_BLOCK parent); 
/*******************************************************/
typedef struct hdr_expression		*PHDR_EXPRESSION    ;
/*******************************************************/
typedef PDX_LIST PHDR_EXPRESSION_LIST					;
PHDR_EXPRESSION_LIST hdr_expression_list_free(PHDR_EXPRESSION_LIST expressions);
PHDR_EXPRESSION      hdr_expression_free(PHDR_EXPRESSION expr);
/*******************************************************/
typedef struct hdr_instruction		*PHDR_INSTRUCTION   ;
typedef struct hdr_complex_token	*PHDR_COMPLEX_TOKEN	;
/*******************************************************/

typedef struct hdr_interpreter *PHDR_INTERPRETER ;
PHDR_INTERPRETER hdr_interpreter_free(PHDR_INTERPRETER interpreter,bool for_async);
enum exec_state { exec_state_ok, exec_state_error, exec_state_loop_exit,exec_state_loop_cont,exec_state_return };/*special return type for handling the loop struct*/

/*******************************************************/
typedef struct hdr_object_class *PHDR_OBJECT_CLASS  ;
PHDR_OBJECT_CLASS hdr_object_class_free(PHDR_OBJECT_CLASS obj);
/***********************************************************************************************************/

PDX_LIST hdr_simple_list_free(PDX_LIST list)  ;
PDX_HASH_TABLE hdr_fast_list_free(PDX_HASH_TABLE list);
/******************************************************/
typedef struct hdr_complex_str_res
{
	PDX_LIST		vars_pos   ; 
	PDX_STRINGLIST  text_lines ; /*the lines of the text. For easier manipulation lines are "break" at character 10 or in a variable name*/

} *PHDR_COMPLEX_STR_RES ;

PHDR_COMPLEX_STR_RES hdr_res_str_free(PHDR_COMPLEX_STR_RES str) ;

/******************************************************/

/******************* DATABASE SUPPORT ************************/

enum hdr_db_type {hdr_db_sqlite,hdr_db_mariadb,hdr_db_odbc};
typedef struct hdr_database_conn
{
	enum hdr_db_type type ;
    void *conn_obj  ;
} *PHDR_DATABASE_CONN ;


PHDR_DATABASE_CONN hdr_dom_db_conn_free(PHDR_DATABASE_CONN db) ;
PDX_QUERY hdr_dom_dataset_free(PDX_QUERY query);
PDX_DB_ROW_WRAP hdr_dom_datarow_free(PDX_DB_ROW_WRAP row);

/******************* SOCKET SUPPORT *************************/
#ifdef _LINUX
bool IS_SOCKETS_INIT = true;
#define INVALID_SOCKET  -1 /*the socket descriptors are unsigned in windows but signed in linux, so we will set the INVALID_SOCKET for Linux as -1*/
int GetLastError(){return errno;}
#define SOCKET int
#endif

typedef struct sock_tcp_client
{
 #ifdef _WIN32
	SOCKET sock ;
 #endif
 #ifdef _LINUX
	int sock ;
 #endif
 struct sockaddr_in remote;
} *PSOCK_TCP_CLIENT , *PSOCK_TCP_SERVER , *PSOCK_TCP_INCOMING;

PSOCK_TCP_CLIENT hdr_sock_client_create()
{
  PSOCK_TCP_CLIENT tsock = (PSOCK_TCP_CLIENT)malloc(sizeof(struct sock_tcp_client)) ;
  if(tsock == NULL) return NULL ;
  tsock->sock   = INVALID_SOCKET ;

  return tsock ;
}

PSOCK_TCP_CLIENT hdr_sock_client_free(PSOCK_TCP_CLIENT tsock)
{
	 if(tsock == NULL) return NULL ;
	 if(tsock->sock != INVALID_SOCKET) 
	 {
		  #ifdef _WIN32
			closesocket((SOCKET)tsock->sock) ;
		  #endif

		  #ifdef _LINUX
			close((int)tsock->sock);
		  #endif
	 }

	 free(tsock);
	 return NULL ;
}



PSOCK_TCP_SERVER hdr_sock_server_create()
{
  PSOCK_TCP_SERVER tsock = (PSOCK_TCP_SERVER)malloc(sizeof(struct sock_tcp_client)) ;
  if(tsock == NULL) return NULL ;
  tsock->sock   = INVALID_SOCKET ;

  return tsock ;
}

PSOCK_TCP_SERVER hdr_sock_server_free(PSOCK_TCP_SERVER tsock)
{
	 if(tsock == NULL) return NULL ;
	 if(tsock->sock != INVALID_SOCKET) 
	 {
		  #ifdef _WIN32
			closesocket((SOCKET)tsock->sock) ;
		  #endif

		  #ifdef _LINUX
			close((int)tsock->sock);
		  #endif
	 }

	 free(tsock);
	 return NULL ;
}

/************ SSL Sockets ***********************************/

typedef struct sock_ssl_obj
{
   WOLFSSL_METHOD       *method   ;
   WOLFSSL_CTX          *ctx      ;
   WOLFSSL              *ssl      ;
#ifdef _WIN32
	SOCKET sock ;
 #endif
 #ifdef _LINUX
	int sock ;
 #endif
   struct sockaddr_in remote	  ;
}*PSOCK_SSL_CLIENT, *PSOCK_SSL_SERVER , *PSOCK_SSL;

PSOCK_SSL hdr_ssl_object_create()
{
    PSOCK_SSL info = (PSOCK_SSL)malloc(sizeof(struct sock_ssl_obj));
    if (info == NULL) return NULL ;
    info->ctx       = NULL ;
    info->method    = NULL ;
    info->sock      = INVALID_SOCKET;
    info->ssl       = NULL ;
   
	/*info->remote = (struct sockaddr_in *)malloc(sizeof(struct sockaddr_in *)) ;
	if(info->remote == NULL) 
	{
	  free(info);
	  return NULL ;
	}
	*/
	return info ;
}

PSOCK_SSL hdr_ssl_object_free(PSOCK_SSL ssl_client)
{
	/*check the members and free them*/
  if(ssl_client == NULL) return  NULL     ;

  //if(ssl_client->remote != NULL) free(ssl_client->remote)         ;
  if(ssl_client->sock != INVALID_SOCKET) 
	 {
		  #ifdef _WIN32
			closesocket((SOCKET)ssl_client->sock) ;
		  #endif

		  #ifdef _LINUX
			close((int)ssl_client->sock);
		  #endif
	 }

  if(ssl_client->ssl != NULL) wolfSSL_free(ssl_client->ssl)       ;
  if(ssl_client->ctx != NULL) wolfSSL_CTX_free(ssl_client->ctx)   ;
  free(ssl_client);
  
  return NULL ;
}



/***********************************************************/

/*
  Special structure to store byte buffers 
*/

typedef struct hdr_bytes
{
	char      *bytes ;
	DXLONG64  length ;

} *PHDR_BYTES;

PHDR_BYTES hdr_bytes_create(DXLONG64 length)
{
  PHDR_BYTES bytes = (PHDR_BYTES)malloc(sizeof(struct hdr_bytes)) ;
  if(bytes == NULL) return NULL ;
  if(length == 0)
  {
    bytes->bytes  = NULL ;
	bytes->length = 0    ;
	return bytes		 ;
  }
  bytes->bytes = (char*)malloc(length) ;
  if(bytes->bytes == NULL)
  {
      free(bytes);
	  return NULL ;
  }

  bytes->length = length ;
  return bytes ;
}

PHDR_BYTES hdr_bytes_free(PHDR_BYTES bytes)
{
  if(bytes == NULL) return NULL ;

  free(bytes->bytes) ;
  free(bytes)		 ;
  return NULL		 ;
}

/***************************************/

/*
 The following structure is the native variable of the Hydra+ and the supported types
 Note : The variables are stored in the code block that are declared in. The scope rules
 are : The variables that are declared in the main script have global scope and all the
 entities can access them. All the other variables can be accesed in the confides of the block
 that are declaret in , and all the blocks that are declared inside this block.
 When the interpreter proccess a variable name , first check if the variable
 exists in the local block , the one that the executed instruction belongs, then
 searches to the parent block and then in its parent block etc until finds the variable.
*/

enum hdr_var_type
{
	hvt_undefined, hvt_pointer, hvt_float, hvt_integer, hvt_null,hvt_bool, hvt_list, hvt_string_list, hvt_string_list_sort, 
	hvt_int_list_sort, hvt_float_list_sort,hvt_fast_list, hvt_http, hvt_database, hvt_dataset,hvt_data_row,
	hvt_tcp_socket_client, hvt_tcp_socket_server,
	hvt_ssl_client, hvt_ssl_server, hvt_bytes, hvt_file, hvt_simple_string,
	hvt_simple_string_bcktck, hvt_complex_string, hvt_complex_string_resolve,hvt_unicode_string,hvt_codepoint,
	hvt_object
};

enum hdr_var_ref
{
	hvf_block, hvf_dynamic, hvf_temporary,hvf_temporary_ref, hvf_system , hvf_temporary_ref_user_func
};
/*
  hvf_block          : a variable declared in a block
  hvf_dynamic        : a variable that is created dynamically in a list 
  hvf_temporary      : a variable that can be freed after use in its entirety
  hvf_temporary_ref  : a variable that can be freed after the obj member is set to NULL as the obj member does not belong to it
  hvf_system		 : a permanent variable that is used by the system for calculations an expression solving
  hvf_temporary_ref_user_func : a variable that has been passed by a user function and it will release its memory.
*/

/*
 vt_undefined				 : The variable is undefined
 hvt_pointer				 : The variable keeps in obj an non Hydra+ object.  
 hvt_float					 : A double type
 hvt_integer				 : An int64_t integer
 hvt_bool					 : The boolean type
 hvt_list					 : The simple list type. A collection of objects with an arithmetic index and an optional alpharithmetic one
 hvt_string_list			 : A simple string list
 hvt_string_list_sort		 : A sorted string list
 hvt_int_list_sort			 : A sorted list of integers
 hvt_float_list_sort		 : A sorted list of doubles
 hvt_fast_list				 : A list with a hash table for fast access of the objects	
 hvt_http					 : A simple http/https client
 hvt_database				 : A database connection , supports SQLite , MariaDB/MySQL , ODBC connections (tested with SQL Server)
 hvt_dataset				 : A dataset created from a query
 hvt_data_row				 : A row from a dataset
 hvt_tcp_socket_client		 : A "raw" client TCP/IP socket
 hvt_tcp_socket_server		 : A "raw" server TCP/IP socket
 hvt_ssl_client				 : A "raw" ssl client socket
 hvt_ssl_server				 : A "raw" ssl server socket
 hvt_bytes					 : The representation of the clasical buffer
 hvt_file					 : A file for reading/writing in the disk
 hvt_simple_string			 : A simple utf8 string with the internal structure of a C string (zero terminated)
 hvt_simple_string_resolved  : A simple utf8 string that will expand every variable name with the notation {$var} in it.
 hvt_complex_string			 : A string that the replace string functions will handle more efficiently
 hvt_complex_string_resolve  : A dynamic list that stores the strings and the variable names seperatly so to make expansions efficiently
 hvt_wide_string			 : A special string that stores every character as an UTF32 number   
 hvt_codepoint				 : The common form of character that all the strings are returning. This is a simple integer number.
 hvt_object					 : The variable is a user defined object
*/

typedef struct hdr_var
{
	DXLONG64     integer	; /*if the variable is integer then its value is stored here*/
	double       real		; /*if the variable is a float then its value is stored here*/
	void*		 obj		; /*Any other type , string type too is stored in this       */
	enum hdr_var_type type	; /*the variable type*/
	PDX_STRING   name		; /*the variable name*/
	PHDR_BLOCK   block		; /*the block the variable belongs*/
	bool		 literal	; /*the variable's value will not change , this is used for literals*/
	bool		 is_ref     ; /*a flag that determine that this variable MUST NOT release its object --OBSOLETE-- but i will lefte the code */
	bool		 need_indx  ; /*a flag that is set to true if the variable must return a byte or character*/
	enum hdr_var_ref  var_ref  ;
	void     *func_ref_var   ; /*this will be set to NULL OR to the variable that a function return. This is mandatory to avoid access violation errors*/
	/*may more members will be added later*/
} *PHDR_VAR;


/*Functions for manipulating variables*/


PHDR_VAR hdr_var_create(void *obj, char *name, enum hdr_var_ref ref_type, PHDR_BLOCK block)
{
	PHDR_VAR var = (PHDR_VAR)malloc(sizeof(struct hdr_var));

	var->integer   = 0       ;
	var->real	   = 0       ;
	var->obj       = obj     ;
	/*******************/
	var->name      = dx_string_createU(NULL, name);
	var->block     = block ;
	var->literal   = false ;
	var->var_ref   = ref_type ;
	var->type	   = hvt_undefined;
	var->is_ref    = false  ;
	var->need_indx = false  ;
	var->func_ref_var = NULL;
	return var			    ;
}

/*
The deallocation function for the variable.
The function checks the type to call the right
deallocation function for the obj.
Returns NULL
*/

bool __hdr_is_complex_type(PHDR_VAR var)
{
	if((var->obj != NULL)&&(var->type !=hvt_simple_string)&&(var->type !=hvt_simple_string_bcktck)&&(var->type !=hvt_unicode_string))
	{
		return true ;
	}

	return false; 
}

PHDR_VAR hdr_var_free(PHDR_VAR var)
{
	if (var == NULL) return NULL;
	dx_string_free(var->name);

	//if(var->is_ref == false)
	//{
		/*
		  a small memory managment to avoid access violation when try to free already freed memory
		  in the finalization of the Hydra+ 
		*/
		if(_POINTERS != NULL)
		{
			if((var->obj != NULL)&&(var->type !=hvt_simple_string)&&(var->type !=hvt_simple_string_bcktck)&&(var->type !=hvt_unicode_string))
			{
			   /*check if the pointer is already freed or it have to be freed*/
			   if(hdr_mem_pointers_is_freed(_POINTERS,(uint64_t)var->obj) == true)
			   goto end ;
			}
		}
		/***********/
		/******************** Detect memory Leaks **********************/
	    if(_LOGGER != NULL)
		{
		  if(__hdr_is_complex_type(var) == true)
		  {
		    hdr_mem_pointers_logger_remove(_LOGGER,var->obj) ;
		  }
		}
		switch (var->type)
		{
			case hvt_undefined			: 
			case hvt_pointer			:
			case hvt_float				:
			case hvt_integer			:
			case hvt_null               :
			case hvt_codepoint			:
			case hvt_bool				:{/*nothing to do*/ }
			break;
			
			case hvt_list					: {hdr_simple_list_free((PDX_LIST)var->obj);}
			break;
			case hvt_string_list			: {dx_stringlist_free((PDX_STRINGLIST)var->obj);}
			break;
			case hvt_string_list_sort		: {}
			break;
			case hvt_int_list_sort			: {}
			break;
			case hvt_float_list_sort		: {}
			break;
			case hvt_fast_list				: {hdr_fast_list_free((PDX_HASH_TABLE)var->obj);}
			break;
			case hvt_http					: {}
			break;
			case hvt_database				: {hdr_dom_db_conn_free((PHDR_DATABASE_CONN)var->obj);}
			break;
			case hvt_dataset				: {hdr_dom_dataset_free((PDX_QUERY)var->obj);}
			break;
			case hvt_data_row				: {hdr_dom_datarow_free((PDX_DB_ROW_WRAP)var->obj);}
			break;
			case hvt_tcp_socket_client		: {hdr_sock_client_free((PSOCK_TCP_CLIENT)var->obj);}
			break;
			case hvt_tcp_socket_server		: {hdr_sock_server_free((PSOCK_TCP_SERVER)var->obj);}
			break;
			case hvt_ssl_client				: {hdr_ssl_object_free((PSOCK_SSL)var->obj);}
			break;
			case hvt_ssl_server				: {hdr_ssl_object_free((PSOCK_SSL)var->obj);}
			break;
			case hvt_bytes					: {hdr_bytes_free((PHDR_BYTES)var->obj);}
			break;
			case hvt_file					: {if(var->obj != NULL)fclose((FILE*)var->obj);}
			break;
			case hvt_simple_string			: { dx_string_free((PDX_STRING)var->obj); }
			break;
			case hvt_simple_string_bcktck	: { dx_string_free((PDX_STRING)var->obj);}break;
			case hvt_complex_string			: { dx_stringlist_free((PDX_STRINGLIST)var->obj); }break;
			case hvt_complex_string_resolve : { hdr_res_str_free((PHDR_COMPLEX_STR_RES)var->obj);}break;
			case hvt_unicode_string			: { dx_string_free((PDX_STRING)var->obj);}break;
			case hvt_object					: { hdr_object_class_free((PHDR_OBJECT_CLASS)var->obj); }break;

		}
	  

	//}
	end:
	free(var);
	return NULL;
}

static inline PHDR_VAR hdr_var_set_obj(PHDR_VAR var , void *obj)
{
    var->obj  = obj  ;

	/*memory leaks detection!*/
	if(_LOGGER != NULL)
	{
		if(__hdr_is_complex_type(var) == true)
		{
          /*
		   We will support variable assigments to clarify better the situation.
		   for example to return accurate info for the type of : 
		   $var = List.CreateList();
		   $var[] =List.CreateList();
		   $var2 = $var[0] ;
		   $var[0].Release();
		   $var.Free();

		   The $var2 remains allocated BUT the system will return Variable -> 0 
		   because it has the creation name that was the 0 index of the list.
		   We want to print the [$var2] as variable name 

		  */ 
 
          /*try to remove the variable and re enter it*/
		  hdr_mem_pointers_logger_remove(_LOGGER,obj) ;
		  hdr_mem_pointers_logger_add(_LOGGER,var->name->stringa,obj) ;
		}
	}

   return var ;
}

static inline PHDR_VAR hdr_var_release_obj(PHDR_VAR var)
{
	/*Memory leaks detection! If the variable is released then we have no way to know if its intentional !*/
	if(_LOGGER != NULL)
	{
		if(__hdr_is_complex_type(var) == true)
		{
		  hdr_mem_pointers_logger_remove(_LOGGER,var->obj) ;
		}
	}
	 var->obj = NULL  ;
	 return var		  ;
}

/*Copy all the "hard" data and the object pointer to a new created variable*/
PHDR_VAR hdr_var_clone(PHDR_VAR var,char *name, PHDR_BLOCK block)
{
	PHDR_VAR v = hdr_var_create(NULL,name,var->var_ref,block);
	v->integer = var->integer ;
	v->real    = var->real	  ;
	v->type	   = var->type	  ;
	v->obj     = var->obj	  ;
	return v;
}

/*Copy all the "hard" data and the object pointer to the destination return the destination*/
PHDR_VAR hdr_var_copy_shallow(PHDR_VAR src , PHDR_VAR dest)
{
	if ((src == NULL) || (dest == NULL )) return NULL;

	dest->integer = src->integer;
	dest->real    = src->real;
	dest->type    = src->type;
	dest->obj     = src->obj;
	dest->block   = src->block;
	dest->name	  = dx_string_createU(dest->name, src->name->stringa);
	dest->type    = src->type ;
	dest->is_ref  = src->is_ref;
	dest->var_ref = src->var_ref ;
	dest->literal = src->literal;
	dest->need_indx = src->need_indx ;
	dest->func_ref_var = NULL;
	return dest;
}

/*Copy all the "hard" data and deep copy the object RESERVED NOT USED YET*/
PHDR_VAR hdr_var_copy(PHDR_VAR var, char *name, PHDR_BLOCK block)
{
	PHDR_VAR v = hdr_var_create(NULL,name,var->var_ref,block);

	v->integer = var->integer;
	v->real	   = var->real;
	v->type    = var->type;
	/*call the specific object copy */
	switch (var->type)
	{
	    case hvt_list					: {}
			break;
		case hvt_string_list			: {}
			break;
		case hvt_string_list_sort		: {}
			break;
		case hvt_int_list_sort			: {}
			break;
		case hvt_float_list_sort		: {}
			break;
		case hvt_fast_list				: {}
			break;
		case hvt_http					: {}
			break;
		case hvt_database				: {}
			break;
		case hvt_dataset				: {}
			break;
		case hvt_tcp_socket_client		: {}
			break;
		case hvt_tcp_socket_server		: {}
			break;
		case hvt_ssl_client				: {}
			break;
		case hvt_ssl_server				: {}
			break;
		case hvt_bytes					: {}
			break;
		case hvt_file					: {}
			break;
		case hvt_simple_string			: {}
			break;
		case hvt_simple_string_bcktck	: {}
			break;
		case hvt_complex_string			: {}
			break;
		case hvt_complex_string_resolve	: {}
			break;
		case hvt_unicode_string			: {}
			break;
		case hvt_object					: {}
			break;

	}

	return v;
}

/***********************************************************************************************************/

 /*
  The following structure stores the local variables in a block
  Note that when a block invalidates (usually only when a thread terminates and when a function returns)
  all this variables will be freed. On the other hand , when a block just exits like for example in a loop{}
  or in an if(){} the variables will be retained.
 */

 /***********************************************************************************************************/

typedef struct hdr_var_list
{
	PDX_HASH_TABLE list  ;
	PHDR_BLOCK     block ; /*in which block the variables belongs*/
} *PHDR_VAR_LIST;


uint32_t hdr_var_list_add(PHDR_VAR_LIST list,PHDR_VAR var);

// the variables of a block or other entities are store in there. This is a list with a hash table for quick searching
/* functions for manipulating the variables list  */

/*Create a variable list (hash table)*/
PHDR_VAR_LIST hdr_var_list_create(PHDR_BLOCK block)
{
	PHDR_VAR_LIST var_list = (PHDR_VAR_LIST) malloc(sizeof(struct hdr_var_list)) ;
	var_list->list  = dx_HashCreateTable(HDR_VAR_LIST_HASH_BUCKETS)             ;
	var_list->block = block ;

	return var_list;
}

void hdr_var_list_copy(PHDR_VAR_LIST dest , PHDR_VAR_LIST source)
{
	 /* The dest and source HAS to be created. 
		The variables from the source will be copied and inserted in the dest.
	 */

	 for(int i = 0; i < source->list->length;i++)
	 {
		  /*for all the buckets*/
		  PDX_LIST    sbucket = source->list->buckets[i] ;
		  PDX_LIST    dbucket = dest->list->buckets[i]   ;
		  PDXL_NODE   snode   = sbucket->start			 ;
		  /*copy the backet contents*/
		  while(snode != NULL)
		  {
			PHDR_VAR svar = (PHDR_VAR)snode->object->obj ;
			PHDR_VAR nvar = hdr_var_clone(svar,svar->name->stringa, dest->block) ;
			hdr_var_list_add(dest,nvar) ;
			snode = snode->right ; /*next variable*/
		  }
	 }
 

	 return ;
}

/*TO USED ONLY IN THE VARIABLE LIST DESTRUCTION*/
void hdr_var_list_free_var_obj(PDXL_OBJECT obj)
{
	PHDR_VAR var = obj->obj			;
	if(var==NULL) return			; /*in case of function parameters that the actual parameter placeholder is NULL*/
	if(var->name == obj->key)
	obj->key = NULL					; // the key in the obj is a reference to the variable name so we set it to NULL
									  // the actual PDXL_OBJECT will be freed in the destruction of the list (hash table)
	/*
	 There is the special case that this object holds an absolute reference of a variable that has been passed as a parameter
	 to a user function , in this case we have to NOT free the variable as it belonfs in another block
	*/
	if(obj->flags != HDR_FUNCTION_PARAM_ABS_REF) obj->obj = hdr_var_free(var)	;
	return;
}

/*Free the list and all the variables in it, returns NULL*/
PHDR_VAR_LIST hdr_var_list_free(PHDR_VAR_LIST list)
{
	dx_HashDestroyTable(list->list, hdr_var_list_free_var_obj);
	free(list) ;
	return NULL;
}

DXLONG64 hdr_var_list_count(PHDR_VAR_LIST list)
{
	if (list == NULL) return -1;
	return list->list->count;
}

/*Add a variable in the list*/
uint32_t hdr_var_list_add(PHDR_VAR_LIST list,PHDR_VAR var)
{
	if((list == NULL)||(var == NULL)) return HDR_VAR_ADD_ERROR;
	PDXL_OBJECT obj = dxl_object_create() ;
	obj->key = var->name ; /*set the hash key as the variable name , this is a reference so beware of freeing usefull memory*/
	obj->obj = var		 ;
	/*add the object in the list*/
	return dx_HashInsertObject(list->list, obj);

}

uint32_t hdr_var_list_add_inherit_block(PHDR_VAR_LIST list, PHDR_VAR var)
{
	/*add the variable in the block and sets the variables block as the lists block*/
	if ((list == NULL) || (var == NULL)) return HDR_VAR_ADD_ERROR;
	PDXL_OBJECT obj = dxl_object_create();
	var->block = list->block;
	obj->key = var->name; /*set the hash key as the variable name , this is a reference so beware of freeing usefull memory*/
	obj->obj = var;
	/*add the object in the list*/
	return dx_HashInsertObject(list->list, obj);

}



PHDR_VAR hdr_var_list_delete_var(PHDR_VAR_LIST list, PDX_STRING var_name)
{
	/*remove the variable (if exists) from the list and release its memory returns NULL*/
	PDXL_OBJECT obj = dx_HashRemoveItem(list->list, var_name);
	if (obj != NULL)
	{
		PHDR_VAR var = obj->obj ;
		obj->key     = NULL		; // this will be released when the variable will be freed as it was just a reference
		hdr_var_free(var)       ;
		free(obj)				;

	}

	return NULL  ;
}

/***********************************************************************************************************/
/*
 The complex token is the entity that represents a Hydra+ Language Variant Token .
 This means that a CT can describe a function and its parameters , a simple variable , a literal ,
 a multipart variable or a special command.
 Examples :
 func("hello",0)
 $var
 $list.Count()
 return
 "Molly"
 $data[1][2].Title[0]
*/

/*
 The type of the token uses.
 hdr_tk_uninit			: The token is uninitialized
 hdr_tk_ref_brackets    : The token is a indexed entity
 hdr_tk_multipart		: The token is an object with a function or an indexed entity(that uses the dot for beautification and not the [])
 hdr_tk_index			: The token is an index to its left token. (in actuality to the root , but first must be dereferenced the left indx if exists)
 hdr_tk_simple		    : The token has not any reference scheme and is a simple word
 hdr_tk_simple_param	: The token is a simple word but is paired with other words in the same sentence ( a line that is terminated by an ; e.g return "hello")
 hdr_tk_function		: The token describes a function and its parameters
 hdr_tk_literal			: The token is a literal value and the val member has a not changeable variable
 hdr_tk_operator		: The token is an operator in an expression
 hdr_tk_expression      : The token is an expression
 hdr_tk_variable		: The token is a single variable
 */

enum hdr_complex_token_type {hdr_tk_uninit,hdr_tk_ref_brackets,hdr_tk_multipart,
							 hdr_tk_index,hdr_tk_simple,hdr_tk_simple_param,hdr_tk_function,
							 hdr_tk_literal,hdr_tk_operator, hdr_tk_expression, hdr_tk_bool_expression,
							 hdr_tk_comp_expression,hdr_tk_variable};


/*functions that copy a complex token and a full expression*/

/*The function will create a token and fill it with the exact data of the token*/
PHDR_COMPLEX_TOKEN hdr_complex_token_copy(PHDR_COMPLEX_TOKEN token) ;

/*the function will create an expression and will fill it with the exact data of the expr*/
PHDR_EXPRESSION       hdr_expression_copy(PHDR_EXPRESSION expr)		;
/*the function will create an expression list identical to the expr_list*/
PHDR_EXPRESSION_LIST hdr_expression_list_copy(PHDR_EXPRESSION_LIST expr_list) ;
/*the function copy and return an instruction */
PHDR_INSTRUCTION hdr_instruction_copy(PHDR_INSTRUCTION instr,PHDR_INSTRUCTION iparent,PHDR_BLOCK parent)  ;

/**********************************************************/

struct hdr_complex_token
{
	PDX_STRING		    	      ID        ; /* the identification of the token, e.g $var, myFunction etc. */
	enum hdr_complex_token_type   type      ; /* what type of token this is */
										   
	PHDR_COMPLEX_TOKEN       root	   ; /* if the token is a multipart token then only the root token will have the resolved value (val)
											the root is pointing to the root token, if the token is the root, this field is NULL*/
	PHDR_COMPLEX_TOKEN		 next_part ; /* The next part it will be NULL if the token is not a multipart one. A multipart token
											separates every part with a dot[.] e.g $var.test or $var["test"].runme()*/
	PHDR_COMPLEX_TOKEN		 previous_part; /* The previous part it will be NULL if the token is not a multipart one or is the root token. A multipart token
											separate every part with a dot[.] e.g $var.test or $var["test"].runme()*/

	PHDR_EXPRESSION_LIST     parameters; /* If the token is a function, there is a posibility that there where some parameters.
											the parameters are expressions that will be evaluated to PHDR_VAR before passed to the
											function. If the token is not a function the parameters is NULL. If the token
											is a function but does not have any parameters the parameters is created but
											its count is 0*/

	PHDR_EXPRESSION			 expression ; /*the token calculate its value from an expression*/

	PHDR_VAR val					    ; /* if the token is a literal value, then this val is set from the loader.
											if the token is a function token then this variable will be shallow copy the
											resulting variable of the function. If the token is resolved to a variable
											then the val will shallow copy the variable.
										 */
};

PHDR_COMPLEX_TOKEN hdr_complex_token_create(PHDR_COMPLEX_TOKEN root, char* ID)
{
	PHDR_COMPLEX_TOKEN token = (PHDR_COMPLEX_TOKEN)malloc(sizeof(struct hdr_complex_token));
	if(token == NULL) return NULL ;

	token->ID = dx_string_createU(NULL, ID) ;
	token->next_part      = NULL;
	token->previous_part  = NULL;
	token->parameters = NULL;
	token->root		  = root;
	token->val = hdr_var_create(NULL, "",hvf_system,NULL);
	token->val->type = hvt_undefined ;
	token->expression = NULL; /*the expression wil be created only if it has at least one token in it*/
	token->type = hdr_tk_uninit;

	return token;
}

PHDR_COMPLEX_TOKEN hdr_complex_token_free(PHDR_COMPLEX_TOKEN token)
{
	if (token == NULL) return NULL;
	/*go to the last token in the list and delete from the ground up*/
   
	PHDR_COMPLEX_TOKEN tkn = token;
	while (tkn->next_part != NULL)
	{
		tkn = tkn->next_part;
	}

	/*now free all the tokens*/
	while (tkn != NULL)
	{
		PHDR_COMPLEX_TOKEN ttkn = tkn;
		tkn = tkn->previous_part;

		dx_string_free(ttkn->ID);

		if(ttkn->type != hdr_tk_literal) /*if the token is not a literal the obj in the val does not belong to the token*/
		ttkn->val->obj = NULL; /* the variable object lives in the code and the blocks variables not in the tokens this is just a reference*/
		
		hdr_var_free(ttkn->val);

		/*free the expression (the check for the NULL is being done in the function)*/
		hdr_expression_free(ttkn->expression);
		/*
		  the parameters are an expression list
		  free the list and the expressions inside
		  (the check for the NULL is being done in the function)
		*/
		hdr_expression_list_free(ttkn->parameters);
		free(ttkn);
	}

	return NULL;
}


void hdr_complex_token_copy_tree(PHDR_COMPLEX_TOKEN root,PHDR_COMPLEX_TOKEN prev_token, PHDR_COMPLEX_TOKEN token_to_copy)
{
/*
   create a token , fill it with the copied data from token and return it
   this is a tricky situation as the tokens have pointers to other tokens.
  */
  if(token_to_copy == NULL) return ;
  PHDR_COMPLEX_TOKEN ntoken = hdr_complex_token_create(NULL,token_to_copy->ID->stringa)  ;
  ntoken->expression		= hdr_expression_copy(token_to_copy->expression)		;
  ntoken->previous_part		= prev_token											; 
  ntoken->parameters		= hdr_expression_list_copy(token_to_copy->parameters)   ;
  ntoken->root				= root													; 
  ntoken->type				= token_to_copy->type ;
  prev_token->next_part		= ntoken			  ; /*connect the token chain*/


  /*in certain cases the token val has usefull info and its not only for temporary calculations or a placeholder*/
  hdr_var_copy_shallow(token_to_copy->val,ntoken->val) ;

  if(token_to_copy->val->type != hvt_undefined)
  {
    /*the simple strings must be copied */
    if ((token_to_copy->val->type == hvt_simple_string)||(token_to_copy->val->type == hvt_simple_string_bcktck)) 
	{
	   if(token_to_copy->val->obj != NULL)
	   ntoken->val->obj = dx_string_createU(NULL,((PDX_STRING)(token_to_copy->val->obj))->stringa) ;
	   else
		   ntoken->val->obj = NULL ;
	}
	else
		if (token_to_copy->val->type == hvt_unicode_string) ntoken->val->obj = dx_string_createX(NULL,((PDX_STRING)(token_to_copy->val->obj))->stringx) ;
  }


  hdr_complex_token_copy_tree(root,ntoken,token_to_copy->next_part) ;
	
  return ;
}

PHDR_COMPLEX_TOKEN hdr_complex_token_copy(PHDR_COMPLEX_TOKEN token) 
{
  /*
   create a token , fill it with the copied data from token and return it
   this is a tricky situation as the tokens have pointers to other tokens.
  */
  if(token == NULL) return NULL ;
  PHDR_COMPLEX_TOKEN ntoken = hdr_complex_token_create(NULL,token->ID->stringa)  ;
  ntoken->expression		= hdr_expression_copy(token->expression)		;
  ntoken->previous_part		= NULL											; /*this is a root token*/
  ntoken->parameters		= hdr_expression_list_copy(token->parameters)   ;
  ntoken->root				= NULL											; /*this is a root token*/
  ntoken->type				= token->type									;

  /*in certain cases the token val has usefull info and its not only for temporary calculations or a placeholder*/
  hdr_var_copy_shallow(token->val,ntoken->val) ;

  if(token->val->type != hvt_undefined)
  {
    /*the simple strings must be copied */
    if ((token->val->type == hvt_simple_string)||(token->val->type == hvt_simple_string_bcktck)) 
	   ntoken->val->obj = dx_string_createU(NULL,((PDX_STRING)(token->val->obj))->stringa) ;
	else
		if (token->val->type == hvt_unicode_string) ntoken->val->obj = dx_string_createX(NULL,((PDX_STRING)(token->val->obj))->stringx) ;
  }
  
  /*if the token is multipart or indexed then it will have a tree under it , we must copy it*/
  hdr_complex_token_copy_tree(ntoken,ntoken,token->next_part) ;

  return ntoken ;
}


/************************** Tokens list **********************/

typedef PDX_LIST PHDR_TOKENS_LIST;

PHDR_TOKENS_LIST hdr_tokens_list_create()
{
	return dx_list_create_list();
}

/*The function release the list and the tokens inside it returns NULL*/
PHDR_TOKENS_LIST hdr_tokens_list_free(PHDR_TOKENS_LIST list)
{
	if (list == NULL) return NULL;
	PDXL_NODE node = list->start;
	while (node != NULL)
	{
		PDXL_OBJECT obj = node->object;
		hdr_complex_token_free((PHDR_COMPLEX_TOKEN)obj->obj);
		free(obj);
		node = node->right;
	}

	dx_list_delete_list(list);
	return NULL ;
}

void hdr_tokens_list_add_token(PHDR_TOKENS_LIST list,PHDR_COMPLEX_TOKEN token)
{

	PDXL_OBJECT obj = dxl_object_create();
	obj->obj = token;
	dx_list_add_node_direct(list, obj);
	return ;
}

PHDR_COMPLEX_TOKEN hdr_tokens_list_retrieve_token(PHDR_TOKENS_LIST list, DXLONG64 indx)
{
  /*returns NULL if the index is not in range*/
	if ((indx > list->count - 1)||(indx < 0)) return NULL;
	PDXL_NODE  node = dx_list_go_to_pos(list, indx) ;
	return node->object->obj;
}

PHDR_COMPLEX_TOKEN hdr_tokens_list_extract_token(PDXL_NODE node)
{
	return node->object->obj;
}

PDXL_NODE  hdr_tokens_list_next_node(PDXL_NODE node)
{
	if (node == NULL) return NULL;
	return node->right;
}


void hdr_complex_token_list_copy(PHDR_TOKENS_LIST dest ,PHDR_TOKENS_LIST source)
{
	if(source == NULL) return  ;
	PDXL_NODE node = source->start ;
	while(node != NULL)
	{
	    PHDR_COMPLEX_TOKEN s_token = hdr_tokens_list_extract_token(node) ;
		PHDR_COMPLEX_TOKEN ntoken  = hdr_complex_token_copy(s_token) ;


		/*the token is created . add it to the destination*/
		hdr_tokens_list_add_token(dest,ntoken)			;

		node = node->right ;
	}

	return ;
}




/***********************************************************************************************************/

/*
 The expression stores an Hydra language expression that can be one of the following types :
 string concatenation       : The expression comprices from tokens that resulting in string types for concatenation
 algebraic calculation      : The expression comprices from tokens that resulting in numbers for calculation
 boolean expression			: The expression is a boolean expression with || && operators
 arbitrary variable return  : The expression resulting to a variable other type than arithmetic or string.


 For an expression to be valid the expression MUST be compriced the following operators and symbols : +-/*^%[]()
 variables, functions , user defines, an language special commands.
 Example : $x + $var.y / PI * number("125") / $some_obj.list["some_value"]

 An expression must ne preproccesed before analyzed to tokens.
 The most crucial part is to determine what type of expression the expression is.
 Hydra does not support arbritary variable types to be calculated or concatenated.
 That means that a string type variable cannot be in an expression along an integer type or a socket type.
 This make the detection and error handling easier.
 The detection is implemented as this : The loader checks  the first token in the expression.
 The first token sets the type of all the expression.
 If the loader detects a token that has different resulting value than the expressions
 type (integer and real types can be calculated together) , The loader stop the execution
 and throw an error.

 The harder part is the expression analyzing to sub expressions and tokens.
 A tricky part is the arithmetic (algebraic) calculations. As the operators
 priority are not the same we have to calculate based to they priorities.

 Priorities : [()] [^] [multiplication , division] [+-]

 I choosed to encapsulate in parenthesis the priorities and then position then
 in a tree for calculate them in the correct order.

 Example : original calculation : $x+3/2+6*7^1
		   calibrated calc      : $x+(3/2)+(6*(7^1))


 */

enum hdr_expression_type {hdr_expr_concat,hdr_expr_calc,hdr_expr_variant,hdr_expr_boolean,hdr_expr_comparison,hdr_expr_uninit};
struct hdr_expression
{
	PHDR_TOKENS_LIST         tokens ; // a list with the tokens to be calculated
	enum hdr_expression_type type   ; // the type of value that the expression is expected to returns
	PHDR_VAR				 value  ; // the actual value that is calculated, this variable object belongs to the script and it will not be freed until the termination
};

PHDR_EXPRESSION hdr_expression_create()
{
	PHDR_EXPRESSION expr = (PHDR_EXPRESSION)(malloc(sizeof(struct hdr_expression)));
	expr->tokens = hdr_tokens_list_create();/*the value is created, so we must free this in expression destruction*/
	expr->type   = hdr_expr_uninit;
	expr->value = hdr_var_create(NULL, "", hvf_system,NULL); /*the value is created so must be freed in expression destruction*/
	return expr ;
}

PHDR_EXPRESSION hdr_expression_free(PHDR_EXPRESSION expr)
{
	if (expr == NULL) return NULL;
	/*returns NULL*/
	expr->tokens = hdr_tokens_list_free(expr->tokens)  ;
	/*the variable object some times can be only a reference that is invalidated. So test before freed!*/
	if(expr->value !=NULL) expr->value->obj = NULL				;
	hdr_var_free(expr->value)           ;
	free(expr)							;
	return NULL							;
}

PHDR_EXPRESSION hdr_expression_copy(PHDR_EXPRESSION expr)
{
   if(expr == NULL) return NULL ;

   PHDR_EXPRESSION nexpr = hdr_expression_create() ;
   nexpr->type = expr->type ;
   /*copy the token list*/
   hdr_complex_token_list_copy(nexpr->tokens,expr->tokens);
   return nexpr ;
}


PHDR_EXPRESSION_LIST hdr_expression_list_create()
{
	return dx_list_create_list();
}

PHDR_EXPRESSION_LIST hdr_expression_list_free(PHDR_EXPRESSION_LIST expressions)
{
	/*
	  the function frees all the expressions in the list and returns NULL
	*/
	if (expressions == NULL) return NULL;
	PDXL_NODE node = expressions->start;
	while (node != NULL)
	{
		PDXL_OBJECT     obj  = node->object ;
		PHDR_EXPRESSION expr = obj->obj     ;
		hdr_expression_free(expr);
		free(obj);
		node = node->right;
	}
	dx_list_delete_list(expressions);
	return NULL;
}


PHDR_EXPRESSION hdr_expression_list_add(PHDR_EXPRESSION_LIST list , PHDR_EXPRESSION expr)
{
	PDXL_OBJECT obj = dxl_object_create() ;
	if (obj == NULL) return NULL ;
	obj->obj = (void*)expr ;
	dx_list_add_node_direct(list, obj);
	return expr;
}

PHDR_EXPRESSION_LIST hdr_expression_list_copy(PHDR_EXPRESSION_LIST expr_list) 
{
	/*copy every expression*/
	if(expr_list == NULL) return NULL ;

	PHDR_EXPRESSION_LIST nexpr_list = hdr_expression_list_create() ;

	PDXL_NODE node = expr_list->start ;
	while(node != NULL)
	{
		PHDR_EXPRESSION expr  = (PHDR_EXPRESSION)node->object->obj	; 
		PHDR_EXPRESSION nexpr = hdr_expression_copy(expr)			;
		hdr_expression_list_add(nexpr_list,nexpr)					;
		node = node->right ;
	}
 
	return nexpr_list ;
}


/*********************************************************************************************************/

/*
  Instructions are the representation of the string commands in a script.
  The instructions is ready to pass in the interpreter and be executes.
  every instruction has a line number that represents its position in
  the plain text script for debugging purposes.
  An instruction can be comprised by only a left side token (usually a function or a special command to be executed),
  or a left side token and a right side token (when the instruction is an assigment or an if) that can be an expression .
  The Instruction can be of one of the following types :

  if / switch	  : The if instruction to execute with branching.
				    an istruction of the if type has the boolean expression
				    in the left_side and if there was an else in the script
				    the instruction of the else is stored to the ielse.
					the instructions to execute if the expression is true is in the code

 loop			  : The loop instruction executes the code for infinity

 assign			  : The instruction will assign a value to a variable

 cmd    		  : The instruction is a simple command, system command , function or a variable declaration.
 cmd_param		  : The instruction is a command that has some parameter after it e.g. return "hello"
 code_block		  : Reserved , not used in this implementation
 */

enum hdr_instr_type {hdr_i_uninit,hdr_i_if, hdr_i_switch,hdr_i_switch_case,hdr_i_switch_default,hdr_i_loop,
					 hdr_i_assign,hdr_i_cmd,hdr_i_cmd_param,hdr_i_code_block, hdr_i_func_decl,hdr_i_obj_decl,hdr_i_var_cmd} ;
typedef struct hdr_instruction
{
	PHDR_BLOCK			parent			; /* what block the instruction belongs */
	PHDR_COMPLEX_TOKEN  left_side		; /* the left side token , this can be a stand alone special command or function, or a variable */
	PHDR_EXPRESSION     right_side		; /* the expression to be evaluated */
	PHDR_INSTRUCTION    ielse			; /* the instruction that will be used from an if as the else instruction*/
	PHDR_INSTRUCTION    iparent			; /* if the istruction is derived from a previous [if] this member points to that instruction (for debuging purposes)*/
	enum hdr_instr_type type			; /* the type of the instruction */
	DXLONG64			line			; /* the script line from that this instruction was produced */
	PDX_STRING			filename		; /* the file name of the script that this line belongs*/
	PHDR_BLOCK			code			; /* the code block (if the instruction is an if or a loop) */

} *PHDR_INSTRUCTION ;

PHDR_INSTRUCTION hdr_instruction_create()
{
	PHDR_INSTRUCTION instr = (PHDR_INSTRUCTION)malloc(sizeof(struct hdr_instruction));
	if (instr == NULL)  return NULL;
	instr->parent		  = NULL ;
	instr->left_side	  = NULL ;
	instr->right_side	  = NULL ;
	instr->ielse		  = NULL ;
	instr->iparent	      = NULL ;
	instr->type			  = hdr_i_uninit ;
	instr->line			  = 0 ;
	instr->code			  = NULL;
    instr->filename = dx_string_createU(NULL,"");
	return instr;
}

PHDR_INSTRUCTION hdr_instruction_free(PHDR_INSTRUCTION instr)
{
	/* frees all the memory of the instruction */
	if (instr == NULL) return NULL;
	hdr_complex_token_free(instr->left_side)  ;
	hdr_expression_free(instr->right_side)    ;
	hdr_instruction_free(instr->ielse)		  ;
	hdr_block_free(instr->code)				  ;
	dx_string_free(instr->filename)			  ;
	free(instr)								  ;

	return NULL								  ;
}

PHDR_INSTRUCTION hdr_instruction_copy(PHDR_INSTRUCTION instr,PHDR_INSTRUCTION iparent,PHDR_BLOCK parent) 
{

  if(instr == NULL) return NULL ;
  
  PHDR_INSTRUCTION ninstr = hdr_instruction_create() ;
  if (instr == NULL)  return NULL;
  ninstr->parent		  = parent ;
  ninstr->code			  = hdr_block_copy(instr->code,parent)			        ;  
  ninstr->left_side		  = hdr_complex_token_copy(instr->left_side)			;
  ninstr->right_side	  = hdr_expression_copy(instr->right_side)				;
  ninstr->ielse			  = hdr_instruction_copy(instr->ielse,ninstr,parent)	;
  ninstr->iparent	      = iparent												;
  ninstr->type			  = instr->type											;
  ninstr->line			  = instr->line											;
  ninstr->filename		  = dx_string_createU(ninstr->filename,instr->filename->stringa)	;

  return ninstr ;

}

typedef PDX_LIST PHDR_INSTRUCTIONS_LIST;

PHDR_INSTRUCTIONS_LIST hdr_instructions_list_create()
{
	return dx_list_create_list();
}

PHDR_INSTRUCTIONS_LIST hdr_instructions_list_free(PHDR_INSTRUCTIONS_LIST list)
{
	/*free all the memory of the list*/
	PDXL_NODE node = list->start;
	while (node != NULL)
	{
		PDXL_OBJECT obj = node->object;
		PHDR_INSTRUCTION instr = obj->obj;
		hdr_instruction_free(instr);
		free(obj);
		node = node->right;
	}

	dx_list_delete_list(list);
	return NULL ;
}

PHDR_INSTRUCTION hdr_instructions_list_add(PHDR_INSTRUCTIONS_LIST list , PHDR_INSTRUCTION instr)
{
	if ((list == NULL) || (instr == NULL)) return NULL ;
	PDXL_OBJECT obj = dxl_object_create();
	if (obj == NULL) return NULL;
	obj->obj = instr ;
	if (dx_list_add_node_direct(list, obj) == NULL)
	{
		free(obj);
		return NULL;
	}
	return instr     ;
}

PHDR_INSTRUCTION hdr_instruction_list_ret_i(PDXL_NODE node)
{
	return (PHDR_INSTRUCTION)node->object->obj;
}

void hdr_instructions_list_copy(PHDR_INSTRUCTIONS_LIST dest_list,PHDR_INSTRUCTIONS_LIST source_list,PHDR_BLOCK parent_block)
{
	if(source_list == NULL) return ;

	PDXL_NODE node = source_list->start ;
	while( node != NULL )
	{
	  PHDR_INSTRUCTION instr = (PHDR_INSTRUCTION)node->object->obj ;
	  PHDR_INSTRUCTION ninstr = hdr_instruction_copy(instr,NULL,parent_block) ;
	  hdr_instructions_list_add(dest_list,ninstr)					    	  ;
	  node = node->right ;
	}

	return ;
}


/***********************************************************************************************************/
/*
 Custom function.
 A custom function is a function that the programmer has wrote for Hydra in the Hydra's syntax.
 A custom function can be called like the system functions and can return a value or not.
 Every parameter that are passed in the custom functions , are passed as references and they are not
 copies of the originals. The literals are passed as constant variables and their values cannot
 be changed inside the function.
 The function can return a variable.
 If a function is executed as [async] then the returned value is lost

 A significant difference when a custom function is executed to the main thread or as async

*/

enum hdr_user_func_result {hdr_user_func_value,hdr_user_func_error,hdr_user_func_success,hdr_user_func_not_found};
enum hdr_user_func_result hdr_run_user_function(PHDR_INTERPRETER inter,PHDR_COMPLEX_TOKEN token,PHDR_VAR* result,bool as_go) ;
typedef struct hdr_custom_function
{

	/*
   a list of pdx_strings that are the name of the parameters.
   When the interpreter create the template from the loader declaration, 
   checks if the names are unique in the function. This means that 
   the parameter names cannot be the same as a global variable name. If the interpreter
   found variables declared with the same names then it throws an error. We do not
   allow same names , because the functions can access all the variables that are being declared 
   in the main script body, and if the parameters have the same names with the global variables
   its sure that some bugs would be created in the scripts. For example if 
   a function changes the parameter but the programmer meant to change (or use) the
   global variable.
   When the interpreter try to execute the function, first
   checks for the  parameter count. If the parameter number is ok then the interpreter 
   finds the variables in the block that have the same name as the parameters 
   and sets the correct values. Keep in mind that in a function ALL the parameters 
   are passed as a reference. Even the simple and unicode strings or the numeric types,
   pass as references. In actuality the interpreter  make the block variable to point to
   the parameter. After the function ends the block variables that points to parameters would be set to NULL
   
 */
	PDX_STRINGLIST		parameters  ;
	PHDR_BLOCK			code		; /*the code block that the function will execute*/
	PHDR_INTERPRETER	interpreter ; /*the interpreter that will execute the code block.
									    If the function is called from the main thread (not async) then
										the interpreter of the script will stop the execution
										of the current block and it will execute the function code block.
										When the function returns a value this value will set the
										interpreter.ret_var variable.
										If the function creates a new thread to execute itself (async)
										then the returned value is lost
									  */

	PDX_STRING			name		; /*the name of the function*/
	PHDR_OBJECT_CLASS   object      ;/*the object that the function belongs if the function belongs to an object*/

} *PHDR_CUSTOM_FUNCTION;

PHDR_CUSTOM_FUNCTION hdr_custom_function_create(PHDR_INTERPRETER interpreter,char *name)
{
	PHDR_CUSTOM_FUNCTION func = (PHDR_CUSTOM_FUNCTION)malloc(sizeof(struct hdr_custom_function));

	func->code		  = NULL							; /*this MUST be created from the caller. In fact this must be a copy of the actual function code from the loader*/
	func->parameters  = dx_stringlist_create()  		; 
	func->interpreter = interpreter						;
	func->name        = dx_string_createU(NULL,name)	;
	func->object	  = NULL							;
	return func;
}





/**************************************************************************************/
void hdr_explicit_invalidate_vars(PHDR_CUSTOM_FUNCTION detach_func);
/*************************************************************************************/
PHDR_CUSTOM_FUNCTION hdr_custom_function_free(PHDR_CUSTOM_FUNCTION func)
{
	/*
	   free all the memory of the func returns NULL
	   29-08-2024 A problem arise along with the added complexity in object functions in async
	   functions if a syntax error occurs. As the normal program flow is interupted ,
	   there is the posibility the function blocks to not invalidate their variables 
	   as the proper routines will never run. So we will check in this function that is 
	   running universally the state of a variable and we will set its object appropriately 
	 */

	hdr_explicit_invalidate_vars(func) ;

	hdr_block_free(func->code)				;
	dx_stringlist_free(func->parameters)	;

	dx_string_free(func->name)			    ;
	free(func) ;
	return NULL;
}


typedef  PDX_HASH_TABLE PHDR_CUSTOM_FUNCTIONS_LIST ;

PHDR_CUSTOM_FUNCTIONS_LIST hdr_custom_functions_list_create()
{
	return dx_HashCreateTable(HDR_CUSTOM_FUNC_LIST_HASH_BUCKETS);
}

/*TO USED ONLY IN THE CUSTOM FUNCTION LIST DESTRUCTION*/
void hdr_free_custom_functions_list_obj(PDXL_OBJECT obj)
{
	PHDR_CUSTOM_FUNCTION func = obj->obj;
	obj->obj = hdr_custom_function_free(func);
	obj->key = NULL; // the key in the obj is a reference to the variable name so we set it to NULL
	// the actual PDXL_OBJECT will be freed in the destruction of the list (hash table)
	return;
}

/*Free the list and all the functions in it, returns NULL*/
PHDR_CUSTOM_FUNCTIONS_LIST hdr_custom_functions_list_free(PHDR_CUSTOM_FUNCTIONS_LIST list)
{
	dx_HashDestroyTable(list, hdr_free_custom_functions_list_obj);
	return NULL ;
}

PHDR_CUSTOM_FUNCTION hdr_custom_functions_list_add(PHDR_CUSTOM_FUNCTIONS_LIST list, PHDR_CUSTOM_FUNCTION func)
{

	if ((list == NULL) || (func == NULL)) return func ;
	PDXL_OBJECT obj = dxl_object_create();
	obj->key = func->name; /*set the hash key as the variable name , this is a reference so beware of freeing usefull memory*/
	obj->obj = func;
	/*add the object in the list*/
	dx_HashInsertObject(list, obj);
	return func;
}


/**********************************************************************************************************/

/*
 Objects
 Hydra has some limited support for objects for better organization for bigger projects.
 An object is a simple collection of variables and custom functions.
 An object class can be declared in the script and then a script can create
 as many instances it needs. An object instance put some burden in the memory
 as it will be a copy of all the functions code and it creates a new variable list for the new instance.
 An object function is executed by the current interpreter like a normal
 custom function and it can return a value as usually.
 An object function cannot run [async] as the Hydra cannot determine if the object
 will not used from the main thread in the same time .
 An object function can use the global variables.
 An object function can use the objects members (variables) by name.
 An object function can use the variables and the functions of an object
 as they had being declared in the local scope 
 In the local block of an object function is forbidden to declare a variable
 with the same name as one of the variable members of the object.
 The objects can be dynamically created in the script, for releasing the memory when is not needed
 a special function is added in all objects instances : $obj_instance.free();
 The free() function releases all the memory that the object occupy

 Remember : The object functions cannot be executed as async

 The objects instances are stored in the code blocks that are declared and the
 scope rules are the same as the variables

 Basic Syntax

 obj myobj
 {
   $myint = integer(0) ;
   $mynum = 18.3		  ;
   $list  = List.Create(simple) ;

   func func1(param1)
   {
     write_console(param1+"hello"+char(13)+char(10)) ;
   }

   func func2()
   {
     echo($myint);
   }

 }

 [*declare an instance with the use of the Class domain*]

 $some_object = Class.myobj ; # this will create a new instance of the myobj

 $some_object.myint = 10  ;
 echo($some_object.myint) ;
 $some_object.func1("molly ") ;
 $some_object.free() ; #release the memory


*/


struct hdr_object_class
{
	PDX_STRING					class_name  ; /*The class name , this will be used for creating the appropriate instance*/
	PHDR_BLOCK				    code        ; /*The variable members that the object has AND its initialization*/
	PHDR_CUSTOM_FUNCTIONS_LIST  functions   ; /*The functions of the class*/
};

typedef PHDR_OBJECT_CLASS PHDR_OBJECT_INSTANCE ;

PHDR_OBJECT_CLASS hdr_object_class_create(char* classname)
{
	PHDR_OBJECT_CLASS obj = (PHDR_OBJECT_CLASS)malloc(sizeof(struct hdr_object_class));
	obj->class_name = dx_string_createU(NULL,classname)  ;
	obj->code       = NULL			 ; /*this must be set from the caller*/
	obj->functions  = hdr_custom_functions_list_create() ;
	return obj;
}

PHDR_OBJECT_CLASS hdr_object_class_free(PHDR_OBJECT_CLASS obj)
{
	/*release all the memory of the object or the object class  and returns NULL*/
	if(obj == NULL) return NULL ;
	dx_string_free(obj->class_name)   ;
	hdr_block_free(obj->code) ;
	hdr_custom_functions_list_free(obj->functions) ;
	free(obj);
	return NULL;
}


typedef PDX_HASH_TABLE PHDR_OBJECT_CLASS_LIST;

PHDR_OBJECT_CLASS_LIST hdr_object_class_list_create()
{
	return dx_HashCreateTable(HDR_OBJ_CLASS_HASH_BUCKETS);
}

/*TO USED ONLY IN THE OBJECT LIST DESTRUCTION*/
void hdr_object_list_free_obj(PDXL_OBJECT obj)
{
	PHDR_OBJECT_CLASS object = obj->obj;
	obj->obj = hdr_object_class_free(object) ;
	obj->key = NULL; // the key in the obj is a reference to the variable name so we set it to NULL
	// the actual PDXL_OBJECT will be freed in the destruction of the list (hash table)
	return;
}

PHDR_OBJECT_CLASS_LIST hdr_object_class_list_free(PHDR_OBJECT_CLASS_LIST list)
{
	dx_HashDestroyTable(list, hdr_object_list_free_obj);
	return NULL;
}

PHDR_OBJECT_CLASS hdr_object_class_list_add(PHDR_OBJECT_CLASS_LIST list, PHDR_OBJECT_CLASS obj_class)
{

	if ((list == NULL) || (obj_class == NULL)) return NULL ;
	PDXL_OBJECT obj = dxl_object_create();
	obj->key = obj_class->class_name; /*set the hash key as the variable name , this is a reference so beware of freeing usefull memory*/
	obj->obj = obj_class;
	/*add the object in the list*/
	dx_HashInsertObject(list, obj);
	return obj_class;
}

PHDR_OBJECT_CLASS hdr_object_class_list_find(PHDR_OBJECT_CLASS_LIST list, PDX_STRING name)
{
	PDXL_OBJECT obj = dx_HashReturnItem(list,name,true) ;
	if(obj == NULL) return NULL ;
	return (PHDR_OBJECT_CLASS)obj->obj ;
}





/**********************************************************************************************************/

/*
 * The main entity for the loader is the block. A block stores all the instructions and variables 
 * in an easy to access form for the interpreter.
*/

struct hdr_block
{
	DXLONG64					ID			   ; /*a global id for identification debuging purposes*/
	PHDR_BLOCK					parent         ;/*the parent block. If the block is the main script block then this member is NULL*/
	PHDR_VAR_LIST				variables	   ;/*the variables (and literals) that the block holds*/
	PHDR_INSTRUCTIONS_LIST		instructions   ;/*the instructions of the code*/
	/*
	  The following is NULL if the block is not a function block or points to the function template (in instruction form)
	  This is valid only to the original function templates in the loader and is not valid or used while the script is executed. 
	*/
	PHDR_INSTRUCTION        belongs_to_func ;
};

PHDR_BLOCK hdr_block_create(PHDR_BLOCK  parent)
{
	/*creates a new block with all the members created and initialized*/
	PHDR_BLOCK block = (PHDR_BLOCK)malloc(sizeof(struct hdr_block)) ;
	if (block == NULL) return NULL ;

	block->ID			   = _GET_GLOBAL_LOADER_ID()		    ;
	block->parent          = parent								;
	block->variables       = hdr_var_list_create(block)			;
	block->instructions    = hdr_instructions_list_create()		;
	block->belongs_to_func = NULL								;
	return block;
}

PHDR_BLOCK hdr_block_free(PHDR_BLOCK block)
{
	if (block == NULL) return NULL;
	/*release all the memory of the block returns NULL*/
	hdr_var_list_free(block->variables)               ;
	hdr_instructions_list_free(block->instructions)   ;
	free(block)	;
	return NULL ;
}

PHDR_BLOCK hdr_block_copy(PHDR_BLOCK block,PHDR_BLOCK parent)
{
	if(block == NULL) return NULL ;
    /*make an identical copy of the block and returned it*/
	PHDR_BLOCK nblock = hdr_block_create(parent) ;
	/*copy the variables*/
	hdr_var_list_copy(nblock->variables , block->variables) ;
	/*copy the instructions*/
    hdr_instructions_list_copy(nblock->instructions,block->instructions,nblock);

    return nblock ;
}


/*************************/
PHDR_VAR hdr_var_list_find_var(PHDR_VAR_LIST list, PDX_STRING var_name)
{
	/*
	 returns the variable with this name IF exists.
	 else returns NULL.
	 The function check all the nested block hierarchy
	*/
	PHDR_BLOCK   current_block = list->block;
	while (current_block != NULL)
	{
		PHDR_VAR_LIST current_list = current_block->variables;
		PDXL_OBJECT obj = dx_HashReturnItem(current_list->list, var_name,true);
		if (obj == NULL) goto check_parent ;
		PHDR_VAR    var = (PHDR_VAR)obj->obj;
		if (var != NULL) return var;

		/*check for parent*/
		check_parent :
		current_block = current_block->parent;
	}

	return NULL;
}

bool hdr_var_list_find_var_scope(PHDR_VAR_LIST list, PDX_STRING var_name , bool global)
{
	/*
	 returns the variable with this name IF exists.
	 else returns NULL.
	 The function check all the nested block hierarchy
	 if the global is true.
	 If the global is false the  function will check all the block hierarchy
	 EXCEPT the main script block.

	 2024-08-29 The function will check the block if its a function block (only the top block in a function declaration can be a function block).
	 If it is the function will check the parameters of the function. If a parameter in the hierarchy of the blocks
	 has the same name with a parameter, the function will return true

	*/
	PHDR_BLOCK   current_block = list->block;
	while (current_block != NULL)
	{
		PHDR_VAR_LIST current_list = current_block->variables;
		PDXL_OBJECT obj = dx_HashReturnItem(current_list->list, var_name,true);
		if (obj == NULL) 
		{
			/*check if the block is a function block*/
			if(current_block->belongs_to_func != NULL )
			{
			  /*check the parameters of the func against the var_name*/
		      PHDR_INSTRUCTION instr =  current_block->belongs_to_func ;
			  PDXL_NODE        node = instr->left_side->parameters->start ;
			  while(node!=NULL)
			  {
				  PHDR_EXPRESSION expr = node->object->obj ;
				  if( dx_string_native_compare(expr->value->name,var_name)==dx_equal) return true ;
			  
				  node = node->right ;
			  }
			}
			/*check the parent we found nothing*/
			goto check_parent ;
		}
		PHDR_VAR    var = (PHDR_VAR)obj->obj;
		if (var != NULL) return true;

		/*check for parent*/
	    check_parent :
		if((global == false)&&(current_block->parent != NULL)) 
		{
		  /*do not check main block*/
		  if (current_block->parent->ID == 1) return false ;	
		};
		current_block = current_block->parent;
	}

	return false;
}

PHDR_VAR hdr_var_list_find_var_striped(PHDR_VAR_LIST list, PDX_STRING var_name)
{
	/*
	 returns the variable with this name IF exists.
	 else returns NULL.
	 The function check all the nested block hierarchy.
	 The var_name must be striped from the $ prefix
	*/
	PDX_STRING tmpstr1 = dx_string_createU(NULL,"$") ;
	PDX_STRING tmpstr2 = dx_string_concat(tmpstr1,var_name) ;
	PHDR_BLOCK   current_block = list->block;
	while (current_block != NULL)
	{
		PHDR_VAR_LIST current_list = current_block->variables;
		PDXL_OBJECT obj = dx_HashReturnItem(current_list->list, tmpstr2,true);
		if (obj == NULL) goto check_parent ;
		PHDR_VAR    var = (PHDR_VAR)obj->obj;
		if (var != NULL) 
		{
			dx_string_free(tmpstr1);
			dx_string_free(tmpstr2);
			return var;
		}
		/*check for parent*/
		check_parent :
		current_block = current_block->parent;
	}

	dx_string_free(tmpstr1);
	dx_string_free(tmpstr2);
	return NULL;
}

/*****************************************************************/

/************************************************************************************************************/

/*Defines*/
enum hdr_define_type {hdr_def_string,hdr_def_int,hdr_def_dbl};
typedef PHDR_VAR PHDR_DEFINE ;


PHDR_DEFINE hdr_define_create(char* name, DXLONG64 lng, double dbl, char* str, enum hdr_define_type def_type)
{
	PHDR_DEFINE def = (PHDR_DEFINE)malloc(sizeof(struct hdr_var)) ;
	def = hdr_var_create(NULL, name, hvf_block,NULL) ;
	switch (def_type)
	{
	case hdr_def_string	: def->obj = dx_string_createU(NULL,str) ; def->type = hvt_simple_string ;
		break;
	case hdr_def_int	: def->integer = lng				; def->type = hvt_integer       ;
		break;
	case hdr_def_dbl	: def->real = dbl					; def->type = hvt_float          ;
		break;
	}

	return def;
}


PHDR_DEFINE hdr_define_free(PHDR_DEFINE def)
{
	hdr_var_free(def);
	return NULL;
}


typedef PHDR_VAR_LIST PHDR_DEFINE_LIST;

PHDR_DEFINE_LIST hdr_define_list_create()
{
	return hdr_var_list_create(NULL);
}

PHDR_DEFINE_LIST hdr_define_list_free(PHDR_DEFINE_LIST list)
{
	hdr_var_list_free(list) ;
	return NULL;
}

PHDR_DEFINE hdr_define_list_add(PHDR_DEFINE_LIST list , PHDR_DEFINE def)
{
	hdr_var_list_add(list, def);
	return def;
}


/************************************************************************************************************/
/*
 Threads
 Hydra is monitoring all the threads that are created by the script.
 A new thread is created by one of the following ways :

 [async "some_string" @ function()]  : The [async] keyword before a function forces the Hydra+ to create a
		    new thread and in this thread a new function instance and a new interpreter will be
		    created. When the function returns all the memory will be freed. The some_string
			will be set as the thread id.


Hydra+ has some special functions to handle threads :

message some_string			  :  The thread will set a message that the main script can access for info

terminate "id"   			  :  Terminates forcefully the thread with this ID. If the id exist more than once then the first
								 thread that we will find will be terminate

threadInfo()			      : The function returns the Threads structure with all the threads
								of the script. If a thread has been terminated then it will be removed from the list
								but it will be removed when the slot is needed.
getThreadId()				  : The function return the id of the current thread. If the function is called
								from the main thread the string "Main" is returned 


Example :
$thread = threadInfo("thread_id");
$thread.running ; (true or false , Hydra does not immediatelly remove the thread from the table after termination)
$thread.message ; (a specific message that the function can set with the setThreadMessage "some message")
$thread.error ; (this is true if the async function returned from an error or from a terminate command)

*/

/************************************************************************************************************/


typedef struct hdr_thread
{
	PDX_STRING  id		;
	bool		running	;
	bool		terminate ; /*the interpreter can set this to stop a threads execution*/
	bool		stop_with_error ; /*this is true if the interpreter of the thread terminated with error*/
	PDX_STRING  message ;
	struct cthreads_thread  *cthread ;
	struct cthreads_args	*cargs	 ;

} *PHDR_THREAD;

PHDR_THREAD hdr_thread_create(PDX_STRING thread_id)
{
	PHDR_THREAD thread = (PHDR_THREAD)malloc(sizeof(struct hdr_thread));
	thread->id = dx_string_createU(NULL, thread_id->stringa) ;
	thread->message = dx_string_createU(NULL, "") ;
	thread->running = false						  ;
	thread->terminate = false				      ;
	thread->stop_with_error = false				  ;
	thread->cthread			= NULL				  ;
	thread->cargs			= NULL				  ;
	return thread;
}

PHDR_THREAD hdr_thread_free(PHDR_THREAD thread)
{
	if(thread == NULL) return NULL ;
	/*Free all the memory and return NULL*/
	dx_string_free(thread->id)      ;
	dx_string_free(thread->message) ;
	free(thread)					;
	return NULL;
}


/*the thread list will be a special structure with a static array to store the threads*/

typedef struct hdr_thread_list
{
	PHDR_THREAD threads[HYDRA_MAXIMUM_THREADS] ; /*static table*/
	DXLONG64 count  ; 
} *PHDR_THREAD_LIST;

PHDR_THREAD_LIST hdr_thread_list_create()
{
	PHDR_THREAD_LIST tlist = malloc(sizeof(struct hdr_thread_list)) ;
	tlist->count = 0 ;
	for(int i=0 ; i < HYDRA_MAXIMUM_THREADS; i++) tlist->threads[i] = NULL ;

	return tlist ;
}

PHDR_THREAD_LIST hdr_thread_list_free(PHDR_THREAD_LIST list)
{
	for(int i = 0;i<HYDRA_MAXIMUM_THREADS ; i++)
	{
	  list->threads[i] = hdr_thread_free(list->threads[i]) ;
	}

	free(list) ;
	return NULL ;
}



PHDR_THREAD *hdr_thread_list_add_thread(PHDR_THREAD_LIST list , PHDR_THREAD thread)
{
  if(list->count == HYDRA_MAXIMUM_THREADS) return false ;
  /*find the next free slot*/
  PHDR_THREAD * free_pos = NULL ; 
  int i =0 ;
  for(int i = 0;i<HYDRA_MAXIMUM_THREADS;i++)
  {
	  if(list->threads[i]==NULL) /*if the position is NULL */ 
	  {
		  free_pos = &(list->threads[i]) ;
		  break ;
	  }
  }

  if(free_pos == NULL) return NULL ;
  *free_pos = thread ; 
  (list->count)++ ;
  return free_pos ;
}

bool hdr_thread_list_remove_thread(PHDR_THREAD_LIST list,PDX_STRING thread_id)
{
	/*find the thread*/
	for(int i=0; i<HYDRA_MAXIMUM_THREADS;i++)
	{
	  if(dx_string_native_compare(list->threads[i]->id,thread_id) == dx_equal)
	  {
	    PHDR_THREAD thread = list->threads[i] ;
		list->threads[i]   = hdr_thread_free(thread) ;
		(list->count)-- ;
		return true ;
	  }
	}

	return false ;
}

PHDR_THREAD hdr_thread_list_return_thread(PHDR_THREAD_LIST list,PDX_STRING thread_id)
{
	/*find the thread*/
	for(int i=0; i<HYDRA_MAXIMUM_THREADS;i++)
	{
      if(list->threads[i]!= NULL)
	  if(dx_string_native_compare(list->threads[i]->id,thread_id) == dx_equal)
	  {
	    PHDR_THREAD thread = list->threads[i] ;
		return thread ;
	  }
	}

	return NULL ;
}



/**************************** Utility Functions **********************/

char* hdr_error_code_to_str(int error_code)
{

	switch (error_code)
	{
	case HDR_ERROR_LOADER_INCLUDE_NO_FILENAME:	return "The 'include' directive needs a file name in string format. e.g include `script.hydra`";
		break;
	case HDR_ERROR_LOADER_NOT_TERMINATED:		return "The instruction is not properly terminated with a ';'";
		break;
	case HDR_ERROR_LOADER_STRING_MISMATCH:      return "The instruction open a string with a '`' or '\"' but it does not close it";
		break;
	case HDR_ERROR_LOADER_STRING_STRAY:			return "A stray '`' or '\"' found in the instruction";
		break;
	case HDR_ERROR_LOADER_IS_NULL:				return "Internal error. Memory allocation failed for loader object";
		break;
	case HDR_ERROR_INCL_SCRIPT_IS_NULL:			return "Internal error. An included script structure passed as NULL. (Memory allocation failed?)";
		break;
	case HDR_ERROR_LOADER_EXPR_MISSING:         return "Internal error. In a state that the loader expected to find an expression, an empty entity was returned";
		break;
	case HDR_ERROR_LOADER_TOKEN_IS_NULL:        return "Internal error. A token retrieved from an entity is NULL";
		break;
	case HDR_ERROR_LOADER_NEW_INSTR_WHILE_PENDING:	return "Syntax error. The loader found the start of a new instruction before the previous one was completed";
		break;
	case HDR_ERROR_LOADER_SYNTAX_ERROR:				return "Syntax error. Malformed instruction.The loader will print a detailed error description";
		break;
	case HDR_ERROR_LOADER_INTERNAL_ERROR:			return "Internal error. A bug or a similar problem produced this error !";
		break;
	case 4000000000:								return "Internal error. A function , object or variable was not inserted in the list as it should";
		break;
	case HDR_ERROR_SYNTAX_ERROR:					return "Syntax error. Something is not as it should";
		break;
	case HDR_ERROR_TOKEN_NAME_ERROR:				return "Syntax error. A token (variable, function etc) does not have a valid name";
		break;
	case HDR_ERROR_TOKEN_NULL_P:					return "Internal error. Failed to allocate the memory for the token creation";
		break;
	case HDR_ERROR_SECTION_MISSMATCH:				return "Syntax error. Missmatch of opening / close character for the section";
		break;
	case COPY_SECTION_STRAY_OPEN:					return "Syntax error. Extra open characters was found in the section";
		break;
	case COPY_SECTION_STRAY_CLOSE :					return "Syntax error. Extra close characters was found in the section";
		break;
	case COPY_SECTION_EMPTY:						return "Syntax error. The returned expression from the section is empty";
		break;
	case COPY_SECTION_NULL_P:						return "Internal error. The expression from the section returned a NULL value.";
		break;
	case HDR_ERROR_LITERAL_AS_PART:				    return "Syntax Error. A multipart token cannot have as a part a literal component (e.g. '$var.8').";
		break;
	case HDR_ERROR_NAME_NULL:				        return "Syntax Error. A name expected but NULL returned.";
		break;	
	case HDR_ERROR_NAME_EMPTY:				        return "Syntax Error. A name expected but empty string returned.";
		break;
	case HDR_ERROR_EXPR_STR_EMPTY:				    return "Syntax Error. An expression expected but empty string returned.";
		break;
	case HDR_ERROR_SEPARATOR_DOT_ :					return "Syntax Error. Missplaced dot [.].";
		break;
	case HDR_ERROR_EXPR_STR_NULL:				    return "Syntax Error. An expression expected but NULL returned.";
		break;
	case ALG_EXPR_NULL:								return "Internal Error. The algebraic expression is NULL.";
		break;
	case ALG_EXPR_EMPTY:							return "Syntax Error. The algebraic expression is empty.";
		break;
	case ALG_TOO_LONG_OPERANT:						return "Syntax Error. An operand in the expression is too long (on : Algebraic expression).";
		break;
	case ALG_LIST_NULL:								return "Internal Error. The list returned as NULL (on : Algebraic expression).";
		break;
	case ALG_MISSPLACED_OPERATOR:					return "Syntax Error. Missplaced operator in the expression (on : Algebraic expression).";
		break;
	case ALG_SYNTAX_ERROR:							return "Syntax Error. The expression is malformed (on : Algebraic expression).";
		break;
	case ALG_OBJ_NULL:								return "Internal Error. The object returned as NULL (on : Algebraic expression).";
		break;
	case ALG_MEM_ERROR:								return "Internal Error. malloc() failed. (on : Algebraic expression).";
		break;
	case ALG_PART_MUST_EMPTY:						return "Syntax Error. Malformed expression. Illegal token in front of a '()' (on : Algebraic expression).";
		break;
	case ALG_SECTION_INVALID:						return "Syntax Error. Malformed expression. Some parenthesis are not correctly paired (on : Algebraic expression).";
		break;
	case ALG_NODES_NOT_MERGED:						return "Internal Error. Nodes where not merged (on : Algebraic expression)";
		break;
	case ALG_BRACKETS_INVALID:						return "Syntax Error. Some brackets are not correctly paired (on : Algebraic expression).";
		break;
	case HDR_ERROR_EMPTY_INDEX_BEFORE_CURRENT:		return "Syntax Error. Another index cannot follow after an empty index. E.g '$var[][$indx]'.";
		break;
	case HDR_ERROR_TOKEN_TYPE_FOR_PART_ROOT:		return "Syntax Error. The token or entity type cannot be a token for a multipart token.";
		break;
	case HDR_ERROR_TOKEN_FUNCTION_CANNOT_HAVE_PART: return "Syntax Error. A function cannot have a part. E.g '$var.func1($param).some_part'.";
		break;
	case HDR_ERROR_STRAY_CHARS_BEFORE_INDEX:		return "Syntax Error. A complex index to a token cannot have any characters prefixed. E.g '$var[12]err[10]'.";
		break;
	case HDR_ERROR_BRACKET_AFTER_DOT:				return "Syntax Error. Is forbidden a bracket '[' to follow a dot '.' in a multipart token.";
		break;
	case HDR_SCRIPT_END:							return "Syntax Error. The code block was finished abruptly. Check your syntax. Especially in a switch you have to check your ';'";
		break;
	}

	return "The error code is not valid.";
}




/******************************* INVALIDATE VARS SYSTEM *******************************/

void hdr_explicit_invalidate_var_list(PHDR_VAR_LIST var_list)
{
    PDX_LIST      *buckets = var_list->list->buckets      ;
	for(uint32_t i = 0; i < var_list->list->length;i++)
	{
		
		PDXL_NODE node = var_list->list->buckets[i]->start ;
		while(node != NULL)
		{
			PDXL_OBJECT obj = node->object ;
			PHDR_VAR var =(PHDR_VAR) obj->obj ;
			if(var != NULL)
			{
				if((var->type == hvt_simple_string)||(var->type == hvt_simple_string_bcktck))
					{
					  /*this will be handled in the function destruction*/
					}
					else
						{
							hdr_var_release_obj(var) ;
						}
			}
			node = node->right ;
		}
		
	}


}


/******************* EXPLICIT INVALIDATION SYSTEM *****************************/
void hdr_explicit_instruction_invalidate(PHDR_INSTRUCTION instr)
{
  if(instr == NULL) return ;
  if(instr->code != NULL) 
  {
	  hdr_explicit_invalidate_var_list(instr->code->variables) ;
  
	  /*now invalidate all the instructions of the block*/
	  PDXL_NODE node = instr->code->instructions->start ;
	  while(node != NULL)
	  {
		PHDR_INSTRUCTION tinstr = (PHDR_INSTRUCTION)node->object->obj ;
		hdr_explicit_instruction_invalidate(tinstr) ;
		node = node->right ;
	  }
	  }
	  /*invalidate else*/
	  hdr_explicit_instruction_invalidate(instr->ielse)        ;
}


void hdr_explicit_invalidate_vars(PHDR_CUSTOM_FUNCTION detach_func)
{
	/*invalidate the top blocks variables*/
	hdr_explicit_invalidate_var_list(detach_func->code->variables) ;
	/*invalidate all the instructions blocks*/
	PDXL_NODE node = detach_func->code->instructions->start ;
	while(node != NULL)
	{
		PHDR_INSTRUCTION instr = (PHDR_INSTRUCTION)node->object->obj ;
		hdr_explicit_instruction_invalidate(instr) ;
		node =node->right ;
	}
	
}

/********************************************************************/



