
/*********************** calculating functions******************/
bool hdr_inter_add_var(PHDR_VAR var1, PHDR_VAR var2, double num)
{
	/*
	 the function add the num or the var2 to the var1.
	 If the var2 is NULL then the num will be add (if different of 0)
	 if the var1 is NULL then a message emmited and false is returned.
	 if the var1 is not compatible with the var2 then a message emmited and false is returned.
	*/
	if (var1 == NULL)
	{
		printf("The first variable of the ADD instruction is NULL. Some bug is at large ?\n");
		return false;
	}
	if (var2 != NULL)
	{
		if (((var1->type == hvt_integer) || (var1->type == hvt_float)) && ((var2->type == hvt_integer) || (var2->type == hvt_float)))
		{
			if (var1->type == hvt_integer)
			{
				if (var2->type == hvt_integer) { var1->integer = var1->integer + var2->integer; return true; }
				if (var2->type == hvt_float) { var1->integer = var1->integer + var2->real; return true; } /*let C make the casting*/
			}

			if (var1->type == hvt_float)
			{
				if (var2->type == hvt_integer) { var1->real = var1->real + var2->integer; return true; }
				if (var2->type == hvt_float) { var1->real = var1->real + var2->real; return true; } /*let C make the casting*/
			}

		}
		else
		{
			printf("The variable '%s' and the variable '%s' are not of a compatible type (integer or float).\n", var1->name->stringa, var2->name->stringa);
			return false;
		}

	}
	else
		if (num != 0)
		{
			if (var1->type == hvt_integer)
			{
				var1->integer = var1->integer + num;
				return true;
			}

			if (var1->type == hvt_float)
			{
				var1->real = var1->real + num;
				return true;
			}

		}

	return true;
}

bool hdr_inter_subtr_var(PHDR_VAR var1, PHDR_VAR var2, double num)
{
	/*
	 the function subtract the num or the var2 from the var1.
	 If the var2 is NULL then only the num will be subtract (if different of 0)
	 if the var1 is NULL then a message emmited and false is returned.
	 if the var1 is not compatible with the var2 then a message emmited and false is returned.
	*/
	if (var1 == NULL)
	{
		printf("The first variable of the SUBTR instruction is NULL. Some bug is at large ?\n");
		return false;
	}

	if (var2 != NULL)
	{
		if (((var1->type == hvt_integer) || (var1->type == hvt_float)) && ((var2->type == hvt_integer) || (var2->type == hvt_float)))
		{
			if (var1->type == hvt_integer)
			{
				if (var2->type == hvt_integer) { var1->integer = var1->integer - var2->integer; return true; }
				if (var2->type == hvt_float) { var1->integer = var1->integer - var2->real; return true; } /*let C make the casting*/
			}

			if (var1->type == hvt_float)
			{
				if (var2->type == hvt_integer) { var1->real = var1->real - var2->integer; return true; }
				if (var2->type == hvt_float) { var1->real = var1->real - var2->real; return true; } /*let C make the casting*/
			}

		}
		else
		{
			printf("The variable '%s' and the variable '%s' are not of a compatible type (integer or float).\n", var1->name->stringa, var2->name->stringa);
			return false;
		}

	}
	else
		if (num != 0)
		{
			if (var1->type == hvt_integer)
			{
				var1->integer = var1->integer - num;
				return true;
			}

			if (var1->type == hvt_float)
			{
				var1->real = var1->real - num;
				return true;
			}

		}

	return true;
}

