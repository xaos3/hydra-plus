/*
 Hydra+ special string for fast expanding variable names in text,
 and special string manipulation routines.

 Nikos Mourgis deus-ex.gr 2024

 Live Long and Prosper.

*/

#define STR_RES_BUFF_LEN 4096



bool hdr_res_str_h_name_exists(PDX_LIST lst , PDX_STRING name)
{
   PDXL_NODE node = lst->start ;
   while(node != NULL )
   {
	   PDXL_OBJECT obj = node->object ;
	   PDX_STRING  str = obj->key ;
	   if(dx_string_native_compare(str,name) == dx_equal) return true ;	   
	   node = node->right ;
   }

   return false ;
}

PHDR_COMPLEX_STR_RES hdr_res_str_create()
{
	PHDR_COMPLEX_STR_RES str = malloc(sizeof(struct hdr_complex_str_res) ) ;
	if(str == NULL) return NULL ;
	str->vars_pos   = dx_list_create_list()   ;
	if(str->vars_pos == NULL) 
	{
	 free(str)   ;
	 return NULL ;
	}
	str->text_lines = dx_stringlist_create()  ;
	if(str->text_lines == NULL)
	{
	  dx_list_delete_list(str->vars_pos) ;
	  free(str);
	  return NULL ;
	}

	return str ;
}


PHDR_COMPLEX_STR_RES hdr_res_str_free(PHDR_COMPLEX_STR_RES str)
{
	/*
	 free the memory of the object and returns NULL
	*/
	if(str == NULL) return NULL ;
	dx_stringlist_free(str->text_lines) ;
	PDXL_NODE node = str->vars_pos->start ;
	while(node != NULL)
	{
	    PDXL_OBJECT obj = node->object ;
		dx_string_free(obj->key);
		free(obj) ;
		node = node->right ;
	}
	dx_list_delete_list(str->vars_pos) ;

	free(str) ;
	return NULL ;
}

