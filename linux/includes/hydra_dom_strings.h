
#include "hydra_utf8.h" 

/*******STRING****************************************************************************/


bool hdr_domStringCreate(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token, PHDR_VAR *result)
{
	/*check param count*/
	if (hdr_inter_check_param_count_error(inter, token, 2) == false) return true;
	PHDR_VAR param1 = hdr_inter_resolve_extract_var_from_expr(inter, token->parameters, 0);
	PHDR_VAR param2 = hdr_inter_resolve_extract_var_from_expr(inter, token->parameters, 1);
	
	if(param1 == NULL)
	{ 
	  printf("Parameter 1 was returned as NULL. Check the validity of the parameter.\n");
	  goto fail ;
	}
	if(param2 == NULL)
	{
	   printf("Parameter 1 was returned as NULL. Check the validity of the parameter.\n");
	   goto fail ;
	}

	/*create the appropriate string*/
	if (param2->type != hvt_integer)
	{
		printf("The String.Create([string],[type of complex string]) can have as a second parameter one of the following constants :[unicode],[concat],[var_expand]\n");
		goto fail;
	}

	if ((hdr_inter_var_gen_type(param1) != hdr_ivt_string)&&(param1->type != hvt_complex_string)&&(param1->type != hvt_complex_string_resolve))
	{
		printf("The String.Create([string],[type of complex string]) accepts as a first parameter a string type.\n");
		goto fail ;
	}

	if (param2->integer == HDR_INTER_SPECIAL_CONCAT)
	{
		/*
		  the concat complex string is in essence a string list that stores any string 
		  as an item. I will use the already existing PDX_STRINGLIST.
		*/

		/*create the list*/
		PDX_STRINGLIST strl = dx_stringlist_create() ;
		if (strl == NULL)
		{
			printf("MEMORY ALLOCATION ERROR. CREATE STRING LIST FAILED FOR CONCAT STRING TYPE.\n");
			goto fail;
		}
		*result = hdr_var_create(strl, "", hvf_temporary_ref, NULL); 
		(*result)->type = hvt_complex_string;

		/*add the string*/
		switch (param1->type)
		{
			case hvt_simple_string :
			case hvt_simple_string_bcktck:
			{
				//PDX_STRING nstr = dx_string_createU(NULL, ((PDX_STRING)param1->obj)->stringa);
				//if (nstr == NULL) goto fail ;
				dx_stringlist_load_text_ex(strl ,((PDX_STRING)param1->obj) , 512);
				//dx_stringlist_add_string(strl,nstr);
				goto success ;
			}break;
			case hvt_complex_string :
			{

				if (HDR_DOMAIN_STRING_CONCAT_CONCAT(*result, param1) == true) goto fail;
				else
					goto success;

			}break;
			case hvt_complex_string_resolve :
			{
				printf("The assignment of a [var_expand] string is not allowed. If you want to retrieve the resolved string, use the appropriate domain functions.\n");
				goto fail ;
			}break;
		}

	}
	else
	if (param2->integer == HDR_INTER_SPECIAL_UNICODE)
	{

		/*create the string*/
		DXCHAR ch[1] ;
		ch[0] = 0    ;
		PDX_STRING str = dx_string_createX(NULL,ch) ;
		if (str == NULL)
		{
			printf("MEMORY ALLOCATION ERROR. CREATE UNICODE STRING  FAILED FOR CONCAT STRING TYPE.\n");
			goto fail;
		}
		*result = hdr_var_create(str, "", hvf_temporary_ref, NULL); 
		(*result)->type = hvt_unicode_string;

		/*add the string*/
		switch (param1->type)
		{
			case hvt_simple_string :
			case hvt_simple_string_bcktck:
			{
				str = dx_string_createX_u(str, ((PDX_STRING)param1->obj)->stringa);
			}break;
			case hvt_unicode_string :
			{
				str = dx_string_createX(str, ((PDX_STRING)param1->obj)->stringx);
			}
			case hvt_complex_string :
			{
				/*
				  the string is a complex string, we have to construct the string from the lines 
				  and then concatenate it to the first string
				*/
				PDX_STRING tstr = dx_stringlist_raw_text((PDX_STRINGLIST)param1->obj) ;
				if (tstr == NULL)
				{
				 printf("The complex string typed 'concat' returned as NULL. A bug maybe ?\n");
				 goto fail ;
				}

				/*ok, now set the string*/
				str = dx_string_createX_u(str,tstr->stringa) ;
				dx_string_free(tstr);
				if(str == NULL)
				{
				  printf("The creation of the 'Unicode' string returned as NULL. A bug maybe or a malloc error?\n");
				  goto fail ;
				}
				goto success;

			}break;
			case hvt_complex_string_resolve :
			{
				printf("The assignment of a [var_expand] string is not allowed. If you want to retrieve the resolved string, use the appropriate domain functions.\n");
				goto fail ;
			}break;
		}
	}
	else
	if (param2->integer == HDR_INTER_SPECIAL_VAR_EXPAND)
	{
		PHDR_COMPLEX_STR_RES estr = hdr_res_str_create() ;
		if (estr == NULL)
		{
			printf("MEMORY ALLOCATION ERROR. CREATE STRING LIST FAILED FOR CONCAT STRING TYPE.\n");
			goto fail;
		}

		*result = hdr_var_create(estr, "", hvf_temporary_ref, NULL); 
		(*result)->type = hvt_complex_string_resolve ;

		/*add the string*/
		switch (param1->type)
		{
			case hvt_simple_string :
			case hvt_simple_string_bcktck:
			{
				PDX_STRING nstr =  (PDX_STRING)param1->obj;
				if(hdr_res_str_assign_str(estr,nstr) == NULL) goto fail ;
				else
					goto success ;
			}break;
			case hvt_complex_string :
			{
				if (hdr_res_str_add_strlist(estr,(PDX_STRINGLIST)param1->obj) == NULL) goto fail;
				else
					goto success;

			}break;
			case hvt_complex_string_resolve :
			{
				printf("The assigment of a [var_expant] to another [var_expant] is not allowed. The [var_expant] string has been already proccesed and cannot be revert back. Use the other string types.\n");
				goto fail ;
			}break;
		}

		goto fail ;
	}


	success :
	if (param1->var_ref == hvf_temporary_ref)
	{
		if (hdr_inter_var_gen_type(param1) == hdr_ivt_string)
		dx_string_free(((PDX_STRING)param1->obj));
		hdr_var_release_obj(param1) ;
		hdr_var_free(param1);
	}

	if (param2->var_ref == hvf_temporary_ref)
	{
		if (hdr_inter_var_gen_type(param2) == hdr_ivt_string)
		dx_string_free(((PDX_STRING)param2->obj));
		hdr_var_release_obj(param2) ;
		hdr_var_free(param2);
	}

	return false;/*success*/

fail:
	if(param1 != NULL)
	if (param1->var_ref == hvf_temporary_ref)
	{
		if (hdr_inter_var_gen_type(param1) == hdr_ivt_string)
		dx_string_free(((PDX_STRING)param1->obj));
		hdr_var_release_obj(param1) ;
		hdr_var_free(param1);
	}

	if(param2 != NULL)
	if (param2->var_ref == hvf_temporary_ref)
	{
		if (hdr_inter_var_gen_type(param2) == hdr_ivt_string)
		dx_string_free(((PDX_STRING)param2->obj));
		hdr_var_release_obj(param2) ;
		hdr_var_free(param2);
	}
	return true;/*error*/
}


bool hdr_domStringConcat(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token , PHDR_VAR target)
{
	/*the target is the string variable that we want to concatenate*/
    /*in theory all the checks have already be done , do the operation*/
	/*The Concat function can have from 1 to multiple parameters*/
	if (token->parameters->count == 0)
	{
		printf("The Concat() function needs at least one parameter of any [String] type, except [var_expand]. None was supplied.\n");
		return true;
	}

	for (int i = 0; i < token->parameters->count; i++)
	{
		PHDR_VAR param = hdr_inter_resolve_extract_var_from_expr(inter, token->parameters, i) ;
		
		if (param == NULL)
		{
			printf("Concat() : Parameter %d  resolved as NULL.\n",i+1);
			return true ;
		}

		if(target->type == hvt_complex_string)
		{
		  if (HDR_DOMAIN_STRING_CONCAT_CONCAT(target, param) == true ) return true ;
		}
		else
			if(target->type == hvt_complex_string_resolve)
			{
				  if(hdr_inter_var_gen_type(param) == hdr_ivt_string)
				  {
					if (hdr_res_str_add_str((PHDR_COMPLEX_STR_RES)target->obj,(PDX_STRING)param->obj) == NULL) 
					{
					  printf("Error in concatenation , the hdr_res_str_add_str() returned NULL\n");
					  return true ;
					}
				  }
				  else if(param->type == hvt_complex_string)
				  {
					if (hdr_res_str_add_strlist((PHDR_COMPLEX_STR_RES)target->obj,(PDX_STRINGLIST)param->obj) == NULL) 
					{
					  printf("Error in concatenation , the hdr_res_str_add_strlist() returned NULL\n");
					  return true ;
					}
				  }
				  else
					{
						printf("The variable type [%s] cannot concatenate with the target variable type [%s]\n",
							hdr_inter_return_variable_type(param->type),hdr_inter_return_variable_type(target->type));
						return true ; 
					}

			}

		if (param->var_ref == hvf_temporary_ref)
		{
			if(hdr_inter_var_gen_type(param) == hdr_ivt_string)
			dx_string_free(((PDX_STRING)param->obj));
			hdr_var_release_obj(param) ;
			hdr_var_free(param);
		}
		
	}
	
	return false;
}


bool hdr_domStringAssign(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token , PHDR_VAR target)
{
	/*the target is the string variable that we want to assign*/
    /*in theory all the checks have already be done , do the operation*/
	/*The Assign function can have only 1 parameter*/
	if (token->parameters->count != 1)
	{
		printf("The Assign() function needs exactly one parameter of any [String] type, except [var_expand].\n");
		return true;
	}

	PHDR_VAR param = hdr_inter_resolve_extract_var_from_expr(inter, token->parameters, 0) ;
		
	if (param == NULL)
	{
		printf("Assign() : parameter resolved as NULL.\n");
		return true ;
	}

	if((hdr_inter_var_gen_type(param) != hdr_ivt_string)&&(param->type != hvt_complex_string)&&(param->type != hvt_complex_string_resolve))
	{
		printf("Assign() : The parameter supplied is not of a valid type.\n");
		return true ;
	}

	if(target->type == hvt_complex_string)
	{
	  PDX_STRINGLIST lst = (PDX_STRINGLIST)target->obj ;
	  dx_stringlist_clear(lst) ;
	  HDR_DOMAIN_STRING_CONCAT_CONCAT(target, param);
	} 
	else
		if(target->type == hvt_complex_string_resolve)
		{
		  if(hdr_inter_var_gen_type(param) == hdr_ivt_string)
		  {
			  if(hdr_res_str_assign_str((PHDR_COMPLEX_STR_RES)target->obj,(PDX_STRING)param->obj) == NULL)
			  {
				  printf("Assign() : The assigment of the string was failed. [hdr_res_str_assign_str] returned NULL.\n");
				  return true ;
			  }
		  }
		  else 
			  if(param->type == hvt_complex_string)
			  {
				   if(hdr_res_str_assign_strlist ((PHDR_COMPLEX_STR_RES)target->obj,(PDX_STRINGLIST)param->obj) == NULL)
				   {
					  printf("Assign() : The assigment of the string was failed. [hdr_res_str_assign_str] returned NULL.\n");
					  return true ;
				   }
			  }
			  else
			  {
				printf("Assign() : The assigment failed. Keep in mind that you cannot assign a [var_expand] string in a [var_expand] string.\n");
				return true ;
			  }

		}
		else
		{
			printf("Assign() : The type is not supported.\n");
			return true ;
		}
	

	if (param->var_ref == hvf_temporary_ref)
	{
		if(hdr_inter_var_gen_type(param) == hdr_ivt_string)
		dx_string_free(((PDX_STRING)param->obj));
		hdr_var_release_obj(param) ;
		hdr_var_free(param);
	}

  
	return false ;
}


bool hdr_domStringExpand(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token , PHDR_VAR target)
{
	if (token->parameters->count != 0)
	{
		printf("The Expand() function does not needs any parameters.\n");
		return true;
	}

	if(hdr_res_str_expand(inter, (PHDR_COMPLEX_STR_RES)target->obj) == NULL)
	{
	  printf("Error expanding the variables in the [var_expand] string '%s'\n",target->name->stringa);
	  return true ; 
	}

	return false ;
}

bool hdr_domStringLength(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token, PHDR_VAR for_var,PHDR_VAR *result)
{

	/*return the string length of all the types of strings*/
	switch(for_var->type)
	{
		case hvt_simple_string :
		case hvt_simple_string_bcktck:
		case hvt_unicode_string:
		{	
			*result = hdr_var_create(NULL, "", hvf_temporary, NULL);
			(*result)->type = hvt_integer;
			(*result)->integer = ((PDX_STRING)for_var->obj)->len ;
		}break;
		case hvt_complex_string:
		{
			*result = hdr_var_create(NULL, "", hvf_temporary, NULL);
			(*result)->type = hvt_integer;
			(*result)->integer = dx_stringlist_strlen(((PDX_STRINGLIST)for_var->obj)) ;
		}break;
		case hvt_complex_string_resolve:
		{
			printf("The 'var_expand' string does not support character counts. \n");
			return true ;
		}break;
	
	}

	if((*result)->integer == -1) return true ;
	return false ;
}

bool hdr_domStringByteCount(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token, PHDR_VAR for_var,PHDR_VAR *result)
{
	/*return the string length in bytes of all the types of strings*/
	switch(for_var->type)
	{
		case hvt_simple_string :
		case hvt_simple_string_bcktck:
		case hvt_unicode_string:
		{	
			*result = hdr_var_create(NULL, "", hvf_temporary, NULL);
			(*result)->type = hvt_integer;
			(*result)->integer = ((PDX_STRING)for_var->obj)->bcount ;
		}break;
		case hvt_complex_string:
		{
			*result = hdr_var_create(NULL, "", hvf_temporary, NULL);
			(*result)->type = hvt_integer;
			(*result)->integer = dx_stringlist_bytelen(((PDX_STRINGLIST)for_var->obj)) ;
		}break;
		case hvt_complex_string_resolve:
		{
			printf("The 'var_expand' string does not support byte counts. \n");
			return true ;
		}break;
	
	}

	if((*result)->integer == -1) return true ;
	return false ;
}

