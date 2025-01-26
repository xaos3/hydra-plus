/*
 This file has a simple implementation for the JSON 
 syntax.

 Nikos Mourgis , deus-ex.gr 2024

 Live Long and Prosper
*/


/*
  Supported types :

  "name":"string"
  "name":number
  "name":bool
  {object}
  [array]

*/

/*
 Parse a JSON string and convert it to a List type

 root  : Fast List
 array : Simple List
 object: Fast List
 string: Simple String
 number: Real
 bool  : Boolean

 */


#define HDR_JSON_HASH_BUCKETS 5

bool hdr_json_handle_object(PDX_HASH_TABLE obj, char **str,PDX_STRING error) ;
bool hdr_json_handle_array(PDX_LIST arr, char **str,PDX_STRING error)        ;
void hdr_json_ret_arr_str(PDX_STRINGLIST json , PDX_LIST arr,bool add_coma)				 ;
void hdr_json_ret_arr_obj(PDX_STRINGLIST json , PDX_LIST arr,bool add_coma)				 ;



char *hdr_json_copy_string(char **json,PDX_STRING error)
{
 /*
  the function copy from *json+1 (skip the first " ) to the next " and is aware 
  and converts the escape sequences \\ \" \/ \r \n \t to bytes
  The function returns a new malloced string with the string and the **json 
  is pointing to the next character after "

 */

  error = dx_string_createU(error,"") ;
  char *str = NULL ;
  char *temp_json = *json ; 
  temp_json++ ; /*skip the first " */
  /*calculate the memory that we will need and do the error checking*/
  DXLONG64 bcount = 0 ;
  while(*temp_json != 0)
  {
     if(*temp_json == '"') break ;
  
	 if((*temp_json == '\\')&&((*(temp_json+1))!='u')) /*the \u signals a unicode char (4 hexadecimal digits follows)*/
	 {
	   /*the next character must create one of the above escape sequences else we will throw an error*/
	  char next_char = *(temp_json+1) ; 
	  if((next_char =='\\')||(next_char =='"')||(next_char =='/')||(next_char =='r')||(next_char =='n')||(next_char =='t'))
	  {
		/*advance two bytes the string index so one in here*/
		temp_json++ ;
	  }
	  else
	  {
	   hdr_inter_hlp_concatenate_and_set(error,"The escape character [\\] has to be accompanied by an [\\] [/] [\"] [r] [n] [t] or [u]. In sentence :",*json,"") ;
	   goto end ;
	  }
	 
	 }
     
	 temp_json++ ;
     bcount++ ;
  }

  /*ok we have the string BUT its very possible that the string is not valid and we never found the " character , check for it*/
  if(*temp_json != '"')
  {
   hdr_inter_hlp_concatenate_and_set(error,"The string is not valid, the closing \" is missing : ",*json,"") ;
   goto end ;
  }

  /*ok we have a valid string! copy it*/
  str = (char*)malloc(bcount+1);
  str[bcount] = 0 ; /*terminate string*/

  /*copy the string*/
  temp_json = *json ;
  temp_json++ ; /*skip the first " */
  char *str_indx  = str   ;
  while(*temp_json != 0)
  {
     if(*temp_json == '"') break ;
  
	 if((*temp_json == '\\')&&((*(temp_json+1))!='u')) /*the \u signals a utf8 character (4 hexadecimal digits follows) */
	 {
	   /*the next character must create one of the above escape sequences else we will throw an error*/
	  char next_char = *(temp_json+1)   ;  
	  switch(next_char)
	  {
		case '\\' :{*str_indx = '\\';}break;
		case '"'  :{*str_indx = '"';}break;
		case '/'  :{*str_indx = '/';}break;
		case 'r'  :{*str_indx = '\r';}break;
		case 'n'  :{*str_indx = '\n';}break;
		case 't'  :{*str_indx = '\t';}break;
	  }
	  
	   /*advance two bytes the string index so one in here*/
	  temp_json++ ;
	 }
	 else
	 { 
	   *str_indx = *temp_json ;
	 }
     
	 temp_json++ ;
     str_indx++ ;
  }


  /*advance the char pointer*/
  (*json) = temp_json+1 ;

  end:
  return str ;
}


char *hdr_json_copy_numeric_token(char **json,PDX_STRING error)
{
 /*
  The function copy from *json until finds a ',' '}' or ']'.
  a hard limit to the length of the token exists in 32 bytes
  as it is too long for a valid number 
 */

  error = dx_string_createU(error,"") ;
  char *str = NULL ;
  char *temp_json = *json ; 
  /*calculate the memory that we will need and do the error checking*/
  DXLONG64 bcount = 0 ;
  while(*temp_json != 0)
  {
     if((*temp_json == ',')||(*temp_json == ']')||(*temp_json == '}')||(*temp_json == ' ')||(*temp_json == '\n')||(*temp_json == '\r')
		 ||(*temp_json == '\t')||(bcount == 33)) break ;
	 /*check for basic numeric character validity*/
	 if(dxIsCharNumberDotOrSign(*temp_json) == false)
	 {
		hdr_inter_hlp_concatenate_and_set(error,"The token is not a valid number : ",*json,"") ;
		goto end ;
     } 
	 temp_json++ ;
     bcount++ ;
  }


  /*ok we have a valid token! copy it*/
  str = (char*)malloc(bcount+1);
  str[bcount] = 0 ; /*terminate string*/

  /*copy the string from the starting index and bcount byte*/
  memcpy((void*)str,(void*)(*json),bcount);
  /*advance the char pointer*/
  (*json) = temp_json ;

  end:
  return str ;
}


bool hdr_json_check_keyword(char **str , char* word,int len)
{
 /*
  check fast if the keyword exists/isvalid
  keywords : true,false,null
  separators , ] }
 */

  char *str_indx = *str ; 
  for(int i = 0 ; i<len;i++)
  {
    if(*str_indx == 0) return false ;
    if(word[i]!=*str_indx) return false ;
	str_indx++ ;
  }

  /*ommit the control chars and spaces*/
  dxGoForwardWhileChars(&str_indx,"\r\n\t ");
  if((*str_indx != ',')&&(*str_indx != ']')&&(*str_indx != '}') ) return false;

  /*set the string index to the separator*/
  *str = str_indx ;
  return true ;
}

PDX_HASH_TABLE hdr_json_create_obj()
{
 return dx_HashCreateTable(HDR_JSON_HASH_BUCKETS);
}

PDX_LIST hdr_json_create_arr()
{
 return dx_list_create_list() ;
}


void hdr_json_add_obj_string(PDX_HASH_TABLE list, char *name,char *value)
{
	PDXL_OBJECT obj = dxl_object_create()          ;
	obj->key		= dx_string_create_bU(name) ;
	obj->int_key    = list->count				   ; /*for special use if we want to print the keys in the order that they was inserted*/
	PHDR_VAR var    = hdr_var_create(dx_string_create_bU(value),"",hvf_dynamic,NULL) ;
	var->type       = hvt_simple_string ;
	obj->obj		= var ;
	dx_HashInsertObject(list,obj) ;

}


