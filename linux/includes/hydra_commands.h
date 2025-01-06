/*
 This file has the implementation of all the arbitrary commands that the hydra+ can use.
 Edit this file to add any functionality with the syntax of 'token ;' or 'token [some expression] ;' 

 Note that your function has to return an exec_state value. See some of the function in there for
 information for the correct format ! 

 Nikos Mourgis deus-ex.gr

 Live Long and Prosper.

*/

bool hdr_get_terminal_var(PHDR_INTERPRETER inter , PHDR_TERMINAL_TOKEN ter_token);

enum exec_state hdr_inter_handle_single_command(PHDR_INTERPRETER inter , PHDR_COMPLEX_TOKEN token);
/*
 This function calls the rignt function based of the token name 
*/

enum exec_state hdr_inter_handle_complex_command(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token,PHDR_EXPRESSION expr);
/*
 This function calls the rignt function based of the token name and executes the expression that accompany it 
*/
bool hydra_set_params(PHDR_BLOCK code , PHDR_VAR_LIST params);
/*
 ***** Special Function for the Hydra+ ********
 The function sets the values of the params (this is retrieved from the command line and is stored in the main hydra object) 
 into the variables with the same name in the code
*/
/*-------------------------------------------------------------------------------------------------*/

/* Simple commands*********************************************************/

enum exec_state hdr_inter_handle_exit(PHDR_INTERPRETER inter) 
{
	if(inter->in_func == false)
	{
		/*stop everything, set a NULL instruction and return*/
		inter->current_instr->instr = NULL ;
		return exec_state_return  ;
	}
	else
	{
		/*the function did not returned any value , for better or worse lets make NULL the coresponding variable*/
		inter->ret_var = NULL ;
		return exec_state_return ;
	}
 
}


enum exec_state hdr_cmd_time_point(PHDR_INTERPRETER inter)
{
	/*print the seconds and milliseconds passed and set the interpreter time value*/
	uint64_t millis = dxGetTickCount() - inter->time_point;
	inter->time_point = dxGetTickCount();
	printf("seconds : %f\n", (double)millis / 1000);
	return exec_state_ok;
}

enum exec_state hdr_cmd_version(PHDR_INTERPRETER inter)
{
	/*print the version and info of hydra*/
	printf("Hydra+ by Nikos Mourgis. Deus-Ex.gr. Version 1.0.0.1 Beta\n");
	return exec_state_ok;
}

enum exec_state hdr_cmd_fill_params(PHDR_INTERPRETER inter)
{
  /*fill the script variables with the command line parameters values*/
  if (hydra_set_params(inter->code,inter->params) == false) return exec_state_error ;
  return exec_state_ok;
}


/***************************************************************************/

/* Complex Comands *********************************************************/
enum exec_state hdr_cmd_dbg(PHDR_INTERPRETER inter, PHDR_EXPRESSION expr)
{
	PHDR_COMPLEX_TOKEN 	 ctoken = hdr_inter_return_list_token(expr->tokens, 0);
	PHDR_VAR			 pvar = hdr_inter_retrieve_var_block(inter, ctoken->ID);
	if (pvar == NULL)
	{
		printf("I do not found the variable . Please be kind with me i'm limited....\n");
		return exec_state_error;
	}

	PDX_STRING str = hdr_inter_return_var_val_str(pvar);
	if (str == NULL)
	{
		printf("PDX_STR IS NULL\n");
		printf("dbg :parent : %d  \n", pvar->block->ID);
	}
	else
	{
		printf("dbg :parent : %d %s = %s \n", pvar->block->ID, ctoken->ID->stringa, str->stringa);
		dx_string_free(str);
	}

	return exec_state_ok;

}

enum exec_state hdr_inter_handle_return(PHDR_INTERPRETER inter, PHDR_EXPRESSION expr)
{
	/*
	 The return command is very special.
	 if the return is called inside a main script , then the interpreter 
	 ends execution imediatelly. If the return returns a string or numeric value,
	 then the interpreter will set the ret_var to the returned value.

	 if the return is called inside a user function, then the function stop execution
	 imediatelly and the interpreter will set the ret_var to the returned value

	*/
	
	bool      error = false ;
	PHDR_VAR  pvar  = hdr_inter_resolve_expr(inter,expr,&error) ;

	if (pvar == NULL)
	{
		printf("The returned expression is NULL. This is not permited. \n");
		return exec_state_error;
	}

	/*return the value*/
		
	inter->ret_var = hdr_var_create(NULL,"",hvf_temporary,NULL) ;
	inter->ret_var->type = pvar->type ;
	switch(pvar->type)
	{
		case  hvt_simple_string:
		case  hvt_simple_string_bcktck:
		{
          PDX_STRING str = dx_string_createU(NULL,((PDX_STRING)pvar->obj)->stringa); 
		  hdr_var_set_obj(inter->ret_var,str) ;
		}break ;
		case  hvt_unicode_string :
		{
			PDX_STRING str = dx_string_createX(NULL,((PDX_STRING)pvar->obj)->stringx); 
		    hdr_var_set_obj(inter->ret_var,str) ;
		}break ;
		case hvt_integer :
		{
			inter->ret_var->integer = pvar->integer ;
		}break ;
		case hvt_float :
		{
			inter->ret_var->real   = pvar->real ;
		}break;
		case hvt_codepoint :
		{
			inter->ret_var->integer = pvar->integer ;
		}break;
		case hvt_bool :
		{
			inter->ret_var->integer = pvar->integer ;
		}break;
		/*all other types are passed as references in the object*/
		default : 
		{
			inter->ret_var->var_ref = hvf_temporary_ref ;
			hdr_var_set_obj(inter->ret_var,pvar->obj) ;

		}
	}

	/*
	  SPECIAL CASE !! the variable that is returning MUST be invalidated in the 
	  function block as it will be handled by the caller and it will be freed by the caller .
	  If we do not do this , then an access violation is guarantied 
	*/
	 PHDR_VAR local_v = (PHDR_VAR)pvar->func_ref_var ;
	 if(local_v != NULL)
	 {
	   /*only for the block variables, if the local_v is NULL then the variable is not a block variable*/
	   if((local_v->type != hvt_simple_string)&&(local_v->type != hvt_simple_string_bcktck)&&(local_v->type != hvt_unicode_string))
	   {
		 /*simple and unicode strings are managed*/
	     if(local_v->obj != NULL)
		 {
		  local_v->obj   = NULL ;
		  local_v->type  = hvt_undefined ;
		 }
	   }


	 }

	/*do the right deallocation of the memory*/
	if ((pvar->var_ref != hvf_dynamic)&&(pvar->var_ref != hvf_block))/*dynamic and block are persistent variables*/
	{
		if(hdr_inter_var_gen_type(pvar) == hdr_ivt_other) pvar->obj = NULL ; /*this is a reference so we will invalidate the object so to not be destroyed*/
		else
		{
			/*free the string if its a string*/
			if((pvar->type == hvt_simple_string)||(pvar->type == hvt_simple_string_bcktck)||(pvar->type == hvt_unicode_string))
			{
			 dx_string_free((PDX_STRING)pvar->obj);
			 hdr_var_release_obj(pvar) ;
			}
			if(pvar->var_ref != hvf_system)/*the system variable MUST not be destroyed*/
			{
			  hdr_var_free(pvar);
			}
		}
	}