bool hdr_domStringRemoveChar(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token, PHDR_VAR for_var,PHDR_VAR *result)
{
	/*
	 remove a character from all types of string
	 the function does not return a new string, returns the same string as it alters the for_var string 
	*/

	if (token->parameters->count != 1)
	{
		printf("The RemoveChar($char) function needs only one parametert. A character to remove (not the 0 character, it will be ignored).\n");
		return true;
	}
	
	PHDR_VAR param = hdr_inter_resolve_extract_var_from_expr(inter, token->parameters, 0) ;	
	if (param == NULL)
	{
		printf("RemoveChar($char) : The parameter  resolved as NULL.\n");
		return true ;
	}

	if((param->type != hvt_simple_string)&&(param->type != hvt_simple_string_bcktck))
	{
	    printf("RemoveChar($char) : The parameter is not of a simple string type.\n");
		goto fail ;
	}

	if(((PDX_STRING)param->obj)->len != 1)
	{
	    printf("RemoveChar($char) : The parameter is not a single utf8 char.\n");
		goto fail ;
	}

	switch(for_var->type)
	{
		case hvt_simple_string :
		case hvt_simple_string_bcktck:
		{
		  dx_utf8_remove_char((PDX_STRING)for_var->obj,(((PDX_STRING)param->obj)->stringa)) ;
		  *result = NULL ;
		}break;
		case hvt_unicode_string:
		{	
			PDX_STRING str = (PDX_STRING)param->obj ;
			DXCHAR thchar = dxConvertUTF8ToInt32(str->stringa,dxUtf8CharByteCount(str->stringa[0]));
			dx_unicode_remove_char((PDX_STRING)for_var->obj, thchar) ;
			*result = NULL ;
		}break;
		case hvt_complex_string:
		{
			dx_stringlist_remove_char((PDX_STRINGLIST)for_var->obj,(((PDX_STRING)param->obj)->stringa)) ;
			for_var->is_ref = true ;
			*result = NULL ;
		}break;
		case hvt_complex_string_resolve:
		{
			dx_stringlist_remove_char(((PHDR_COMPLEX_STR_RES)for_var->obj)->text_lines,(((PDX_STRING)param->obj)->stringa)) ;
			for_var->is_ref = true ;
			*result = NULL ;
		}break;
	
	}

	if (param->var_ref == hvf_temporary_ref)
	{
		if(hdr_inter_var_gen_type(param) == hdr_ivt_string)
		dx_string_free(((PDX_STRING)param->obj));
		hdr_var_release_obj(param) ;
		hdr_var_free(param);
	}

	return false ;

	fail :
	if (param->var_ref == hvf_temporary_ref)
	{
		if(hdr_inter_var_gen_type(param) == hdr_ivt_string)
		dx_string_free(((PDX_STRING)param->obj));
		hdr_var_release_obj(param) ;
		hdr_var_free(param);
	}
	return true ;

}

bool hdr_domStringReplaceChar(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token, PHDR_VAR for_var,PHDR_VAR *result)
{
	/*
	 replace a character from all types of string
	 the function does not return a new string, returns the same string as it alters the for_var string 
	*/

	if (token->parameters->count != 2)
	{
		printf("The ReplaceChar($char,$replace_with) function needs a character to replace and its replacement (not the 0 character, it will be ignored). None was supplied.\n");
		return true;
	}
	
	PHDR_VAR param1 = hdr_inter_resolve_extract_var_from_expr(inter, token->parameters, 0) ;	
	if (param1 == NULL)
	{
		printf("ReplaceChar($char,$replace_with) : The first parameter  resolved as NULL.\n");
		return true ;
	}

	PHDR_VAR param2 = hdr_inter_resolve_extract_var_from_expr(inter, token->parameters, 1) ;	
	if (param2 == NULL)
	{
		printf("ReplaceChar($char,$replace_with) : The second parameter  resolved as NULL.\n");
		return true ;
	}

	if((param1->type != hvt_simple_string)&&(param1->type != hvt_simple_string_bcktck))
	{
	    printf("ReplaceChar($char,$replace_with) : The first parameter is not of a simple string type.\n");
		goto fail ;
	}

	if((param2->type != hvt_simple_string)&&(param2->type != hvt_simple_string_bcktck))
	{
	    printf("ReplaceChar($char,$replace_with) : The second parameter is not of a simple string type.\n");
		goto fail ;
	}

	if(((PDX_STRING)param1->obj)->len != 1)
	{
	    printf("ReplaceChar($char,$replace_with) : The first parameter is not a single utf8 char.\n");
		goto fail ;
	}

	if(((PDX_STRING)param2->obj)->len != 1)
	{
	    printf("ReplaceChar($char,$replace_with) : The second parameter is not a single utf8 char.\n");
		goto fail ;
	}


	switch(for_var->type)
	{
		case hvt_simple_string :
		case hvt_simple_string_bcktck:
		{
		  dx_utf8_replace_char((PDX_STRING)for_var->obj,(((PDX_STRING)param1->obj)->stringa),(((PDX_STRING)param2->obj)->stringa)) ;
		 *result = NULL ;
		}break;
		case hvt_unicode_string:
		{	
			PDX_STRING str  = (PDX_STRING)param1->obj ;
			PDX_STRING str2 = (PDX_STRING)param2->obj ;
			DXCHAR thchar    = dxConvertUTF8ToInt32(str->stringa,dxUtf8CharByteCount(str->stringa[0]));
			DXCHAR repl_with = dxConvertUTF8ToInt32(str2->stringa,dxUtf8CharByteCount(str2->stringa[0]));
			dx_unicode_replace_char((PDX_STRING)for_var->obj, thchar,repl_with) ;
			*result = NULL ;
		}break;
		case hvt_complex_string:
		{
			dx_stringlist_replace_char((PDX_STRINGLIST)for_var->obj,(((PDX_STRING)param1->obj)->stringa),(((PDX_STRING)param2->obj)->stringa)) ;
			for_var->is_ref = true ;
			*result = NULL ;
		}break;
		case hvt_complex_string_resolve:
		{
			dx_stringlist_replace_char(((PHDR_COMPLEX_STR_RES)for_var->obj)->text_lines,(((PDX_STRING)param1->obj)->stringa),
				(((PDX_STRING)param2->obj)->stringa)) ;
			for_var->is_ref = true ;
			*result = NULL ;
		}break;
	
	}

	if (param1->var_ref == hvf_temporary_ref)
	{
		if(hdr_inter_var_gen_type(param1) == hdr_ivt_string)
		dx_string_free(((PDX_STRING)param1->obj));
		hdr_var_release_obj(param1) ;
		hdr_var_free(param1);
	}

	if (param2->var_ref == hvf_temporary_ref)
	{
		if(hdr_inter_var_gen_type(param2) == hdr_ivt_string)
		dx_string_free(((PDX_STRING)param2->obj));
		hdr_var_release_obj(param2) ;
		hdr_var_free(param2);
	}

	return false ;

	fail :
	if (param1->var_ref == hvf_temporary_ref)
	{
		if(hdr_inter_var_gen_type(param1) == hdr_ivt_string)
		dx_string_free(((PDX_STRING)param1->obj));
		hdr_var_release_obj(param1) ;
		hdr_var_free(param1);
	}

	if (param2->var_ref == hvf_temporary_ref)
	{
		if(hdr_inter_var_gen_type(param2) == hdr_ivt_string)
		dx_string_free(((PDX_STRING)param2->obj));
		hdr_var_release_obj(param2) ;
		hdr_var_free(param2);
	}

	return true ;

}


bool hdr_domStringFindWord(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token, PHDR_VAR for_var,PHDR_VAR *result)
{
	/*
	 The function searches for a word in all types of strings. If it finds it then returns the index 
	 in characters.
	 The first parameter is the word that we will found and the second will return the
	 index in bytes , else it will return -1 
	*/

	if (token->parameters->count != 2)
	{
		printf("The FindWord($word,$from_index) function needs a word to search for and the index that will be the starting point. Please check your parameters.\n");
		return true;
	}
	
	PHDR_VAR param1 = hdr_inter_resolve_extract_var_from_expr(inter, token->parameters, 0) ;	
	if (param1 == NULL)
	{
		printf("FindWord($word,$from_index) : The first parameter  resolved as NULL.\n");
		return true ;
	}

	PHDR_VAR param2 = hdr_inter_resolve_extract_var_from_expr(inter, token->parameters, 1) ;	
	if (param2 == NULL)
	{
		printf("FindWord($word,$from_index) : The second parameter  resolved as NULL.\n");
		return true ;
	}


	if((param1->type != hvt_simple_string)&&(param1->type != hvt_simple_string_bcktck))
	{
	    printf("FindWord($word,$from_index) : The first parameter is not of a simple string type.\n");
		goto fail ;
	}

	if((param2->type != hvt_integer)&&(param2->type != hvt_float))
	{
	    printf("FindWord($word,$from_index) : The second parameter is not of a Numeric type.\n");
		goto fail ;
	}


	if(((PDX_STRING)param1->obj)->len == 0)
	{
	    printf("FindWord($word,$from_index) : The first parameter is an empty string.\n");
		goto fail ;
	}

	switch(for_var->type)
	{
		case hvt_simple_string :
		case hvt_simple_string_bcktck:
		{
		   PDX_STRING str  = (PDX_STRING)param1->obj ;
		   DXLONG64   fr_indx = 0 ;
			if(param2->type == hvt_integer)
				fr_indx = param2->integer  ;
			else
				fr_indx = param2->real     ;

		   DXLONG64 char_indx   = 0 ;
		   char * res = dx_utf8_find_word((PDX_STRING)for_var->obj ,fr_indx ,str->stringa , &char_indx) ;
		   *result = hdr_var_create(NULL,"",hvf_temporary_ref,NULL) ;
		   (*result)->integer = char_indx ;
		   (*result)->type    = hvt_integer ;

		}break;
		case hvt_unicode_string:
		{	
			PDX_STRING str  = (PDX_STRING)param1->obj ;
			PDX_STRING word = dx_string_convertX(str) ;
			DXLONG64   fr_indx = 0 ;
			if(param2->type == hvt_integer)
				fr_indx = param2->integer  ;
			else
				fr_indx = param2->real     ;
           
			DXLONG64 res = dx_unicode_find_word((PDX_STRING)for_var->obj,fr_indx,word->stringx) ;
			dx_string_free(word) ;
			 
			*result = hdr_var_create(NULL,"",hvf_temporary_ref,NULL) ;
			(*result)->integer = res ;
			(*result)->type = hvt_integer ;
		}break;
		case hvt_complex_string:
		{
		   PDX_STRING str  = (PDX_STRING)param1->obj ;
		   DXLONG64   fr_indx = 0 ;
			if(param2->type == hvt_integer)
				fr_indx = param2->integer  ;
			else
				fr_indx = param2->real     ;

		   DXLONG64 char_indx   = 0 ;
		   DXLONG64 line        = 0 ;
		   DXLONG64 index =  dx_stringlist_find_word((PDX_STRINGLIST)for_var->obj ,str->stringa ,fr_indx ,&line,&char_indx) ;

		   *result = hdr_var_create(NULL,"",hvf_temporary_ref,NULL) ;
		   (*result)->integer = index       ;
		   (*result)->type    = hvt_integer ;
		}break;
		case hvt_complex_string_resolve:
		{
		   PDX_STRING str  = (PDX_STRING)param1->obj ;
		   DXLONG64   fr_indx = 0 ;
			if(param2->type == hvt_integer)
				fr_indx = param2->integer  ;
			else
				fr_indx = param2->real     ;

		   DXLONG64 char_indx   = 0 ;
		   DXLONG64 line        = 0 ;
		   PDX_STRINGLIST strl  = ((PHDR_COMPLEX_STR_RES)for_var->obj)->text_lines ; 
		   DXLONG64 index =  dx_stringlist_find_word(strl ,str->stringa ,fr_indx ,&line,&char_indx) ;
		   *result = hdr_var_create(NULL,"",hvf_temporary_ref,NULL) ;
		   (*result)->integer = index       ;
		   (*result)->type    = hvt_integer ;
		}break;
	
	}

	if (param1->var_ref == hvf_temporary_ref)
	{
		if(hdr_inter_var_gen_type(param1) == hdr_ivt_string)
		dx_string_free(((PDX_STRING)param1->obj));
		hdr_var_release_obj(param1) ;
		hdr_var_free(param1);
	}

	if (param2->var_ref == hvf_temporary_ref)
	{
		if(hdr_inter_var_gen_type(param2) == hdr_ivt_string)
		dx_string_free(((PDX_STRING)param2->obj));
		hdr_var_release_obj(param2) ;
		hdr_var_free(param2);
	}

	return false ;

	fail :
	if (param1->var_ref == hvf_temporary_ref)
	{
		if(hdr_inter_var_gen_type(param1) == hdr_ivt_string)
		dx_string_free(((PDX_STRING)param1->obj));
		hdr_var_release_obj(param1) ;
		hdr_var_free(param1);
	}

	if (param2->var_ref == hvf_temporary_ref)
	{
		if(hdr_inter_var_gen_type(param2) == hdr_ivt_string)
		dx_string_free(((PDX_STRING)param2->obj));
		hdr_var_release_obj(param2) ;
		hdr_var_free(param2);
	}

	return true ;

}