PHDR_COMPLEX_STR_RES hdr_res_str_add_str(PHDR_COMPLEX_STR_RES str,PDX_STRING src_str)
{
 /*
   the function gets a src_str and assign it to the str. In the proccess, the src_str will be 
   encoded to utf8 if its in another form and all the variables names will create new lines and entries
   in the vars_pos
 */
  if(str == NULL)      return NULL ;
  if(src_str == NULL)  return NULL ;
  char * utf8str = NULL ;

  switch(src_str->type)
  {
	  case dx_wide : 
	  {
	    utf8str = dx_ConvertWToUtf8(src_str->stringw) ;
	  }break;
	  case dx_ansi :
	  {
		  utf8str = dx_ConvertAnsiToUtf8(src_str->stringa) ;
	  }break;
	  case dx_utf8 :
	  {
		  utf8str = src_str->stringa ;
	  }break;
	  case dx_wstring :
	  {
		  utf8str = dx_ConvertDXSToUtf8(src_str->stringx);
	  }break;
	  case dx_null :
	  {
		  return NULL ;
	  }
  
  }

  /*parse the string and break the text in lines based the variables and character end of line (10)*/
  char *b_line = (char*)malloc(STR_RES_BUFF_LEN) ;
  int b_line_count = 0		   ;
  char *b_line_indx = b_line   ;
  *b_line_indx		= 0		   ;/*initialize to empty string*/
  char *utf8str_indx = utf8str ; 
  char *prev_char = NULL ;
  int char_len	  = 0    ;
  char ch[5]			 ;

  while(true)
  {
    dx_get_utf8_char_ex2(&utf8str_indx,&prev_char,&char_len) ;
	short bc = dxUtf8CharByteCount(*prev_char) ;
	if(bc == 0) 
	{
		free(b_line);
		return NULL ;
	}

	dxRetrieveBytes(ch,prev_char,bc) ; /*get the utf8 character*/

	if( ch[0] == 0 ) 
	{
		/*check if there is data remain to write to the string*/
		if(b_line_count != 0)
		{
		  /*save the line*/
		  *b_line_indx = 0 ; 
		  dx_stringlist_add_raw_string(str->text_lines,b_line) ;
		}
		break ;
	}
	/*check if the buffer is already full, we will check for 2 bytes and not one as we 
	  need room to store the 10 character and the terminating one 
	  in special cases
	*/
	if(b_line_count == STR_RES_BUFF_LEN - 2)
	{
	 /*add the buffer to the string*/
	  b_line[STR_RES_BUFF_LEN-2] = 0 ;
	  dx_stringlist_add_raw_string(str->text_lines,b_line) ;
	  b_line_indx = b_line ;
	  b_line_count = 0     ;
	  continue ;
	}
	/*check the character and act acordinally*/
	if (ch[0] == 10)
	{
	  /*new line, create a new entry to the list*/
	  *b_line_indx = 10 ;
	  b_line_indx++;
	  *b_line_indx     = 0   ;/*terminating*/
	  dx_stringlist_add_raw_string(str->text_lines,b_line) ;
	  b_line_indx = b_line ;
	  b_line_count = 0     ;
	  continue ;
	}

	/*check for the '{' character*/
	if(ch[0] == '{')
	{
	  /*check if the next for proccess byte is the $*/
      if(*(utf8str_indx) == '$')
	  {
	    /*a variable expansion command must terminate with } and all the command is {$some_name}*/
		/*the function check for the HYDRA_MAX_ENTITY_NAME in bytes as we do not support
		  names in entities other than ascii 127 
		*/	
		int char_count = 1 ;
		char entity_name [HYDRA_MAX_ENTITY_NAME+1] ;
		char *ch_indx = utf8str_indx+1 ; 
		entity_name[0] = '$' ;
		bool success = false ; 
		while((char_count<(HYDRA_MAX_ENTITY_NAME+1))&&(*ch_indx !=0))
		{
		  if(*ch_indx == '}')
		  {
			  success = true ;
			  ch_indx++ ;
			  entity_name[char_count] = 0 ; /*terminate string*/
			  break;
		  }
		  entity_name[char_count] = *ch_indx ;
		  ch_indx++ ;
		  char_count++ ;
		}
		/*if the operation succeded then we add an empty line in text and store the variable's info*/

		if(success == false)
		{
		 /*we do nothing , we let the normal flow to add the character in the string for storage*/
		}
		else
		{
			/*add the line that we have accumulate so far*/
			if(b_line_count > 0 )
			{
				*b_line_indx     = 0   ;/*terminating*/
				dx_stringlist_add_raw_string(str->text_lines,b_line) ;
			}
			/*add a line*/
			dx_stringlist_add_raw_string(str->text_lines,entity_name) ;/*the name is just a placeholder for debuging*/
			/*get the address of the object*/
			PDXL_OBJECT obj = str->text_lines->end->object ;
			/*add the variable's info to the proper structure*/
			PDXL_OBJECT nobj = (PDXL_OBJECT)malloc(sizeof(struct dxl_object)) ;
			nobj->obj     = obj ;
			nobj->key     = dx_string_createU(NULL,entity_name);
			nobj->int_key = str->text_lines->count - 1 ; /*save the position of the line in the text */ 
			dx_list_add_node_direct(str->vars_pos,nobj)    ; 
			/*pass through the variable text*/
			utf8str_indx  = ch_indx  ;
			b_line_count  = 0		 ;
			b_line_indx = b_line     ;
		 continue ;
		}
	  }

	}

	if(ch[0] == 0)
	{
	 /*Add the current line and exit*/
	 *b_line_indx = 0 ;/*terminating*/
	  dx_stringlist_add_raw_string(str->text_lines,b_line) ;
	  break;
	 
	}
	/*add the current character to the line*/
	memcpy(b_line_indx,ch,bc) ;
	b_line_indx = b_line_indx + bc ;
	b_line_count = b_line_count + bc ;

  }

  /*free the temporary buffer if needed*/
  if(src_str->type != dx_utf8) free(utf8str);
  free(b_line) ;
  return str;
}