	/*
		handle the code flow
	*/
	if(inter->in_func == false)
	{
		/*stop everything, set a NULL instruction and return*/
		inter->current_instr->instr = NULL ;
		return exec_state_return  ;
	}
	else
	{
		/*we do not doing anything special*/
		return exec_state_return ;
	}
	printf("The code cannot branch in this point. What is going on ?\n");
	return exec_state_return  ;
}


enum exec_state hdr_inter_handle_async(PHDR_INTERPRETER inter, PHDR_EXPRESSION expr)
{
	PHDR_COMPLEX_TOKEN 	 ftoken = hdr_inter_return_list_token(expr->tokens, 0);
	PHDR_COMPLEX_TOKEN 	 stoken = hdr_inter_return_list_token(expr->tokens, 1);
	PHDR_COMPLEX_TOKEN 	 ttoken = hdr_inter_return_list_token(expr->tokens, 2);
	
	if((ftoken == NULL)||(stoken == NULL)||(ttoken == NULL))
	{
	  printf("The [async] command failed. Check the syntax , as it may be malformed. Example : async `thread_id`@user_function() ;");
	  return exec_state_error ;
	}

	/*check for syntax errors*/

	if((ftoken->type != hdr_tk_literal)&&(ftoken->type != hdr_tk_variable)) 
	{
	  printf("The [async] command failed. The [thread_id] is not a literal or a variable. Check the syntax , as the may be malformed. Example : async `thread_id`@user_function() ;");
	  return exec_state_error ;
	}

	PDX_STRING thread_id = NULL ;

	if(ftoken->type == hdr_tk_variable)
	{
		PHDR_VAR			 pvar = hdr_inter_retrieve_var_block(inter, ftoken->ID);
		if (pvar == NULL)
		{
			printf("The variable that represents the thread_id was not found.Keep in mind that only simple and not indexed or multipart variables can be used in the [async] command.\n");
			return exec_state_error;
		}

		if((pvar->type!=hvt_simple_string)&&(pvar->type!=hvt_simple_string_bcktck))
		{
		    printf("The variable that represents the thread_id must be a simple string.\n");
			return exec_state_error;
		}

		thread_id = (PDX_STRING)pvar->obj ; 

	}
	else
		{
		  thread_id = (PDX_STRING)ftoken->val->obj ;
		}


	if(stoken->type != hdr_tk_operator)
	{
	   printf("The second element in an [async] command MUST be a '@' character .\n");
	   return exec_state_error;
	} 

	if(stoken->ID->stringa[0]!='@')
	{
	   printf("The second element in an [async] command MUST be a '@' character .\n");
	   return exec_state_error;
	}

	if(ttoken->type != hdr_tk_function)
	{
	   printf("The third element in an [async] command MUST be a user function.\n");
	   return exec_state_error;
	}

	/*syntax cheks out , create a new thread add it to the thread list and start it*/
	/*find the custom function*/
	PHDR_CUSTOM_FUNCTION func		= hdr_custom_functions_list_find(inter,ttoken->ID) ;
	if(func == NULL)
	{
	  printf("Fatal Error -> Line : %d The user function [%s] was not found.\n", 
	  inter->current_instr->instr->line,ttoken->ID->stringa);
	  return exec_state_error ;
	}
	/*check if the parameters are matching in number*/
	if(func->parameters->count != ttoken->parameters->count)
	{
	  printf("Fatal Error -> Line : %d The user function [%s] needs %d parameters. %d was provided.\n", 
	  inter->current_instr->instr->line,ttoken->ID->stringa,func->parameters->count,ttoken->parameters->count);

	  return exec_state_error ;
	}

	return hdr_async_run_function(inter,func,ttoken,thread_id);
}

enum exec_state hdr_inter_handle_go(PHDR_INTERPRETER inter, PHDR_EXPRESSION expr)
{
	PHDR_COMPLEX_TOKEN 	 ftoken = hdr_inter_return_list_token(expr->tokens, 0);
	
	if(ftoken == NULL)
	{
	  printf("The [go] command failed. Check the syntax , as it may be malformed. Example : go cleanup() ;");
	  return exec_state_error ;
	}

	/*check for syntax errors*/

	if(ftoken->type != hdr_tk_function)
	{
	   printf("The token after the [go] command MUST be a user function.\n");
	   return exec_state_error;
	}
	PHDR_VAR result = NULL ;
	enum hdr_user_func_result ret = hdr_run_user_function(inter,ftoken,&result,true) ;
	hdr_var_free(result);/*the value is discarded*/

	if(ret == hdr_user_func_error) return exec_state_error ;
	if(ret == hdr_user_func_not_found) 
	{
	  printf("There is not declared any function with the name [%s]\n",ftoken->ID->stringa);
	  return exec_state_error ;	
	}
	return exec_state_ok ;
	
}

enum exec_state hdr_inter_handle_asfunc(PHDR_INTERPRETER inter, PHDR_EXPRESSION expr)
{

	/*we will resolve the expression as it can be literal string or a string for an expression*/
	bool error = false ;
	PHDR_VAR strfunc = hdr_inter_resolve_expr(inter, expr, &error);

	if((error == true)||(strfunc == NULL))
	{
	  printf("The [asfunc] command failed. Check the syntax , as it may be malformed. Example : asfunc `somefunction($param1,$param2);`\n");
	  return exec_state_error ;
	}

	/*check for syntax errors*/

	if((strfunc->type != hvt_simple_string)&&(strfunc->type != hvt_simple_string_bcktck))
	{
	   printf("The token after the [asfunc] command MUST be a string (or a simple string variable) that describes an already declared function.\n");
	   return exec_state_error;
	}

	/*convert the string to a valid function token*/
	int status = 0 ;
	PHDR_EXPRESSION temp_expr = hdr_expression_create() ; 
	if (hdr_expr_h_add_function(temp_expr, ((PDX_STRING)strfunc->obj)->stringa, 0, &status) == false)
	{
	  hdr_expression_free(temp_expr);
	  printf("The string [%s] %s\n",((PDX_STRING)strfunc->obj)->stringa," was not converted to a function call successfully");
	  return exec_state_error ;	
	}

	PHDR_COMPLEX_TOKEN ftoken = hdr_inter_return_list_token(temp_expr->tokens, 0);
	PHDR_VAR result = NULL ;
	enum hdr_user_func_result ret = hdr_run_user_function(inter,ftoken,&result,true) ;
	hdr_var_free(result);/*the value is discarded*/

	if(ret == hdr_user_func_error) return exec_state_error ;
	if(ret == hdr_user_func_not_found) 
	{
	  printf("There is not declared any function with the name [%s]\n",ftoken->ID->stringa);
	  hdr_expression_free(temp_expr);
	  return exec_state_error ;	
	}
	hdr_expression_free(temp_expr);
	return exec_state_ok ;
}


/**********************************************************************************************/

