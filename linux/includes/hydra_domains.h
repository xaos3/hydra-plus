/*
  This file has the code for the native domains of the Hydra+ .

  Nikos Mourgis deus-ex.gr 2024

  Live Long and Prosper.
 
*/

/*

 NOTES : 

 An indexed multipart variable has the indexes in its expression and the parts in the next_part members.
 so its not so intuitive , but first you resolve the variable return in the expression and 
 then apply the parts in the resulting variable

*/

/********************** FUNCTIONS ***************************/

/**************** STRINGS **********************************/

bool HDR_DOMAIN_STRING_CONCAT_CONCAT(PHDR_VAR base_str, PHDR_VAR str)
{
	/*
	  The function adds the str to the base_str.
	  The base_str MUST be a hvt_complex_string and the str can be any string type
	  return false if all is ok an true if an error exists. I know. I fcked up earlier
	  and now i want to be consistent
	*/

	if (hdr_inter_var_gen_type(str) == hdr_ivt_string)
	{
		/*add the simple string to a new element in the list*/
		PDX_STRINGLIST list  = (PDX_STRINGLIST)base_str->obj ;
		PDX_STRING	   dxstr = (PDX_STRING)str->obj      ;
		//PDX_STRING	   nstr  = dx_string_createU(NULL,dxstr->stringa)  ;
		dx_stringlist_load_text_ex(list , dxstr , 512) ;
		//dx_stringlist_add_string(list, nstr) ;
		return false; /*all ok*/
	}
	else
		if ((str->type == hvt_complex_string) || (str->type == hvt_complex_string_resolve))
		{
			if (str->type == hvt_complex_string)
			{
				PDX_STRINGLIST list = (PDX_STRINGLIST)base_str->obj;
				dx_stringlist_concat(list, (PDX_STRINGLIST)str->obj);
			}
			else
			{
				printf("The concatenation of a [var_expand] string is not allowed. If you want to retrieve the resolved string, use the appropriate domain functions.\n");
				return true;
			}
		}
		else
		{
			printf("The variable '%s' type '%s' is not one of the supported types for concatenation with the complex string.\n",
				str->name->stringa, hdr_inter_return_variable_type(str->type));
			return true;
		}

	return false;
}



/******************** DOMAINS *****************************/

PHDR_VAR HDR_DOMAIN_CLASS(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token, bool* error)
{
	/*
	 handle the class domain. The class domain is the domain that is used for all the user objects
	*/
	*error = false ; 
	if ((token->type != hdr_tk_multipart) || (token->expression != NULL))
	{
		*error = true;
		printf("The 'Class' domain cannot have an index and must be followed by a dot (.) and the appropriate object class.\n");
		return NULL;
	}

	token = token->next_part;
	/*search for the object class*/

	PHDR_OBJECT_CLASS oclass =hdr_object_class_list_find(inter->object_classes, token->ID) ;
	if(oclass == NULL)
	{
		*error = true;
		printf("The '%s' object class was not found in the script.\n",token->ID->stringa);
		return NULL;
	}

	if (token->type != hdr_tk_simple) goto not_supported;

	/*create a new variable that is pointing to an object*/
	PHDR_VAR result				= NULL;
	PHDR_OBJECT_INSTANCE obj	= hdr_object_class_create_instance(inter,oclass) ; 
	result						= hdr_var_create(obj,"",hvf_temporary_ref,inter->current_block) ; 
	result->type				= hvt_object ;

	if(result == NULL)
	{
	 hdr_object_class_free(obj) ;
	 printf("The variable was not created. Memory error.\n");
	 goto error_exit;
	}

	/*now a little trick, we will run the object initialization code*/
	/*save the state as the current instruction and block  will be changed in the interpreter*/
	PHDR_INTER_STORE saved_state = hdr_inter_store(inter);
	if (saved_state == NULL)
	{
		printf("Fatal Error -> Line : %d INTERNAL ERROR ON SAVED_STATE: MEMORY NOT ALLOCATED.\n", inter->current_instr->instr->line);
		*error = true ;
		return NULL;
	}

	hdr_inter_set_block(inter, obj->code);

	enum exec_state exec_state = hdr_inter_execute_instructions(inter) ;
	if ( exec_state == exec_state_error)
	{
		printf("Fatal Error -> An error occured while the object [%s] was initialized. See the above messages.\n",obj->class_name->stringa);
		*error = true ;
		/*restore interpreter state*/
		saved_state = hdr_inter_restore(inter, saved_state);
		return NULL ;
		goto error_exit ;
	}

	saved_state = hdr_inter_restore(inter, saved_state);

	return result ;
	
	not_supported:
		printf("The object class '%s' is malformed. Check for invalid characters.\n", token->ID->stringa);
	error_exit:
		*error = true;
		return NULL;

}



