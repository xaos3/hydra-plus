/*
 Deus Ex 26-12-2023 deus-ex.gr
 This module has a unified object model for
 communication with different databases.
 Supported databases :
 Sqlite (native , the amagalmation file is compiled in the module support ONLY utf8)
 MariaDb / Mysql
 SQL Server (via ODBC. Supports only utf8 when fetching data.
             The binary data can be true binary or in text form in hexadecimal., see the blob_as_string flag,
             The status messages are  coded as utf8)
 Generic odbc support
*/

#define DXDATABASES
/*include for compile sqlite support*/

#ifndef UNICODE
#define UNICODE
#endif

/*sqlite*/
#include "thirdparty/sqllite/sqlite3.c"
/*ODBC*/
#include <sql.h>
#include <sqlext.h>
#include <odbc_port.h>
/*mariadb*/
#include <mysql.h>


#define DX_DB_SUCCESS 0
#define DX_DB_GENERAL_ERROR -10000
#define DX_FIELD_VALUE_NULL  0
#define DX_FIELD_VALUE_INT   1
#define DX_FIELD_VALUE_FLOAT 2
#define DX_FIELD_VALUE_TEXT  3
#define DX_FIELD_VALUE_BLOB  4
#define DX_FIELD_VALUE_DATE  5

typedef struct dx_sqlite
/*#
{
  sqlite3      *db   ;
  int          error_code ;

}*PDX_SQLITE ;
#*/
{
  sqlite3      *db        ;
  int          error_code ;
  PDX_STRING   error      ;

}*PDX_SQLITE ;

typedef PDX_LIST PDX_ROW       ;
/*#
 This will serve as the row of the dataset
#*/
typedef PDX_LIST PDX_HEADER    ;
/*#
 This will serve as the header of the dataset
#*/

typedef PDX_LIST PDX_DATASET   ;
/*#
 This will serve as the dataset
#*/

typedef PDXL_OBJECT PDX_FIELD  ;
/*#
 This will serve as a data field,
 the -> key       will have the varchar or text data in utf8 form,
 the -> int_key   will have the integer data
 the -> float_key will have the real types data
 the -> obj       will have the blob and other binary types
 the -> flags     will have the type of the field e.g TEXT or BLOB

#*/

typedef PDXL_OBJECT PDX_COLUMN ;
/*
 This will serve as a column specification,
 the -> key       will have the field name as utf8
 the -> obj       will have the type as a utf8
*/
typedef struct dx_query
/*#
{
  PDX_HEADER header ;
  PDX_LIST dataset  ; // a list of PDX_ROWS
  DXLONG64 row_count ;
} *PDX_QUERY ;

A Query is just a collection of all the rows
that we retrieved from the database. The fields
can be a utf8 text , a float value , an integer value
or a binary value.


#*/
{
  PDX_HEADER header  ;
  PDX_LIST dataset   ; // a list of PDX_ROWS
  DXLONG64 row_count ;
} *PDX_QUERY ;


/*typedef struct dx_database
{
  enum dx_data_types type ;
  PDX_STRING host         ;
  PDX_STRING database     ;
  PDX_STRING user         ;
  PDX_STRING password     ;
  bool connected          ;

}*PDX_DATABASE ;*/


/*
 for Hydra+ a wraper class to help with the indexing in the interpreter
*/

typedef struct dx_db_row_wrap
{
    PDX_QUERY query ;
    PDX_ROW   row   ;
} *PDX_DB_ROW_WRAP;

PDX_DB_ROW_WRAP dx_db_row_wrap_create()
{
 return (PDX_DB_ROW_WRAP) malloc(sizeof(struct dx_db_row_wrap));
}
PDX_DB_ROW_WRAP dx_db_row_wrap_free(PDX_DB_ROW_WRAP row_wrap)
{
    free(row_wrap) ;
    return NULL ;
}

/*********************************/


/* Functions        ******************************/

/* SQLITE ******************************************/

PDX_SQLITE dx_sqlite_init() ;
/*#
 Initialize the sqlite object
#*/
bool dx_sqlite_connect(PDX_STRING database , PDX_SQLITE sqlite );
/*#
 The function try to connect to an sqlite database and return true if the connection
 is successful or false if it is not.
 If the database does not exists then the function will create it.
#*/

bool dx_sqlite_close(PDX_SQLITE sqlite);
/*#
 Closes and finalize the connection, it release all the memory of the object
#*/

bool dx_sqlite_execute(PDX_SQLITE sqlite , PDX_STRING query);
/*#
 Executes a statement that does not return any data.
 If the function fail then false is returned and the
 sqlite->error has the error message
#*/
PDX_QUERY dx_sqlite_select(PDX_SQLITE sqlite , PDX_STRING query);
/*#
 Executes a statement and returns the dataset.
#*/


/* ************************************************* */


/* ODBC ******************************************/

#define ODBC_BUFFER_LEN 4096

typedef struct dx_odbc
{
  PDX_STRING dsn     ; /*the connection string*/
  PDX_STRING driver  ; /*the driver name for the ODBC*/
  SQLHDBC    dbc     ; /*the odbc handle */
  SQLHENV    env     ; /*the enviroment handle */
  bool       blob_as_string ; /*if true the blobs will be retrieved as strings (probably as hexadecimal representation)*/
} *PDX_ODBC ;


bool dx_odbc_succeeded(SQLRETURN ret,PDX_ODBC odbc,PDX_STRING status,SQLSMALLINT type) ;
/*#
 Retrieving the status of the last action , the status will be set with the
 string description of the status.
 returns false in failure and true in success.
 The function is returning true in SUCCESS and in SUCCESS_WITH_INFO
 so better check the status
#*/

PDX_ODBC dx_odbc_init(PDX_STRING conn_string,PDX_STRING status,bool blob_as_string) ;
/*#
 Initialize the odbc dbc object if an error occurs
 then NULL is returned and the status will have the error message
#*/

void dx_odbc_free(PDX_ODBC odbc);
/*#
 Closes and finalize the connection, it release all the memory of the object
#*/

PDX_QUERY dx_odbc_execute(PDX_ODBC odbc , PDX_STRING query,PDX_STRING status);
/*#
 Executes a statement and returns the dataset. If the query does not
 return a result set then the function returns NULL.
 The status is empty in success and is set to the specific error
 in failure.

 The query has to be in widestring as in windows the unicode define
 forces use the functions of the (W) variety

#*/


/* ************************************************* */

/* ********************MY SQL ********************** */

MYSQL * dx_mariadb_connect(PDX_STRING host,PDX_STRING user,PDX_STRING passwd,PDX_STRING db,
                         unsigned int port,PDX_STRING unix_socket,unsigned long flags,PDX_STRING charset,PDX_STRING status) ;
/*#
 Initialize and connect to a mariadb/mysql database  , if the function fails
 then NULL is returned and the status has the actual error message
#*/

PDX_QUERY dx_mariadb_execute(MYSQL * conn ,PDX_STRING query,PDX_STRING status);
/*#
 Executes a query and returns the dataset IF the query returns data.
 The status will be empty if all its good , else the status will be set
 with an error message.
#*/

void dx_mariadb_disconnect(MYSQL * conn);
/*#
 Terminate the connections
#*/


/* ************************************************ */


/* ***************** GENERAL DATABASE CODE *************/

typedef struct dx_db_blob
{
  DXLONG64 count ;
  char *data     ;
} *PDX_DB_BLOB ;

PDX_DATASET dx_db_dataset_create() ;
/*#
Creates a new PDX_LIST item that we will use as the dataset
#*/

PDX_DATASET dx_db_dataset_free(PDX_DATASET dataset) ;
/*#
 Deallocates the memory of a dataset
#*/

PDX_ROW     dx_db_row_create()                  ;
/*#
 Creates a new PDX_LIST item that we will use as a row in a dataset
#*/
PDX_ROW     dx_db_row_free(PDX_ROW row)         ;
/*#
 Deallocates the memory of a row
#*/
PDX_HEADER    dx_db_header_create()               ;
/*#
 Creates a new PDX_LIST item that we will use as a header in the dataset (optically the first row but internally is not in the same list)
#*/
PDX_HEADER    dx_db_header_free(PDX_HEADER header)   ;
/*#
 Deallocates the memory of a header
#*/
PDX_COLUMN  dx_db_column_create(PDX_STRING name , PDX_STRING type);
/*#
  Create a new column to insert in the header
#*/
PDX_COLUMN  dx_db_column_free(PDX_FIELD column) ;
/*#
 Deallocates the memory of the column
#*/
PDX_FIELD  dx_db_field_create()                ;
/*#
 Allocates the memory for the PDX_FIELD ,
 you have to set the values manually after creation
 for the text field the [key] is used
#*/
PDX_FIELD  dx_db_field_free(PDX_FIELD field)   ;
/*#
 Deallocates the memory of the field
#*/