PHDR_CUSTOM_FUNCTION hdr_detach_custom_function_copy(PHDR_INTERPRETER inter,PDX_STRING funcName,PHDR_OBJECT_CLASS object)
{
  /*copy the function.*/
	PHDR_CUSTOM_FUNCTION nfunc = NULL ;
	if(object == NULL)
	{
		 PDXL_NODE node = inter->loader->functions->start;
		 while( node != NULL )
		 {
				PHDR_INSTRUCTION ifunc = (PHDR_INSTRUCTION)node->object->obj ;
				if(dx_string_native_compare(ifunc->left_side->ID,funcName) == dx_equal)
				{
				  /*create a function instance*/
				  nfunc = hdr_interpreter_copy_function_from_instr(inter,ifunc,object) ; 
				  break ;
				}
				node = node->right ;
		 }
	}
	else
	{
		/*this function belongs to an object*/
		/*find the original object class */
		PHDR_LOADER_OBJ_PROT ob_class = NULL ; 
		PDXL_NODE node = inter->loader->objects->start ;
		while(node!=NULL)
		{
		  PHDR_LOADER_OBJ_PROT obj = node->object->obj ;
		  if(dx_string_native_compare(obj->name, object->class_name) == dx_equal) 
		  {
			  ob_class = obj ;
			  break ;
		  }
		  node = node->right ;
		}

		 /*find the function*/
		 node = ob_class->functions->start;
		 while( node != NULL )
		 {
				PHDR_INSTRUCTION ifunc = (PHDR_INSTRUCTION)node->object->obj ;
				if(dx_string_native_compare(ifunc->left_side->ID,funcName) == dx_equal)
				{
				  /*create a function instance*/
				  nfunc = hdr_interpreter_copy_function_from_instr(inter,ifunc,object) ;
				  break ;
				}
				node = node->right ;
		 }
		 
		 /*an object function has the pecularity that needs as a code block parent the object code block, so we have to set it*/
		 nfunc->code->parent = object->code ;
	}


  return nfunc ;
}



enum hdr_user_func_result hdr_detach_execute(PHDR_INTERPRETER inter,PHDR_COMPLEX_TOKEN token,PHDR_CUSTOM_FUNCTION func,PHDR_VAR* result)
{
	/*check the parameters count*/
	if(func->parameters->count != token->parameters->count)
	{
	  printf("Fatal Error -> Line : %d The user function [%s] needs %d parameters. %d was provided.\n", 
		  inter->current_instr->instr->line,token->ID->stringa,func->parameters->count,token->parameters->count);
	  return hdr_user_func_error ;
	}

	/*we need to setup the parameters of the function and initialize all the variables (not so neccessary but better safe than sorry)*/

	hdr_user_func_code_init_var(func->code->variables) ;
	/*
	   now , the parameters setup is tricky, we will get the name of the first parameter , and we will retrieve the 
	   first actual parameter. We will find the variable with the same name as the parameter in the block's variables
	   and we will set the pointer of the variable hash table of the block to the parameter variable.
	*/

	/* another part that needs special handling is that the parameters can be temporary created 
	   variables AND MUST be FREED before this function exits.
	*/


	/*create a list to store the parameters */
	PDX_LIST params = dx_list_create_list() ;
	if(params == NULL) 
	{
	 
		printf("Fatal Error -> Line : %d INTERNAL ERROR ON PARAM LIST CREATE: MEMORY NOT ALLOCATED.\n", inter->current_instr->instr->line);
		return hdr_user_func_error;
	}

	PDXL_NODE node = func->parameters->start  ;
	int indx = 0 ;
	while(node != NULL)
	{
	  PDX_STRING  pname    = node->object->key ;
	  PHDR_VAR    rparam   = hdr_inter_resolve_extract_var_from_expr(inter, token->parameters, indx);
	  if(rparam == NULL)
	  {
	    printf("Fatal Error -> Line : %d The function parameter [%s] was not found in the scope \n", inter->current_instr->instr->line,pname->stringa);
		return hdr_user_func_error;
	  }
	  
	  /*check the reference type and change it acordinally */
	  if(rparam->var_ref == hvf_temporary_ref) rparam->var_ref = hvf_temporary_ref_user_func ;
	 
	  /***store the rparam for later deallocation*/
	  PDXL_OBJECT obj = dxl_object_create();
	  obj->obj = (void*)rparam ;
	  dx_list_add_node_direct(params,obj) ;
	  /******************************************/
	  PDXL_OBJECT ppointer = dx_HashReturnItem(func->code->variables->list,pname,true); 
	  if(ppointer != NULL) 
	  {
		  /*
			The ppointer has a "true" key that is the actual name of the parameter and an empty obj that we will
		    point to the variable that is the parameter
		  */
		  ppointer->obj = rparam ;
		  /*set the flag that signals that the parameter MUST NOT BE DELETED because is an absolute reference to a variable*/
		  if(rparam->var_ref != hvf_temporary_ref) ppointer->flags = HDR_FUNCTION_PARAM_ABS_REF ;
	  }
	  indx++ ;
	  node = node->right ;
	}

	/*save the state as the current instruction and block  will be changed in the interpreter*/
	PHDR_INTER_STORE saved_state = hdr_inter_store(inter);
	if (saved_state == NULL)
	{
		hdr_user_func_h_release_params(params) ;
		printf("Fatal Error -> Line : %d INTERNAL ERROR ON SAVED_STATE: MEMORY NOT ALLOCATED.\n", inter->current_instr->instr->line);
		return hdr_user_func_error;
	}

	inter->in_func   = true   ;
	inter->curr_func = func   ;
	hdr_inter_set_block(inter, func->code);

	/*execute the function*/
	enum exec_state exec_state = hdr_inter_execute_instructions(inter) ;
	if ( exec_state == exec_state_error)
	{
		hdr_user_func_h_release_params(params) ;
		printf("Fatal Error -> An error occured in the execution of the user custom function [%s]. See the above messages.\n",token->ID->stringa);

		goto error ;
	}
	else 
		if (exec_state == exec_state_return)
		{
			/*
			 ok the function used the return command to stop execution
			 this means one of the following cases :
			 1. The function does not return a value , the return is not followed by any expression.
			 2. The function returns a value and we have to propagate it to the main flow.
			 In actuality all this is handled in the return command. So the only thing we need to do 
			 is to assign the returned value!
			
			*/

			*result			= inter->ret_var ;
			inter->ret_var  = NULL			 ; /*the variable was being create as temporary ref os we need not to free it here it will be freed later*/

		}


	/**************clean up******************************/
	/*release the memory of the temporary variables that may have been created for the parameters*/
	params = hdr_user_func_h_release_params(params) ;	
	/*return the parameters to the NULL status*/
	node = func->parameters->start  ;
	indx = 0 ;
	while(node != NULL)
	{
	  PDX_STRING  pname    = node->object->key ;
	  PDXL_OBJECT ppointer = dx_HashReturnItem(func->code->variables->list,pname,true); 
	  ppointer->obj = NULL ; /*set this to NULL because it was a absolute reference to the parameter variable*/
	  indx++ ;
	  node = node->right ;
	}


	/*restore interpreter state*/
	saved_state = hdr_inter_restore(inter, saved_state);

	return hdr_user_func_success ;


	error:

	/*return the parameters to the NULL status even in the error state 
	  because the interpreter will try to deallocate the memory
	  of the parameters and this will create a runtime error.
	*/
	node = func->parameters->start  ;
	indx = 0 ;
	while(node != NULL)
	{
		PDX_STRING  pname    = node->object->key ;
		PDXL_OBJECT ppointer = dx_HashReturnItem(func->code->variables->list,pname,true); 
		ppointer->obj = NULL ; /*set this to NULL because it was a absolute reference to the parameter variable*/
		indx++ ;
		node = node->right ;
	}

	/*restore interpreter state*/
	saved_state = hdr_inter_restore(inter, saved_state);
	return hdr_user_func_error ;
}

/************** OBSOLETE ************************************/
void hdr_detach_invalidate_var_list(PHDR_VAR_LIST var_list)
{
    PDX_LIST      *buckets = var_list->list->buckets      ;
	for(uint32_t i = 0; i < var_list->list->length;i++)
	{
		
		PDXL_NODE node = var_list->list->buckets[i]->start ;
		while(node != NULL)
		{
			PDXL_OBJECT obj = node->object ;
			PHDR_VAR var =(PHDR_VAR) obj->obj ;
			if(var != NULL)
			{
				if((var->type == hvt_simple_string)||(var->type == hvt_simple_string_bcktck))
					{
					  /*this will be handled in the function destruction*/
					}
					else
						{
							hdr_var_release_obj(var) ;
						}
			}
			node = node->right ;
		}
		
	}


}