bool hdr_inter_mult_var(PHDR_VAR var1, PHDR_VAR var2, double num)
{
	/*
	 the function multiply the num or var2 with the var1.
	 if the var1 is NULL then a message emmited and false is returned.
	 if the var1 is not compatible with the var2 then a message emmited and false is returned.
	*/
	if (var1 == NULL)
	{
		printf("The first variable of the MULT instruction is NULL. Some bug is at large ?\n");
		return false;
	}

	if (var2 != NULL)
	{
		if (((var1->type == hvt_integer) || (var1->type == hvt_float)) && ((var2->type == hvt_integer) || (var2->type == hvt_float)))
		{
			if (var1->type == hvt_integer)
			{
				if (var2->type == hvt_integer) { var1->integer = var1->integer * var2->integer; return true; }
				if (var2->type == hvt_float) { var1->integer = var1->integer * var2->real; return true; } /*let C make the casting*/
			}

			if (var1->type == hvt_float)
			{
				if (var2->type == hvt_integer) { var1->real = var1->real * var2->integer; return true; }
				if (var2->type == hvt_float) { var1->real = var1->real * var2->real; return true; } /*let C make the casting*/
			}

		}
		else
		{
			printf("The variable '%s' and the variable '%s' are not of a compatible type (integer or float).\n", var1->name->stringa, var2->name->stringa);
			return false;
		}

	}
	else
		{
			if (var1->type == hvt_integer)
			{
				var1->integer = var1->integer * num;
				return true;
			}

			if (var1->type == hvt_float)
			{
				var1->real = var1->real * num;
				return true;
			}

		}

	return true;
}

bool hdr_inter_div_var(PHDR_VAR var1, PHDR_VAR var2, double num)
{
	/*
	 the function divides var1 with the num or the var2.
	 If the var2 is NULL then the num will be the divider
	 if the var1 is NULL then a message emmited and false is returned.
	 if the var1 is not compatible with the var2 then a message emmited and false is returned.
	*/
	if (var1 == NULL)
	{
		printf("The first variable of the DIV instruction is NULL. Some bug is at large ?\n");
		return false;
	}

	if (var2 != NULL)
	{

		if (((var1->type == hvt_integer) || (var1->type == hvt_float)) && ((var2->type == hvt_integer) || (var2->type == hvt_float)))
		{
			if (var1->type == hvt_integer)
			{
				if (var2->type == hvt_integer) 
				{
					if (var2->integer == 0)
					{
						printf("Calculation Error : Division with Zero.\n");
						return false;
					}
					var1->integer = var1->integer / var2->integer; return true;
				}
				if (var2->type == hvt_float) 
				{ 
					if (var2->real == 0) 
					{ 
						printf("Calculation Error : Division with Zero.\n"); 
						return false;
					}
					var1->integer = var1->integer / var2->real; return true; 
				} /*let C make the casting*/
			}

			if (var1->type == hvt_float)
			{
				if (var2->type == hvt_integer) 
				{ 
					if (var2->integer == 0) 
					{ 
					  printf("Calculation Error : Division with Zero.\n"); 
					  return false; 
					} 
					  var1->real = var1->real / var2->integer; 
					  return true; 
				}
				if (var2->type == hvt_float) 
				{ 
					if (var2->real == 0) 
					{ 
						printf("Calculation Error : Division with Zero.\n");
						return false;
					}
					var1->real = var1->real / var2->real;
					return true; 
				} /*let C make the casting*/
			}

		}
		else
		{
			printf("The variable '%s' and the variable '%s' are not of a compatible type (integer or float).\n", var1->name->stringa, var2->name->stringa);
			return false;
		}

	}
	else
		if (num != 0)
		{
			if (var1->type == hvt_integer)
			{
				var1->integer = var1->integer / num;
				return true;
			}

			if (var1->type == hvt_float)
			{
				var1->real = var1->real / num;
				return true;
			}

		}
		else
		{
			printf("Calculation Error : Division with Zero.\n");
			return false; 
		}


	return true;
}

