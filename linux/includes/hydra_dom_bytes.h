/*
  This file has the implementation for the bytes (buffers) support for the Hydra+  

  Nikos Mourgis deus-ex.gr 2024

  Live Long and Prosper

*/


bool hdr_domBytesCreate(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token, PHDR_VAR *result)
{
   PHDR_SYS_FUNC_PARAMS params = hdr_sys_func_init_params(inter,token->parameters,1) ;
   if(params == NULL)
   {
    printf("The system function Bytes.Create($length : Integer) failed.\n");
    return true ;
   }
   
   bool type_error = false ;
   DXLONG64 len = hdr_inter_ret_integer(params->params[0],&type_error) ; 
   if(type_error == true)
   {
	 printf("The parameter must be an integer.\n");
     goto fail ;
   }

   *result = hdr_var_create(NULL, "", hvf_temporary_ref, NULL) ;
   (*result)->type    = hvt_bytes ; 
   PHDR_BYTES bt = hdr_bytes_create(len) ;
   hdr_var_set_obj(*result,bt) ;
   if((*result)->obj == NULL) goto fail ;

    success:
    hdr_sys_func_free_params(params) ;
    return false ;

    fail : 
    printf("The system function Bytes.Create($length : Integer) failed.\n");
    hdr_sys_func_free_params(params) ;
    return true ;
}

bool hdr_domBytesReturnLen(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token, PHDR_VAR for_var , PHDR_VAR *result)
{
   PHDR_SYS_FUNC_PARAMS params = hdr_sys_func_init_params(inter,token->parameters,0) ;
   if(params == NULL)
   {
    printf("The system function Bytes.Length($length : Integer) failed.\n");
    return true ;
   }

   PHDR_BYTES bytes = (PHDR_BYTES)for_var->obj ;

   *result = hdr_var_create(NULL, "", hvf_temporary_ref, NULL) ;
   (*result)->type    = hvt_integer   ; 
   (*result)->integer = bytes->length ;

    success:
    hdr_sys_func_free_params(params) ;
    return false ;

    fail : 
    printf("The system function Bytes.Create($length : Integer) failed.\n");
    hdr_sys_func_free_params(params) ;
    return true ;
}


bool hdr_domBytesSetToZero(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token, PHDR_VAR for_var)
{
   PHDR_SYS_FUNC_PARAMS params = hdr_sys_func_init_params(inter,token->parameters,0) ;
   if(params == NULL)
   {
    printf("The system function Bytes.SetToZero() failed.\n");
    return true ;
   }

   PHDR_BYTES bytes = (PHDR_BYTES)for_var->obj ;
   bytes->length = 0   ;
   free(bytes->bytes)  ;
   bytes->bytes = NULL ;


   success:
    hdr_sys_func_free_params(params) ;
    return false ;

   fail : 
    printf("The system function Bytes.SetToZero() failed.\n");
    hdr_sys_func_free_params(params) ;
    return true ;
}

bool hdr_domBytesReset(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token, PHDR_VAR for_var)
{
   PHDR_SYS_FUNC_PARAMS params = hdr_sys_func_init_params(inter,token->parameters,1) ;
   if(params == NULL)
   {
    printf("The system function Bytes.Reset($length : integer) failed.\n");
    return true ;
   }

   bool type_error = false ;
   DXLONG64 len = hdr_inter_ret_integer(params->params[0],&type_error) ; 
   if(type_error == true)
   {
	 printf("The parameter must be an integer.\n");
     goto fail ;
   }

   PHDR_BYTES bytes = (PHDR_BYTES)for_var->obj ;
   bytes->length = len   ;
   free(bytes->bytes)    ;
   bytes->bytes = (char*)malloc(len);


   success:
    hdr_sys_func_free_params(params) ;
    return false ;

   fail : 
    printf("The system function Bytes.Reset($length : integer) failed.\n");
    hdr_sys_func_free_params(params) ;
    return true ;
}