void hdr_detach_instruction_invalidate(PHDR_INSTRUCTION instr)
{
  if(instr == NULL) return ;
  if(instr->code != NULL) 
  {
	  hdr_detach_invalidate_var_list(instr->code->variables) ;
  
	  /*now invalidate all the instructions of the block*/
	  PDXL_NODE node = instr->code->instructions->start ;
	  while(node != NULL)
	  {
		PHDR_INSTRUCTION tinstr = (PHDR_INSTRUCTION)node->object->obj ;
		hdr_detach_instruction_invalidate(tinstr) ;
		node = node->right ;
	  }
	  }
	  /*invalidate else*/
	  hdr_detach_instruction_invalidate(instr->ielse)        ;
}


void hdr_detach_invalidate_vars(PHDR_CUSTOM_FUNCTION detach_func)
{
	/*invalidate the top blocks variables*/
	hdr_detach_invalidate_var_list(detach_func->code->variables) ;
	/*invalidate all the instructions blocks*/
	PDXL_NODE node = detach_func->code->instructions->start ;
	while(node != NULL)
	{
		PHDR_INSTRUCTION instr = (PHDR_INSTRUCTION)node->object->obj ;
		hdr_detach_instruction_invalidate(instr) ;
		node =node->right ;
	}
	
}
/*******************************************************************************/

enum exec_state hdr_detach_run_function(PHDR_INTERPRETER inter , PHDR_CUSTOM_FUNCTION func ,PHDR_COMPLEX_TOKEN token,PHDR_COMPLEX_TOKEN targetv) 
{
	/*
	  The detach function will be run as a typical function. BUT there is a problem. The normal functions are permanent in the
	  lifetime of the script , and the async functions are completelly standalone. So in actuality its blocks and by extension
	  its variables, are not destroyed until the scripts end. BUT the detached functions are being destroyed the moment that
	  their execution stops. This , makes the variables in the blocks to be released , BUT the variables in its block 
	  can be just references to live variables. So we have to mitigate this and invalidate the objects
	  in the variables , except of the simple strings that will be freed.

	*/
	PHDR_CUSTOM_FUNCTION detach_func = hdr_detach_custom_function_copy(inter,func->name,func->object) ; 

	if(detach_func == NULL)
	{
	  printf("The %s function that the [detach] command tried to execute does not exist in the script.",func->name->stringa);
	  return exec_state_error ;
	}

	PHDR_VAR target = NULL ;

	if(targetv != NULL)
	{
		target = hdr_inter_retrieve_var_block(inter,targetv->ID) ;
		if(target == NULL)
		{
		  printf("The target variable %s for the [detach] command was not found in the scope.",targetv->ID->stringa);
		  hdr_detach_invalidate_vars(detach_func) ;
		  hdr_custom_function_free(detach_func) ;
		  return exec_state_error ;
		}
	}

	PHDR_VAR result = NULL ;
	enum hdr_user_func_result res = hdr_detach_execute(inter,token,detach_func,&result) ;
    if((result!=NULL)&&(target != NULL))
	{
	 /*do the assigment*/
	 if (hdr_inter_assign_and_clear(target, result, false) == false)
	 {
		printf("Error in assigment of the detached function.\n") ;
		hdr_detach_invalidate_vars(detach_func) ;
		hdr_custom_function_free(detach_func) ;
		if (hdr_inter_var_gen_type(result) != hdr_ivt_string)
		hdr_var_release_obj(result); /*we do not want the actual object to be freed except if it is a simple string*/
		hdr_var_free(result);
		
		return exec_state_error ;
	 }

	 /*the result was used just to transport the actual object data , now delete it , EXCEPT the data is a simple string, then we free the memory*/
	 if (hdr_inter_var_gen_type(result) != hdr_ivt_string)
	 hdr_var_release_obj(result); /*we do not want the actual object to be freed except if it is a simple string*/
	 hdr_var_free(result);

	}
	
	/*
	 OBSOLETE! Changed in 29-08-2024 as to mitigate multiple problems with async function destruction the invalidation
	 was moved to hdr_custom_function_free.
	
	  invalidate all variable objects except simple strings
	  hdr_detach_invalidate_vars(detach_func) ;
	*/
	/*free the function*/
	hdr_custom_function_free(detach_func) ;

	if( res == hdr_user_func_value)    return exec_state_ok       ;
	if( res == hdr_user_func_error)    return exec_state_error ;
	if( res == hdr_user_func_success)  return exec_state_ok	   ;
	
    return exec_state_ok ;
}


enum exec_state hdr_inter_handle_detach(PHDR_INTERPRETER inter, PHDR_EXPRESSION expr)
{
	/*
	 
	 the detach is created to solve the problem of the function recursion.
	 every function except the asyncs retain the same variable list in the life time
	 of the program. This saves memory and cpu cycles when the function starts to executes.
	 BUT if we need recursion OR a function to be thread safe this is impossible with the current schema.
	 The [detach] run a copy of the function. The [detach] is structured like detach $var myfunction();
	 the $var is optional and is the same as the assigment $var = myfunction();

	 18-10-2024 check if the function is a member of an object. We need to have the ability to run an object function
	 as detached because in a multithreaded enviroment is very easy to have corrupted memory when a thread calls 
	 a member function of an object simultaneously or semi simultaneously with another. When the function will be called for the second time,
	 its parameters and local variables will be invalidated for preparation of the execution , and the previous thread will crash
	 in the best case , and in the worst case invalid data will be proccessed. 

	 */

	PHDR_COMPLEX_TOKEN 	 vtoken = NULL ;
	PHDR_COMPLEX_TOKEN   otoken = NULL ;
	PHDR_COMPLEX_TOKEN 	 ftoken = NULL ;

	if(expr->tokens->count == 1)
	{
	  ftoken = hdr_inter_return_list_token(expr->tokens, 0);
	  if(ftoken == NULL)
	  {
	    printf("The [detach] command failed. Check the syntax , as it may be malformed. Example : detach optional -> [$var < ] user_function() ;");
	    return exec_state_error ;
	  }
	}
	else
	if(expr->tokens->count == 3)
	{
	  vtoken = hdr_inter_return_list_token(expr->tokens, 0);
	  if(vtoken == NULL)
	  {
	    printf("The [detach] command failed. Check the syntax , as it may be malformed. Example : detach optional -> [$var < ] user_function() ;");
	    return exec_state_error ;
	  }

	  otoken = hdr_inter_return_list_token(expr->tokens, 1);
	  if(otoken == NULL)
	  {
	    printf("The [detach] command failed. Check the syntax , as it may be malformed. Example : detach optional -> [$var < ] user_function() ;");
	    return exec_state_error ;
	  }

	  ftoken = hdr_inter_return_list_token(expr->tokens, 2);
	  if(ftoken == NULL)
	  {
	    printf("The [detach] command failed. Check the syntax , as it may be malformed. Example : detach optional -> [$var < ] user_function() ;");
	    return exec_state_error ;
	  }

	}
	else
	{
	    printf("The [detach] command failed. Check the syntax , as it may be malformed (the supplied parameters are not in the right count). Example : detach optional -> [$var < ] user_function() ;");
	    return exec_state_error ;
	}