bool hdr_inter_expon_var(PHDR_VAR var1, PHDR_VAR var2, double num)
{
	/*
	 the function do the exponentiation of the var1 with the var2 or num.
	 If the var2 is NULL then the num will be the exponent
	 if the var1 is NULL then a message emmited and false is returned.
	 if the var1 is not compatible with the var2 then a message emmited and false is returned.
	*/
	if (var1 == NULL)
	{
		printf("The first variable of the EXPON instruction is NULL. Some bug is at large ?\n");
		return false;
	}

	if (var2 != NULL)
	{
		if (((var1->type == hvt_integer) || (var1->type == hvt_float)) && ((var2->type == hvt_integer) || (var2->type == hvt_float)))
		{
			if (var1->type == hvt_integer)
			{
				if (var2->type == hvt_integer) { var1->integer = dxPower(var1->integer, var2->integer); return true; }
				if (var2->type == hvt_float) { var1->integer = dxPower(var1->integer, var2->real); return true; } /*let C make the casting*/
			}

			if (var1->type == hvt_float)
			{
				if (var2->type == hvt_integer) { var1->real = pow(var1->real, var2->integer); return true; }
				if (var2->type == hvt_float) { var1->real = pow(var1->real, var2->real); return true; } /*let C make the casting*/
			}

		}
		else
		{
			printf("The variable '%s' and the variable '%s' are not of a compatible type (integer or float).\n", var1->name->stringa, var2->name->stringa);
			return false;
		}

	}
	else
		if (num != 0)
		{
			if (var1->type == hvt_integer)
			{
				var1->integer = pow(var1->integer, num);
				return true;
			}

			if (var1->type == hvt_float)
			{
				var1->real = pow(var1->real, num);
				return true;
			}

		}
		else
		{
			if (var1->type == hvt_integer)
			{
				var1->integer = 1;
				return true;
			}

			if (var1->type == hvt_float)
			{
				var1->real = 1;
				return true;
			}
		}

	return true;
}

bool hdr_inter_mod_var(PHDR_VAR var1, PHDR_VAR var2, double num)
{
	/*
	 the function set the var1 with the modulo of the num or the var2 against var1.
	 If the var2 is NULL then  the num will be used
	 if the var1 is NULL then a message emmited and false is returned.
	 if the var1 is not compatible with the var2 then a message emmited and false is returned.
	 The function will cast the variables to integer if they are float
	*/
	if (var1 == NULL)
	{
		printf("The first variable of the MOD instruction is NULL. Some bug is at large ?\n");
		return false;
	}

	if (var2 != NULL)
	{
		if (((var1->type == hvt_integer) || (var1->type == hvt_float)) && ((var2->type == hvt_integer) || (var2->type == hvt_float)))
		{
			if (var1->type == hvt_integer)
			{
				if (var2->type == hvt_integer) { if(var2->integer == 0) goto div_zero ; var1->integer = var1->integer % var2->integer; return true; }
				if (var2->type == hvt_float)   { if(var2->real == 0) goto div_zero ; var1->integer = var1->integer % (DXLONG64)var2->real; return true;}
			}

			if (var1->type == hvt_float)
			{
				if (var2->type == hvt_integer) {if(var2->integer == 0) goto div_zero ;var1->real = (DXLONG64)var1->real % var2->integer; return true;}
				if (var2->type == hvt_float)   {if(var2->real == 0) goto div_zero ;var1->real = (DXLONG64)var1->real % (DXLONG64)var2->real; return true; }
			}

		}
		else
		{
			printf("The variable '%s' and the variable '%s' are not of a compatible type (integer or float).\n", var1->name->stringa, var2->name->stringa);
			return false;
		}

	}
	else
		{
			if (var1->type == hvt_integer)
			{
				var1->integer = var1->integer % (DXLONG)num; 
				return true; 
			}

			if (var1->type == hvt_float)
			{
				var1->real = (DXLONG64)var1->real % (DXLONG)num;
				return true;
			}

		}

	return true;

	div_zero : 
	printf("Calculation Error : Division with Zero.\n"); 
	return false ;
}

/***************************************************************/