PDX_QUERY dx_db_query_create();
/*#
Creates a new query object
#*/

PDX_ROW dx_db_query_row_from_node(PDXL_NODE node);
/*#
The function gets as parameter a node of the
query.dataset and returns the PDX_ROW that reside
in the Node. This function is for convinience
as the row resides in node->object->obj
#*/

PDX_QUERY dx_db_query_free(PDX_QUERY query);
/*#
 Deallocates all the memory of the query and its data
#*/

/************* Convenience Functions *****************/

PDX_STRING dx_db_field_as_string(PDX_FIELD field,int float_digit);
/*#
 The function return the field as a string.
 The BLOB type is returned as "[BLOB]" string
 and not as a value.
 The NULL value is returned as "NULL"
#*/

PDX_STRING dx_db_field_name(PDX_QUERY dataset , DXLONG64 indx);
/*#
 The function return the field name from the header of the dataset 
 in this index or NULL in error
#*/

DXLONG64 dx_db_find_field_pos(PDX_QUERY query,PDX_STRING fieldName);
/*#
 The function return the column position of the field in the row.
 The returned value is the index that can be used to retrieve the field.
 If the field does not exists -1 is returned
#*/


/* IMPLEMENTATION ********************************/

PDX_SQLITE dx_sqlite_init()
{

  PDX_SQLITE sqlite = (PDX_SQLITE)malloc(sizeof(struct dx_sqlite)) ;
  if(sqlite == NULL) return NULL ;
  sqlite->db          = NULL ;
  sqlite->error_code  = DX_DB_SUCCESS    ;
  sqlite->error       = dx_string_createU(NULL, "") ;
  return sqlite ;
}


bool dx_sqlite_connect(PDX_STRING database , PDX_SQLITE sqlite )
{
  if(database->type == dx_utf8)
  {
    sqlite->error_code = sqlite3_open(database->stringa ,&sqlite->db);
    if (sqlite->error_code != SQLITE_OK)
    {
        const char * err = sqlite3_errmsg(sqlite->db) ;
        dx_string_createU(sqlite->error,(char*)err)    ;
        sqlite3_close(sqlite->db);
        return false ;
    } else return true ;
  } else
     if(database->type == dx_wide)
     {

        sqlite->error_code = sqlite3_open16((void *)database->stringw ,&sqlite->db);
        if (sqlite->error_code != SQLITE_OK)
        {
            const char * err = sqlite3_errmsg(sqlite->db) ;
            dx_string_createU(sqlite->error,(char*)err);
            sqlite3_close(sqlite->db);
            return false;
        } else return true ;
     }
     else
         {
           dx_string_createU(sqlite->error,"The [database] name string must be in utf-8 OR utf-16");
           return false ;
         }

    return false ;
}


bool dx_sqlite_close(PDX_SQLITE sqlite)
{
    sqlite->error_code = sqlite3_close(sqlite->db) ;
    if (sqlite->error_code != SQLITE_OK)
    {
         const char * err = sqlite3_errmsg(sqlite->db) ;
         printf("Database connection closing error : %s\n", (char*)err);
         dx_string_free(sqlite->error)  ;
         free(sqlite)                   ;
         return false                   ;
    }

    dx_string_free(sqlite->error)  ;
    free(sqlite)                   ;

    return true ;

}

bool dx_sqlite_execute(PDX_SQLITE sqlite , PDX_STRING query)
{

   if (query->type != dx_utf8)
   {
      dx_string_createU(sqlite->error,"The execute function accept only utf8 encoded queries");
      return false;
   }
   /* Execute SQL statement */
   char *err = NULL ;
   sqlite->error_code = sqlite3_exec(sqlite->db, query->stringa, NULL, NULL, &err);
   dx_string_createU(sqlite->error,err);
   sqlite3_free(err) ;
   if( sqlite->error_code != SQLITE_OK ) return false ;

   return true ;
}


/*
-----------------Helper functions for dx_sqlite_select ------------------
*/
PDX_STRING dx_sqlite_ctype_to_str(int col_type)
{
    switch(col_type)
    {
       case SQLITE_INTEGER  : return dx_string_createU(NULL,"Integer") ;
                              break ;
       case SQLITE_FLOAT    : return dx_string_createU(NULL,"Float")   ;
                              break ;
       case SQLITE_TEXT     : return dx_string_createU(NULL,"Text")    ;
                              break ;
       case SQLITE_BLOB     : return dx_string_createU(NULL,"Blob")    ;
                              break ;
       case SQLITE_NULL     : return dx_string_createU(NULL,"NULL") ;
                              break ;
    }

   return dx_string_createU(NULL,"Unknown") ;
}


sqlite3_stmt * dx_h_sqlite_prepare_statement(PDX_SQLITE sqlite , PDX_STRING query)
{
	/* prepare the statement*/
   sqlite3_stmt *pStmt = NULL ;
   sqlite->error_code = sqlite3_prepare_v2(sqlite->db,query->stringa,-1,&pStmt,NULL);
   if(sqlite->error_code != SQLITE_OK)
    {
      const char *err   = sqlite3_errstr(sqlite->error_code) ;
      dx_string_createU(sqlite->error,(char*)err);
      return NULL ;
    }

	return pStmt ;
}

void dx_h_sqlite_finalize_statement(PDX_SQLITE sqlite,sqlite3_stmt * pStmt)
{
   sqlite->error_code = sqlite3_finalize(pStmt);
    if(sqlite->error_code != SQLITE_OK)
    {
      const char *err    = sqlite3_errstr(sqlite->error_code) ;
      dx_string_createU(sqlite->error,(char*)err);
    } else
         sqlite->error_code = DX_DB_SUCCESS ;

}


bool dx_h_sqlite_check_step_result(PDX_SQLITE sqlite,PDX_QUERY dquery,sqlite3_stmt * pStmt,int result)
{
  switch(result)
  {
	case SQLITE_BUSY :
	{
	  sqlite->error_code = SQLITE_BUSY ;
	  dx_string_createU(sqlite->error,"Database is locked or already accessed.");
	  sqlite3_finalize(pStmt) ;
	  dx_db_query_free(dquery)   ;
	  return false ;
	} break ;

	case  SQLITE_MISUSE :
	{
	  sqlite->error_code = SQLITE_MISUSE ;
	  dx_string_createU(sqlite->error,"Internal error : SQLITE_MISUSE.");
	  sqlite3_finalize(pStmt) ;
	  dx_db_query_free(dquery)   ;
	  return false ;
	} break ;

	 case  SQLITE_ERROR :
	{
	  sqlite->error_code = SQLITE_ERROR ;
	  dx_string_createU(sqlite->error,(char*)sqlite3_errmsg(sqlite->db));
	  sqlite3_finalize(pStmt) ;
	  dx_db_query_free(dquery)   ;
	  return false ;
	} break ;

  }

  return true ;

}

void dx_h_sqlite_init_header(sqlite3_stmt *pStmt , PDX_QUERY dquery , int *unknown_col)
{
  int ccount = sqlite3_column_count(pStmt);
  for(int i = 0 ; i < ccount ; i++)
  {
	  int ctype_int = sqlite3_column_type(pStmt,i) ;
	  PDX_STRING coltype =  dx_sqlite_ctype_to_str(ctype_int) ;
	  PDX_STRING colname =  dx_string_createU(NULL,(char*)sqlite3_column_name(pStmt, i)) ;
	  PDX_COLUMN col     =  dx_db_column_create(colname,coltype) ;
	  dx_string_free(coltype);
	  dx_string_free(colname);

	  col->flags = ctype_int ; // set the type
	  if(ctype_int == SQLITE_NULL) (*unknown_col)++ ; // there is at least one unknown column
	  dx_list_add_node_direct(dquery->header,col) ;

  }

}

void dx_h_sqlite_check_unknown_header(sqlite3_stmt *pStmt , PDX_QUERY dquery , int *unknown_col)
{

	PDXL_NODE node = dquery->header->start ;
	int i = 0 ;
	while(node != NULL)
	{
		PDX_COLUMN col = node->object ;
		if(col->flags == SQLITE_NULL)
		{
			 int ctype_int = sqlite3_column_type(pStmt,i) ;
			 if(ctype_int != SQLITE_NULL)
			 {
				//ok we found the type !
				unknown_col--  ;
				col->flags = ctype_int ;
				/* set the actual type*/
				PDX_STRING coltype =  dx_sqlite_ctype_to_str(ctype_int) ;
				dx_string_createU((PDX_STRING)col->obj,coltype->stringa);
				dx_string_free(coltype);
			 }
		}

		node = node->right ;
		i++ ;
	}

}