PHDR_VAR HDR_DOMAIN_STRING(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token, bool* error)
{
		if ((token->type != hdr_tk_multipart) || (token->expression != NULL))
		{
			*error = true;
			printf("The 'String' domain cannot have an index and must be followed by a dot (.) and the appropriate function.\n");
			return NULL;
		}

		token = token->next_part;
		if (hdr_inter_fast_str(token->ID, "Create", 6) == true)
		{
			if (token->type != hdr_tk_function) goto not_supported;

			PHDR_VAR result = NULL;
			PHDR_COMPLEX_TOKEN temp_token = hdr_complex_token_create(NULL, "!S__CR");
			temp_token->parameters = token->parameters;
			if (hdr_inter_handle_function(inter, temp_token,NULL, &result) == true) goto  error_exit;
			temp_token->parameters = NULL;/*clear this the parameters belongs to the token */
			hdr_complex_token_free(temp_token); /*used only for the functions subsystem, is safe to free it*/
			return result;

		}

	not_supported:
		printf("The command '%s' is not one of the supported commands of the 'String' domain.\n", token->ID->stringa);
	error_exit:
		*error = true;
		return NULL;
}


PHDR_VAR HDR_DOMAIN_LIST(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token, bool* error)
{
		if ((token->type != hdr_tk_multipart) || (token->expression != NULL))
		{
			*error = true;
			printf("The 'List' domain cannot have an index and must be followed by a dot (.) and the appropriate function.\n");
			return NULL;
		}

		token = token->next_part;

		if (hdr_inter_fast_str(token->ID, "CreateList", 10) == true)
		{
			if (token->type != hdr_tk_function) goto not_supported;

			PHDR_VAR result = NULL;
			PHDR_COMPLEX_TOKEN temp_token = hdr_complex_token_create(NULL, "!L__CR");
			temp_token->parameters = token->parameters;
			if (hdr_inter_handle_function(inter, temp_token,NULL, &result) == true) goto  error_exit;
			temp_token->parameters = NULL;/*clear this the parameters belongs to the token */
			hdr_complex_token_free(temp_token); /*used only for the functions subsystem, is safe to free it*/
			return result;

		}

		if (hdr_inter_fast_str(token->ID, "CreateStringList", 16) == true)
		{
			if (token->type != hdr_tk_function) goto not_supported;

			PHDR_VAR result = NULL;
			PHDR_COMPLEX_TOKEN temp_token = hdr_complex_token_create(NULL, "!LS_CR");
			temp_token->parameters = token->parameters;
			if (hdr_inter_handle_function(inter, temp_token,NULL, &result) == true) goto  error_exit;
			temp_token->parameters = NULL;/*clear this the parameters belongs to the token */
			hdr_complex_token_free(temp_token); /*used only for the functions subsystem, is safe to free it*/
			return result;

		}

		if (hdr_inter_fast_str(token->ID, "CreateFastList", 14) == true)
		{
			if (token->type != hdr_tk_function) goto not_supported;

			PHDR_VAR result = NULL;
			PHDR_COMPLEX_TOKEN temp_token = hdr_complex_token_create(NULL, "!LF_CR");
			temp_token->parameters = token->parameters;
			if (hdr_inter_handle_function(inter, temp_token,NULL, &result) == true) goto  error_exit;
			temp_token->parameters = NULL;/*clear this the parameters belongs to the token */
			hdr_complex_token_free(temp_token); /*used only for the functions subsystem, is safe to free it*/
			return result;

		}

	not_supported:
		printf("The command '%s' is not one of the supported commands of the 'List' domain.\n", token->ID->stringa);
	error_exit:
		*error = true;
		return NULL;

}