	if((ftoken->type != hdr_tk_function)&&(ftoken->type != hdr_tk_multipart))
	{
	   printf("Only a standalone user function or an object function can follow a [detach] command.\n");
	   return exec_state_error;
	}

	if(vtoken != NULL)
	{
      if(vtoken->type != hdr_tk_variable)
	  {
	    printf("Only a variable can be set as the destination of the function result in the [detach] command.\n");
	    return exec_state_error;
	  }

	  if(otoken->type != hdr_tk_operator)
	  {
	    printf("Only the [<] sign can be used in the [detach] command.\n");
	    return exec_state_error;
	  }
	}

	/*syntax cheks out , find the custom function*/
	PHDR_CUSTOM_FUNCTION func		= hdr_custom_functions_list_find(inter,ftoken->ID) ;
	if(func == NULL)
	{
	  /*Check if the function is inside an object*/
	  if(inter->curr_obj != NULL)
	  {
	    func = hdr_custom_functions_list_find_gen(inter->curr_obj->functions , ftoken->ID) ;
	  }
	}
	
	if(func == NULL)
	{
	  /*check if the function is a member of an object*/

	  PHDR_VAR obj = hdr_inter_retrieve_var_from_block(inter, ftoken->ID) ;
	  if(obj == NULL)
	  {
	    printf("Fatal Error -> Line : %d The detach function needs a valid variable [object] with a valid member function to run. The variable was not found in the scope. Token : [%s].\n", 
	    inter->current_instr->instr->line,ftoken->ID->stringa);
	    return exec_state_error ;
	  }

	  /*check if the variable is of the object type*/
	  if(obj->type != hvt_object)
	  {
	    printf("Fatal Error -> Line : %d The detach function needs a valid variable [object] with a valid member function to run.The variable is not an object. Variable name : [%s].\n", 
	    inter->current_instr->instr->line,ftoken->ID->stringa);
	    return exec_state_error ;
	  }

	  /*ok is an object , check if the member is a function */
	  if(ftoken->next_part->type != hdr_tk_function)
	  {
	    printf("Fatal Error -> Line : %d The detach function needs a valid member function to run. The member is not a function. Token : [%s].\n", 
	    inter->current_instr->instr->line,ftoken->next_part->ID->stringa);
	    return exec_state_error ;
	  }

	  /*search for the function in the object class*/
	  PHDR_OBJECT_CLASS true_obj = obj->obj ;
	  
	  func = hdr_custom_functions_list_find_gen(true_obj->functions , ftoken->next_part->ID) ;
	  /*re-purpose the ftoken to use bellow*/
	  ftoken = ftoken->next_part ;
	}

	if(func == NULL)
	{
	  printf("Fatal Error -> Line : %d The user function [%s] was not found.\n", 
	  inter->current_instr->instr->line,ftoken->ID->stringa);
	  return exec_state_error ;
	}
	/*check if the parameters are matching in number*/
	if(func->parameters->count != ftoken->parameters->count)
	{
	  printf("Fatal Error -> Line : %d The user function [%s] needs %d parameters. %d was provided.\n", 
	  inter->current_instr->instr->line,ftoken->ID->stringa,func->parameters->count,ftoken->parameters->count);

	  return exec_state_error ;
	}

	return hdr_detach_run_function(inter,func,ftoken,vtoken);
}

/***************************************************************************************************************/

enum exec_state hdr_inter_handle_terminate(PHDR_INTERPRETER inter, PHDR_EXPRESSION expr)
{
	PHDR_COMPLEX_TOKEN 	 ftoken = hdr_inter_return_list_token(expr->tokens, 0);
	
	if(ftoken == NULL)
	{
	  printf("The [terminate] command failed. Check the syntax , as it may be malformed. Example : terminate `thread_id`;");
	  return exec_state_error ;
	}

	/*check for syntax errors*/

	if((ftoken->type != hdr_tk_literal)&&(ftoken->type != hdr_tk_variable)) 
	{
	  printf("The [terminate] command failed. The [thread_id] is not a literal or a variable. Check the syntax , as the may be malformed. Example : terminate `thread_id`;");
	  return exec_state_error ;
	}

	PDX_STRING thread_id = NULL ;

	if(ftoken->type == hdr_tk_variable)
	{
		PHDR_VAR			 pvar = hdr_inter_retrieve_var_block(inter, ftoken->ID);
		if (pvar == NULL)
		{
			printf("The variable that represents the thread_id was not found.Keep in mind that only simple and not indexed or multipart variables can be used in the [terminate] command.\n");
			return exec_state_error;
		}

		if((pvar->type!=hvt_simple_string)&&(pvar->type!=hvt_simple_string_bcktck))
		{
		    printf("The variable that represents the thread_id must be a simple string.\n");
			return exec_state_error;
		}

		thread_id = (PDX_STRING)pvar->obj ; 

	}
	else
		{
		  thread_id = (PDX_STRING)ftoken->val->obj ;
		}


	if(hdr_terminate_thread(inter , thread_id) == false) return exec_state_error ;
	else
		return exec_state_ok ;

}

enum exec_state hdr_inter_handle_message(PHDR_INTERPRETER inter, PHDR_EXPRESSION expr)
{
	if(inter->thread == NULL) 
	{
		/*this is not a fatal erro*/
		if(inter->warnings == true)
		printf("The [message] command can be used only inside an [async] function");
		return exec_state_ok ;
	}

	PHDR_COMPLEX_TOKEN 	 ftoken = hdr_inter_return_list_token(expr->tokens, 0);
	
	if(ftoken == NULL)
	{
	  printf("The [message] command failed. Check the syntax , as it may be malformed. Example : message `some message`;");
	  return exec_state_error ;
	}

	/*check for syntax errors*/

	if((ftoken->type != hdr_tk_literal)&&(ftoken->type != hdr_tk_variable)) 
	{
	  printf("The [message] command failed. The [thread_id] is not a literal or a variable. Check the syntax , as the may be malformed. Example : message `some message`;");
	  return exec_state_error ;
	}

	PDX_STRING message = NULL ;

	if(ftoken->type == hdr_tk_variable)
	{
		PHDR_VAR			 pvar = hdr_inter_retrieve_var_block(inter, ftoken->ID);
		if (pvar == NULL)
		{
			printf("The variable that represents the thread_id was not found.Keep in mind that only simple and not indexed or multipart variables can be used in the [message] command.\n");
			return exec_state_error;
		}

		if((pvar->type!=hvt_simple_string)&&(pvar->type!=hvt_simple_string_bcktck))
		{
		    printf("The variable that represents the thread_id must be a simple string.\n");
			return exec_state_error;
		}

		message = (PDX_STRING)pvar->obj ; 

	}
	else
		{
		  message = (PDX_STRING)ftoken->val->obj ;
		}

	dx_string_createU(inter->thread->message,message->stringa) ;
	return exec_state_ok ;
}


enum exec_state hdr_inter_handle_wait(PHDR_INTERPRETER inter, PHDR_EXPRESSION expr)
{
	if(inter->thread != NULL) 
	{
		/*this is not a fatal error but does not have any effect*/
		printf("The [terminateAll] command can be called only in the main thread.\n");
		return exec_state_ok ;
	}

	PHDR_COMPLEX_TOKEN 	 ftoken = hdr_inter_return_list_token(expr->tokens, 0);
	
	if(ftoken == NULL)
	{
	  printf("The [termonateAll] command failed. Check the syntax , as it may be malformed. Example : wait $seconds ;");
	  return exec_state_error ;
	}