bool hdr_domBytesCopy(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token, PHDR_VAR for_var)
{
   PHDR_SYS_FUNC_PARAMS params = hdr_sys_func_init_params(inter,token->parameters,1) ;
   if(params == NULL)
   {
    printf("The system function Bytes.Copy($bytes : Bytes) failed.\n");
    return true ;
   }

   PHDR_VAR srcv = params->params[0] ;
   if(srcv->type != hvt_bytes)
   {
    printf("The parameter MUST be another Bytes variable.\n");
    goto fail ;
   }

   PHDR_BYTES dest = (PHDR_BYTES)for_var->obj ;
   PHDR_BYTES src   = (PHDR_BYTES)srcv->obj    ;

   if (dest == src) goto success ;

   if(dest->length < src->length )
   {
    printf("The destination [Bytes] length is smaller than the source [Bytes] length.\n");
    goto fail ;
   }

   /*copy*/
   for(DXLONG64 i = 0 ; i < src->length;i++)
   {
     dest->bytes[i] = src->bytes[i] ;
   }


   success:
    hdr_sys_func_free_params(params) ;
    return false ;

   fail : 
    printf("The system function Bytes.Copy($bytes : Bytes) failed.\n");
    hdr_sys_func_free_params(params) ;
    return true ;
}


bool hdr_domBytesCopyEx(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token, PHDR_VAR for_var)
{
   PHDR_SYS_FUNC_PARAMS params = hdr_sys_func_init_params(inter,token->parameters,4) ;
   if(params == NULL)
   {
    printf("The system function Bytes.CopyEx($bytes : Bytes , $dest_start : Integer , $source_start : Integer , $source_end : Integer) failed.\n");
    return true ;
   }

   PHDR_VAR srcv = params->params[0] ;
   
   if(srcv->type != hvt_bytes)
   {
    printf("The first parameter MUST be another Bytes variable.\n");
    goto fail ;
   }

   bool type_error = false ; 
   DXLONG64 dest_start = hdr_inter_ret_integer(params->params[1],&type_error) ;

   if (type_error == true)
   {
    printf("The second parameter MUST be a numeric value.\n");
    goto fail ;
   }
   
   DXLONG64 source_start = hdr_inter_ret_integer(params->params[2],&type_error) ;

   if(type_error == true)
   {
    printf("The third parameter MUST be a numeric value.\n");
    goto fail ;
   }
   
   DXLONG64 source_end = hdr_inter_ret_integer(params->params[3],&type_error) ;

   if(type_error == true)
   {
    printf("The fourth parameter MUST be a numeric value.\n");
    goto fail ;
   }


   PHDR_BYTES dest = (PHDR_BYTES)for_var->obj ;
   PHDR_BYTES src   = (PHDR_BYTES)srcv->obj    ;

   if (dest == src) 
   {
       printf("The source and the destination [Bytes] are the same. Hydra+ does not support this operation.\n");
       goto fail ;
   }

   /*make the calculation to ensure that no buffer wil go out of range*/

   if((dest_start < 0)||(dest_start >= dest->length))
   {
        printf("The destination start index is not in the valid range 0 - %d of the [Bytes].\n",dest->length);
        goto fail ;
   }

   if((source_start < 0)||(source_start >= src->length)||(source_end < 0)||(source_end > src->length)||(source_end < source_start))
   {
        printf("The source start or end index is not in the valid range 0 - %d of the Source [Bytes].\n",src->length);
        printf("Or the source end index and the source start index are not correct.\n");
        goto fail ;
   }

   /*check if the destination is large enough to copy the buffer */
   DXLONG64 len = (source_end - source_start)+1 ; //+1 because these are indexes
   if(len > (dest->length - dest_start))
   {
     printf("The bytes that will be copied are more than the bytes that the destination can hold from the given index.\n");
     goto fail ;
   }
   /*copy*/
   DXLONG64 dstindx = dest_start ;
   for(DXLONG64 i = source_start ; i <= source_end ;i++) /*<= because these are indexes*/
   {
     dest->bytes[dstindx] = src->bytes[i] ;
     dstindx++ ;
   }


   success:
    hdr_sys_func_free_params(params) ;
    return false ;

   fail : 
    printf("The system function Bytes.CopyEx($bytes : Bytes , $dest_start : Integer , $source_start : Integer , $source_end : Integer) failed.\n");
    hdr_sys_func_free_params(params) ;
    return true ;
}