bool dx_h_sqlite_add_row_to_dataset(sqlite3_stmt *pStmt ,PDX_SQLITE sqlite, PDX_QUERY dquery)
{
	PDX_ROW row = dx_db_row_create();

	PDXL_NODE node = dquery->header->start ;
	int indx = 0 ;
	while(node != NULL)
	{
		PDX_COLUMN col   = node->object      ;
		PDX_FIELD  field = dx_db_field_create() ;
		/* handle each type with the appropriate function,
		   use the DX_FIELD_VALUE_NULL and the DX_FIELD_VALUE_SET for determine the NULL value
		*/
		switch(col->flags)
		{
           case SQLITE_INTEGER  :
                                   {
                                       field->flags     =  DX_FIELD_VALUE_INT ;
                                       field->int_key   = (DXLONG64)sqlite3_column_int64(pStmt, indx) ;
                                   }
                                  break ;
           case SQLITE_FLOAT    :  {
                                       field->flags       =  DX_FIELD_VALUE_FLOAT ;
                                       field->float_key   = sqlite3_column_double(pStmt, indx) ;
                                   }
                                  break ;
           case SQLITE_TEXT     :  {
                                       field->flags =  DX_FIELD_VALUE_TEXT ;
                                       field->key   = dx_string_createU(NULL,(char*)sqlite3_column_text(pStmt, indx)) ;
                                   }
                                  break ;
           case SQLITE_BLOB     : {
                                       field->flags     =  DX_FIELD_VALUE_BLOB ;
                                       DXLONG64 bytes_count = sqlite3_column_bytes(pStmt, indx);
                                       PDX_DB_BLOB blob = malloc(sizeof(struct dx_db_blob)) ;
                                       blob->data  = (void*)malloc(bytes_count) ;
                                       blob->count = bytes_count ;
                                       if(blob->data == NULL)
                                       {
                                           dx_string_createU(sqlite->error,"Error on memory allocation for the row.");
                                           sqlite->error_code = DX_DB_GENERAL_ERROR ;
                                           return false ;
                                       }
                                       //copy the data

                                       memcpy(blob->data,sqlite3_column_text(pStmt, indx),bytes_count) ;
                                       field->obj = blob ;
                                  }
                                  break ;
           case SQLITE_NULL     : {
                                    field->flags = DX_FIELD_VALUE_NULL ;
                                  }
                                  break ;

		}
		//add the field in the row
		dx_list_add_node_direct(row,field) ;

		node = node->right ;
		indx++ ;
	}

	//add the row in the dataset
	PDXL_OBJECT obj = (PDXL_OBJECT)malloc(sizeof(struct dxl_object)) ;
	obj->obj = row ;
	dx_list_add_node_direct(dquery->dataset,obj);
	return true ;
}

/*----------------------------*/

PDX_QUERY dx_sqlite_select(PDX_SQLITE sqlite , PDX_STRING query)
{
    /*
       This is were all the magic is done! xD
       We will prepare and execute the query.
       We will return the dataset
    */

   if (query->type != dx_utf8)
   {
      dx_string_createU(sqlite->error,"The query can be only as a utf8 encoded string");
      return NULL;
   }

    dx_string_createU(sqlite->error,"") ;
    PDX_QUERY dquery = dx_db_query_create() ;
    sqlite3_stmt *pStmt =  dx_h_sqlite_prepare_statement(sqlite,query);
    if(pStmt == NULL) return NULL ;
   /*
     the statement is ready procceed
   */
   int unknown_col = 0 ;
   while(true)
   {
      int result = sqlite3_step(pStmt);
	  if(dx_h_sqlite_check_step_result(sqlite,dquery,pStmt,result) == false) return NULL ;

      /* out of the case without error. Nice! lets create the header if this is the first row! */

    if(result == SQLITE_DONE)
    {
      sqlite->error_code = DX_DB_SUCCESS  ;
      break ;
    }

    if(dquery->header->count == 0)
    {
      /*as it seems this is the first row that we have retrieved, lets construct the header*/
	  dx_h_sqlite_init_header(pStmt,dquery,&unknown_col);

    } else
       if(unknown_col > 0)
        {
            /* we will waste cpu cycles but we have unknown type columns! */
			dx_h_sqlite_check_unknown_header(pStmt,dquery,&unknown_col);
        }

        /* the header is set , now we will create the row and its fields*/
        dx_h_sqlite_add_row_to_dataset(pStmt,sqlite,dquery);
        if(sqlite->error_code != DX_DB_SUCCESS)
        {
          sqlite3_finalize(pStmt) ;
	      dx_db_query_free(dquery)   ;
	      return NULL             ;
        }
   }


    dx_h_sqlite_finalize_statement(sqlite,pStmt);
    dquery->row_count = dquery->dataset->count  ;
    return dquery ;
}



/* ************* ODBC ****************************************************/

PDX_STRING odbc_extract_error(SQLHANDLE handle, SQLSMALLINT type)
{
    SQLINTEGER i = 0;
    SQLINTEGER NativeError;
    SQLWCHAR SQLState[7];
    SQLWCHAR MessageText[2048]; /*A VALUE OF 254 FOR EXAMPLE CREATES A STACK SMASHING ERROR IN LINUX. WHY? */
    SQLSMALLINT TextLength;
    SQLRETURN ret;
    PDX_STRING astr = dx_string_createW(NULL,L"");
    PDX_STRING tmps  = dx_string_createW(NULL,L"");
    do
    {
        ret = SQLGetDiagRec(type, handle, ++i, SQLState, &NativeError,MessageText, 2048, &TextLength);
        if (SQL_SUCCEEDED(ret))
        {
             dx_string_copy(tmps,astr) ;
             dx_string_free(astr);
             PDX_STRING tmp2 = odbc_string_create(NULL,MessageText);
             astr = dx_string_concat(tmps,tmp2);
             dx_string_free(tmp2);
        }
    }
    while( ret == SQL_SUCCESS );
    dx_string_free(tmps);
    return astr ;
}



bool dx_odbc_succeeded(SQLRETURN ret,PDX_ODBC odbc,PDX_STRING status,SQLSMALLINT type)
{
   if(odbc == NULL) return false ;

  SQLHANDLE sqlhandle = NULL;
  if(type == SQL_HANDLE_DBC) sqlhandle = odbc->dbc;
  else
      if(type == SQL_HANDLE_ENV) sqlhandle = odbc->env;

   switch(ret)
   {
       case SQL_SUCCESS : {dx_string_createU(status , "");return true ;}break ;
       case SQL_SUCCESS_WITH_INFO :
           {
               PDX_STRING ret_stat = odbc_extract_error(sqlhandle , type);
               PDX_STRING utf8 = dx_string_convertU(ret_stat);
               dx_string_copy(status,utf8);
               dx_string_free(utf8);
               dx_string_free(ret_stat);
               return true ;
           }
       break ;
       case SQL_NO_DATA : {dx_string_createU(status , "");return true ;}break ;
       case SQL_ERROR :
           {
               PDX_STRING ret_stat = odbc_extract_error(sqlhandle , type);
               PDX_STRING utf8 = dx_string_convertU(ret_stat);
               dx_string_copy(status,utf8);
               dx_string_free(utf8);
               dx_string_free(ret_stat);
               return false ;

           }

       break ;
       case SQL_INVALID_HANDLE :  {dx_string_createU(status , "INVALID_HANDLE");return false ;}break ;
       break ;
       case SQL_STILL_EXECUTING : {dx_string_createU(status , "STILL_EXECUTING");return false ;}break ;
       break ;

   }

   return false ;

}

/*
 Error messages for statements
-------------------------------
*/
PDX_STRING stm_extract_error(SQLHANDLE handle)
{
    SQLINTEGER i = 0;
    SQLINTEGER NativeError;
    SQLWCHAR SQLState[7];
    SQLWCHAR MessageText[256];
    SQLSMALLINT TextLength;
    SQLRETURN ret;
    PDX_STRING wstr = dx_string_createW(NULL,L"");
    PDX_STRING tmps  = dx_string_createW(NULL,L"");
    do
    {
        ret = SQLGetDiagRec(SQL_HANDLE_STMT, handle, ++i, SQLState, &NativeError,MessageText, 256, &TextLength);
        if (SQL_SUCCEEDED(ret))
        {
             dx_string_copy(tmps,wstr) ;
             dx_string_free(wstr);
             PDX_STRING tmp2 = odbc_string_create(NULL,MessageText);
             wstr = dx_string_concat(tmps,tmp2);
             dx_string_free(tmp2);
        }
    }
    while( ret == SQL_SUCCESS );
    dx_string_free(tmps);
    PDX_STRING ustr = dx_string_convertU(wstr) ;
    dx_string_free(wstr);
    return ustr ;
}


