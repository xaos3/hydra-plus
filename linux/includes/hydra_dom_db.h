/*
  This file has the support for the databases.
  Databases supported : 
  Native			: SQLite
  MariaDb / MySQl   : Via mariadb-connector-c
  ODBC				: Native ODBC drivers of the OS

  Nikos Mourgis deus-ex.gr 2024 

  Live Long and Prosper.
*/



bool hdr_domSqliteOpen(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token, PHDR_VAR *result)
{
   PHDR_SYS_FUNC_PARAMS params = hdr_sys_func_init_params(inter,token->parameters,3) ;
   if(params == NULL)
   {
    printf("The system function Database.SQLite($filename:String , $doCreate : Boolean, out -> $error : String):SQLite Connection failed.\n");
    return true ;
   }

   bool type_error = false ;

   PDX_STRING fname = hdr_inter_ret_string(params->params[0],&type_error) ; 
   if(type_error == true)
   {
	 printf("The first parameter must be a String.\n");
     goto fail ;
   }

   bool can_create  = hdr_inter_ret_bool(params->params[1],&type_error) ; 
   if(type_error == true)
   {
	 printf("The second parameter must be a boolean value.\n");
     goto fail ;
   }

   PDX_STRING error = hdr_inter_ret_string(params->params[2],&type_error) ; 
   if(type_error == true)
   {
	 printf("The third parameter must be a String.\n");
     goto fail ;
   }

   error = dx_string_createU(error,"");

   *result            = hdr_var_create(NULL, "", hvf_temporary_ref, NULL) ;
   (*result)->type    = hvt_database ; 

   /*
    first check if the database file exists. If the file does not exists and the can_create is false then 
    we will return an empty object
   */
   if(can_create == false)
   {
     if(dxFileExists(fname) == false) 
     {
         error = (void*)dx_string_createU(error,"Database file not found"); 
         goto success ;
     }
   }

    /*connect to the database*/
    PDX_SQLITE dbconn = dx_sqlite_init() ;
    if (dbconn == NULL)
    {
      error = dx_string_createU(error,"The memory allocation for the connection failed. MALLOC() failed.");
      goto success ;
    }

    if(dx_sqlite_connect(fname , dbconn ) == false)
    {
      error = dx_string_createU(error,dbconn->error->stringa);
      free(dbconn);
      goto success ;
    }

   if(dbconn != NULL)
   {
    PHDR_DATABASE_CONN db_c = (PHDR_DATABASE_CONN)malloc(sizeof(struct hdr_database_conn)) ;
    db_c->type     = hdr_db_sqlite ;
    db_c->conn_obj = (void*)dbconn ;
    hdr_var_set_obj(*result,db_c) ;/*all ok*/
   }

    success:
    hdr_sys_func_free_params(params) ;
    return false ;

    fail :     
    printf("The system functionDatabase.SQLite($filename:String , $doCreate : Boolean, out -> $error : String):SQLite Connection failed.\n");
    hdr_sys_func_free_params(params) ;
    return true ;
}