PHDR_VAR HDR_DOMAIN_BYTES(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token, bool *error)
{
	if ((token->type != hdr_tk_multipart) || (token->expression != NULL))
		{
			*error = true;
			printf("The 'Bytes' domain cannot have an index and must be followed by a dot (.) and the appropriate function.\n");
			return NULL;
		}

		token = token->next_part;

		if (hdr_inter_fast_str(token->ID, "Create", 6) == true)
		{
			if (token->type != hdr_tk_function) goto not_supported;

			PHDR_VAR result = NULL;
			PHDR_COMPLEX_TOKEN temp_token = hdr_complex_token_create(NULL, "!B__CR");
			temp_token->parameters = token->parameters;
			if (hdr_inter_handle_function(inter, temp_token,NULL, &result) == true) goto  error_exit;
			temp_token->parameters = NULL;/*clear this the parameters belongs to the token */
			hdr_complex_token_free(temp_token); /*used only for the functions subsystem, is safe to free it*/
			return result;

		}


 	not_supported:
		printf("The command '%s' is not one of the supported commands of the 'Bytes' domain.\n", token->ID->stringa);
	error_exit:
		*error = true;
		return NULL;

}


PHDR_VAR HDR_DOMAIN_HYDRA(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token, bool *error)
{
	if ((token->type != hdr_tk_multipart) || (token->expression != NULL))
		{
			*error = true;
			printf("The 'Hydra' domain cannot have an index and must be followed by a dot (.) and the appropriate function.\n");
			return NULL;
		}

		token = token->next_part;

		if (hdr_inter_fast_str(token->ID, "ScriptPath", 10) == true)
		{
			if (token->type != hdr_tk_function) goto not_supported;

			PHDR_VAR result = NULL;
			PHDR_COMPLEX_TOKEN temp_token = hdr_complex_token_create(NULL, "!HS___");
			temp_token->parameters = token->parameters;
			if (hdr_inter_handle_function(inter, temp_token,NULL, &result) == true) goto  error_exit;
			temp_token->parameters = NULL;/*clear this the parameters belongs to the token */
			hdr_complex_token_free(temp_token); /*used only for the functions subsystem, is safe to free it*/
			return result;

		}

		if (hdr_inter_fast_str(token->ID, "Path", 4) == true)
		{
			if (token->type != hdr_tk_function) goto not_supported;

			PHDR_VAR result = NULL;
			PHDR_COMPLEX_TOKEN temp_token = hdr_complex_token_create(NULL, "!HP___");
			temp_token->parameters = token->parameters;
			if (hdr_inter_handle_function(inter, temp_token,NULL, &result) == true) goto  error_exit;
			temp_token->parameters = NULL;/*clear this the parameters belongs to the token */
			hdr_complex_token_free(temp_token); /*used only for the functions subsystem, is safe to free it*/
			return result;

		}

		if (hdr_inter_fast_str(token->ID, "ScriptName", 10) == true)
		{
			if (token->type != hdr_tk_function) goto not_supported;

			PHDR_VAR result = NULL;
			PHDR_COMPLEX_TOKEN temp_token = hdr_complex_token_create(NULL, "!HN___");
			temp_token->parameters = token->parameters;
			if (hdr_inter_handle_function(inter, temp_token,NULL, &result) == true) goto  error_exit;
			temp_token->parameters = NULL;/*clear this the parameters belongs to the token */
			hdr_complex_token_free(temp_token); /*used only for the functions subsystem, is safe to free it*/
			return result;

		}


 	not_supported:
		printf("The command '%s' is not one of the supported commands of the 'Hydra' domain.\n", token->ID->stringa);
	error_exit:
		*error = true;
		return NULL;

}


PHDR_VAR HDR_DOMAIN_FILE(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token, bool *error)
{
	if ((token->type != hdr_tk_multipart) || (token->expression != NULL))
		{
			*error = true;
			printf("The 'FILE' domain cannot have an index and must be followed by a dot (.) and the appropriate function.\n");
			return NULL;
		}

		token = token->next_part;

		if (hdr_inter_fast_str(token->ID, "Open", 4) == true)
		{
			if (token->type != hdr_tk_function) goto not_supported;

			PHDR_VAR result = NULL;
			PHDR_COMPLEX_TOKEN temp_token = hdr_complex_token_create(NULL, "!F__CR");
			temp_token->parameters = token->parameters;
			if (hdr_inter_handle_function(inter, temp_token,NULL, &result) == true) goto  error_exit;
			temp_token->parameters = NULL;/*clear this the parameters belongs to the token */
			hdr_complex_token_free(temp_token); /*used only for the functions subsystem, is safe to free it*/
			return result;

		}


		
 	not_supported:
		printf("The command '%s' is not one of the supported commands of the 'File' domain.\n", token->ID->stringa);
	error_exit:
		*error = true;
		return NULL;

}