	/*check for syntax errors*/

	if((ftoken->type != hdr_tk_literal)&&(ftoken->type != hdr_tk_variable)) 
	{
	  printf("The [terminateAll] command failed. The [seconds] parameter is not a literal or a variable. Check the syntax , as it may be malformed. Example : wait $seconds ;");
	  return exec_state_error ;
	}

	int seconds = 0 ;

	if(ftoken->type == hdr_tk_variable)
	{
		PHDR_VAR			 pvar = hdr_inter_retrieve_var_block(inter, ftoken->ID);
		if (pvar == NULL)
		{
			printf("The variable that represents the thread_id was not found.Keep in mind that only simple and not indexed or multipart variables can be used in the [message] command.\n");
			return exec_state_error;
		}

		if((pvar->type!=hvt_integer)&&(pvar->type!=hvt_float))
		{
		    printf("The variable that represents the seconds must be a numeric value.\n");
			return exec_state_error;
		}
		bool type_error = false ;
		seconds = hdr_inter_ret_integer(pvar,&type_error);

	}
	else
		{
			bool type_error = false ;
			seconds = hdr_inter_ret_integer(ftoken->val,&type_error);
		}

	
	hdr_threads_term_and_wait(inter,seconds) ;

	return exec_state_ok ;
}


/***************************************************************************/

/**************************Handle multipart command*******************/