void hdr_inter_var_clear(PHDR_VAR var)
{
	var->type    = hvt_undefined;
	var->integer = 0			;
	var->literal = false		;
	var->obj	 = NULL			;
	var->real	 = 0			;
	var->var_ref = hvf_system   ; /*default value*/
	return;
}

enum hdr_inter_vt {hdr_ivt_numeric, hdr_ivt_string,hdr_ivt_other};
enum hdr_inter_vt  hdr_inter_var_gen_type(PHDR_VAR var)
{
	if ((var->type == hvt_float) || (var->type == hvt_integer) || (var->type == hvt_bool)) return hdr_ivt_numeric;
	if ((var->type == hvt_simple_string) || (var->type == hvt_simple_string_bcktck)||
		(var->type == hvt_unicode_string) || (var->type == hvt_codepoint)) return hdr_ivt_string ;
	return hdr_ivt_other;
}

enum hdr_inter_vt  hdr_inter_var_gen_type_str(PHDR_VAR var)
{
	if ((var->type == hvt_float) || (var->type == hvt_integer)) return hdr_ivt_numeric;
	if ((var->type == hvt_simple_string) || (var->type == hvt_simple_string_bcktck) || (var->type == hvt_complex_string)||
		(var->type == hvt_complex_string_resolve)||(var->type == hvt_unicode_string)) return hdr_ivt_string;
	return hdr_ivt_other;
}

bool hdr_inter_var_is_simple_string(PHDR_VAR var)
{
 if ((var->type == hvt_simple_string) || (var->type == hvt_simple_string_bcktck)) return true ;
 else 
	 return false ;
}

bool hdr_inter_var_is_numeric(PHDR_VAR var)
{
 if ((var->type ==hvt_float) || (var->type == hvt_integer)) return true ;
 else 
	 return false ;
}

//bool hdr_inter_vars_are_interchangable(PHDR_VAR var1 , PHDR_VAR var2)
//{
	 


//}