bool hdr_domStringFindWordBinary(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token, PHDR_VAR for_var,PHDR_VAR *result)
{
	/*
	 The function searches for a word in all types of strings. If it finds it then returns the index 
	 in characters.
	 The first parameter is the word that we will found and the second will return the
	 index in bytes , else it will return -1 
	*/

	if (token->parameters->count != 2)
	{
		printf("The String.FindWordBinary($word,$from_index):Integer (-1 not found or index of first char of the word) function needs a word to search for and the index that will be the starting point. Please check your parameters.\n");
		return true;
	}
	
	PHDR_VAR param1 = hdr_inter_resolve_extract_var_from_expr(inter, token->parameters, 0) ;	
	if (param1 == NULL)
	{
		printf("String.FindWordBinary($word,$from_index):Integer (-1 not found or index of first char of the word) : The first parameter  resolved as NULL.\n");
		return true ;
	}

	PHDR_VAR param2 = hdr_inter_resolve_extract_var_from_expr(inter, token->parameters, 1) ;	
	if (param2 == NULL)
	{
		printf("String.FindWordBinary($word,$from_index):Integer (-1 not found or index of first char of the word) : The second parameter  resolved as NULL.\n");
		return true ;
	}


	if((param1->type != hvt_simple_string)&&(param1->type != hvt_simple_string_bcktck))
	{
	    printf("String.FindWordBinary($word,$from_index):Integer (-1 not found or index of first char of the word) : The first parameter is not of a simple string type.\n");
		goto fail ;
	}

	if((param2->type != hvt_integer)&&(param2->type != hvt_float))
	{
	    printf("String.FindWordBinary($word,$from_index):Integer (-1 not found or index of first char of the word) : The second parameter is not of a Numeric type.\n");
		goto fail ;
	}


	if(((PDX_STRING)param1->obj)->len == 0)
	{
	    printf("String.FindWordBinary($word,$from_index):Integer (-1 not found or index of first char of the word) : The first parameter is an empty string.\n");
		goto fail ;
	}

	switch(for_var->type)
	{
		case hvt_simple_string :
		case hvt_simple_string_bcktck:
		{
		   PDX_STRING str  = (PDX_STRING)param1->obj ;
		   DXLONG64   fr_indx = 0 ;
			if(param2->type == hvt_integer)
				fr_indx = param2->integer  ;
			else
				fr_indx = param2->real     ;

		   DXLONG64 char_indx   = 0 ;
		  char *res = dx_binary_find_word((PDX_STRING)for_var->obj , &fr_indx , str->stringa) ;

		  if (res == NULL) fr_indx = -1 ;
		   *result = hdr_var_create(NULL,"",hvf_temporary_ref,NULL) ;
		   (*result)->integer = fr_indx ;
		   (*result)->type    = hvt_integer ;

		}break;
		case hvt_unicode_string:
		{	
			/*unicode string search is binary as it has not weird encoding, just bigger characters so no changes in code */
			PDX_STRING str  = (PDX_STRING)param1->obj ;
			PDX_STRING word = dx_string_convertX(str) ;
			DXLONG64   fr_indx = 0 ;
			if(param2->type == hvt_integer)
				fr_indx = param2->integer  ;
			else
				fr_indx = param2->real     ;
           
			DXLONG64 res = dx_unicode_find_word((PDX_STRING)for_var->obj,fr_indx,word->stringx) ;
			dx_string_free(word) ;
			 
			*result = hdr_var_create(NULL,"",hvf_temporary_ref,NULL) ;
			(*result)->integer = res ;
			(*result)->type = hvt_integer ;
		}break;
		case hvt_complex_string:
		{
		   PDX_STRING str  = (PDX_STRING)param1->obj ;
		   DXLONG64   fr_indx = 0 ;
			if(param2->type == hvt_integer)
				fr_indx = param2->integer  ;
			else
				fr_indx = param2->real     ;

		   DXLONG64 char_indx   = 0 ;
		   DXLONG64 line        = 0 ;
		   DXLONG64 index		=  dx_stringlist_find_word_binary((PDX_STRINGLIST)for_var->obj ,str->stringa ,fr_indx ,&line,&char_indx) ;

		   *result = hdr_var_create(NULL,"",hvf_temporary_ref,NULL) ;
		   (*result)->integer = index       ;
		   (*result)->type    = hvt_integer ;
		}break;
		case hvt_complex_string_resolve:
		{
		   PDX_STRING str  = (PDX_STRING)param1->obj ;
		   DXLONG64   fr_indx = 0 ;
			if(param2->type == hvt_integer)
				fr_indx = param2->integer  ;
			else
				fr_indx = param2->real     ;

		   DXLONG64 char_indx   = 0 ;
		   DXLONG64 line        = 0 ;
		   PDX_STRINGLIST strl  = ((PHDR_COMPLEX_STR_RES)for_var->obj)->text_lines ;
		   DXLONG64 index		=  dx_stringlist_find_word_binary(strl ,str->stringa ,fr_indx ,&line,&char_indx) ;

		   *result = hdr_var_create(NULL,"",hvf_temporary_ref,NULL) ;
		   (*result)->integer = index       ;
		   (*result)->type    = hvt_integer ;
		}break;
	
	}

	if (param1->var_ref == hvf_temporary_ref)
	{
		if(hdr_inter_var_gen_type(param1) == hdr_ivt_string)
		dx_string_free(((PDX_STRING)param1->obj));
		hdr_var_release_obj(param1) ;
		hdr_var_free(param1);
	}

	if (param2->var_ref == hvf_temporary_ref)
	{
		if(hdr_inter_var_gen_type(param2) == hdr_ivt_string)
		dx_string_free(((PDX_STRING)param2->obj));
		hdr_var_release_obj(param2) ;
		hdr_var_free(param2);
	}

	return false ;

	fail :
	if (param1->var_ref == hvf_temporary_ref)
	{
		if(hdr_inter_var_gen_type(param1) == hdr_ivt_string)
		dx_string_free(((PDX_STRING)param1->obj));
		hdr_var_release_obj(param1) ;
		hdr_var_free(param1);
	}

	if (param2->var_ref == hvf_temporary_ref)
	{
		if(hdr_inter_var_gen_type(param2) == hdr_ivt_string)
		dx_string_free(((PDX_STRING)param2->obj));
		hdr_var_release_obj(param2) ;
		hdr_var_free(param2);
	}

	return true ;

}

bool hdr_domStringReplace(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token, PHDR_VAR for_var,PHDR_VAR *result)
{
	/*
	 replace a word from all types of string
	 the function does not return a new string, returns the same string as it alters the for_var string 
	*/

	if (token->parameters->count != 4)
	{
		printf("The Replace($word,$with_word,$from_indx,$replace_all):[character indx] function needs a word to replace and its replacement.\n");
		return true;
	}
	
	PHDR_VAR param1 = hdr_inter_resolve_extract_var_from_expr(inter, token->parameters, 0) ;	
	if (param1 == NULL)
	{
		printf("Replace($word,$with_word,$from_indx,$replace_all):[character indx] The first parameter  resolved as NULL.\n");
		return true ;
	}

	PHDR_VAR param2 = hdr_inter_resolve_extract_var_from_expr(inter, token->parameters, 1) ;	
	if (param2 == NULL)
	{
		printf("Replace($word,$with_word,$from_indx,$replace_all):[character indx] The second parameter  resolved as NULL.\n");
		return true ;
	}

	PHDR_VAR param3 = hdr_inter_resolve_extract_var_from_expr(inter, token->parameters, 2) ;	
	if (param2 == NULL)
	{
		printf("Replace($word,$with_word,$from_indx,$replace_all):[character indx] The third parameter  resolved as NULL.\n");
		return true ;
	}

	PHDR_VAR param4 = hdr_inter_resolve_extract_var_from_expr(inter, token->parameters, 3) ;	
	if (param2 == NULL)
	{
		printf("Replace($word,$with_word,$from_indx,$replace_all):[character indx] The fourth parameter  resolved as NULL.\n");
		return true ;
	}


	if((param1->type != hvt_simple_string)&&(param1->type != hvt_simple_string_bcktck))
	{
	    printf("Replace($word,$with_word,$from_indx,$replace_all):[character indx] The first parameter is not of a simple string type.\n");
		goto fail ;
	}

	if((param2->type != hvt_simple_string)&&(param2->type != hvt_simple_string_bcktck))
	{
	    printf("Replace($word,$with_word,$from_indx,$replace_all):[character indx] The second parameter is not of a simple string type.\n");
		goto fail ;
	}

	if((param3->type != hvt_integer)&&(param3->type != hvt_float))
	{
	    printf("Replace($word,$with_word,$from_indx,$replace_all):[character indx] The third parameter is not of a Numeric type.\n");
		goto fail ;
	}

	if(param4->type != hvt_bool)
	{
	    printf("Replace($word,$with_word,$from_indx,$replace_all):[character indx] The fourth parameter is not of a Boolean type.\n");
		goto fail ;
	}

	switch(for_var->type)
	{
		case hvt_simple_string :
		case hvt_simple_string_bcktck:
		{

		 DXLONG64 indx = 0 ; 
		 if(param3->type == hvt_integer) indx = param3->integer ;
		 else
			 indx = param3->real ;
		 
		 DXLONG64 index = dx_utf8_replace_word((PDX_STRING)for_var->obj,indx, (PDX_STRING)param1->obj,
		 (PDX_STRING)param2->obj,param4->integer) ;
		 
		 *result = hdr_var_create(NULL,"",hvf_temporary_ref,NULL) ;
		 (*result)->integer = index       ;
		 (*result)->type    = hvt_integer ;

		}break;
		case hvt_unicode_string:
		{	
			 DXLONG64 indx = 0 ; 
			 if(param3->type == hvt_integer) indx = param3->integer ;
			 else
				 indx = param3->real ;
		 
			 PDX_STRING word      = dx_string_convertX((PDX_STRING)param1->obj) ;
			 PDX_STRING repl_with = dx_string_convertX((PDX_STRING)param2->obj) ;
			 DXLONG64 index = dx_replace_word_unicode((PDX_STRING)for_var->obj,indx,word ,
			 repl_with,param4->integer) ;
		 
			 dx_string_free(word)      ;
			 dx_string_free(repl_with) ;
			 *result = hdr_var_create(NULL,"",hvf_temporary_ref,NULL) ;
			 (*result)->integer = index       ;
			 (*result)->type    = hvt_integer ;
		}break;
		case hvt_complex_string:
		{
		    printf("The 'concat' string type does not support word replacing. \n");
			*result = NULL ;
			goto fail ;
		}break;
		case hvt_complex_string_resolve:
		{
			printf("The 'var_expand' string type does not support word replacing. \n");
			*result = NULL ;
			goto fail ;
		}break;
	
	}

	hdr_inter_param_free(param1);
	hdr_inter_param_free(param2);
	hdr_inter_param_free(param3);
	hdr_inter_param_free(param4);
	

	return false ;

	fail :

	hdr_inter_param_free(param1);
	hdr_inter_param_free(param2);
	hdr_inter_param_free(param3);
	hdr_inter_param_free(param4);

	return true ;

}

bool hdr_domStringReplaceBinary(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token, PHDR_VAR for_var,PHDR_VAR *result)
{
	/*
	 replace a word from all types of string
	 the function does not return a new string, returns the same string as it alters the for_var string 
	*/

	if (token->parameters->count != 4)
	{
		printf("The ReplaceBinary($word,$with_word,$from_indx,$replace_all):[character indx] function needs a word to replace and its replacement.\n");
		return true;
	}
	
	PHDR_VAR param1 = hdr_inter_resolve_extract_var_from_expr(inter, token->parameters, 0) ;	
	if (param1 == NULL)
	{
		printf("ReplaceBinary($word,$with_word,$from_indx,$replace_all):[character indx] The first parameter  resolved as NULL.\n");
		return true ;
	}

	PHDR_VAR param2 = hdr_inter_resolve_extract_var_from_expr(inter, token->parameters, 1) ;	
	if (param2 == NULL)
	{
		printf("ReplaceBinary($word,$with_word,$from_indx,$replace_all):[character indx] The second parameter  resolved as NULL.\n");
		return true ;
	}

	PHDR_VAR param3 = hdr_inter_resolve_extract_var_from_expr(inter, token->parameters, 2) ;	
	if (param2 == NULL)
	{
		printf("ReplaceBinary($word,$with_word,$from_indx,$replace_all):[character indx] The third parameter  resolved as NULL.\n");
		return true ;
	}

	PHDR_VAR param4 = hdr_inter_resolve_extract_var_from_expr(inter, token->parameters, 3) ;	
	if (param2 == NULL)
	{
		printf("ReplaceBinary($word,$with_word,$from_indx,$replace_all):[character indx] The fourth parameter  resolved as NULL.\n");
		return true ;
	}


	if((param1->type != hvt_simple_string)&&(param1->type != hvt_simple_string_bcktck))
	{
	    printf("ReplaceBinary($word,$with_word,$from_indx,$replace_all):[character indx] The first parameter is not of a simple string type.\n");
		goto fail ;
	}

	if((param2->type != hvt_simple_string)&&(param2->type != hvt_simple_string_bcktck))
	{
	    printf("ReplaceBinary($word,$with_word,$from_indx,$replace_all):[character indx] The second parameter is not of a simple string type.\n");
		goto fail ;
	}

	if((param3->type != hvt_integer)&&(param3->type != hvt_float))
	{
	    printf("ReplaceBinary($word,$with_word,$from_indx,$replace_all):[character indx] The third parameter is not of a Numeric type.\n");
		goto fail ;
	}

	if(param4->type != hvt_bool)
	{
	    printf("ReplaceBinary($word,$with_word,$from_indx,$replace_all):[character indx] The fourth parameter is not of a Boolean type.\n");
		goto fail ;
	}

	switch(for_var->type)
	{
		case hvt_simple_string :
		case hvt_simple_string_bcktck:
		{

		 DXLONG64 indx = 0 ; 
		 if(param3->type == hvt_integer) indx = param3->integer ;
		 else
			 indx = param3->real ;
		 
		 DXLONG64 index = dx_replace_word_binary((PDX_STRING)for_var->obj,indx, (PDX_STRING)param1->obj,
		 (PDX_STRING)param2->obj,param4->integer) ;
		 
		 *result = hdr_var_create(NULL,"",hvf_temporary_ref,NULL) ;
		 (*result)->integer = index       ;
		 (*result)->type    = hvt_integer ;

		}break;
		case hvt_unicode_string:
		{	 
			DXLONG64 indx = 0 ; 
			 if(param3->type == hvt_integer) indx = param3->integer ;
			 else
				 indx = param3->real ;
		 
			 PDX_STRING word      = dx_string_convertX((PDX_STRING)param1->obj) ;
			 PDX_STRING repl_with = dx_string_convertX((PDX_STRING)param2->obj) ;
			
			 DXLONG64 index = dx_replace_word_unicode((PDX_STRING)for_var->obj,indx,word ,
			 repl_with,param4->integer) ;
		 
			 dx_string_free(word)      ;
			 dx_string_free(repl_with) ;
			 *result = hdr_var_create(NULL,"",hvf_temporary_ref,NULL) ;
			 (*result)->integer = index       ;
			 (*result)->type    = hvt_integer ;

		}break;
		case hvt_complex_string:
		{
			printf("The 'concat' string type does not support word replacing. \n");
			*result = NULL ;
			goto fail ;
		}break;
		case hvt_complex_string_resolve:
		{
			printf("The 'var_expand' string type does not support word replacing.\n");
			*result = NULL ;
			goto fail ;
		}break;
	
	}

	hdr_inter_param_free(param1);
	hdr_inter_param_free(param2);
	hdr_inter_param_free(param3);
	hdr_inter_param_free(param4);
	

	return false ;

	fail :

	hdr_inter_param_free(param1);
	hdr_inter_param_free(param2);
	hdr_inter_param_free(param3);
	hdr_inter_param_free(param4);

	return true ;

}


