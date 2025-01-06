/*
 Hydra file support

 Nikos Mourgis deus-ex.gr 2024

 Live Long and Prosper
*/


bool hdr_domFileOpen(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token, PHDR_VAR *result)
{
   PHDR_SYS_FUNC_PARAMS params = hdr_sys_func_init_params(inter,token->parameters,4) ;
   if(params == NULL)
   {
    printf("The system function File.Open($filename:String,mode : _read | _write , $doCreate : boolean,$error : String):File failed.\n");
    return true ;
   }
   
   PDX_STRING res = dx_string_createU(NULL,"") ; 

   bool type_error = false ;

   PDX_STRING error_code = hdr_inter_ret_string(params->params[3],&type_error) ; 
   if(type_error == true)
   {
	 printf("The fourth parameter must be a String variable.\n");
     goto fail ;
   }

   PDX_STRING fname = hdr_inter_ret_string(params->params[0],&type_error) ; 
   if(type_error == true)
   {
	 printf("The first parameter must be a String.\n");
     goto fail ;
   }

   DXLONG64 mode  = hdr_inter_ret_integer(params->params[1],&type_error) ; 
   if((type_error == true)||((mode != HDR_INTER_FILE_READ)&&(mode != HDR_INTER_FILE_WRITE)))
   {
	 printf("The second parameter must be _read or _write.\n");
     goto fail ;
   }

   bool can_create = hdr_inter_ret_bool(params->params[2],&type_error) ; 
   if(type_error == true)
   {
	 printf("The third parameter must be a Boolean.\n");
     goto fail ;
   }

   *result            = hdr_var_create(NULL, "", hvf_temporary_ref, NULL) ;
   (*result)->type    = hvt_file ; 

   /*
    first check if the file exists. If the file does not exists and the can_create is false then 
    we will return a file not found error
   */
   if(can_create == false)
   {
     if(dxFileExists(fname) == false) 
     {
         res = (void*)dx_string_createU(res,"File not found"); 
         goto success ;
     }
   }

   char access_mode[3] ;
   access_mode[2] = 0 ;
   if(mode == HDR_INTER_FILE_READ) {access_mode[0] ='r';access_mode[1] ='b';}
   else
   if(mode == HDR_INTER_FILE_WRITE) {access_mode[0] ='a';access_mode[1] ='b';}
   
   FILE *f = fopen(fname->stringa,access_mode) ;
   if(f == NULL)
   {
     PDX_STRING str      = dx_string_createU(NULL,"Error opening the file. System error code : ");
     PDX_STRING err_code = dx_IntToStr(errno) ; 
     dx_string_free(res);
     res = dx_string_concat(str,err_code);
     dx_string_free(str)      ;
     dx_string_free(err_code) ;
     goto success ;
   }

   hdr_var_set_obj(*result,f) ;

    success:
    error_code = dx_string_createU(error_code,res->stringa) ;
    dx_string_free(res);
    hdr_sys_func_free_params(params) ;
    return false ;

    fail : 
    error_code = dx_string_createU(error_code,res->stringa) ;
    dx_string_free(res) ;
    printf("The system function File.Open($filename:String,mode : _read | _write , $doCreate : boolean,$error : String):File failed.\n");
    hdr_sys_func_free_params(params) ;
    return true ;
}