bool dx_odbc_stm_succeeded(SQLRETURN ret, SQLHANDLE stmt,PDX_STRING status)
{

  SQLHANDLE sqlhandle = stmt;

   switch(ret)
   {
       case SQL_SUCCESS : {dx_string_createU(status , "");return true ;}break ;
       case SQL_SUCCESS_WITH_INFO :
           {
               PDX_STRING ret_stat = stm_extract_error(sqlhandle);
               dx_string_copy(status,ret_stat);
               dx_string_free(ret_stat);
               return true ;
           }
       break ;
       case SQL_NO_DATA : {dx_string_createU(status , "");return true ;}break ;
       case SQL_ERROR :
           {
               PDX_STRING ret_stat = stm_extract_error(sqlhandle);
               dx_string_copy(status,ret_stat);
               dx_string_free(ret_stat);
               return false ;

           }

       break ;
       case SQL_INVALID_HANDLE :  {dx_string_createU(status , "INVALID_HANDLE");return false ;}break ;
       break ;
       case SQL_STILL_EXECUTING : {dx_string_createU(status , "STILL_EXECUTING");return false ;}break ;
       break ;

   }

   return false ;

}

/*
-------------------------------
*/

PDX_ODBC dx_odbc_init(PDX_STRING conn_string,PDX_STRING status,bool blob_as_string)
{
    /*
      Create the odbc object , make the connection and return it
    */

    PDX_ODBC odbc = malloc(sizeof(struct dx_odbc));
    odbc->blob_as_string = blob_as_string ;

    /* Allocate an environment handle */
    SQLRETURN ret = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &(odbc->env)) ;
    if (ret != SQL_SUCCESS)
    {
        PDX_STRING mess   = dx_string_createW(NULL,L"The enviroment variable was not allocated. Error Code : ");
        PDX_STRING retstr = dx_IntToStr(ret);
        PDX_STRING cmess  = dx_string_concat(mess,retstr);

        PDX_STRING utf8 = dx_string_convertU(cmess);
        dx_string_copy(status,utf8);
        dx_string_free(utf8);
        dx_string_free(mess);
        dx_string_free(retstr);
        dx_string_free(cmess);
        free(odbc);
        return NULL ;
    }

    /* We want ODBC 3 support */
    ret = SQLSetEnvAttr(odbc->env, SQL_ATTR_ODBC_VERSION, (void *) SQL_OV_ODBC3, 0);
    if (dx_odbc_succeeded(ret,odbc,status,SQL_HANDLE_ENV)== false)
    {
        SQLFreeHandle( SQL_HANDLE_ENV, odbc->env );
        free(odbc);
        return NULL ;
    }

    /* Allocate a connection handle */
    ret = SQLAllocHandle(SQL_HANDLE_DBC, odbc->env, &(odbc->dbc));
    if (dx_odbc_succeeded(ret,odbc,status,SQL_HANDLE_ENV)== false)
    {
        SQLFreeHandle( SQL_HANDLE_ENV, odbc->env );
        free(odbc);
        return NULL ;
    }

    /* Connect*/
    SQLWCHAR *c_str = dx_odbc_wchar_to_sqlwchar(conn_string->stringw);
    ret = SQLDriverConnect(odbc->dbc, NULL, c_str, SQL_NTS,NULL, 0, NULL, SQL_DRIVER_NOPROMPT);
    #ifdef _LINUX
    free(c_str) ;
    #endif

    if (dx_odbc_succeeded(ret,odbc,status,SQL_HANDLE_DBC)== false)
    {
        SQLFreeHandle( SQL_HANDLE_DBC , odbc->dbc );
        SQLFreeHandle( SQL_HANDLE_ENV , odbc->env );
        free(odbc);
        return NULL ;
    }

    /* SUCCESS! Al ok, return the odbc object ready to use for statements*/
    return odbc ;

}


void dx_odbc_free(PDX_ODBC odbc)
{
   /* Disconnect from the database. */
    SQLDisconnect( odbc->dbc );
    /* Free the connection handle. */
    SQLFreeHandle( SQL_HANDLE_DBC, odbc->dbc );
    /* Free the environment handle. */
    SQLFreeHandle( SQL_HANDLE_ENV, odbc->env );
    free(odbc) ;
}


PDX_STRING dx_odbc_translate_col_type_to_str(SQLSMALLINT coltype)
{
    switch(coltype)
    {

        case SQL_VARCHAR : {return dx_string_createU(NULL,"SQl_VARCHAR");}break;
        case SQL_LONGVARCHAR : {return dx_string_createU(NULL,"SQL_LONGVARCHAR");}break;
        case SQL_WCHAR : {return dx_string_createU(NULL,"SQL_WCHAR");}break;
        case SQL_WVARCHAR : {return dx_string_createU(NULL,"SQL_WVARCHAR");}break;
        case SQL_WLONGVARCHAR : {return dx_string_createU(NULL,"SQL_WLONGVARCHAR");}break;
        case SQL_DECIMAL : {return dx_string_createU(NULL,"SQL_DECIMAL");}break;
        case SQL_NUMERIC : {return dx_string_createU(NULL,"SQL_NUMERIC");}break;
        case SQL_SMALLINT : {return dx_string_createU(NULL,"SQL_SMALLINT");}break;
        case SQL_INTEGER : {return dx_string_createU(NULL,"SQL_INTEGER");}break;
        case SQL_REAL : {return dx_string_createU(NULL,"SQL_REAL");}break;
        case SQL_FLOAT : {return dx_string_createU(NULL,"SQL_FLOAT");}break;
        case SQL_DOUBLE : {return dx_string_createU(NULL,"SQL_DOUBLE");}break;
        case SQL_BIT : {return dx_string_createU(NULL,"SQL_BIT");}break;
        case SQL_TINYINT : {return dx_string_createU(NULL,"SQL_TINYINT");}break;
        case SQL_BIGINT : {return dx_string_createU(NULL,"SQL_BIGINT");}break;
        case SQL_BINARY : {return dx_string_createU(NULL,"SQL_BINARY");}break;
        case SQL_VARBINARY : {return dx_string_createU(NULL,"SQL_VARBINARY");}break;
        case SQL_LONGVARBINARY : {return dx_string_createU(NULL,"SQL_LONGVARBINARY");}break;
        case SQL_TYPE_DATE : {return dx_string_createU(NULL,"SQL_TYPE_DATE");}break;
        case SQL_TYPE_TIME : {return dx_string_createU(NULL,"SQL_TYPE_TIME");}break;
        case SQL_TYPE_TIMESTAMP : {return dx_string_createU(NULL,"SQL_TYPE_TIMESTAMP");}break;
      //  case SQL_TYPE_UTCDATETIME : {}break;
      //  case SQL_TYPE_UTCTIME : {}break;
        case SQL_INTERVAL_MONTH : {return dx_string_createU(NULL,"SQL_INTERVAL_MONTH");}break;
        case SQL_INTERVAL_YEAR : {return dx_string_createU(NULL,"SQL_INTERVAL_YEAR");}break;
        case SQL_INTERVAL_YEAR_TO_MONTH : {return dx_string_createU(NULL,"SQL_INTERVAL_YEAR_TO_MONTH");}break;
        case SQL_INTERVAL_DAY : {return dx_string_createU(NULL,"SQL_INTERVAL_DAY");}break;
        case SQL_INTERVAL_HOUR : {return dx_string_createU(NULL,"SQL_INTERVAL_HOUR");}break;
        case SQL_INTERVAL_MINUTE : {return dx_string_createU(NULL,"SQL_INTERVAL_MINUTE");}break;
        case SQL_INTERVAL_SECOND : {return dx_string_createU(NULL,"SQL_INTERVAL_SECOND");}break;
        case SQL_INTERVAL_DAY_TO_HOUR : {return dx_string_createU(NULL,"SQL_INTERVAL_DAY_TO_HOUR");}break;
        case SQL_INTERVAL_DAY_TO_MINUTE : {return dx_string_createU(NULL,"SQL_INTERVAL_DAY_TO_MINUTE");}break;
        case SQL_INTERVAL_DAY_TO_SECOND : {return dx_string_createU(NULL,"SQL_INTERVAL_DAY_TO_SECOND");}break;
        case SQL_INTERVAL_HOUR_TO_MINUTE : {return dx_string_createU(NULL,"SQL_INTERVAL_HOUR_TO_MINUTE");}break;
        case SQL_INTERVAL_HOUR_TO_SECOND : {return dx_string_createU(NULL,"SQL_INTERVAL_HOUR_TO_SECOND");}break;
        case SQL_INTERVAL_MINUTE_TO_SECOND : {return dx_string_createU(NULL,"SQL_INTERVAL_MINUTE_TO_SECOND");}break;
        case SQL_GUID : {return dx_string_createU(NULL,"SQL_GUID");}break;
        default  : dx_string_createU(NULL,"UNKNOWN") ;
    }

    return NULL ;
}