bool hdr_domMariaDBOpen(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token, PHDR_VAR *result)
{
   PHDR_SYS_FUNC_PARAMS params = hdr_sys_func_init_params(inter,token->parameters,6) ;
   if(params == NULL)
   {
    printf("The system function Database.MariaDB($host:String , $user : String, $pass : String , $db : String , $port: Integer, out -> $error: String):MariaDB failed.\n");
    return true ;
   }

   bool type_error = false ;

   PDX_STRING host = hdr_inter_ret_string(params->params[0],&type_error) ; 
   if(type_error == true)
   {
	 printf("The first parameter must be a String.\n");
     goto fail ;
   }

   PDX_STRING user = hdr_inter_ret_string(params->params[1],&type_error) ; 
   if(type_error == true)
   {
	 printf("The second parameter must be a String.\n");
     goto fail ;
   }

   PDX_STRING pass = hdr_inter_ret_string(params->params[2],&type_error) ; 
   if(type_error == true)
   {
	 printf("The third parameter must be a String.\n");
     goto fail ;
   }

   PDX_STRING db = hdr_inter_ret_string(params->params[3],&type_error) ; 
   if(type_error == true)
   {
	 printf("The fourth parameter must be a String.\n");
     goto fail ;
   }

   DXLONG64 port = hdr_inter_ret_integer(params->params[4],&type_error) ; 
   if(type_error == true)
   {
	 printf("The fith parameter must be an Integer.\n");
     goto fail ;
   }

   PDX_STRING error = hdr_inter_ret_string(params->params[5],&type_error) ; 
   if(type_error == true)
   {
	 printf("The sixth parameter must be a String.\n");
     goto fail ;
   }

   error = dx_string_createU(error,"");

   *result            = hdr_var_create(NULL, "", hvf_temporary_ref, NULL) ;
   (*result)->type    = hvt_database ; 

    /*connect to the database*/
    
    PDX_STRING null = dx_string_createU(NULL,"")        ;
    PDX_STRING charset = dx_string_createU(NULL,"utf8mb4") ; /*we support only utf charsets*/
    MYSQL *sql = dx_mariadb_connect(host,user,pass,db,(unsigned int)port,null,0,charset,error) ;

    dx_string_free(null)     ;
    dx_string_free(charset)  ;

    
   if(sql != NULL)
   {
     PHDR_DATABASE_CONN db_c = (PHDR_DATABASE_CONN)malloc(sizeof(struct hdr_database_conn)) ;
     db_c->type     = hdr_db_mariadb ;
     db_c->conn_obj = (void*)sql     ;
     hdr_var_set_obj(*result,db_c)  ;
   }
    success:
    hdr_sys_func_free_params(params) ;
    return false ;

    fail :     
    printf("The system function Database.MariaDB($host:String , $user : String, $pass : String , $db : String , $port: Integer, out -> $error: String):MariaDB failed.\n");
    hdr_sys_func_free_params(params) ;
    return true ;
}


bool hdr_domODBCOpen(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token, PHDR_VAR *result)
{
   PHDR_SYS_FUNC_PARAMS params = hdr_sys_func_init_params(inter,token->parameters,3) ;
   if(params == NULL)
   {
    printf("The system function Database.ODBC($connection_string: String, out -> $error: String,blob_as_string : Boolean):ODBC Connection failed.\n");
    return true ;
   }

   bool type_error = false ;

   PDX_STRING conn_str = hdr_inter_ret_string(params->params[0],&type_error) ; 
   if(type_error == true)
   {
	 printf("The first parameter must be a String.\n");
     goto fail ;
   }

   PDX_STRING error = hdr_inter_ret_string(params->params[1],&type_error) ; 
   if(type_error == true)
   {
	 printf("The second parameter must be a String.\n");
     goto fail ;
   }

   bool blob_as_string = hdr_inter_ret_bool(params->params[2],&type_error) ; 
   if(type_error == true)
   {
	 printf("The third parameter must be a Boolean.\n");
     goto fail ;
   }

   error = dx_string_createU(error,"");

   *result            = hdr_var_create(NULL, "", hvf_temporary_ref, NULL) ;
   (*result)->type    = hvt_database ; 

    /* 
      connect to the database
      the ODBC connection needs the connection string as a widestring .
      So we will suply it
    */
    PDX_STRING wconstr = dx_string_convertW(conn_str) ;
    PDX_ODBC   odbc    =  dx_odbc_init(wconstr,error,blob_as_string) ;
    dx_string_free(wconstr);
    
   if(odbc != NULL)
   {
     PHDR_DATABASE_CONN db_c = (PHDR_DATABASE_CONN)malloc(sizeof(struct hdr_database_conn)) ;
     db_c->type     = hdr_db_odbc     ;
     db_c->conn_obj = (void*)odbc     ;
     hdr_var_set_obj(*result,db_c)  ;
   }

    success:
    hdr_sys_func_free_params(params) ;
    return false ;

    fail :     
    printf("The system function Database.ODBC($connection_string: String, out -> $error: String,blob_as_string : Boolean):ODBC Connection failed.\n");
    hdr_sys_func_free_params(params) ;
    return true ;
}


