/*
 Hydras+ various functions.

 Nikos Mourgis deus-ex.gr 2024

 Live Long and Prosper
*/



/************************** return the type of the variable as an int to be tested with the special define values ******************/
bool hdr_all_ret_type(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token,PHDR_VAR for_var,PHDR_VAR *result)
{
   PHDR_SYS_FUNC_PARAMS params = hdr_sys_func_init_params(inter,token->parameters,0) ;
   if(params == NULL)
   {
    printf("The system function Variable.Type():[Variable Type] failed.\n");
    return true ;
   }

   *result            = hdr_var_create(NULL, "", hvf_temporary_ref, NULL) ;
   (*result)->type    = hvt_integer                                       ; 
   
   switch(for_var->type)
   {
        case hvt_undefined :  (*result)->integer = HDR_VAR_UNDEFINED	;break;				
        case hvt_pointer   :  (*result)->integer = HDR_VAR_POINTER	    ;break;					 
        case hvt_float     :  (*result)->integer = HDR_VAR_FLOAT        ;break;						
        case hvt_integer   :  (*result)->integer = HDR_VAR_INTEGER	    ;break;					
        case hvt_null      :  (*result)->integer = HDR_VAR_NULL	        ;break;					
        case hvt_bool      :  (*result)->integer = HDR_VAR_BOOL         ;break;						
        case hvt_list      :  (*result)->integer = HDR_VAR_LIST         ;break;						
        case hvt_string_list      : (*result)->integer = HDR_VAR_STRINGLIST   ;break;					
        case hvt_string_list_sort : (*result)->integer = HDR_VAR_STR_LIST_SORT;break;				 
        case hvt_int_list_sort    : (*result)->integer = HDR_VAR_INT_LIST_SORT;break;				
        case hvt_float_list_sort  :  (*result)->integer = HDR_VAR_FLOAT_LIST_SORT;break;					
        case hvt_fast_list        :  (*result)->integer = HDR_VAR_FAST_LIST      ;break;					
        case hvt_http             :  (*result)->integer = HDR_VAR_HTTP           ;break;						
        case hvt_database         :  (*result)->integer = HDR_VAR_DATABASE       ;break;					
        case hvt_dataset          :  (*result)->integer = HDR_VAR_DATASET        ;break;							
        case hvt_data_row         :  (*result)->integer = HDR_VAR_DATA_ROW       ;break;					
        case hvt_tcp_socket_client:  (*result)->integer = HDR_VAR_TCP_SOCK_CL    ;break;					
        case hvt_tcp_socket_server:  (*result)->integer = HDR_VAR_TCP_SOCK_SRV   ;break;				
        case hvt_ssl_client       :  (*result)->integer = HDR_VAR_SSL_CLIENT     ;break;					
        case hvt_ssl_server       :  (*result)->integer = HDR_VAR_SSL_SRV        ;break;						
        case hvt_bytes            :  (*result)->integer = HDR_VAR_BYTES          ;break;						
        case hvt_file             :  (*result)->integer = HDR_VAR_FILE           ;break;						
        case hvt_simple_string    :  (*result)->integer = HDR_VAR_SIMPLE_STR     ;break;					
        case hvt_simple_string_bcktck   :  (*result)->integer = HDR_VAR_SIMPLE_STR_BCK;break;				
        case hvt_complex_string         :  (*result)->integer = HDR_VAR_CMPLX_STR     ;break;					
        case hvt_complex_string_resolve :  (*result)->integer = HDR_VAR_STR_EXPAND    ;break;					
        case hvt_unicode_string         :  (*result)->integer = HDR_VAR_UNICODE_STR   ;break;					
        case hvt_codepoint              :  (*result)->integer = HDR_VAR_CODEPOINT     ;break;					
        case hvt_object                 :  (*result)->integer = HDR_VAR_OBJECT        ;break;						

        default : (*result)->integer = -1 ;
   }


    success:
    hdr_sys_func_free_params(params) ;
    return false ;

    fail : 
    printf("The system function Variable.Type():[Variable Type] failed.\n");
    hdr_sys_func_free_params(params) ;
    return true ;
}