bool hdr_inter_do_calcs_concat(PHDR_VAR var, PHDR_VAR sec, char op)
{
	/*
	if((hdr_inter_var_gen_type(tvar)!=hdr_ivt_numeric)&& (hdr_inter_var_gen_type(tvar) != hdr_ivt_string)&&
					(tvar->type != hvt_complex_string)&& (tvar->type != hvt_complex_string_resolve))
				{
					if (tvar->type == hvt_undefined) printf("The variable : '%s' is undefined and cannot take part in calculations or concatenations.\n", tvar->name->stringa);
					else
					printf("The variable : '%s' is not of a valid type (numeric or string) for the calculation or concatenation.\n", tvar->name->stringa);
					return NULL;
				}
	*/

	if (var->type == hvt_undefined)
	{
		printf("The '%s' is undefined and cannot take part in calculations or concatenations.\n",var->name->stringa);
		return false;
	}

	if (sec->type == hvt_undefined)
	{
		printf("The '%s' is undefined and cannot take part in calculations or concatenations.\n", sec->name->stringa);
		return false;
	}

	if (hdr_inter_var_gen_type(var) != hdr_inter_var_gen_type(sec))
	{
		printf("The types for concatenation / calculation has to be the same. In this expression there is a type missmatch. Check if you try to concatenate numbers with strings with out type cast.\n");
		return false;
	}

	switch (op)
	{
		case '+':
		{
			if (hdr_inter_var_gen_type(var) == hdr_ivt_numeric)
			{
				return hdr_inter_add_var(var, sec, 0);
			}
			else
				if (hdr_inter_var_gen_type(var) == hdr_ivt_string)
				{

					if(var->type != hvt_codepoint)
					{
						 if(sec->type == hvt_codepoint)
						 {
							 if((var->type == hvt_simple_string)||(var->type == hvt_simple_string_bcktck))
							 {
							   char buffer[5] ;
							   dxConvertUint32ToUTF8(buffer, sec->integer);
							   PDX_STRING tstr = dx_string_createU(NULL,buffer); 
							   PDX_STRING nstr = dx_string_concat((PDX_STRING)var->obj, tstr);
							   dx_string_free(tstr);
							   dx_string_free((PDX_STRING)var->obj); /*the old string is invalid now free it*/
							   /*the new string will be set*/
							   hdr_var_set_obj(var,nstr) ;
							 }
							 else
							 if(var->type == hvt_unicode_string)
							 {
							   /*
							    the unicode string has its data to other fields so we must handle it specialy
							   */
							   dx_concat_codepoint((PDX_STRING)var->obj,sec->integer);
							 }
						 }
						 else
						 {
							/*the concatenation is transparent for all the simple and unicode string types */
							PDX_STRING nstr = dx_string_concat((PDX_STRING)var->obj, (PDX_STRING)sec->obj);
							dx_string_free((PDX_STRING)var->obj); /*the old string is invalid now free it*/
							hdr_var_set_obj(var,nstr) ;
						 }
					}
					else
					{	
						 /*two characters are a string!*/
						 var->type = hvt_simple_string ;
						 if(sec->type == hvt_codepoint)
						 {
						   
						   char buffer[5] ;
						   dxConvertUint32ToUTF8(buffer, var->integer);
						   char buffer2[5] ;
						   dxConvertUint32ToUTF8(buffer, sec->integer);

						   PDX_STRING tstr = dx_string_createU(NULL,buffer) ;
						   PDX_STRING tstr2 = dx_string_createU(NULL,buffer2); 
						   PDX_STRING nstr = dx_string_concat(tstr, tstr2);
						   
						   dx_string_free(tstr);
						   /*the new string wiil be set*/
						   hdr_var_set_obj(var,nstr) ;
						 }
						 else
						 {
						   /*the concatenation is transparent for all the simple and unicode string types */
						   char buffer[5] ;
						   dxConvertUint32ToUTF8(buffer, var->integer)    ;
						   PDX_STRING tstr = dx_string_createU(NULL,buffer) ;
						   PDX_STRING nstr = dx_string_concat(tstr, (PDX_STRING)sec->obj);
						   dx_string_free(tstr);
						   hdr_var_set_obj(var,nstr) ;/*the new string will be set*/
						 }
					}
					return true;
				}
				else
				{
					printf("The '+' operator is valid only for numeric or simple string literals and variables.\n");
					return false;
				}
		}break;

		case '-':
		{
			if (hdr_inter_var_gen_type(var) != hdr_ivt_numeric)
			{
				printf("The '-' operator is valid only for numeric literals and variables.\n");
				return false;
			}

			return hdr_inter_subtr_var(var, sec, 0) ;

		}break;

		case '/':
		{
			if (hdr_inter_var_gen_type(var) != hdr_ivt_numeric)
			{
				printf("The '\' operator is valid only for numeric literals and variables.\n");
				return false;
			}

			return hdr_inter_div_var(var, sec, 0)  ;
		}break;

		case '*':
		{
			if (hdr_inter_var_gen_type(var) != hdr_ivt_numeric)
			{
				printf("The '*' operator is valid only for numeric literals and variables.\n");
				return false;
			}

			return hdr_inter_mult_var(var, sec, 0) ;
		}break;

		case '%':
		{
			if (hdr_inter_var_gen_type(var) != hdr_ivt_numeric)
			{
				printf("The '%%' operator is valid only for numeric literals and variables.\n");
				return false;
			}
			return hdr_inter_mod_var(var,sec, 0) ;
		}break;

		case '^':
		{
			if (hdr_inter_var_gen_type(var) != hdr_ivt_numeric)
			{
				printf("The '^' operator is valid only for numeric literals and variables.\n");
				return false;
			}
			return hdr_inter_expon_var(var, sec, 0) ;

		}break;

		default:
		{
			printf("Unknown operator : %c\n",op);
			return false;
		}
	}

	return true;
}