bool hdr_get_terminal_var(PHDR_INTERPRETER inter , PHDR_TERMINAL_TOKEN ter_token)
{

	/*
	 the ter_token->last_token must being initialized with the multipart token
	 the var must be NULL.
	 The function will set the var and the last_token to the correct values
	 returns true on error , false on success
	*/

	if (ter_token->last_token == NULL) return false; /*end of token chain*/

	bool error;
	if (ter_token->last_token->type == hdr_tk_multipart)
	{
		/*init the multipart return value*/
		ter_token->last_token->val->type = hvt_undefined;
		/*get the first variable*/
		ter_token->var = hdr_inter_retrieve_var_from_block(inter, ter_token->last_token->ID);
		if (ter_token->var == NULL)
		{
			printf("The multipart variable '%s' was not found in the scope.\n", ter_token->last_token->ID->stringa);
			return true;
		}

		/*
		 we have the first variable , but there is the possibility that the root token (this token) has an index too , 
		 resolve the value first and then find the right variable
		*/
		if (ter_token->last_token->expression != NULL)
		{
			PHDR_VAR indx = hdr_inter_resolve_expr(inter, ter_token->last_token->expression, &error);
			if(indx == NULL) error = true ; /*if the indx is not a valid variable then the indx will return as NULL*/
			if (error == true) return true ; 
			/*check the index*/
			if (hdr_inter_var_gen_type(indx) == hdr_ivt_other) 
			{printf("The only valid types for an index are the 'Numeric' and 'Simple string' types.\n"); return true;}

			/*Check if the variable can have an index*/
			if(hdr_inter_check_index_type(ter_token->var) == false)
			{printf("The variable '%s' is not of a type that can have an index. Keep in mind that the characters of a complex string cannot be accessed by an index.\n",ter_token->var->name->stringa); return true;}

			/*all test are passed , we need to forward the index to the handler and get the variable back*/
			ter_token->var = hdr_inter_resolve_index(inter, ter_token->var, indx);

			/*reset indx that is a persistent variable of the expression*/
			if (hdr_inter_var_gen_type(indx) == hdr_ivt_string)
			indx->obj = dx_string_free((PDX_STRING)indx->obj);
			
			hdr_var_release_obj(indx) ;

			indx->type = hvt_undefined;

			if (ter_token->var == NULL) return true; /*an error was printed in the hdr_inter_resolve_index*/
		}

		/*ok , the variable is set , resolve the next part*/
		ter_token->last_token = ter_token->last_token->next_part;
		error = hdr_get_terminal_var(inter, ter_token);

		if (error == true) return true;
		/*check the variable , if there is a variable*/

		return false ;
	}
	else
	if (ter_token->last_token->type == hdr_tk_ref_brackets)
	{
		/*this token is an token with an index */

		/*
		  check for the actual token
		  keep in mind that a root indexed variable MUST be resolved first as the function has not being
		  in the initialization state that the multipart case above create.
		*/

		if (ter_token->last_token->previous_part == NULL)
		{
			/*standalone indexed variable. initialize the ter_token*/
			ter_token->last_token->val->type = hvt_undefined;
			/*get the first variable*/
			ter_token->var = hdr_inter_retrieve_var_from_block(inter, ter_token->last_token->ID);
			if (ter_token->var == NULL)
			{
				printf("The variable '%s' (in indexed state) was not found in the scope.\n", ter_token->last_token->ID->stringa);
				return true;
			}

			/*check if variable is initialized*/
			if (ter_token->var->type == hvt_undefined)
			{
				printf("The variable's '%s' (in indexed state) type is not set.\n", ter_token->last_token->ID->stringa);
				return true;
			}

		}
		else
		{
			/*the index is part of a multipart token*/
			PHDR_VAR indx = ter_token->last_token->val;
			hdr_var_set_obj(indx,ter_token->last_token->ID) ;
			indx->type = hvt_simple_string;
			ter_token->var = hdr_inter_resolve_index(inter, ter_token->var, indx);
			if (ter_token->var == NULL) return true; /*an error was printed in the hdr_inter_resolve_index*/
			/*ok reset the indx to be safe*/
			hdr_var_release_obj(indx) ;
			indx->type = hvt_undefined;
		}
				
		PHDR_VAR indx = hdr_inter_resolve_expr(inter, ter_token->last_token->expression, &error);
		if (indx == NULL)
		{
			/*check if the variable is a simple list, the simple list can create indexes on the fly */
			if(ter_token->var->type == hvt_list)
			{
			  /*create a new variable*/
			  PHDR_VAR nvar  = hdr_var_create(NULL,"",hvf_dynamic,NULL) ;
			  PDX_LIST list = (PDX_LIST)ter_token->var->obj ;
			  PDXL_OBJECT obj = dxl_object_create() ;
			  obj->obj = nvar ; /*add the new undefined variable*/
			  dx_list_add_node_direct(list,obj) ;
			  /*the function must set this variable as the current one*/
			  ter_token->var = nvar ;
			  ter_token->last_token = ter_token->last_token->next_part;
			  return  hdr_get_terminal_var(inter, ter_token);

			}
			else
			printf("The variable index returned as NULL. Check if the index is a variable that is declared in the scope.\n"); return true;
		}

		if (error == true) return true;
		/*check the index*/
		if(ter_token->last_token->expression->tokens->count != 0 )  /*an index can be empty (for list dynamic variable creation)*/
		if (hdr_inter_var_gen_type(indx) == hdr_ivt_other)
		{
			printf("The only valid types for an index are the 'Numeric' and 'Simple string' types.\n"); return true;
		}

		/*Check if the variable can have an index*/
		if (hdr_inter_check_index_type(ter_token->var) == false)
		{
			printf("The variable '%s' is not of a type that can have an index. Keep in mind that the characters of a complex string cannot be accessed by an index.\n", ter_token->var->name->stringa); return true;
		}

		/*all test are passed , we need to forward the index to the handler and get the variable back*/
		ter_token->var = hdr_inter_resolve_index(inter, ter_token->var, indx);


		/*reset index*/
		if (hdr_inter_var_gen_type(indx) == hdr_ivt_string)
		{
			dx_string_free((PDX_STRING)indx->obj);
		}
		
		hdr_var_release_obj(indx) ;
		indx->type = hvt_undefined;

		if (ter_token->var == NULL) return true; /*an error was printed in the hdr_inter_resolve_index*/

		ter_token->last_token = ter_token->last_token->next_part;
		return  hdr_get_terminal_var(inter, ter_token);


	}
	else
	if (ter_token->last_token->type == hdr_tk_index)
	{
		/*this token is an index to the current variable , check first the variable's validity and then the index validity*/
		PHDR_VAR indx = hdr_inter_resolve_expr(inter, ter_token->last_token->expression, &error);
		if (error == true) return true;
		/*check the index. The index CANNOT be NULL EXCEPT if the variable is a list and a new varianble is created*/
		if((indx == NULL)&&(ter_token->var->type != hvt_list))
		{
		  printf("An empty bracket section is valid only for the List type.\n"); return true;
		}
		else
		if((indx == NULL)&&(ter_token->var->type == hvt_list))
		{
		    /*create a new variable*/
			  PHDR_VAR nvar  = hdr_var_create(NULL,"",hvf_dynamic,NULL) ;
			  PDX_LIST list = (PDX_LIST)ter_token->var->obj ;
			  PDXL_OBJECT obj = dxl_object_create() ;
			  obj->obj = nvar ; /*add the new undefined variable*/
			  dx_list_add_node_direct(list,obj) ;
			  /*the function must set this variable as the current one*/
			  ter_token->var = nvar ;
			  ter_token->last_token = ter_token->last_token->next_part;
			  return  hdr_get_terminal_var(inter, ter_token);
		
		}
		if (hdr_inter_var_gen_type(indx) == hdr_ivt_other)
		{
			printf("The only valid types for an index are the 'Numeric' and 'Simple string' types.\n"); 
			return true;
		}

		/*Check if the variable can have an index*/
		if (hdr_inter_check_index_type(ter_token->var) == false)
		{
			printf("The variable '%s' is not of a type that can have an index. Keep in mind that the characters of a complex string cannot be accessed by an index.\n", ter_token->var->name->stringa); 
			return true;
		}

		/*Last check , a string or bytes cannot have another token after the bracket indexing*/
		if(((ter_token->var->type == hvt_bytes)||(ter_token->var->type == hvt_simple_string)||(ter_token->var->type == hvt_simple_string_bcktck))&&(ter_token->var->need_indx == true))
		{
			printf("The indexed token '%s' cannot have another index after the initial one. [Strings] and [Bytes] cannot have multiple indexes.\n", ter_token->var->name->stringa);
			return true;
		}

		/*all test are passed , we need to forward the index to the handler and get the variable back*/
		ter_token->var = hdr_inter_resolve_index(inter, ter_token->var, indx);
		
		/*reset index*/
		if (hdr_inter_var_gen_type(indx) == hdr_ivt_string)
			indx->obj = dx_string_free((PDX_STRING)indx->obj);
		
		hdr_var_release_obj(indx) ;
		indx->type = hvt_undefined;
		
		if (ter_token->var == NULL) return true; /*an error was printed in the hdr_inter_resolve_index*/
		
		ter_token->last_token = ter_token->last_token->next_part;
		return  hdr_get_terminal_var(inter, ter_token);


	}else
	if (ter_token->last_token->type == hdr_tk_simple) 
	{
		/*this is a string type index actually so we will return the correct variable*/
		/*Check if the variable can have an index*/
		if (hdr_inter_check_index_type(ter_token->var) == false)
		{
			printf("The variable '%s' is not of a type that can have an index. Keep in mind that the characters of a complex string cannot be accessed by an index.\n", ter_token->var->name->stringa); 
			return true;
		}

		/*all test are passed , we need to forward the index to the handler and get the variable back*/
		/*we will use the already existing variable in the token to act as the index*/
		PHDR_VAR indx = ter_token->last_token->val   ;
		hdr_var_set_obj(indx,ter_token->last_token->ID) ;
		indx->type = hvt_simple_string;
		ter_token->var = hdr_inter_resolve_index(inter, ter_token->var, indx);
		if (ter_token->var == NULL) return true; /*an error was printed in the hdr_inter_resolve_index*/
		/*ok reset the indx to be safe*/
		hdr_var_release_obj(indx) ;
		indx->type = hvt_undefined;
		/*next token*/
		ter_token->last_token = ter_token->last_token->next_part;
		return  hdr_get_terminal_var(inter, ter_token);

	}
	else 
	if (ter_token->last_token->type == hdr_tk_function)
	{
		/*
		 this is a terminal token that can or cannot return a variable.
		 we will execute the function and we will return.
		*/
		/*we will use the value of the root token to save the function result*/
		PHDR_VAR result = NULL ;
		error = hdr_inter_handle_function(inter, ter_token->last_token,ter_token->var, &result);
		
		if (error == true) return true;
		

		/*
		 13-11-2024 This was a memory leak and a big one !
		 Check for string in the ter_token. If the string are in a temporary created 
		 var we have to release it
		*/

		if(ter_token->var->var_ref == hvf_temporary_ref)
		{
			if((ter_token->var->type == hvt_simple_string)||(ter_token->var->type == hvt_simple_string_bcktck)||
				(ter_token->var->type == hvt_unicode_string))
			{
				dx_string_free((PDX_STRING)ter_token->var->obj);
			}
			/*release the variable  too as it will be not used anymore*/
			hdr_var_release_obj(ter_token->var) ;
			hdr_var_free(ter_token->var) ;
		}

		/*****************************************************************************/

		if (result != NULL)
		{
			ter_token->var = ter_token->last_token->val;
			if((ter_token->var->var_ref != hvf_system)&&(ter_token->var->var_ref != hvf_block)&&(ter_token->var->var_ref != hvf_dynamic))
			ter_token->var->var_ref = hvf_temporary_ref; /*the caller will check this to determine if the variable is a true one or just a carrier of values*/
			/*return the tokens variable after "copying" it*/
			if (hdr_inter_var_gen_type(result) == hdr_ivt_string)
			{
				ter_token->var->type = result->type;

				if(ter_token->var->type != hvt_unicode_string)
				{
				  PDX_STRING str = dx_string_createU((PDX_STRING)ter_token->var->obj,((PDX_STRING)result->obj)->stringa) ;
				  hdr_var_set_obj(ter_token->var,str) ; 
				}
				else
				{
					 PDX_STRING str = dx_string_createX((PDX_STRING)ter_token->var->obj,((PDX_STRING)result->obj)->stringx) ;
					 hdr_var_set_obj(ter_token->var,str) ; 
				}
			
				/*invalidate the resulted string*/ 
				if((result->var_ref != hvf_system)&&(result->var_ref != hvf_block)&&(result->var_ref != hvf_dynamic))
				{
					dx_string_free(((PDX_STRING)result->obj));
					hdr_var_release_obj(result) ;
				}
			}
			else
			{
				ter_token->var->type = result->type ;
				if((ter_token->var->type == hvt_bool)||(ter_token->var->type == hvt_integer)||(ter_token->var->type == hvt_codepoint))
				ter_token->var->integer = result->integer ;
				else
				if(ter_token->var->type == hvt_float) ter_token->var->real = result->real ;
				else
				{
				 hdr_var_set_obj(ter_token->var,result->obj);
				}
				/*
				 there is the posibility the function to return a reference to a block or dynamic
				 variable , we have to set the is_ref so the interpreter will not try to release two times the same memory
				*/
				ter_token->var->is_ref = result->is_ref ;
				if ((result->var_ref != hvf_block)&&(result->var_ref != hvf_system)&&(result->var_ref != hvf_dynamic))
				hdr_var_release_obj(result) ;/*we do not want the actual object to be freed except if it is a simple string*/
			}
			if((result->var_ref != hvf_system)&&(result->var_ref != hvf_block)&&(result->var_ref != hvf_dynamic))
			hdr_var_free(result);
		}
		else
		{
			ter_token->var = NULL ;
		}

		return false;
	}
	else
	{
	 
		printf("The variable's '%s' (in indexed state) type is not set.\n", ter_token->last_token->ID->stringa);
		return true;
			
	}

	return false;
}