PHDR_COMPLEX_STR_RES hdr_res_str_assign_str(PHDR_COMPLEX_STR_RES str,PDX_STRING src_str)
{
 /*free the memory of the object and add the string*/

 dx_stringlist_clear(str->text_lines) ;

 PDXL_NODE node = str->vars_pos->start ;
 while(node != NULL)
 {
	PDXL_OBJECT obj = node->object ;
	dx_string_free(obj->key);
	free(obj) ;
	node = node->right ;
 }

 dx_list_empty_list(str->vars_pos);

 return hdr_res_str_add_str(str,src_str) ;

}


PHDR_COMPLEX_STR_RES hdr_res_str_add_strlist(PHDR_COMPLEX_STR_RES str,PDX_STRINGLIST strlst)
{

	PDXL_NODE node = strlst->start ;
	while(node != NULL)
	{
		PDXL_OBJECT obj = node->object ;
		PDX_STRING  insert_str = obj->key	   ;

		PHDR_COMPLEX_STR_RES res = hdr_res_str_add_str(str,insert_str)   ;
		if(res == NULL) return NULL ;

		node = node->right;
	}
	return str ;
}

PHDR_COMPLEX_STR_RES hdr_res_str_assign_strlist(PHDR_COMPLEX_STR_RES str,PDX_STRINGLIST strlst)
{
	 dx_stringlist_clear(str->text_lines) ;

	 PDXL_NODE node = str->vars_pos->start ;
	 while(node != NULL)
	 {
		PDXL_OBJECT obj = node->object ;
		dx_string_free(obj->key);
		free(obj) ;
		node = node->right ;
	 }

	 dx_list_empty_list(str->vars_pos);
	
	 return hdr_res_str_add_strlist(str,strlst) ;
}