PHDR_COMPLEX_TOKEN hdr_inter_return_list_token(PHDR_TOKENS_LIST tlist, DXLONG64 pos)
{
	if (pos < 0) return NULL;
	DXLONG64  indx = 0;
	if (pos > (tlist->count - 1))
		return NULL;
	else
	{
		PDXL_NODE node = tlist->start;
		while (node != NULL)
		{
			if (indx == pos) return (PHDR_COMPLEX_TOKEN)node->object->obj;
			indx++;
			node = node->right ;
		}
	}

	return NULL;
}

char* hdr_inter_return_variable_type(enum hdr_var_type vtype)
{
	switch (vtype)
	{
	case hvt_undefined	: return "undefined"; break;
	case hvt_float		: return "float"; break;
	case hvt_integer		: return "integer"; break;
	case hvt_bool		: return "bool"; break;
	case hvt_list		: return "list"; break;
	case hvt_string_list	: return "stringlist"; break;
	case hvt_string_list_sort: return "sorted_stringlist"; break;
	case hvt_int_list_sort	: return "sorted integer list"; break;
	case hvt_float_list_sort	: return "sorted float list"; break;
	case hvt_fast_list		: return "fast list"; break;
	case hvt_http			: return "http"; break;
	case hvt_database		: return "database"; break;
	case hvt_dataset		: return "dataset"; break;
	case hvt_data_row       : return "datarow"; break ;
	case hvt_null			: return "null";break ;
	case hvt_tcp_socket_client : return "tcp client socket"; break;
	case hvt_tcp_socket_server : return "tcp server socket"; break;
	case hvt_ssl_client		  : return "ssl client socket"; break;
	case hvt_ssl_server		  : return "ssl server socket"; break;
	case hvt_bytes			  : return "bytes"; break;
	case hvt_file			  : return "file"; break;
	case hvt_simple_string	  : return "simple string"; break;
	case hvt_simple_string_bcktck  : return "simple string backtick"; break;
	case hvt_complex_string		     : return "complex string"; break; 
	case hvt_complex_string_resolve  : return "complex string resolved"; break;
	case hvt_object					 : return "object"; break;
	case hvt_codepoint				 : return "UTF Codepoint";break ;
	case hvt_unicode_string			 : return "Hydra+ unicode string";break ;
	default						     : return "unknown type. This is a bug"; break;
	}

	return NULL;
}

char* hdr_return_token_type(PHDR_COMPLEX_TOKEN token)
{
	switch (token->type)
	{
	case hdr_tk_uninit:  return "Uninitialized"; break;
	case hdr_tk_ref_brackets: return "Indexed variable"; break;
	case hdr_tk_multipart: return "Multipart token/variable"; break;
	case hdr_tk_index:return "Index"; break;
	case hdr_tk_simple:return "Simple token"; break;
	case hdr_tk_simple_param:return "Token with param"; break;
	case hdr_tk_function:return "Function"; break;
	case hdr_tk_literal:return "Literal"; break;
	case hdr_tk_operator:return "Operator"; break;
	case hdr_tk_expression:return "Expression"; break;
	case hdr_tk_bool_expression:return "Boolean expression"; break;
	case hdr_tk_comp_expression:return "Comparison expression"; break;
	case hdr_tk_variable:return "Variable"; break;
	default: "No recognizable. Most probably a bug!"; break;
	}

	return "How the hell this was returned ?";
}


bool hdr_inter_fast_two(PDX_STRING str, char* str2)
{
	/*i hope this is faster than the alternative for the two character operators :'(*/

	if (str->len != 2) return false;

	if ((str->stringa[0] == str2[0]) && (str->stringa[1] == str2[1])) return true;

	return false;
}

bool hdr_inter_fast_str(PDX_STRING str, char* str2,int str2len)
{
	/*i hope this is faster than the alternative for bigger keywords :'(*/

	if (str->len != str2len) return false;

	for (int i = 0; i < str2len; i++)
	{
		if (str->stringa[i] != str2[i]) return false;
	}

	return true;
}