bool hdr_dom_db_exec(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token,PHDR_VAR for_var, PHDR_VAR *result)
{
   PHDR_SYS_FUNC_PARAMS params = hdr_sys_func_init_params(inter,token->parameters,1) ;
   if(params == NULL)
   {
    printf("The system function Database.Exec($query): String -> Error failed.\n");
    return true ;
   }

   bool type_error = false ;
   PDX_STRING query = hdr_inter_ret_string(params->params[0],&type_error) ; 
   if(type_error == true)
   {
	 printf("The first parameter must be a String.\n");
     goto fail ;
   }


   *result            = hdr_var_create(NULL, "", hvf_temporary_ref, NULL) ;
   (*result)->type    = hvt_simple_string ; 
   PDX_STRING str     = dx_string_createU(NULL,"");
   hdr_var_set_obj(*result,str) ;

    if(for_var->obj != NULL)
    {
      PHDR_DATABASE_CONN db = (PHDR_DATABASE_CONN)for_var->obj ;
      switch(db->type)
      {
          case hdr_db_sqlite :
          {
            if (dx_sqlite_execute((PDX_SQLITE)db->conn_obj,query) == false)
            {
              PDX_SQLITE sqlt = (PDX_SQLITE)db->conn_obj ;
              PDX_STRING ner = dx_string_createU((PDX_STRING)((*result)->obj),sqlt->error->stringa) ;
              hdr_var_set_obj(*result,ner) ;
            }
          }break ;
          case hdr_db_mariadb :
          {
            PDX_STRING status = dx_string_createU(NULL,"");
            PDX_QUERY res = dx_mariadb_execute((MYSQL*)db->conn_obj ,query,status);
            if (res != NULL)
            {
              if(inter->warnings == true)
              {
                hdr_inter_print_warning(inter,"The query returned a dataset, this dataset released and returned to void.\n") ;
              }
              if(status->len > 0)
              {
                PDX_STRING nstr = dx_string_createU((PDX_STRING)((*result)->obj),status->stringa) ;
                hdr_var_set_obj(*result,nstr) ;
              }
              dx_string_free(status);
              dx_db_query_free(res);
            }
            else
            {
              if(status->len > 0)
              {
                PDX_STRING nstr = dx_string_createU((PDX_STRING)((*result)->obj),status->stringa);
                hdr_var_set_obj(*result,nstr) ;
              }
              dx_string_free(status);
            }

          }break ;
          case hdr_db_odbc :
          {
                PDX_STRING status = dx_string_createU(NULL,"");
                PDX_STRING wquery = dx_string_convertW(query) ;
                PDX_QUERY res     = dx_odbc_execute((PDX_ODBC)db->conn_obj ,wquery,status);
                dx_string_free(wquery);
                if (res != NULL)
                {
                  if(inter->warnings == true)
                  {
                    hdr_inter_print_warning(inter,"The query returned a dataset, this dataset released and returned to void.\n") ;
                  }
                  if(status->len > 0)
                  {
                    PDX_STRING nstr = dx_string_createU((PDX_STRING)((*result)->obj),status->stringa);
                    hdr_var_set_obj(*result,nstr) ;
                  }
                  dx_string_free(status);
                  dx_db_query_free(res);
                }
                else
                {
                  if(status->len > 0)
                  {
                    PDX_STRING nstr = dx_string_createU((PDX_STRING)((*result)->obj),status->stringa);
                    hdr_var_set_obj(*result,nstr) ;
                  }
                  dx_string_free(status);
                }

          } break ;

      }

    }
    else
    {
         PDX_STRING nstr = dx_string_createU((PDX_STRING)((*result)->obj),"The connection is invalid. Are you sure that the connection was successful?");
         hdr_var_set_obj(*result,nstr) ;
    }

    success:
    hdr_sys_func_free_params(params) ;
    return false ;

    fail : 
    printf("The system function Database.Exec($query): String -> Error failed.\n");
    hdr_sys_func_free_params(params) ;
    return true ;
}



bool hdr_dom_db_is_valid(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token,PHDR_VAR for_var,PHDR_VAR *result)
{
   PHDR_SYS_FUNC_PARAMS params = hdr_sys_func_init_params(inter,token->parameters,0) ;
   if(params == NULL)
   {
    printf("The system function Database.IsValid():Boolean failed.\n");
    return true ;
   }

   *result            = hdr_var_create(NULL, "", hvf_temporary_ref, NULL) ;
   (*result)->type    = hvt_bool    ; 
   (*result)->integer = 0           ;

    if(for_var->obj != NULL) (*result)->integer = 1 ; 

    success:
    hdr_sys_func_free_params(params) ;
    return false ;

    fail : 
    printf("The system function Database.isValid():Boolean failed.\n");
    hdr_sys_func_free_params(params) ;
    return true ;
}