bool hdr_domBytesFromUTF8(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token, PHDR_VAR for_var)
{
   /* gets a unicode string , release any old memory in the Bytes object and creates a new buffer with the bytes of the string*/
   PHDR_SYS_FUNC_PARAMS params = hdr_sys_func_init_params(inter,token->parameters,1) ;
   if(params == NULL)
   {
    printf("The system function Bytes.FromUTF8($str : String) failed.\n");
    return true ;
   }

   bool type_error ;
   PDX_STRING str = hdr_inter_ret_string(params->params[0],&type_error) ;
   if(type_error == true)
   {
    printf("The parameter MUST be a [String] variable.\n");
    goto fail ;
   }

   PHDR_BYTES bytes = (PHDR_BYTES)for_var->obj ;
   free(bytes->bytes);
   bytes->length = str->bcount ;
   bytes->bytes = (char*)malloc(str->bcount) ;
   if(bytes->bytes == NULL)
   {
     printf("The memory allocation for the [Bytes] failed!\n");
     goto fail ;
   }

   for(DXLONG64 i = 0;i<bytes->length;i++)
   {
    bytes->bytes[i] = str->stringa[i] ;
   }

   success:
    hdr_sys_func_free_params(params) ;
    return false ;

   fail : 
    printf("The system function Bytes.FromUTF8($str : String) failed.\n");
    hdr_sys_func_free_params(params) ;
    return true ;
}


bool hdr_domBytesToUTF8(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token, PHDR_VAR for_var,PHDR_VAR *result)
{
   /*creates a new PDX_STRING from the bytes*/
   PHDR_SYS_FUNC_PARAMS params = hdr_sys_func_init_params(inter,token->parameters,1) ;
   if(params == NULL)
   {
    printf("The system function Bytes.ToUTF8($byte_count : Integer):String failed.\n");
    return true ;
   }

   bool type_error ;
   DXLONG64 bcount = hdr_inter_ret_integer(params->params[0],&type_error) ;
   if(type_error == true)
   {
    printf("The parameter MUST be an Integer.\n");
    goto fail ;
   }


   PHDR_BYTES bytes = (PHDR_BYTES)for_var->obj ; 
   
   if(bcount < 0) bcount = bytes->length ;
   else 
     if(bcount > bytes->length ) bcount = bytes->length ;
   
   char *buff       = (char*)malloc(bcount+1) ; /*need a terminating zero!*/ 
   buff[bcount] = 0 ;
   for(DXLONG64 i = 0 ; i < bcount ; i++)
   {
    buff[i] = bytes->bytes[i] ;
   }


   *result = hdr_var_create(NULL, "", hvf_temporary_ref, NULL) ;
   (*result)->type    = hvt_simple_string ; 
   PDX_STRING ns      = (void*)dx_string_create_bU(buff)  ;
   hdr_var_set_obj(*result,ns) ;

   success:
    hdr_sys_func_free_params(params) ;
    return false ;

   fail : 
    printf("The system function Bytes.FromUTF8($str : String) failed.\n");
    hdr_sys_func_free_params(params) ;
    return true ;
}