/************************** return the type of the variable as a string ******************/
bool hdr_all_ret_type_str(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token,PHDR_VAR for_var,PHDR_VAR *result)
{
   PHDR_SYS_FUNC_PARAMS params = hdr_sys_func_init_params(inter,token->parameters,0) ;
   if(params == NULL)
   {
    printf("The system function Variable.TypeAsStr():[String] failed.\n");
    return true ;
   }

   *result            = hdr_var_create(NULL, "", hvf_temporary_ref, NULL) ;
   (*result)->type    = hvt_simple_string                                 ; 

   switch(for_var->type)
   {
        case hvt_undefined :  (*result)->obj     = dx_string_createU(NULL,"undefined")  	;break;				
        case hvt_pointer   :  (*result)->obj     = dx_string_createU(NULL,"pointer") 	    ;break;					 
        case hvt_float     :  (*result)->obj     = dx_string_createU(NULL,"real")           ;break;						
        case hvt_integer   :  (*result)->obj     = dx_string_createU(NULL,"integer") 	    ;break;					
        case hvt_null      :  (*result)->obj     = dx_string_createU(NULL,"null")           ;break;					
        case hvt_bool      :  (*result)->obj     = dx_string_createU(NULL,"boolean")        ;break;						
        case hvt_list      :  (*result)->obj     = dx_string_createU(NULL,"simple list")    ;break;						
        case hvt_string_list      : (*result)->obj     = dx_string_createU(NULL,"stringlist") ;break;					
        case hvt_string_list_sort : (*result)->obj     = dx_string_createU(NULL,"stringlist sort") ;break;				 
        case hvt_int_list_sort    : (*result)->obj     = dx_string_createU(NULL,"int list sort") ;break;				
        case hvt_float_list_sort  : (*result)->obj     = dx_string_createU(NULL,"real list sort") ;break;					
        case hvt_fast_list        :  (*result)->obj     = dx_string_createU(NULL,"fast list") ;break;					
        case hvt_http             :  (*result)->obj     = dx_string_createU(NULL,"http") ;break;						
        case hvt_database         :  (*result)->obj     = dx_string_createU(NULL,"database") ;break;					
        case hvt_dataset          :  (*result)->obj     = dx_string_createU(NULL,"dataset") ;break;							
        case hvt_data_row         :  (*result)->obj     = dx_string_createU(NULL,"data row") ;break;					
        case hvt_tcp_socket_client:  (*result)->obj     = dx_string_createU(NULL,"socket client") ;break;					
        case hvt_tcp_socket_server:  (*result)->obj     = dx_string_createU(NULL,"socket server") ;break;				
        case hvt_ssl_client       :  (*result)->obj     = dx_string_createU(NULL,"ssl client") ;break;					
        case hvt_ssl_server       :  (*result)->obj     = dx_string_createU(NULL,"ssl server") ;break;						
        case hvt_bytes            :  (*result)->obj     = dx_string_createU(NULL,"bytes") ;break;						
        case hvt_file             :  (*result)->obj     = dx_string_createU(NULL,"file") ;break;						
        case hvt_simple_string    :  (*result)->obj     = dx_string_createU(NULL,"simple string") ;break;					
        case hvt_simple_string_bcktck   :  (*result)->obj     = dx_string_createU(NULL,"simple string backtick") ;break;				
        case hvt_complex_string         :  (*result)->obj     = dx_string_createU(NULL,"complex string") ;break;					
        case hvt_complex_string_resolve :  (*result)->obj     = dx_string_createU(NULL,"complex string expand") ;break;					
        case hvt_unicode_string         :  (*result)->obj     = dx_string_createU(NULL,"unicode string") ;break;					
        case hvt_codepoint              :  (*result)->obj     = dx_string_createU(NULL,"codepoint") ;break;					
        case hvt_object                 :  (*result)->obj     = dx_string_createU(NULL,"object") ;break;						

        default :(*result)->obj     = dx_string_createU(NULL,"unknown") ;
   }


    success:
    hdr_sys_func_free_params(params) ;
    return false ;

    fail : 
    printf("The system function Variable.TypeAsStr():[String] failed.\n");
    hdr_sys_func_free_params(params) ;
    return true ;
}