bool hdr_domStringToSimple(PHDR_INTERPRETER inter , PHDR_COMPLEX_TOKEN token, PHDR_VAR for_var,PHDR_VAR *result)
{
	/*construct a simple string from a concat string*/

	if (token->parameters->count != 0)
	{
		printf("The ToSimple() function does not accept any parameters.\n");
		return true;
	}

	PDX_STRING str = NULL ;
	if(for_var->type == hvt_complex_string) str = dx_stringlist_raw_text((PDX_STRINGLIST)for_var->obj);
	else
		str = hdr_convert_str_expand(inter,(PHDR_COMPLEX_STR_RES)for_var->obj);
	if(str == NULL) return false ;
	*result			= hdr_var_create(str,"",hvf_temporary_ref,NULL) ;
	(*result)->type = hvt_simple_string ;

	return false ;
}

bool hdr_domStringExpandToSimple(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token , PHDR_VAR for_var,PHDR_VAR *result)
{

	/*
	  This function creates a simple string replaceing the variables names with the actual current values
	  WITHOUT altering the iternal state of the string. This is usefull if the application needs to keep the previous values 
	  that have been set with the Expand. But the bigger perc of this function is that is thread safe as it creates new
	  memory and does not alter any shared one.
	*/

	if (token->parameters->count != 0)
	{
		printf("The ExpandToSimple() function does not needs any parameters.\n");
		return true;
	}

	PDX_STRING str = NULL ;
	if(for_var->type == hvt_complex_string_resolve) str = hdr_res_str_temp_expand(inter,(PHDR_COMPLEX_STR_RES)for_var->obj);
	if(str == NULL) return false ;
	*result			= hdr_var_create(str,"",hvf_temporary_ref,NULL) ;
	(*result)->type = hvt_simple_string ;

	return false ;
}


bool hdr_domStringReleaseMem(PHDR_VAR for_var)
{
   if(for_var->obj == NULL) return false ;
   if(for_var->type == hvt_complex_string) dx_stringlist_free((PDX_STRINGLIST)for_var->obj) ;
   else
	if(for_var->type == hvt_complex_string_resolve) hdr_res_str_free((PHDR_COMPLEX_STR_RES)for_var->obj) ;
   hdr_var_release_obj(for_var) ;
   for_var->type = hvt_undefined ;

   return false ;
}


bool hdr_domStringEmpty(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token, PHDR_VAR for_var)
{

	/*empty the data of all types of strings*/
	switch(for_var->type)
	{
		case hvt_simple_string :
		case hvt_simple_string_bcktck:
		case hvt_unicode_string:
		{	
			dx_string_createU(((PDX_STRING)for_var->obj),"") ;
		}break;
		case hvt_complex_string:
		{
			dx_stringlist_clear(((PDX_STRINGLIST)for_var->obj)) ;
		}break;
		case hvt_complex_string_resolve:
		{
			PDX_STRINGLIST strl = ((PHDR_COMPLEX_STR_RES)for_var->obj)->text_lines ;
			dx_stringlist_clear(strl) ;
			PDX_LIST lst = ((PHDR_COMPLEX_STR_RES)for_var->obj)->vars_pos ;

			PDXL_NODE node = lst->start ;
			while(node != NULL)
			{
				PDXL_OBJECT obj = node->object ;
				dx_string_free(obj->key);
				free(obj) ;
				node = node->right ;
			}

			dx_list_empty_list(lst) ;


		}break;
	
	}

	return false ;
}


bool hdr_domStringCopyIndxBinary(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token, PHDR_VAR for_var,PHDR_VAR *result)
{
	/*
	 copy the string from an index to an index. If the end index is bigger than the string length then
	 the string until the end is returned
	*/

	if (token->parameters->count != 2)
	{
		printf("The CopyIndexBinary($from_indx,$to_indx):[simple string] function needs both the start and end indexes ( $to_indx = -1 for the end of the string).\n");
		return true;
	}
	
	PHDR_VAR param1 = hdr_inter_resolve_extract_var_from_expr(inter, token->parameters, 0) ;	
	if (param1 == NULL)
	{
		printf("CopyIndexBinary($from_indx,$to_indx):[simple string] The first parameter  resolved as NULL.\n");
		return true ;
	}

	PHDR_VAR param2 = hdr_inter_resolve_extract_var_from_expr(inter, token->parameters, 1) ;	
	if (param2 == NULL)
	{
		printf("CopyIndexBinary($from_indx,$to_indx):[simple string] The second parameter  resolved as NULL.\n");
		return true ;
	}


	if((param1->type != hvt_integer)&&(param1->type != hvt_float))
	{
	    printf("CopyIndexBinary($from_indx,$to_indx):[simple string] The first parameter is not of a Numeric type.\n");
		goto fail ;
	}

	if((param2->type != hvt_integer)&&(param2->type != hvt_float))
	{
	   printf("CopyIndexBinary($from_indx,$to_indx):[simple string] The second parameter is not of a Numeric type.\n");
		goto fail ;
	}

	switch(for_var->type)
	{
		case hvt_simple_string :
		case hvt_simple_string_bcktck:
		{

		 DXLONG64 from_indx = 0 ;
		 DXLONG64 to_indx = 0 ;
		 if(param1->type == hvt_integer) from_indx = param1->integer ;
		 else
			 from_indx = param1->real ;
		 
		 if(param2->type == hvt_integer) to_indx = param2->integer ;
		 else
			 to_indx = param2->real ;


		 PDX_STRING str = CopyIndxToIndx((PDX_STRING)for_var->obj,from_indx,to_indx) ;
		 
		 *result = hdr_var_create(NULL,"",hvf_temporary_ref,NULL) ;
		 hdr_var_set_obj(*result,str)		  ;
		 (*result)->type   = hvt_simple_string ;

		}break;
		case hvt_unicode_string:
		{	
			printf("The 'unicode' string type does not support copying. \n");
			*result = NULL ;
			goto fail ;

		}break;
		case hvt_complex_string:
		{
		    printf("The 'concat' string type does not support copying. \n");
			*result = NULL ;
			goto fail ;
		}break;
		case hvt_complex_string_resolve:
		{
			printf("The 'var_expand' string type does not support word copying. \n");
			*result = NULL ;
			goto fail ;
		}break;
	
	}

	hdr_inter_param_free(param1);
	hdr_inter_param_free(param2);
	

	return false ;

	fail :

	hdr_inter_param_free(param1);
	hdr_inter_param_free(param2);

	return true ;

}


bool hdr_domStringCopyBinary(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token, PHDR_VAR for_var,PHDR_VAR *result)
{
	/*
	 copy the string from an index to an index. If the end index is bigger than the string length then
	 the string until the end is returned
	*/

	if (token->parameters->count != 2)
	{
		printf("The CopyBinary($from_indx,$char_count):[simple string] function needs both the start index and the character count to copy ( $char_count = -1 for the end of the string).\n");
		return true;
	}
	
	PHDR_VAR param1 = hdr_inter_resolve_extract_var_from_expr(inter, token->parameters, 0) ;	
	if (param1 == NULL)
	{
		printf("CopyBinary($from_indx,$char_count):[simple string] The first parameter  resolved as NULL.\n");
		return true ;
	}

	PHDR_VAR param2 = hdr_inter_resolve_extract_var_from_expr(inter, token->parameters, 1) ;	
	if (param2 == NULL)
	{
		printf("CopyBinary($from_indx,$char_count):[simple string] The second parameter  resolved as NULL.\n");
		return true ;
	}


	if((param1->type != hvt_integer)&&(param1->type != hvt_float))
	{
	    printf("CopyBinary($from_indx,$char_count):[simple string] The first parameter is not of a Numeric type.\n");
		goto fail ;
	}

	if((param2->type != hvt_integer)&&(param2->type != hvt_float))
	{
	   printf("CopyBinary($from_indx,$char_count):[simple string] The second parameter is not of a Numeric type.\n");
		goto fail ;
	}

	switch(for_var->type)
	{
		case hvt_simple_string :
		case hvt_simple_string_bcktck:
		{

		 DXLONG64 from_indx = 0 ;
		 DXLONG64 char_len  = 0 ;
		 if(param1->type == hvt_integer) from_indx = param1->integer ;
		 else
			 from_indx = param1->real ;
		 
		 if(param2->type == hvt_integer) char_len = param2->integer ;
		 else
			 char_len = param2->real ;


		 PDX_STRING str = CopyIndx((PDX_STRING)for_var->obj,from_indx,char_len) ;
		 
		 *result = hdr_var_create(NULL,"",hvf_temporary_ref,NULL) ;
		 hdr_var_set_obj(*result,str) ;   
		 (*result)->type   = hvt_simple_string ;

		}break;
		case hvt_unicode_string:
		{	
			printf("The 'unicode' string type does not support copying. \n");
			*result = NULL ;
			goto fail ;
			 
		}break;
		case hvt_complex_string:
		{
		    printf("The 'concat' string type does not support copying. \n");
			*result = NULL ;
			goto fail ;
		}break;
		case hvt_complex_string_resolve:
		{
			printf("The 'var_expand' string type does not support word copying. \n");
			*result = NULL ;
			goto fail ;
		}break;
	
	}

	hdr_inter_param_free(param1);
	hdr_inter_param_free(param2);
	

	return false ;

	fail :

	hdr_inter_param_free(param1);
	hdr_inter_param_free(param2);

	return true ;

}

bool hdr_domStringCopyCharBinary(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token, PHDR_VAR for_var,PHDR_VAR *result)
{
	/*
	 copy the string from an index to an index. If the end index is bigger than the string length then
	 the string until the end is returned
	*/

	if (token->parameters->count != 2)
	{
		printf("The CopyCharBinary($from_indx,$char):[simple string] function needs both the start index and the terminal character.\n");
		return true;
	}
	
	PHDR_VAR param1 = hdr_inter_resolve_extract_var_from_expr(inter, token->parameters, 0) ;	
	if (param1 == NULL)
	{
		printf("CopyCharBinary($from_indx,$char):[simple string] The first parameter  resolved as NULL.\n");
		return true ;
	}

	PHDR_VAR param2 = hdr_inter_resolve_extract_var_from_expr(inter, token->parameters, 1) ;	
	if (param2 == NULL)
	{
		printf("CopyCharBinary($from_indx,$char):[simple string] The second parameter  resolved as NULL.\n");
		return true ;
	}


	if((param1->type != hvt_integer)&&(param1->type != hvt_float))
	{
	    printf("CopyCharBinary($from_indx,$char):[simple string] The first parameter is not of a Numeric type.\n");
		goto fail ;
	}

	if((param2->type != hvt_simple_string)&&(param2->type != hvt_simple_string_bcktck)&&(param2->type != hvt_codepoint))
	{
	   printf("CopyCharBinary($from_indx,$char):[simple string] The second parameter is not of a string or character type.\n");
		goto fail ;
	}

	if(((param2->type != hvt_simple_string)&&(param2->type != hvt_simple_string_bcktck))&&(((PDX_STRING)param2->obj)->len != 1))
	{
	   printf("CopyCharBinary($from_indx,$char):[simple string] The second parameter is not a single character.\n");
	   goto fail ;
	}

	char th_char ;
	if(param2->type == hvt_codepoint) th_char = (char)param2->integer ;
	else
		th_char = ((PDX_STRING)param2->obj)->stringa[0] ;
	
	switch(for_var->type)
	{
		case hvt_simple_string :
		case hvt_simple_string_bcktck:
		{

		 DXLONG64 from_indx = 0 ;
		 if(param1->type == hvt_integer) from_indx = param1->integer ;
		 else
			 from_indx = param1->real ;


		 char *buff = CopyStrToChar(((PDX_STRING)for_var->obj)->stringa, &from_indx , th_char)  ;
		 PDX_STRING str = dx_string_create_bU(buff);

		 if(param1->type == hvt_integer) param1->integer = from_indx ;
		 else
			 param1->real = from_indx ;

		 *result = hdr_var_create(NULL,"",hvf_temporary_ref,NULL) ;
		 hdr_var_set_obj(*result,str)		  ;
		 (*result)->type   = hvt_simple_string ;

		}break;
		case hvt_unicode_string:
		{	
			printf("The 'unicode' string type does not support copying. \n");
			*result = NULL ;
			goto fail ;

		}break;
		case hvt_complex_string:
		{
		    printf("The 'concat' string type does not support copying. \n");
			*result = NULL ;
			goto fail ;
		}break;
		case hvt_complex_string_resolve:
		{
			printf("The 'var_expand' string type does not support word copying. \n");
			*result = NULL ;
			goto fail ;
		}break;
	
	}

	hdr_inter_param_free(param1);
	hdr_inter_param_free(param2);
	

	return false ;

	fail :

	hdr_inter_param_free(param1);
	hdr_inter_param_free(param2);

	return true ;

}


bool hdr_domStringCopyIndx(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token, PHDR_VAR for_var,PHDR_VAR *result)
{
	/*
	 copy the string from an index to an index. If the end index is bigger than the string length then
	 the string until the end is returned
	*/

	if (token->parameters->count != 2)
	{
		printf("The CopyIndx($from_indx,$to_indx):[simple string] function needs both the start and end indexes ( $to_indx = -1 for the end of the string).\n");
		return true;
	}
	
	PHDR_VAR param1 = hdr_inter_resolve_extract_var_from_expr(inter, token->parameters, 0) ;	
	if (param1 == NULL)
	{
		printf("CopyIndx($from_indx,$to_indx):[simple string] The first parameter  resolved as NULL.\n");
		return true ;
	}

	PHDR_VAR param2 = hdr_inter_resolve_extract_var_from_expr(inter, token->parameters, 1) ;	
	if (param2 == NULL)
	{
		printf("CopyIndx($from_indx,$to_indx):[simple string] The second parameter  resolved as NULL.\n");
		return true ;
	}


	if((param1->type != hvt_integer)&&(param1->type != hvt_float))
	{
	    printf("CopyIndx($from_indx,$to_indx):[simple string] The first parameter is not of a Numeric type.\n");
		goto fail ;
	}

	if((param2->type != hvt_integer)&&(param2->type != hvt_float))
	{
	   printf("CopyIndx($from_indx,$to_indx):[simple string] The second parameter is not of a Numeric type.\n");
		goto fail ;
	}

	switch(for_var->type)
	{
		case hvt_simple_string :
		case hvt_simple_string_bcktck:
		{

		 DXLONG64 from_indx = 0 ;
		 DXLONG64 to_indx = 0 ;
		 if(param1->type == hvt_integer) from_indx = param1->integer ;
		 else
			 from_indx = param1->real ;
		 
		 if(param2->type == hvt_integer) to_indx = param2->integer ;
		 else
			 to_indx = param2->real ;


		 char *buf = utf8CopyIndexToIndex(((PDX_STRING)for_var->obj)->stringa,from_indx,to_indx) ;
		 PDX_STRING str = dx_string_create_bU(buf) ;

		 *result = hdr_var_create(NULL,"",hvf_temporary_ref,NULL) ;
		 hdr_var_set_obj(*result,str)		  ;
		 (*result)->type   = hvt_simple_string ;

		}break;
		case hvt_unicode_string:
		{	
			printf("The 'unicode' string type does not support copying. \n");
			*result = NULL ;
			goto fail ;

		}break;
		case hvt_complex_string:
		{
		    printf("The 'concat' string type does not support copying. \n");
			*result = NULL ;
			goto fail ;
		}break;
		case hvt_complex_string_resolve:
		{
			printf("The 'var_expand' string type does not support word copying. \n");
			*result = NULL ;
			goto fail ;
		}break;
	
	}

	hdr_inter_param_free(param1);
	hdr_inter_param_free(param2);
	

	return false ;

	fail :

	hdr_inter_param_free(param1);
	hdr_inter_param_free(param2);

	return true ;

}


