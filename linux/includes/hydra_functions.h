/*
 This file has the native functions that the Hydra+ supports.
 
 Nikolaos Mourgis deus-ex.gr 2024

 Live Long and Prosper.


  Put in this file any additional function you want the Hydra+ to support.

*/


typedef struct hdr_sys_func_params
{
	PHDR_VAR params[20]  ; /*maximum 20 parameters*/
	DXLONG64 param_count ;
} *PHDR_SYS_FUNC_PARAMS;

PHDR_SYS_FUNC_PARAMS hdr_sys_func_init_params(PHDR_INTERPRETER inter ,PHDR_EXPRESSION_LIST params,int expected_count) ;
PHDR_SYS_FUNC_PARAMS hdr_sys_func_free_params(PHDR_SYS_FUNC_PARAMS sys_params) ;
PHDR_VAR hdr_inter_resolve_extract_var_from_expr(PHDR_INTERPRETER inter ,PHDR_EXPRESSION_LIST params , int param_pos);



/***************** user custom functions ******************/

PHDR_CUSTOM_FUNCTION hdr_custom_functions_list_find(PHDR_INTERPRETER inter , PDX_STRING func_name)
{
	PDXL_OBJECT obj = dx_HashReturnItem(inter->funcs,func_name,true) ;
	if (obj == NULL) return NULL ;
	return (PHDR_CUSTOM_FUNCTION)obj->obj ; 
}

PHDR_CUSTOM_FUNCTION hdr_custom_functions_list_find_gen(PHDR_CUSTOM_FUNCTIONS_LIST lst , PDX_STRING func_name)
{
	PDXL_OBJECT obj = dx_HashReturnItem(lst,func_name,true) ;
	if (obj == NULL) return NULL ;
	return (PHDR_CUSTOM_FUNCTION)obj->obj ; 
}

void hdr_user_func_code_init_var(PHDR_VAR_LIST list)
{
	 /*we will set all the variables to hvt_undefined*/
	 for(int i = 0; i < list->list->length;i++)
	 {
		  /*for all the buckets*/
		  PDX_LIST    bucket = list->list->buckets[i] ;
		  PDXL_NODE   node   = bucket->start		  ;
		  /*set the variables type*/
		  while(node != NULL)
		  {
			PHDR_VAR var = (PHDR_VAR)node->object->obj ;
			if(var != NULL)
		    var->type = hvt_undefined  ;
			node = node->right ; /*next variable*/
		  }
	 }
  
}

PDX_LIST hdr_user_func_h_release_params(PDX_LIST params)
{

	PDXL_NODE node = params->start ;
	while(node!=NULL)
	{
	  PDXL_OBJECT obj = node->object ;
	  PHDR_VAR   var  = (PHDR_VAR)obj->obj ;
	  if((var->var_ref != hvf_block)&&(var->var_ref != hvf_dynamic)&&(var->var_ref != hvf_system))
	  {
		if(var->var_ref == hvf_temporary_ref_user_func)
		{
		    if((var->type == hvt_simple_string)||(var->type == hvt_simple_string_bcktck)||(var->type == hvt_unicode_string))
			dx_string_free((PDX_STRING)var->obj) ;
			hdr_var_release_obj(var) ;
			hdr_var_free(var);
		}
	  
	  }
	  else
		  if(var->var_ref == hvf_system)
		  {
		   /*system variables MUST not be freed but if its type are a simple string or unicode string , need deallocation*/
		    if((var->type == hvt_simple_string)||(var->type == hvt_simple_string_bcktck)||(var->type == hvt_unicode_string))
		    {
		      dx_string_free((PDX_STRING)var->obj);
			  hdr_var_release_obj(var) ;
		    }
		  }

	  free(obj) ;
	  node = node->right ;
	}

  dx_list_delete_list(params);

  return NULL ;
}

enum hdr_user_func_result hdr_run_user_function(PHDR_INTERPRETER inter,PHDR_COMPLEX_TOKEN token,PHDR_VAR* result,bool as_go) 
{
	/*
	  now we have to search for the function in the function list. if we do not find it we return hdr_user_func_not_found,
	  if the obj is not NULL then the search is done in the object function list first and then in the scripts function list.
	  If the as_go (special calling for accessing the local variables) is true then the user function parend code block will be set to the interpreter current code 
	*/

	PHDR_CUSTOM_FUNCTIONS_LIST lst = inter->funcs ;
	if(inter->curr_obj != NULL) lst = inter->curr_obj->functions ;

	PHDR_CUSTOM_FUNCTION func = hdr_custom_functions_list_find_gen( lst , token->ID) ;
	if(func == NULL)
	{
		if(inter->curr_obj != NULL)
		if(lst == inter->curr_obj->functions) func = hdr_custom_functions_list_find_gen( lst , token->ID) ;
	}

	if(func == NULL) return hdr_user_func_not_found ;
	/*
	  The function exists , we have to set the interpreter to execute the function code
	*/

	/*check the parameters count*/
	if(func->parameters->count != token->parameters->count)
	{
	  printf("Fatal Error -> Line : %d The user function [%s] needs %d parameters. %d was provided.\n", 
		  inter->current_instr->instr->line,token->ID->stringa,func->parameters->count,token->parameters->count);
	  return hdr_user_func_error ;
	}

	/*we need to setup the parameters of the function and initialize all the variables (not so neccessary but better safe than sorry)*/

	/*set the local block as the parent to permit full access to the local variables, albeit the globals will not be accessible*/
	if(as_go == true)
	{
	 /*implement the as_go special calling*/
	 func->code->parent = inter->current_block ;
	}

	hdr_user_func_code_init_var(func->code->variables) ;
	/*
	   now , the parameters setup is tricky, we will get the name of the first parameter , and we will retrieve the 
	   first actual parameter. We will find the variable with the same name as the parameter in the block's variables
	   and we will set the pointer of the variable hash table of the block to the parameter variable.
	*/

	/* another part that needs special handling is that the parameters can be temporary created 
	   variables AND MUST be FREED before this function exits. 
	*/

	PHDR_INTER_STORE saved_state = NULL ;

	/*create a list to store the parameters */
	PDX_LIST params = dx_list_create_list() ;
	if(params == NULL) 
	{
	 
		printf("Fatal Error -> Line : %d INTERNAL ERROR ON PARAM LIST CREATE: MEMORY NOT ALLOCATED.\n", inter->current_instr->instr->line);
		/*restore the parent block*/
		if(as_go == true)
		{
		 /*implement the as_go special calling*/
		 func->code->parent = inter->code ;
		}
		goto error ;
	}