bool hdr_json_add_obj_numeric(PDX_HASH_TABLE list, char *name,char *value)
{
	PDXL_OBJECT obj = dxl_object_create()          ;
	obj->key		= dx_string_create_bU(name) ;
	obj->int_key    = list->count				   ; /*for special use if we want to print the keys in the order that they was inserted*/
	PHDR_VAR var    = hdr_var_create(NULL,"",hvf_dynamic,NULL) ;
	var->type       = hvt_float ;
	PDX_STRING tmps = dx_string_create_bU(value) ;/*wrap the value*/
	bool error		= false;
	var->real		= dx_StringToReal(tmps,&error);
	obj->obj		= var ;
	tmps->stringa   = NULL ; /*the value will be handed by the caller*/
	dx_string_free(tmps) ;
	dx_HashInsertObject(list,obj) ;

	if(error == true) return false ;
	return true ;

}

void hdr_json_add_obj_bool(PDX_HASH_TABLE list, char *name,bool is_true)
{
	PDXL_OBJECT obj = dxl_object_create()          ;
	obj->key		= dx_string_create_bU(name) ;
	obj->int_key    = list->count				   ; /*for special use if we want to print the keys in the order that they was inserted*/
	PHDR_VAR var    = hdr_var_create(NULL,"",hvf_dynamic,NULL) ;
	var->type       = hvt_bool ;
    
	var->integer = 1 ;
	if(is_true == false) var->integer = 0 ;

	obj->obj		= var ;
	dx_HashInsertObject(list,obj) ;
}

void hdr_json_add_obj_null(PDX_HASH_TABLE list, char *name)
{
	PDXL_OBJECT obj = dxl_object_create()          ;
	obj->key		= dx_string_create_bU(name) ;
	obj->int_key    = list->count				   ; /*for special use if we want to print the keys in the order that they was inserted*/
	PHDR_VAR var    = hdr_var_create(NULL,"",hvf_dynamic,NULL) ;
	var->type       = hvt_null ;

	obj->obj		= var ;
	dx_HashInsertObject(list,obj) ;
}

PDX_HASH_TABLE hdr_json_add_obj_obj(PDX_HASH_TABLE list , char *name)
{
	PDXL_OBJECT obj = dxl_object_create()          ;
	obj->key		= dx_string_create_bU(name) ;
	obj->int_key    = list->count				   ; /*for special use if we want to print the keys in the order that they was inserted*/
	PDX_HASH_TABLE tobj = hdr_json_create_obj() ;
	PHDR_VAR var    = hdr_var_create(tobj,"",hvf_dynamic,NULL) ;
	var->type       = hvt_fast_list ;
	obj->obj		= var ; 
	dx_HashInsertObject(list,obj) ;
	return tobj ;
}

PDX_LIST hdr_json_add_obj_arr(PDX_HASH_TABLE list, char *name)
{
	PDXL_OBJECT obj = dxl_object_create()          ;
	obj->key		= dx_string_create_bU(name) ;
	obj->int_key    = list->count				   ; /*for special use if we want to print the keys in the order that they was inserted*/
	PDX_LIST tobj   = hdr_json_create_arr() ;
	PHDR_VAR var    = hdr_var_create(tobj,"",hvf_dynamic,NULL) ;
	var->type       = hvt_list ;
	obj->obj		= var ; 
	dx_HashInsertObject(list,obj) ;
	return tobj ;
}



/**** Array ****/

void hdr_json_add_arr_string(PDX_LIST list,char *value)
{
	PDXL_OBJECT obj = dxl_object_create()          ;
	PHDR_VAR var    = hdr_var_create(dx_string_create_bU(value),"",hvf_dynamic,NULL) ;
	var->type       = hvt_simple_string ;
	obj->obj		= var ;
	dx_list_add_node_direct(list,obj) ;

}


bool hdr_json_add_arr_numeric(PDX_LIST list, char *value)
{
	PDXL_OBJECT obj = dxl_object_create()          ;
	PHDR_VAR var    = hdr_var_create(NULL,"",hvf_dynamic,NULL) ;
	var->type       = hvt_float ;
	PDX_STRING tmps = dx_string_create_bU(value) ;/*wrap the value*/
	bool error		= false;
	var->real		= dx_StringToReal(tmps,&error);
	obj->obj		= var ;
	tmps->stringa   = NULL ; /*the value will be handed by the caller*/
	dx_string_free(tmps) ;
	dx_list_add_node_direct(list,obj) ;

	if(error == true) return false ;

	return true ;
}

void hdr_json_add_arr_bool(PDX_LIST list, bool is_true)
{
	PDXL_OBJECT obj = dxl_object_create()          ;
	PHDR_VAR var    = hdr_var_create(NULL,"",hvf_dynamic,NULL) ;
	var->type       = hvt_bool ;
    
	var->integer = 1 ;
	if(is_true == false) var->integer = 0 ;
	obj->obj		= var ;
	dx_list_add_node_direct(list,obj) ;
}

void hdr_json_add_arr_null(PDX_LIST list)
{
	PDXL_OBJECT obj = dxl_object_create()          ;
	PHDR_VAR var    = hdr_var_create(NULL,"",hvf_dynamic,NULL) ;
	var->type       = hvt_null ;
	obj->obj		= var ;
	dx_list_add_node_direct(list,obj) ;
}

PDX_HASH_TABLE hdr_json_add_arr_obj(PDX_LIST list)
{
	PDXL_OBJECT obj = dxl_object_create()          ;
	PDX_HASH_TABLE tobj = hdr_json_create_obj() ;
	PHDR_VAR var    = hdr_var_create(tobj,"",hvf_dynamic,NULL) ;
	var->type       = hvt_fast_list ;
	obj->obj		= var ; 
	dx_list_add_node_direct(list,obj) ;
	return tobj ;
}

PDX_LIST hdr_json_add_arr_arr(PDX_LIST list)
{
	PDXL_OBJECT obj = dxl_object_create()          ;
	PDX_LIST tobj = hdr_json_create_arr() ;
	PHDR_VAR var    = hdr_var_create(tobj,"",hvf_dynamic,NULL) ;
	var->type       = hvt_list ;
	obj->obj		= var ; 
	dx_list_add_node_direct(list,obj) ;
	return tobj ;
}