PHDR_COMPLEX_STR_RES hdr_res_str_expand(PHDR_INTERPRETER inter,PHDR_COMPLEX_STR_RES res_str)
{
	/*
	 the function alters the internal state of the res_str without destroying the 
	 mutable state of it. So the function can be called multiple times and it will display the correct values 
	 based of the state of the variables that is refered in.
	*/
	if(res_str == NULL) return NULL					 ;
	if(res_str->vars_pos->start == NULL) return res_str ; /*to not have vars is not an error*/

	/*for all the variable lines*/
	PDXL_NODE   node = res_str->vars_pos->start ;
	while(node != NULL)
	{
		 PDXL_OBJECT obj  = node->object		 ;
		 PHDR_VAR    var  = hdr_inter_retrieve_var_from_block(inter,obj->key) ;
		 if (var == NULL)
		 {
		   printf("The variable name [%s] is not declared in the scope. Keep in mind that only simple variables (not expression or complex tokens) can be expanded in the string.\n",obj->key->stringa);
		   return NULL ;
		 }

		 PDX_STRING line = ((PDXL_OBJECT)obj->obj)->key ;

		 if(hdr_inter_var_gen_type(var) == hdr_ivt_string )
		 {
			/*simple enough*/
			if((var->type == hvt_simple_string)||(var->type == hvt_simple_string_bcktck))
			{
				dx_string_createU(line,((PDX_STRING)var->obj)->stringa) ;
				goto succ_cont ;
			}
			else
			if(var->type == hvt_unicode_string)
			{
			  PDX_STRING ustr =  dx_string_convertU((PDX_STRING)var->obj) ;
			  /*we will do a litle trick to avoid another memory allocation*/
			  if(ustr == NULL) 
			  {
			   printf("Error in converting the Hydra+ unicode string to utf8 string.\n");
			   return NULL ;
			  }

			  dx_string_setU(line,ustr->stringa)  ;
			  ustr->stringa = NULL		  ;
			  free(ustr)				  ;
			  goto succ_cont ;
			}
			else
				if(var->type == hvt_codepoint)
				{
					char buffer[5] ;
					if(dx_string_convert_uint32_to_utf8(buffer,var->integer) != 0)
					{
					  dx_string_createU(line,buffer) ;
					  goto succ_cont ;
					}
					else
					{
					 /*emmit a warning , not a fatal error*/
					 hdr_inter_print_warning(inter, "The conversion of the unicode codepoint to a utf8 character failed. Check the validity of the range.\n");
					 printf("Code Point : %d\n",var->integer);
					 dx_string_createU(line,"-0-") ;
					 goto succ_cont ;
					}
				}

		 }
		 else
		 if(var->type == hvt_complex_string )
		 {
			/*create a single buffer from the complex string and emmit it to the line*/
			 PDX_STRING str = dx_stringlist_raw_text((PDX_STRINGLIST)var->obj) ;
			/*we will do a litle trick to avoid another memory allocation*/
			dx_string_setU(line,str->stringa) ;
			str->stringa = NULL ;
			free(str);
			goto succ_cont ;
		 }
		 if(var->type == hvt_complex_string_resolve)
		 {
	        /*create a single buffer from the string and emmit it to the line*/
			 PDX_STRING str = dx_stringlist_raw_text(((PHDR_COMPLEX_STR_RES)var->obj)->text_lines) ;
			/*we will do a litle trick to avoid another memory allocation*/
			dx_string_setU(line,str->stringa) ;
			str->stringa = NULL ;
			free(str);
			goto succ_cont ;
		 }
		 if(var->type == hvt_bool)
		 {
		   if(var->integer == 0) dx_string_createU(line,"false") ;
		   else
			   dx_string_createU(line,"true") ;
		   goto succ_cont ;
		 }
		 if(var->type == hvt_integer)
		 {
			PDX_STRING int_str = dx_IntToStr(var->integer) ;
			dx_string_setU(line,int_str->stringa) ;
		    int_str->stringa = NULL ;
			free(int_str)			;
			goto succ_cont			;
		 }
		 else
		 if(var->type == hvt_float)
		 {
			PDX_STRING float_str = dx_FloatToStr(var->real,8) ;
			dx_string_setU(line,float_str->stringa) ;
		    float_str->stringa = NULL ;
			free(float_str)			;
			goto succ_cont			;
		 }
		 else
		 {
			 printf("The variable '%s' is of type '%s' that is not emmitable in a string.\n",var->name->stringa,hdr_inter_return_variable_type(var->type));
			 return NULL ;
		 }

		 succ_cont :
		 node = node->right ;
	}

	return res_str ;
}


/*return a simple string from a var_expand*/

PDX_STRING hdr_convert_str_expand(PHDR_INTERPRETER inter,PHDR_COMPLEX_STR_RES res_str)
{
	/*
     The function returns a simple string constructed from the var_expand
	*/

   /*	if(hdr_res_str_expand(inter,res_str) == NULL) return NULL ; we do not auto expand the values*/

	PDX_STRING str = dx_stringlist_raw_text(res_str->text_lines) ;

	return str ;

}