	PDXL_NODE node = func->parameters->start  ;
	int indx = 0 ;
	while(node != NULL)
	{
	  PDX_STRING  pname    = node->object->key ;
	  PHDR_VAR    rparam   = hdr_inter_resolve_extract_var_from_expr(inter, token->parameters, indx);
	  if(rparam == NULL)
	  {
		/*restore the parent block*/
		if(as_go == true)
		{
		 /*implement the as_go special calling*/
		 func->code->parent = inter->code ;
		}

	    printf("Fatal Error -> Line : %d The function parameter [%s] was not found in the scope \n", inter->current_instr->instr->line,pname->stringa);
		
		params = hdr_user_func_h_release_params(params) ;
		goto error ;
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
	saved_state = hdr_inter_store(inter);
	/*reset the loop counter*/
	inter->loops = 0 ;
	if (saved_state == NULL)
	{
		params = hdr_user_func_h_release_params(params) ;
		printf("Fatal Error -> Line : %d INTERNAL ERROR ON SAVED_STATE: MEMORY NOT ALLOCATED.\n", inter->current_instr->instr->line);
		if(as_go == true)
		{
		 /*implement the as_go special calling*/
		 func->code->parent = inter->code ;
		}

		goto error ;
	}

	/*We do not permit a function to call itself without the special command  [detach] */
	if(inter->curr_func != NULL)
	{
	  if ((dx_string_native_compare(inter->curr_func->name,func->name) == dx_equal)&&(inter->curr_func->object == func->object))
	  {
	    params = hdr_user_func_h_release_params(params) ;
		printf("Fatal Error -> The function [%s] cannot call itself without the detach command. Example : detach Myfunction();\n",token->ID->stringa);

		goto error ;
	  }
	}

	inter->in_func   = true   ;
	inter->curr_func = func   ;
	hdr_inter_set_block(inter, func->code);

	/*execute the function*/
	enum exec_state exec_state = hdr_inter_execute_instructions(inter) ;
	if ( exec_state == exec_state_error)
	{
		params = hdr_user_func_h_release_params(params) ;
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
	if(as_go == true)
	{
	 /*implement the as_go special calling*/
	 func->code->parent = inter->code ;
	}

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
		/*
		 We will chech the ppointer for NULL value. This in normal conditions is not possible (to be null). BUT if the Hydra+ proccess
		 terminates while its threads are running then this phenomenon may occur
		*/
		if(ppointer!=NULL) ppointer->obj = NULL ; /*set this to NULL because it was a absolute reference to the parameter variable*/
		indx++ ;
		node = node->right ;
	}

	/*restore interpreter state*/
	if(as_go == true)
	{
	 /*implement the as_go special calling*/
	 func->code->parent = inter->code ;
	}
	if(saved_state != NULL) saved_state = hdr_inter_restore(inter, saved_state);
	return hdr_user_func_error ;
}






/*******************************************************************************/

bool hdr_inter_check_param_count_error(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token,int valid_params )
{
	/*return true if all its ok */
	if (valid_params == token->parameters->count) return true;
	PHDR_INSTRUCTION instr = inter->last_instr;
	if (instr == NULL) printf(" Fatal Error -> NULL instruction. A bug most propably!");
	else
	{
		    printf("[%s] Fatal Error -> Line %d . The function '%s' expecting %d parameters but %d was provided.\n", 
			instr->filename->stringa, (int)instr->line,token->ID->stringa,valid_params,(int)token->parameters->count);
	}

	return false;
}

bool hdr_inter_check_param_type_error(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token , PHDR_VAR var , enum hdr_var_type valid_type , int param_pos)
{
	/*return true if all its ok*/
	if (var->type == valid_type) return true;

	PHDR_INSTRUCTION instr = inter->last_instr;
	if (instr == NULL) printf(" Fatal Error -> NULL instruction. A bug most propably!");
	else
	{
		printf("[%s] Fatal Error -> Line %d . The function '%s' parameter %d is of an incompatible type. Expected type : %s , type provided : %s.\n",
			instr->filename->stringa, instr->line, token->ID->stringa, param_pos,hdr_inter_return_variable_type(valid_type),
			hdr_inter_return_variable_type(var->type));
	}

	return false;
}

bool hdr_inter_check_param_type_num_error(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token, PHDR_VAR var, int param_pos)
{
	/*returns true if the parameter is of the numeric type*/
	if ((var->type == hvt_float)||(var->type == hvt_integer)) return true;

	PHDR_INSTRUCTION instr = inter->last_instr;
	if (instr == NULL) printf(" Fatal Error -> NULL instruction. A bug most propably!");
	else
	{
		printf("[%s] Fatal Error -> Line %d . The function '%s' parameter %d is of an incompatible type. Expected type : Numeric , type provided : %s.\n",
			instr->filename->stringa, instr->line, token->ID->stringa, param_pos,hdr_inter_return_variable_type(var->type));
	}

	return false;
}


bool hdr_inter_check_param_type_simple_str_error(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token, PHDR_VAR var, int param_pos)
{
	/*return true if the parameter is of the simple string type*/
	if ((var->type == hvt_simple_string) || (var->type == hvt_simple_string_bcktck)) return true;

	PHDR_INSTRUCTION instr = inter->last_instr;
	if (instr == NULL) printf(" Fatal Error -> NULL instruction. A bug most propably!");
	else
	{
		printf("[%s] Fatal Error -> Line %d . The function '%s' parameter %d is of an incompatible type. Expected type : Simple String , type provided : %s.\n",
			instr->filename->stringa, instr->line, token->ID->stringa, param_pos, hdr_inter_return_variable_type(var->type));
	}

	return false;
}


void hdr_inter_param_free(PHDR_VAR param)
{
    if (param->var_ref == hvf_temporary_ref)
	{
		if(hdr_inter_var_gen_type(param) == hdr_ivt_string)
		dx_string_free(((PDX_STRING)param->obj));
		
		hdr_var_release_obj(param) ;
		hdr_var_free(param);
	}
	return ;
}

PHDR_VAR hdr_inter_retrieve_var_from_block(PHDR_INTERPRETER inter, PDX_STRING varname)
{
	/*
	   the function searches for the varname in the block and if it does not find
	   a var searches and the parent and the parent's parent etc
	*/

	PHDR_VAR var = NULL;
	PHDR_BLOCK block = inter->current_block;
	while (block != NULL)
	{
		var = hdr_var_list_find_var(inter->current_block->variables, varname);
		if (var != NULL) return var;
		block = block->parent; /*check its parent*/
	}

	return NULL;
}

PHDR_VAR hdr_inter_resolve_extract_var_from_expr(PHDR_INTERPRETER inter ,PHDR_EXPRESSION_LIST params , int param_pos)
{
	/*
	 the parameters of a function can be a full fledged expression.
	 But the functions needs a single variable to operate.
	 Now we have another pecularity. If an expression is a true expression, 
	 meaning that its a calculation or a function that needs resolving,
	 the returned variable will be a temporary variable and can be assigned BUT
	 the value will be lost.
	 ONLY if the expression returns an object that is by its nature a reference inside the variable
	 the function can change the actual state of the object. 
	 Ofcourse we have to pass a pure variable (the expression has only one token and its a variable) as it self so the 
	 functions can safelly alter the state and be persistent.

	 NOTE : There is a problem with the temporary variables when are passed in the nested functions.
	 For example , if the f1("some literal"){f2($the_parameter_of_f1){}} executes then the "some literal"
	 will deallocate two times. To avoid this , when the type of a block variable is 
	 hvf_temporary_ref_user_func (meaning is already passed as a literal parameter up in the tree) the fucntion will COPY the variable and not pass it as reference 

	*/
	PDXL_NODE       node = dx_list_go_to_pos(params, param_pos) ;
	PDXL_OBJECT     obj  = node->object							;
	PHDR_EXPRESSION expr = (PHDR_EXPRESSION)obj->obj			;
	PHDR_VAR var  = NULL ;
	PHDR_VAR rvar = NULL ;
	if (expr->tokens->count == 1)
	{
		PHDR_COMPLEX_TOKEN token = hdr_inter_return_list_token(expr->tokens, 0);

		if (token->type == hdr_tk_variable)
		{
			/*return the actual variable*/
			rvar = hdr_inter_retrieve_var_from_block(inter, token->ID);
			if (rvar == NULL) return NULL;
			/*check if the variable is a literal thta has been passed from other functions down to this function*/
			if (rvar->var_ref == hvf_temporary_ref_user_func)
			{
			  
			  var = hdr_var_create(NULL,rvar->name->stringa,hvf_temporary_ref_user_func,NULL) ;
			  var->type = rvar->type ;
			  if ((rvar->type == hvt_simple_string)||(rvar->type == hvt_simple_string_bcktck))
			  {
				PDX_STRING str = (PDX_STRING)rvar->obj ;
			    hdr_var_set_obj(var,dx_string_createU(NULL,str->stringa)) ;
			  } 
			   else 
				  if((rvar->type == hvt_integer)||(rvar->type == hvt_codepoint)||(rvar->type == hvt_float)||(rvar->type == hvt_bool))
				  {
		            var->real     = rvar->real    ;
					var->integer  = rvar->integer ;
				  }
				  else
				  {
				    printf("The variable type for the literal that was passed as a parameter was not one of the simple types. How is this possible ?.\n");
					return NULL ;
				  }
			}
			else
				var = rvar ;
			return var; /*handle the NULL in the caller*/
		}
	}

	/*the expression is a true expression resolve it*/
	bool error;
	rvar = hdr_inter_resolve_expr(inter,expr,&error);
	/*a parameter can NEVER being NULL*/
	if (rvar == NULL) return NULL;

	/*the hdr_inter_resolve_expr returns a temporary variable object that is valid only until the function return , so copy it to a variable*/
	PHDR_VAR res = hdr_var_clone(rvar, "", inter->current_block);
	/*reset var */
	rvar->type = hvt_undefined;
	hdr_var_release_obj(rvar) ;

	res->var_ref = hvf_temporary_ref;
	return res;
}


/*helper functions for the system functions handling*/

PHDR_SYS_FUNC_PARAMS hdr_sys_func_init_params(PHDR_INTERPRETER inter ,PHDR_EXPRESSION_LIST params,int expected_count)
{
	 /*we will initialize the structure with the params of the system function*/

	 PHDR_SYS_FUNC_PARAMS sparams = (PHDR_SYS_FUNC_PARAMS)malloc(sizeof(struct hdr_sys_func_params)) ;
	 if(sparams == NULL) return NULL ;
	 sparams->param_count = params->count ;
	 if(params->count > 20) 
	 {
	  printf("The system functions cannot have more than 20 parameters\n");
	  free(sparams);
	  return NULL ;
	 }

	 if(expected_count != params->count)
	 {
	  printf("The function expects %d parameters but %d was provided\n",expected_count,params->count);
	  free(sparams);
	  return NULL ;
	 }

	 /*fill the array*/
	 for(int i = 0 ; i < params->count;i++)
	 {
      sparams->params[i] = hdr_inter_resolve_extract_var_from_expr(inter ,params , i);
	  if(sparams->params[i] == NULL) 
	  {
	   printf("The %d parameter of the system function was returned as NULL. This is forbidden.\n",i+1);
	   return NULL ;
	  }
	 }
	 return sparams ;

}
PHDR_SYS_FUNC_PARAMS hdr_sys_func_free_params(PHDR_SYS_FUNC_PARAMS sys_params) 
{
	 /*the function release the system functions params IF the params must be freed*/
     for(int i = 0;i < sys_params->param_count;i++)
	 {
		if ((sys_params->params[i]->var_ref == hvf_temporary_ref)||(sys_params->params[i]->var_ref == hvf_temporary_ref_user_func)) /* 06-09-2024*/
		{
			if(hdr_inter_var_gen_type(sys_params->params[i]) == hdr_ivt_string)
			dx_string_free(((PDX_STRING)sys_params->params[i]->obj));
			hdr_var_release_obj(sys_params->params[i]) ;
			hdr_var_free(sys_params->params[i]);
		}
	 }
	 free(sys_params);
	 return NULL ;
}




/*
**************************ADD DOMAINS AND OTHER MODULES*****************************************
*/

#include <thirdparty/base64/b64.h>
#include "hydra_dates.h"
#include "hydra_json.h"
#include "hydra_xml.h"
#include "hydra_dom_lists.h"
#include "hydra_dom_strings.h"
#include "hydra_dom_bytes.h"
#include "hydra_dom_hydra.h"
#include "hydra_dom_files.h"
#include "hydra_dom_sockets.h"
#include "hydra_dom_db.h"
#include "hydra_utils.h"
#include "hydra_images.h"

/*
*******************************************************************************
*/


bool hdrGetTickCount(PHDR_INTERPRETER inter , PHDR_COMPLEX_TOKEN token, PHDR_VAR *result)
{
	if (hdr_inter_check_param_count_error(inter, token, 0) == false) return true;

	*result = hdr_var_create(NULL, "", hvf_temporary, NULL);
	(*result)->type = hvt_integer;
	(*result)->integer = dxGetTickCount();

	return false;
}

bool hdrInc(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token)
{
	/*check param count*/
	if (hdr_inter_check_param_count_error(inter, token, 1) == false) return true;
	/*return the variable from the expression*/
	PHDR_VAR var = hdr_inter_resolve_extract_var_from_expr(inter, token->parameters, 0);
	if (var == NULL)
	{
		printf("The 'inc()' parameter was returned as NULL.\n");
		return true;
	}
	if (var->var_ref == hvf_temporary_ref)
	{
		printf("The 'inc()' parameter MUST be a simple numeric variable and not an expression.\n");
		if (hdr_inter_var_gen_type(var) == hdr_ivt_string)
		dx_string_free(((PDX_STRING)var->obj));
		hdr_var_release_obj(var) ;
		hdr_var_free(var);
		return true;
	}
	/*check param types*/
	if (hdr_inter_check_param_type_num_error(inter, token, var, 1) == false)	return true;

	/*the function does not return a value , only sets a +1 in the first parameter */
	if (var->type == hvt_integer)
		var->integer++;
	else
		var->real++;

	return false;
}

bool hdrDec(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token)
{
	/*check param count*/
	if (hdr_inter_check_param_count_error(inter, token, 1) == false) return true;
	/*return the variable from the expression*/
	PHDR_VAR var = hdr_inter_resolve_extract_var_from_expr(inter, token->parameters, 0);
	if (var == NULL)
	{
		printf("The dec($num : Numeric) parameter was returned as NULL.\n");
		return true;
	}
	if (var->var_ref == hvf_temporary_ref)
	{
		printf("The dec($num : Numeric) parameter MUST be a simple numeric variable and not an expression.");
		if (hdr_inter_var_gen_type(var) == hdr_ivt_string)
		dx_string_free(((PDX_STRING)var->obj));
		hdr_var_release_obj(var) ;
		hdr_var_free(var);
		return true;
	}
	/*check param types*/
	if (hdr_inter_check_param_type_num_error(inter, token, var, 1) == false)	return true;

	/*the function does not return a value , only sets a +1 in the first parameter */
	if (var->type == hvt_integer)
		var->integer--;
	else
		var->real--;


	return false;
}

bool hdrEchoHydra(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token)
{
	/*check param count*/
	if (hdr_inter_check_param_count_error(inter, token, 0) == false) return true;
	 
printf("                                 $$$$$$$$                             \n");
printf("                            $$$$$$$$$$$$                              \n");
printf("                          $$$$$$$$$$$$$$$$                            \n");
printf("                  $$$$$$$$$$$$$$$$$$$$$$                 $$$$$$$$     \n");
printf("                  $$$$$$$$$$$$$$$$$$$$$$$           $$$$$$$$$$$$$$    \n");
printf("                       $$$$$$$$$$$$$$$$$$$  $$$$$$$$$$$$$$$$$$$$$$$   \n");
printf("                   $$$$$$$$$$$$$$$$$$$$$   $$$$$$$$$$$$$$$$$$$$$$     \n");
printf("                    $$$$$$$$$$$$$$$$$$$$$   $$$$$$$$$$$$$$$$$$$$$$$   \n");
printf("                         $$$$$$$$$$$$           $$$$$$$$$$$$$$$$$     \n");
printf("                         $$$$$$$$$$$$       $$$$$$$$$$$$$$$$$$$$$$    \n");
printf("              $$$$$$$$   $$$$$$$$$$$$            $$$$ $$$$$$$$$       \n");
printf("         $$$$$$$$$$$$    $$$$$$$$$$$$             $$$  $$$$$$$        \n");
printf("       $$$$$$$$$$$$$$$$  $$$$$$$$$$$$             $$$  $$$$$$$        \n");
printf("$$$$$$$$$$$$$$$$$$$$$$   $$$$$$$$$$$$$            $$$  $$$$$$$        \n");
printf("$$$$$$$$$$$$$$$$$$$$$$   $$$$$$$$$$$$$$$$$        $$$  $$$$$$$        \n");
printf("    $$$$$$$$$$$$$$$$$$$  $$$$$$$$$$$$$$$$$$       $$$  $$$$$$$        \n");
printf("$$$$$$$$$$$$$$$$$$$$$    $$$$$$$$$$$$$$$$$$$      $$$ $$$$$$$$        \n");
printf(" $$$$$$$$$$$$$$$$$$$$$     $$$$$$$$$$$$$$$$$$   $$$$$$$$$$$$$$        \n");
printf("      $$$$$$$$$$$$$          $$$$$$$$$$$$   $$$$$$$$$$$$$$$$$$        \n");
printf("      $$$$$$$$$$$$$$            $$$$$$    $$$$$$$$$$$$$$$$$$$$        \n");
printf("      $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$    $$$$$$$$$$$$$$$$$$$$$         \n");
printf("       $$$$$$$$$$$$$$$$$$$$$$$$$$$$    $$$$$$$$$$$$$$$$$$$$$          \n");
printf("        $$$$$$$$$$$$$$$$$$$$$$$$$$    $$$$$$$$$$$$$$$$$$$$            \n");
printf("          $$$$$$$$$$$$$$$$$$$$$$$    $$$$$$$$$$$$$$$$$$   $$$$$$$$$   \n");
printf("            $$$$$$$$$$$$$$$$$$$$     $$$$$$$$           $$$$$$$$$$$   \n");
printf("              $$$$$$$$$$$$$$$$$$     $$$$$$$$         $$$$$$$$$       \n");
printf("                            $$$$$    $$$$$$$$        $$$$$$$$$        \n");
printf("                               $$$$  $$$$$$$$       $$$$$$$$$$        \n");
printf("                                $$$  $$$$$$$$       $$$$$$$$$$        \n");
printf("                                $$$  $$$$$$$$       $$$$$$$$$$        \n");
printf("   $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$    \n");
printf(" $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$  \n");
printf("$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$\n");
printf("$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$\n");
printf("$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$\n");
printf("$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$\n");
printf("  $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$  \n");
printf("  $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$  $\n");

  
	return false;
}

bool hdrEcho(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token)
{
	/*check param count*/
	if (hdr_inter_check_param_count_error(inter, token, 1) == false) return true;

	PHDR_VAR var = hdr_inter_resolve_extract_var_from_expr(inter, token->parameters, 0);
	if (var == NULL)
	{
		printf("The 'echo()' parameter was returned as NULL.\n");
		return true;
	}

	if(var->type == hvt_undefined)
	{
	  printf("The variable that is set in the echo(%s) is not defined.\n",var->name->stringa);
	  goto fail ;
	}

	/*the echo function does not return a value , it only echo the resulting string in the standard output */
	enum hdr_inter_vt gent = hdr_inter_var_gen_type(var);
	if (gent == hdr_ivt_other)
	{

		if ((var->type == hvt_complex_string) || (var->type == hvt_complex_string_resolve))
		{
			if (var->type == hvt_complex_string)
			{
				PDX_STRINGLIST lst = (PDX_STRINGLIST)var->obj;
				PDXL_NODE node = lst->start;
				while (node != NULL)
				{
					PDXL_OBJECT obj = node->object;
					printf(obj->key->stringa);
				    node = node->right ;
				}
				printf("\n");
			}
			else
			{
				/*
				 echo the current values of the lines
				*/

				PDX_STRINGLIST lst = ((PHDR_COMPLEX_STR_RES)var->obj)->text_lines;
				PDXL_NODE node = lst->start;
				while (node != NULL)
				{
					PDXL_OBJECT obj = node->object;
					printf(obj->key->stringa);
				    node = node->right ;
				}
				printf("\n");

			}

		}
		else 
		if(var->type == hvt_pointer)
		{
		 printf("%p\n",var->obj);
		}
		else
		if(var->type == hvt_bytes)
		{
		  /*print it as characters*/
		  PHDR_BYTES bytes = (PHDR_BYTES)var->obj ;
		  for(DXLONG64 i = 0 ; i < bytes->length ; i++)
		  printf("%c",bytes->bytes[i]) ;
		  printf("\n");
		}
		else
		{
			printf("Echo can print only numeric and string types.\n");
			goto fail ;
		}

	}
	else
	if (gent == hdr_ivt_numeric)
	{
		if (var->type == hvt_float) printf("%f\n", var->real);
		else
			printf("%d\n", var->integer);
	}
	else
	if(var->type == hvt_codepoint)
	{
		/*convert the codepoint to a utf8 char and print it*/
		char buffer[5] ;
		int len = dxConvertUint32ToUTF8(buffer, (uint32_t)var->integer);
		if(len == 0) 
		{
			/*no fatal error but a warning will be emmited*/
			if(inter->warnings == true) printf("A UTF codepoint was not converted to utf8 character succesfully. Codepoint : %d",var->integer);
		}
		else
		{
		buffer[len] = 0 ; /*terminate the buffer*/
		printf("%s\n",buffer);
		}

	}else
	if((var->type == hvt_simple_string)||(var->type == hvt_simple_string_bcktck))
	{
		/*string*/
		printf("%s\n", ((PDX_STRING)var->obj)->stringa);

	}else
    if(var->type == hvt_null)
	{
		/*string*/
		printf("%s\n", "NULL");

	}else
	if(var->type == hvt_unicode_string)
	{
		/*unicode string*/
		PDX_STRING str = (PDX_STRING)var->obj ;
		for(DXLONG64 i = 0 ; i < str->len;i++)
		{
			char utf8c[5] ;
			dxConvertUint32ToUTF8(utf8c,str->stringx[i]);
			printf("%s",utf8c);
		}

		printf("\n") ;
	}
	else
	{
	 printf("Echo does not support the data type [%s]\n",hdr_inter_return_variable_type(var->type));
	 goto fail ;
	}

success :
	/*the memory that is returned in an expression is temporary so we will free the string to not become "dead" memory if the variable is not a permanent one*/
		if (var->var_ref == hvf_temporary_ref)
		{
			if (hdr_inter_var_gen_type(var) == hdr_ivt_string)
			dx_string_free(((PDX_STRING)var->obj));
			hdr_var_release_obj(var);
			var = hdr_var_free(var);
		}
	return false;

	fail :
	/*the memory that is returned in an expression is temporary so we will free the string to not become "dead" memory if the variable is not a permanent one*/
		if (var->var_ref == hvf_temporary_ref)
		{
			if (hdr_inter_var_gen_type(var) == hdr_ivt_string)
			dx_string_free(((PDX_STRING)var->obj));
			hdr_var_release_obj(var);
			var = hdr_var_free(var);
		}
	return true;
}

bool hdrWrite(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token)
{
	/*check param count*/
	if (hdr_inter_check_param_count_error(inter, token, 1) == false) return true;

	PHDR_VAR var = hdr_inter_resolve_extract_var_from_expr(inter, token->parameters, 0);
	if (var == NULL)
	{
		printf("The 'write($var)' parameter was returned as NULL.\n");
		return true;
	}

	if(var->type == hvt_undefined)
	{
	  printf("The variable that is set in the write(%s) is not defined.\n",var->name->stringa);
	  goto fail ;
	}

	/*the echo function does not return a value , it only echo the resulting string in the standard output */
	enum hdr_inter_vt gent = hdr_inter_var_gen_type(var);
	if (gent == hdr_ivt_other)
	{

		if ((var->type == hvt_complex_string) || (var->type == hvt_complex_string_resolve))
		{
			if (var->type == hvt_complex_string)
			{
				PDX_STRINGLIST lst = (PDX_STRINGLIST)var->obj;
				PDXL_NODE node = lst->start;
				while (node != NULL)
				{
					PDXL_OBJECT obj = node->object;
					printf(obj->key->stringa);
				    node = node->right ;
				}

			}
			else
			{
				/*
				 echo the current values of the lines
				*/

				PDX_STRINGLIST lst = ((PHDR_COMPLEX_STR_RES)var->obj)->text_lines;
				PDXL_NODE node = lst->start;
				while (node != NULL)
				{
					PDXL_OBJECT obj = node->object;
					printf(obj->key->stringa);
				    node = node->right ;
				}

			}

		}
		else 
		if(var->type == hvt_pointer)
		{
		 printf("%p",var->obj);
		}
		else
		if(var->type == hvt_bytes)
		{
		  /*print it as characters*/
		  PHDR_BYTES bytes = (PHDR_BYTES)var->obj ;
		  for(DXLONG64 i = 0 ; i < bytes->length ; i++)
		  printf("%c",bytes->bytes[i]) ;
		}
		else
		{
			printf("write can print only numeric and string types.\n");
			goto fail ;
		}

	}
	else
	if (gent == hdr_ivt_numeric)
	{
		if (var->type == hvt_float) printf("%f", var->real);
		else
			printf("%d", var->integer);
	}
	else
	if(var->type == hvt_codepoint)
	{
		/*convert the codepoint to a utf8 char and print it*/
		char buffer[5] ;
		int len = dxConvertUint32ToUTF8(buffer, (uint32_t)var->integer);
		if(len == 0) 
		{
			/*no fatal error but a warning will be emmited*/
			if(inter->warnings == true) printf("A UTF codepoint was not converted to utf8 character succesfully. Codepoint : %d",var->integer);
		}
		else
		{
		buffer[len] = 0 ; /*terminate the buffer*/
		printf("%s",buffer);
		}

	}else
	if((var->type == hvt_simple_string)||(var->type == hvt_simple_string_bcktck))
	{
		/*string*/
		printf("%s", ((PDX_STRING)var->obj)->stringa);

	}else
    if(var->type == hvt_null)
	{
		/*string*/
		printf("%s", "NULL");

	}else
	if(var->type == hvt_unicode_string)
	{
		/*unicode string*/
		PDX_STRING str = (PDX_STRING)var->obj ;
		for(DXLONG64 i = 0 ; i < str->len;i++)
		{
			char utf8c[5] ;
			dxConvertUint32ToUTF8(utf8c,str->stringx[i]);
			printf("%s",utf8c);
		}

	}
	else
	{
	 printf("write does not support the data type [%s]\n",hdr_inter_return_variable_type(var->type));
	 goto fail ;
	}

success :
	/*the memory that is returned in an expression is temporary so we will free the string to not become "dead" memory if the variable is not a permanent one*/
		if (var->var_ref == hvf_temporary_ref)
		{
			if (hdr_inter_var_gen_type(var) == hdr_ivt_string)
			dx_string_free(((PDX_STRING)var->obj));
			hdr_var_release_obj(var);
			var = hdr_var_free(var);
		}
	return false;

	fail :
	/*the memory that is returned in an expression is temporary so we will free the string to not become "dead" memory if the variable is not a permanent one*/
		if (var->var_ref == hvf_temporary_ref)
		{
			if (hdr_inter_var_gen_type(var) == hdr_ivt_string)
			dx_string_free(((PDX_STRING)var->obj));
			hdr_var_release_obj(var);
			var = hdr_var_free(var);
		}
	return true;
}

bool hdrStrLen(PHDR_INTERPRETER inter , PHDR_COMPLEX_TOKEN token, PHDR_VAR *result)
{
	if (hdr_inter_check_param_count_error(inter, token, 1) == false) 
	{
		printf("The strLen(`some string`) function needs a simple string as a parameter.\n");
		return true;
	}

	PHDR_VAR var = hdr_inter_resolve_extract_var_from_expr(inter, token->parameters, 0);
	if (var == NULL)
	{
		printf("The 'strLen()' parameter was returned as NULL.\n");
		return true;
	}

	if(var->type == hvt_undefined)
	{
	  printf("The variable %s that is set in the strLen() is not defined.\n",var->name->stringa);
	  goto fail ;
	}

	if((var->type != hvt_simple_string)&&(var->type != hvt_simple_string_bcktck)&&(var->type != hvt_unicode_string))
	{
	 printf("The strLen() function can return the characters length of a simple string only. To retrieve the length of a complex string variable use the $some_str.Length() function.\n");
	 goto fail ;
	}

	*result = hdr_var_create(NULL, "", hvf_temporary, NULL);
	(*result)->type    = hvt_integer;
	(*result)->integer = ((PDX_STRING)var->obj)->len ;

	/*the memory that is returned in an expression is temporary so we will free the string to not become "dead" memory if the variable is not a permanent one*/
	if (var->var_ref == hvf_temporary_ref)
	{
		if (hdr_inter_var_gen_type(var) == hdr_ivt_string)
		dx_string_free(((PDX_STRING)var->obj));
		hdr_var_release_obj(var);
		hdr_var_free(var);
	}

	return false;

	fail :

		if (var->var_ref == hvf_temporary_ref)
		{
			if (hdr_inter_var_gen_type(var) == hdr_ivt_string)
			dx_string_free(((PDX_STRING)var->obj));
			hdr_var_release_obj(var);
			hdr_var_free(var);
		}

	return true ;
}

bool hdrIsNull(PHDR_INTERPRETER inter , PHDR_COMPLEX_TOKEN token, PHDR_VAR *result)
{
PHDR_SYS_FUNC_PARAMS params = hdr_sys_func_init_params(inter,token->parameters,1) ;
   if(params == NULL)
   {
    printf("The system function isNull($var):Boolean failed.\n");
    return true ;
   }
   
   PHDR_VAR var = params->params[0] ; 

   *result = hdr_var_create(NULL, "", hvf_temporary_ref, NULL) ;
   (*result)->type    = hvt_bool  ; 
   (*result)->integer = 0		  ;

   if(var->type == hvt_null) (*result)->integer = 1 ;

   success:
    hdr_sys_func_free_params(params) ;
    return false ;

   fail : 
    printf("The system function isNull($var):Boolean failed.\n");
    hdr_sys_func_free_params(params) ;
    return true ;
}



bool hdrSetAsInt(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token, PHDR_VAR *result)
{
   PHDR_SYS_FUNC_PARAMS params = hdr_sys_func_init_params(inter,token->parameters,1) ;
   if(params == NULL)
   {
    printf("The system function integer($number : Integer) failed.\n");
    return true ;
   }
   
   bool type_error = false ;
   DXLONG64 val = hdr_inter_ret_integer(params->params[0],&type_error) ; 
   if(type_error == true)
   {
	 printf("The parameter must be an numeric value.\n");
     goto fail ;
   }


   *result = hdr_var_create(NULL, "", hvf_temporary_ref, NULL) ;
   (*result)->type    = hvt_integer  ; 
   (*result)->integer = val			 ;

   success:
    hdr_sys_func_free_params(params) ;
    return false ;

   fail : 
    printf("The system function integer($int : Integer) failed.\n");
    hdr_sys_func_free_params(params) ;
    return true ;
}

bool hdrSetAsPointer(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token, PHDR_VAR *result)
{
   /*The pointer type is a special type that Hydra uses to pass informationm to third party libraries and code*/
   PHDR_SYS_FUNC_PARAMS params = hdr_sys_func_init_params(inter,token->parameters,0) ;
   if(params == NULL)
   {
    printf("The system function pointer() failed.\n");
    return true ;
   }

   *result = hdr_var_create(NULL, "", hvf_temporary_ref, NULL) ;
   (*result)->type    = hvt_pointer  ; 

   success:
    hdr_sys_func_free_params(params) ;
    return false ;

   fail : 
    printf("The system function pointer() failed.\n");
    hdr_sys_func_free_params(params) ;
    return true ;
}


bool hdrRunExe(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token, PHDR_VAR *result)
{
   PHDR_SYS_FUNC_PARAMS params = hdr_sys_func_init_params(inter,token->parameters,5) ;
   if(params == NULL)
   {
    printf("The system function runExe($exeName : String , $workDir : String , $parametersList : Stringlist , out->$procId : Pointer , $Success : Boolean):String failed.\n");
    return true ;
   }
   
   PDX_STRING outp = NULL ;

   bool type_error = false ;
   PDX_STRING exe = hdr_inter_ret_string(params->params[0],&type_error) ; 
   if(type_error == true)
   {
	 printf("The first parameter must be a [String].\n");
     goto fail ;
   }

   PDX_STRING work_dir = hdr_inter_ret_string(params->params[1],&type_error) ; 
   if(type_error == true)
   {
	 printf("The second parameter must be a [String].\n");
     goto fail ;
   }

   if(params->params[2]->type != hvt_string_list)
   {
     printf("The third parameter must be a [StringList].\n");
     goto fail ;
   }
   PDX_STRINGLIST exeparam = (PDX_STRINGLIST)(params->params[2]->obj) ;
   
   if(params->params[3]->type != hvt_pointer)
   {
     printf("The fourth parameter must be a [Pointer] that will be filled with the process handle.\n");
     goto fail ;
   }
   PHDR_VAR varpointer  = params->params[3] ;

   if(params->params[4]->type != hvt_bool)
   {
     printf("The fifth parameter must be a [Boolean].\n");
     goto fail ;
   }
   PHDR_VAR succeed = params->params[4] ;
   succeed->integer = 1 ;

   /*run the function*/
   PHDR_PROCESS out_process = NULL ;
   bool error = false ;
   outp = hdr_run_exe(exe,work_dir, exeparam ,&out_process,&error) ;
   
   if(error == true) 
   {
	   hdr_var_release_obj(varpointer)   ;
	   outp = dx_string_createU(outp,"") ;
	   succeed->integer = 0 ;
   }
   
   hdr_var_set_obj(varpointer,out_process) ;
   
   *result = hdr_var_create(NULL, "", hvf_temporary_ref, NULL) ;
   (*result)->type    = hvt_simple_string  ; 
   hdr_var_set_obj(*result,outp) ;


   success:
    hdr_sys_func_free_params(params) ;
    return false ;

	fail : 
	if(outp != NULL) dx_string_free(outp);
    printf("The system function runExe($exeName : String , $workDir : String , $parametersList : Stringlist,out->$procId : Pointer, $Succeed : Boolean):String failed.\n");
    hdr_sys_func_free_params(params) ;
    return true ;
}

bool hdrThreadInfo(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token, PHDR_VAR *result)
{
   PHDR_SYS_FUNC_PARAMS params = hdr_sys_func_init_params(inter,token->parameters,1) ;
   if(params == NULL)
   {
    printf("The system function threadInfo($threadId : String):List failed.\n");
    return true ;
   }
   
   PDX_STRING outp = NULL ;

   bool type_error = false ;
   PDX_STRING thread_id = hdr_inter_ret_string(params->params[0],&type_error) ; 
   if(type_error == true)
   {
	 printf("The first parameter must be a [String].\n");
     goto fail ;
   }

   /*create a new list*/
   PDX_LIST list = dx_list_create_list() ;
   /*get the infos*/
   PHDR_THREAD thread = hdr_thread_list_return_thread(inter->threads,thread_id) ;
   
   PHDR_VAR mess    = hdr_var_create(NULL,"",hvf_dynamic,NULL) ;
   mess->type		= hvt_simple_string ;
   PDX_STRING ns    = dx_string_createU(NULL, "")  ;
   hdr_var_set_obj(mess,ns) ;
   PHDR_VAR running = hdr_var_create(NULL,"",hvf_dynamic,NULL) ;
   running->type    = hvt_bool ;
   running->integer = 0 ;
   PHDR_VAR stopw   = hdr_var_create(NULL,"",hvf_dynamic,NULL) ;
   stopw->type		= hvt_bool ;
   stopw->integer   = 0   ;
   PHDR_VAR exists  = hdr_var_create(NULL,"",hvf_dynamic,NULL) ;
   exists->type     = hvt_bool ;
   exists->integer  = 0 ;


   if(thread != NULL)
   {
	  exists->integer = 1 ;
	  PDX_STRING nstr = dx_string_createU((PDX_STRING)mess->obj, thread->message->stringa)  ;
	  hdr_var_set_obj(mess,nstr)								;
	  if(thread->running == true)         running->integer = 1  ;
	  if(thread->stop_with_error == true) stopw->integer   = 1  ;
   }

   PDXL_OBJECT obj = dxl_object_create() ;
   obj->obj = (void*)mess				 ;
   obj->key = dx_string_createU(NULL,"message") ;
   dx_list_add_node_direct(list,obj)     ;

   obj = dxl_object_create() ;
   obj->obj = (void*)running			 ;
   obj->key = dx_string_createU(NULL,"running") ;
   dx_list_add_node_direct(list,obj)     ;

   obj = dxl_object_create() ;
   obj->obj = (void*)stopw   			 ;
   obj->key = dx_string_createU(NULL,"error") ;
   dx_list_add_node_direct(list,obj)     ;

   obj = dxl_object_create() ;
   obj->obj = (void*)exists   			 ;
   obj->key = dx_string_createU(NULL,"exists") ;
   dx_list_add_node_direct(list,obj)     ;

  
   *result			  = hdr_var_create(list, "", hvf_temporary_ref, NULL)  ;
   (*result)->type    = hvt_list;

   success:
    hdr_sys_func_free_params(params) ;
    return false ;

	fail : 
    printf("The system function threadInfo($thread_id : String):List failed.\n");
    hdr_sys_func_free_params(params) ;
    return true ;
}


bool hdrGetThreadId(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token, PHDR_VAR *result)
{
   PHDR_SYS_FUNC_PARAMS params = hdr_sys_func_init_params(inter,token->parameters,0) ;
   if(params == NULL)
   {
    printf("The system function getThreadId():String failed.\n");
    return true ;
   }
   
   /*get the thread*/
   PDX_STRING thread_id = NULL ;
   if(inter->thread != NULL) thread_id = dx_string_createU(NULL,inter->thread->id->stringa) ;
   else
	   thread_id = dx_string_createU(NULL,"Main") ;

   *result			  = hdr_var_create(thread_id, "", hvf_temporary_ref, NULL)  ;
   (*result)->type    = hvt_simple_string ;

   success:
    hdr_sys_func_free_params(params) ;
    return false ;

	fail : 
    printf("The system function getThreadId():String failed.\n");
    hdr_sys_func_free_params(params) ;
    return true ;
}

/************************************ DOMAINS ********************************************/



/************ NUMERIC ****************************************************************************************/

bool hdr_domNumericToString(PHDR_INTERPRETER inter ,PHDR_COMPLEX_TOKEN token,PHDR_VAR for_var, PHDR_VAR* result)
{

	if (token->parameters->count != 1)
	{
		printf("The ToString($decimals) function needs the number of decimals that the number will display.\n");
		return true;
	}
	
	PHDR_VAR param = hdr_inter_resolve_extract_var_from_expr(inter, token->parameters, 0) ;	
	if (param == NULL)
	{
		printf("ToString($decimals) : The parameter  resolved as NULL.\n");
		return true ;
	}

	if((param->type != hvt_integer)&&(param->type != hvt_float))
	{
	    printf("ToString($decimals) : The parameter is not of a numeric type.\n");
		goto fail ;
	}


	switch(for_var->type)
	{
		case hvt_integer:
		{
		  *result = hdr_var_create(NULL, "", hvf_temporary, NULL);
		  (*result)->type    = hvt_simple_string;
		  PDX_STRING ns      = dx_IntToStr(for_var->integer) ;
		  hdr_var_set_obj(*result,ns) ;
		}break;
		case hvt_float:
		{	
		  int dec = 0 ;
		  if(param->type == hvt_integer)
			  dec = param->integer ;
		  else
			  dec = param->real ;

		  *result = hdr_var_create(NULL, "", hvf_temporary, NULL);
		  (*result)->type    = hvt_simple_string;
		  PDX_STRING nstr    = dx_FloatToStr(for_var->real, dec) ;
		  hdr_var_set_obj(*result,nstr) ;
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

/************ NUMERIC END ************************************************************************************/

/*********************** BOOL ********************************************************************************/

bool hdr_domBoolToString(PHDR_INTERPRETER inter ,PHDR_COMPLEX_TOKEN token,PHDR_VAR for_var, PHDR_VAR* result)
{

	if (token->parameters->count != 0)
	{
		printf("The ToString() function does not needs parameters.\n");
		return true;
	}


		
	*result = hdr_var_create(NULL, "", hvf_temporary, NULL);
	(*result)->type    = hvt_simple_string;
	if( for_var->integer == 0 )
	{
	  PDX_STRING ns = dx_string_createU(NULL,"false") ;
	  hdr_var_set_obj(*result,ns) ;
	}
	else
	{
	  PDX_STRING ns = dx_string_createU(NULL,"true") ;
	  hdr_var_set_obj(*result,ns) ;
	}

		
	return false ;

	fail :
	return true ;

}

/********************************** check variables for validity **************************************************************************/

bool hdr_all_is_undef(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token,PHDR_VAR for_var,PHDR_VAR *result)
{
   PHDR_SYS_FUNC_PARAMS params = hdr_sys_func_init_params(inter,token->parameters,0) ;
   if(params == NULL)
   {
    printf("The system function Variable.IsUndef():Boolean failed.\n");
    return true ;
   }

   *result            = hdr_var_create(NULL, "", hvf_temporary_ref, NULL) ;
   (*result)->type    = hvt_bool    ; 
   (*result)->integer = 0           ;

    if(for_var->type == hvt_undefined) (*result)->integer = 1  ; 

    success:
    hdr_sys_func_free_params(params) ;
    return false ;

    fail : 
    printf("The system function Variable.isUndef():Boolean failed.\n");
    hdr_sys_func_free_params(params) ;
    return true ;
}

/************** return an undefined variable (Is used like the NULL return in C) ************************/
bool hdr_all_ret_undef(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token,PHDR_VAR *result)
{
   PHDR_SYS_FUNC_PARAMS params = hdr_sys_func_init_params(inter,token->parameters,0) ;
   if(params == NULL)
   {
    printf("The system function undef():Variable failed.\n");
    return true ;
   }

   *result            = hdr_var_create(NULL, "", hvf_temporary_ref, NULL) ;
   (*result)->type    = hvt_undefined    ; 
   
    success:
    hdr_sys_func_free_params(params) ;
    return false ;

    fail : 
    printf("The system function undef():Variable failed.\n");
    hdr_sys_func_free_params(params) ;
    return true ;
}

/************** release a variable , this make it undef , if its a string the string is released and 
if its an object the member is set to NULL. The actual object is not being destroyed************************/
bool hdr_all_set_undef(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token,PHDR_VAR for_var)
{
   PHDR_SYS_FUNC_PARAMS params = hdr_sys_func_init_params(inter,token->parameters,0) ;
   if(params == NULL)
   {
    printf("The system function Variable.Release() failed.\n");
    return true ;
   }

   if((for_var->type == hvt_simple_string)||(for_var->type == hvt_simple_string_bcktck)||(for_var->type == hvt_unicode_string))
	  dx_string_free((PDX_STRING)for_var->obj);
	
   hdr_var_release_obj(for_var)  ;
   for_var->type = hvt_undefined ;

    success:
    hdr_sys_func_free_params(params) ;
    return false ;

    fail : 
    printf("The system function Variable.Release() failed.\n");
    hdr_sys_func_free_params(params) ;
    return true ;
}




/************ SPECIAL CLASS FUNCTIONS ***********************************************************************/

bool hdr_domClassReleaseMem(PHDR_VAR for_var)
{
   if(for_var->obj == NULL) return false ;

   hdr_object_class_free((PHDR_OBJECT_INSTANCE)for_var->obj) ;
   hdr_var_release_obj(for_var) ;
   for_var->type = hvt_undefined ;

   return false ;
}





/************************************************************************************************************/

bool hdr_inter_handle_function(PHDR_INTERPRETER inter ,PHDR_COMPLEX_TOKEN token,PHDR_VAR for_var, PHDR_VAR* result)
{
	/* return true for error and false for success*/

	/*
	 the for_var is the variable that calls an inate function (or NULL). Example :
	 $str.Concat() the for_var is the $str variable.
	*/

	/* the result  will be used from the functions to return values.
	   If the function does not return a value this will be remain undefined ,
	   this variable will be freed by the caller
	 */

	/*
	 now select the appropriate function. This is a grude and simple 
	 way to do it but maybe in the future IF the functions are trully so many
	 that can have an impact in speed, i will change it to something more sofisticated.
	 For better branching though i will check for a prefix
	*/
	if (for_var == NULL) /*a user custom function or simple native function or domain core function (most likely a creation function)*/
	{
		if (token->ID->stringa[0] != '!')
		{

			if (hdr_inter_fast_str(token->ID, "GetTickCount", 12) == true) { hdrGetTickCount(inter, token, result); goto var_returned; } /* -> the goto is obsolete */
			if (hdr_inter_fast_str(token->ID, "inc", 3) == true) return hdrInc(inter, token);
			if (hdr_inter_fast_str(token->ID, "dec", 3) == true) return hdrDec(inter, token);
			if (hdr_inter_fast_str(token->ID, "echo", 4) == true) return hdrEcho(inter, token);
			if (hdr_inter_fast_str(token->ID, "echoHydra", 9) == true) return hdrEchoHydra(inter, token);
			if (hdr_inter_fast_str(token->ID, "write", 5) == true) return hdrWrite(inter, token);
			if (hdr_inter_fast_str(token->ID, "strLen", 6) == true) return hdrStrLen(inter, token,result);
			if (hdr_inter_fast_str(token->ID, "integer", 7) == true) return hdrSetAsInt(inter, token,result);
			if (hdr_inter_fast_str(token->ID, "pointer", 7) == true) return hdrSetAsPointer(inter, token,result);
			if (hdr_inter_fast_str(token->ID, "runExe", 6) == true) return hdrRunExe(inter, token,result);
			if (hdr_inter_fast_str(token->ID, "threadInfo", 10) == true) return hdrThreadInfo(inter, token,result);
			if (hdr_inter_fast_str(token->ID, "getThreadId", 11) == true) return hdrGetThreadId(inter, token,result);
			if (hdr_inter_fast_str(token->ID, "isNull", 6) == true) return hdrIsNull(inter, token,result);
			
			/*date time functions*/
			
			if (hdr_inter_fast_str(token->ID, "dateToPOSIX", 11) == true)	 return hdr_funcDateToPOSIX(inter, token,result);
			if (hdr_inter_fast_str(token->ID, "nowPOSIX", 8) == true)		 return hdr_funcDateNowPOSIX(inter, token,result);
			if (hdr_inter_fast_str(token->ID, "Now", 3) == true)			 return hdr_funcNow(inter, token,result);
			if (hdr_inter_fast_str(token->ID, "timeToDate", 10) == true)	 return hdr_funcTimeToDate(inter, token,result);
			if (hdr_inter_fast_str(token->ID, "daysDiff", 8) == true)		 return hdr_funcDaysDiff(inter, token,result);
			if (hdr_inter_fast_str(token->ID, "secsDiff", 8) == true)		 return hdr_funcSecDiff(inter, token,result);
			if (hdr_inter_fast_str(token->ID, "addDays", 7) == true)		 return hdr_funcAddDays(inter, token,result);
			if (hdr_inter_fast_str(token->ID, "datesCompare", 12) == true)   return hdr_funcDatesCompare(inter, token,result);
			if (hdr_inter_fast_str(token->ID, "dateInfo", 8) == true)		 return hdr_funcDateInfo(inter, token,result);
			
			/*database functions*/
			if (hdr_inter_fast_str(token->ID, "asString", 8) == true)      return hdr_funcasString(inter, token,result);
			
			/*util functions*/
			if (hdr_inter_fast_str(token->ID, "random", 6) == true)		   return hdr_random(inter, token,result);
			if (hdr_inter_fast_str(token->ID, "random_id", 9) == true)	   return hdr_random_id(inter, token,result);
			if (hdr_inter_fast_str(token->ID, "sleep", 5) == true)		   return hdr_sleep(inter, token,result);
			if (hdr_inter_fast_str(token->ID, "undef", 5) == true)         return hdr_all_ret_undef(inter,token,result) ;	
			if (hdr_inter_fast_str(token->ID, "setCurDir", 9) == true)     return hdr_set_cur_dir(inter,token) ;
			if (hdr_inter_fast_str(token->ID, "setFromChars", 12) == true) return hdr_func_set_from_chars(inter,token, result);
			if (hdr_inter_fast_str(token->ID, "loadFromFile", 12) == true) return hdr_domStringLoadFromFile(inter,token, result);
			if (hdr_inter_fast_str(token->ID, "varExists", 9) == true)     return hdrVarExists(inter, token,result);

			/*file/folder functions*/
			if (hdr_inter_fast_str(token->ID, "createDir", 9) == true)     return hdr_create_dir(inter,token,result)  ;
			if (hdr_inter_fast_str(token->ID, "zipDir", 6) == true)        return hdr_dir_to_zip(inter,token,result)  ;
			if (hdr_inter_fast_str(token->ID, "zipFiles", 8) == true)      return hdr_zip_files(inter,token,result)   ;
			if (hdr_inter_fast_str(token->ID, "extractZip", 10) == true)   return hdr_extract_zip(inter,token,result) ;
			if (hdr_inter_fast_str(token->ID, "getFiles", 8) == true)      return hdr_get_files(inter,token,result)   ;
			if (hdr_inter_fast_str(token->ID, "getDirs", 7) == true)       return hdr_get_dirs(inter,token,result)    ;
			if (hdr_inter_fast_str(token->ID, "getFilesDirs", 12) == true) return hdr_get_files_dirs(inter,token,result)   ;
			if (hdr_inter_fast_str(token->ID, "emptyFile", 9) == true)     return hdr_domEmptyFile(inter, token);
			if (hdr_inter_fast_str(token->ID, "fileExists", 10) == true) return hdr_domFileExists(inter, token,result) ;
			if (hdr_inter_fast_str(token->ID, "dirExists", 9) == true)   return hdr_domDirExists(inter, token,result)    ;
			if (hdr_inter_fast_str(token->ID, "deleteFile", 10) == true) return hdr_domFileRemove(inter, token,result)    ;
			if (hdr_inter_fast_str(token->ID, "emptyDir", 8) == true)    return hdr_domEmptyDir(inter, token,result)      ;
			if (hdr_inter_fast_str(token->ID, "deleteDir", 9) == true)   return hdr_domDeleteDir(inter, token,result)    ;

			/*
			 2024-12-14 I added support for image manipulation , i will keep it simple , im using the stb_image.h and 
			 the other stb_ headers (i love them <3) and i will support image convertion and resize. I do not have 
			 the itent to make more complicated functions or support more exotic features as Hydra+ has another purpose.
			 BUT as i want to use Hydra+ to replace some middlewares that do handle images (from/to eshops) i prefer
			 to habe native support for this simple image operations. For more complex actions the right way is to 
			 call from a Hydra+ script , another executable that will handle the images.
			*/

			/*images functions*/
			if (hdr_inter_fast_str(token->ID, "imageInfo", 9) == true) return hdr_imgImageInfo(inter, token,result) ;
			if (hdr_inter_fast_str(token->ID, "convertImage", 12) == true) return hdr_imgConvertImage(inter, token,result) ;
			if (hdr_inter_fast_str(token->ID, "resizeImage", 11) == true) return hdr_imgResizeImage(inter, token,result) ;


			/* check for a user function. The system functions are always in priority , so no overrides can exists.
			   if a user function has the same name as a system function then the user function will not be executed.
			*/
			enum hdr_user_func_result res = hdr_run_user_function(inter,token,result,false) ;
			if( res == hdr_user_func_value)    goto var_returned ;
			if( res == hdr_user_func_error)    return true       ;
			if( res == hdr_user_func_success)  return false		 ;

		}
		else
		{
			/*the core domain functions start with the ! as it is not a valid character for user functions*/
			if (token->ID->stringa[1] == 'S')
			{
				/*complex string functions*/
				/*create the string*/
				if (hdr_inter_fast_str(token->ID, "!S__CR", 6) == true) return hdr_domStringCreate(inter, token, result);
			}

			if (token->ID->stringa[1] == 'L')
			{
				/*create the list*/
				if (hdr_inter_fast_str(token->ID, "!L__CR", 6) == true) return hdr_domSimpleListCreate(inter, token, result);
				if (hdr_inter_fast_str(token->ID, "!LS_CR", 6) == true) return hdr_domStringListCreate(inter, token, result);
				if (hdr_inter_fast_str(token->ID, "!LF_CR", 6) == true) return hdr_domFastListCreate(inter, token, result);
			}

			if (token->ID->stringa[1] == 'B')
			{
				if (hdr_inter_fast_str(token->ID, "!B__CR", 6) == true) return hdr_domBytesCreate(inter, token, result);
			}
			/*HYDRA+ domain*/
			if (token->ID->stringa[1] == 'H')
			{	
				if (hdr_inter_fast_str(token->ID, "!HS___", 6) == true) return hdr_domHydraScriptPath(inter, token, result);
				if (hdr_inter_fast_str(token->ID, "!HP___", 6) == true) return hdr_domHydraPath(inter, token, result);
				if (hdr_inter_fast_str(token->ID, "!HN___", 6) == true) return hdr_domHydraScriptName(inter, token, result);
			}

			if (token->ID->stringa[1] == 'F')
			{	
				if (hdr_inter_fast_str(token->ID, "!F__CR", 6) == true) return hdr_domFileOpen(inter, token, result);

			}

			if (token->ID->stringa[1] == 'N')
			{	
				if (hdr_inter_fast_str(token->ID, "!NC_CR", 6) == true) return hdr_domTCPClientSocketCreate(inter, token, result);
				if (hdr_inter_fast_str(token->ID, "!NS_CR", 6) == true) return hdr_domTCPServerSocketCreate(inter, token, result);
			}

			if (token->ID->stringa[1] == 'C')
			{	
				if (hdr_inter_fast_str(token->ID, "!C__CR", 6) == true) return hdr_domTCPClientSSLCreate(inter, token, result);
				if (hdr_inter_fast_str(token->ID, "!CS_CR", 6) == true) return hdr_domSSLServerCreate(inter, token, result);
			}

			if (token->ID->stringa[1] == 'D')
			{	
				if (hdr_inter_fast_str(token->ID, "!DL_CR", 6) == true) return hdr_domSqliteOpen(inter,token,result) ;
				if (hdr_inter_fast_str(token->ID, "!DM_CR", 6) == true) return hdr_domMariaDBOpen(inter,token,result);
				if (hdr_inter_fast_str(token->ID, "!DO_CR", 6) == true) return hdr_domODBCOpen(inter,token,result)   ;
			}


		}
	}
	else/********** DOMAIN FUNCTIONS **********************************************/
	{
		/****** Strings************************************************************/

		if(hdr_inter_var_gen_type_str(for_var) == hdr_ivt_string)	
		{
			/*generic string functions , some functions does not support all types of string but the error hadling is in the functions*/
			if(hdr_inter_fast_str(token->ID, "Length", 6) == true)				return hdr_domStringLength(inter, token, for_var,result) ;
			if(hdr_inter_fast_str(token->ID, "ByteCount", 9) == true)			return hdr_domStringByteCount(inter, token, for_var,result) ;
			if(hdr_inter_fast_str(token->ID, "RemoveChar", 10) == true)			return hdr_domStringRemoveChar(inter, token, for_var,result) ;
			if(hdr_inter_fast_str(token->ID, "ReplaceChar", 11) == true)		return hdr_domStringReplaceChar(inter,token,for_var,result) ;
			if(hdr_inter_fast_str(token->ID, "FindWord", 8) == true)		    return hdr_domStringFindWord(inter,token, for_var,result);
			if(hdr_inter_fast_str(token->ID, "FindWordBinary", 14) == true)     return hdr_domStringFindWordBinary(inter,token, for_var,result);
			if(hdr_inter_fast_str(token->ID, "Replace", 7) == true)             return hdr_domStringReplace(inter,token, for_var,result);
			if(hdr_inter_fast_str(token->ID, "ReplaceBinary", 13) == true)      return hdr_domStringReplaceBinary(inter,token, for_var,result);
			if(hdr_inter_fast_str(token->ID, "Empty", 5) == true)				return hdr_domStringEmpty(inter,token, for_var);
			if(hdr_inter_fast_str(token->ID, "CopyIndexBinary", 15) == true)	return hdr_domStringCopyIndxBinary(inter,token, for_var,result);
			if(hdr_inter_fast_str(token->ID, "CopyBinary", 10) == true)			return hdr_domStringCopyBinary(inter,token, for_var,result);
			if(hdr_inter_fast_str(token->ID, "CopyCharBinary", 14) == true)		return hdr_domStringCopyCharBinary(inter,token, for_var,result);
			if(hdr_inter_fast_str(token->ID, "CopyIndex", 9) == true)			return hdr_domStringCopyIndx(inter,token, for_var,result);
			if(hdr_inter_fast_str(token->ID, "Copy", 4) == true)				return hdr_domStringCopy(inter,token, for_var,result);
			if(hdr_inter_fast_str(token->ID, "CopyToChar", 10) == true)			return hdr_domStringCopyChar(inter,token, for_var,result);
			if(hdr_inter_fast_str(token->ID, "TrimLeft", 8) == true)			return hdr_domStringTrimLeft(inter,token, for_var,result);
			if(hdr_inter_fast_str(token->ID, "TrimRight", 9) == true)			return hdr_domStringTrimRight(inter,token, for_var,result);
			if(hdr_inter_fast_str(token->ID, "Trim", 4) == true)				return hdr_domStringTrim(inter,token, for_var);
			if(hdr_inter_fast_str(token->ID, "ToInteger", 9) == true)			return hdr_domStringInt(inter,token, for_var,result);
			if(hdr_inter_fast_str(token->ID, "ToReal", 6) == true)				return hdr_domStringReal(inter,token, for_var,result);
			if(hdr_inter_fast_str(token->ID, "AddFromFile", 11) == true)		return hdr_domStringAddFromFile(inter,token, for_var);
			if(hdr_inter_fast_str(token->ID, "SaveToFile", 10) == true)         return hdr_domStringSaveToFile(inter,token,for_var)		  ;
			/*if the function was not found maybe is for some types of string*/
			if ((for_var->type == hvt_complex_string)||(for_var->type == hvt_complex_string_resolve))
			{
				if (hdr_inter_fast_str(token->ID, "Concat", 6) == true) return hdr_domStringConcat(inter, token, for_var) ;
				if (hdr_inter_fast_str(token->ID, "Assign", 6) == true) return hdr_domStringAssign(inter, token, for_var) ;
				if(for_var->type == hvt_complex_string_resolve) 
				{
				  if(hdr_inter_fast_str(token->ID, "Expand", 6) == true) return hdr_domStringExpand(inter,token,for_var) ;
				  if(hdr_inter_fast_str(token->ID, "ExpandToSimple", 14) == true) return hdr_domStringExpandToSimple(inter,token,for_var,result) ;
				}
				if(hdr_inter_fast_str(token->ID, "ToSimple", 8) == true) return hdr_domStringToSimple(inter,token, for_var,result);
				if(hdr_inter_fast_str(token->ID, "Free", 4) == true) return hdr_domStringReleaseMem(for_var) ;
			}
			else
				if((for_var->type == hvt_simple_string)||(for_var->type == hvt_simple_string_bcktck))
				{
				  if(hdr_inter_fast_str(token->ID, "SetFromChars", 12) == true)	 return hdr_domStringSetFromChars(inter,token, for_var);
				  if(hdr_inter_fast_str(token->ID, "CharPos", 7) == true)		 return hdr_domStringCharPos(inter,token, for_var,result);
				  if(hdr_inter_fast_str(token->ID, "CopyUntilChar", 13) == true) return hdr_domStringCopyUntilChar(inter,token, for_var,result);
				  if(hdr_inter_fast_str(token->ID, "Explode", 7) == true)        return hdr_domStringExplode(inter,token, for_var,result);
				  if(hdr_inter_fast_str(token->ID, "UrlEncode", 9) == true)      return hdr_domStringUrlEncode(inter,token,for_var,result);
				  if(hdr_inter_fast_str(token->ID, "UrlDecode", 9) == true)      return hdr_domStringUrlDecode(inter,token,for_var,result);
				  if(hdr_inter_fast_str(token->ID, "Base64Encode", 12) == true)  return hdr_domStringBase64Encode(inter,token,for_var,result) ;
				  if(hdr_inter_fast_str(token->ID, "Base64Decode", 12) == true)  return hdr_domStringBase64Decode(inter,token,for_var,result) ;
				  if(hdr_inter_fast_str(token->ID, "B64DBinary", 10) == true)    return hdr_domStringBase64DBinary(inter,token,for_var,result) ;
				  if(hdr_inter_fast_str(token->ID, "JsonToList", 10) == true)	 return hdr_domStringToJson(inter,token,for_var,result)	  ;
				  if(hdr_inter_fast_str(token->ID, "XmlToList", 9) == true)	     return hdr_domStringToXml(inter,token,for_var,result)	  ;
				  if(hdr_inter_fast_str(token->ID, "GetExt", 6) == true)		 return hdr_domStringGetExt(inter,token,for_var,result)	  ;
				  if(hdr_inter_fast_str(token->ID, "XorHex", 6) == true)         return hdr_domStringXorHex(inter,token,for_var,result)   ;
				  if(hdr_inter_fast_str(token->ID, "XorHexStr", 9) == true)      return hdr_domStringXorHexStr(inter,token,for_var,result);
				  if(hdr_inter_fast_str(token->ID, "Xor", 3) == true)            return hdr_domStringXor(inter,token,for_var,result)      ;
				  if(hdr_inter_fast_str(token->ID, "Upper", 5) == true)      return hdr_domStringUpper(inter,token,for_var,result)    ;
				  if(hdr_inter_fast_str(token->ID, "Lower", 5) == true)      return hdr_domStringLower(inter,token,for_var,result)    ;
				  if(hdr_inter_fast_str(token->ID, "UpperGr", 7) == true)    return hdr_domStringUpperGr(inter,token,for_var,result)  ;
				  if(hdr_inter_fast_str(token->ID, "LowerGr", 7) == true)    return hdr_domStringLowerGr(inter,token,for_var,result)  ;
				  if(hdr_inter_fast_str(token->ID, "PrepareForJson", 14) == true) return hdr_domStringPrepareFJson(inter,token,for_var,result)    ;
				}

		} 
		else
		if((for_var->type == hvt_integer)||(for_var->type == hvt_float))
		{
			/*numeric types*/
			if(hdr_inter_fast_str(token->ID, "ToString", 8) == true) return hdr_domNumericToString(inter,token, for_var,result);
			
		}
		else
		if(for_var->type == hvt_bool)
		{
			/*numeric types*/
			if(hdr_inter_fast_str(token->ID, "ToString", 8) == true) return hdr_domBoolToString(inter,token, for_var,result);
			
		}
		else
		if(for_var->type == hvt_object)	
		{
			/*handle the object functions :D
			 Keep in mind that the object will search first in its own function list before searches the script functions
			*/
			inter->curr_obj = (PHDR_OBJECT_INSTANCE)for_var->obj ;
			/*first check for the special class functions*/
			if(hdr_inter_fast_str(token->ID, "Free", 4) == true) return hdr_domClassReleaseMem(for_var);
			/*********************************************/
			enum hdr_user_func_result res = hdr_run_user_function(inter,token,result,false) ;
			/*restore the curr_obj as the function ended*/
			inter->curr_obj = NULL ;
			if( res == hdr_user_func_value)    goto var_returned ;
			if( res == hdr_user_func_error)    return true       ;
			if( res == hdr_user_func_success)  return false		 ;
		}
		else
		if(for_var->type == hvt_list)	
		{

			if(hdr_inter_fast_str(token->ID, "Free", 4) == true) return hdr_domSimpleListReleaseMem(for_var);
			if(hdr_inter_fast_str(token->ID, "AtPos", 5) == true) return hdr_domSimpleListAtPos(inter,token, for_var,result);
			if(hdr_inter_fast_str(token->ID, "NamedIndex", 10) == true) return hdr_domSimpleListNamedIndex(inter,token, for_var,result);
			if(hdr_inter_fast_str(token->ID, "SetNamedIndex", 13) == true) return hdr_domSimpleListSetNamedIndex(inter,token, for_var,result);
			if(hdr_inter_fast_str(token->ID, "ChangeNamedIndex", 16) == true) return hdr_domSimpleListChangeNamedIndex(inter,token, for_var,result);
			if(hdr_inter_fast_str(token->ID, "Count", 5) == true) return hdr_domSimpleListCount(inter,token, for_var,result);
			if(hdr_inter_fast_str(token->ID, "Exists", 6) == true) return hdr_domSimpleListExists(inter,token, for_var,result);
			if(hdr_inter_fast_str(token->ID, "Delete", 6) == true) return hdr_domSimpleListDelete(inter,token, for_var,result);
			if(hdr_inter_fast_str(token->ID, "DeleteAll", 9) == true) return hdr_domSimpleListDeleteAll(inter,token, for_var,result);
			if(hdr_inter_fast_str(token->ID, "Remove", 6) == true) return hdr_domSimpleListRemove(inter,token, for_var,result);
			if(hdr_inter_fast_str(token->ID, "RemoveAll", 9) == true) return hdr_domSimpleListRemoveAll(inter,token, for_var,result);
			if(hdr_inter_fast_str(token->ID, "Insert", 6) == true) return hdr_domSimpleListInsert(inter,token, for_var)				;
			if(hdr_inter_fast_str(token->ID, "ToJson", 6) == true) return hdr_domJsonListToString(inter,token,for_var,result)       ;
			if(hdr_inter_fast_str(token->ID, "ListToXmlStr", 12) == true) return hdr_domListToXmlStr(inter,token,for_var,result)	  ;
		}
		else
		if(for_var->type == hvt_fast_list)	
		{
			if(hdr_inter_fast_str(token->ID, "Free", 4) == true) return hdr_domFastListReleaseMem(for_var);
			if(hdr_inter_fast_str(token->ID, "Count", 5) == true) return hdr_domFastListCount(inter,token, for_var,result);
			if(hdr_inter_fast_str(token->ID, "Exists", 6) == true) return hdr_domFastListExists(inter,token, for_var,result);
			if(hdr_inter_fast_str(token->ID, "Delete", 6) == true) return hdr_domFastListDelete(inter,token, for_var,result);
			if(hdr_inter_fast_str(token->ID, "DeleteAll", 9) == true) return hdr_domFastListDeleteAll(inter,token, for_var,result);
			if(hdr_inter_fast_str(token->ID, "Remove", 6) == true) return hdr_domFastListRemove(inter,token, for_var,result);
			if(hdr_inter_fast_str(token->ID, "RemoveAll", 9) == true) return hdr_domFastListRemoveAll(inter,token, for_var,result);
			if(hdr_inter_fast_str(token->ID, "ToJson", 6) == true) return hdr_domJsonObjToString(inter,token,for_var,result)      ;
		}
		else
		if(for_var->type == hvt_string_list)	
		{
			if(hdr_inter_fast_str(token->ID, "Count", 5) == true) return hdr_domStringListCount(inter,token, for_var,result);
			if(hdr_inter_fast_str(token->ID, "Add", 3) == true) return hdr_domStringListAdd(inter,token, for_var);
			if(hdr_inter_fast_str(token->ID, "ToSimple", 8) == true) return hdr_domStringListToSimple(inter,token, for_var,result);
			if(hdr_inter_fast_str(token->ID, "ToSimpleLn", 10) == true) return hdr_domStringListToSimpleLn(inter,token, for_var,result);
			if(hdr_inter_fast_str(token->ID, "Clear", 5) == true) return hdr_domStringListClear(for_var);
			if(hdr_inter_fast_str(token->ID, "Free", 4) == true) return hdr_domStringListReleaseMem(for_var);
			if(hdr_inter_fast_str(token->ID, "LoadFromFile", 12) == true) return hdr_domStringListLoadFromFile(inter,token, for_var);
			if(hdr_inter_fast_str(token->ID, "SaveToFile", 10) == true) return hdr_domStringListSaveToFile(inter,token, for_var);
		}
		else
		if(for_var->type == hvt_bytes)
		{
			if(hdr_inter_fast_str(token->ID, "Length", 6) == true) return hdr_domBytesReturnLen(inter,token, for_var,result);
			if(hdr_inter_fast_str(token->ID, "SetToZero", 9) == true) return hdr_domBytesSetToZero(inter,token, for_var);
			if(hdr_inter_fast_str(token->ID, "Reset", 5) == true) return hdr_domBytesReset(inter,token, for_var);
			if(hdr_inter_fast_str(token->ID, "Free", 4) == true) return hdr_domBytesFree(for_var);
			if(hdr_inter_fast_str(token->ID, "Copy", 4) == true) return hdr_domBytesCopy(inter,token, for_var);	
			if(hdr_inter_fast_str(token->ID, "CopyEx", 6) == true) return hdr_domBytesCopyEx(inter,token, for_var);	
			if(hdr_inter_fast_str(token->ID, "FromUTF8", 8) == true) return hdr_domBytesFromUTF8(inter,token, for_var);	
			if(hdr_inter_fast_str(token->ID, "ToUTF8", 6) == true) return hdr_domBytesToUTF8(inter,token, for_var,result);	
			if(hdr_inter_fast_str(token->ID, "Base64Encode", 12) == true) return hdr_domBytesBase64Encode(inter,token,for_var,result) ;
			if(hdr_inter_fast_str(token->ID, "FindPattern", 11) == true)  return hdr_domBytesFindPattern(inter,token,for_var,result)   ;
			if(hdr_inter_fast_str(token->ID, "Compare", 7) == true)       return hdr_domBytesCompare(inter,token,for_var,result)   ;
			if(hdr_inter_fast_str(token->ID, "Xor", 3) == true)           return hdr_domBytesXor(inter,token,for_var,result);
		}
		else
		if(for_var->type == hvt_file)
		{
			if(hdr_inter_fast_str(token->ID, "Close", 5) == true) return hdr_domFileClose(inter,token,for_var) ;
			if(hdr_inter_fast_str(token->ID, "Seek", 4) == true) return hdr_domFileSeek(inter,token,for_var)   ;
			if(hdr_inter_fast_str(token->ID, "Read", 4) == true) return hdr_domFileRead(inter,token,for_var,result)     ;
			if(hdr_inter_fast_str(token->ID, "Write", 5) == true) return hdr_domFileWrite(inter,token,for_var,result)   ;
			if(hdr_inter_fast_str(token->ID, "IsOpen", 6) == true) return hdr_domFileIsOpen(inter,token,for_var,result) ;	
			if(hdr_inter_fast_str(token->ID, "Size", 4) == true) return hdr_domFileSize(inter,token,for_var,result)     ;

		}
		/************* SOCKETS ***************************************************/
		if((for_var->type == hvt_tcp_socket_client))
		{
			if(hdr_inter_fast_str(token->ID, "Close", 5) == true) return hdr_domTCPSocketClose(inter,token,for_var) ;
			if(hdr_inter_fast_str(token->ID, "SendUTF8", 8) == true) return hdr_domTCPSocketSendUTF8(inter,token,for_var,result) ;
			if(hdr_inter_fast_str(token->ID, "Send", 4) == true) return hdr_domTCPSocketSendBytes(inter,token,for_var,result)    ;
			if(hdr_inter_fast_str(token->ID, "ReceiveUTF8", 11) == true) return hdr_domTCPSocketRecvUTF8(inter,token,for_var)	 ;
			if(hdr_inter_fast_str(token->ID, "Receive", 7) == true) return hdr_domTCPSocketRecv(inter,token,for_var,result)		 ;
			if(hdr_inter_fast_str(token->ID, "IsValid", 7) == true) return hdr_domTCPSocketIsValid(inter,token,for_var,result)   ;
			if(hdr_inter_fast_str(token->ID, "Free", 4) == true) return hdr_domTCPSocketFree(inter,token,for_var)                ;
		}
		if((for_var->type == hvt_tcp_socket_server))
		{
			if(hdr_inter_fast_str(token->ID, "Close", 5) == true) return hdr_domTCPSocketClose(inter,token,for_var) ;
			if(hdr_inter_fast_str(token->ID, "Accept", 6) == true) return hdr_domTCPServerSocketAccept(inter,token,for_var,result) ;
			if(hdr_inter_fast_str(token->ID, "IsValid", 7) == true) return hdr_domTCPSocketIsValid(inter,token,for_var,result)     ;
			if(hdr_inter_fast_str(token->ID, "Free", 4) == true) return hdr_domTCPSocketFree(inter,token,for_var)                  ;
		}

		if((for_var->type == hvt_ssl_client))
		{
			if(hdr_inter_fast_str(token->ID, "Close", 5) == true) return hdr_domSSLClose(inter,token,for_var) ;
			if(hdr_inter_fast_str(token->ID, "Free", 4) == true) return hdr_domSSLFree(inter,token,for_var)   ;
			if(hdr_inter_fast_str(token->ID, "SendUTF8", 8) == true) return hdr_domSSLSendUTF8(inter,token,for_var,result) ;
			if(hdr_inter_fast_str(token->ID, "Send", 4) == true) return hdr_domSSLSendBytes(inter,token,for_var,result)    ;
			if(hdr_inter_fast_str(token->ID, "ReceiveUTF8", 11) == true) return hdr_domSSLRecvUTF8(inter,token,for_var)	   ;
			if(hdr_inter_fast_str(token->ID, "Receive", 7) == true) return hdr_domSSLRecv(inter,token,for_var,result)	   ;
			if(hdr_inter_fast_str(token->ID, "IsValid", 7) == true) return hdr_domSSLIsValid(inter,token,for_var,result)   ;
		}
		
		if((for_var->type == hvt_ssl_server))
		{
			if(hdr_inter_fast_str(token->ID, "Close", 5) == true) return hdr_domSSLClose(inter,token,for_var) ;
			if(hdr_inter_fast_str(token->ID, "Free", 4) == true) return hdr_domSSLFree(inter,token,for_var)   ;
			if(hdr_inter_fast_str(token->ID, "IsValid", 7) == true) return hdr_domSSLIsValid(inter,token,for_var,result) ;
			if(hdr_inter_fast_str(token->ID, "Accept", 6) == true) return hdr_domSSLServerAccept(inter,token,for_var,result) ;
		}

		/****************************DATABASES****************************************************/
		if(for_var->type == hvt_database)
		{
			if(hdr_inter_fast_str(token->ID, "Close", 5) == true) return hdr_dom_db_close(inter,token,for_var) ;
			if(hdr_inter_fast_str(token->ID, "Free", 4) == true) return hdr_dom_db_free(inter,token,for_var)   ;
			if(hdr_inter_fast_str(token->ID, "IsValid", 7) == true) return hdr_dom_db_is_valid(inter,token,for_var,result) ;
			if(hdr_inter_fast_str(token->ID, "Exec", 4) == true) return hdr_dom_db_exec(inter,token,for_var,result) ;
			if(hdr_inter_fast_str(token->ID, "Query", 5) == true) return hdr_dom_db_ret_dataset(inter,token,for_var,result) ;
		}
		
		if(for_var->type == hvt_dataset)
		{
			if(hdr_inter_fast_str(token->ID, "Free", 4) == true) return hdr_dom_ds_free(inter,token,for_var)   ;
			if(hdr_inter_fast_str(token->ID, "IsValid", 7) == true) return hdr_dom_ds_is_valid(inter,token,for_var,result) ;
			if(hdr_inter_fast_str(token->ID, "FieldsCount", 11) == true) return hdr_dom_ds_fields_count(inter,token,for_var,result) ;
			if(hdr_inter_fast_str(token->ID, "RowsCount", 9) == true) return hdr_dom_ds_rows_count(inter,token,for_var,result) ;
			if(hdr_inter_fast_str(token->ID, "FieldName", 9) == true) return hdr_dom_ds_field_name(inter,token,for_var,result) ;
			if(hdr_inter_fast_str(token->ID, "FieldGenType", 12) == true) return hdr_dom_ds_field_gen_type(inter,token,for_var,result) ;	
			if(hdr_inter_fast_str(token->ID, "FieldType", 9) == true) return hdr_dom_ds_field_type(inter,token,for_var,result)         ;	
			if(hdr_inter_fast_str(token->ID, "ToJson", 6) == true) return hdr_dom_ds_to_json(inter,token,for_var,result)			   ;	
		}

		if(for_var->type == hvt_data_row)
		{
			if(hdr_inter_fast_str(token->ID, "Free", 4) == true) return hdr_dom_dr_free(inter,token,for_var)   ;
			if(hdr_inter_fast_str(token->ID, "IsValid", 7) == true) return hdr_dom_dr_is_valid(inter,token,for_var,result) ;	
		}

		/************************** for all variables ************************************************/
		if(hdr_inter_fast_str(token->ID, "IsUndef", 7) == true) return hdr_all_is_undef(inter,token,for_var,result) ;		
		if(hdr_inter_fast_str(token->ID, "Type", 4) == true) return hdr_all_ret_type(inter,token,for_var,result) ;	
		if(hdr_inter_fast_str(token->ID, "TypeAsStr", 9) == true) return hdr_all_ret_type_str(inter,token,for_var,result) ;
		if(hdr_inter_fast_str(token->ID, "Release", 7) == true) return hdr_all_set_undef(inter,token,for_var) ;	

	}



	/************** END ****************************************************/
	hdr_inter_print_error(inter,"Unknown function");
	printf("The function '%s' is not one of the native Hydra+ or user defined functions.\n",token->ID->stringa);
	return true;

    var_returned :
	/*check if the function is called by an assigment, if the function does not being called 
	 for an assigment and returns a value a warning will be produced*/
	if(inter->warnings == true)
	if (inter->current_instr->instr->type != hdr_i_assign)
	{
		printf("%s : ",token->ID->stringa);
		hdr_inter_print_warning(inter, "The function return a value that is not assigned. This may leads to memory leaks.");
	}
	return false;
}