bool hdr_dom_db_close(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token,PHDR_VAR for_var)
{
   PHDR_SYS_FUNC_PARAMS params = hdr_sys_func_init_params(inter,token->parameters,0) ;
   if(params == NULL)
   {
    printf("The system function Database.Close() failed.\n");
    return true ;
   }

    if(for_var->obj != NULL)
    {
      PHDR_DATABASE_CONN db = (PHDR_DATABASE_CONN)for_var->obj ;
      switch(db->type)
      {
          case hdr_db_sqlite :
          {
            if(db->conn_obj != NULL)
            {
                if (dx_sqlite_close((PDX_SQLITE)db->conn_obj) == false)
                {
                 if (inter->warnings == true)
                 {
                  hdr_inter_print_warning(inter,"Error while closing the SQLite database connection.");
                 }
                }
                db->conn_obj = NULL ;
            }
          }break ;
          case hdr_db_mariadb :
          {
            if(db->conn_obj != NULL)
            {
              dx_mariadb_disconnect((MYSQL*)db->conn_obj);
              db->conn_obj = NULL ;
            }
          }break ;
          case hdr_db_odbc :
          {
            if(db->conn_obj != NULL)
            {
              dx_odbc_free((PDX_ODBC)db->conn_obj);
              db->conn_obj = NULL ;
            }
          } break ;

      }

    }
    else
    {
        if(inter->warnings == true){hdr_inter_print_warning(inter,"The database connection that you are trying to close is not valid or already closed.");}
    }

    success:
    hdr_sys_func_free_params(params) ;
    return false ;

    fail : 
    printf("The system function Database.Close() failed.\n");
    hdr_sys_func_free_params(params) ;
    return true ;
}


bool hdr_dom_db_free(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token,PHDR_VAR for_var)
{
        PHDR_SYS_FUNC_PARAMS params = hdr_sys_func_init_params(inter,token->parameters,0) ;
        if(params == NULL)
        {
         printf("The system function Database.Free().\n");
         return true ;
        } 

        /*close the database first*/
        if(for_var->obj != NULL)
        {
          PHDR_DATABASE_CONN db = (PHDR_DATABASE_CONN)for_var->obj ;
          switch(db->type)
          {
              case hdr_db_sqlite :
              {
                if(db->conn_obj != NULL)
                {
                    if (dx_sqlite_close((PDX_SQLITE)db->conn_obj) == false)
                    {
                     if (inter->warnings == true)
                     {
                      hdr_inter_print_warning(inter,"Error while clossing the SQLite database connection.");
                     }
                    }
                    db->conn_obj = NULL ;
                }
              }break ;
              case hdr_db_mariadb :
              {
                   if(db->conn_obj != NULL)
                    {
                      dx_mariadb_disconnect((MYSQL*)db->conn_obj) ;
                      db->conn_obj = NULL ;
                    }
              }break ;
              case hdr_db_odbc :
              {
                 if(db->conn_obj != NULL)
                 {
                    dx_odbc_free((PDX_ODBC)db->conn_obj) ;
                    db->conn_obj = NULL ;
                 }
              } break ;

          }
        }

    free(for_var->obj) ;
    hdr_var_release_obj(for_var) ;
    for_var->type  = hvt_undefined ;

    success:
    hdr_sys_func_free_params(params) ;
    return false ;

    fail : 
    printf("The system function Database.Free() failed.\n");
    hdr_sys_func_free_params(params) ;
    return true ;

}

PHDR_DATABASE_CONN hdr_dom_db_conn_free(PHDR_DATABASE_CONN db)
{
        /*close the database first*/
        if(db != NULL)
        {
          
          switch(db->type)
          {
              case hdr_db_sqlite :
              {
                if(db->conn_obj != NULL)
                {
                    dx_sqlite_close((PDX_SQLITE)db->conn_obj) ;
                    db->conn_obj = NULL ;
                }
              }break ;
              case hdr_db_mariadb :
              {
                if(db->conn_obj != NULL)
                {
                    dx_mariadb_disconnect((MYSQL*)db->conn_obj) ;
                    db->conn_obj = NULL ;
                }
              }break ;
              case hdr_db_odbc :
              {
                if(db->conn_obj != NULL)
                {
                    dx_odbc_free((PDX_ODBC)db->conn_obj) ;
                    db->conn_obj = NULL ;
                }
              } break ;

          }
        }
    
        free(db);
    return NULL ;
}