unsigned long dx_odbc_generic_type(SQLSMALLINT coltype)
{
    switch(coltype)
    {

        case SQL_VARCHAR        : {return DX_FIELD_VALUE_TEXT;}break;
        case SQL_LONGVARCHAR    : {return DX_FIELD_VALUE_TEXT;}break;
        case SQL_WCHAR          : {return DX_FIELD_VALUE_TEXT;}break;
        case SQL_WVARCHAR       : {return DX_FIELD_VALUE_TEXT;}break;
        case SQL_WLONGVARCHAR   : {return DX_FIELD_VALUE_TEXT;}break;
        case SQL_DECIMAL        : {return DX_FIELD_VALUE_FLOAT;}break;
        case SQL_NUMERIC        : {return DX_FIELD_VALUE_FLOAT;}break;
        case SQL_SMALLINT       : {return DX_FIELD_VALUE_INT;}break;
        case SQL_INTEGER        : {return DX_FIELD_VALUE_INT;}break;
        case SQL_REAL           : {return DX_FIELD_VALUE_FLOAT;}break;
        case SQL_FLOAT          : {return DX_FIELD_VALUE_FLOAT;}break;
        case SQL_DOUBLE         : {return DX_FIELD_VALUE_FLOAT;}break;
        case SQL_BIT            : {return DX_FIELD_VALUE_INT;}break;
        case SQL_TINYINT        : {return DX_FIELD_VALUE_INT;}break;
        case SQL_BIGINT         : {return DX_FIELD_VALUE_INT;}break;
        case SQL_BINARY         : {return DX_FIELD_VALUE_BLOB;}break;
        case SQL_VARBINARY      : {return DX_FIELD_VALUE_BLOB;}break;
        case SQL_LONGVARBINARY  : {return DX_FIELD_VALUE_BLOB;}break;
        case SQL_TYPE_DATE      : {return DX_FIELD_VALUE_DATE;}break;
        case SQL_TYPE_TIME      : {return DX_FIELD_VALUE_DATE;}break;
        case SQL_TYPE_TIMESTAMP : {return DX_FIELD_VALUE_DATE;}break;
      //  case SQL_TYPE_UTCDATETIME : {}break;
      //  case SQL_TYPE_UTCTIME : {}break;
        case SQL_INTERVAL_MONTH : {return DX_FIELD_VALUE_FLOAT;}break;
        case SQL_INTERVAL_YEAR  : {return DX_FIELD_VALUE_FLOAT;}break;
        case SQL_INTERVAL_YEAR_TO_MONTH : {return DX_FIELD_VALUE_FLOAT;}break;
        case SQL_INTERVAL_DAY   : {return DX_FIELD_VALUE_FLOAT;}break;
        case SQL_INTERVAL_HOUR  : {return DX_FIELD_VALUE_FLOAT;}break;
        case SQL_INTERVAL_MINUTE : {return DX_FIELD_VALUE_FLOAT;}break;
        case SQL_INTERVAL_SECOND : {return DX_FIELD_VALUE_FLOAT;}break;
        case SQL_INTERVAL_DAY_TO_HOUR      : {return DX_FIELD_VALUE_FLOAT;}break;
        case SQL_INTERVAL_DAY_TO_MINUTE    : {return DX_FIELD_VALUE_FLOAT;}break;
        case SQL_INTERVAL_DAY_TO_SECOND    : {return DX_FIELD_VALUE_FLOAT;}break;
        case SQL_INTERVAL_HOUR_TO_MINUTE   : {return DX_FIELD_VALUE_FLOAT;}break;
        case SQL_INTERVAL_HOUR_TO_SECOND   : {return DX_FIELD_VALUE_FLOAT;}break;
        case SQL_INTERVAL_MINUTE_TO_SECOND : {return DX_FIELD_VALUE_FLOAT;}break;
        case SQL_GUID                      : {return DX_FIELD_VALUE_TEXT;}break;
        default  : return DX_FIELD_VALUE_TEXT ;
    }


    return DX_FIELD_VALUE_TEXT ;
}


bool dx_odbc_construct_header(PDX_QUERY dquery,SQLSMALLINT columns,SQLHANDLE stmt,PDX_STRING status)
{

 /*
   the header is encoded in utf8 strings
 */
  for(int i = 1 ; i <=columns;i++)
  {
        SQLWCHAR        colname[128] ;
        SQLSMALLINT      colnamelen;
        SQLSMALLINT     coltype ;
        SQLULEN         colsize;
        SQLSMALLINT     scale;

        SQLRETURN ret = SQLDescribeCol(stmt, i , colname, sizeof(colname),&colnamelen, &coltype, &colsize, &scale, NULL);

        if (dx_odbc_stm_succeeded(ret,stmt,status)== false)
        {
          return false ;
        }



        PDX_STRING coltypestru =  dx_odbc_translate_col_type_to_str(coltype) ;
        PDX_STRING colnamew   =  odbc_string_create(NULL,colname) ;
        PDX_STRING colnameu   =  dx_string_convertU(colnamew) ;

        PDX_COLUMN col        =  dx_db_column_create(colnameu,coltypestru) ;

        dx_string_free(coltypestru);
        dx_string_free(colnamew);
        dx_string_free(colnameu);

        col->flags = dx_odbc_generic_type(coltype) ; // set the type
        dx_list_add_node_direct(dquery->header,col) ;

  }

    return true ;
}


int dx_odbc_retrieve_big_text(SQLHANDLE stmt , SQLSMALLINT colnum , SQLRETURN ret , SQLLEN indicator, SQLWCHAR *buf , PDX_STRING data)
{
    /*
      returns -1 for NULL , 0 for success
    */
    if (SQL_SUCCEEDED(ret))
    {
        /* Handle null columns */
        if (indicator == SQL_NULL_DATA) return -1 ;
            else
            {

              //check if all the data was received
              if (indicator <= ODBC_BUFFER_LEN)
              {
               odbc_string_create(data,buf);
               return 0 ;
              }
               else
               {
                   //fetch all the data
                   odbc_string_create(data,buf);
                   PDX_STRING tmps   = odbc_string_create(NULL,NULL);
                   while(SQLGetData(stmt, colnum, SQL_C_WCHAR,buf, ODBC_BUFFER_LEN, &indicator) != SQL_NO_DATA)
                   {
                      // printf("len : %d indicator : %d",StrLenW(buf),indicator);
                       dx_string_copy(tmps,data) ;
                       PDX_STRING tmp2 = odbc_string_create(NULL,buf);
                       PDX_STRING concs = dx_string_concat(tmps,tmp2);
                       dx_string_createW(data,concs->stringw) ;
                       dx_string_free(concs);
                       dx_string_free(tmp2);
                   }

                   dx_string_free(tmps);
               }


            }


    }


    return 0 ;

}

int dx_odbc_retrieve_big_data(SQLHANDLE stmt,SQLSMALLINT colnum,SQLRETURN ret , SQLLEN indicator, SQLCHAR *buf , SQLCHAR * nbuf,DXLONG64 *total_bytes)
{
    /*
      returns -1 for NULL , 0 for success
    */
    if (SQL_SUCCEEDED(ret))
    {
        /* Handle null columns */
        if (indicator == SQL_NULL_DATA) return -1 ;
            else
            {

              //check if all the data was received
              if (indicator <= ODBC_BUFFER_LEN)
              {
                *total_bytes = indicator ;
                memcpy(nbuf,buf,indicator);
               return 0 ;
              }
               else
               {
                   /*copy the first part of the data*/
				   memcpy(nbuf,buf,ODBC_BUFFER_LEN);
				   *total_bytes = indicator ; /*we already know the memory we will need for this operation*/
				   /*calibrate the offset*/
				   nbuf = nbuf + ODBC_BUFFER_LEN;
				   /*fetch the rest of the data*/
				   SQLLEN prev_data_count = indicator ;
                   while(SQLGetData(stmt, colnum, SQL_C_BINARY,buf, ODBC_BUFFER_LEN , &indicator) != SQL_NO_DATA)
                   {
					    if (indicator < ODBC_BUFFER_LEN)
                        {
                           memcpy(nbuf,buf,indicator)  ;
                           /*no need for repositioning the index we are done!*/
                        }
                         else
                         {
                           // printf("prev_data - indicator : %d - %d diff : %d \n",prev_data_count,indicator,(prev_data_count - indicator));
                            memcpy(nbuf,buf,(prev_data_count - indicator))  ;
                            nbuf = nbuf + (prev_data_count - indicator) ;
                            prev_data_count = indicator  ;
                         }


                   }

               }


            }


    }

    return 0 ;

}