PDX_STRING hdr_res_str_temp_expand(PHDR_INTERPRETER inter,PHDR_COMPLEX_STR_RES res_str)
{
	/*
	 the function does not alters the internal state of the res_str and construct on the fly the simple string,
	 but with the correct variable values
	*/

	/*create a new string list to store and construct the new string*/
	PDX_STRINGLIST tmp_list = dx_stringlist_create() ;
	PDXL_NODE node = res_str->text_lines->start ;
	while(node != NULL)
	{
	  PDXL_OBJECT obj = node->object;
	  dx_stringlist_add_raw_string( tmp_list,obj->key->stringa) ;
	  node = node->right ;
	}

	/*the temp string list is ready. Now we have to replace the variables with the actual values*/
	/*for all the variable lines*/
	node = res_str->vars_pos->start ;
	while(node != NULL)
	{
		 PDXL_OBJECT obj  = node->object		 ;
		 PDX_STRING  line = dx_stringlist_string(tmp_list , obj->int_key)     ; /*get to the right line in the temporary list*/
		 PHDR_VAR    var  = hdr_inter_retrieve_var_from_block(inter,obj->key) ;
		 if (var == NULL)
		 {
		   printf("The variable name [%s] is not declared in the scope. Keep in mind that only simple variables (not expression or complex tokens) can be expanded in the string.\n",obj->key->stringa);
		   return NULL ;
		 }

		 if(hdr_inter_var_gen_type(var) == hdr_ivt_string )
		 {
			/*simple enough*/
			if((var->type == hvt_simple_string)||(var->type == hvt_simple_string_bcktck))
			{
				
				dx_string_createU(line,((PDX_STRING)var->obj)->stringa) ;
				goto succ_cont ;
			}
			else
			if(var->type == hvt_unicode_string)
			{
			  PDX_STRING ustr =  dx_string_convertU((PDX_STRING)var->obj) ;
			  /*we will do a litle trick to avoid another memory allocation*/
			  if(ustr == NULL) 
			  {
			   printf("Error in converting the Hydra+ unicode string to utf8 string.\n");
			   return NULL ;
			  }

			  dx_string_setU(line,ustr->stringa)  ;
			  ustr->stringa = NULL		  ;
			  free(ustr)				  ;
			  goto succ_cont ;
			}
			else
				if(var->type == hvt_codepoint)
				{
					char buffer[5] ;
					if(dx_string_convert_uint32_to_utf8(buffer,var->integer) != 0)
					{
					  dx_string_createU(line,buffer) ;
					  goto succ_cont ;
					}
					else
					{
					 /*emmit a warning , not a fatal error*/
					 hdr_inter_print_warning(inter, "The conversion of the unicode codepoint to a utf8 character failed. Check the validity of the range.\n");
					 printf("Code Point : %d\n",var->integer);
					 dx_string_createU(line,"-0-") ;
					 goto succ_cont ;
					}
				}

		 }
		 else
		 if(var->type == hvt_complex_string )
		 {
			/*create a single buffer from the complex string and emmit it to the line*/
			 PDX_STRING str = dx_stringlist_raw_text((PDX_STRINGLIST)var->obj) ;
			/*we will do a litle trick to avoid another memory allocation*/
			dx_string_setU(line,str->stringa) ;
			str->stringa = NULL ;
			free(str);
			goto succ_cont ;
		 }
		 if(var->type == hvt_complex_string_resolve)
		 {
	        /*create a single buffer from the string and emmit it to the line*/
			 PDX_STRING str = dx_stringlist_raw_text(((PHDR_COMPLEX_STR_RES)var->obj)->text_lines) ;
			/*we will do a litle trick to avoid another memory allocation*/
			dx_string_setU(line,str->stringa) ;
			str->stringa = NULL ;
			free(str);
			goto succ_cont ;
		 }
		 if(var->type == hvt_bool)
		 {
		   if(var->integer == 0) dx_string_createU(line,"false") ;
		   else
			   dx_string_createU(line,"true") ;
		   goto succ_cont ;
		 }
		 if(var->type == hvt_integer)
		 {
			PDX_STRING int_str = dx_IntToStr(var->integer) ;
			dx_string_setU(line,int_str->stringa) ;
		    int_str->stringa = NULL ;
			free(int_str)			;
			goto succ_cont			;
		 }
		 else
		 if(var->type == hvt_float)
		 {
			PDX_STRING float_str = dx_FloatToStr(var->real,8) ;
			dx_string_setU(line,float_str->stringa) ;
		    float_str->stringa = NULL ;
			free(float_str)			;
			goto succ_cont			;
		 }
		 else
		 {
			 printf("The variable '%s' is of type '%s' that is not emmitable in a string.\n",var->name->stringa,hdr_inter_return_variable_type(var->type));
			 return NULL ;
		 }

		 succ_cont :
		 node = node->right ;
	}

	/*so far so good as we have the temp string list ready. Return as string*/

	
	PDX_STRING str = dx_stringlist_raw_text(tmp_list) ;
	dx_stringlist_free(tmp_list) ;
	return str ;
}