bool hdr_domStringCopy(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token, PHDR_VAR for_var,PHDR_VAR *result)
{
	/*
	 copy the string from an index to an index. If the end index is bigger than the string length then
	 the string until the end is returned
	*/

	if (token->parameters->count != 2)
	{
		printf("The Copy($from_indx,$char_count):[simple string] function needs both the start index and the character count to copy ( $char_count = -1 for the end of the string).\n");
		return true;
	}
	
	PHDR_VAR param1 = hdr_inter_resolve_extract_var_from_expr(inter, token->parameters, 0) ;	
	if (param1 == NULL)
	{
		printf("Copy($from_indx,$char_count):[simple string] The first parameter  resolved as NULL.\n");
		return true ;
	}

	PHDR_VAR param2 = hdr_inter_resolve_extract_var_from_expr(inter, token->parameters, 1) ;	
	if (param2 == NULL)
	{
		printf("Copy($from_indx,$char_count):[simple string] The second parameter  resolved as NULL.\n");
		return true ;
	}


	if((param1->type != hvt_integer)&&(param1->type != hvt_float))
	{
	    printf("Copy($from_indx,$char_count):[simple string] The first parameter is not of a Numeric type.\n");
		goto fail ;
	}

	if((param2->type != hvt_integer)&&(param2->type != hvt_float))
	{
	   printf("Copy($from_indx,$char_count):[simple string] The second parameter is not of a Numeric type.\n");
		goto fail ;
	}

	switch(for_var->type)
	{
		case hvt_simple_string :
		case hvt_simple_string_bcktck:
		{

		 DXLONG64 from_indx = 0 ;
		 DXLONG64 char_len  = 0 ;
		 if(param1->type == hvt_integer) from_indx = param1->integer ;
		 else
			 from_indx = param1->real ;
		 
		 if(param2->type == hvt_integer) char_len = param2->integer ;
		 else
			 char_len = param2->real ;


		 char *buf = utf8CopyIndex(((PDX_STRING)for_var->obj)->stringa,from_indx,char_len) ; 
		 
		 PDX_STRING str = dx_string_create_bU(buf) ;

		 *result = hdr_var_create(NULL,"",hvf_temporary_ref,NULL) ;
		 hdr_var_set_obj(*result,str)		  ;
		 (*result)->type   = hvt_simple_string ;

		}break;
		case hvt_unicode_string:
		{	
			printf("The 'unicode' string type does not support copying. \n");
			*result = NULL ;
			goto fail ;
			 
		}break;
		case hvt_complex_string:
		{
		    printf("The 'concat' string type does not support copying. \n");
			*result = NULL ;
			goto fail ;
		}break;
		case hvt_complex_string_resolve:
		{
			printf("The 'var_expand' string type does not support word copying. \n");
			*result = NULL ;
			goto fail ;
		}break;
	
	}

	hdr_inter_param_free(param1);
	hdr_inter_param_free(param2);
	

	return false ;

	fail :

	hdr_inter_param_free(param1);
	hdr_inter_param_free(param2);

	return true ;

}

bool hdr_domStringCopyChar(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token, PHDR_VAR for_var,PHDR_VAR *result)
{
	/*
	 copy the string from an index to an index. If the end index is bigger than the string length then
	 the string until the end is returned
	*/

	if (token->parameters->count != 2)
	{
		printf("The CopyChar($from_indx,$char):[simple string] function needs both the start index and the terminal character.\n");
		return true;
	}
	
	PHDR_VAR param1 = hdr_inter_resolve_extract_var_from_expr(inter, token->parameters, 0) ;	
	if (param1 == NULL)
	{
		printf("CopyChar($from_indx,$char):[simple string] The first parameter  resolved as NULL.\n");
		return true ;
	}

	PHDR_VAR param2 = hdr_inter_resolve_extract_var_from_expr(inter, token->parameters, 1) ;	
	if (param2 == NULL)
	{
		printf("CopyChar($from_indx,$char):[simple string] The second parameter  resolved as NULL.\n");
		return true ;
	}


	if((param1->type != hvt_integer)&&(param1->type != hvt_float))
	{
	    printf("CopyChar($from_indx,$char):[simple string] The first parameter is not of a Numeric type.\n");
		goto fail ;
	}

	if((param2->type != hvt_simple_string)&&(param2->type != hvt_simple_string_bcktck)&&(param2->type != hvt_codepoint))
	{
	   printf("CopyChar($from_indx,$char):[simple string] The second parameter is not of a string or character type.\n");
		goto fail ;
	}

	if(((param2->type != hvt_simple_string)&&(param2->type != hvt_simple_string_bcktck))&&(((PDX_STRING)param2->obj)->len != 1))
	{
	   printf("CopyChar($from_indx,$char):[simple string] The second parameter is not a single character.\n");
	   goto fail ;
	}

	char th_char[5] ;
	if(param2->type == hvt_codepoint) dxConvertUint32ToUTF8(th_char,param2->integer) ;
	else
		dx_strcpy(th_char,((PDX_STRING)param2->obj)->stringa) ;
	
	switch(for_var->type)
	{
		case hvt_simple_string :
		case hvt_simple_string_bcktck:
		{

		 DXLONG64 from_indx = 0 ;
		 if(param1->type == hvt_integer) from_indx = param1->integer ;
		 else
			 from_indx = param1->real ;

		 char *buff = utf8CopyToCh(((PDX_STRING)for_var->obj)->stringa, &from_indx , th_char)  ;
		 PDX_STRING str = dx_string_create_bU(buff);

		 if(param1->type == hvt_integer) param1->integer = from_indx ;
		 else
			 param1->real = from_indx ;

		 *result = hdr_var_create(NULL,"",hvf_temporary_ref,NULL) ;
		 hdr_var_set_obj(*result,str)		  ;
		 (*result)->type   = hvt_simple_string ;

		}break;
		case hvt_unicode_string:
		{	
			printf("The 'unicode' string type does not support copying. \n");
			*result = NULL ;
			goto fail ;

		}break;
		case hvt_complex_string:
		{
		    printf("The 'concat' string type does not support copying. \n");
			*result = NULL ;
			goto fail ;
		}break;
		case hvt_complex_string_resolve:
		{
			printf("The 'var_expand' string type does not support word copying. \n");
			*result = NULL ;
			goto fail ;
		}break;
	
	}

	hdr_inter_param_free(param1);
	hdr_inter_param_free(param2);
	

	return false ;

	fail :

	hdr_inter_param_free(param1);
	hdr_inter_param_free(param2);

	return true ;

}

bool hdr_domStringRevCpyBt(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token, PHDR_VAR for_var,PHDR_VAR *result)
{
    /*
	  Copy the string from the indx until the ascii character searching in reverse. If the character does not exists then
	  the function returns all the string
	*/

   PHDR_SYS_FUNC_PARAMS params = hdr_sys_func_init_params(inter,token->parameters,2) ;
   if(params == NULL)
   {
	 printf("The system function String.ReverseCopyCh($indx : Integer , $char : Integer (as char) ):Simple String failed.\n");
     return true ;
   }

    bool type_error = false ;
    DXLONG64 indx  = hdr_inter_ret_integer(params->params[0],&type_error) ;
	if(type_error == true)
    {
	  printf("The first parameter must be an integer.\n");
      goto fail ;
    }

    type_error = false ;
    DXLONG64 chr  = hdr_inter_ret_integer(params->params[1],&type_error) ;
	if(type_error == true)
    {
	  printf("The second parameter must be an integer tha represents an ASCII char (0~255).\n");
      goto fail ;
    }
   
	PDX_STRING src  = (PDX_STRING)for_var->obj ;
	char *str_indx  = src->stringa ;
	char *nbuff = dxCopyStrToCharReverse(&str_indx, (char)chr,"") ;
	PDX_STRING nstr = dx_string_create_bU(nbuff) ;
	*result = hdr_var_create(NULL,"",hvf_temporary_ref,NULL) ;
	(*result)->type    = hvt_simple_string                   ;
	hdr_var_set_obj(*result,nstr)      ;

    success:
    hdr_sys_func_free_params(params) ;
    return false ;

    fail : 
    printf("The system function String.ReverseCopyCh($indx : Integer, $char : Integer (as char) ):Simple String failed.\n");
    hdr_sys_func_free_params(params) ;
    return true ;
}


bool hdr_domStringCharPos(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token,PHDR_VAR for_var,PHDR_VAR *result)
{
	/*
	 The function finds the character that is passed as a parameter and returns the position of the character in
	 the string. If the character does not exists then the return value is -1
	*/

   PHDR_SYS_FUNC_PARAMS params = hdr_sys_func_init_params(inter,token->parameters,2) ;
   if(params == NULL)
   {
	 printf("The system function String.CharPos($char : Char,$from_indx:Integer):Integer failed.\n");
     return true ;
   }

    PHDR_VAR chr = params->params[0] ; 

   	if((chr->type != hvt_simple_string)&&(chr->type != hvt_simple_string_bcktck)&&(chr->type != hvt_codepoint))
	{
	   printf("String.CharPos($char : Char,$from_indx:Integer):Integer The second parameter is not of a string or character type.\n");
		goto fail ;
	}

	if(((chr->type != hvt_simple_string)&&(chr->type != hvt_simple_string_bcktck))&&(((PDX_STRING)chr->obj)->len != 1))
	{
	   printf("String.CharPos($char : Char,$from_indx:Integer):Integer The second parameter is not a single character.\n");
	   goto fail ;
	}

	/*convert  a utf8 char to a codepoint or copy the char if it is already a codepoint*/
	DXUTF8CHAR utf8char = 0 ;
	if(chr->type == hvt_codepoint) utf8char = chr->integer ;
	else
     utf8char = dxConvertUTF8ToInt32(((PDX_STRING)chr->obj)->stringa,((PDX_STRING)chr->obj)->bcount) ;

    bool type_error = false ;
   
    DXLONG64 indx  = hdr_inter_ret_integer(params->params[1],&type_error) ;
	if(type_error == true)
    {
	  printf("The second parameter must be an integer .\n");
      goto fail ;
    }
	
	*result = hdr_var_create(NULL,"",hvf_temporary_ref,NULL) ;
	(*result)->integer = -1 ;
	(*result)->type    = hvt_integer ;

	if(indx < 0 ) goto success ;
	/*the result will be indx + returned value as the returned value is in relation with the index see dxUtf8CharPos*/
	(*result)->integer = hdr_Utf8CharPos((PDX_STRING)for_var->obj,utf8char,indx) ;

    success:
    hdr_sys_func_free_params(params) ;
    return false ;

    fail : 
    printf("The system function String.CharPos($char : Char,$from_indx:Integer):Integer failed.\n");
    hdr_sys_func_free_params(params) ;
    return true ;
}

bool hdr_domStringCopyUntilChar(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token,PHDR_VAR for_var,PHDR_VAR *result)
{
	/*
	 The function starts from the $from_index and copy the string until the $char. If the $char does not exists 
	 then all the string from $from_indx until the end is copied and the $from_indx will set to -1.
	 When the function return the $from_indx will have the position of the 
	 next character after the $char OR if the character is the last of the string , the $from_indx will
	 have the position of the $char
	*/

   PHDR_SYS_FUNC_PARAMS params = hdr_sys_func_init_params(inter,token->parameters,2) ;
   if(params == NULL)
   {
	 printf("The system function String.CopyUntilChar($char : Char,$from_indx:Integer):String failed.\n");
     return true ;
   }

    PHDR_VAR chr = params->params[0] ; 

   	if((chr->type != hvt_simple_string)&&(chr->type != hvt_simple_string_bcktck)&&(chr->type != hvt_codepoint))
	{
	   printf("String.CopyUntilChar($char : Char,$from_indx:Integer):String The first parameter is not of a string or character type.\n");
		goto fail ;
	}

	if(((chr->type != hvt_simple_string)&&(chr->type != hvt_simple_string_bcktck))&&(((PDX_STRING)chr->obj)->len != 1))
	{
	   printf("String.CopyUntilChar($char : Char,$from_indx:Integer):String The first parameter is not a single character.\n");
	   goto fail ;
	}

	/*convert  a utf8 char to a codepoint or copy the char if it is already a codepoint*/
	DXUTF8CHAR utf8char = 0 ;
	if(chr->type == hvt_codepoint) utf8char = chr->integer ;
	else
     utf8char = dxConvertUTF8ToInt32(((PDX_STRING)chr->obj)->stringa,((PDX_STRING)chr->obj)->bcount) ;

    bool type_error = false ;
   
    DXLONG64 indx  = hdr_inter_ret_integer(params->params[1],&type_error) ;
	if(type_error == true)
    {
	  printf("The second parameter must be an integer .\n");
      goto fail ;
    }
	
	*result = hdr_var_create(NULL,"",hvf_temporary_ref,NULL) ;
	(*result)->type    = hvt_simple_string ;
	hdr_var_set_obj(*result,dx_string_createU(NULL,"")) ;

	if(indx < 0 ) goto success ;
	/*the result will be indx + returned value as the returned value is in relation with the index see dxUtf8CharPos*/
	dx_string_free((PDX_STRING)(*result)->obj) ;

	hdr_var_set_obj(*result,hdr_Utf8CopyUntilChar((PDX_STRING)for_var->obj,utf8char,&indx)) ;
	
	/*set the returned value*/
	if(params->params[1]->type == hvt_integer) params->params[1]->integer = indx ;
	else
		params->params[1]->real = indx ;

    success:
    hdr_sys_func_free_params(params) ;
    return false ;

    fail : 
    printf("The system function String.CopyUntilChar($char : Char,$from_indx:Integer):String failed.\n");
    hdr_sys_func_free_params(params) ;
    return true ;
}