/****************************** DATASET *******************************/

/*the dataset creation routine is the .Query() function of the Database domain */


bool hdr_dom_db_ret_dataset(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token,PHDR_VAR for_var, PHDR_VAR *result)
{
   PHDR_SYS_FUNC_PARAMS params = hdr_sys_func_init_params(inter,token->parameters,2) ;
   if(params == NULL)
   {
    printf("The system function Database.Query($query,out -> $error): Dataset failed.\n");
    return true ;
   }

   bool type_error = false ;
   PDX_STRING query = hdr_inter_ret_string(params->params[0],&type_error) ; 
   if(type_error == true)
   {
	 printf("The first parameter must be a String.\n");
     goto fail ;
   }

   PDX_STRING error = hdr_inter_ret_string(params->params[1],&type_error) ; 
   if(type_error == true)
   {
	 printf("The second parameter must be a String.\n");
     goto fail ;
   }
   error = dx_string_createU(error,"");

   *result            = hdr_var_create(NULL, "", hvf_temporary_ref, NULL) ;
   (*result)->type    = hvt_dataset ; 

    if(for_var->obj != NULL)
    {
      PHDR_DATABASE_CONN db = (PHDR_DATABASE_CONN)for_var->obj ;
      switch(db->type)
      {
          case hdr_db_sqlite :
          {
            if(db->conn_obj != NULL)
            {
               PDX_SQLITE slt = (PDX_SQLITE)db->conn_obj ;
               PDX_QUERY dataset = dx_sqlite_select(slt , query) ;
               if(dataset == NULL)
               {
                   error = dx_string_createU(error,slt->error->stringa) ;
                   goto success ;
               }

               hdr_var_set_obj(*result,dataset) ;

            }
            else 
            {
                if(inter->warnings == true){hdr_inter_print_warning(inter,"The SQLite connection is not valid. No action can be done in an invalid connection.");}
            }
          }break ;
          case hdr_db_mariadb :
          {
               if(db->conn_obj != NULL)
                {
                   MYSQL *msql = (MYSQL*)db->conn_obj ;
                   PDX_QUERY dataset = dx_mariadb_execute(msql,query,error) ;
                   hdr_var_set_obj(*result,dataset) ;
                }
                else 
                {
                    if(inter->warnings == true){hdr_inter_print_warning(inter,"The Mariadb connection is not valid. No action can be done in an invalid connection.");}
                }
          }break ;
          case hdr_db_odbc :
          {
            if(db->conn_obj != NULL)
            {
               PDX_ODBC odbc      = (PDX_ODBC)db->conn_obj ;
               /*ODBC needs thw string to be in widestring*/
               PDX_STRING wquery = dx_string_convertW(query) ;
               PDX_QUERY dataset  = dx_odbc_execute(odbc,wquery,error) ;
               dx_string_free(wquery);
               hdr_var_set_obj(*result,dataset) ;
            }
            else 
               {
                  if(inter->warnings == true){hdr_inter_print_warning(inter,"The ODBC connection is not valid. No action can be done in an invalid connection.");}
               }
          } break ;

      }

    }
    else
    {
      error = dx_string_createU(error,"The connection is invalid. Are you sure that the connection was successful?");
      hdr_var_set_obj(*result,NULL);
    }

    success:
    hdr_sys_func_free_params(params) ;
    return false ;

    fail : 
    printf("The system function Database.Query($query,$error): String -> Error failed.\n");
    hdr_sys_func_free_params(params) ;
    return true ;
}


bool hdr_dom_ds_fields_count(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token,PHDR_VAR for_var,PHDR_VAR *result)
{
   PHDR_SYS_FUNC_PARAMS params = hdr_sys_func_init_params(inter,token->parameters,0) ;
   if(params == NULL)
   {
    printf("The system function Dataset.FieldsCount():Integer failed.\n");
    return true ;
   }


   *result            = hdr_var_create(NULL, "", hvf_temporary_ref, NULL) ;
   (*result)->type    = hvt_integer    ; 
   (*result)->integer = 0           ;

    if(for_var->obj == NULL) (*result)->integer = -1 ; 
    else
    {
      PDX_QUERY q = (PDX_QUERY)for_var->obj ;
      (*result)->integer = q->header->count ;
    }

    success:
    hdr_sys_func_free_params(params) ;
    return false ;

    fail : 
    printf("The system function Dataset.FieldsCount():Integer failed.\n");
    hdr_sys_func_free_params(params) ;
    return true ;
}