PHDR_VAR HDR_DOMAIN_SOCKET(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token, bool *error)
{
	if ((token->type != hdr_tk_multipart) || (token->expression != NULL))
		{
			*error = true;
			printf("The 'Socket' domain cannot have an index and must be followed by a dot (.) and the appropriate function.\n");
			return NULL;
		}

		token = token->next_part;

		if (hdr_inter_fast_str(token->ID, "ConnectTCP", 10) == true)
		{
			if (token->type != hdr_tk_function) goto not_supported;

			PHDR_VAR result = NULL;
			PHDR_COMPLEX_TOKEN temp_token = hdr_complex_token_create(NULL, "!NC_CR");
			temp_token->parameters = token->parameters;
			if (hdr_inter_handle_function(inter, temp_token,NULL, &result) == true) goto  error_exit;
			temp_token->parameters = NULL;/*clear this the parameters belongs to the token */
			hdr_complex_token_free(temp_token); /*used only for the functions subsystem, is safe to free it*/
			return result;

		}

		if (hdr_inter_fast_str(token->ID, "TCPServer", 9) == true)
		{
			if (token->type != hdr_tk_function) goto not_supported;

			PHDR_VAR result = NULL;
			PHDR_COMPLEX_TOKEN temp_token = hdr_complex_token_create(NULL, "!NS_CR");
			temp_token->parameters = token->parameters;
			if (hdr_inter_handle_function(inter, temp_token,NULL, &result) == true) goto  error_exit;
			temp_token->parameters = NULL;/*clear this the parameters belongs to the token */
			hdr_complex_token_free(temp_token); /*used only for the functions subsystem, is safe to free it*/
			return result;

		}

 	not_supported:
		printf("The command '%s' is not one of the supported commands of the 'Socket' domain.\n", token->ID->stringa);
	error_exit:
		*error = true;
		return NULL;

}

PHDR_VAR HDR_DOMAIN_SSL(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token, bool *error)
{
	if ((token->type != hdr_tk_multipart) || (token->expression != NULL))
		{
			*error = true;
			printf("The 'SSL' domain cannot have an index and must be followed by a dot (.) and the appropriate function.\n");
			return NULL;
		}

		token = token->next_part;

		if (hdr_inter_fast_str(token->ID, "Connect", 7) == true)
		{
			if (token->type != hdr_tk_function) goto not_supported;

			PHDR_VAR result = NULL;
			PHDR_COMPLEX_TOKEN temp_token = hdr_complex_token_create(NULL, "!C__CR");
			temp_token->parameters = token->parameters;
			if (hdr_inter_handle_function(inter, temp_token,NULL, &result) == true) goto  error_exit;
			temp_token->parameters = NULL;/*clear this the parameters belongs to the token */
			hdr_complex_token_free(temp_token); /*used only for the functions subsystem, is safe to free it*/
			return result;

		}

		if (hdr_inter_fast_str(token->ID, "Server", 6) == true)
		{
			if (token->type != hdr_tk_function) goto not_supported;

			PHDR_VAR result = NULL;
			PHDR_COMPLEX_TOKEN temp_token = hdr_complex_token_create(NULL, "!CS_CR");
			temp_token->parameters = token->parameters;
			if (hdr_inter_handle_function(inter, temp_token,NULL, &result) == true) goto  error_exit;
			temp_token->parameters = NULL;/*clear this the parameters belongs to the token */
			hdr_complex_token_free(temp_token); /*used only for the functions subsystem, is safe to free it*/
			return result;

		}


 	not_supported:
		printf("The command '%s' is not one of the supported commands of the 'SSL' domain.\n", token->ID->stringa);
	error_exit:
		*error = true;
		return NULL;

}