bool hdr_json_handle_array(PDX_LIST arr, char **str,PDX_STRING error)
{
	/*the str_indx must be a string that describe an array we have make sure that the string is encapsulated properly*/
	/*
	  go to the position of the next element, in an array the 
	  elements can be : numeric,"string",null,true,false,{},[]
	*/
	bool pending_token = true ;
	bool isEmpty       = true ; /*use this to determine if the array is an empty array*/
	char * str_indx = *str ;
	str_indx++ ;
	while(*str_indx != 0)
	{
		/*skip tabs spaces and crlf*/
		dxGoForwardWhileChars(&str_indx,"\r\n\t ");
		switch(*str_indx)
		{
			case '"' :
			{
			 if(isEmpty == true ) isEmpty = false ;
			 if (pending_token == false) goto pending_token_error ; 
			 /*retrieve the string*/
			 char *str = hdr_json_copy_string(&str_indx,error) ;
			 if( str == NULL) goto fail ;
			 /*after the string MUST exists a [,] or the ]*/
			 dxGoForwardWhileChars(&str_indx,"\r\n\t ");
			 if(*str_indx == 0) 
			 {
			  dx_string_createU(error,"The string was abruptly terminated.");
			 }
			 if(*str_indx == ',')
			 {
			   /*save this value and continue*/
			   pending_token = true ;
			   hdr_json_add_arr_string(arr,str) ;
			   str = NULL ;
			   str_indx++ ;
			 }
			 else
				 if(*str_indx == ']')
				 {
			      /*the array ends goto success*/
				  pending_token = false ;
				  hdr_json_add_arr_string(arr,str) ;
				  str = NULL ;
				  str_indx++;
				  goto success ;
				 }
				 else
				 {
				  hdr_inter_hlp_concatenate_and_set(error,"A ',' or ']' character was expected but a diferent character was found. String : ",str_indx,"") ;
				  free(str);
				  goto fail ;
				 }

			}break ;
			case 'n' :
			{
				 if(isEmpty == true ) isEmpty = false ;
				if (pending_token == false) goto pending_token_error ; 
				/*check if the value is null*/
				if(hdr_json_check_keyword(&str_indx,"null",4) == false)
				{
				  error = dx_string_createU(error,"An unknown token was found in the string that starts with 'n'. String : ") ;
				  hdr_inter_hlp_concatenate_and_set(error,str_indx,"","") ;
				  goto fail ;
				}
				/*add the NULL*/
				pending_token = false ;
				hdr_json_add_arr_null(arr) ;
				if(*str_indx == ']') /*end of array*/
				{
					str_indx++ ; 
					goto success;
				}
			
			}break;
			case 't' :
			{
				if(isEmpty == true ) isEmpty = false ;
				if (pending_token == false) goto pending_token_error ; 
				/*check if the value is true*/
				if(hdr_json_check_keyword(&str_indx,"true",4) == false)
				{
				  hdr_inter_hlp_concatenate_and_set(error,"An unknown token was found in the string that starts with 't'. String : ",
					  str_indx,"") ;
				  goto fail ;
				}
				/*add the true*/
				pending_token = false ;
				hdr_json_add_arr_bool(arr,true) ;
				if(*str_indx == ']') /*end of array*/
				{
					str_indx++ ; 
					goto success;
				}


			}break;
			case 'f' :
			{
				if(isEmpty == true ) isEmpty = false ;
				if (pending_token == false) goto pending_token_error ; 
				/*check if the value is false*/
				if(hdr_json_check_keyword(&str_indx,"false",5) == false)
				{
				  hdr_inter_hlp_concatenate_and_set(error,"An unknown token was found in the string that starts with 'f'. String : ",
					  str_indx,"") ;
				  goto fail ;
				}
				/*add the false*/
				pending_token = false ;
				hdr_json_add_arr_bool(arr,false) ;
				if(*str_indx == ']') /*end of array*/
				{
					str_indx++ ; 
					goto success;
				}

			}break;
			case '{':
			{
				if(isEmpty == true ) isEmpty = false ;
				if (pending_token == false) goto pending_token_error ; 
				/*the element is a new object , handle it*/
				pending_token = false ;
				PDX_HASH_TABLE new_obj = hdr_json_add_arr_obj(arr) ;
				if(hdr_json_handle_object(new_obj,&str_indx,error) == false) goto fail ;
			}break;
			case '[':
			{
				if(isEmpty == true ) isEmpty = false ;
				if (pending_token == false) goto pending_token_error ; 
				/*the element is a new array , handle it*/
				pending_token = false ;
				PDX_LIST new_arr = hdr_json_add_arr_arr(arr) ;
				if(hdr_json_handle_array(new_arr,&str_indx,error) == false) goto fail ;

			}break;
			case ']':
			{
			 if ((pending_token == true)&&(isEmpty ==false)) goto pending_token_error ; 
			 /*the array was done , return */
			 str_indx++ ;
			 goto success ;
			}break;
			case ',':
			{
			 str_indx++ ;
			 if(pending_token == true)
			 {
				 hdr_inter_hlp_concatenate_and_set(error,"A value was expected but a ',' character was found. String : ",
					  str_indx,"") ;
				 goto fail ;
			 }
			 pending_token = true ;
			}break;
			default :
			{
			 /*check if its a numeric value if not then fail*/
			 /*copy the token until a separator is found or an error ocurs*/
			 if(isEmpty == true ) isEmpty = false ;
			 if (pending_token == false) goto pending_token_error ; 
			 char *token = hdr_json_copy_numeric_token(&str_indx,error) ;
			 if (token == NULL) 
			 {
				hdr_inter_hlp_concatenate_and_set(error,"An empty token was found in the string. Check the syntax and the ',' characters. String : ",
				str_indx,"") ;
				 goto fail ;
			 }
			 if (token[0] == 0) 
			 {
				hdr_inter_hlp_concatenate_and_set(error,"An empty token was found in the string. Check the syntax and the ',' characters. String : ",
				str_indx,"") ;
				 goto fail ;
			 }
			 /*try to add it as number*/
			 pending_token = false ;
			 if(hdr_json_add_arr_numeric(arr,token) == false)
			 {
			  free(token);
			  goto fail ;
			 }

			 free(token) ;
			}

		}
	}

success:
	/*all ok update the global string index */
	*str = str_indx ; 
	return true ;

fail :
	return false; 

pending_token_error:
 hdr_inter_hlp_concatenate_and_set(error,"A missplaced token was found in the string. Check the syntax and the ',' characters. String : ",
				str_indx,"") ;
	return false;



}