bool hdr_dom_ds_field_name(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token,PHDR_VAR for_var,PHDR_VAR *result)
{
   PHDR_SYS_FUNC_PARAMS params = hdr_sys_func_init_params(inter,token->parameters,1) ;
   if(params == NULL)
   {
    printf("The system function Dataset.FieldName($indx:Integer):String failed.\n");
    return true ;
   }

   bool type_error ;
   DXLONG64 indx = hdr_inter_ret_integer(params->params[0],&type_error) ; 
   if(type_error == true)
   {
	 printf("The first parameter must be an Integer.\n");
     goto fail ;
   }

   *result            = hdr_var_create(NULL, "", hvf_temporary_ref, NULL) ;
   (*result)->type    = hvt_simple_string                                 ; 
   (*result)->obj     = (void*)dx_string_createU(NULL,"")                 ;

    if(for_var->obj == NULL) 
    {
       goto success ; 
    }
    else
    {
      PDX_QUERY q = (PDX_QUERY)for_var->obj ;
      if((indx <0)||(indx >= q->header->count))
      {
          hdr_inter_print_warning(inter,"The indx for the fields name is not in the dataset range.") ;
          goto success ; 
      }

       PDX_STRING name = dx_db_field_name(q,indx) ;
       if(name == NULL) goto success ;
       (*result)->obj = dx_string_free((PDX_STRING)(*result)->obj) ;
       hdr_var_set_obj(*result,name);
    }

    success:
    hdr_sys_func_free_params(params) ;
    return false ;

    fail : 
    printf("The system function Dataset.FieldName($indx:Integer):String failed.\n");
    hdr_sys_func_free_params(params) ;
    return true ;
}

bool hdr_dom_ds_field_gen_type(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token,PHDR_VAR for_var,PHDR_VAR *result)
{
   PHDR_SYS_FUNC_PARAMS params = hdr_sys_func_init_params(inter,token->parameters,1) ;
   if(params == NULL)
   {
    printf("The system function Dataset.FieldGenericType($indx:Integer):Integer failed.\n");
    return true ;
   }

   bool type_error ;
   DXLONG64 indx = hdr_inter_ret_integer(params->params[0],&type_error) ; 
   if(type_error == true)
   {
	 printf("The first parameter must be an Integer.\n");
     goto fail ;
   }

   *result            = hdr_var_create(NULL, "", hvf_temporary_ref, NULL) ;
   (*result)->type    = hvt_integer                                       ; 
   (*result)->integer = 0                                                 ;

    if(for_var->obj == NULL) 
    {
       goto success ; 
    }
    else
    {
      PDX_QUERY q = (PDX_QUERY)for_var->obj ;
      if((indx <0)||(indx >= q->header->count))
      {
          hdr_inter_print_warning(inter,"The indx for the field type is not in the dataset range.") ;
          goto success ; 
      }

      PDXL_NODE node = dx_list_go_to_pos(q->header,indx) ;
      if(node == NULL) 
      {
        (*result)->integer = DX_FIELD_VALUE_NULL ;
        goto success ;
      }

      PDXL_OBJECT obj    = node->object ;
      (*result)->integer =   obj->flags ;
	        
    }

    success:
    hdr_sys_func_free_params(params) ;
    return false ;

    fail : 
    printf("The system function Dataset.FieldGenericType():Integer failed.\n");
    hdr_sys_func_free_params(params) ;
    return true ;
}