bool hdr_domStringExplode(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token,PHDR_VAR for_var,PHDR_VAR *result)
{
	/*
	  The function "explodes" a string and store it in a list.
	  The function needs a separator to use that can be multibyte
	*/

   PHDR_SYS_FUNC_PARAMS params = hdr_sys_func_init_params(inter,token->parameters,1) ;
   if(params == NULL)
   {
	 printf("The system function String.Explode($separator : String):StringList failed.\n");
     return true ;
   }

    bool type_error = false ;
    PDX_STRING str  = hdr_inter_ret_string(params->params[0],&type_error) ;
	if(type_error == true)
    {
	  printf("The first parameter must be a String.\n");
      goto fail ;
    }
   
	*result = hdr_var_create(NULL,"",hvf_temporary_ref,NULL) ;
	(*result)->type    = hvt_string_list ;
	hdr_var_set_obj(*result,dx_stringlist_create()) ;
	
	ExplodeEx((PDX_STRINGLIST)(*result)->obj,(PDX_STRING)for_var->obj,str);


    success:
    hdr_sys_func_free_params(params) ;
    return false ;

    fail : 
    printf("The system function String.Explode($separator : String):StringList failed.\n");
    hdr_sys_func_free_params(params) ;
    return true ;
}


bool hdr_domStringExplodeChar(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token,PHDR_VAR for_var,PHDR_VAR *result)
{
	/*
	  The function "explodes" a string and store it in a list.
	  The function needs a separator to use that can be a single byte
	*/

   PHDR_SYS_FUNC_PARAMS params = hdr_sys_func_init_params(inter,token->parameters,1) ;
   if(params == NULL)
   {
	 printf("The system function String.ExplodeChar($separator : Integer (as char) ):StringList failed.\n");
     return true ;
   }

    bool type_error = false ;
    DXLONG64 chr  = hdr_inter_ret_integer(params->params[0],&type_error) ;
	if(type_error == true)
    {
	  printf("The first parameter must be an integer.\n");
      goto fail ;
    }
   
	*result = hdr_var_create(NULL,"",hvf_temporary_ref,NULL) ;
	(*result)->type    = hvt_string_list ;
	hdr_var_set_obj(*result,dx_stringlist_create()) ;
	
	ExplodeChar((PDX_STRINGLIST)(*result)->obj,(PDX_STRING)for_var->obj,(char)chr);

    success:
    hdr_sys_func_free_params(params) ;
    return false ;

    fail : 
    printf("The system function String.ExplodeEx($separator : Integer (as char)):StringList failed.\n");
    hdr_sys_func_free_params(params) ;
    return true ;
}



bool hdr_domStringTrimLeft(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token, PHDR_VAR for_var,PHDR_VAR *result)
{
	/*
	 remove the characters from left of the string 
	*/

	if (token->parameters->count != 1)
	{
		printf("The TrimLeft($chars) function needs a string with the characters to remove.\n");
		return true;
	}
	
	PHDR_VAR param = hdr_inter_resolve_extract_var_from_expr(inter, token->parameters, 0) ;	
	if (param == NULL)
	{
		printf("TrimLeft($chars) : The parameter  resolved as NULL.\n");
		return true ;
	}

	if((param->type != hvt_simple_string)&&(param->type != hvt_simple_string_bcktck))
	{
	    printf("TrimLeft($chars) : The parameter is not of a simple string type.\n");
		goto fail ;
	}

	if(((PDX_STRING)param->obj)->len == 0)
	{
	    printf("TrimLeft($chars) : The parameter is an empty string.\n");
		goto fail ;
	}

	switch(for_var->type)
	{
		case hvt_simple_string :
		case hvt_simple_string_bcktck:
		{
		  dx_utf8_trim_left((PDX_STRING)for_var->obj,(((PDX_STRING)param->obj)->stringa)) ;
		  *result = NULL ;
		}break;
		case hvt_unicode_string:
		{	
			printf("The 'unicode' string type does not support triming. \n");
			goto fail ;

		}break;
		case hvt_complex_string:
		{
			printf("The 'concat' string type does not support triming. \n");
			goto fail ;
		}break;
		case hvt_complex_string_resolve:
		{
			printf("The 'var_expand' string type does not support triming. \n");
			goto fail ;
		}break;
	
	}

	if (param->var_ref == hvf_temporary_ref)
	{
		if(hdr_inter_var_gen_type(param) == hdr_ivt_string)
		dx_string_free(((PDX_STRING)param->obj));
		hdr_var_release_obj(param) ;
		hdr_var_free(param);
	}

	return false ;

	fail :
	if (param->var_ref == hvf_temporary_ref)
	{
		if(hdr_inter_var_gen_type(param) == hdr_ivt_string)
		dx_string_free(((PDX_STRING)param->obj));
		hdr_var_release_obj(param) ;
		hdr_var_free(param);
	}
	return true ;

}

bool hdr_domStringTrimRight(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token, PHDR_VAR for_var,PHDR_VAR *result)
{
	/*
	 remove the characters from the right of the string 
	*/

	if (token->parameters->count != 1)
	{
		printf("The TrimRight($chars) function needs a string with the characters to remove.\n");
		return true;
	}
	
	PHDR_VAR param = hdr_inter_resolve_extract_var_from_expr(inter, token->parameters, 0) ;	
	if (param == NULL)
	{
		printf("TrimRight($chars) : The parameter  resolved as NULL.\n");
		return true ;
	}

	if((param->type != hvt_simple_string)&&(param->type != hvt_simple_string_bcktck))
	{
	    printf("TrimRight($chars) : The parameter is not of a simple string type.\n");
		goto fail ;
	}

	if(((PDX_STRING)param->obj)->len == 0)
	{
	    printf("TrimRight($chars) : The parameter is an empty string.\n");
		goto fail ;
	}

	switch(for_var->type)
	{
		case hvt_simple_string :
		case hvt_simple_string_bcktck:
		{
    
		  dx_utf8_trim_right((PDX_STRING)for_var->obj,(((PDX_STRING)param->obj)->stringa)) ;
		  *result = NULL ;
		}break;
		case hvt_unicode_string:
		{	
			printf("The 'unicode' string type does not support triming. \n");
			*result = NULL ;
			goto fail ;

		}break;
		case hvt_complex_string:
		{
			printf("The 'concat' string type does not support triming. \n");
			*result = NULL ;
			goto fail ;
		}break;
		case hvt_complex_string_resolve:
		{
			printf("The 'var_expand' string type does not support triming. \n");
			*result = NULL ;
			goto fail ;
		}break;
	
	}

	if (param->var_ref == hvf_temporary_ref)
	{
		if(hdr_inter_var_gen_type(param) == hdr_ivt_string)
		dx_string_free(((PDX_STRING)param->obj));
		hdr_var_release_obj(param) ;
		hdr_var_free(param);
	}

	return false ;

	fail :
	if (param->var_ref == hvf_temporary_ref)
	{
		if(hdr_inter_var_gen_type(param) == hdr_ivt_string)
		dx_string_free(((PDX_STRING)param->obj));
		hdr_var_release_obj(param) ;
		hdr_var_free(param);
	}
	return true ;

}

bool hdr_domStringTrim(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token, PHDR_VAR for_var)
{
	/*
	 remove the characters from left and right of the string 
	*/
	if (token->parameters->count != 1)
	{
		printf("The Trim($chars) function needs a string with the characters to remove.\n");
		return true;
	}
	
	PHDR_VAR param = hdr_inter_resolve_extract_var_from_expr(inter, token->parameters, 0) ;	
	if (param == NULL)
	{
		printf("Trim($chars) : The parameter  resolved as NULL.\n");
		return true ;
	}

	if((param->type != hvt_simple_string)&&(param->type != hvt_simple_string_bcktck))
	{
	    printf("Trim($chars) : The parameter is not of a simple string type.\n");
		goto fail ;
	}

	if(((PDX_STRING)param->obj)->len == 0)
	{
	    printf("Trim($chars) : The parameter is an empty string.\n");
		goto fail ;
	}

	switch(for_var->type)
	{
		case hvt_simple_string :
		case hvt_simple_string_bcktck:
		{
		  dx_utf8_trim((PDX_STRING)for_var->obj,(((PDX_STRING)param->obj)->stringa)) ;
		}break;
		case hvt_unicode_string:
		{	
			printf("The 'unicode' string type does not support triming. \n");
			goto fail ;

		}break;
		case hvt_complex_string:
		{
			printf("The 'concat' string type does not support triming. \n");
			goto fail ;
		}break;
		case hvt_complex_string_resolve:
		{
			printf("The 'var_expand' string type does not support triming. \n");
			goto fail ;
		}break;
	
	}

	if (param->var_ref == hvf_temporary_ref)
	{
		if(hdr_inter_var_gen_type(param) == hdr_ivt_string)
		dx_string_free(((PDX_STRING)param->obj));
		hdr_var_release_obj(param) ;
		hdr_var_free(param);
	}

	return false ;

	fail :
	if (param->var_ref == hvf_temporary_ref)
	{
		if(hdr_inter_var_gen_type(param) == hdr_ivt_string)
		dx_string_free(((PDX_STRING)param->obj));
		hdr_var_release_obj(param) ;
		hdr_var_free(param);
	}
	return true ;

}


bool hdr_domStringInt(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token, PHDR_VAR for_var,PHDR_VAR *result)
{

   PHDR_SYS_FUNC_PARAMS params = hdr_sys_func_init_params(inter,token->parameters,1) ;
   if(params == NULL)
   {
    printf("The system function String.ToInteger($error : [Simple String]):Integer failed.\n");
    return true ;
   }
   
   bool type_error = false ;
   PDX_STRING strerror = hdr_inter_ret_string(params->params[0],&type_error) ; 
   if(type_error == true)
   {
	 printf("The parameter must be a string.\n");
     hdr_sys_func_free_params(params) ;
	 return true ;
   }

   strerror = dx_string_createU(strerror,"") ;

	switch(for_var->type)
	{
		case hvt_simple_string :
		case hvt_simple_string_bcktck:
		{	
			DXLONG64 intval = 0 ;

			bool error ;


			intval = dx_StringToInt((PDX_STRING)for_var->obj,&error) ;
			if(error == true) 
			{
				strerror = dx_string_createU(strerror,"The conversion of the string into integer failed.");
				return false ;
			}
			*result = hdr_var_create(NULL, "", hvf_temporary, NULL);
			(*result)->type = hvt_integer;
			(*result)->integer = intval;
		}break;
		case hvt_unicode_string:
		{
			printf("The 'unicode' string does not support the conversion to integer. \n");
			hdr_sys_func_free_params(params) ;
			return true ;
		}break ;
		case hvt_complex_string:
		{
			printf("The 'concat' string does not support the conversion to integer. \n");
			hdr_sys_func_free_params(params) ;
			return true ;
		}break;
		case hvt_complex_string_resolve:
		{
			printf("The 'var_expand' string does not support the conversion to integer. \n");
			hdr_sys_func_free_params(params) ;
			return true ;
		}break;
	
	}

	hdr_sys_func_free_params(params) ;
	return false ;
}


bool hdr_domStringReal(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token, PHDR_VAR for_var,PHDR_VAR *result)
{
   PHDR_SYS_FUNC_PARAMS params = hdr_sys_func_init_params(inter,token->parameters,1) ;
   if(params == NULL)
   {
    printf("The system function String.ToReal($error : [Simple String]):Real failed.\n");
    return true ;
   }
   
   bool type_error = false ;
   PDX_STRING error = hdr_inter_ret_string(params->params[0],&type_error) ; 
   if(type_error == true)
   {
	 printf("The parameter must be a string.\n");
     hdr_sys_func_free_params(params) ;
	 return true ;
   }

    error = dx_string_createU(error,"") ;

	switch(for_var->type)
	{
		case hvt_simple_string :
		case hvt_simple_string_bcktck:
		{	
			bool terror ;
			double dval = dx_StringToReal((PDX_STRING)for_var->obj,&terror) ;
			if(terror == true) 
			{
				error = dx_string_createU(NULL,"The conversion of the string into real failed.");
				hdr_sys_func_free_params(params) ;
				return false ;
			}
			*result = hdr_var_create(NULL, "", hvf_temporary, NULL);
			(*result)->type = hvt_float;
			(*result)->real = dval;
		}break;
		case hvt_unicode_string:
		{
			printf("The 'unicode' string does not support the conversion to real. \n");
			hdr_sys_func_free_params(params) ;
			return true ;
		}break ;
		case hvt_complex_string:
		{
			printf("The 'concat' string does not support the conversion to real. \n");
			hdr_sys_func_free_params(params) ;
			return true ;
		}break;
		case hvt_complex_string_resolve:
		{
			printf("The 'var_expand' string does not support the conversion to real. \n");
			hdr_sys_func_free_params(params) ;
			return true ;
		}break;
	
	}

	hdr_sys_func_free_params(params) ;
	return false ;
}

bool hdr_domStringHexInt(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token, PHDR_VAR for_var,PHDR_VAR *result)
{
   PHDR_SYS_FUNC_PARAMS params = hdr_sys_func_init_params(inter,token->parameters,1) ;
   if(params == NULL)
   {
    printf("The system function String.FromHexToInt($error : [Simple String]):Integer failed.\n");
    return true ;
   }
   
   bool type_error = false ;
   PDX_STRING error = hdr_inter_ret_string(params->params[0],&type_error) ; 
   if(type_error == true)
   {
	 printf("The parameter must be a string.\n");
     hdr_sys_func_free_params(params) ;
	 return true ;
   }

    error = dx_string_createU(error,"") ;

	switch(for_var->type)
	{
		case hvt_simple_string :
		case hvt_simple_string_bcktck:
		{	
			PDX_STRING str = (PDX_STRING)for_var->obj ;
			DXLONG64 dval = dxHexToDec(str->stringa) ;
			if(dval == -1) 
			{
				error = dx_string_createU(NULL,"The conversion of the hexadecimal string into an Integer failed.");
				hdr_sys_func_free_params(params) ;
				return false ;
			}
			*result = hdr_var_create(NULL, "", hvf_temporary, NULL);
			(*result)->type = hvt_integer;
			(*result)->integer = dval;
		}break;
		case hvt_unicode_string:
		{
			printf("The 'unicode' string does not support the conversion from hexadecimal to integer. \n");
			hdr_sys_func_free_params(params) ;
			return true ;
		}break ;
		case hvt_complex_string:
		{
			printf("The 'concat' string does not support the conversion from hexadecimal to integer. \n");
			hdr_sys_func_free_params(params) ;
			return true ;
		}break;
		case hvt_complex_string_resolve:
		{
			printf("The 'var_expand' string does not support the conversion from hexadecimal to integer. \n");
			hdr_sys_func_free_params(params) ;
			return true ;
		}break;
	
	}

	hdr_sys_func_free_params(params) ;
	return false ;
}