bool hdr_json_handle_object(PDX_HASH_TABLE obj, char **str,PDX_STRING error)
{
 	/*the str_indx must be a string that describe an object we have make sure that the string is encapsulated properly*/
	/*
	  go to the position of the next element, in an object the 
	  elements can be : numeric,"string",null,true,false,{},[] BUT the have an 
	  name before them e.g. "element_name" : "some string","element_name":[1,7,10]
	*/

	bool pending_token = true ;
	bool isEmpty       = true ; /*use this to determine if the object is an empty object*/
	char * str_indx = *str ;
	str_indx++ ;
	char *name		  = NULL ;/*this will store the current name*/
	while(*str_indx != 0)
	{
		/*skip tabs spaces and crlf*/
		dxGoForwardWhileChars(&str_indx,"\r\n\t ");
		switch(*str_indx)
		{
			case '"' :
			{
			 if(isEmpty == true) isEmpty = false ;
			 if (pending_token == false) goto pending_token_error ; 
			 /*retrieve the string , maybe is a name maybe is a value*/
			 char *val = hdr_json_copy_string(&str_indx,error) ;
			 if( val == NULL) goto fail ;
			 /*after the string MUST exists a [,] a [:] or the ]*/
			 dxGoForwardWhileChars(&str_indx,"\r\n\t ");
			 if(*str_indx == 0) 
			 {
			  dx_string_createU(error,"The string was abruptly terminated.");
			 }
			 if(*str_indx == ',')
			 {
				/*check if a name exists */
				if(name != NULL)
				{
			      /*save this value and continue*/
				  pending_token = true ;
			      hdr_json_add_obj_string(obj,name,val) ;
			      str_indx++ ;
				  name = NULL ;
				  val  = NULL ; /*do this to avoid access violations if an error occurs and the str is freed while assigned*/
				}
				else
				{
				  hdr_inter_hlp_concatenate_and_set(error,"The value has not a name associated with it."," String : ",str_indx) ;
				  free(val);
				  goto fail ;
				}
			 }else
			 if(*str_indx == ':')
			 {
				/*check if a name already exists */
				if(name != NULL)
				{
				  hdr_inter_hlp_concatenate_and_set(error,"A value was expected but a ':' was found."," String : ",str_indx) ;
				  free(val)  ;
				  free(name) ;
				  goto fail  ;
				}
				else
				{
				 name = val ; /*store the name for later*/
				 str_indx++ ;
				}
			 }
			 else
				 if(*str_indx == '}')
				 {
				  
				  if (pending_token == false) goto pending_token_error ; 
				  
				  /*check if there is not any name pending*/
				  if(name == NULL)
				  {
					hdr_inter_hlp_concatenate_and_set(error,"The value has not a name associated with it. String : ",str_indx,"") ;
					free(val);
					goto fail ;
				  }
				  pending_token = false ;
			      /*the object ends goto success*/
				  hdr_json_add_obj_string(obj,name,val) ;
				  name = NULL ;
				  val  = NULL ;
				  str_indx++;
				  goto success ;
				 }
				 else
				 {
				  hdr_inter_hlp_concatenate_and_set(error,"A ',' or '}' character was expected but a diferent character was found. String : ",str_indx,"") ;
				  free(val);
				  goto fail ;
				 }

			}break ;
			case 'n' :
			{
				 if(isEmpty == true) isEmpty = false ;
				 if (pending_token == false) goto pending_token_error ; 
				/*check if the value is null*/
				if(hdr_json_check_keyword(&str_indx,"null",4) == false)
				{
				  hdr_inter_hlp_concatenate_and_set(error,"An unknown token was found in the string that starts with 'n'. String : ",str_indx,"") ;
				  goto fail ;
				}
				/*check if a name is pending*/
				if(name == NULL) 
				{
				  hdr_inter_hlp_concatenate_and_set(error,"The value [null] is not associated with any name. String : ",str_indx,"") ;
				  goto fail ;
				}
				pending_token = false ;
				/*add the NULL*/
				hdr_json_add_obj_null(obj,name) ;
				name = NULL ;
				if(*str_indx == '}') /*end of object*/
				{
					str_indx++ ; 
					goto success;
				}

			}break;
			case 't' :
			{
				 if(isEmpty == true) isEmpty = false ;
				 if (pending_token == false) goto pending_token_error ; 
				/*check if the value is true*/
				if(hdr_json_check_keyword(&str_indx,"true",4) == false)
				{
				  hdr_inter_hlp_concatenate_and_set(error,"An unknown token was found in the string that starts with 't'. String : ",str_indx,"") ;
				  goto fail ;
				}

				/*check if a name is pending*/
				if(name == NULL) 
				{
				  hdr_inter_hlp_concatenate_and_set(error,"The value [true] is not associated with any name.",str_indx,"") ;
				  goto fail ;
				}
				/*add the true*/
				pending_token = false ;
				hdr_json_add_obj_bool(obj,name,true) ;
				name = NULL ;
				if(*str_indx == '}') /*end of object*/
				{
					str_indx++ ; 
					goto success;
				}

			}break;
			case 'f' :
			{
				 if(isEmpty == true) isEmpty = false ;
				 if (pending_token == false) goto pending_token_error ; 
				/*check if the value is false*/
				if(hdr_json_check_keyword(&str_indx,"false",5) == false)
				{
				  hdr_inter_hlp_concatenate_and_set(error,"An unknown token was found in the string that starts with 'f'. String : ",str_indx,"") ;
				  goto fail ;
				}

				/*check if a name is pending*/
				if(name == NULL) 
				{
				  hdr_inter_hlp_concatenate_and_set(error,"The value [false] is not associated with any name.",str_indx,"") ;
				  goto fail ;
				}

				/*add the false*/
				pending_token = false ;
				hdr_json_add_obj_bool(obj,name,false) ;
				name = NULL ;
				if(*str_indx == '}') /*end of object*/
				{
					str_indx++ ; 
					goto success;
				}

			}break;
			case '{':
			{
				 if(isEmpty == true) isEmpty = false ;
				 if (pending_token == false) goto pending_token_error ; 
				/*the element is a new object , handle it*/
				/*check if a name is pending*/
				if(name == NULL) 
				{
				  hdr_inter_hlp_concatenate_and_set(error,"The object is not associated with any name.",str_indx,"") ;
				  goto fail ;
				}

				pending_token = false ;
				PDX_HASH_TABLE new_obj = hdr_json_add_obj_obj(obj,name) ;
				name = NULL ;
				if(hdr_json_handle_object(new_obj,&str_indx,error) == false) goto fail ;
			}break;
			case '[':
			{
				 if(isEmpty == true) isEmpty = false ;
				 if (pending_token == false) goto pending_token_error ; 
				/*the element is a new array , handle it*/
				/*check if a name is pending*/
				if(name == NULL) 
				{
				  hdr_inter_hlp_concatenate_and_set(error,"The array is not associated with any name.",str_indx,"") ;
				  goto fail ;
				}

				pending_token = false ;
				PDX_LIST new_arr = hdr_json_add_obj_arr(obj,name) ;
				name = NULL ;
				if(hdr_json_handle_array(new_arr,&str_indx,error) == false) goto fail ;

			}break;
			case '}':
			{
			 if ((pending_token == true)&&(isEmpty == false)) goto pending_token_error ; 
			 /*the object was done , return */
			 /*check if a name is pending*/
			 if(name != NULL)
			 {
				hdr_inter_hlp_concatenate_and_set(error,"A name is not associated with any value. String : ",str_indx,"") ;
				goto fail ;
			 }
			 str_indx++ ;
			 goto success ;
			}break;
			case ',':
			{
			 if(isEmpty == true) isEmpty = false ;
			 str_indx++ ;
			 if(pending_token == true)
			 {
				 hdr_inter_hlp_concatenate_and_set(error,"A value was expected but a ',' character was found. String : ",
					  str_indx,"") ;
				 goto fail ;
			 }
			 pending_token = true ;
			}break;
			default :
			{
			 /*check if its a numeric value if not then fail*/
			 /*copy the token until a separator is found or an error ocurs*/
			 if(isEmpty == true) isEmpty = false ;
			 if (pending_token == false) goto pending_token_error ; 
			 char *token = hdr_json_copy_numeric_token(&str_indx,error) ;
			 if (token == NULL) 
			 {
				hdr_inter_hlp_concatenate_and_set(error,"An empty token was found in the string. Check the syntax and the ',' characters. String : ",
				str_indx,"") ;
				 goto fail ;
			 }
			 if (token[0] == 0) 
			 {
				hdr_inter_hlp_concatenate_and_set(error,"An empty token was found in the string. Check the syntax and the ',',':' characters. String : ",
				str_indx,"") ;
				 goto fail ;
			 }

			/*check if a name is pending*/
			if(name == NULL) 
			{
				hdr_inter_hlp_concatenate_and_set(error,"The numeric token is not associated with any name.",str_indx,"") ;
				goto fail ;
			}

			 /*try to add it as number*/
			 pending_token = false ;
			 if(hdr_json_add_obj_numeric(obj,name,token) == false)
			 {
			  free(token);
			  name = NULL ;
			  goto fail ;
			 }
			 name = NULL ;
			 free(token) ;
			}

		}
	}

success:
	/*all ok update the global string index */
	*str = str_indx ; 
	return true ;

fail :
	if(name != NULL) free(name);
	return false; 

pending_token_error:
 hdr_inter_hlp_concatenate_and_set(error,"A missplaced token was found in the string. Check the syntax and the ',' characters. String : ",
				str_indx,"") ;
	return false;


}