enum exec_state hdr_handle_mutipart_command(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token)
{
	/*
     find the appropriate referenced variable and run the command
	 data structured example : 
	 $var1[2].some_field[1][2].runme()

	 token->id = $var1
	 token->expression = 2 (NULL if it does not have any index)
	 token->next_part = some_field

	 some_field->id = some_field
	 some_field->expression = 1
	 some_field->next_part = index[]

	 index[]->id = index[]
	 index[]->expression = 2
	 index[]->next_part = runme()

	 runme->type = function
	 ...

    */

	
	if (token->ID->stringa[0] != '$')
	{
		printf("The token '%s' is not a variable. Indexes and multiparts in the left side of an expression are valid only in variables.\n",token->ID->stringa);
		return exec_state_error;
	}

	/*
		execute terminal command. if the function returns a value then we will treat this as an error, 
		as we forbid the returning of variables when out of an expression or assigment
	*/
	struct hdr_terminal_token ter_token;
	ter_token.var = NULL;
	ter_token.last_token = token;
	if (hdr_get_terminal_var(inter,&ter_token) == true) return exec_state_error;
	
	if (ter_token.var == NULL) return exec_state_ok; /*all ok no variable was retrieved*/

	if (ter_token.var->type != hvt_undefined)
	{
		/*we will throw a warning , because this can lead to memory leaks if its not intended*/
		if(inter->warnings == true)
			hdr_inter_print_warning(inter,"The instruction returns a variable , but the variable is not used in any expression or assigment.\n");
		return exec_state_ok;
	}

	return exec_state_ok;
}

/**************************************************************************/

/*------------------------------------------------------------------------------------------------*/

enum exec_state hdr_inter_handle_single_command(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token)
{

	/*check if the command is a multipart command */

	if (token->type == hdr_tk_multipart)
	{
		return hdr_handle_mutipart_command(inter,token);
	}

	if (hdr_inter_fast_str(token->ID, "return", 6) == true)
	{
		return hdr_inter_handle_exit(inter) ;
	}

	if (hdr_inter_fast_str(token->ID, "fill_params", 11) == true)
	{
		return hdr_cmd_fill_params(inter) ;
	}

	if (hdr_inter_fast_str(token->ID, "time_point", 10) == true)
	{
		return hdr_cmd_time_point(inter) ;
	}

    if (hdr_inter_fast_str(token->ID, "show_version", 12) == true)
	{
		return hdr_cmd_version(inter) ;
	}

	if (hdr_inter_fast_str(token->ID, "nop", 3) == true)
	{
		return exec_state_ok ; /*no operation*/
	}

	if (hdr_inter_fast_str(token->ID, "show_warnings", 13) == true)
	{
		if (inter->warnings == true)
		{
			hdr_inter_print_warning(inter, "The 'show_warning directive has been already used in the script.'");
		}
		 else
		   inter->warnings = true;
		return exec_state_ok ;
	}
	/*check for empty variable declaration e.g $test ;*/
	if(token->ID->stringa[0] == '$' )
	{
	  printf("Hydra+ does not allow declaration of variables without type. Variable : %s\n",token->ID->stringa);
	  return exec_state_error;
	}

	/*not recognizable token*/
	printf("The token '%s' is not a recognizable command.\n",token->ID->stringa);
	return exec_state_error;
}


enum exec_state hdr_inter_handle_complex_command(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token, PHDR_EXPRESSION expr)
{
	if (hdr_inter_fast_str(token->ID, "dbg", 3) == true)
	{
		return hdr_cmd_dbg(inter, expr);
	}

	/*handle the return with a value*/
	if (hdr_inter_fast_str(token->ID, "return", 6) == true)
	{
		return hdr_inter_handle_return(inter,expr) ;
	}

	if (hdr_inter_fast_str(token->ID, "async",5) == true)
	{
		return hdr_inter_handle_async(inter,expr) ;
	}

	if(hdr_inter_fast_str(token->ID,"go",2)==true)
	{
	   return hdr_inter_handle_go(inter,expr) ;
	}

	if(hdr_inter_fast_str(token->ID,"asfunc",6)==true)
	{
	   return hdr_inter_handle_asfunc(inter,expr) ;
	}
	
	if(hdr_inter_fast_str(token->ID,"detach",6)==true)
	{
	   return hdr_inter_handle_detach(inter,expr) ;
	}

	if (hdr_inter_fast_str(token->ID, "terminate",9) == true)
	{
		return hdr_inter_handle_terminate(inter,expr) ;
	}

	if (hdr_inter_fast_str(token->ID, "message",7) == true)
	{
		return hdr_inter_handle_message(inter,expr) ;
	}

	if (hdr_inter_fast_str(token->ID, "terminateAll",12) == true)
	{
		return hdr_inter_handle_wait(inter,expr) ;
	}

	/*not recognizable token*/
	printf("The token '%s' is not a recognizable complex command.\n", token->ID->stringa);
	return exec_state_error;
}


bool hydra_set_params(PHDR_BLOCK code , PHDR_VAR_LIST param_var_list)
{
  
    /*find the variable in the block and set the value*/
    PHDR_VAR_LIST varlist    = code->variables ;
    PHDR_VAR_LIST paramlist  = param_var_list;
    if(paramlist->list->count == 0 ) return true ; /*no params*/

    PDX_LIST *buckets = paramlist->list->buckets ;
    int bcount   = paramlist->list->length       ;
    for(int i = 0 ; i < bcount ; i++)
    {
      PDX_LIST bucket = buckets[i]   ;
      PDXL_NODE node = bucket->start ;
      while(node != NULL)
      {
        PHDR_VAR param = (PHDR_VAR)node->object->obj ;

        PHDR_VAR var = hdr_var_list_find_var(varlist,param->name);
        if(var == NULL)
        {
         printf("***Fatal Error : The parameter [%s] is not being set in the main script as a variable.",param->name->stringa);
         return false ;
        }

        /*check the type*/
        if((hdr_inter_var_is_simple_string(var) == true)&&(hdr_inter_var_is_simple_string(param) == true))
        {
            dx_string_free((PDX_STRING)var->obj);
            PDX_STRING str = dx_string_createU(NULL,((PDX_STRING)param->obj)->stringa) ;
			hdr_var_set_obj(var,str) ;
        } 
        else
            if((hdr_inter_var_is_numeric(var)==true)&&(hdr_inter_var_is_numeric(param)==true))
            {
                
                if(var->type != param->type) goto error_type ;
                if(var->type == hvt_integer) var->integer = param->integer ;
                else
                    var->real = param->real ;

            }
            else
                {
                    error_type :
                    printf("***Fatal Error : The parameter [%s] is of a different type than the variable [%s] .",param->name->stringa,param->name->stringa);
                    return false ;
                }

        node = node->right ;
      }
    }

    return true ;
}