bool hdr_domStringAddFromFile(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token, PHDR_VAR for_var)
{

   PHDR_SYS_FUNC_PARAMS params = hdr_sys_func_init_params(inter,token->parameters,2) ;
   if(params == NULL)
   {
    printf("The system function String.AddFromFile($filename:String, out-> $error ) failed.\n");
    return true ;
   }

   bool type_error = false ;
   PDX_STRING fname = hdr_inter_ret_string(params->params[0],&type_error) ; 
   if(type_error == true)
   {
	 printf("The first parameter must be a String variable.\n");
     goto fail ;
   }

   PDX_STRING error = hdr_inter_ret_string(params->params[1],&type_error) ; 
   if(type_error == true)
   {
	 printf("The second parameter must be a String variable.\n");
     goto fail ;
   }

   error = dx_string_createU(error,"");

	switch(for_var->type)
	{
		case hvt_simple_string :
		case hvt_simple_string_bcktck:
		{	
			bool berror = false ;
			PDX_STRING str = dx_LoadUTF8TextFromFile(fname,&berror) ;
			if (berror == true)
			{
			 error = dx_string_createU(error,"Error in accessing the file.\n");
			 dx_string_free(str);
			 goto success ;
			}

			/*add the string*/
			PDX_STRING base_str = (PDX_STRING)for_var->obj ;
			PDX_STRING nstr     = dx_string_concat(base_str,str) ;
			dx_string_free(str) ;
			/*do some alchemy to avoid some memory allocations/deallocations*/
			base_str = dx_string_setU(base_str,nstr->stringa) ;
			nstr->stringa = NULL ; 
			dx_string_free(nstr);

		}break;
		case hvt_unicode_string:
		{
			printf("The 'unicode' string does not support YET load from disk. \n");
			return true ;
		}break ;
		case hvt_complex_string:
		{
			bool berror = false ;
			PDX_STRING str = dx_LoadUTF8TextFromFile(fname,&berror) ;
			if (berror == true)
			{
			 error = dx_string_createU(error,"Error in accessing the file.\n");
			 dx_string_free(str);
			 goto success ;
			}

			/*add the string*/
			PDX_STRINGLIST base_str = (PDX_STRINGLIST)for_var->obj ;
			dx_stringlist_add_string(base_str,str) ;
		}break;
		case hvt_complex_string_resolve:
		{
			bool berror = false ;
			PDX_STRING str = dx_LoadUTF8TextFromFile(fname,&berror) ;
			if (berror == true)
			{
			 error = dx_string_createU(error,"Error in accessing the file.\n");
			 dx_string_free(str);
			 goto success ;
			}

			/*add the string*/
			PHDR_COMPLEX_STR_RES base_str = (PHDR_COMPLEX_STR_RES)for_var->obj ;
			hdr_res_str_add_str(base_str,str) ;
			dx_string_free(str);
	
		}break;
	
	}

    success:
    hdr_sys_func_free_params(params) ;
    return false ;

    fail : 
    printf("The system function String.AddFromFile($filename:String, out-> $error ) failed.\n");
    hdr_sys_func_free_params(params) ;
    return true ;
}