PDX_LIST hdrParseJsonString(PDX_STRING json,PDX_STRING error)
{

 /*the root object is a list and the json is the first and only node*/
 if(json == NULL) return NULL ;
 error = dx_string_createU(error,"");
 PDX_LIST root = dx_list_create_list() ;
 
 char *str_indx = json->stringa ; 
 /*check the type , can be an object or array*/
 if(*str_indx=='[')
 {
  /* if(dxCheckSectionPairing(str_indx,'[',']',"\"") == false) 
   {
	   error = dx_string_createU(error,"The string is not a valid JSON string. The closing ']' is missing") ; 
	   dx_list_delete_list(root) ;
	   return NULL ;
   }
   */
   PDX_LIST arr = hdr_json_add_arr_arr(root) ;
   if(hdr_json_handle_array(arr,&str_indx,error) == false )
   {
	   /*free all the memory in the caller*/
      
	   return root ;
   }


 }
 else
	 if(*str_indx=='{')
	 {
	/*   if(dxCheckSectionPairing(str_indx,'{','}',"\"") == false) 
	   {
		   error = dx_string_createU(error,"The string is not a valid JSON string. The closing '}' is missing") ; 
		   root = dx_list_delete_list(root) ;
		   return NULL ;
	   }
	*/
	   PDX_HASH_TABLE obj = hdr_json_add_arr_obj(root) ;
	   if(hdr_json_handle_object(obj,&str_indx,error)==false) 
	   {
	     /*free all the memory in the caller*/
	     return root ;
	   }
	 }
	 else
	 {
	  error = dx_string_createU(error,"The string is not a valid JSON string. A valid JSON string is encapsulated by {} or []") ;
	  return NULL ;
	 }


 return root ;

}



/*JSON TO STRING*/

PDX_STRING hdr_json_h_replace_cntrl(PDX_STRING val)
{
 
  if(val->stringa[0]==0)
  {
    return dx_string_createU(NULL,"") ;
  }

  PDX_STRINGLIST strl = dx_stringlist_create() ;
  char accum[512] ;
  accum[511] = 0  ;
  char *accum_indx = accum ;
  char *str_indx = val->stringa ;
  
  while(true)
  {
    if(*accum_indx == 0)
	{
	 /*store the string*/
	 dx_stringlist_add_raw_string(strl,accum) ;
	 accum_indx = accum ;
	}
    
	/*check if the character is one of the \r\n\t \/  \\ \" */

	if(*str_indx == '\r')
	{
		*accum_indx = 0 ;
		dx_stringlist_add_raw_string(strl,accum) ;
		dx_stringlist_add_raw_string(strl,"\\r") ;
		*accum_indx = '#' ;/*to clear the 0*/
		accum_indx = accum ;
	}
	else
	if(*str_indx == '\n')
	{
		*accum_indx = 0 ;
		dx_stringlist_add_raw_string(strl,accum) ;
		dx_stringlist_add_raw_string(strl,"\\n") ;
		*accum_indx = '#' ;/*to clear the 0*/
		accum_indx = accum ;
	}
	else
	if(*str_indx == '\t')
	{
		*accum_indx = 0 ;
		dx_stringlist_add_raw_string(strl,accum) ;
		dx_stringlist_add_raw_string(strl,"\\t") ;
		*accum_indx = '#' ;/*to clear the 0*/
		accum_indx = accum ;
	}
	else
	if(*str_indx == '/')
	{
		*accum_indx = 0 ;
		dx_stringlist_add_raw_string(strl,accum) ;
		dx_stringlist_add_raw_string(strl,"\\/") ;
		*accum_indx = '#' ;/*to clear the 0*/
		accum_indx = accum ;
	}
	else
	if((*str_indx == '\\')&&((*(str_indx+1)) !='u'))
	{
		*accum_indx = 0 ;
		dx_stringlist_add_raw_string(strl,accum) ;
		dx_stringlist_add_raw_string(strl,"\\\\") ;
		*accum_indx = '#' ;/*to clear the 0*/
		accum_indx = accum ;
	}
	else
	if(*str_indx == '"')
	{
		*accum_indx = 0 ;
		dx_stringlist_add_raw_string(strl,accum) ;
		dx_stringlist_add_raw_string(strl,"\\\"") ;
		*accum_indx = '#' ;/*to clear the 0*/
		accum_indx = accum ;
	}
	else
	{
	  *accum_indx = *str_indx ;
	  accum_indx++ ;
	}


	str_indx++   ;

   if(*str_indx == 0 )
   {
      *accum_indx = 0 ;
	  dx_stringlist_add_raw_string(strl,accum) ;
	  break ;
   }
  }

  PDX_STRING  buff = dx_stringlist_raw_text(strl) ;
  dx_stringlist_free(strl) ;
  return buff ;

}