bool hdr_inter_check_index_type(PHDR_VAR var)
{
	/*returns true if the variable is indexable*/
	switch (var->type)
	{
	 case hvt_object:
	 case hvt_list :
	 case hvt_string_list:
	 case hvt_string_list_sort:
	 case hvt_int_list_sort:
	 case hvt_float_list_sort:
	 case hvt_fast_list:
	 case hvt_dataset:
	 case hvt_data_row:
	 case hvt_bytes:
	 case hvt_unicode_string:
	 case hvt_simple_string:
	 case hvt_simple_string_bcktck: return true;
	 default: return false;
	}

	return false;
}

DXLONG64 hdr_inter_ret_integer(PHDR_VAR num,bool *type_error)
{
  *type_error = false ;
  if(num->type == hvt_integer)
  {
	  return num->integer ;
  }
  else 
	  if(num->type == hvt_float)
	  {
		  return num->real ;
	  }
	  else 
	  {
		*type_error = true ;
		return -1 ;
	  }

  return -1 ;
}

double hdr_inter_ret_real(PHDR_VAR num,bool *type_error)
{
  *type_error = false ;
  if(num->type == hvt_integer)
  {
	  return num->integer ;
  }
  else 
	  if(num->type == hvt_float)
	  {
		  return num->real ;
	  }
	  else 
	  {
		*type_error = true ;
		return -1 ;
	  }

  return -1 ;
}

PDX_STRING hdr_inter_ret_string(PHDR_VAR str,bool *type_error)
{
  *type_error = false ;
  if((str->type == hvt_simple_string)||(str->type == hvt_simple_string_bcktck))
  {
	  return (PDX_STRING)str->obj ;
  }
	  else 
	  {
		*type_error = true ;
		return NULL ;
	  }

  return NULL ;
}

bool hdr_inter_ret_bool(PHDR_VAR var,bool *type_error)
{
  *type_error = false ;
  if(var->type == hvt_bool)
  {
	  return (bool)var->integer ;
  }
  else 
	  {
		*type_error = true ;
		return false ;
	  }

  return false ;
}

bool hdr_inter_bool_to_int(bool val)
{
  if(val  == false) return 0 ;
  return 1 ;
}

FILE *hdr_inter_ret_file(PHDR_VAR var,bool *type_error)
{
  *type_error = false ;
  if(var->type == hvt_file)
  {
	  return (FILE*)var->obj ;
  }
  else 
	  {
		*type_error = true ;
		return NULL ;
	  }

  return NULL ;
}

PHDR_BYTES hdr_inter_ret_bytes(PHDR_VAR var,bool *type_error)
{
  *type_error = false ;
  if(var->type == hvt_bytes)
  {
	  return (PHDR_BYTES)var->obj ;
  }
  else 
	  {
		*type_error = true ;
		return NULL ;
	  }

  return NULL ;
}


DXLONG64 hdr_inter_ret_socket(PHDR_VAR var,bool *type_error)
{
  *type_error = false ;
  if((var->type == hvt_tcp_socket_client)||(var->type == hvt_tcp_socket_server))
  {
	  return var->integer ;
  }
  else 
	  {
		*type_error = true ;
		return -1 ;
	  }

  return -1 ;
}

PDX_STRINGLIST hdr_inter_ret_stringlist(PHDR_VAR var,bool *type_error)
{
  *type_error = false ;
  if(var->type == hvt_string_list)
  {
	  return (PDX_STRINGLIST)var->obj ;
  }
  else 
	  {
		*type_error = true ;
		return NULL ;
	  }

  return NULL ;
}

/************ LISTS FUNCTIONS ********************/