bool hdr_random(PHDR_INTERPRETER inter , PHDR_COMPLEX_TOKEN token, PHDR_VAR *result)
{
  /*
    Returns a pseudo random number 
   */
   PHDR_SYS_FUNC_PARAMS params = hdr_sys_func_init_params(inter,token->parameters,1) ;
   if(params == NULL)
   {
     printf("The system function random($seed : Integer):Integer failed.\n");
     return true ;
   }

   bool type_error = false ;
   DXLONG64 seed = hdr_inter_ret_integer(params->params[0],&type_error) ; 
   if(type_error == true)
   {
	 printf("The first parameter must be an Integer.\n");
     goto fail ;
   }

   srand(seed) ;
   *result = hdr_var_create(NULL, "", hvf_temporary_ref, NULL) ;
   (*result)->type        = hvt_integer ;  
   (*result)->integer     = rand()      ; 

   success:
    hdr_sys_func_free_params(params) ;
    return false ;

   fail : 
    printf("The system function random($seed : Integer):Integer failed.\n");
    hdr_sys_func_free_params(params) ;
    return true ;
}

bool hdr_random_id(PHDR_INTERPRETER inter , PHDR_COMPLEX_TOKEN token, PHDR_VAR *result)
{
  /*
    Returns a string with random capital latin characters
  */
   PHDR_SYS_FUNC_PARAMS params = hdr_sys_func_init_params(inter,token->parameters,1) ;
   if(params == NULL)
   {
     printf("The system function random_id($length : Integer):String failed.\n");
     return true ;
   }

   bool type_error = false ;
   DXLONG64 len = hdr_inter_ret_integer(params->params[0],&type_error) ; 
   if(type_error == true)
   {
	 printf("The first parameter must be an Integer.\n");
     goto fail ;
   }

   if(len > 128)
   {
    len = 128 ;
    hdr_inter_print_warning(inter,"The length of the random id that the random_id() produces cannot be bigger than 128 characters. The length was set to 128 characters.");
   }

   if(len < 2)
   {
    len = 2 ;
    hdr_inter_print_warning(inter,"The length of the random id that the random_id() produces cannot be smaller than 2 characters. The length was set to 2 characters.");
   }

   srand(dxGetTickCount()%4000000000) ;
   char *rid = (char*)malloc(len+1)   ;
   rid[len] = 0  ;
   for(int i = 0 ; i < len; i++)
   {
     rid[i] = 65 +(rand() %25) ;
   }

   
   *result = hdr_var_create(NULL, "", hvf_temporary_ref, NULL) ;
   (*result)->type        = hvt_simple_string                  ;  
   PDX_STRING ns          = dx_string_create_bU(rid)           ;
   hdr_var_set_obj(*result,ns)                                ;

   success:
    hdr_sys_func_free_params(params) ;
    return false ;

   fail : 
    printf("The system function random_id($length : Integer):String failed.\n");
    hdr_sys_func_free_params(params) ;
    return true ;
}


bool hdr_sleep(PHDR_INTERPRETER inter , PHDR_COMPLEX_TOKEN token, PHDR_VAR *result)
{
  /*
    Returns a string with random capital latin characters
  */
   PHDR_SYS_FUNC_PARAMS params = hdr_sys_func_init_params(inter,token->parameters,1) ;
   if(params == NULL)
   {
     printf("The system function sleep($milliseconds : Integer) failed.\n");
     return true ;
   }

   bool type_error = false ;
   DXLONG64 millis = hdr_inter_ret_integer(params->params[0],&type_error) ; 
   if(type_error == true)
   {
	 printf("The first parameter must be an Integer.\n");
     goto fail ;
   }

   dxSleep(millis);

   success:
    hdr_sys_func_free_params(params) ;
    return false ;

   fail : 
    printf("The system function sleep($milliseconds : Integer) failed.\n");
    hdr_sys_func_free_params(params) ;
    return true ;
}