PDX_STRING hdr_json_ret_prop_val(PDXL_OBJECT obj,bool add_coma)
{
   PDX_STRING str = dx_string_createU(NULL,"")  ;
   PHDR_VAR   var = (PHDR_VAR)obj->obj			; 

   char coma[3];
   if(add_coma == false)
   coma[0] = 0 ;
   else
   {
	 coma[0] = ','    ;
	 coma[1] = '\n'   ;
	 coma[2] = 0	  ;
   }


   PDX_STRING prop_name = dx_string_createU(NULL,"") ;
   hdr_inter_hlp_concatenate_and_set(prop_name,"\"",obj->key->stringa,"\"") ;

   if((var->type == hvt_simple_string)||(var->type == hvt_simple_string_bcktck))
   { 
	 PDX_STRING tvar = hdr_json_h_replace_cntrl(((PDX_STRING)var->obj));
     hdr_inter_hlp_concatenate_and_set5(str,prop_name->stringa,":\"",tvar->stringa,"\"",coma) ;
	 dx_string_free(tvar);
   }
   else
	   if(var->type == hvt_integer)
	   {
	     PDX_STRING intr = dx_IntToStr(var->integer) ; 
	     hdr_inter_hlp_concatenate_and_set5(str,prop_name->stringa,":",intr->stringa,coma,"") ;
		 dx_string_free(intr) ;
	   }
	   else
		   if(var->type == hvt_float)
		   {
			 PDX_STRING flt = dx_FloatToStr(var->real,8) ; 
			 /*do a litle trick to trim the trailing zeros*/
			 if(dxCharExistsInStr(flt->stringa,'.',"") != -1)
			 {
				char *tstr = &(flt->stringa[flt->bcount-1]) ;
				dxGoBackWhileChars(&tstr,flt->stringa,"0") ;

				if(*tstr=='.') *tstr = 0; /*remove dot if the number is like 100.0000*/
				else
				if(*tstr!=0) *(tstr+1) = 0 ;

				flt->len    = StrLenU(flt->stringa) ;
				flt->bcount = StrLenA(flt->stringa) ;
			 }

			 hdr_inter_hlp_concatenate_and_set5(str,prop_name->stringa,":",flt->stringa,coma,"") ;
			 dx_string_free(flt) ;
		   }
		   else
		  	if(var->type == hvt_bool)
		    {
			  if(var->integer == 1)
			  hdr_inter_hlp_concatenate_and_set5(str,prop_name->stringa,":true",coma,"","") ;
			  else
				  hdr_inter_hlp_concatenate_and_set5(str,prop_name->stringa,":false",coma,"","") ;
		    }
			else
				if(var->type == hvt_null)
				{
				  hdr_inter_hlp_concatenate_and_set5(str,prop_name->stringa,":null",coma,"","") ;
				}
				else
				   {
					hdr_inter_hlp_concatenate_and_set5(str,prop_name->stringa,":unknown",coma,"","") ;
				   }

    dx_string_free(prop_name);
	return str ;
}


PDX_STRING hdr_json_ret_arr_val(PDXL_OBJECT obj,bool add_coma)
{
   PDX_STRING str = dx_string_createU(NULL,"")  ;
   PHDR_VAR   var = (PHDR_VAR)obj->obj			;  

   char coma[3];
   if(add_coma == false)
   coma[0] = 0 ;
   else
   {
	 coma[0] = ','    ;
	 coma[1] = '\n'   ;
	 coma[2] = 0	  ;
   }

   if((var->type == hvt_simple_string)||(var->type == hvt_simple_string_bcktck))
   {

	 PDX_STRING tval = hdr_json_h_replace_cntrl(((PDX_STRING)var->obj)) ;
	 
	 PDX_STRING temp = dx_string_createU(NULL,"");
	 hdr_inter_hlp_concatenate_and_set(temp,"\"",tval->stringa,"\"") ;
     hdr_inter_hlp_concatenate_and_set(str,temp->stringa,coma,"") ;
	 dx_string_free(temp);
	 dx_string_free(tval);
   }
   else
	   if(var->type == hvt_integer)
	   {
	     PDX_STRING intr = dx_IntToStr(var->integer) ; 
		 hdr_inter_hlp_concatenate_and_set(str,intr->stringa,coma,"") ;
		 dx_string_free(intr) ;
	   }
	   else
		   if(var->type == hvt_float)
		   {
			 PDX_STRING flt = dx_FloatToStr(var->real,8) ; 
			 
			 /*do a litle trick to trim the trailing zeros*/
			if(dxCharExistsInStr(flt->stringa,'.',"") != -1)
			 {
				char *tstr = &(flt->stringa[flt->bcount-1]) ;
				dxGoBackWhileChars(&tstr,flt->stringa,"0") ;
				
				if(*tstr=='.')*tstr=0; /*remove dot if the number is like 100.0000*/
				else
				if(*tstr!=0) *(tstr+1) = 0 ;

				flt->len    = StrLenU(flt->stringa) ;
				flt->bcount = StrLenA(flt->stringa) ;
			 }
			 
			 hdr_inter_hlp_concatenate_and_set(str,flt->stringa,coma,"") ;
			 dx_string_free(flt) ;
		   }
		   else
		   if(var->type == hvt_bool)
		   {
			 if(var->integer == 1)
			 hdr_inter_hlp_concatenate_and_set(str,"true",coma,"") ;
			 else
				hdr_inter_hlp_concatenate_and_set(str,"false",coma,"") ;
		   }
		   else
		   if(var->type == hvt_null)
		   {
		    hdr_inter_hlp_concatenate_and_set(str,"null",coma,"") ;
		   }
		   else
		   {
		     hdr_inter_hlp_concatenate_and_set(str,"unknown",coma,"") ;
		   }


	return str ;
}