PDX_LIST hdr_simple_list_free(PDX_LIST list)
{
	 /*the list HAS to have variables as objects, returns void*/
	 if(list == NULL) return NULL ;
	 PDXL_NODE node = list->start ;
	 while(node != NULL)
	 {
		PDXL_OBJECT obj = node->object ;
		dx_string_free(obj->key)	   ;
		PHDR_VAR    var = (PHDR_VAR)node->object->obj ;
		hdr_var_free(var) ;
		free(obj)		  ;
		node = node->right ;
	 }

	 dx_list_delete_list(list) ;

	 return NULL ;
}



/********************* SPECIAL STRING FUNCTIONS**************************/

void hdr_inter_hlp_concatenate(PDX_STRING dest,char *s1,char *s2,char *s3)
{
    PDX_STRING temps  = dx_string_createU(NULL,s1)   ;
	PDX_STRING temps2 = dx_string_concat(dest,temps) ;
	dx_string_createU(dest,temps2->stringa)                   ;
	dx_string_free(temps) ;
	dx_string_free(temps2) ;

    PDX_STRING tmp = dx_string_createU(NULL,s2) ;
    PDX_STRING str = dx_string_concat(dest,tmp) ;
    dx_string_createU(tmp,str->stringa)         ;
    dx_string_createU(str,s3)                   ;
    PDX_STRING str2 = dx_string_concat(tmp,str) ;
    dx_string_createU(dest,str2->stringa)       ;

    dx_string_free(tmp) ;
    dx_string_free(str) ;
    dx_string_free(str2);

    return ;
}

void hdr_inter_hlp_concatenate_and_set(PDX_STRING dest,char *s1,char *s2,char *s3)
{
    dx_string_createU(dest,s1);
    PDX_STRING tmp = dx_string_createU(NULL,s2) ;
    PDX_STRING str = dx_string_concat(dest,tmp) ;
    dx_string_createU(tmp,str->stringa)         ;
    dx_string_createU(str,s3)                   ;
    PDX_STRING str2 = dx_string_concat(tmp,str) ;
    dx_string_createU(dest,str2->stringa)       ;

    dx_string_free(tmp) ;
    dx_string_free(str) ;
    dx_string_free(str2);

    return ;
}

void hdr_inter_hlp_concatenate_and_set5(PDX_STRING dest,char *s1,char *s2,char *s3,char *s4,char *s5)
{
    dx_string_createU(dest,s1);
    PDX_STRING tmp = dx_string_createU(NULL,s2) ;
    PDX_STRING str = dx_string_concat(dest,tmp) ;
    dx_string_createU(tmp,str->stringa)         ;
    dx_string_createU(str,s3)                   ;
    PDX_STRING str2 = dx_string_concat(tmp,str) ;
    dest = dx_string_createU(dest,str2->stringa);
	tmp = dx_string_createU(tmp,s4)				;

	str = dx_string_free(str);
	str = dx_string_concat(dest,tmp)			;				
	dest = dx_string_createU(dest,str->stringa) ;					
	dx_string_free(str)							;

	tmp = dx_string_createU(tmp,s5)			    ;
	str = dx_string_concat(dest,tmp)			;		
	dest = dx_string_createU(dest,str->stringa) ;

    dx_string_free(tmp) ;
    dx_string_free(str) ;
    dx_string_free(str2);

    return ;
}






/*****************DEBUG**********************/
PDX_STRING hdr_inter_return_var_val_str(PHDR_VAR var)
{
	
	switch (hdr_inter_var_gen_type(var))
	{
	  case hdr_ivt_string: { printf("PLEASE IMPLEMENT THE STRING FOR THE DEBUG\n"); }break;
	  case hdr_ivt_numeric: 
	  {
		  /*return the number*/
		  if (var->type == hvt_integer) return dx_IntToStr(var->integer);
		  else
			  if (var->type == hvt_float) return dx_FloatToStr(var->real, 8);
	  }break;
	  default: { printf("COMPLEX DATATYPE\n"); }
	}

	return NULL ;
}