bool hdr_domBytesBase64Encode(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token, PHDR_VAR for_var,PHDR_VAR *result)
{
   /*creates a new PDX_STRING from the bytes*/
   PHDR_SYS_FUNC_PARAMS params = hdr_sys_func_init_params(inter,token->parameters,0) ;
   if(params == NULL)
   {
    printf("The system function Bytes.Base64Encode():String failed.\n");
    return true ;
   }



   PHDR_BYTES bytes = (PHDR_BYTES)for_var->obj ; 
   
   PDX_STRING nstr = dx_string_create_bU(b64_encode(bytes->bytes,bytes->length)) ;
	if(nstr == NULL) 
	{
		nstr = dx_string_createU(NULL,"") ;
		if(inter->warnings == true)
		hdr_inter_print_warning(inter,"The conversion of the string to a base64 encoded string failed. Empty string returned") ;
	}

	*result   = hdr_var_create((void*)nstr,"",hvf_temporary_ref,NULL) ;
	(*result)->type = hvt_simple_string ;

   success:
    hdr_sys_func_free_params(params) ;
    return false ;

   fail : 
    printf("The system function Bytes.Base64Encode():String failed.\n");
    hdr_sys_func_free_params(params) ;
    return true ;
}


bool hdr_domBytesFree(PHDR_VAR for_var)
{
   if(for_var->obj == NULL) return false ;
   hdr_bytes_free((PHDR_BYTES)for_var->obj) ;
   hdr_var_release_obj(for_var) ;
   for_var->type = hvt_undefined ;

   return false ;
}

bool hdr_domBytesFindPattern(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token, PHDR_VAR for_var,PHDR_VAR *result)
{
	/*
	 The function searches for a patern of bytes (word). If it finds it then returns the index 
	 in characters or -1 if does not exists 
	*/

   PHDR_SYS_FUNC_PARAMS params = hdr_sys_func_init_params(inter,token->parameters,2) ;
   if(params == NULL)
   {
    printf("The system function Bytes.FindPattern($pattern:String,$startPos:Integer):Integer failed.\n");
    return true ;
   }

   bool type_error ;
   PDX_STRING pattern = hdr_inter_ret_string(params->params[0],&type_error) ;
   if(type_error == true)
   {
    printf("The first parameter MUST be a String.\n");
    goto fail ;
   }

   DXLONG64 pos = hdr_inter_ret_integer(params->params[1],&type_error) ;
   if(type_error == true)
   {
    printf("The second parameter MUST be an Integer.\n");
    goto fail ;
   }

    PHDR_BYTES bytes = (PHDR_BYTES)for_var->obj ; 
  
	*result   = hdr_var_create(NULL,"",hvf_temporary_ref,NULL) ;
	(*result)->type = hvt_integer ;
    (*result)->integer = dxFindPatternInBytes(bytes->bytes,bytes->length,pattern->stringa,pos) ;

   success:
    hdr_sys_func_free_params(params) ;
    return false ;

   fail : 
    printf("The system function Bytes.FindPattern($pattern:String,$startPos:Integer):Integer failed.\n");
    hdr_sys_func_free_params(params) ;
    return true ;

}


bool hdr_domBytesCompare(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token, PHDR_VAR for_var,PHDR_VAR *result)
{
	/*
	 The function compares the string's BYTES with the corresponding buffer bytes if they match then true is returned
     else returns false.
	*/

   PHDR_SYS_FUNC_PARAMS params = hdr_sys_func_init_params(inter,token->parameters,2) ;
   if(params == NULL)
   {
    printf("The system function Bytes.Compare($str:String,$startPos:Integer):Boolean failed.\n");
    return true ;
   }

   bool type_error ;
   PDX_STRING pattern = hdr_inter_ret_string(params->params[0],&type_error) ;
   if(type_error == true)
   {
    printf("The first parameter MUST be a String.\n");
    goto fail ;
   }

   DXLONG64 pos = hdr_inter_ret_integer(params->params[1],&type_error) ;
   if(type_error == true)
   {
    printf("The second parameter MUST be an Integer.\n");
    goto fail ;
   }

    PHDR_BYTES bytes = (PHDR_BYTES)for_var->obj ; 
  
	*result   = hdr_var_create(NULL,"",hvf_temporary_ref,NULL) ;
	(*result)->type = hvt_bool ;
    (*result)->integer = dxCompareBytes(bytes->bytes,bytes->length,pattern->stringa,pos) ;

   success:
    hdr_sys_func_free_params(params) ;
    return false ;

   fail : 
    printf("The system function Bytes.Compare($str:String,$startPos:Integer):Boolean failed.\n");
    hdr_sys_func_free_params(params) ;
    return true ;

}