bool hdr_dom_ds_field_type(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token,PHDR_VAR for_var,PHDR_VAR *result)
{
   PHDR_SYS_FUNC_PARAMS params = hdr_sys_func_init_params(inter,token->parameters,1) ;
   if(params == NULL)
   {
    printf("The system function Dataset.FieldType($indx:Integer):String failed.\n");
    return true ;
   }

   bool type_error ;
   DXLONG64 indx = hdr_inter_ret_integer(params->params[0],&type_error) ; 
   if(type_error == true)
   {
	 printf("The first parameter must be an Integer.\n");
     goto fail ;
   }

   *result            = hdr_var_create(NULL, "", hvf_temporary_ref, NULL) ;
   (*result)->type    = hvt_simple_string                                 ; 
   PDX_STRING nstr =dx_string_createU(NULL,"");    
   hdr_var_set_obj(*result,nstr);

    if(for_var->obj == NULL) 
    {
       goto success ; 
    }
    else
    {
      PDX_QUERY q = (PDX_QUERY)for_var->obj ;
      if((indx <0)||(indx >= q->header->count))
      {
          hdr_inter_print_warning(inter,"The indx for the field type is not in the dataset range.") ;
          goto success ; 
      }

      PDXL_NODE node = dx_list_go_to_pos(q->header,indx) ;
      if(node == NULL) 
      {
        (*result)->integer = DX_FIELD_VALUE_NULL ;
        goto success ;
      }

      PDXL_OBJECT obj  = node->object ;      
      PDX_STRING nstr = dx_string_createU((PDX_STRING)(*result)->obj,((PDX_STRING)obj->obj)->stringa) ;
      hdr_var_set_obj(*result,nstr);
    }

    success:
    hdr_sys_func_free_params(params) ;
    return false ;

    fail : 
    printf("The system function Dataset.FieldType():String failed.\n");
    hdr_sys_func_free_params(params) ;
    return true ;
}


bool hdr_dom_ds_to_json(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token,PHDR_VAR for_var,PHDR_VAR *result)
{
   PHDR_SYS_FUNC_PARAMS params = hdr_sys_func_init_params(inter,token->parameters,0) ;
   if(params == NULL)
   {
    printf("The system function Dataset.ToJson():String failed.\n");
    return true ;
   }


   *result            = hdr_var_create(NULL, "", hvf_temporary_ref, NULL) ;
   (*result)->type    = hvt_simple_string                                 ; 
   PDX_STRING nstr    = dx_string_createU(NULL,"");    
   hdr_var_set_obj(*result,nstr);

    if(for_var->obj == NULL) 
    {
       goto success ; 
    }
    else
    {
      PDX_QUERY q = (PDX_QUERY)for_var->obj ;
      dx_string_free(nstr);
      nstr = hdrDatasetToJson(q);
      hdr_var_set_obj(*result,nstr);
    }

    success:
    hdr_sys_func_free_params(params) ;
    return false ;

    fail : 
    printf("The system function Dataset.ToJson():String failed.\n");
    hdr_sys_func_free_params(params) ;
    return true ;
}

bool hdr_dom_ds_rows_count(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token,PHDR_VAR for_var,PHDR_VAR *result)
{
   PHDR_SYS_FUNC_PARAMS params = hdr_sys_func_init_params(inter,token->parameters,0) ;
   if(params == NULL)
   {
    printf("The system function Dataset.RowsCount():Integer failed.\n");
    return true ;
   }

   *result            = hdr_var_create(NULL, "", hvf_temporary_ref, NULL) ;
   (*result)->type    = hvt_integer    ; 
   (*result)->integer = 0           ;

    if(for_var->obj == NULL) (*result)->integer = -1 ; 
    else
    {
      PDX_QUERY q = (PDX_QUERY)for_var->obj ;
      (*result)->integer = q->row_count ;
    }

    success:
    hdr_sys_func_free_params(params) ;
    return false ;

    fail : 
    printf("The system function Dataset.RowsCount():Integer failed.\n");
    hdr_sys_func_free_params(params) ;
    return true ;
}


bool hdr_dom_ds_is_valid(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token,PHDR_VAR for_var,PHDR_VAR *result)
{
   PHDR_SYS_FUNC_PARAMS params = hdr_sys_func_init_params(inter,token->parameters,0) ;
   if(params == NULL)
   {
    printf("The system function Dataset.IsValid():Boolean failed.\n");
    return true ;
   }

   *result            = hdr_var_create(NULL, "", hvf_temporary_ref, NULL) ;
   (*result)->type    = hvt_bool    ; 
   (*result)->integer = 0           ;

    if(for_var->obj != NULL) (*result)->integer = 1 ; 

    success:
    hdr_sys_func_free_params(params) ;
    return false ;

    fail : 
    printf("The system function Dataset.isValid():Boolean failed.\n");
    hdr_sys_func_free_params(params) ;
    return true ;
}