bool hdr_set_cur_dir(PHDR_INTERPRETER inter , PHDR_COMPLEX_TOKEN token)
{
  /*
    Set as the current working directory the directory in the path
  */
   PHDR_SYS_FUNC_PARAMS params = hdr_sys_func_init_params(inter,token->parameters,1) ;
   if(params == NULL)
   {
     printf("The system function setCurrDir($dir : String) failed.\n");
     return true ;
   }

   bool type_error = false ;
   PDX_STRING dir = hdr_inter_ret_string(params->params[0],&type_error) ; 
   if(type_error == true)
   {
	 printf("The first parameter must be a String.\n");
     goto fail ;
   }

#ifdef _LINUX
    chdir(dir->stringa);
#endif

#ifdef _WIN32
       _chdir(dir->stringa);
#endif

   success:
    hdr_sys_func_free_params(params) ;
    return false ;

   fail : 
    printf("The system function setCurrDir($dir : String) failed.\n");
    hdr_sys_func_free_params(params) ;
    return true ;
}



bool hdr_func_set_from_chars(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token,PHDR_VAR *result)
{
	/*
	 Hydra+ does not provides an easy way to add characters into a string like \n\t etc, 
	 this function provides an easy way to create a string from simple ANSI characters
	 for example , to create an /r/n sequence the parameter must be "#13#10"
	*/

   PHDR_SYS_FUNC_PARAMS params = hdr_sys_func_init_params(inter,token->parameters,1) ;
   if(params == NULL)
   {
	 printf("The system function setFromChars(Chars : String[#13#10...]) failed.\n");
     return true ;
   }

    bool type_error = false ;
   
    PDX_STRING str  = hdr_inter_ret_string(params->params[0],&type_error) ;
    if(type_error == true)
    {
	  printf("The first parameter must be a String.\n");
      goto fail ;
    }

	/*normalize the input , remove the spaces*/
	char *nstr = dxRemoveChar(str->stringa,' ') ;
	/*support 1024 ansi characters */
	char chars[1024] ;
	/*parse the input*/
	char *nstrindx = nstr ;
	int   charscnt = 0    ;
	while(*nstrindx != 0)
	{
	 /*first get the # and then copy until the next number*/
	  char * elem = dxCopyStrToChar(&nstrindx,'#',"") ;
	  if(*nstrindx != 0) nstrindx++ ; /*the dxCopyStrToChar make the nstrindx to point to the c char and not the char after*/
	  if(elem[0] == 0)
	  {
		   free(elem);
		   continue ;
	  }
	  bool error = false ;
	  DXLONG64 ch = dx_string_to_int(elem,&error) ;
	  free(elem);
	  if(error == true)
	  {
	    printf("The input string with the characters is malformed. Please check the validity of the format. Example :`#13#10#30`\n");
        free(nstr);
	    goto fail ;
	  }

	  if(ch == 0)
	  {
		  printf("There is a #0 character in the input string. This is forbidden as the #0 character is used as a terminating character\n");
	      free(nstr);
		  goto fail ;
	  }
	  else
		  if(ch > 254)
		  {
			 printf("There is a character in the input string that exceeds the value of [254]. This is forbidden as only characters from #1 to #254 are supported.\n");
	         free(nstr);
		     goto fail ;		   
		  }
	  chars[charscnt] = (char)ch ;
	  charscnt++ ;
	  if(charscnt == 1023) 
	  {
	     printf("The input string has a character limit of 1023 characters. The input string exceeds this limit.\n");
	     free(nstr);
		 goto fail ;
	  }
	}
	chars[charscnt] = 0 ;
    free(nstr);

	/*create the new string*/
	PDX_STRING nst = dx_string_createU(NULL,chars) ;
	
    *result    = hdr_var_create(nst,"",hvf_temporary_ref,NULL);
    (*result)->type = hvt_simple_string ;

    success:
    hdr_sys_func_free_params(params) ;
    return false ;

    fail : 
    printf("The system function setFromChars(Chars : String[#13#10...]) failed.\n");
    hdr_sys_func_free_params(params) ;
    return true ;
}