PHDR_VAR HDR_DOMAIN_DATABASE(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token, bool *error)
{
	if ((token->type != hdr_tk_multipart) || (token->expression != NULL))
		{
			*error = true;
			printf("The 'Database' domain cannot have an index and must be followed by a dot (.) and the appropriate function.\n");
			return NULL;
		}

		token = token->next_part;

		if (hdr_inter_fast_str(token->ID, "SQLite", 6) == true)
		{
			if (token->type != hdr_tk_function) goto not_supported;

			PHDR_VAR result = NULL;
			PHDR_COMPLEX_TOKEN temp_token = hdr_complex_token_create(NULL, "!DL_CR");
			temp_token->parameters = token->parameters;
			if (hdr_inter_handle_function(inter, temp_token,NULL, &result) == true) goto  error_exit;
			temp_token->parameters = NULL;/*clear this the parameters belongs to the token */
			hdr_complex_token_free(temp_token); /*used only for the functions subsystem, is safe to free it*/
			return result;
		}

		if (hdr_inter_fast_str(token->ID, "MariaDB", 7) == true)
		{
			if (token->type != hdr_tk_function) goto not_supported;

			PHDR_VAR result = NULL;
			PHDR_COMPLEX_TOKEN temp_token = hdr_complex_token_create(NULL, "!DM_CR");
			temp_token->parameters = token->parameters;
			if (hdr_inter_handle_function(inter, temp_token,NULL, &result) == true) goto  error_exit;
			temp_token->parameters = NULL;/*clear this the parameters belongs to the token */
			hdr_complex_token_free(temp_token); /*used only for the functions subsystem, is safe to free it*/
			return result;
		}

		if (hdr_inter_fast_str(token->ID, "ODBC", 4) == true)
		{
			if (token->type != hdr_tk_function) goto not_supported;

			PHDR_VAR result = NULL;
			PHDR_COMPLEX_TOKEN temp_token = hdr_complex_token_create(NULL, "!DO_CR");
			temp_token->parameters = token->parameters;
			if (hdr_inter_handle_function(inter, temp_token,NULL, &result) == true) goto  error_exit;
			temp_token->parameters = NULL;/*clear this the parameters belongs to the token */
			hdr_complex_token_free(temp_token); /*used only for the functions subsystem, is safe to free it*/
			return result;
		}

		

 	not_supported:
		printf("The command '%s' is not one of the supported commands of the 'Database' domain.\n", token->ID->stringa);
	error_exit:
		*error = true;
		return NULL;

}

/*********************************************************/



PHDR_VAR hdr_inter_handle_domain(PHDR_INTERPRETER inter ,PHDR_COMPLEX_TOKEN token, bool* error)
{
	/*Classes*/
	if (hdr_inter_fast_str(token->ID, "Class", 5) == true)
	{
		
		PHDR_VAR var = HDR_DOMAIN_CLASS(inter, token, error);
		if (*error == true) return NULL;
		return var;
	}
	/*String*/
	if (hdr_inter_fast_str(token->ID, "String", 6) == true)
	{
		
		PHDR_VAR var = HDR_DOMAIN_STRING(inter, token, error);
		if (*error == true) return NULL;
		return var;
	}

	/*Lists*/
	if (hdr_inter_fast_str(token->ID, "List", 4) == true)
	{
		
		PHDR_VAR var = HDR_DOMAIN_LIST(inter, token, error);
		if (*error == true) return NULL;
		return var;
	}

	/*Bytes*/
	if (hdr_inter_fast_str(token->ID, "Bytes", 5) == true)
	{
		
		PHDR_VAR var = HDR_DOMAIN_BYTES(inter, token, error);
		if (*error == true) return NULL;
		return var;
	}

	/*Hydra*/
	if (hdr_inter_fast_str(token->ID, "Hydra", 5) == true)
	{
		
		PHDR_VAR var = HDR_DOMAIN_HYDRA(inter, token, error);
		if (*error == true) return NULL;
		return var;
	}

	/*Files*/
	if (hdr_inter_fast_str(token->ID, "File", 4) == true)
	{
		
		PHDR_VAR var = HDR_DOMAIN_FILE(inter, token, error);
		if (*error == true) return NULL;
		return var;
	}
	/*sockets*/
	if (hdr_inter_fast_str(token->ID, "Socket", 6) == true)
	{
		
		PHDR_VAR var = HDR_DOMAIN_SOCKET(inter, token, error);
		if (*error == true) return NULL;
		return var;
	}
	/*SSL*/
	if (hdr_inter_fast_str(token->ID, "SSL", 3) == true)
	{
		
		PHDR_VAR var = HDR_DOMAIN_SSL(inter, token, error);
		if (*error == true) return NULL;
		return var;
	}
	/*databases*/
	if (hdr_inter_fast_str(token->ID, "Database", 8) == true)
	{
		
		PHDR_VAR var = HDR_DOMAIN_DATABASE(inter, token, error);
		if (*error == true) return NULL;
		return var;
	}

	printf("The domain [%s] does not exists.\n",token->ID->stringa) ;
	*error = true; 
	return NULL ;
}