bool hdr_domStringSetFromChars(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token,PHDR_VAR for_var)
{
	/*
	 Hydra+ does not provides an easy way to add characters into a string like \n\t etc, 
	 this function provides an easy way to create a string from simple ANSI characters
	 for example , to create an /r/n sequence the parameter must be "#13#10"
	*/

   PHDR_SYS_FUNC_PARAMS params = hdr_sys_func_init_params(inter,token->parameters,1) ;
   if(params == NULL)
   {
	 printf("The system function String.SetFromChars(Chars : String[#13#10...]) failed.\n");
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

	/*create the new string in place of the old*/

	PDX_STRING nst = dx_string_createU((PDX_STRING)for_var->obj,chars) ;
	hdr_var_set_obj(for_var,nst) ;
    success:
    hdr_sys_func_free_params(params) ;
    return false ;

    fail : 
    printf("The system function String.SetFromChars(Chars : String[#13#10...]) failed.\n");
    hdr_sys_func_free_params(params) ;
    return true ;
}



bool hdr_domStringLoadFromFile(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token, PHDR_VAR *result)
{

   PHDR_SYS_FUNC_PARAMS params = hdr_sys_func_init_params(inter,token->parameters,2) ;
   if(params == NULL)
   {
    printf("The system function loadFromFile($filename:String, out-> $error ) failed.\n");
    return true ;
   }

   bool type_error = false ;
   PDX_STRING fname = hdr_inter_ret_string(params->params[0],&type_error) ; 
   if(type_error == true)
   {
	 printf("The first parameter must be a String variable.\n");
     goto fail ;
   }

   PDX_STRING error = hdr_inter_ret_string(params->params[1],&type_error) ; 
   if(type_error == true)
   {
	 printf("The second parameter must be a String variable.\n");
     goto fail ;
   }

   error = dx_string_createU(error,"");

	
   bool berror = false ;
   PDX_STRING str = dx_LoadUTF8TextFromFile(fname,&berror) ;
	if (berror == true)
	{
		error = dx_string_createU(error,"Error in accessing the file.\n");
		dx_string_free(str);
		goto success ;
	}

   *result = hdr_var_create(str, "", hvf_temporary_ref, NULL) ;
   (*result)->type    = hvt_simple_string ; 
		

    success:
    hdr_sys_func_free_params(params) ;
    return false ;

    fail : 
    printf("The system function loadFromFile($filename:String, out-> $error ) failed.\n");
    hdr_sys_func_free_params(params) ;
    return true ;
}


bool hdr_domStringSaveToFile(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token, PHDR_VAR for_var)
{

   PHDR_SYS_FUNC_PARAMS params = hdr_sys_func_init_params(inter,token->parameters,2) ;
   if(params == NULL)
   {
    printf("The system function SaveToFile($filename:String, out-> $error ) failed.\n");
    return true ;
   }

   bool type_error = false ;
   PDX_STRING fname = hdr_inter_ret_string(params->params[0],&type_error) ; 
   if(type_error == true)
   {
	 printf("The first parameter must be a String variable.\n");
     goto fail ;
   }

   PDX_STRING error = hdr_inter_ret_string(params->params[1],&type_error) ; 
   if(type_error == true)
   {
	 printf("The second parameter must be a String variable.\n");
     goto fail ;
   }

   error = dx_string_createU(error,"");

	
   bool berror = false ;
   FILE *f = fopen(fname->stringa,"wb") ;
   if(f == NULL)
	{
		error = dx_string_createU(error,"Error in accessing the file.\n");
		goto success ;
	}

    if((for_var->type == hvt_simple_string)||(for_var->type == hvt_simple_string_bcktck))
	{
		PDX_STRING this_str = (PDX_STRING)for_var->obj ;
		DXLONG64 ret = fwrite(this_str->stringa,this_str->bcount,1,f) ;
		if(ret != 1) error = dx_string_createU(error,"Error in writing in the file.\n");
		fclose(f);
	}
	else
		if(for_var->type == hvt_unicode_string)
		{
		 PDX_STRING this_str = (PDX_STRING)for_var->obj ;
		 DXLONG64 ret = fwrite(this_str->stringx,this_str->bcount,1,f) ;
		 if(ret != 1) error = dx_string_createU(error,"Error in writing in the file.\n");
		 fclose(f);
		}else
		if(for_var->type == hvt_complex_string)
		{

		   PDX_STRINGLIST this_str = (PDX_STRINGLIST)for_var->obj        ;
		   PDXL_NODE node =this_str->start ; 
		   while(node != NULL)
		   {
		     PDX_STRING str = (PDX_STRING)node->object->key ;
		     DXLONG64   ret = fwrite(str->stringa,str->bcount,1,f) ;
			 if(ret != 1) {error = dx_string_createU(error,"Error in writing in the file.\n");break;};
			 node = node->right ;
		   }

		   fclose(f);
		}else
			if(for_var->type == hvt_complex_string_resolve)
			{

			   PHDR_COMPLEX_STR_RES this_str = (PHDR_COMPLEX_STR_RES)for_var->obj ;
			   PDX_STRINGLIST		strl = this_str->text_lines	;
			   PDXL_NODE node = strl->start ; 
			   while(node != NULL)
			   {
				 PDX_STRING str = (PDX_STRING)node->object->key  ;
				 if(str->bcount >0)
				 {
					 DXLONG64   ret = fwrite(str->stringa,str->bcount,1,f) ;
					 if(ret != 1) {error = dx_string_createU(error,"Error in writing in the file.\n");break;};
				 }
				 node = node->right ;
			   }

			   fclose(f);
			}

	success:
    hdr_sys_func_free_params(params) ;
    return false ;

    fail : 
    printf("The system function SaveToFile($filename:String, out-> $error ) failed.\n");
    hdr_sys_func_free_params(params) ;
    return true ;
}

bool hdr_domStringUrlEncode(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token,PHDR_VAR for_var,PHDR_VAR *result)
{
	/*
	 The function returns the string as a urlencoded string
	*/

   PHDR_SYS_FUNC_PARAMS params = hdr_sys_func_init_params(inter,token->parameters,1) ;
   if(params == NULL)
   {
	 printf("The system function String.UrlEncode($space_as_plus:Boolean):String failed.\n");
     return true ;
   }

    bool type_error = false ;
    bool as_plus = hdr_inter_ret_bool(params->params[0],&type_error) ;
    if(type_error == true)
    {
	  printf("The parameter must be a Boolean.\n");
      goto fail ;
    }


    PDX_STRING str  = (PDX_STRING)for_var->obj ;
	
	PDX_STRING nstr = dxUrlEncodeUTF8(str,as_plus) ;
	if(nstr == NULL) 
	{
		nstr = dx_string_createU(NULL,"") ;
		if(inter->warnings == true)
		hdr_inter_print_warning(inter,"The conversion of the original string to a url encoded string failed. Empty string returned") ;
	}

	*result   = hdr_var_create((void*)nstr,"",hvf_temporary_ref,NULL) ;
	(*result)->type = hvt_simple_string ;

    success:
    hdr_sys_func_free_params(params) ;
    return false ;

    fail : 
    printf("The system function String.UrlEncode($space_as_plus:Boolean):String failed.\n");
    hdr_sys_func_free_params(params) ;
    return true ;
}


bool hdr_domStringUrlDecode(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token,PHDR_VAR for_var,PHDR_VAR *result)
{
	/*
	 The function returns the string as a urlencoded string
	*/

   PHDR_SYS_FUNC_PARAMS params = hdr_sys_func_init_params(inter,token->parameters,1) ;
   if(params == NULL)
   {
	 printf("The system function String.UrlDecode($plus_as_space:Boolean):String failed.\n");
     return true ;
   }

    bool type_error = false ;
    bool plus_as_space = hdr_inter_ret_bool(params->params[0],&type_error) ;
    if(type_error == true)
    {
	  printf("The parameter must be a Boolean.\n");
      goto fail ;
    }


    PDX_STRING str  = (PDX_STRING)for_var->obj ;
	
	PDX_STRING nstr = dxUrlDecodeUTF8(str,plus_as_space) ;
	if(nstr == NULL) 
	{
		nstr = dx_string_createU(NULL,"") ;
		if(inter->warnings == true)
		hdr_inter_print_warning(inter,"The conversion of the url encoded string to a decoded string failed. Empty string returned") ;
	}

	*result   = hdr_var_create((void*)nstr,"",hvf_temporary_ref,NULL) ;
	(*result)->type = hvt_simple_string ;

    success:
    hdr_sys_func_free_params(params) ;
    return false ;

    fail : 
    printf("The system function String.UrlDecode($plus_as_space:Boolean):String failed.\n");
    hdr_sys_func_free_params(params) ;
    return true ;
}


bool hdr_domStringBase64Encode(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token,PHDR_VAR for_var,PHDR_VAR *result)
{
	/*
	 The function returns the string as a base64 encoded string
	*/

   PHDR_SYS_FUNC_PARAMS params = hdr_sys_func_init_params(inter,token->parameters,0) ;
   if(params == NULL)
   {
	 printf("The system function String.Base64Encode():String failed.\n");
     return true ;
   }


    PDX_STRING str  = (PDX_STRING)for_var->obj ;
	
	PDX_STRING nstr = dx_string_create_bU(b64_encode(str->stringa,str->bcount)) ;
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
    printf("The system function String.Base64Encode():String failed.\n");
    hdr_sys_func_free_params(params) ;
    return true ;
}


bool hdr_domStringBase64Decode(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token,PHDR_VAR for_var,PHDR_VAR *result)
{
	/*
	 The function returns the string as a base64 decoded string
	*/

   PHDR_SYS_FUNC_PARAMS params = hdr_sys_func_init_params(inter,token->parameters,0) ;
   if(params == NULL)
   {
	 printf("The system function String.Base64Decode():String failed.\n");
     return true ;
   }


    PDX_STRING str  = (PDX_STRING)for_var->obj ;
	
	PDX_STRING nstr = dx_string_create_bU(b64_decode(str->stringa,str->bcount)) ;
	if(nstr == NULL) 
	{
		nstr = dx_string_createU(NULL,"") ;
		if(inter->warnings == true)
		hdr_inter_print_warning(inter,"The conversion of a base64 encoded string to reqular string failed. Empty string returned") ;
	}

	*result   = hdr_var_create((void*)nstr,"",hvf_temporary_ref,NULL) ;
	(*result)->type = hvt_simple_string ;

    success:
    hdr_sys_func_free_params(params) ;
    return false ;

    fail : 
    printf("The system function String.Base64Decode():String failed.\n");
    hdr_sys_func_free_params(params) ;
    return true ;
}


bool hdr_domStringBase64DBinary(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token,PHDR_VAR for_var,PHDR_VAR *result)
{
	/*
	 The function decodes the string as a bytes array
	*/

   PHDR_SYS_FUNC_PARAMS params = hdr_sys_func_init_params(inter,token->parameters,0) ;
   if(params == NULL)
   {
	 printf("The system function String.B64DBinary():Bytes failed.\n");
     return true ;
   }


    PDX_STRING str  = (PDX_STRING)for_var->obj ;
	
	PHDR_BYTES bytes = NULL ;
	size_t ret_len = 0 ;
	char *buff = b64_decode_ex(str->stringa,str->bcount,&ret_len) ;
	if(buff == NULL) 
	{
		bytes =  hdr_bytes_create(0) ;
		if(inter->warnings == true)
		hdr_inter_print_warning(inter,"The conversion of a base64 encoded string to bytes failed. Empty buffer returned") ;
	}
	else
	{
	  bytes = (PHDR_BYTES) malloc(sizeof(struct hdr_bytes)) ;
	  bytes->bytes  = buff ;
	  bytes->length = ret_len ;
	}

	*result   = hdr_var_create(bytes,"",hvf_temporary_ref,NULL) ;
	(*result)->type = hvt_bytes ;

    success:
    hdr_sys_func_free_params(params) ;
    return false ;

    fail : 
    printf("The system function String.Base64Decode():String failed.\n");
    hdr_sys_func_free_params(params) ;
    return true ;
}


/*** 

The following function converts a string in JSON form to a list with the appropriate fields.
for example a JSON as {"field1":"value","field2":[10,28,34]}
will be transformed to a list as :
$list.field1
$list.field2[0]
$list.field2[1]
$list.field2[2]

***/


bool hdr_domStringToJson(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token,PHDR_VAR for_var,PHDR_VAR *result)
{
	/*
	 The function returns the string as a list structured based to the JSON structure
	*/

   PHDR_SYS_FUNC_PARAMS params = hdr_sys_func_init_params(inter,token->parameters,1) ;
   if(params == NULL)
   {
	 printf("The system function String.JsonToList($error:String):List failed.\n");
     return true ;
   }

   bool type_error ;
   PDX_STRING error = hdr_inter_ret_string(params->params[0],&type_error) ; 
   if(type_error == true)
   {
	 printf("The  parameter must be a String variable.\n");
     goto fail ;
   }

   error = dx_string_createU(error,"");

   PDX_STRING str  = (PDX_STRING)for_var->obj ;

   PDX_LIST root = hdrParseJsonString(str,error) ;
   if(error->len != 0) 
   {
	   /*create an empty list and return it*/
	   root = dx_list_create_list() ;
	   *result   = hdr_var_create(root,"",hvf_temporary_ref,NULL) ;
       (*result)->type = hvt_list ;
   }
   else
   {
      *result   = hdr_var_create(root,"",hvf_temporary_ref,NULL) ;
      (*result)->type = hvt_list ;
   }
    success:
    hdr_sys_func_free_params(params) ;
    return false ;

    fail : 
    printf("The system function  String.JsonToList($error:String):List failed.\n");
    hdr_sys_func_free_params(params) ;
    return true ;
}

bool hdr_domStringToXml(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token,PHDR_VAR for_var,PHDR_VAR *result)
{
	/*
	 The function returns the string as a list structured based to the xml structure,
	 the list will have every xml node as a simple list in a named index with the value as a simple string
	 and the properties as a list of named indexed strings
	*/

   PHDR_SYS_FUNC_PARAMS params = hdr_sys_func_init_params(inter,token->parameters,1) ;
   if(params == NULL)
   {
	 printf("The system function String.XmlToList($error:String):List failed.\n");
     return true ;
   }

   bool type_error ;
   PDX_STRING error = hdr_inter_ret_string(params->params[0],&type_error) ; 
   if(type_error == true)
   {
	 printf("The  parameter must be a String variable.\n");
     goto fail ;
   }

   error = dx_string_createU(error,"");

   PDX_STRING str  = (PDX_STRING)for_var->obj ;

   

   *result   = hdrParseXmlString(str,error) ;
   (*result)->type = hvt_list ;

    success:
    hdr_sys_func_free_params(params) ;
    return false ;

    fail : 
    printf("The system function  String.XmlToList($error:String):List failed.\n");
    hdr_sys_func_free_params(params) ;
    return true ;
}



bool hdr_domStringGetExt(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token,PHDR_VAR for_var,PHDR_VAR *result)
{
	/*
	 The function return the text from the end until the [.] character. If the dot does not exist then all the 
	 string is returned.
	*/

   PHDR_SYS_FUNC_PARAMS params = hdr_sys_func_init_params(inter,token->parameters,0) ;
   if(params == NULL)
   {
	 printf("The system function String.GetExt():String failed.\n");
     return true ;
   }
    char *src_indx = ((PDX_STRING)for_var->obj)->stringa ; 
	char* ext = dxCopyStrToCharReverse(&src_indx,'.',"")  ;
	
	*result = hdr_var_create(NULL,"",hvf_temporary_ref,NULL) ;
	(*result)->type    = hvt_simple_string ;
	hdr_var_set_obj(*result,dx_string_createU(NULL,ext)) ;
	free(ext);
	

    success:
    hdr_sys_func_free_params(params) ;
    return false ;

    fail : 
    printf("The system function String.GetExt():String failed.\n");
    hdr_sys_func_free_params(params) ;
    return true ;
}


bool hdr_domStringXorHex(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token,PHDR_VAR for_var,PHDR_VAR *result)
{
	/*
	 The function returns the string as a hexadecimal representing xored by the key 
	*/

   PHDR_SYS_FUNC_PARAMS params = hdr_sys_func_init_params(inter,token->parameters,1) ;
   if(params == NULL)
   {
	 printf("The system function String.XorHex($key:String):String failed.\n");
     return true ;
   }

    bool type_error = false ;
    PDX_STRING key  = hdr_inter_ret_string(params->params[0],&type_error) ;
	if(type_error == true)
    {
	  printf("The parameter must be a string .\n");
      goto fail ;
    }


    PDX_STRING str  = (PDX_STRING)for_var->obj ;
	
	PDX_STRING nstr = dxStrToXorHex(str,key)   ;
	if(nstr == NULL) 
	{
		nstr = dx_string_createU(NULL,"") ;
		if(inter->warnings == true)
			hdr_inter_print_warning(inter, "The [XOR] of the string to a hexadecimal encoded string failed. Empty string returned");
	}

	*result   = hdr_var_create((void*)nstr,"",hvf_temporary_ref,NULL) ;
	(*result)->type = hvt_simple_string ;

    success:
    hdr_sys_func_free_params(params) ;
    return false ;

    fail : 
    printf("The system function String.XorHex($key : String):String failed.\n");
    hdr_sys_func_free_params(params) ;
    return true ;
}

bool hdr_domStringXorHexStr(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token,PHDR_VAR for_var,PHDR_VAR *result)
{
	/*
	 The function returns a hexadecimal representing xored by the key string to its original form 
	*/

   PHDR_SYS_FUNC_PARAMS params = hdr_sys_func_init_params(inter,token->parameters,1) ;
   if(params == NULL)
   {
	 printf("The system function String.XorHexStr($key:String):String failed.\n");
     return true ;
   }

    bool type_error = false ;
    PDX_STRING key  = hdr_inter_ret_string(params->params[0],&type_error) ;
	if(type_error == true)
    {
	  printf("The parameter must be a string .\n");
      goto fail ;
    }


    PDX_STRING str  = (PDX_STRING)for_var->obj ;
	
	PDX_STRING nstr = dxStrUnXorHex(str,key)   ;
	if(nstr == NULL) 
	{
		nstr = dx_string_createU(NULL,"") ;
		if(inter->warnings == true)
			hdr_inter_print_warning(inter, "The decoding of a hexadecimal key xored encoded string failed. Empty string returned");
	}

	*result   = hdr_var_create((void*)nstr,"",hvf_temporary_ref,NULL) ;
	(*result)->type = hvt_simple_string ;

    success:
    hdr_sys_func_free_params(params) ;
    return false ;

    fail : 
    printf("The system function String.XorHexStr($key : String):String failed.\n");
    hdr_sys_func_free_params(params) ;
    return true ;
}



bool hdr_domStringXor(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token,PHDR_VAR for_var,PHDR_VAR *result)
{
	/*
	 The function returns the string as binary xored with the key
	*/

   PHDR_SYS_FUNC_PARAMS params = hdr_sys_func_init_params(inter,token->parameters,1) ;
   if(params == NULL)
   {
	 printf("The system function String.Xor($key:String):Bytes failed.\n");
     return true ;
   }

    bool type_error = false ;
    PDX_STRING key  = hdr_inter_ret_string(params->params[0],&type_error) ;
	if(type_error == true)
    {
	  printf("The parameter must be a string .\n");
      goto fail ;
    }


    PDX_STRING str  = (PDX_STRING)for_var->obj ;
	char * buff = dxBytesXor(str->stringa,str->bcount,key)   ;

    PHDR_BYTES bytes = NULL ;

	if(buff == NULL) 
	{
		bytes = hdr_bytes_create(0);
		if(inter->warnings == true)
			hdr_inter_print_warning(inter, "The [XOR] of the string failed. Empty [Bytes] returned");
	}

    bytes = (PHDR_BYTES) malloc(sizeof(struct hdr_bytes)) ;
	bytes->length = str->bcount ;
	bytes->bytes  = buff        ;

    *result   = hdr_var_create(bytes,"",hvf_temporary_ref,NULL) ;
	(*result)->type = hvt_bytes ;


    success:
    hdr_sys_func_free_params(params) ;
    return false ;

    fail : 
    printf("The system function String.Xor($key : String):Bytes failed.\n");
    hdr_sys_func_free_params(params) ;
    return true ;
}

bool hdr_domStringUpper(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token,PHDR_VAR for_var,PHDR_VAR *result)
{
	/*
	 The function returns the strings latin (ASCII) characters as uppercase 
	*/

   PHDR_SYS_FUNC_PARAMS params = hdr_sys_func_init_params(inter,token->parameters,0) ;
   if(params == NULL)
   {
	 printf("The system function String.Upper():String failed.\n");
     return true ;
   }

    PDX_STRING str    = (PDX_STRING)for_var->obj ;
	PDX_STRING newstr = UpperCase(str) ;

    *result   = hdr_var_create(newstr,"",hvf_temporary_ref,NULL) ;
	(*result)->type = hvt_simple_string ;


    success:
    hdr_sys_func_free_params(params) ;
    return false ;

    fail : 
    printf("The system function String.Upper():String failed.\n");
    hdr_sys_func_free_params(params) ;
    return true ;
}

bool hdr_domStringLower(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token,PHDR_VAR for_var,PHDR_VAR *result)
{
	/*
	 The function returns the strings latin (ASCII) characters as uppercase 
	*/

   PHDR_SYS_FUNC_PARAMS params = hdr_sys_func_init_params(inter,token->parameters,0) ;
   if(params == NULL)
   {
	 printf("The system function String.Lower():String failed.\n");
     return true ;
   }

    PDX_STRING str    = (PDX_STRING)for_var->obj ;
	PDX_STRING newstr = LowerCase(str) ;

    *result   = hdr_var_create(newstr,"",hvf_temporary_ref,NULL) ;
	(*result)->type = hvt_simple_string ;


    success:
    hdr_sys_func_free_params(params) ;
    return false ;

    fail : 
    printf("The system function String.Lower():String failed.\n");
    hdr_sys_func_free_params(params) ;
    return true ;
}


bool hdr_domStringUpperGr(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token,PHDR_VAR for_var,PHDR_VAR *result)
{
	/*
	 The function returns the strings greeks characters as uppercase
	*/

   PHDR_SYS_FUNC_PARAMS params = hdr_sys_func_init_params(inter,token->parameters,1) ;
   if(params == NULL)
   {
	 printf("The system function String.UpperGr(plain:Boolean):String failed.\n");
     return true ;
   }

    bool type_error = false ;
    bool no_tonos  = hdr_inter_ret_bool(params->params[0],&type_error) ;
	if(type_error == true)
    {
	  printf("The parameter must be a boolean .\n");
      goto fail ;
    }


    PDX_STRING str  = (PDX_STRING)for_var->obj ;
	
	PDX_STRING nstr = UpperCaseGr(str,no_tonos) ;

    *result   = hdr_var_create(nstr,"",hvf_temporary_ref,NULL) ;
	(*result)->type = hvt_simple_string ;


    success:
    hdr_sys_func_free_params(params) ;
    return false ;

    fail : 
    printf("The system function String.UpperGr(plain:Boolean):String failed.\n");
    hdr_sys_func_free_params(params) ;
    return true ;
}

bool hdr_domStringLowerGr(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token,PHDR_VAR for_var,PHDR_VAR *result)
{
	/*
	 The function returns the strings greeks characters as uppercase
	*/

   PHDR_SYS_FUNC_PARAMS params = hdr_sys_func_init_params(inter,token->parameters,1) ;
   if(params == NULL)
   {
	 printf("The system function String.LowerGr(plain:Boolean):String failed.\n");
     return true ;
   }

    bool type_error = false ;
    bool no_tonos  = hdr_inter_ret_bool(params->params[0],&type_error) ;
	if(type_error == true)
    {
	  printf("The parameter must be a boolean .\n");
      goto fail ;
    }


    PDX_STRING str  = (PDX_STRING)for_var->obj ;
	
	PDX_STRING nstr = LowerCaseGr(str,no_tonos) ;

    *result   = hdr_var_create(nstr,"",hvf_temporary_ref,NULL) ;
	(*result)->type = hvt_simple_string ;


    success:
    hdr_sys_func_free_params(params) ;
    return false ;

    fail : 
    printf("The system function String.LowerGr(plain:Boolean):String failed.\n");
    hdr_sys_func_free_params(params) ;
    return true ;
}


bool hdr_domStringPrepareFJson(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token,PHDR_VAR for_var,PHDR_VAR *result)
{
	/*
	 The function  replace the json invalid characters in the the strings with the valid symbols and returns a new string 
	*/

   PHDR_SYS_FUNC_PARAMS params = hdr_sys_func_init_params(inter,token->parameters,0) ;
   if(params == NULL)
   {
	 printf("The system function String.PrepareForJson():String failed.\n");
     return true ;
   }

    PDX_STRING str    = (PDX_STRING)for_var->obj ;
	PDX_STRING newstr =hdr_json_h_replace_cntrl(str) ;

    *result   = hdr_var_create(newstr,"",hvf_temporary_ref,NULL) ;
	(*result)->type = hvt_simple_string ;


    success:
    hdr_sys_func_free_params(params) ;
    return false ;

    fail : 
    printf("The system function String.PrepareForJson():String failed.\n");
    hdr_sys_func_free_params(params) ;
    return true ;
}

bool hdr_domStringConvertEscapedU(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token,PHDR_VAR for_var,PHDR_VAR *result)
{
	/*
	 The function returns a string that have escaped utf8 characters (\u0000) to a true utf8 characters string 
	*/

   PHDR_SYS_FUNC_PARAMS params = hdr_sys_func_init_params(inter,token->parameters,0) ;
   if(params == NULL)
   {
	 printf("The system function String.ConvertEUTF8():String failed.\n");
     return true ;
   }

    PDX_STRING str  = (PDX_STRING)for_var->obj ;	
	PDX_STRING nstr = ConvertEscapedUTF8ToString(str) ;

	*result   = hdr_var_create((void*)nstr,"",hvf_temporary_ref,NULL) ;
	(*result)->type = hvt_simple_string ;

    success:
    hdr_sys_func_free_params(params) ;
    return false ;

    fail : 
    printf("The system function String.ConvertEUTF8()($space_as_plus:Boolean):String failed.\n");
    hdr_sys_func_free_params(params) ;
    return true ;
}