bool hdrVarExists(PHDR_INTERPRETER inter , PHDR_COMPLEX_TOKEN token, PHDR_VAR *result)
{
   PHDR_SYS_FUNC_PARAMS params = hdr_sys_func_init_params(inter,token->parameters,1) ;
   if(params == NULL)
   {
    printf("The system function varExists($varName:String):Boolean failed.\n");
    return true ;
   }
   
    bool type_error = false ;
   
    PDX_STRING str  = hdr_inter_ret_string(params->params[0],&type_error) ;
    if(type_error == true)
    {
	  printf("The first parameter must be a String.\n");
      goto fail ;
    }

   *result = hdr_var_create(NULL, "", hvf_temporary_ref, NULL) ;
   (*result)->type    = hvt_bool  ; 
   (*result)->integer = 0		  ;

   PHDR_VAR var    = hdr_inter_retrieve_var_block(inter,str) ;
   if(var != NULL) (*result)->integer = 1		  ;

   success:
    hdr_sys_func_free_params(params) ;
    return false ;

   fail : 
    printf("The system function  varExists($varName:String):Boolean failed.\n");
    hdr_sys_func_free_params(params) ;
    return true ;
}


bool hdr_create_dir(PHDR_INTERPRETER inter , PHDR_COMPLEX_TOKEN token, PHDR_VAR *result)
{
  /*
    Create a directory
  */
   PHDR_SYS_FUNC_PARAMS params = hdr_sys_func_init_params(inter,token->parameters,1) ;
   if(params == NULL)
   {
     printf("The system function createDir($dir : $String):Boolean failed.\n");
     return true ;
   }

   bool type_error = false ;
   PDX_STRING dirName = hdr_inter_ret_string(params->params[0],&type_error) ; 
   if(type_error == true)
   {
	 printf("The first parameter must be a Simple String.\n");
     goto fail ;
   }

   
   *result = hdr_var_create(NULL, "", hvf_temporary_ref, NULL) ;
   (*result)->type        = hvt_bool                  ;  
   if(dxCreateDirectory(dirName) == true)  (*result)->integer  = 1 ;
   else
     (*result)->integer  = 0 ;

   success:
    hdr_sys_func_free_params(params) ;
    return false ;

   fail : 
    printf("The system function createDir($dir : String):Boolean failed.\n");
    hdr_sys_func_free_params(params) ;
    return true ;
}


bool hdr_dir_to_zip(PHDR_INTERPRETER inter , PHDR_COMPLEX_TOKEN token, PHDR_VAR *result)
{
  /*
    compresses a directory
  */
   PHDR_SYS_FUNC_PARAMS params = hdr_sys_func_init_params(inter,token->parameters,2) ;
   if(params == NULL)
   {
     printf("The system function zipDir($fileName : String , $dir : $String):Boolean failed.\n");
     return true ;
   }

   bool type_error = false ;
   PDX_STRING fileName = hdr_inter_ret_string(params->params[0],&type_error) ; 
   if(type_error == true)
   {
	 printf("The first parameter must be a Simple String.\n");
     goto fail ;
   }

   
   PDX_STRING dirName = hdr_inter_ret_string(params->params[1],&type_error) ; 
   if(type_error == true)
   {
	 printf("The second parameter must be a Simple String.\n");
     goto fail ;
   }

   
   *result = hdr_var_create(NULL, "", hvf_temporary_ref, NULL) ;
   (*result)->type        = hvt_bool                  ;  
   if(dxDirToZip(fileName,dirName) == true)  (*result)->integer  = 1 ;
   else
     (*result)->integer  = 0 ;

   success:
    hdr_sys_func_free_params(params) ;
    return false ;

   fail : 
    printf("The system function zipDir($fileName : String , $dir : $String):Boolean failed.\n");
    hdr_sys_func_free_params(params) ;
    return true ;
}