bool hdr_domBytesXor(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token,PHDR_VAR for_var,PHDR_VAR *result)
{
	/*
	 The function returns the bytes xored with the key
	*/

   PHDR_SYS_FUNC_PARAMS params = hdr_sys_func_init_params(inter,token->parameters,1) ;
   if(params == NULL)
   {
	 printf("The system function Bytes.Xor($key:String):Bytes failed.\n");
     return true ;
   }

    bool type_error = false ;
    PDX_STRING key  = hdr_inter_ret_string(params->params[0],&type_error) ;
	if(type_error == true)
    {
	  printf("The parameter must be a string .\n");
      goto fail ;
    }


    PHDR_BYTES obytes  = (PHDR_BYTES)for_var->obj ;
	char * buff = dxBytesXor(obytes->bytes,obytes->length,key)   ;

    PHDR_BYTES bytes = NULL ;

	if(buff == NULL) 
	{
		bytes = hdr_bytes_create(0);
		if(inter->warnings == true)
			hdr_inter_print_warning(inter, "The [XOR] on the bytes failed. Empty [Bytes] returned");
	}

    bytes = (PHDR_BYTES) malloc(sizeof(struct hdr_bytes)) ;
	bytes->length = obytes->length ;
	bytes->bytes  = buff           ;

    *result   = hdr_var_create(bytes,"",hvf_temporary_ref,NULL) ;
	(*result)->type = hvt_bytes ;


    success:
    hdr_sys_func_free_params(params) ;
    return false ;

    fail : 
    printf("The system function Bytes.Xor($key : String):Bytes failed.\n");
    hdr_sys_func_free_params(params) ;
    return true ;
}

bool hdr_domBytesFillZero(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token, PHDR_VAR for_var)
{
   PHDR_SYS_FUNC_PARAMS params = hdr_sys_func_init_params(inter,token->parameters,0) ;
   if(params == NULL)
   {
    printf("The system function Bytes.FillWithZero() failed.\n");
    return true ;
   }

   PHDR_BYTES bytes = (PHDR_BYTES)for_var->obj ;
   memset(bytes->bytes,0,bytes->length) ;


   success:
    hdr_sys_func_free_params(params) ;
    return false ;

   fail : 
    printf("The system function Bytes.FillWithZero() failed.\n");
    hdr_sys_func_free_params(params) ;
    return true ;
}

bool hdr_domBytesFillByte(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token, PHDR_VAR for_var)
{
   PHDR_SYS_FUNC_PARAMS params = hdr_sys_func_init_params(inter,token->parameters,1) ;
   if(params == NULL)
   {
    printf("The system function Bytes.FillWithByte($byte : Integer) failed.\n");
    return true ;
   }

    bool type_error = false ;
    DXLONG64 bt = hdr_inter_ret_integer(params->params[0],&type_error) ;
	if(type_error == true)
    {
	  printf("The parameter must be an integer (0~255) .\n");
      goto fail ;
    }

   PHDR_BYTES bytes = (PHDR_BYTES)for_var->obj ;
   memset(bytes->bytes,(char)bt,bytes->length) ;


   success:
    hdr_sys_func_free_params(params) ;
    return false ;

   fail : 
    printf("The system function Bytes.FillWithByte($byte : Integer) failed.\n");
    hdr_sys_func_free_params(params) ;
    return true ;
}