bool hdr_domFileIsOpen(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token,PHDR_VAR for_var,PHDR_VAR *result)
{
   PHDR_SYS_FUNC_PARAMS params = hdr_sys_func_init_params(inter,token->parameters,0) ;
   if(params == NULL)
   {
    printf("The system function File.IsOpen():Boolean failed.\n");
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
    printf("The system function File.isOpen():Boolean failed.\n");
    hdr_sys_func_free_params(params) ;
    return true ;
}

bool hdr_domFileSize(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token,PHDR_VAR for_var,PHDR_VAR *result)
{
   PHDR_SYS_FUNC_PARAMS params = hdr_sys_func_init_params(inter,token->parameters,0) ;
   if(params == NULL)
   {
    printf("The system function File.Size():Integer failed.\n");
    return true ;
   }

   *result            = hdr_var_create(NULL, "", hvf_temporary_ref, NULL) ;
   (*result)->type    = hvt_integer    ; 
   (*result)->integer = -1             ;

    if(for_var->obj != NULL)
    {
      (*result)->integer = dx_GetFileSize((FILE*)for_var->obj) ;
    }


    success:
    hdr_sys_func_free_params(params) ;
    return false ;

    fail : 
    printf("The system function File.Size():Integer failed.\n");
    hdr_sys_func_free_params(params) ;
    return true ;
}




bool hdr_domFileClose(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token,PHDR_VAR for_var)
{
   PHDR_SYS_FUNC_PARAMS params = hdr_sys_func_init_params(inter,token->parameters,0) ;
   if(params == NULL)
   {
    printf("The system function File.Close() failed.\n");
    return true ;
   }

    if(for_var->obj != NULL)
    fclose((FILE*)for_var->obj) ;
    else
    {
     if(inter->warnings == true ) printf("The file object that was passed in the File.Close() (in syntax $file.Close()) was NULL.\n");
    }

    hdr_var_release_obj(for_var) ;

    success:
    hdr_sys_func_free_params(params) ;
    return false ;

    fail : 
    printf("The system function File.Close() failed.\n");
    hdr_sys_func_free_params(params) ;
    return true ;
}

bool hdr_domFileSeek(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token,PHDR_VAR for_var)
{
   PHDR_SYS_FUNC_PARAMS params = hdr_sys_func_init_params(inter,token->parameters,2) ;
   if(params == NULL)
   {
    printf("The system function File.Seek($offset : Integer , from : _start | _end | _curr) failed.\n");
    return true ;
   }


   bool type_error = false ;

   DXLONG64 offset = hdr_inter_ret_integer(params->params[0],&type_error) ; 
   if(type_error == true)
   {
	 printf("The first parameter must be an Integer.\n");
     goto fail ;
   }

   DXLONG64 from = hdr_inter_ret_integer(params->params[1],&type_error) ; 
   if(type_error == true)
   {
	 printf("The second parameter must be an Integer.\n");
     goto fail ;
   }

   if(for_var->obj == NULL)
   {
     if(inter->warnings == true ) printf("The file object that was passed in the File.Seek($offset : Integer , from : _start | _end | _curr) (in syntax $file.Seek(...)) was NULL.\n");
     goto success ;
   }

   fseek((FILE*)for_var->obj,offset,from) ;

   success:
    hdr_sys_func_free_params(params) ;
    return false ;

   fail : 
    printf("The system function File.Seek($offset : Integer , from : _start | _end | _curr) failed.\n");
    hdr_sys_func_free_params(params) ;
    return true ;
}

bool hdr_domFileRead(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token,PHDR_VAR for_var , PHDR_VAR *result)
{
   PHDR_SYS_FUNC_PARAMS params = hdr_sys_func_init_params(inter,token->parameters,1) ;
   if(params == NULL)
   {
    printf("The system function File.Read($buff : Bytes):Integer failed.\n");
    return true ;
   }

   *result            = hdr_var_create(NULL, "", hvf_temporary_ref, NULL) ;
   (*result)->type    = hvt_integer ; 
   (*result)->integer = - 1         ;


   bool type_error = false ;

   PHDR_BYTES bytes = hdr_inter_ret_bytes(params->params[0],&type_error) ; 
   if(type_error == true)
   {
	 printf("The first parameter must be a Bytes object.\n");
     goto fail ;
   }

   if(for_var->obj == NULL)
   {
     if(inter->warnings == true ) printf("The file object that was passed in the File.Read($buff : Bytes , $count : Integer) (in syntax $file.Read(...)) was NULL.\n");
     goto success ;
   } 

   /*copy the bytes*/
   DXLONG64 bread = fread(bytes->bytes,1,bytes->length,(FILE*)for_var->obj);
   (*result)->integer = bread ;
   if(bread == 0)
   {
    /*check if this is an error or the file ends*/
    if (ferror((FILE*)for_var->obj)!=0) (*result)->integer = -1; 
   }

   success:
    hdr_sys_func_free_params(params) ;
    return false ;

   fail : 
    printf("The system function File.Read($buff : Bytes , $count : Integer) failed.\n");
    hdr_sys_func_free_params(params) ;
    return true ;
}

bool hdr_domEmptyFile(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token)
{
   PHDR_SYS_FUNC_PARAMS params = hdr_sys_func_init_params(inter,token->parameters,1) ;
   if(params == NULL)
   {
    printf("The system function emptyFile($filename:String) failed.\n");
    return true ;
   }
   
   bool type_error = false ;

   PDX_STRING fname = hdr_inter_ret_string(params->params[0],&type_error) ; 
   if(type_error == true)
   {
	 printf("The first parameter must be a String variable.\n");
     goto fail ;
   }
    if(dxFileExists(fname)==true) fclose(fopen(fname->stringa,"wb")) ;

    success:
    hdr_sys_func_free_params(params) ;
    return false ;

    fail : 
    printf("The system function emptyFile($filename : String) failed.\n");
    hdr_sys_func_free_params(params) ;
    return true ;
}

bool hdr_domFileExists(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token,PHDR_VAR *result)
{
   PHDR_SYS_FUNC_PARAMS params = hdr_sys_func_init_params(inter,token->parameters,1) ;
   if(params == NULL)
   {
    printf("The system function fileExist($filename:String):Boolean failed.\n");
    return true ;
   }
   
   bool type_error = false ;

   PDX_STRING fname = hdr_inter_ret_string(params->params[0],&type_error) ; 
   if(type_error == true)
   {
	 printf("The first parameter must be a String variable.\n");
     goto fail ;
   }

    bool ret = dxFileExists(fname) ;

    *result           = hdr_var_create(NULL, "", hvf_temporary_ref, NULL) ;
   (*result)->type    = hvt_bool ; 
   (*result)->integer =  hdr_inter_bool_to_int(ret)      ;

    success:
    hdr_sys_func_free_params(params) ;
    return false ;

    fail : 
    printf("The system function fileExist($filename:String):Boolean failed.\n");
    hdr_sys_func_free_params(params) ;
    return true ;
}

bool hdr_domDirExists(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token,PHDR_VAR *result)
{
   PHDR_SYS_FUNC_PARAMS params = hdr_sys_func_init_params(inter,token->parameters,1) ;
   if(params == NULL)
   {
    printf("The system function dirExist($dirname:String):Boolean failed.\n");
    return true ;
   }
   
   bool type_error = false ;

   PDX_STRING fname = hdr_inter_ret_string(params->params[0],&type_error) ; 
   if(type_error == true)
   {
	 printf("The first parameter must be a String variable.\n");
     goto fail ;
   }

    bool ret = dxDirExists(fname) ;

    *result           = hdr_var_create(NULL, "", hvf_temporary_ref, NULL) ;
   (*result)->type    = hvt_bool ; 
   (*result)->integer =  hdr_inter_bool_to_int(ret)      ;

    success:
    hdr_sys_func_free_params(params) ;
    return false ;

    fail : 
    printf("The system function dirExist($filename:String):Boolean failed.\n");
    hdr_sys_func_free_params(params) ;
    return true ;
}


bool hdr_domFileWrite(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token,PHDR_VAR for_var , PHDR_VAR *result)
{
   PHDR_SYS_FUNC_PARAMS params = hdr_sys_func_init_params(inter,token->parameters,2) ;
   if(params == NULL)
   {
    printf("The system function File.Write($buff : Bytes , $count : Integer):Integer failed.\n");
    return true ;
   }

   *result            = hdr_var_create(NULL, "", hvf_temporary_ref, NULL) ;
   (*result)->type    = hvt_integer ; 
   (*result)->integer = - 1         ;


   bool type_error = false ;

   PHDR_BYTES bytes = hdr_inter_ret_bytes(params->params[0],&type_error) ; 
   if(type_error == true)
   {
	 printf("The first parameter must be a Bytes object.\n");
     goto fail ;
   }

   DXLONG64 count = hdr_inter_ret_integer(params->params[1],&type_error) ; 
   if(type_error == true)
   {
	 printf("The second parameter must be an Integer.\n");
     goto fail ;
   }

   if(for_var->obj == NULL)
   {
     if(inter->warnings == true ) printf("The file object that was passed in the File.Write($buff : Bytes , $count : Integer) (in syntax $file.Write(...)) was NULL.\n");
     goto success ;
   } 

   if(count > bytes->length) count = bytes->length ;

   /*write the bytes*/
   DXLONG64 bwrite = fwrite(bytes->bytes,1,count,(FILE*)for_var->obj);
   (*result)->integer = bwrite ;
   if(bwrite == 0)
   {
    /*check if this is an error or the file ends*/
    if (ferror((FILE*)for_var->obj)!=0) (*result)->integer = -1; 
   }

   success:
    hdr_sys_func_free_params(params) ;
    return false ;

   fail : 
    printf("The system function File.Write($buff : Bytes , $count : Integer) failed.\n");
    hdr_sys_func_free_params(params) ;
    return true ;
}


bool hdr_domFileRemove(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token,PHDR_VAR *result)
{
   PHDR_SYS_FUNC_PARAMS params = hdr_sys_func_init_params(inter,token->parameters,1) ;
   if(params == NULL)
   {
    printf("The system function deleteFile($filename:String):Boolean failed.\n");
    return true ;
   }
   
   bool type_error = false ;

   PDX_STRING fname = hdr_inter_ret_string(params->params[0],&type_error) ; 
   if(type_error == true)
   {
	 printf("The first parameter must be a String variable.\n");
     goto fail ;
   }

    int ret = remove(fname->stringa) ;
    if (ret  == 0 ) ret = 1 ;
    else
        ret = 0 ;

    *result           = hdr_var_create(NULL, "", hvf_temporary_ref, NULL) ;
   (*result)->type    = hvt_bool ; 
   (*result)->integer = ret      ;

    success:
    hdr_sys_func_free_params(params) ;
    return false ;

    fail : 
    printf("The system function deleteFile($filename:String):Boolean failed.\n");
    hdr_sys_func_free_params(params) ;
    return true ;
}

bool hdr_domDeleteDir(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token,PHDR_VAR *result)
{
   PHDR_SYS_FUNC_PARAMS params = hdr_sys_func_init_params(inter,token->parameters,1) ;
   if(params == NULL)
   {
    printf("The system function deleteDir($filename:String):Boolean failed.\n");
    return true ;
   }
   
   bool type_error = false ;

   PDX_STRING fname = hdr_inter_ret_string(params->params[0],&type_error) ; 
   if(type_error == true)
   {
	 printf("The first parameter must be a String variable.\n");
     goto fail ;
   }

    bool ret = dxDeleteDir(fname,true) ;


    *result           = hdr_var_create(NULL, "", hvf_temporary_ref, NULL) ;
   (*result)->type    = hvt_bool ; 
   (*result)->integer = hdr_inter_bool_to_int(ret)      ;

    success:
    hdr_sys_func_free_params(params) ;
    return false ;

    fail : 
    printf("The system function deleteDir($filename:String):Boolean failed.\n");
    hdr_sys_func_free_params(params) ;
    return true ;
}

bool hdr_domEmptyDir(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token,PHDR_VAR *result)
{
   PHDR_SYS_FUNC_PARAMS params = hdr_sys_func_init_params(inter,token->parameters,1) ;
   if(params == NULL)
   {
    printf("The system function emptyDir($filename:String):Boolean failed.\n");
    return true ;
   }
   
   bool type_error = false ;

   PDX_STRING fname = hdr_inter_ret_string(params->params[0],&type_error) ; 
   if(type_error == true)
   {
	 printf("The first parameter must be a String variable.\n");
     goto fail ;
   }

   bool ret = dxDeleteDir(fname,false) ;
    

    *result           = hdr_var_create(NULL, "", hvf_temporary_ref, NULL) ;
   (*result)->type    = hvt_bool ; 
   (*result)->integer = hdr_inter_bool_to_int(ret)     ;

    success:
    hdr_sys_func_free_params(params) ;
    return false ;

    fail : 
    printf("The system function emptyDir($filename:String):Boolean failed.\n");
    hdr_sys_func_free_params(params) ;
    return true ;
}