bool hdr_zip_files(PHDR_INTERPRETER inter , PHDR_COMPLEX_TOKEN token, PHDR_VAR *result)
{
  /*
    Compresses the files in the list 
  */
   PHDR_SYS_FUNC_PARAMS params = hdr_sys_func_init_params(inter,token->parameters,2) ;
   if(params == NULL)
   {
     printf("The system function zipFiles($fileName : String , $fileList : StringList):Boolean failed.\n");
     return true ;
   }

   bool type_error = false ;
   PDX_STRING fileName = hdr_inter_ret_string(params->params[0],&type_error) ; 
   if(type_error == true)
   {
	 printf("The first parameter must be a Simple String.\n");
     goto fail ;
   }

   
   PDX_STRINGLIST list = hdr_inter_ret_stringlist(params->params[1],&type_error) ; 
   if(type_error == true)
   {
	 printf("The second parameter must be a StringList.\n");
     goto fail ;
   }

   
   *result = hdr_var_create(NULL, "", hvf_temporary_ref, NULL) ;
   (*result)->type        = hvt_bool                  ;  
   if(dxFilesToZip(fileName,list) == true)  (*result)->integer  = 1 ;
   else
     (*result)->integer  = 0 ;

   success:
    hdr_sys_func_free_params(params) ;
    return false ;

   fail : 
    printf("The system function zipFiles($fileName : String , $fileList : StringList):Boolean failed.\n");
    hdr_sys_func_free_params(params) ;
    return true ;
}


bool hdr_extract_zip(PHDR_INTERPRETER inter , PHDR_COMPLEX_TOKEN token, PHDR_VAR *result)
{
  /*
    decompress the zip file to the directory
  */
   PHDR_SYS_FUNC_PARAMS params = hdr_sys_func_init_params(inter,token->parameters,2) ;
   if(params == NULL)
   {
     printf("The system function extractZip($fileName : String , $dir : String):Boolean failed.\n");
     return true ;
   }

   bool type_error = false ;
   PDX_STRING fileName = hdr_inter_ret_string(params->params[0],&type_error) ; 
   if(type_error == true)
   {
	 printf("The first parameter must be a String.\n");
     goto fail ;
   }

   
   PDX_STRING dir = hdr_inter_ret_string(params->params[1],&type_error) ; 
   if(type_error == true)
   {
	 printf("The second parameter must be a String.\n");
     goto fail ;
   }

   
   *result = hdr_var_create(NULL, "", hvf_temporary_ref, NULL) ;
   (*result)->type        = hvt_bool                  ;  
   if(dxExtractZip(fileName,dir) == true)  (*result)->integer  = 1 ;
   else
     (*result)->integer  = 0 ;

   success:
    hdr_sys_func_free_params(params) ;
    return false ;

   fail : 
    printf("The system function extractZip($fileName : String , $dir : String):Boolean failed.\n");
    hdr_sys_func_free_params(params) ;
    return true ;
}

bool hdr_get_files(PHDR_INTERPRETER inter , PHDR_COMPLEX_TOKEN token, PHDR_VAR *result)
{
  /*
    get the files of the directory 
  */
   PHDR_SYS_FUNC_PARAMS params = hdr_sys_func_init_params(inter,token->parameters,3) ;
   if(params == NULL)
   {
     printf("The system function getFiles($dirName : String , out->$fileList : StringList , $recursive : Boolean):Boolean failed.\n");
     return true ;
   }

   bool type_error = false ;
   PDX_STRING fileName = hdr_inter_ret_string(params->params[0],&type_error) ; 
   if(type_error == true)
   {
	 printf("The first parameter must be a Simple String.\n");
     goto fail ;
   }

   
   PDX_STRINGLIST list = hdr_inter_ret_stringlist(params->params[1],&type_error) ; 
   if(type_error == true)
   {
	 printf("The second parameter must be a StringList.\n");
     goto fail ;
   }

   bool recursive = hdr_inter_ret_bool(params->params[2],&type_error) ; 
   if(type_error == true)
   {
	 printf("The third parameter must be a Boolean.\n");
     goto fail ;
   }

   *result = hdr_var_create(NULL, "", hvf_temporary_ref, NULL) ;
   (*result)->type        = hvt_bool                  ;  
   if(dxGetFiles(fileName,list,recursive) == true)  (*result)->integer  = 1 ;
   else
     (*result)->integer  = 0 ;

   success:
    hdr_sys_func_free_params(params) ;
    return false ;

   fail : 
    printf("The system function getFiles($dirName : String , out->$fileList : StringList , $recursive : Boolean):Boolean failed.\n");
    hdr_sys_func_free_params(params) ;
    return true ;
}