void hdr_json_ret_obj_str(PDX_STRINGLIST json , PDX_HASH_TABLE obj,bool add_coma)
{
	 /*convert the object to string and adds it to the json string*/
     dx_stringlist_add_raw_string(json,"{\n") ;
	 DXLONG64 elem_indx = 0 ;
	 PDX_LIST *buckets = obj->buckets ;
	 for(DXLONG64 i=0;i<obj->length;i++)
	 {
		  PDX_LIST bucket = buckets[i] ;
		  PDXL_NODE node = bucket->start ;
		  while(node!=NULL)
		  {
			elem_indx++ ;
			PDXL_OBJECT pobj = node->object ;
			PHDR_VAR var     = (PHDR_VAR)pobj->obj ; 
			PDX_STRING prop = NULL ; 
			
			switch(var->type)
			{
			  case hvt_simple_string :
			  case hvt_simple_string_bcktck:
			  case hvt_integer:
			  case hvt_float:
			  case hvt_bool:
			  case hvt_null:
			  {

				if(elem_indx != obj->count) prop =  hdr_json_ret_prop_val(pobj,true) ;
				else
					prop =  hdr_json_ret_prop_val(pobj,false) ;

				dx_stringlist_add_string(json,prop) ;
			  }break ;

			  case hvt_list :
			  {
			    /*array*/
				/*add the name of the property*/
				PDX_STRING propn = dx_string_createU(NULL,"");
				hdr_inter_hlp_concatenate_and_set(propn,"\"",pobj->key->stringa,"\":") ;
				dx_stringlist_add_string(json,propn);
				if(elem_indx != obj->count)
				hdr_json_ret_arr_str(json,(PDX_LIST)var->obj,true) ;
				else
				   hdr_json_ret_arr_str(json,(PDX_LIST)var->obj,false) ;

			  } break;
			  case hvt_fast_list :
			  {
			    /*object*/
			    /*add the name of the property*/
				PDX_STRING propn = dx_string_createU(NULL,"");
				hdr_inter_hlp_concatenate_and_set(propn,"\"",pobj->key->stringa,"\":") ;
				dx_stringlist_add_string(json,propn);
				if(elem_indx != obj->count)
				hdr_json_ret_obj_str(json,(PDX_HASH_TABLE)var->obj,true) ;
				else
				  hdr_json_ret_obj_str(json,(PDX_HASH_TABLE)var->obj,false) ;

			  } break;

			  default : dx_stringlist_add_raw_string(json,"\n[unknown]\n");

			}
		
			node = node->right ;
		  }
 
	 }
	  if(add_coma == true)
		 dx_stringlist_add_raw_string(json,"\n},\n") ;
		 else
			dx_stringlist_add_raw_string(json,"\n}") ;
}

void hdr_json_ret_arr_str(PDX_STRINGLIST json , PDX_LIST arr,bool add_coma)
{
	 /*convert the object to string and adds it to the json string*/
     dx_stringlist_add_raw_string(json,"[\n") ;

		  PDXL_NODE node = arr->start ;
		  while(node!=NULL)
		  {
			PDXL_OBJECT pobj = node->object ;
			PHDR_VAR var     = (PHDR_VAR)pobj->obj ; 
			PDX_STRING prop = NULL ; 
			
			switch(var->type)
			{
			  case hvt_simple_string :
			  case hvt_simple_string_bcktck:
			  case hvt_integer:
			  case hvt_float:
			  case hvt_bool:
			  case hvt_null:
			  {
				if(node->right != NULL) prop =  hdr_json_ret_arr_val(pobj,true) ;
				else
					prop =  hdr_json_ret_arr_val(pobj,false) ;

				dx_stringlist_add_string(json,prop) ;
			  }break;

			  case hvt_list :
			  {
			    /*array*/
				if(node->right != NULL) hdr_json_ret_arr_str(json,(PDX_LIST)var->obj,true) ;
				else
					hdr_json_ret_arr_str(json,(PDX_LIST)var->obj,false) ;

			  } break;
			  case hvt_fast_list :
			  {
			    /*obj*/
				if(node->right != NULL) hdr_json_ret_obj_str(json,(PDX_HASH_TABLE)var->obj,true) ;
				else
					hdr_json_ret_obj_str(json,(PDX_HASH_TABLE)var->obj,false) ;

			  } break;

			  default : dx_stringlist_add_raw_string(json,"\n[unknown]\n");


			}

			node = node->right ;
		  }
	 if(add_coma == true)
		 dx_stringlist_add_raw_string(json,"\n],\n") ;
		 else
			dx_stringlist_add_raw_string(json,"\n]") ;
}


PDX_STRING hdrJSONToString(PDX_LIST root,PDX_STRING error)
{
	if(root->count != 1 )
	{
	  dx_string_createU(error,"The root MUST have one element of the correct JSON type (object or array). Check if the root is not a valid JSON object") ;
	  return NULL ;
	}

	PDX_STRINGLIST strl    = dx_stringlist_create();

	PDXL_OBJECT    dxlobj  = root->start->object   ;
	PHDR_VAR       var     = (PHDR_VAR)dxlobj->obj ;
	if(var->type == hvt_list) 
	{
	  hdr_json_ret_arr_str(strl,(PDX_LIST)var->obj,false) ;
	}
	else
	if(var->type == hvt_fast_list)
	{
	  hdr_json_ret_obj_str(strl,(PDX_HASH_TABLE)var->obj,false) ;
	}
	else
	{
	 dx_string_createU(error,"Unrecognizable variable type in the root.") ;
	 dx_stringlist_free(strl);
	 return NULL ;
	}

	PDX_STRING str = dx_stringlist_raw_text(strl) ;
	dx_stringlist_free(strl);
	return str ;
}

PDX_STRING hdrArrayToString(PDX_LIST arr,PDX_STRING error)
{
	if(arr->count == 0 )
	{
	  return dx_string_createU(NULL,"[]");
	}

	PDX_STRINGLIST strl    = dx_stringlist_create();

	hdr_json_ret_arr_str(strl,arr,false) ;
	
	PDX_STRING str = dx_stringlist_raw_text(strl) ;
	dx_stringlist_free(strl);
	return str ;
}

PDX_STRING hdrObjectToString(PDX_HASH_TABLE obj,PDX_STRING error)
{
	if(obj->count == 0 )
	{
	  return dx_string_createU(NULL,"{}");
	}

	PDX_STRINGLIST strl    = dx_stringlist_create();

	hdr_json_ret_obj_str(strl,obj,false) ;
	
	PDX_STRING str = dx_stringlist_raw_text(strl) ;
	dx_stringlist_free(strl);
	return str ;
}



PDX_STRING hdr_json_db_h_gent(DXLONG64 ftype)
{
	/*check what generic type of field is this*/
	switch(ftype)
	{
	    
		case DX_FIELD_VALUE_NULL:
		{
		  return dx_string_createU(NULL,"NULL") ;
		}break ;

		case DX_FIELD_VALUE_INT:
		{
			return dx_string_createU(NULL,"INTEGER") ;
		}break ;
			   
		case DX_FIELD_VALUE_FLOAT:
		{
			return dx_string_createU(NULL,"REAL") ;
		}break ;

		case DX_FIELD_VALUE_TEXT:
		{
			return dx_string_createU(NULL,"STRING") ;
		}break ;
		case DX_FIELD_VALUE_BLOB:
		{
			return dx_string_createU(NULL,"BLOB") ;
		}
		case DX_FIELD_VALUE_DATE:
		{
			return dx_string_createU(NULL,"DATETIME") ;
		}

		default :  dx_string_createU(NULL,"UNKNOWN") ;
	}

	return NULL ;
}