bool dx_odbc_create_rows(PDX_ODBC odbc,PDX_QUERY dquery,SQLHANDLE stmt, PDX_STRING status,SQLSMALLINT columns)
{
 SQLRETURN ret;
 while (SQL_SUCCEEDED(ret = SQLFetch(stmt)))
 {
    SQLUSMALLINT i;
    PDX_ROW    row = dx_db_row_create(); /*Create the row*/
    /* Loop through the columns */
    for (i = 1; i <= columns; i++)
    {
        SQLLEN indicator ;

        /* retrieve column data as its native type except the blob fields that are regulated by the blob_as_string flag */

        PDX_COLUMN col = dx_list_go_to_pos( dquery->header , i-1 )->object ;
        PDX_FIELD  field = dx_db_field_create() ; /*create the field of the row*/
        switch (col->flags)
        {
            case DX_FIELD_VALUE_INT   :
                                        {
                                            long int val = 0 ;
                                            ret = SQLGetData(stmt, i, SQL_C_SLONG ,(SQLPOINTER)&val, sizeof(val), &indicator);

                                            if (indicator == SQL_NULL_DATA)
                                                field->flags =  DX_FIELD_VALUE_NULL;
                                                 else
                                                    field->flags     =  DX_FIELD_VALUE_INT ;
                                            field->int_key   = (DXLONG64)val ;
                                        }
                                        break ;
            case DX_FIELD_VALUE_FLOAT :
                                        {
                                            double val = 0 ;
                                            ret = SQLGetData(stmt, i, SQL_C_DOUBLE ,(SQLPOINTER)&val, sizeof(val), &indicator);
                                            if (indicator == SQL_NULL_DATA)
                                                field->flags =  DX_FIELD_VALUE_NULL;
                                                 else
                                                    field->flags =  DX_FIELD_VALUE_FLOAT ;
                                            field->float_key   = val ;
                                        }
                                        break ;
            case DX_FIELD_VALUE_TEXT  :
                                         {
                                                SQLWCHAR    buff[ODBC_BUFFER_LEN] ;
                                                PDX_STRING  data = odbc_string_create(NULL,NULL);
                                                ret = SQLGetData(stmt, i, SQL_C_WCHAR ,(SQLPOINTER)buff, sizeof(buff), &indicator);
                                                if (dx_odbc_retrieve_big_text(stmt,i,ret , indicator, buff , data) == -1)
                                                    field->flags =  DX_FIELD_VALUE_NULL;
                                                     else
                                                        field->flags       =  DX_FIELD_VALUE_TEXT ;
                                                field->key         =  dx_string_convertU(data) ;
                                                dx_string_free(data); /*free the memory*/

                                         }
                                         break ;
            case DX_FIELD_VALUE_BLOB  :
                                        {
                                            if(odbc->blob_as_string == false)
                                            {
                                                SQLCHAR    buff[ODBC_BUFFER_LEN] ;
                                                ret = SQLGetData(stmt, i, SQL_C_BINARY ,(SQLPOINTER)buff, ODBC_BUFFER_LEN, &indicator);
                                                /* create a buffer large enough to accomodate the blob*/
												SQLCHAR *nbuff = malloc(indicator);
												DXLONG64 bytes_count = 0 ;
												if (dx_odbc_retrieve_big_data(stmt,i,ret , indicator, buff , nbuff,&bytes_count) == -1)
                                                    field->flags =  DX_FIELD_VALUE_NULL;
                                                     else
                                                        field->flags =  DX_FIELD_VALUE_BLOB ;

                                                PDX_DB_BLOB blob = malloc(sizeof(struct dx_db_blob)) ;
                                                blob->count = bytes_count ;
                                                blob->data  = (char*)nbuff ;
                                                field->obj =  blob ;

                                            } else
                                                 {
                                                     SQLWCHAR    buff[ODBC_BUFFER_LEN] ;
                                                     PDX_STRING  data = dx_string_createW(NULL,L"");
                                                     ret = SQLGetData(stmt, i, SQL_C_WCHAR ,(SQLPOINTER)buff, ODBC_BUFFER_LEN, &indicator);
                                                     if (dx_odbc_retrieve_big_text(stmt,i,ret , indicator, buff , data) == -1)
                                                        field->flags =  DX_FIELD_VALUE_NULL;
                                                         else
                                                            field->flags       =  DX_FIELD_VALUE_TEXT ; /*the application can check this to determine the actual data type*/

                                                     field->key         =  dx_string_convertU(data) ;
                                                     dx_string_free(data); /*free the memory*/

                                                 }

                                        }
                                        break ;
            case DX_FIELD_VALUE_DATE  : {
                                                SQLWCHAR    buff[ODBC_BUFFER_LEN] ;
                                                PDX_STRING  data = dx_string_createW(NULL,L"");
                                                ret = SQLGetData(stmt, i, SQL_C_WCHAR ,(SQLPOINTER)buff, sizeof(buff), &indicator);
                                                if (dx_odbc_retrieve_big_text(stmt,i,ret , indicator, buff , data) == -1)
                                                    field->flags =  DX_FIELD_VALUE_NULL;
                                                     else
                                                        field->flags       =  DX_FIELD_VALUE_DATE ;
                                                field->key  =  dx_string_convertU(data) ;
                                                dx_string_free(data); /*free the memory*/
                                        }
                                        break ;

        }

          if (dx_odbc_stm_succeeded(ret,stmt,status)== false)
          {
             return false ;
          }

        //add the field in the row
		dx_list_add_node_direct(row,field) ;
    }

    //add the row in the dataset
	PDXL_OBJECT obj = (PDXL_OBJECT)malloc(sizeof(struct dxl_object)) ;
	obj->obj = row ;
	dx_list_add_node_direct(dquery->dataset,obj);

  }


  return true ;
}



PDX_QUERY dx_odbc_execute(PDX_ODBC odbc , PDX_STRING query,PDX_STRING status)
{
 /*
   this is the core of the odbc database , this function executes the query and if its
   a select query returns a native dataset
 */
  SQLHSTMT stmt ;
  /* Allocate a statement handle */
  SQLRETURN ret = SQLAllocHandle(SQL_HANDLE_STMT, odbc->dbc, &stmt);
  if (dx_odbc_succeeded(ret,odbc,status,SQL_HANDLE_DBC)== false)
  {
    return NULL ;
  }

  /* execute the query */
  /* we will use the stringw  member as we will use unicode */
  SQLWCHAR *q = dx_odbc_wchar_to_sqlwchar(query->stringw) ;
  ret = SQLExecDirect(stmt,q, SQL_NTS); // zero terminated string
  #ifdef _LINUX
  free(q); /* in linux we have created a new buffer with the data */
  #endif // _LINUX
  if (dx_odbc_stm_succeeded(ret,stmt,status)== false)
  {
    SQLFreeHandle( SQL_HANDLE_STMT, stmt );
    return NULL ;
  }

  /*Success , check how many columns are there */
  SQLSMALLINT columns ;
  ret = SQLNumResultCols(stmt, &columns);
  if (dx_odbc_stm_succeeded(ret,stmt,status)== false)
  {
    SQLFreeHandle( SQL_HANDLE_STMT, stmt );
    return NULL ;
  }

  if(columns == 0)
  {
      //no data all ok
      SQLFreeHandle( SQL_HANDLE_STMT, stmt );
      return NULL ;
  }

  /* there is data returned , translate the data to native dataset*/

  PDX_QUERY dquery = dx_db_query_create();

    /* Loop through the rows in the result-set
       and construct the header
    */
  if(dx_odbc_construct_header(dquery,columns,stmt,status) == false )
  {
    SQLFreeHandle( SQL_HANDLE_STMT, stmt );
    dx_db_query_free(dquery);
    return NULL ;
  }

 /* Header ready, create the rows */

  if(dx_odbc_create_rows(odbc,dquery,stmt,status,columns) == false )
  {
    SQLFreeHandle( SQL_HANDLE_STMT, stmt );
    dx_db_query_free(dquery);
    return NULL ;
  }

  /*all ok, the data are transformed in native form, release the statement handle*/
   SQLFreeHandle( SQL_HANDLE_STMT, stmt );
  dquery->row_count = dquery->dataset->count  ;
  return dquery ;

}


/* ********************MY SQL ********************** */

MYSQL * dx_mariadb_connect(PDX_STRING host,PDX_STRING user,PDX_STRING passwd,PDX_STRING db,
                         unsigned int port,PDX_STRING unix_socket,unsigned long flags,PDX_STRING charset,PDX_STRING status)
{

      MYSQL *conn = mysql_init(NULL);
      if (conn == NULL)
      {
          dx_string_createU(status,(char*)mysql_error(conn));
          return NULL ;
      }

      mysql_optionsv(conn, MYSQL_SET_CHARSET_NAME, (void *)charset->stringa);
      my_bool verify_cert = 0;
      mysql_optionsv(conn, MYSQL_OPT_SSL_VERIFY_SERVER_CERT, (void *)&verify_cert);

      if (mysql_real_connect(conn, host->stringa, user->stringa, passwd->stringa,db->stringa, port, unix_socket->stringa, flags) == NULL)
      {
          dx_string_createU(status,(char*)mysql_error(conn));
          mysql_close(conn);
          return NULL ;
      }

    /* connection ok! */
    /* set auto commit on */
   mysql_autocommit(conn,true);
   return conn ;

}