bool hdr_get_dirs(PHDR_INTERPRETER inter , PHDR_COMPLEX_TOKEN token, PHDR_VAR *result)
{
  /*
    get the directories of the directory 
  */
   PHDR_SYS_FUNC_PARAMS params = hdr_sys_func_init_params(inter,token->parameters,3) ;
   if(params == NULL)
   {
     printf("The system function getDirs($dirName : String , out->$dirList : StringList , $recursive : Boolean):Boolean failed.\n");
     return true ;
   }

   bool type_error = false ;
   PDX_STRING fileName = hdr_inter_ret_string(params->params[0],&type_error) ; 
   if(type_error == true)
   {
	 printf("The first parameter must be a Simple String.\n");
     goto fail ;
   }

   
   PDX_STRINGLIST list = hdr_inter_ret_stringlist(params->params[1],&type_error) ; 
   if(type_error == true)
   {
	 printf("The second parameter must be a StringList.\n");
     goto fail ;
   }

   bool recursive = hdr_inter_ret_bool(params->params[2],&type_error) ; 
   if(type_error == true)
   {
	 printf("The third parameter must be a Boolean.\n");
     goto fail ;
   }

   *result = hdr_var_create(NULL, "", hvf_temporary_ref, NULL) ;
   (*result)->type        = hvt_bool                  ;  
   if(dxGetDirs(fileName,list,recursive) == true)  (*result)->integer  = 1 ;
   else
     (*result)->integer  = 0 ;

   success:
    hdr_sys_func_free_params(params) ;
    return false ;

   fail : 
    printf("The system function getDirs($dirName : String , out->$dirsList : StringList , $recursive : Boolean):Boolean failed.\n");
    hdr_sys_func_free_params(params) ;
    return true ;
}

bool hdr_get_files_dirs(PHDR_INTERPRETER inter , PHDR_COMPLEX_TOKEN token, PHDR_VAR *result)
{
  /*
    get the directories of the directory 
  */
   PHDR_SYS_FUNC_PARAMS params = hdr_sys_func_init_params(inter,token->parameters,3) ;
   if(params == NULL)
   {
     printf("The system function getFilesDirs($dirName : String , out->$fileList : StringList , $recursive : Boolean):Boolean failed.\n");
     return true ;
   }

   bool type_error = false ;
   PDX_STRING fileName = hdr_inter_ret_string(params->params[0],&type_error) ; 
   if(type_error == true)
   {
	 printf("The first parameter must be a Simple String.\n");
     goto fail ;
   }

   
   PDX_STRINGLIST list = hdr_inter_ret_stringlist(params->params[1],&type_error) ; 
   if(type_error == true)
   {
	 printf("The second parameter must be a StringList.\n");
     goto fail ;
   }

   bool recursive = hdr_inter_ret_bool(params->params[2],&type_error) ; 
   if(type_error == true)
   {
	 printf("The third parameter must be a Boolean.\n");
     goto fail ;
   }

   *result = hdr_var_create(NULL, "", hvf_temporary_ref, NULL) ;
   (*result)->type        = hvt_bool                  ;  
   if(dxGetFileDirs(fileName,list,recursive) == true)  (*result)->integer  = 1 ;
   else
     (*result)->integer  = 0 ;

   success:
    hdr_sys_func_free_params(params) ;
    return false ;

   fail : 
    printf("The system function getFilesDirs($dirName : String , out->$fileList : StringList , $recursive : Boolean):Boolean failed.\n");
    hdr_sys_func_free_params(params) ;
    return true ;
}