bool hdr_dom_ds_free(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token,PHDR_VAR for_var)
{
        PHDR_SYS_FUNC_PARAMS params = hdr_sys_func_init_params(inter,token->parameters,0) ;
        if(params == NULL)
        {
         printf("The system function Dataset.Free().\n");
         return true ;
        } 

        if(for_var->obj != NULL)
        {
            dx_db_query_free((PDX_QUERY)for_var->obj) ;
            hdr_var_release_obj(for_var) ;
        }
      
    for_var->type  = hvt_undefined ;

    success:
    hdr_sys_func_free_params(params) ;
    return false ;

    fail : 
    printf("The system function Dataset.Free() failed.\n");
    hdr_sys_func_free_params(params) ;
    return true ;

}


PDX_QUERY hdr_dom_dataset_free(PDX_QUERY query)
{
    if(query != NULL)
    {
        dx_db_query_free(query) ;
    }
    return NULL ;
}


/****************** DATA ROW *******************************/

bool hdr_dom_dr_is_valid(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token,PHDR_VAR for_var,PHDR_VAR *result)
{
   PHDR_SYS_FUNC_PARAMS params = hdr_sys_func_init_params(inter,token->parameters,0) ;
   if(params == NULL)
   {
    printf("The system function DataRow.IsValid():Boolean failed.\n");
    return true ;
   }

   *result            = hdr_var_create(NULL, "", hvf_temporary_ref, NULL) ;
   (*result)->type    = hvt_bool    ; 
   (*result)->integer = 0           ;

    if(for_var->obj != NULL) (*result)->integer = 1 ; 

    success:
    hdr_sys_func_free_params(params) ;
    return false ;

    fail : 
    printf("The system function DataRow.isValid():Boolean failed.\n");
    hdr_sys_func_free_params(params) ;
    return true ;
}


bool hdr_dom_dr_free(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token,PHDR_VAR for_var)
{
        PHDR_SYS_FUNC_PARAMS params = hdr_sys_func_init_params(inter,token->parameters,0) ;
        if(params == NULL)
        {
         printf("The system function DataRow.Free().\n");
         return true ;
        } 

        if(for_var->obj != NULL)
        {
            free(for_var->obj) ;
            hdr_var_release_obj(for_var) ;
        }
      
    for_var->type  = hvt_undefined ;

    success:
    hdr_sys_func_free_params(params) ;
    return false ;

    fail : 
    printf("The system function DataRow.Free() failed.\n");
    hdr_sys_func_free_params(params) ;
    return true ;

}

PDX_DB_ROW_WRAP hdr_dom_datarow_free(PDX_DB_ROW_WRAP row)
{
    if(row != NULL)
    {
        free(row) ;
    }  
    return NULL ;
}





/**************** UTLITY FUNCTIONS **********************/

bool hdr_funcasString(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token, PHDR_VAR *result)
{
   PHDR_SYS_FUNC_PARAMS params = hdr_sys_func_init_params(inter,token->parameters,1) ;
   if(params == NULL)
   {
    printf("The system function asString($var : variable):String failed.\n");
    return true ;
   }
   
   PHDR_VAR var = params->params[0] ; 
   PDX_STRING str = NULL ;
   enum hdr_inter_vt gtype = hdr_inter_var_gen_type(var) ;
   if(gtype == hdr_ivt_string)
   {
     if(var->type == hvt_unicode_string)
     str = dx_string_convertU((PDX_STRING)var->obj);
     else
         str = dx_string_createU(NULL,((PDX_STRING)var->obj)->stringa) ;
   }
   else
    if(gtype == hdr_ivt_numeric)
    {
        if (var->type == hvt_integer)
        {
          str = dx_IntToStr(var->integer) ;
        }
        else
        if (var->type == hvt_float)
        {
          str = dx_FloatToStr(var->real,9) ;
        }
        else 
            if(var->type == hvt_bool)
            {
            if(var->integer == 0) 
            str = dx_string_createU(NULL,"false") ;
            else 
                str = dx_string_createU(NULL,"true") ;
            }
    }
    else
    if(var->type == hvt_null)
    {
        str = dx_string_createU(NULL,"NULL") ;
    }
    else
    {
        str = dx_string_createU(NULL,"[Unknown]") ;
    }

   *result = hdr_var_create(NULL, "", hvf_temporary_ref, NULL) ;
   (*result)->type    = hvt_simple_string  ; 
   hdr_var_set_obj(*result,str);

   success:
    hdr_sys_func_free_params(params) ;
    return false ;

   fail : 
    printf("The system function asString($var : variable):String failed.\n");
    hdr_sys_func_free_params(params) ;
    return true ;
}




