void dx_return_mdb_type_str(int ftype,PDX_STRING stype,uint32_t *native_type)
{
    /*
        DX_FIELD_VALUE_NULL  0
        DX_FIELD_VALUE_INT   1
        DX_FIELD_VALUE_FLOAT 2
        DX_FIELD_VALUE_TEXT  3
        DX_FIELD_VALUE_BLOB  4
        DX_FIELD_VALUE_DATE  5
    */
    switch(ftype)
    {
        case MYSQL_TYPE_DECIMAL     : {dx_string_createU(stype,"DECIMAL");*native_type = DX_FIELD_VALUE_FLOAT; }
        break;
        case MYSQL_TYPE_TINY        : {dx_string_createU(stype,"TINY");*native_type = DX_FIELD_VALUE_INT; }
        break;
        case MYSQL_TYPE_SHORT       : {dx_string_createU(stype,"SHORT");*native_type = DX_FIELD_VALUE_INT; }
        break;
        case MYSQL_TYPE_LONG        : {dx_string_createU(stype,"LONG");*native_type = DX_FIELD_VALUE_INT; }
        break;
        case MYSQL_TYPE_FLOAT       : {dx_string_createU(stype,"FLOAT");*native_type = DX_FIELD_VALUE_FLOAT; }
        break;
        case MYSQL_TYPE_DOUBLE      : {dx_string_createU(stype,"DOUBLE");*native_type = DX_FIELD_VALUE_FLOAT; }
        break;
        case MYSQL_TYPE_NULL        : {dx_string_createU(stype,"NULL");*native_type = DX_FIELD_VALUE_NULL; }
        break;
        case MYSQL_TYPE_TIMESTAMP   : {dx_string_createU(stype,"DATE");*native_type = DX_FIELD_VALUE_DATE; }
        break;
        case MYSQL_TYPE_LONGLONG    : {dx_string_createU(stype,"LONGLONG");*native_type = DX_FIELD_VALUE_INT; }
        break;
        case MYSQL_TYPE_INT24       : {dx_string_createU(stype,"INT24");*native_type = DX_FIELD_VALUE_INT; }
        break;
        case MYSQL_TYPE_DATE        : {dx_string_createU(stype,"DATE");*native_type = DX_FIELD_VALUE_DATE; }
        break;
        case MYSQL_TYPE_TIME        : {dx_string_createU(stype,"TIME");*native_type = DX_FIELD_VALUE_DATE; }
        break;
        case MYSQL_TYPE_DATETIME    : {dx_string_createU(stype,"DATETIME");*native_type = DX_FIELD_VALUE_DATE; }
        break;
        case MYSQL_TYPE_YEAR        : {dx_string_createU(stype,"YEAR");*native_type = DX_FIELD_VALUE_INT; }
        break;
        case MYSQL_TYPE_NEWDATE     : {dx_string_createU(stype,"NEWDATE");*native_type = DX_FIELD_VALUE_DATE; }
        break;
        case MYSQL_TYPE_VARCHAR     : {dx_string_createU(stype,"VARCHAR");*native_type = DX_FIELD_VALUE_TEXT; }
        break;
        case MYSQL_TYPE_BIT         : {dx_string_createU(stype,"BIT");*native_type = DX_FIELD_VALUE_INT; }
        break;
        case MYSQL_TYPE_TIMESTAMP2  : {dx_string_createU(stype,"TIMESTAMP2");*native_type = DX_FIELD_VALUE_DATE; }
        break;
        case MYSQL_TYPE_DATETIME2   : {dx_string_createU(stype,"DATETIME2");*native_type = DX_FIELD_VALUE_DATE; }
        break;
        case MYSQL_TYPE_TIME2       : {dx_string_createU(stype,"TIME2");*native_type = DX_FIELD_VALUE_DATE; }
        break;
        case MYSQL_TYPE_JSON        : {dx_string_createU(stype,"JSON");*native_type = DX_FIELD_VALUE_TEXT; }
        break;
        case MYSQL_TYPE_NEWDECIMAL  : {dx_string_createU(stype,"NEWDECIMAL");*native_type = DX_FIELD_VALUE_FLOAT; }
        break;
        case MYSQL_TYPE_ENUM        : {dx_string_createU(stype,"ENUM");*native_type = DX_FIELD_VALUE_TEXT; }
        break;
        case MYSQL_TYPE_SET         : {dx_string_createU(stype,"SET");*native_type = DX_FIELD_VALUE_TEXT; }
        break;
        case MYSQL_TYPE_TINY_BLOB   : {dx_string_createU(stype,"TINY_BLOB");*native_type = DX_FIELD_VALUE_BLOB; }
        break;
        case MYSQL_TYPE_MEDIUM_BLOB : {dx_string_createU(stype,"MEDIUM_BLOB");*native_type = DX_FIELD_VALUE_BLOB; }
        break;
        case MYSQL_TYPE_LONG_BLOB   : {dx_string_createU(stype,"LONG_BLOB");*native_type = DX_FIELD_VALUE_BLOB; }
        break;
        case MYSQL_TYPE_BLOB        : {dx_string_createU(stype,"BLOB");*native_type = DX_FIELD_VALUE_BLOB; }
        break;
        case MYSQL_TYPE_VAR_STRING  : {dx_string_createU(stype,"VAR_STRING");*native_type = DX_FIELD_VALUE_TEXT; }
        break;
        case MYSQL_TYPE_STRING      : {dx_string_createU(stype,"STRING");*native_type = DX_FIELD_VALUE_TEXT; }
        break;
        case MYSQL_TYPE_GEOMETRY    : {dx_string_createU(stype,"GEOMETRY");*native_type = DX_FIELD_VALUE_TEXT; }
        break;
    }
}


void dx_mariadb_create_header(PDX_QUERY dquery,MYSQL_RES *res,DXLONG64 num_fields,PDX_STRING status)
{
    MYSQL_FIELD * field ;
    for(int i = 0; i < num_fields; i++)
    {
        field = mysql_fetch_field(res);

        PDX_STRING strt = dx_string_createU(NULL,"")          ;
        PDX_STRING strn = dx_string_createU(NULL,field->name) ;

        uint32_t col_type = 0 ;
        dx_return_mdb_type_str(field->type,strt,&col_type);
        PDX_COLUMN col =  dx_db_column_create(strn,strt)  ;
        col->flags     = col_type ;

        dx_string_free(strt) ;
        dx_string_free(strn) ;

        dx_list_add_node_direct(dquery->header,col) ;
    }

    dx_string_createU(status,"");
    return ;
}


void dx_mariadb_create_rows(PDX_QUERY dquery,MYSQL_RES *res,DXLONG64 num_fields,PDX_STRING status)
{

  MYSQL_ROW   row ;
  PDX_HEADER  hdr =  dquery->header ;
  while ((row = mysql_fetch_row(res)))
  {
       unsigned long *lengths = mysql_fetch_lengths(res);
       if(lengths == NULL) return ;
       PDX_ROW    qrow = dx_db_row_create(); /*Create the row*/
       for(int i = 0; i < num_fields; i++)
       {

            PDX_COLUMN col = dx_list_go_to_pos(dquery->header,i)->object ;
            PDX_FIELD  field = dx_db_field_create() ; /*create the field of the row*/
            switch (col->flags)
            {
                case DX_FIELD_VALUE_INT   :
                                            {
                                                if (row[i] == NULL)
                                                {
                                                    field->flags   =  DX_FIELD_VALUE_NULL;
                                                    field->int_key = 0 ;
                                                }
                                                 else
                                                  {
                                                    field->flags =  DX_FIELD_VALUE_INT ;
                                                    field->int_key = strtol(row[i],NULL,10) ; // no error code will be returned
                                                  }

                                            }
                                            break ;
                case DX_FIELD_VALUE_FLOAT :
                                            {
                                                 if (row[i] == NULL)
                                                 {
                                                    field->flags =  DX_FIELD_VALUE_NULL;
                                                    field->float_key = 0 ;
                                                 }
                                                  else
                                                 {
                                                  field->flags =  DX_FIELD_VALUE_FLOAT ;
                                                   field->float_key = strtod(row[i],NULL) ;
                                                 }
                                            }
                                            break ;
                case DX_FIELD_VALUE_TEXT  :
                                             {
                                                    if (row[i] == NULL)
                                                    field->flags =  DX_FIELD_VALUE_NULL;
                                                     else
                                                        field->flags =  DX_FIELD_VALUE_TEXT ;
                                                  field->key =  dx_string_createU(NULL,row[i])   ;
                                             }
                                             break ;
                case DX_FIELD_VALUE_BLOB  :
                                            {
                                                if (row[i] == NULL)
                                                   field->flags =  DX_FIELD_VALUE_NULL;
                                                    else
                                                      field->flags   =  DX_FIELD_VALUE_BLOB ; /*the application can check this to determine the actual data type*/

                                                 PDX_DB_BLOB blob = malloc(sizeof(struct dx_db_blob));
                                                 blob->count = lengths[i]             ;
                                                 blob->data  = malloc(lengths[i])     ;
                                                 memcpy(blob->data,row[i],lengths[i]) ;
                                                 field->obj = blob ;
                                            }
                                            break ;
                case DX_FIELD_VALUE_DATE  : {
                                                   if (row[i] == NULL)
                                                     field->flags =  DX_FIELD_VALUE_NULL;
                                                      else
                                                        field->flags =  DX_FIELD_VALUE_DATE ;
                                                    field->key  = dx_string_createU(NULL,row[i]) ;
                                            }
                                            break ;

            }
        //add the field in the row
		dx_list_add_node_direct(qrow,field) ;
       } // for int

        //add the row in the dataset
        PDXL_OBJECT obj = (PDXL_OBJECT)malloc(sizeof(struct dxl_object)) ;
        obj->obj = qrow ;
        dx_list_add_node_direct(dquery->dataset,obj);

  } //while

  dquery->row_count = dquery->dataset->count  ;

}