PDX_STRING hdr_json_db_h_val(PDX_FIELD field, PDXL_OBJECT hobj)
{
  
	switch(field->flags)
	{
	    
		case DX_FIELD_VALUE_NULL:
		{
		  return dx_string_createU(NULL,"null") ;
		}break ;

		case DX_FIELD_VALUE_INT:
		{
			return dx_IntToStr(field->int_key) ;
		}break ;
			   
		case DX_FIELD_VALUE_FLOAT:
		{
			 PDX_STRING flt = dx_FloatToStr(field->float_key,8) ; 
			 /*do a litle trick to trim the trailing zeros*/
			if(dxCharExistsInStr(flt->stringa,'.',"") != -1)
			 {
				char *tstr = &(flt->stringa[flt->bcount-1]) ;
				dxGoBackWhileChars(&tstr,flt->stringa,"0") ;
				
				if(*tstr=='.')*tstr=0; /*remove dot if the number is like 100.0000*/
				else
				if(*tstr!=0) *(tstr+1) = 0 ;

				flt->len    = StrLenU(flt->stringa) ;
				flt->bcount = StrLenA(flt->stringa) ;
			 }
			
			 PDX_STRING ret = dx_string_create_bU(flt->stringa) ;
			 flt->stringa = NULL ;
			 dx_string_free(flt) ;
			 return ret ;

		}break ;

		case DX_FIELD_VALUE_TEXT:
		{
			return hdr_json_h_replace_cntrl(field->key);
		}break ;
		case DX_FIELD_VALUE_BLOB:
		{
			/*
			  now we have a problem. The BLOB type is binary and the JSON does not transport
			  Binary. So we have to transported it in differend form, BASE64 I choose YOU!!
			*/
			PDX_DB_BLOB blob = (PDX_DB_BLOB)field->obj ;
			char* strblob = b64_encode(blob->data,blob->count) ;

			return dx_string_create_bU(strblob) ;
		}
		case DX_FIELD_VALUE_DATE:
		{
			return dx_string_createU(NULL,field->key->stringa) ;
		}

		default : return  dx_string_createU(NULL,"UNKNOWN") ;
	}


}

PDX_STRING hdrDatasetToJson(PDX_QUERY dataset)
{

 /*
  There is a very serious problem with large datasets. 
  The lists ,strings, and structs of hydra+ are memory consuming so for large datasets the string list
  is not very efficient structure. To mitigate this problem we will use a more complex algorithm that 
  will extract the strings of a stringlist every 100 lines as simple string and will add it to the final stringlist.
 */

  if(dataset == NULL) return dx_string_createU(NULL,"");
  PDX_STRINGLIST tempList = dx_stringlist_create() ;	
  PDX_STRINGLIST json    = dx_stringlist_create() ;
  dx_stringlist_add_raw_string(tempList,"[\n")         ;

  DXLONG64 row_indx  = 1 ;
  DXLONG64 findex    = 1 ;
  DXLONG   line_indx = 0 ; 
  PDXL_NODE node = dataset->dataset->start ;
  while(node!=NULL)
  {
     PDX_ROW row = dx_db_query_row_from_node(node) ; 
	 /*new row */
	 PDX_STRING  fcnt      = dx_IntToStr(row_indx)  ;
	 dx_stringlist_add_raw_string(tempList,"[") ;
	 dx_stringlist_add_string(tempList,fcnt)    ;
	 dx_stringlist_add_raw_string(tempList,",") ;

	 PDXL_NODE fnode = row->start ; 
	 PDXL_NODE hnode = dataset->header->start ;
	 /*for all the fields*/
	 while(fnode != NULL)
	 {
	   PDX_FIELD   field = fnode->object ;
	   
	   PDX_STRING  fname     = hnode->object->key			  ; /*field name*/
	   PDX_STRING  ftype     = (PDX_STRING)hnode->object->obj ;/*field database specific type*/
	 
	   /*need disposing*/
	   PDX_STRING  fgtype    = hdr_json_db_h_gent(hnode->object->flags)     ;/*field generic type*/
	   /***************/
	  
	   PDX_STRING  fval      = hdr_json_db_h_val(field,hnode->object)		;/*field value*/
	   PDX_STRING  strfield  = dx_string_createU(NULL,"")					;/*constructed json value for a field as an object {}*/
	   /*add the field as {"name":"","type":"","gtype":"","value":}*/
	   hdr_inter_hlp_concatenate_and_set5(strfield,"{\"name\":\"",fname->stringa,"\",","\"type\":\"",ftype->stringa);
	   dx_stringlist_add_string(tempList,strfield);
	   strfield = dx_string_createU(NULL,"") ;
	   hdr_inter_hlp_concatenate_and_set5(strfield,"\",","\"gtype\":\"",fgtype->stringa,"\",","\"value\":");
	   dx_stringlist_add_string(tempList,strfield);
	   bool encaps = false ;
	   if((hnode->object->flags == DX_FIELD_VALUE_TEXT)||(hnode->object->flags == DX_FIELD_VALUE_BLOB)||
		   (hnode->object->flags == DX_FIELD_VALUE_DATE))
				encaps = true ;

	   if((encaps == true)&&(field->flags != DX_FIELD_VALUE_NULL)) /*null values are not be encapsulated in ""*/
	   dx_stringlist_add_raw_string(tempList,"\"") ; 

	   dx_stringlist_add_string(tempList,fval);

	   if((encaps == true)&&(field->flags != DX_FIELD_VALUE_NULL)) /*null values do not be encapsed in ""*/
	   dx_stringlist_add_raw_string(tempList,"\"") ;

	   if(fnode->right!=NULL)
	   dx_stringlist_add_raw_string(tempList,"},") ;
	   else
		dx_stringlist_add_raw_string(tempList,"}") ;


	   dx_string_free(fgtype) ;
 	   hnode = hnode->right ;
	   fnode = fnode->right ;
	 }

   if(node->right == NULL)
   dx_stringlist_add_raw_string(tempList,"]") ;
   else
	dx_stringlist_add_raw_string(tempList,"],\n") ;

   node = node->right ;
   row_indx++  ;
   line_indx++ ;

   /*do the magic!*/
   if(line_indx == 100)
   {
      line_indx = 0 ;
	  /*save the string*/
	  PDX_STRING tmps = dx_stringlist_raw_text(tempList) ;
	  dx_stringlist_add_string(json,tmps)                ;
	  dx_stringlist_clear(tempList)                      ;
   }

  }
  
  PDX_STRING tmps = dx_stringlist_raw_text(tempList) ; /*this will be freed in the stringlist*/
  dx_stringlist_add_string(json,tmps)                ;
  dx_stringlist_add_raw_string(json,"\n]")           ;

  PDX_STRING ret = dx_stringlist_raw_text(json) ;
  dx_stringlist_free(tempList);
  dx_stringlist_free(json) ;
  
  return ret ;

}