PDX_QUERY dx_mariadb_execute(MYSQL * conn ,PDX_STRING query,PDX_STRING status)
{
  if(conn == NULL)
  {
    status = dx_string_createU(status,"NULL") ;
    return NULL ;
  }
  if (mysql_real_query(conn, query->stringa,query->bcount)!=0)
  {
      dx_string_createU(status,(char*)mysql_error(conn));
      return NULL ;
  }

  MYSQL_RES *res = mysql_store_result(conn);
  if (res != NULL)
  {
          PDX_QUERY dquery = dx_db_query_create();
          // get the number of the columns
          DXLONG64 num_fields = mysql_num_fields(res);
          if(num_fields == 0)
          {
              return NULL ;
          }

    dx_mariadb_create_header(dquery,res,num_fields,status) ;
    /*
      Load the dataset. The data will be transformed to the native types
    */
    dx_mariadb_create_rows(dquery,res,num_fields,status)   ;
    mysql_free_result(res);
    return dquery ;

   }

  mysql_free_result(res);
  dx_string_createU(status,"");
  return NULL ;
}

void dx_mariadb_disconnect(MYSQL * conn)
{
    mysql_close(conn);
}

/* ************************************************ */


/* ************* GENERAL DATABASE CODE **********************************/



PDX_QUERY dx_db_query_create()
{
    PDX_QUERY query = (PDX_QUERY) malloc(sizeof(struct dx_query));
    query->header   = dx_list_create_list();
    query->dataset  = dx_list_create_list();
    return query ;
}


PDX_QUERY dx_db_query_free(PDX_QUERY query)
{
    if (query == NULL) return NULL ;
    /*
      Free all the memory of the object , dispose of the object itself
      and return NULL
    */

    PDX_HEADER   hdr      = query->header  ;
    PDX_DATASET  dataset  = query->dataset ;

    query->header  = dx_db_header_free(hdr);
    query->dataset = dx_db_dataset_free(dataset);

    //free the query
    free(query);

    return NULL;
}

PDX_ROW dx_db_query_row_from_node(PDXL_NODE node)
{
   return (PDX_ROW) node->object->obj ;

}


PDX_DATASET dx_db_dataset_create()
{
  /* create a simple list as a dataset */
  return dx_list_create_list();
}

PDX_DATASET dx_db_dataset_free(PDX_DATASET dataset)
{
    if(dataset == NULL) return NULL ;
  /* free the rows of a dataset and the dataset itself */
    PDXL_NODE node = dataset->start ;
    while(node != NULL)
    {
        PDX_ROW row = dx_db_query_row_from_node(node) ;
        dx_db_row_free(row);
        free(node->object) ;
        node = node->right ;
    }

    dx_list_delete_list(dataset) ;
    return NULL   ;
}

PDX_ROW dx_db_row_create()
{
    return dx_list_create_list();
}

PDX_ROW dx_db_row_free(PDX_ROW row)
{
    /* we will free the fields in the row */
    /* Free the memory of the objects inside the list nodes */
    PDXL_NODE node = row->start ;
    while(node != NULL)
    {
        PDX_FIELD field = node->object ;
        dx_db_field_free(field);
        node = node->right ;
    }

    dx_list_delete_list(row);
    return NULL ;
}

PDX_HEADER dx_db_header_create()
{
  return dx_list_create_list();
}

PDX_HEADER dx_db_header_free(PDX_HEADER header)
{
    if(header == NULL) return NULL ;
 /* free every column of the header and the header itself */
    PDXL_NODE node = header->start ;
    while(node != NULL)
    {
        PDX_COLUMN col = node->object ;
        dx_db_column_free(col);
        node = node->right ;
    }

    dx_list_delete_list(header);
    return NULL ;
}

PDX_COLUMN  dx_db_column_create(PDX_STRING name , PDX_STRING type)
{
   /* create an object to take the role of a column */
   PDX_COLUMN col = (PDX_COLUMN)malloc(sizeof(struct dxl_object)) ;
   col->key       = dx_string_convertU(name) ; // if the string is not a utf8 then the string will be converted to utf8
   col->obj       = (void*)dx_string_convertU(type) ;
   col->flags     = 0 ; /* the flags will be set to the right SQL type*/
   col->float_key = 0 ;
   col->int_key   = 0 ;
   return col ;
}

PDX_COLUMN  dx_db_column_free(PDX_FIELD column)
{
    dx_string_free(column->key)             ;
    dx_string_free((PDX_STRING)column->obj) ;
    free(column)                            ;
    return NULL                             ;
}

PDX_FIELD  dx_db_field_create()
{
   PDX_FIELD field = (PDX_FIELD)malloc(sizeof(struct dxl_object)) ;
   field->key       = NULL ;
   field->obj       = NULL ;
   field->flags     = DX_FIELD_VALUE_NULL ; // GENERAL NULL VALUE
   field->float_key = 0 ;
   field->int_key   = 0 ;
   return field ;
}


PDX_FIELD  dx_db_field_free(PDX_FIELD field)
{
   if(field->flags == DX_FIELD_VALUE_BLOB)
   {
       free(((PDX_DB_BLOB)field->obj)->data) ;
       free((PDX_DB_BLOB)field->obj);
   }
    else
       if(field->flags == DX_FIELD_VALUE_TEXT)
       {
           dx_string_free(field->key);
           field->key = NULL ;
       }


  /* for safety measures... */
  if(field->key != NULL)
  {
       dx_string_free(field->key) ;
  }

  free(field) ;
  return NULL ;
}


/************* convinience functions******************/


PDX_STRING dx_db_field_as_string(PDX_FIELD field,int float_digit)
{
    if(field == NULL) return dx_string_createU(NULL,"NULL") ; ;
    switch(field->flags)
    {
      case DX_FIELD_VALUE_INT   :
      {
        return dx_IntToStr(field->int_key);
      } break ;
      case DX_FIELD_VALUE_FLOAT :
      {
        return dx_FloatToStr(field->float_key,float_digit);
      } break ;
      case DX_FIELD_VALUE_TEXT  :
      {
        return dx_string_clone(field->key) ;
      } break ;
      case DX_FIELD_VALUE_BLOB  :
      {
        return dx_string_createU(NULL,"[BLOB]");
      } break ;
      case DX_FIELD_VALUE_DATE  :
      {
        return dx_string_clone(field->key) ;
      } break ;
    }

    return  dx_string_createU(NULL,"NULL") ;
}

PDX_STRING dx_db_field_name(PDX_QUERY dataset , DXLONG64 indx)
{
   if(dataset == NULL) return NULL ;

   PDXL_NODE node = dx_list_go_to_pos(dataset->header, indx) ;
   if(node == NULL) return NULL ;
   PDXL_OBJECT obj = node->object ;
   return dx_string_createU(NULL,obj->key->stringa) ;
}

DXLONG64 dx_db_find_field_pos(PDX_QUERY query ,PDX_STRING fieldName)
{
 
    if((query == NULL) || (fieldName == NULL)) return -1 ;

    PDX_HEADER header = query->header ;

    DXLONG64 indx = 0 ; 
    PDXL_NODE node = header->start ;
    while(node!=NULL)
    {
        PDXL_OBJECT obj = node->object ;
        if(dx_string_native_compare(obj->key,fieldName) == dx_equal) return indx ;
        indx++ ;
        node = node->right ;
    }


    return -1 ;
}





