/*
  This module implements the interpreter for the Hydra+.

  In summary the interpreter do :
  
  1. In the first loading of the hydra the interpreter
  retrieves the functions and objects declarations from the loader
  and creates a more apropriate form ready for spawing instances.

  2. Retrieves the first instruction and executes it.
  
  3. Continues to executes the instructions until the end of the code.


  Hydra+
  Header file module for the loader
  Live Long And Prosper
  Nikos Mourgis deus-ex.gr 2024

  */


#ifndef HYDRA_LOADER
#include "hydra_loader.h"
#endif

DXLONG64 INTER_ID_GEN = 0 ; 


/*********************** HEADERS ***********************/

typedef struct hdr_terminal_token
{
	PHDR_VAR var; /*the variable that is produced from the indexd/multipart token*/
	PHDR_COMPLEX_TOKEN last_token;/*the actual command (if exists) for the var*/

} *PHDR_TERMINAL_TOKEN;

typedef struct hdr_inter_instr
{
	PHDR_INSTRUCTION instr ;/*the executable instruction*/
	PDXL_NODE        node  ;/*the position of the instruction in the code block, mandatory for retrieve the next instruction*/
} *PHDR_INTER_INSTR;


typedef struct hdr_inter_store
{
	PHDR_BLOCK				code		  ;
	PHDR_INTER_INSTR		current_instr ;
	PHDR_BLOCK				current_block ;
	int						loops         ;
	bool					in_func       ;
	PHDR_CUSTOM_FUNCTION	curr_func     ;
	PHDR_OBJECT_INSTANCE	curr_obj	  ;

} *PHDR_INTER_STORE ;

bool HDR_DOMAIN_STRING_CONCAT_CONCAT(PHDR_VAR base_str, PHDR_VAR str);
PHDR_VAR hdr_inter_retrieve_var_from_block(PHDR_INTERPRETER inter, PDX_STRING varname);
bool hdr_inter_handle_function(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token,PHDR_VAR for_var, PHDR_VAR* result);
bool hdr_inter_replace_utf8_character(PHDR_INTERPRETER inter ,PDX_STRING str,PHDR_VAR newval, 
	DXUTF8CHAR utf8c , char * utf8cc ,DXLONG64 iindx);
DXLONG64 hdr_inter_return_code_point_from_indx(PHDR_INTERPRETER inter , PHDR_TERMINAL_TOKEN ter_token) ;
DXLONG64 hdr_inter_return_byte_value_from_indx(PHDR_INTERPRETER inter , PHDR_TERMINAL_TOKEN ter_token) ;
void hdr_inter_set_block(PHDR_INTERPRETER inter , PHDR_BLOCK block);
PHDR_INTER_STORE hdr_inter_store(PHDR_INTERPRETER inter);
PHDR_INTER_STORE hdr_inter_restore(PHDR_INTERPRETER inter, PHDR_INTER_STORE saved_state);	

PHDR_OBJECT_INSTANCE hdr_object_class_create_instance(PHDR_INTERPRETER inter ,PHDR_OBJECT_CLASS class);
PHDR_CUSTOM_FUNCTION hdr_custom_function_copy(PHDR_INTERPRETER inter,PHDR_CUSTOM_FUNCTION func);
PHDR_CUSTOM_FUNCTION hdr_custom_functions_list_find(PHDR_INTERPRETER inter , PDX_STRING func_name) ;
PHDR_INTERPRETER hdr_interpreter_create(PHDR_LOADER loader,bool loader_can_be_null);
enum exec_state  hdr_inter_execute_instructions_thread(PHDR_INTERPRETER inter);
bool hdr_interpreter_set_functions_from_loader(PHDR_INTERPRETER inter, PHDR_LOADER loader);
PDX_LIST hdr_user_func_h_release_params(PDX_LIST params);
PHDR_VAR hdr_inter_resolve_extract_var_from_expr(PHDR_INTERPRETER inter ,PHDR_EXPRESSION_LIST params , int param_pos);
PDX_LIST hdr_user_func_h_release_params(PDX_LIST params);
void hdr_user_func_code_init_var(PHDR_VAR_LIST list);
PHDR_CUSTOM_FUNCTION hdr_interpreter_copy_function_from_instr(PHDR_INTERPRETER inter,PHDR_INSTRUCTION ifunc,PHDR_OBJECT_CLASS obj_class);
PHDR_CUSTOM_FUNCTION hdr_custom_functions_list_find_gen(PHDR_CUSTOM_FUNCTIONS_LIST lst , PDX_STRING func_name);
bool hdr_inter_assign_and_clear(PHDR_VAR var, PHDR_VAR temp_var,bool clear);

/********************************************************/

#include "hydra_inter_hlp.h" 
#define HYDRA_INTERPRETER 


/*********************** SPECIAL TOKENS **************************/
#define HDR_INTER_SPECIAL_TRUE			1 
#define HDR_INTER_SPECIAL_FALSE			0

#define HDR_INTER_SPECIAL_CONCAT		1001
#define HDR_INTER_SPECIAL_VAR_EXPAND	1002
#define HDR_INTER_SPECIAL_UNICODE		1003
#define HDR_INTER_FILE_READ				1004
#define HDR_INTER_FILE_WRITE			1005
#define HDR_INTER_BIGGER				1006
#define HDR_INTER_SMALLER				1007
#define HDR_INTER_EQUAL 				1008

#define HDR_INTER_FILE_START			SEEK_SET
#define HDR_INTER_FILE_END				SEEK_END
#define HDR_INTER_FILE_CURR				SEEK_CUR

#define HDR_INTER_DB_NULL               DX_FIELD_VALUE_NULL  
#define HDR_INTER_DB_INT				DX_FIELD_VALUE_INT   
#define HDR_INTER_DB_REAL				DX_FIELD_VALUE_FLOAT
#define HDR_INTER_DB_STRING				DX_FIELD_VALUE_TEXT  
#define HDR_INTER_DB_BLOB				DX_FIELD_VALUE_BLOB  
#define HDR_INTER_DB_DATETIME			DX_FIELD_VALUE_DATE  

/* Variable types **************************************/
#define HDR_VAR_UNDEFINED					1100
#define HDR_VAR_POINTER						1101 
#define HDR_VAR_FLOAT						1102
#define HDR_VAR_INTEGER						1103
#define HDR_VAR_NULL						HDR_INTER_DB_NULL
#define HDR_VAR_BOOL						1105
#define HDR_VAR_LIST						1106
#define HDR_VAR_STRINGLIST					1107
#define HDR_VAR_STR_LIST_SORT				1108 
#define HDR_VAR_INT_LIST_SORT				1109
#define HDR_VAR_FLOAT_LIST_SORT				1110	
#define HDR_VAR_FAST_LIST					1111
#define HDR_VAR_HTTP						1112
#define HDR_VAR_DATABASE					1113
#define HDR_VAR_DATASET						1114	
#define HDR_VAR_DATA_ROW					1115
#define HDR_VAR_TCP_SOCK_CL					1116
#define HDR_VAR_TCP_SOCK_SRV				1117
#define HDR_VAR_SSL_CLIENT					1118
#define HDR_VAR_SSL_SRV						1119
#define HDR_VAR_BYTES						1120
#define HDR_VAR_FILE						1121
#define HDR_VAR_SIMPLE_STR					1122
#define HDR_VAR_SIMPLE_STR_BCK				1123
#define HDR_VAR_CMPLX_STR					1124
#define HDR_VAR_STR_EXPAND					1125
#define HDR_VAR_UNICODE_STR					1126
#define HDR_VAR_CODEPOINT					1127
#define HDR_VAR_OBJECT						1128



/******************************************************/



DXLONG64 hdr_inter_return_special_token(PHDR_COMPLEX_TOKEN token, enum hdr_var_type *vtype)
{
	*vtype = hvt_integer ;
	if (hdr_inter_fast_str(token->ID, "concat", 6) == true) return HDR_INTER_SPECIAL_CONCAT  ;
	if (hdr_inter_fast_str(token->ID, "var_expand", 10) == true) return HDR_INTER_SPECIAL_VAR_EXPAND ;
	if (hdr_inter_fast_str(token->ID, "unicode", 7) == true) return HDR_INTER_SPECIAL_UNICODE ;
	if (hdr_inter_fast_str(token->ID, "true", 4) == true) {*vtype = hvt_bool; return HDR_INTER_SPECIAL_TRUE;};
	if (hdr_inter_fast_str(token->ID, "false", 5) == true) {*vtype = hvt_bool; return HDR_INTER_SPECIAL_FALSE;}
	if (hdr_inter_fast_str(token->ID, "_read", 5) == true) {return HDR_INTER_FILE_READ;}
	if (hdr_inter_fast_str(token->ID, "_write", 6) == true) {return HDR_INTER_FILE_WRITE;}
	if (hdr_inter_fast_str(token->ID, "_start", 6) == true) {return HDR_INTER_FILE_START;}
	if (hdr_inter_fast_str(token->ID, "_end", 4) == true) {return HDR_INTER_FILE_END;}
	if (hdr_inter_fast_str(token->ID, "_curr", 5) == true) {return HDR_INTER_FILE_CURR;}

	if (hdr_inter_fast_str(token->ID, "bigger", 6) == true) {return HDR_INTER_BIGGER;}
	if (hdr_inter_fast_str(token->ID, "smaller", 7) == true) {return HDR_INTER_SMALLER;}
	if (hdr_inter_fast_str(token->ID, "equal", 5) == true) {return HDR_INTER_EQUAL;}

	/*DB*/
	if (hdr_inter_fast_str(token->ID, "NULL", 4) == true) {return HDR_INTER_DB_NULL;}              
	if (hdr_inter_fast_str(token->ID, "INTEGER", 7) == true) {return HDR_INTER_DB_INT;}				
	if (hdr_inter_fast_str(token->ID, "REAL", 4) == true) {return HDR_INTER_DB_REAL;}				
	if (hdr_inter_fast_str(token->ID, "STRING", 6) == true) {return HDR_INTER_DB_STRING;}				
	if (hdr_inter_fast_str(token->ID, "BLOB", 4) == true) {return HDR_INTER_DB_BLOB;}				  
	if (hdr_inter_fast_str(token->ID, "DATETIME", 8) == true) {return HDR_INTER_DB_DATETIME;}

	/*VARIABLES*/
	if (hdr_inter_fast_str(token->ID, "_Undefined", 10) == true) {return HDR_VAR_UNDEFINED;}					
	if (hdr_inter_fast_str(token->ID, "_Pointer", 8) == true) {return HDR_VAR_POINTER;}						
	if (hdr_inter_fast_str(token->ID, "_Real", 5) == true) {return HDR_VAR_FLOAT;}					
	if (hdr_inter_fast_str(token->ID, "_Integer", 8) == true) {return HDR_VAR_INTEGER;}											
	if (hdr_inter_fast_str(token->ID, "_Boolean", 8) == true) {return HDR_VAR_BOOL;}						
	if (hdr_inter_fast_str(token->ID, "_SimpleList", 11) == true) {return HDR_VAR_LIST;}						
	if (hdr_inter_fast_str(token->ID, "_Stringlist", 11) == true) {return HDR_VAR_STRINGLIST;}					
	if (hdr_inter_fast_str(token->ID, "_StringlistSort", 15) == true) {return HDR_VAR_STR_LIST_SORT;}				
	if (hdr_inter_fast_str(token->ID, "_IntListSort", 12) == true) {return HDR_VAR_INT_LIST_SORT;}				
	if (hdr_inter_fast_str(token->ID, "_RealListSort", 13) == true) {return HDR_VAR_FLOAT_LIST_SORT;}				
	if (hdr_inter_fast_str(token->ID, "_FastList", 9) == true) {return HDR_VAR_FAST_LIST;}					
	if (hdr_inter_fast_str(token->ID, "_Http", 5) == true) {return HDR_VAR_HTTP;}						
	if (hdr_inter_fast_str(token->ID, "_Database", 9) == true) {return HDR_VAR_DATABASE;}					
	if (hdr_inter_fast_str(token->ID, "_Dataset", 8) == true) {return HDR_VAR_DATASET;}						
	if (hdr_inter_fast_str(token->ID, "_DataRow", 8) == true) {return HDR_VAR_DATA_ROW;}					
	if (hdr_inter_fast_str(token->ID, "_TcpClient", 10) == true) {return HDR_VAR_TCP_SOCK_CL;}					
	if (hdr_inter_fast_str(token->ID, "_TcpServer", 10) == true) {return HDR_VAR_TCP_SOCK_SRV;}				
	if (hdr_inter_fast_str(token->ID, "_SSLClient", 10) == true) {return HDR_VAR_SSL_CLIENT;}					
	if (hdr_inter_fast_str(token->ID, "_SSLServer", 10) == true) {return HDR_VAR_SSL_SRV;}					
	if (hdr_inter_fast_str(token->ID, "_Bytes", 6) == true) {return HDR_VAR_BYTES;}					
	if (hdr_inter_fast_str(token->ID, "_File", 5) == true) {return HDR_VAR_FILE;}					
	if (hdr_inter_fast_str(token->ID, "_SimpleStr", 10) == true) {return HDR_VAR_SIMPLE_STR;}					
	if (hdr_inter_fast_str(token->ID, "_SimpleStrBck", 13) == true) {return HDR_VAR_SIMPLE_STR_BCK;}			
	if (hdr_inter_fast_str(token->ID, "_Concat", 7) == true) {return HDR_VAR_CMPLX_STR;}				
	if (hdr_inter_fast_str(token->ID, "_VarExpand", 10) == true) {return HDR_VAR_STR_EXPAND;}					
	if (hdr_inter_fast_str(token->ID, "_Unicode", 8) == true) {return HDR_VAR_UNICODE_STR;}				
	if (hdr_inter_fast_str(token->ID, "_CodePoint", 10) == true) {return HDR_VAR_CODEPOINT;}					
	if (hdr_inter_fast_str(token->ID, "_Object", 7) == true) {return HDR_VAR_OBJECT;}					

	return -1;
}
/*********************** PHDR_INTERPRETER ***********************/

enum exec_state  hdr_inter_execute_instructions(PHDR_INTERPRETER inter);
PHDR_VAR hdr_inter_retrieve_var_block(PHDR_INTERPRETER inter, PDX_STRING varname);
PHDR_VAR hdr_inter_resolve_expr(PHDR_INTERPRETER inter, PHDR_EXPRESSION expr, bool* error);
PHDR_VAR hdr_inter_resolve_index(PHDR_INTERPRETER inter, PHDR_VAR base_var, PHDR_VAR indx);


PHDR_INSTRUCTION hdr_interpreter_ret_instr(PDXL_NODE node)
{
	if (node == NULL) return NULL  ;
	PDXL_OBJECT obj = node->object ;
	return (PHDR_INSTRUCTION)obj->obj;
}


typedef struct hdr_interpreter
{
	DXLONG64 ID								;/*an id for debuging purposes in async functions*/ 
	PHDR_LOADER		 loader					;/*the loader*/
	
	PHDR_VAR_LIST    params					;/*this will be set after the parameters are load. This is a reference to a member in the PHYDRA object*/ 
	PHDR_THREAD_LIST threads				;/*this will be set by the PHYDRA object and is a pointer to the PHYDRA->threads*/

	PHDR_BLOCK		 code					;/*a reference to the code in the loader just for ease to use*/
	PHDR_INTER_INSTR current_instr			;/*the current instruction that the interpreter will/is executing*/
	PHDR_BLOCK		 current_block			;/*the current block that the interpreter executes*/
	PHDR_INSTRUCTION last_instr				;/*for error reporting*/
	
	/*loop control*/
	int  loops				 ;/*this flag is mainly for fast checking if the interpreter is in a loop or not*/

	/*functions*/
	bool in_func								  ; /*this flag is set to true while the interpreter runs code of a user defined function*/
	PHDR_CUSTOM_FUNCTION        curr_func         ; /*The current function that the interpreter executes*/ 
	PHDR_VAR		            ret_var			  ; /*this is used for returnig a variable if the interpreter is called from another script (see return command) */
	PHDR_CUSTOM_FUNCTIONS_LIST	funcs			  ; /*the functions in this list are ready for execution or for the async to peek , copy,initialize and execute in another thread*/
	
	/*object classes*/
	PHDR_OBJECT_CLASS_LIST object_classes ;
	PHDR_OBJECT_INSTANCE   curr_obj       ; /*the current object code that the interpreter executes*/

	/*async functions execution*/
	PHDR_THREAD thread					  ;/*this will be set to the thread struct with the hydra thread information*/
	PDX_LIST async_params				  ; /*the list that keeps the parameters of the async function to free if needed after execution.*/
	/*debug tools*/
	DXLONG64 time_point;
	bool warnings; /*if true then the warning message from the interpreter will be printed*/

} *PHDR_INTERPRETER;

/****************************************************************/

void hdr_inter_print_error(PHDR_INTERPRETER inter, char* message)
{
	PHDR_INSTRUCTION instr   = inter->last_instr    ;
	PHDR_INSTRUCTION cinstr  = inter->current_instr->instr ;
	if (instr == NULL) printf(" Fatal Error -> NULL instruction. A bug most propably!");
	else
	{
		if(cinstr!=NULL) printf("[%s] Fatal Error -> Line %d . Message: %s\n", cinstr->filename->stringa, cinstr->line, message);
		else
		printf("[%s] Fatal Error -> Line %d . Message: %s\n", instr->filename->stringa, instr->line, message);
	}
}

void hdr_inter_print_warning(PHDR_INTERPRETER inter, char* message)
{
	PHDR_INSTRUCTION instr = inter->last_instr;
	if (instr == NULL) printf(" Warning -> NULL instruction. A bug most propably!");
	else
		printf("[%s] Warning -> Line %d . Message: %s\n", instr->filename->stringa, instr->line, message);
}




/****************************************************************/
#include "hydra_res_str.h"
#include "hydra_threads.h"
#include "hydra_commands.h"
#include "hydra_functions.h"
#include "hydra_domains.h"

/******************* INTERPRETER STATE STORE/RESTORE *************************/


PHDR_INTER_STORE hdr_inter_store(PHDR_INTERPRETER inter)
{
	PHDR_INTER_STORE saved_state = (PHDR_INTER_STORE)malloc(sizeof(struct hdr_inter_store));
	saved_state->current_instr = (PHDR_INTER_INSTR)malloc(sizeof(struct hdr_inter_store));
	if (saved_state == NULL) return NULL				;
	if (saved_state->current_instr == NULL)
	{
		free(saved_state)   ;
		return NULL			;
	}
	saved_state->code			= inter->code			;
	saved_state->current_block  = inter->current_block	;
	saved_state->current_instr->instr = inter->current_instr->instr	;
	saved_state->current_instr->node  = inter->current_instr->node  ;
	saved_state->in_func			  = inter->in_func				;
	saved_state->curr_func			  = inter->curr_func			; 
	saved_state->curr_obj			  = inter->curr_obj				;
	saved_state->loops                = inter->loops				;
    return saved_state;
}

PHDR_INTER_STORE hdr_inter_restore(PHDR_INTERPRETER inter, PHDR_INTER_STORE saved_state)
{
	/*the saved state is restored and freed*/
	inter->code				= saved_state->code				;
	inter->current_block	= saved_state->current_block	;
	inter->current_instr->instr	= saved_state->current_instr->instr	;
	inter->current_instr->node  = saved_state->current_instr->node  ;
	inter->in_func				= saved_state->in_func				;
	inter->curr_func			= saved_state->curr_func			;
	inter->curr_obj				= saved_state->curr_obj				;
	inter->loops				= saved_state->loops			    ;
	free(saved_state->current_instr) ;
	free(saved_state) ;
	return NULL		  ;
}
/*****************************************************************************/


PHDR_INTERPRETER hdr_interpreter_create(PHDR_LOADER loader,bool loader_can_be_null)
{
	if(loader_can_be_null == false)
	if (loader == NULL)
	{
		printf("***Fatal Error. System Error : Loader is NULL.");
		return NULL ;
	}
	PHDR_INTERPRETER interpreter = (PHDR_INTERPRETER) malloc(sizeof(struct hdr_interpreter));
	if(interpreter == NULL)
	{
		printf("***Fatal Error. System Error : Interpreter is NULL.");
		return NULL;
	}

	interpreter->current_instr			= (PHDR_INTER_INSTR) malloc(sizeof(struct hdr_inter_instr))		;
	if (interpreter->current_instr == NULL)
	{
		printf("***Fatal Error. System Error : Interpreter->current_instr is NULL.");
		return NULL;
	}
	interpreter->last_instr = interpreter->current_instr->instr ;

	interpreter->ID            = INTER_ID_GEN++ ;
	interpreter->loader		   = loader; 
	interpreter->params		   = NULL  ;
	interpreter->threads	   = NULL  ;
	interpreter->loops		   = 0     ;
	interpreter->warnings	   = false ;
	interpreter->time_point	   = dxGetTickCount() ;
	interpreter->in_func	   = false			  ;
	interpreter->curr_func     = NULL             ;
	interpreter->ret_var	   = NULL			  ;

	interpreter->funcs		   = hdr_custom_functions_list_create() ;
	if(interpreter->funcs == NULL)
	{
		printf("***Fatal Error. System Error : Interpreter->funcs is NULL.");
		return NULL;
	}

	interpreter->object_classes = hdr_object_class_list_create();
	interpreter->curr_obj		= NULL ;

	interpreter->thread			= NULL ;
	interpreter->async_params   = NULL ;

	return interpreter;
}

PHDR_INTERPRETER hdr_interpreter_free(PHDR_INTERPRETER inter,bool for_async)
{
	hdr_var_free(inter->ret_var) ;
	free(inter->current_instr)   ;
	if(for_async == false)
	{
	 inter->funcs			= hdr_custom_functions_list_free(inter->funcs) ;
	 inter->object_classes	= hdr_object_class_list_free(inter->object_classes) ;
	 inter->loader		 = NULL	 ;
	}
	else
	{
	  hdr_user_func_h_release_params(inter->async_params) ;
	}
	free(inter)					 ;
	return NULL					 ;
}

bool hdr_interpreter_compare_var_param_names(PHDR_INTERPRETER inter , PHDR_CUSTOM_FUNCTION func )
{
 
	PDXL_NODE node =func->parameters->start ;
	while(node != NULL)
	{
	    PDX_STRING vname = node->object->key ;
		if(vname->len > 3)
		if((vname->stringa[1]=='_')&&(vname->stringa[2]=='_'))
		{
			if( dx_HashReturnItem(inter->code->variables->list,vname,true) != NULL )
			{
				printf("Param name : %s\n",vname->stringa);
				return false ;/*a match!*/
			}
		}
		node = node->right ;
	}


	return true ; /*no match*/
}

bool hdr_interpreter_compare_var_param_names_obj(PHDR_INTERPRETER inter , PHDR_CUSTOM_FUNCTION func,PHDR_OBJECT_CLASS obj_class)
{

	if(obj_class == NULL) return true ;
    /*The functions parameters in the objects cannot have the same names as the object variable members */
	PDXL_NODE node =func->parameters->start ;
	while(node != NULL)
	{
	    PDX_STRING vname = node->object->key ;

		if( dx_HashReturnItem(obj_class->code->variables->list,vname,true) != NULL )
		{
			printf("Object class [%s] -> Param name : %s\n",obj_class->class_name->stringa,vname->stringa);
			return false ;/*a match!*/
		}

		node = node->right ;
	}


	return true ; /*no match*/
}

void hdr_interpreter_remove_local_global_vars(PHDR_INTERPRETER inter, PHDR_CUSTOM_FUNCTION func)
{

	PDX_STRINGLIST var_list = dx_stringlist_create() ;

	PDX_LIST *lists = func->code->variables->list->buckets ;
	for(int i = 0 ; i < func->code->variables->list->length; i++)
	{
		PDX_LIST list = lists[i] ;
		
		PDXL_NODE node = list->start ;
		while(node != NULL)
		{
			PDX_STRING vname = node->object->key ;
			if(vname->len > 3)
			if((vname->stringa[1]=='_')&&(vname->stringa[2]=='_'))
			{
				if( dx_HashReturnItem(inter->code->variables->list,vname,true) != NULL )
				{
				  /*a global variable , add it for removal from the function variables*/
					dx_stringlist_add_raw_string(var_list,vname->stringa) ;
				}
			}

			node = node->right ;
		}
	}

	/*remove the variables*/
	PDXL_NODE node = var_list->start ;
	while(node != NULL)
	{
     hdr_var_list_delete_var(func->code->variables,node->object->key) ;
	 node = node->right ;
	}
	dx_stringlist_free(var_list) ;
	return ;
}


/*
 The interpreter creates a function list for execution for the main thread and as template for the async functions
 The function handles and the peculiarities of the object functions
*/
PHDR_CUSTOM_FUNCTION hdr_interpreter_copy_function_from_instr(PHDR_INTERPRETER inter,PHDR_INSTRUCTION ifunc,PHDR_OBJECT_CLASS obj_class)
{

	PHDR_CUSTOM_FUNCTION func = hdr_custom_function_create(inter,ifunc->left_side->ID->stringa);
	if(func == NULL) 
	{
		printf("***Fatal Error. System Error : hdr_custom_function_create(%s) returned NULL.",ifunc->left_side->ID->stringa);
		return NULL;
	}

	/*copy the parameters*/
	PDXL_NODE enode = ifunc->left_side->parameters->start ;
	while(enode!=NULL)
	{
		PHDR_EXPRESSION param = (PHDR_EXPRESSION)enode->object->obj ; 
		dx_stringlist_add_raw_string(func->parameters,param->value->name->stringa) ;
		enode = enode->right ;
	}

	/*copy the code*/
	func->code = hdr_block_copy(ifunc->code,inter->code) ;/*to access the globals*/
	
	/*
	 we do not permit parameter names identical as global variables ,
	 so we will check for it and we will stop execution if this is the case
	*/

	if (hdr_interpreter_compare_var_param_names(inter , func) == false)
	{
	   printf("Hydra+ does not permit the parameters of a custom function to have the same names as the global variables. See the above message.\n");
	   return NULL ;
	}
	
	/*
	 For objects only , check the parameters name against the object members
	*/
	if (hdr_interpreter_compare_var_param_names_obj(inter,func,obj_class) == false)
	{
	   printf("Hydra+ does not permit the parameters of a custom function to have the same names as the global variables or of its object members. See the above message.\n");
	   return NULL ;
	}

	/*
	  we do not want all the variables in the main script to be global (although this was my initial intent),
	  but we really need global variables. So we will set a special prefix for the global variables,
	  the [$__]. Every variable that has as prefix the $__ will be handled as a global from the functions.
	  To achieve this we have to remove from the function code block  all the variables with the same name as the global variables in the 
	  main code block. Keep in mnd that to declare a global variable in other block than the main script block
	  result to a local scope variable.
	*/
	/*^^^^^^^ THE ABOVE FUNCTIONALITY WAS MOVED TO THE LOADER. Thus the hdr_interpreter_remove_local_global_vars is not used.  ^^^^^^^^^*/

	// hdr_interpreter_remove_local_global_vars(inter,func) ;


	/*
	 now there is a tricky part. Hydra+ wants all the function parameters to pass as references (Threads are exceptions as the parameters pass as 
	 object reference and not vaariable reference)
	 even simple strings and numeric values that usually are copied and managed by the Hydra+.  
	 so we will find the variables that have the same name as the parameters (they are references to the parameters)
	 and we will copy the name in the hash table key (for the variables is a reference) and we will free the variable and set the object
	 to NULL. With this trick every time a function is called , we will set the pointer that is refer to a parameter to the
	 apropriate script variable.
	 If the parameter does not exists as a variable in the code block then it will be inserted.(if the parameter is used only for comparison 
	 and not for an assigment, then the code block do will not know about the variable , and for this reason we have to insert it) 
	*/
	PDXL_NODE node = func->parameters->start ;
	while(node != NULL)
	{
	  PDXL_OBJECT obj = node->object ;
	  /*find the variable*/
	  PDXL_OBJECT hash_var = dx_HashReturnItem(func->code->variables->list,obj->key,true) ;
	  if(hash_var != NULL)
	  {
	    /*copy the actual parameter name*/
		hash_var->key = dx_string_createU(NULL,((PHDR_VAR)hash_var->obj)->name->stringa) ;
		/*delete variable*/
		hash_var->obj = hdr_var_free(((PHDR_VAR)hash_var->obj)) ; /*set the object to NULL*/
	  }
	   else
		  {
		    /*the parameter does not have a corresponding variable placeholder in the block . we will insert it*/
			PDXL_OBJECT nobj = dxl_object_create();
			nobj->key = dx_string_createU(NULL,obj->key->stringa)				;
			nobj->obj = NULL													;
			dx_HashInsertObject(func->code->variables->list,nobj)				;

		  }

	  node = node->right ;
	}


	return func ;
}

bool hdr_interpreter_set_functions(PHDR_INTERPRETER inter)
{
 /*
  The function gets the function templates from the loader 
  and creates the functions list in the interpreter.
  The function list is used from the main thread interpreter to execute the apropriate function,
  and for the async command for creating a new thread with a new interpreter and to execute the async function.

 */

 /*parse all the functions from the loader and create the executable function*/

	 PDXL_NODE node = inter->loader->functions->start;
	 while( node != NULL )
	 {
			PHDR_INSTRUCTION ifunc = (PHDR_INSTRUCTION)node->object->obj ;
			/*create a function instance*/
			PHDR_CUSTOM_FUNCTION func = hdr_interpreter_copy_function_from_instr(inter,ifunc,NULL) ; 
			if(func == NULL) return false ;
			/*add the function in the function list*/
		    hdr_custom_functions_list_add(inter->funcs,func) ;

			node = node->right ;
	 }

	 return true ;

}

bool hdr_interpreter_set_functions_from_loader(PHDR_INTERPRETER inter, PHDR_LOADER loader)
{
 /*
  The function gets the function templates from the loader 
  and creates the functions list in the interpreter.
  The function list is used from the main thread interpreter to execute the apropriate function,
  and for the async command for creating a new thread with a new interpreter and to execute the async function.

 */

 /*parse all the functions from the loader and create the executable function*/

	 PDXL_NODE node = loader->functions->start;
	 while( node != NULL )
	 {
			PHDR_INSTRUCTION ifunc = (PHDR_INSTRUCTION)node->object->obj ;
			/*create a function instance*/
			PHDR_CUSTOM_FUNCTION func = hdr_interpreter_copy_function_from_instr(inter,ifunc,NULL) ; 
			if(func == NULL) return false ;
			/*add the function in the function list*/
		    hdr_custom_functions_list_add(inter->funcs,func) ;

			node = node->right ;
	 }

	 return true ;

}


PHDR_CUSTOM_FUNCTION hdr_custom_function_copy(PHDR_INTERPRETER inter,PHDR_CUSTOM_FUNCTION func)
{
  PHDR_CUSTOM_FUNCTION nfunc = hdr_custom_function_create(inter,func->name->stringa) ;
  if(nfunc == NULL) return NULL ;
  nfunc->code				 = hdr_block_copy(func->code,inter->code) ;/*init with the main code block*/
  nfunc->interpreter		 = inter								  ;
  nfunc->object				 = NULL									  ;
  PDXL_NODE node = func->parameters->start ;
  while(node != NULL)
  {
    dx_stringlist_add_raw_string(nfunc->parameters,node->object->key->stringa);
	node = node->right ;
  }

  return nfunc ;
}

/*
 The object_instance function copy is diferrent of the simple function copy because it must 
 handle the parameters different of the template function , as the parameters are already NULL as variables in the code block ,
 and we cannot copy them.
 */


uint32_t hdr_var_list_add_param_aware(PHDR_VAR_LIST list,PHDR_VAR var,PDX_STRING name)
{
	if(list == NULL) return HDR_VAR_ADD_ERROR;
	PDXL_OBJECT obj = dxl_object_create();
	if(var == NULL) obj->key = dx_string_createU(NULL,name->stringa) ; /*this is not a reference*/
	else 
		obj->key = var->name ; /*set the hash key as the variable name , this is a reference so beware of freeing usefull memory*/
	
	obj->obj = var		 ;
	/*add the object in the list*/
	return dx_HashInsertObject(list->list, obj);

}

void hdr_var_list_copy_param_aware(PHDR_VAR_LIST dest , PHDR_VAR_LIST source)
{
	 /* The dest and source HAS to be created. 
		The variables from the source will be copied and inserted in the dest.
		BUT the function is aware of the parameters of a function
	 */

	 for(int i = 0; i < source->list->length;i++)
	 {
		  /*for all the buckets*/
		  PDX_LIST    sbucket = source->list->buckets[i] ;
		  PDX_LIST    dbucket = dest->list->buckets[i]   ;
		  PDXL_NODE   snode   = sbucket->start			 ;
		  /*copy the bucket contents*/
		  while(snode != NULL)
		  {
			PHDR_VAR nvar = NULL ;
			PHDR_VAR svar = (PHDR_VAR)snode->object->obj ;
			if(svar != NULL)
			nvar = hdr_var_clone(svar,svar->name->stringa, dest->block) ;

			hdr_var_list_add_param_aware(dest,nvar,snode->object->key) ;
			snode = snode->right ; /*next variable*/
		  }
	 }
 

	 return ;
}

PHDR_BLOCK hdr_obj_inst_block_copy(PHDR_BLOCK block , PHDR_BLOCK parent)
{
	if(block == NULL) return NULL ;
    /*make an identical copy of the block and returned it*/
	PHDR_BLOCK nblock = hdr_block_create(parent) ;
	/*copy the variables BUT be aware of the parameters*/
	hdr_var_list_copy_param_aware(nblock->variables , block->variables) ;
	/*copy the instructions*/
    hdr_instructions_list_copy(nblock->instructions,block->instructions,nblock);

    return nblock ;
}

PHDR_CUSTOM_FUNCTION hdr_object_instance_function_copy(PHDR_INTERPRETER inter,PHDR_CUSTOM_FUNCTION func)
{
  PHDR_CUSTOM_FUNCTION nfunc = hdr_custom_function_create(inter,func->name->stringa) ;
  if(nfunc == NULL) return NULL ;
  nfunc->code				 = hdr_obj_inst_block_copy(func->code,inter->code) ;/*init with the main code block*/
  nfunc->interpreter		 = inter								  ;
  nfunc->object				 = NULL									  ;
  PDXL_NODE node = func->parameters->start ;
  while(node != NULL)
  {
    dx_stringlist_add_raw_string(nfunc->parameters,node->object->key->stringa);
	node = node->right ;
  }

  return nfunc ;
}

PHDR_OBJECT_INSTANCE hdr_object_class_create_instance(PHDR_INTERPRETER inter ,PHDR_OBJECT_CLASS class)
{
  /*
    The function creates a new object instance from the obj class.
	In actuality the object classs and the object instance are the same structure.
  */

  PHDR_OBJECT_INSTANCE inst = hdr_object_class_create(class->class_name->stringa) ;
  if(inst == NULL) return NULL ;
  /*copy the code*/
  inst->code = hdr_block_copy(class->code,class->code->parent) ; /*the parent is set when the classes are created to the main code block*/
  /*copy the function list*/
  PDX_LIST *buckets = class->functions->buckets ;
  for(int i = 0; i<class->functions->length;i++)
  {
    PDX_LIST bucket = buckets[i] ;
	PDXL_NODE node = bucket->start ;
	while(node != NULL)
	{
	    PDXL_OBJECT pobj = node->object ;
		PHDR_CUSTOM_FUNCTION cfunc = (PHDR_CUSTOM_FUNCTION)pobj->obj ;
		/*copy the function*/
		PHDR_CUSTOM_FUNCTION nfunc = hdr_object_instance_function_copy(inter,cfunc) ;
		/*set the extra info for the object function*/
		nfunc->object = inst ;
		nfunc->code->parent = inst->code ; /*the object function can access all the variables of the object*/
		/*add to the list*/
		hdr_custom_functions_list_add(inst->functions,nfunc) ;
		node = node->right ;
	}
   
  }

  return inst ;
}

bool hdr_interpreter_set_obj_classes(PHDR_INTERPRETER inter)
{
  /*
   The object are stored in the loader , we will copy them to the interpreter 
   in the appropriate form , ready for spawing.
  */

	PDXL_NODE node = inter->loader->objects->start;
	while(node != NULL)
	{
	   PHDR_LOADER_OBJ_PROT obj_prot = (PHDR_LOADER_OBJ_PROT)node->object->obj ;
	   /*create a new class*/
	   PHDR_OBJECT_CLASS obj_class   = hdr_object_class_create(obj_prot->name->stringa) ;
	   if(obj_class == NULL) 
	   {
		   printf("The creation of the object class [%s] failed.\n",obj_prot->name->stringa);
		   return false ;
	   }
	   /*setup the initialization code and the variables,
	     the object functions and initialization code has access to the global variables*/
	   /*copy the code block*/
	   obj_class->code      = hdr_block_copy(obj_prot->instr->code,inter->code) ; 
	   
	   /*copy the functions*/
	   /*parse all the functions from the object and create the executable function*/

		PDXL_NODE fnode = obj_prot->functions->start;
		while( fnode != NULL )
		{
			PHDR_INSTRUCTION ifunc = (PHDR_INSTRUCTION)fnode->object->obj ;
			/*create a function instance*/
			PHDR_CUSTOM_FUNCTION func = hdr_interpreter_copy_function_from_instr(inter,ifunc,obj_class) ; 
			if(func == NULL) return false ;
			 /*the following two instructions are not mandatory as this is just the template for the object*/
			func->code->parent = obj_class->code ;
			func->object	   = obj_class ; 
			/*add the function in the function list*/
			hdr_custom_functions_list_add(obj_class->functions,func) ;

			fnode = fnode->right ;
		}

		/*add the object class in the list*/
		hdr_object_class_list_add(inter->object_classes,obj_class) ;

	   node = node->right ;
	}


	return true ;
}


bool hdr_interpreter_init(PHDR_INTERPRETER inter)
{
	/*setup the code*/
	inter->current_instr->instr = hdr_interpreter_ret_instr(inter->loader->code->instructions->start);
	inter->last_instr			= inter->current_instr->instr;
	inter->current_instr->node  = inter->loader->code->instructions->start;
	inter->code					= inter->loader->code ;
	inter->current_block		= inter->code		  ;

	/*setup user custom functions*/
	if (hdr_interpreter_set_functions(inter) == false ) return false;
	/*setup object classes*/
	if (hdr_interpreter_set_obj_classes(inter) == false ) return false;
	return true;
}

void hdr_inter_next_instr(PHDR_INTERPRETER inter)
{
	/*point to the next instruction for execution*/
	PDXL_NODE node = inter->current_instr->node->right ;
	if (node == NULL)
	{
		inter->current_instr->node  = NULL;
		inter->current_instr->instr = NULL;
	}
	else
	{
		inter->current_instr->node = node;
		inter->current_instr->instr = hdr_interpreter_ret_instr(node);
	}
}

void hdr_inter_set_block(PHDR_INTERPRETER inter , PHDR_BLOCK block)
{
	inter->current_block = block;
	inter->current_instr->instr = hdr_interpreter_ret_instr(block->instructions->start) ;
	inter->current_instr->node  = block->instructions->start							;
}



/***************************************************************/

PHDR_VAR hdr_inter_retrieve_var_block(PHDR_INTERPRETER inter,PDX_STRING varname)
{
  /*
    the function searches for the varname in the block and if it does not find 
	a var searches the parent and the parent's parent etc
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



bool hdr_inter_assign_and_clear(PHDR_VAR var, PHDR_VAR temp_var,bool clear)
{
	/*
	 the var will be set to the temp_var values and the temp_var will be reset
	 if the clear is true.
	 Return false in error , true in success
	*/
	
	/*
	 A special case is the return values from a function, we will signal this to the return command 
	 so to invalidate the variable in the function to avoid access violations
	*/
	if(temp_var->var_ref == hvf_block)
	var->func_ref_var = temp_var ;
	else
		var->func_ref_var = NULL ;

	/*if a variable is a literal (a case that exists in the parameters of a user function) we do not change its value*/
	if(var->literal == true)
	{
		printf("A literal value cannot change in the lifetime of a script. Check if you try to modify a parameter of a function that is a literal value.\n");
		return false;
	}

	var->type = temp_var->type;
	//var->var_ref = temp_var->var_ref; i do not remember why is this here, this is not right acording to the logic of the code flow. aggrrrr.... it will bite me later...
	switch (temp_var->type)
	{
		case hvt_undefined: 
		{
		   /*
		     12-07-2024 i will permit undefined variables to return because we need to know if a function failed to return an object etc
			 supporting functions :  Variable.IsUndef() : Boolean
									 $var = undef() (declared it as undefined)
		   */
			 
		}break;
		case hvt_float:
		{
			if (var->type == hvt_undefined) var->type = temp_var->type;
			else
				if (hdr_inter_var_gen_type(var) != hdr_ivt_numeric)
				{
					printf("The value that the variable try to be assigned to is not a numeric value.\n");
					return false;
				}
			if (var->type == hvt_float)
				var->real = temp_var->real;
			else
				var->integer = temp_var->real;
		}break;

		case hvt_integer:
		{
			if (var->type == hvt_undefined) var->type = temp_var->type;
			else
				if ((hdr_inter_var_gen_type(var) != hdr_ivt_numeric)&&(var->type != hvt_codepoint))
				{
					printf("The value that the variable try to be assigned to is not a numeric value.\n");
					return false;
				}
			if ((var->type == hvt_integer)||(var->type == hvt_codepoint))
			var->integer = temp_var->integer;
			else
				var->real = temp_var->integer;
		}break;
		case hvt_bool:
		{
			if (var->type == hvt_undefined) var->type = temp_var->type;
			else 
				if(var->type != hvt_bool) 
				{
					printf("The value that the variable try to be assigned to is not a boolean value.\n");
					return false;
				}
			/*the bool type is mapped in the integer of the variable as 0 for false and 1 for true*/
			var->integer = temp_var->integer ;
		}break;
		case hvt_codepoint:
		{
			if (var->type == hvt_undefined) var->type = temp_var->type;
			else 
				if(var->type != hvt_codepoint) 
				{
					printf("The value that the variable try to be assigned to is not a codepoint (unicode character) value.\n");
					return false;
				}
			/*the codepoint type is mapped in the integer of the variable*/
			var->integer = temp_var->integer ;
		}break;
		case hvt_simple_string:
		case hvt_simple_string_bcktck : 
		{
			if (var->type == hvt_undefined) var->type = temp_var->type;
			else
				if ((var->type != hvt_simple_string)&&(var->type != hvt_simple_string_bcktck)&&(var->type != hvt_unicode_string))
				{
					printf("The value that the variable try to be assigned to is not a simple string.\n");
					return false;
				}
			 if ((var->type == hvt_simple_string)||(var->type == hvt_simple_string_bcktck)) 
			 {
			   /*copy the actual string memory*/
		     	PDX_STRING cpy_val = (PDX_STRING)(temp_var->obj);
				PDX_STRING ns = dx_string_createU((PDX_STRING)var->obj,cpy_val->stringa);
				hdr_var_set_obj(var,ns);
			 }
			 else
				if (var->type == hvt_unicode_string)
				{
				  /*copy the actual string memory*/
		     	   PDX_STRING cpy_val = (PDX_STRING)(temp_var->obj);
				   PDX_STRING ns = dx_string_createX_u((PDX_STRING)var->obj,cpy_val->stringa);
				   hdr_var_set_obj(var,ns);
				}
			/*all ok, if the temp_var is created by a function then the caller has the responsibility to free it*/
		}break;
		case hvt_unicode_string : 
		{
			if (var->type == hvt_undefined) var->type = temp_var->type;
			else
				if ((var->type != hvt_simple_string)&&(var->type != hvt_simple_string_bcktck)&&(var->type != hvt_unicode_string))
				{
					printf("The value that the variable try to be assigned to is not a simple  string.\n");
					return false;
				}
			 
			 if ((var->type == hvt_simple_string)||(var->type == hvt_simple_string_bcktck)) 
			 {
				/*copy the actual string memory*/
				PDX_STRING cpy_val = (PDX_STRING)(temp_var->obj);
				char *utf8str =  dx_ConvertDXSToUtf8(cpy_val->stringx) ;
				PDX_STRING ns = dx_string_createU((PDX_STRING)var->obj,utf8str) ;
				hdr_var_set_obj(var,ns);
				free(utf8str) ;
				/*all ok, if the temp_var is created by a function then the caller has the responsibility to free it*/
			 }
			 else
				 if (var->type == hvt_unicode_string)
				 {
				    /*copy the actual string memory*/
					PDX_STRING cpy_val = (PDX_STRING)(temp_var->obj);
					PDX_STRING ns = dx_string_createX((PDX_STRING)var->obj,cpy_val->stringx) ;
					hdr_var_set_obj(var,ns);
					/*all ok, if the temp_var is created by a function then the caller has the responsibility to free it*/
				 }
		}break;

		default :
		{
			/*all the other objects are mapped in the obj member*/
			if (var->type == hvt_undefined)
			{
				var->type = temp_var->type;
				hdr_var_set_obj(var,temp_var->obj);
			}
			else
				if (var->type == temp_var->type)
				{
					/*Hydra+ does not forbid the assigment of a variable that are not freed first BEWARE OF MEMORY LEAKS*/
					if(var->obj != NULL)
					{
						/*reserved for different handling in the future*/
					}

					/*do the assigment always by referencing*/
					hdr_var_set_obj(var,temp_var->obj);
				}
				else
				{
					printf("The value that the variable try to be assigned to is not a of the same type. Types : %s <-> %s \n", hdr_inter_return_variable_type(var->type),
						hdr_inter_return_variable_type(temp_var->type));
					return false;
				}

		}
	}
	/*clear the temp variable*/
	if(clear == true) hdr_inter_var_clear(temp_var) ;

	return true;
}


PHDR_VAR hdr_inter_return_object_member(PHDR_VAR base_var, PHDR_VAR indx)
{
    /*an object cannot accesed by an numeric index so we will check for it*/
	if(indx->type != hvt_simple_string) 
	{
	  printf("The object's [%s] (and all objects) members cannot be accessed with a numeric index.\n",base_var->name->stringa);
	}
	/*search in the object for the variable that the indx describes*/
	return hdr_var_list_find_var_striped(((PHDR_OBJECT_INSTANCE)base_var->obj)->code->variables,(PDX_STRING)indx->obj) ;
}


PHDR_VAR hdr_inter_return_list_item(PHDR_VAR base_var,PHDR_VAR indx)
{
  /*check if it is assigment for a new item*/
  
  PDX_LIST list = (PDX_LIST)base_var->obj ;
  if(indx->type == hvt_undefined)
  {
    /*create a new undefined variable add it in the list and return*/
	PHDR_VAR var = hdr_var_create(NULL,"",hvf_dynamic,NULL) ; /*the variable is a dynamic variable */
	if(var == NULL)
	{
	 printf("MEMORY ERROR. THE PHDR_VAR IS NULL.\n");
	 return NULL ;
	}

	PDXL_OBJECT obj = dxl_object_create();
	if(obj == NULL)
	{
	  printf("MEMORY ERROR. The PDXL_OBJECT returned as NULL\n");
	  return NULL ;
	}
	obj->obj = var ;
	dx_list_add_node_direct(list,obj) ;
	dx_string_free(var->name);
	var->name = dx_IntToStr(list->count-1) ; 

	return var ;
  }
  else
  {
    /*return the appropriate variable*/
	/*check the type of the index*/ 
    DXLONG64 iindx = -1 ;
	if(hdr_inter_var_gen_type(indx) == hdr_ivt_numeric )
	{
		if((indx->type == hvt_float)) iindx = indx->real ;
		else
			if((indx->type == hvt_integer)) iindx = indx->integer ;
			else
			   if(indx->type == hvt_bool) 
			   {
				 printf("The index to the list HAS to be of [Numeric] or [String] type\n");
				 return NULL ;
			   }

	   if((iindx < 0)||(iindx >= list->count) )
	   {
	    printf("The index that was supplied to the list was out of bounds. List range : 0 ~ %d\n",list->count-1) ;
	    return NULL ;
	   }

	   PDXL_NODE pnode = dx_list_go_to_pos(list,iindx) ;
	   if(pnode == NULL) 
	   {
		   printf("Internal error. The index that was supplied for the [List] '%s' returned as NULL node.\n",base_var->name->stringa);
		   return NULL ;
	   }
	   PHDR_VAR  var   = (PHDR_VAR)pnode->object->obj  ;
	   return var ;

    }
	else 
		if((indx->type == hvt_simple_string)||(indx->type == hvt_simple_string_bcktck))
		{
			/*the list will create a new item if the indx is not present in it or it will return the item*/
			PDX_STRING str = (PDX_STRING)indx->obj ;
			if(str->len == 0)
			{
				printf("The index that was supplied for the list, was an empty string. This is forbidden.\n");
				return NULL ;
			}
			/*find the indx*/
			PDXL_NODE node = list->start ;
			while(node != NULL)
			{
			  PDXL_OBJECT obj = node->object ;
			  if(obj->key != NULL)
			  {
				if(dx_string_native_compare(obj->key,(PDX_STRING)indx->obj) == dx_equal)  
				{
				 return (PHDR_VAR)obj->obj ;
				}
			  }
			  node = node->right ;
			}

			/*if we are here then the list does not have this key. We will insert it*/
			PDXL_OBJECT obj = dxl_object_create() ;
			obj->key = dx_string_createU(NULL,((PDX_STRING)indx->obj)->stringa)  ;
			obj->obj = hdr_var_create(NULL,((PDX_STRING)indx->obj)->stringa,hvf_dynamic,NULL) ;
			dx_list_add_node_direct(list,obj)						  ;
			return obj->obj ;
		}
		else
		{
		  printf("The index in a [List] has to be of a [Numeric] or [String] type.\n");
		  return NULL ;
		}

  }
  

  printf(" Error branching at hdr_inter_return_list_item(). Why the execution reached this point ?\n");
  return NULL ;
}


PHDR_VAR hdr_inter_return_fast_list_item(PHDR_VAR base_var,PHDR_VAR indx)
{
  
  PDX_HASH_TABLE list = (PDX_HASH_TABLE)base_var->obj ;
  if(indx->type == hvt_undefined)
  {
	  printf("The index for a fast list cannot be empty. To create a new item supply a new named index.\n");
	  return NULL ;
  }
  else
  {
    /*return the appropriate variable*/
	if((indx->type == hvt_simple_string)||(indx->type == hvt_simple_string_bcktck))
	{
		/*the list will create a new item if the indx is not present in it or it will return the item*/
		PDX_STRING str = (PDX_STRING)indx->obj ;
		if(str->len == 0)
		{
			printf("The index that was supplied for the list, was an empty string. This is forbidden.\n");
			return NULL ;
		}
		/*find the indx*/
		PDXL_OBJECT obj = dx_HashReturnItem(list,str,true) ;
		if(obj != NULL) 
		{
		  return (PHDR_VAR)obj->obj ;
		}

		/*if we are here then the list does not have this key. We will insert it*/
	    obj = dxl_object_create() ;
		obj->key = dx_string_createU(NULL,((PDX_STRING)indx->obj)->stringa)  ;
		obj->obj = hdr_var_create(NULL,((PDX_STRING)indx->obj)->stringa,hvf_dynamic,NULL)		  ;
		dx_HashInsertObject(list,obj);
		return obj->obj ;
	}
	else
		if((indx->type == hvt_integer)||(indx->type == hvt_float))
		{
		 /* return the element that we found when searches the buckets in serial*/
			bool type_error ;
			DXLONG64 nindx = hdr_inter_ret_integer(indx,&type_error);
			PDXL_OBJECT obj = dx_HashItemByIndex(list , nindx);
			if (obj == NULL)
			{
			 printf("The numeric index that was supplied for the [Fast List] was not in the valid range : %d:%d supplied : %d.\n",0,list->count,nindx);
			 return NULL ;
			}
		
			return obj->obj;
		}
	else
	{
		printf("The index in a [Fast List] has to be of [String] or [Numeric] type.\n");
		return NULL ;
	}

  }
  

  printf(" Error branching at hdr_inter_return_list_item(). Why the execution reached this point ?\n");
  return NULL ;
}


PHDR_VAR hdr_inter_return_stringlist_item(PHDR_VAR base_var,PHDR_VAR indx)
{
  /*check if it is assigment for a new item*/
  
  PDX_LIST list = (PDX_LIST)base_var->obj ;
  if(indx->type == hvt_undefined)
  {
	  printf("The 'Stringlist' type does not support the syntax of $strlist[] = `string`. To add a new string use the $strlist.Add($string) function.\n");
	  return NULL ;
  }
  else
  {
    /*return the appropriate variable*/
	/*check the type of the index*/ 
    DXLONG64 iindx = -1 ;
	if(hdr_inter_var_gen_type(indx) == hdr_ivt_numeric )
	{
		if((indx->type == hvt_float)) iindx = indx->real ;
		else
			if((indx->type == hvt_integer)) iindx = indx->integer ;
			else
			   if(indx->type == hvt_bool) 
			   {
				 printf("The index to the list HAS to be of [Numeric] or [String] type\n");
				 return NULL ;
			   }

	   if((iindx < 0)||(iindx >= list->count) )
	   {
	    printf("The index that was supplied to the list was out of bounds. Stringlist range : 0 ~ %d\n",list->count-1) ;
	    return NULL ;
	   }

	   /*find the string*/
	   PDXL_NODE pnode = dx_list_go_to_pos(list,iindx) ;
	   if(pnode == NULL) 
	   {
		   printf("Internal error. The index that was supplied for the [List] '%s' returned as NULL node.\n",base_var->name->stringa);
		   return NULL ;
	   }

	   PDX_STRING str = pnode->object->key ;

	   PHDR_VAR var = hdr_var_create(NULL,"",hvf_temporary_ref,NULL) ; /*the variable is a temporary variable hydra dispose automatically this variables that hold strings*/
	   if(var == NULL)
	   {
		 printf("MEMORY ERROR. THE PHDR_VAR IS NULL.\n");
		 return NULL ;
	   }

	   var->type = hvt_simple_string ;
	   PDX_STRING ns = dx_string_createU(NULL,str->stringa) ;
	   hdr_var_set_obj(var, ns);

	   return var ;
    }
    else
		{
		  printf("The index in a [Stringlist] has to be of a [Numeric] type.\n");
		  return NULL ;
		}
  }
  

  printf(" Error branching at hdr_inter_return_stringlist_item(). Why the execution reached this point ?\n");
  return NULL ;
}


PHDR_VAR hdr_inter_return_dataset_row(PHDR_VAR base_var,PHDR_VAR indx)
{
  
  PDX_QUERY query = (PDX_QUERY)base_var->obj ;
  if(indx->type == hvt_undefined)
  {
    printf("A [Dataset] does not handle void indexing. Always set the row number that you want to access.\n");
    return NULL ;
  }
  else
  {
    /*return the appropriate variable, the dataset can be accessed only by row number*/
    DXLONG64 iindx = -1 ;
	if(hdr_inter_var_gen_type(indx) == hdr_ivt_numeric )
	{
		if((indx->type == hvt_float)) iindx = indx->real ;
		else
			if((indx->type == hvt_integer)) iindx = indx->integer ;
			else
			   if(indx->type == hvt_bool) 
			   {
				 printf("The index to the [Dataset] HAS to be of [Numeric] type\n");
				 return NULL ;
			   }

	   if((iindx < 0)||(iindx >= query->row_count) )
	   {
	    printf("The index that was supplied to the [Dataset] was out of bounds. Dataset range : 0 ~ %d",query->row_count-1) ;
	    return NULL ;
	   }

	   PDXL_NODE pnode = dx_list_go_to_pos(query->dataset,iindx) ;
	   if(pnode == NULL) 
	   {
		   printf("Internal error. The index that was supplied for the [Dataset] '%s' returned as NULL node.\n",base_var->name->stringa);
		   return NULL ;
	   }
	   PDX_ROW   row = dx_db_query_row_from_node(pnode); 
	   PDX_DB_ROW_WRAP rwrap = dx_db_row_wrap_create() ;
	   rwrap->query = query ;
	   rwrap->row   = row   ;
	   PHDR_VAR  var = hdr_var_create(rwrap,"",hvf_temporary_ref,NULL) ;  
	   var->type = hvt_data_row ;
		  
	   return var ;

    }
	 else
		{
		  printf("The index in a [Dataset] has to be of a [Numeric] type.\n");
		  return NULL ;
		}

  }
  

  printf(" Error branching at hdr_inter_return_dataset_row(). Why the execution reached this point ?\n");
  return NULL ;
}


PHDR_VAR hdr_inter_return_data_row_field(PHDR_VAR base_var,PHDR_VAR indx)
{
  
  PDX_DB_ROW_WRAP wrow = (PDX_DB_ROW_WRAP)base_var->obj ;
  PDX_ROW		  row  = wrow->row ;
  if(indx->type == hvt_undefined)
  {
     printf("The [Data Row] does not handle void indexing. Always supply the numeric index or the name of the field you want to access.\n");
     return NULL ;
  }
  else
  {
    /*return the appropriate variable*/
	/*check the type of the index*/ 
    DXLONG64 iindx = -1 ;
	if(hdr_inter_var_gen_type(indx) == hdr_ivt_numeric )
	{
		if((indx->type == hvt_float)) iindx = indx->real ;
		else
			if((indx->type == hvt_integer)) iindx = indx->integer ;
			else
			   if(indx->type == hvt_bool) 
			   {
				 printf("The index HAS to be of [Numeric] or [String] type\n");
				 return NULL ;
			   }

	   if((iindx < 0)||(iindx >= row->count) )
	   {
	    printf("The index that was supplied was out of bounds. List range : 0 ~ %d\n",row->count-1) ;
	    return NULL ;
	   }

	   PDXL_NODE pnode = dx_list_go_to_pos(row,iindx) ;
	   if(pnode == NULL) 
	   {
		   printf("Internal error. The field index that was supplied for the [Data Row] '%s' returned as NULL node.\n",base_var->name->stringa);
		   return NULL ;
	   }

	   /*now we have to basic type of the field to convert it to a native PHDR_VAR */
	   PHDR_VAR  var   = hdr_var_create(NULL,"",hvf_temporary_ref,NULL) ;
	   PDX_FIELD field = pnode->object ;
	   switch(field->flags)
	   {
	    
		   case DX_FIELD_VALUE_NULL:
		   {
		     var->type    = hvt_null ;
			 var->integer = 0			;
		   }break ;

		   case DX_FIELD_VALUE_INT:
		   {
		     var->type    = hvt_integer		;
			 var->integer = field->int_key	;
		   }break ;
			   
		   case DX_FIELD_VALUE_FLOAT:
		   {
			 var->type    = hvt_float		 ;
			 var->real	  = field->float_key ;
		   }break ;

		   case DX_FIELD_VALUE_TEXT:
		   {
			 var->type    = hvt_simple_string ;
			 PDX_STRING ns = dx_string_createU(NULL,field->key->stringa) ;
			 hdr_var_set_obj(var,ns) ;
		   }break ;
		   case DX_FIELD_VALUE_BLOB:
		   {
		     var->type    = hvt_bytes   ;
			 PDX_DB_BLOB blob = (PDX_DB_BLOB)field->obj ;
			 PHDR_BYTES bytes = hdr_bytes_create(blob->count) ;
			 /*copy the bytes*/
			 memcpy(bytes->bytes,blob->data,bytes->length) ;
			 hdr_var_set_obj(var,bytes);
		   }
		   case DX_FIELD_VALUE_DATE:
		   {
		     var->type     = hvt_simple_string ;
			 PDX_STRING ns = dx_string_createU(NULL,field->key->stringa) ;
			 hdr_var_set_obj(var,ns) ;
		   }
	   }

	   	return var ;
    }
	else 
		if((indx->type == hvt_simple_string)||(indx->type == hvt_simple_string_bcktck))
		{
			PDX_STRING str = (PDX_STRING)indx->obj ;
			if(str->len == 0)
			{
				printf("The field name that was supplied for the [Data Row], was an empty string. This is forbidden.\n");
				return NULL ;
			}
			/*find the indx*/
			DXLONG64 findx = dx_db_find_field_pos(wrow->query,str) ;
			if (findx == -1)
			{
			    printf("The field name that was supplied for the [Data Row], was not found in the dataset.\n");
				return NULL ;
			}

			
			PDXL_NODE pnode = dx_list_go_to_pos(row,findx) ;
		   /*now we have to basic type of the field to convert it to a native PHDR_VAR */
		   PHDR_VAR  var   = hdr_var_create(NULL,"",hvf_temporary_ref,NULL) ;
		   PDX_FIELD field = pnode->object ;
		   switch(field->flags)
		   {
	    
			   case DX_FIELD_VALUE_NULL:
			   {
				 var->type    = hvt_null ;
				 var->integer = 0			;
			   }break ;

			   case DX_FIELD_VALUE_INT:
			   {
				 var->type    = hvt_integer		;
				 var->integer = field->int_key	;
			   }break ;
			   
			   case DX_FIELD_VALUE_FLOAT:
			   {
				 var->type    = hvt_float		 ;
				 var->real	  = field->float_key ;
			   }break ;

			   case DX_FIELD_VALUE_TEXT:
			   {
				 var->type    = hvt_simple_string ;
				 PDX_STRING ns = dx_string_createU(NULL,field->key->stringa) ;
				 hdr_var_set_obj(var,ns) ;
			   }break ;
			   case DX_FIELD_VALUE_BLOB:
			   {
				 var->type    = hvt_bytes   ;
				 PDX_DB_BLOB blob = (PDX_DB_BLOB)field->obj ;
				 PHDR_BYTES bytes = hdr_bytes_create(blob->count) ;
				 /*copy the bytes*/
				 memcpy(bytes->bytes,blob->data,bytes->length) ;
				 hdr_var_set_obj(var,bytes);
			   }
			   case DX_FIELD_VALUE_DATE:
			   {
				 var->type     = hvt_simple_string ;
				 PDX_STRING ns = dx_string_createU(NULL,field->key->stringa) ;  
				 hdr_var_set_obj(var,ns) ;
			   }
		   }

		   return var ;
		
		}
		else
		{
		  printf("The index in a [Data Row] has to be of a [Numeric] or [String] type.\n");
		  return NULL ;
		}

  }
  

  printf(" Error branching at hdr_inter_return_data_row_field(). Why the execution reached this point ?\n");
  return NULL ;
}


PHDR_VAR hdr_inter_resolve_index(PHDR_INTERPRETER inter, PHDR_VAR base_var, PHDR_VAR indx)
{
	/*
	  the function finds the appropriate variable (or value) in the base_var based of the indx  
	  returns the variable or NULL for error 
	*/

	/*Not all native types can be indexed. we will check the variable type and we will handle it appropriately*/
	switch (base_var->type)
	{
		case hvt_undefined			: { printf("The variable '%s' is undefined.\n", base_var->name->stringa); return NULL; }
		break;
		case hvt_float				: { printf("The 'Numeric (Real)' type cannot be accessed by an index. Variable : '%s'\n", base_var->name->stringa); return NULL; }
		break;
		case hvt_integer			: { printf("The 'Numeric (Integer)' type cannot be accessed by an index. Variable : '%s'\n", base_var->name->stringa); return NULL; }
		break;
		case hvt_bool				: { printf("The 'Boolean' type cannot be accessed by an index. Variable : '%s'\n", base_var->name->stringa); return NULL; }
		break;
		case hvt_codepoint			: { printf("The 'Codepoint' type cannot be accessed by an index. Variable : '%s'\n", base_var->name->stringa); return NULL; }
		break;
		case hvt_list				: { return  hdr_inter_return_list_item(base_var,indx); }
		break;
		case hvt_string_list		: { return hdr_inter_return_stringlist_item(base_var,indx);}
		break;
		case hvt_string_list_sort	: {printf("NOT IMPLEMENTED YET 'string_list_sort' %s.\n", base_var->name->stringa); return NULL;}
		break;
		case hvt_int_list_sort		: {printf("NOT IMPLEMENTED YET 'hvt_int_list_sort' %s.\n", base_var->name->stringa); return NULL;}
		break;
		case hvt_float_list_sort    : {printf("NOT IMPLEMENTED YET 'hvt_float_list_sort' %s.\n", base_var->name->stringa); return NULL;};
		break;
		case hvt_fast_list	        : {return  hdr_inter_return_fast_list_item(base_var,indx);}
		break;
		case hvt_dataset			: 
		{
			/*
			 User MUST free the dataset row as it is copyed from the memory and not be passed as a reference
			 emit a warning as it is not intuitive and memory leaks are very easy to be created
			*/
			if(inter->warnings == true) hdr_inter_print_warning(inter,"MEMORY LEAK WARNING! The row from the dataset is copied and not a reference. Use the .Free function after use. DO NOT USE THE SYNTAX $dataset[$indx].field as it leave the memory allocated.");
			return hdr_inter_return_dataset_row(base_var,indx);
		}
		break;
		case hvt_data_row			: {return hdr_inter_return_data_row_field(base_var,indx);}
		break;
		case hvt_object				: { return  hdr_inter_return_object_member(base_var,indx);}
		break;
		case hvt_bytes				: 
		case hvt_simple_string		: 
		case hvt_simple_string_bcktck :
		case hvt_unicode_string			:
		case hvt_complex_string			: 
		case hvt_complex_string_resolve : 
		{
			/*
				The [strings] are transparent as to its characters, same for the [bytes].
				We will return the base variable and we will handle them differently. 
			*/

			if( hdr_inter_var_gen_type(indx) != hdr_ivt_numeric)
			{printf("A 'String'or 'Bytes' type can have only a numeric index that will point to a character (or a Byte) in the string. Variable : %s\n", base_var->name->stringa); return NULL;}
			/*set the flag that signal that this variable return trully a byte or character*/
			base_var->need_indx = true ;
			return base_var ;

		}
		break;
		default : 
		{
			printf("The variable type '%s' cannot access data by indexes\n",hdr_inter_return_variable_type(base_var->type));
		}
	}

	return NULL;
}

enum exec_state hdr_inter_handle_str_indx_asgn(PHDR_INTERPRETER inter,PHDR_TERMINAL_TOKEN ter_token,PHDR_EXPRESSION expr) 
{
  /*handle the anomaly of assigment of utf8 characters,unicode and bytes*/

  bool error	= false ;
  PHDR_VAR indx = hdr_inter_resolve_expr(inter,ter_token->last_token->expression,&error) ;
  if(error == true)
  {
	printf("The supplied index for the '%s' variable was not resolved in a valid value.\n",ter_token->var->name->stringa);
	return exec_state_error ;
  }

  if((indx->type != hvt_integer)&&(indx->type != hvt_float))
  {
	printf("The supplied index for the '%s' variable MUST be of 'Numeric' type.\n",ter_token->var->name->stringa);
	return exec_state_error ;
  }

  /*get the number*/
  DXLONG64 iindx = 0 ;
  if(indx->type == hvt_integer) iindx = indx->integer ;
  else
	  iindx = (DXLONG64)floor(indx->real) ; 

  /*get the value to set*/
  PHDR_VAR newval = hdr_inter_resolve_expr(inter,expr,&error) ;
  if(error == true)
  {
	printf("The supplied expression to calculate the value to set in the '%s' variable was not resolved in a valid value.\n",ter_token->var->name->stringa);
	return exec_state_error ;
  }

  if((newval->type != hvt_integer)&&(newval->type != hvt_float))
  if((hdr_inter_var_gen_type(newval)!=hdr_ivt_string))
  {
	printf("The supplied value for the '%s' variable (String type) MUST be of 'Numeric' type or a single char , and must represent (in case of 'Numeric') a utf8 code point.\n",ter_token->var->name->stringa);
	return exec_state_error ;
  }

  DXUTF8CHAR utf8c = 0 ;
  char *utf8cc = NULL   ;
  if((hdr_inter_var_gen_type(newval)==hdr_ivt_string))
  {
    /*check if the string is one character long*/
    if(((PDX_STRING)newval->obj)->len != 1)
	{
		printf("A utf8 character expected but a string was provided.\n");
		return exec_state_error ;
	}

	utf8cc = ((PDX_STRING)newval->obj)->stringa ;
  }
  else
	if(newval->type == hvt_integer) utf8c = newval->integer ;
	  else
		utf8c = (DXUTF8CHAR)floor(newval->real) ; 

  if((utf8c == 0)&&(utf8cc == NULL))
  {
    printf("The utf8 character that was provided is not valid.\n");
	return exec_state_error ;
  }

  switch(ter_token->var->type)
  {
	case hvt_simple_string:
	case hvt_simple_string_bcktck:
	{
		/*get the utf8char position and replace it with the actual value*/
		if(hdr_inter_replace_utf8_character(inter , (PDX_STRING)ter_token->var->obj,newval,utf8c,utf8cc,iindx) == false)return exec_state_error;
		else 
		{
			dx_string_free((PDX_STRING)newval->obj);
			hdr_var_release_obj(newval) ;
			newval->type = hvt_undefined  ; 
			return exec_state_ok ;
		}

	}break ;
	case hvt_unicode_string :
	{
	  PDX_STRING str = (PDX_STRING)ter_token->var->obj ;
	  if((iindx < 0)||(iindx >= str->len)) return exec_state_error ;
	  else
	  {
		if(utf8cc == NULL)
		str->stringx[iindx] = utf8c ;
		else
			str->stringx[iindx] = dx_ConvertUtf8ToDXSc(utf8cc) ;

		dx_string_free((PDX_STRING)newval->obj);
		hdr_var_release_obj(newval) ;
		newval->type = hvt_undefined  ; 
		return exec_state_ok ; 
	  }
	}break;
	case hvt_complex_string :
	{
		printf("A complex string cannot be accessed by an index.\n");
		return exec_state_error ;
	}break;
	case hvt_complex_string_resolve :
	{
		printf("A complex string cannot be accessed by an index.\n");
		return exec_state_error ;
	}break;
	case hvt_bytes :
	{
	  PHDR_BYTES bytes = (PHDR_BYTES)ter_token->var->obj ;
	  if((iindx < 0)||(iindx >= bytes->length)) return exec_state_error ;
	  else
	  {
		if(utf8cc == NULL)
		bytes->bytes[iindx] = (char)utf8c ; /**/
		else
			bytes->bytes[iindx] = (char)dx_ConvertUtf8ToDXSc(utf8cc) ;

		dx_string_free((PDX_STRING)newval->obj);
		hdr_var_release_obj(newval) ;
		newval->type = hvt_undefined  ; 
		return exec_state_ok ; 
	  }

	} break ;
	default : printf("The variable or token is not of a type that can be accessed by an index.\n");
  }

  return exec_state_error ; /*the program cannot reach this point but you never know...*/

}


PHDR_VAR hdr_inter_resolve_expr(PHDR_INTERPRETER inter,PHDR_EXPRESSION expr,bool *error)
{
	/*
	 resolve an expression (not a boolean one) and returns a variable with the value.
	 the value can be referenced or not.

	 if a value is returned from the expression , a PHDR_VAR is returned or a NULL value is returned.
	 if the error is true , the expression has thrown an error an the execution has to be halt. If the error is
	 false then the expression was succesfully executed.

	*/

	/*
	 now the fun part! we have to parse the tokens , if we found 
	 an expression we have to resolve it and set the value to the ->value.
	 after all of the expression are resolved then we will execute the calculation
	 from left to right.
	 We can calculate ONLY numeric values , and we can concatenate ONLY 
	 string values. Every other object cannot be used in a calculation
	*/
	if(expr == NULL) return NULL ;
	*error = false;
	PHDR_VAR var = expr->value; //we will use this var to store the value
	var->is_ref  = false      ; /*initialize. Default is that the variable will be not a reference*/
	var->type = hvt_undefined	; 
	char op   = 0				;
	enum hdr_inter_vt gentype = hdr_ivt_other ;
	PDXL_NODE node = expr->tokens->start;

	while (node != NULL)
	{
		PDXL_OBJECT		   obj		= node->object	;
		PHDR_COMPLEX_TOKEN token	= obj->obj		;
		switch (token->type)
		{ 
			case hdr_tk_expression :
			{
				if (hdr_inter_resolve_expr(inter, token->expression, error) == NULL)
				{
					if (*error == true) return NULL;
				}

				if (var->type == hvt_undefined)
				{
					var->type = token->expression->value->type;
				} else
				/*check if this expression has the same type with the general expression*/
				if (var->type != token->expression->value->type)
				{
					printf("The resolved expression does not have the expected returned type : ");
					_DEBUG_PRINT_EXPRESSION_STR(token->expression);
					printf(" [%s !=  %s] ",hdr_inter_return_variable_type(var->type),hdr_inter_return_variable_type(token->expression->value->type));
					printf("\n");
					return NULL ;
				}

				/*do the calculation*/
				if (op == 0)
				{
					/*assign the value and clear the expression value to be ready for next time*/
					if (hdr_inter_assign_and_clear(var, token->expression->value, true) == false)
					{
						*error = true ;
						return NULL;
					}
				}
				else
				{
					/*check the operator and the validity of the operation and do the operation*/
					if (hdr_inter_do_calcs_concat(var, token->expression->value,op) == false)
					{
						*error = true ;
						return NULL;
					}

				}

			}break;
			case hdr_tk_literal :
			{

				/*do the calculation*/
				if (op == 0)
				{
					/*assign the value*/
					if (hdr_inter_assign_and_clear(var, token->val, false) == false)
					{
						*error = true ;
						return NULL;
					}
				}
				else
				{
					/*check the operator and the validity of the operation and do the operation*/
					if (hdr_inter_do_calcs_concat(var, token->val, op) == false) 
					{
						*error = true ;
						return NULL;
					}

				}

			}break;
			case hdr_tk_operator :
			{
				op = token->ID->stringa[0] ;
			}break;
			case hdr_tk_variable :
			{
				/*resolve the variable*/
				PHDR_VAR tvar = hdr_inter_retrieve_var_block(inter,token->ID);
				if (tvar == NULL)
				{
					printf("The variable : '%s' is not declared in the scope.\n", token->ID->stringa);
					return NULL;
				}

				/*do the calculation*/
				if (op == 0)
				{
					/*assign the value */
					if (hdr_inter_assign_and_clear(var, tvar, false) == false)
					{
						*error = true ;
						return NULL;
					}
					else
					{
					  /*if this is not a simple type then is a reference and we have to set the appropriate flag*/
					  if(hdr_inter_var_gen_type(var)==hdr_ivt_other) 
					  {
						  var->is_ref = true ;
					  }
					}
				}
				else
				{
					/*check the operator and the validity of the operation and do the operation*/
					if (hdr_inter_do_calcs_concat(var, tvar, op) == false) 
					{
						*error = true ;
						return NULL ;
					}
				}

			}break;

			case hdr_tk_function:
			{
				/*this is tricky but very important. A token that is a function can be either a hydra+ native function , or user defined one*/
				PHDR_VAR fres = NULL;
				bool ferror = hdr_inter_handle_function(inter,token,NULL, &fres);
				if (ferror == true)
				{
					return NULL;
				}
				/*the function was executed succesfully , check if does not return a value*/
				if (fres == NULL)
				{
					/*does not return a value , check if its the only token of the list because only then this is a legal action */
					if (expr->tokens->count > 0) return NULL;
					else
					{
						*error = false;
						/*
						  the NULL return is not invalid as the function is the only token in the expression
						  if the expression is an assigment , then the caller will throw the appropriate error.
						*/
						return NULL;
					}
				}
				else
				{
					/*do the calculation*/
					if (op == 0)
					{
						/*assign the value*/
						if (hdr_inter_assign_and_clear(var, fres, false) == false)
						{
							*error = true ;
							return NULL;
						}
					}
					else
					{
						/*do the operation*/
						if (hdr_inter_do_calcs_concat(var, fres, op) == false) 
						{
							*error = true ;
							return NULL;
						}
					}

					/*the fres was used just to transport the actual object data , now delete it , EXCEPT the data is a simple string, then we free the memory*/
					if (hdr_inter_var_gen_type(fres) != hdr_ivt_string)
						hdr_var_release_obj(fres); /*we do not want the actual object to be freed except if it is a simple string*/
					
					hdr_var_free(fres);
				}


			}break;

			case hdr_tk_multipart:
			{
				/*a multipart variable or token , check the type*/
				if (token->ID->stringa[0] == '$')
				{
					/*the token is a multipart variable , we will handle it and return the variable*/
					/*
						find the refered variable that the multipart token describes
					*/
					struct hdr_terminal_token ter_token;
					ter_token.var = NULL;
					ter_token.last_token = token;
					if (hdr_get_terminal_var(inter,&ter_token) == true) 
					{
						printf("The multipart token '%s' was not dereferenced to a valid value.\n", token->ID->stringa);
						*error = true ;
						return NULL;
					}
	
					if (ter_token.var == NULL)
					{
						return NULL; /*all ok no variable was retrieved but the caller must check if expecting a variable or not*/
					}
					if ((ter_token.var->type == hvt_undefined)&&(op!=0)) /*30-08-2024 support undefined returns from functions*/
					{
						printf("The expression returns a variable , but the variable is undefined and unabled to take part to any calculation.\n");
						*error = true ;

						if(ter_token.var->var_ref == hvf_temporary_ref)
						{
							/*the variable was specifically created for transport a value*/ 
							hdr_var_release_obj(ter_token.var) ;
							hdr_var_free(ter_token.var) ;
						}
						else
							if ( ter_token.var->var_ref == hvf_system ) 
							{
								if ((ter_token.var->type == hvt_simple_string)||(ter_token.var->type == hvt_simple_string_bcktck)
									|| (ter_token.var->type == hvt_unicode_string))
								{
									dx_string_free((PDX_STRING)ter_token.var->obj) ;
								}

								hdr_var_release_obj(ter_token.var) ;
							}
						return NULL;
					}


					/*do the calculation*/
					if (op == 0)
					{
						/*assign the value*/
						var->type = ter_token.var->type;
						
						if ((ter_token.var->type == hvt_bool)||(ter_token.var->type == hvt_integer)||(ter_token.var->type == hvt_codepoint))
						var->integer = ter_token.var->integer ;
						else
						if (ter_token.var->type == hvt_float)
						var->real = ter_token.var->real ;
						else
						if((ter_token.var->type == hvt_simple_string)||(ter_token.var->type == hvt_simple_string_bcktck))
						{
							PDX_STRING ns = dx_string_createU((PDX_STRING)var->obj,((PDX_STRING)(ter_token.var->obj))->stringa);
						    hdr_var_set_obj(var,ns) ;
						}
						else
						if(ter_token.var->type == hvt_unicode_string) 
						{
						   PDX_STRING ns = dx_string_createX((PDX_STRING)var->obj,((PDX_STRING)(ter_token.var->obj))->stringx); 
						   hdr_var_set_obj(var,ns) ;
						}
						else
						if(ter_token.var->type == hvt_undefined)
						{
						 var->obj = NULL ; /*30-08-2024 support undefined returns from the functions*/
						}
						else
						{
						  var->obj  = ter_token.var->obj;
						  void *obj = ter_token.var->obj ;
						  hdr_var_set_obj(var,obj) ;
						}
						/*strings is always managed by Hydra+ so free them*/
						if((ter_token.var->var_ref != hvf_block)&&(ter_token.var->var_ref != hvf_dynamic))
						{
							if((ter_token.var->type == hvt_simple_string)||(ter_token.var->type == hvt_simple_string_bcktck)||
								(ter_token.var->type == hvt_unicode_string))
							{
							   dx_string_free((PDX_STRING)ter_token.var->obj);
							   hdr_var_release_obj(ter_token.var) ;
							}
						}

						if(ter_token.var->var_ref == hvf_temporary_ref)
						{
							/*the variable was specifically created for transport a value*/ 
							hdr_var_release_obj(ter_token.var) ;
							hdr_var_free(ter_token.var) ;
						}
						else
							if ( ter_token.var->var_ref == hvf_system ) 
							{
								hdr_var_release_obj(ter_token.var) ;
							}
					}
					else
					{
						/*do the operation*/
						if (hdr_inter_do_calcs_concat(var, ter_token.var, op) == false)
						{
							*error = true ;
							return NULL;
						}
						if(ter_token.var->var_ref == hvf_temporary_ref)
						{
							/*if the variable is a simple string the string must be freed because the strings 
							  in hydra are ALWAYS copied and not referenced 
							*/
							if((ter_token.var->type == hvt_unicode_string)||(ter_token.var->type == hvt_simple_string)||
									(ter_token.var->type == hvt_simple_string_bcktck)) 
								{
									dx_string_free((PDX_STRING)ter_token.var->obj);
								}
							/*the variable was specifically created for transport a value*/ 
							hdr_var_release_obj(ter_token.var) ;
							hdr_var_free(ter_token.var) ;
						}
						else
							if ( ter_token.var->var_ref == hvf_system ) 
							{
								if((ter_token.var->type == hvt_unicode_string)||(ter_token.var->type == hvt_simple_string)||
									(ter_token.var->type == hvt_simple_string_bcktck)) 
								{
									dx_string_free((PDX_STRING)ter_token.var->obj);
									hdr_var_release_obj(ter_token.var);
								}
								else
								{
								  hdr_var_release_obj(ter_token.var);
								}
							}

					}
				}
				else
				{
					/*
					  If the multipart is a simple token and not a variable then this signify a domain special method
					*/

					PHDR_VAR ret = hdr_inter_handle_domain(inter, token, error);
					if (*error == true) return NULL;

					if ((var->type == hvt_simple_string) || (var->type == hvt_simple_string_bcktck))
					{
						/*check if we have allocate memory from another operation and free it*/
						dx_string_free((PDX_STRING)var->obj);
						hdr_var_release_obj(var)  ;
					}

					/*do the assigment*/
					if (op == 0)
					{

						var->type = ret->type;
						if ((ret->type == hvt_bool)||(ret->type == hvt_integer)||(ret->type == hvt_codepoint))
						var->integer = ret->integer ;
						else
						if (ret->type == hvt_float)
						var->real = ret->real ;
						else
						hdr_var_set_obj(var,ret->obj);
				
						hdr_var_release_obj(ret) ;
						hdr_var_free(ret) ;
					}
					else
					{
						/*do the operation*/
						if (hdr_inter_do_calcs_concat(var, ret, op) == false)
						{
							*error = true ;
							return NULL;
						}
						if(ret->var_ref == hvf_temporary_ref)
						{
							/*the variable was specifically created for transport a value*/ 
							hdr_var_release_obj(ret) ;
							hdr_var_free(ret) ;
						}
						else
							if ( ret->var_ref == hvf_system )
							{
								hdr_var_release_obj(ret) ;
							}

					}

				}
			}break;

			case hdr_tk_ref_brackets :
			{
				/*
				  an indexed token, we will return the values of the variable that the index points
				*/
				struct hdr_terminal_token ter_token;
				ter_token.var = NULL;
				ter_token.last_token = token;
				if (hdr_get_terminal_var(inter,&ter_token) == true) 
				{
					printf("The indexed token '%s' was not dereferenced to a valid value.\n", token->ID->stringa);
					*error = true ;
					return NULL;
				}
	
				if (ter_token.var == NULL)
				{
					printf("The indexed token '%s' was not dereferenced to a valid value.\n", token->ID->stringa);
					*error = true ;
					return NULL;
				}
				if (ter_token.var->type == hvt_undefined)
				{
					printf("The expression returns a variable , but the variable is undefined and unabled to take part to any calculation.\n");
					*error = true ;
					
					if(ter_token.var->var_ref == hvf_temporary_ref)
					{
						/*the variable was specifically created for transport a value*/ 
						hdr_var_release_obj(ter_token.var) ;
						hdr_var_free(ter_token.var) ;
					}
					
					return NULL;
				}

				/*
				   there is a special case in strings. The index MUST return a utf8 codepoint in numeric form.
				   we have to handle this in here!
				*/
				if((hdr_inter_var_gen_type(ter_token.var) == hdr_ivt_string)&&(ter_token.var->need_indx == true))
				{
					ter_token.var->need_indx = false ; /*reset for next time*/
					/*re enstate the token if the ter_token has been cleared, this is a special case in string indexing*/
					if(ter_token.last_token == NULL) ter_token.last_token = token ;
					/*we need a temporary variable to store the values*/
					token->val->type     = hvt_codepoint ;
					token->val->integer = hdr_inter_return_code_point_from_indx(inter,&ter_token) ;
					if(token->val->integer == -1)
					{
						printf("The indexed token '%s' was not dereferenced to a valid UTF codepoint value.\n", token->ID->stringa);
						*error = true ;
						return NULL;
					}

					if(ter_token.var->var_ref == hvf_temporary_ref)
					{
						/*the variable was specifically created for transporting a value*/ 
						hdr_var_release_obj(ter_token.var);
						hdr_var_free(ter_token.var) ;
					}

					ter_token.var = NULL ; /*we do not use this , we have a codepoint as a value its an anomaly*/
				}
				else
					if((ter_token.var->type == hvt_bytes)&&(ter_token.var->need_indx == true))
					{
					  /*return a byte value as an integer value*/

						ter_token.var->need_indx = false ; /*reset for next time*/
						/*re enstate the token if the ter_token has been cleared, this is a special case in byte indexing*/
						if(ter_token.last_token == NULL) ter_token.last_token = token ;
						/*we need a temporary variable to store the values*/
						token->val->type     = hvt_integer ;
	                    token->val->integer = hdr_inter_return_byte_value_from_indx(inter,&ter_token) ;
						if(token->val->integer == -1)
						{
							printf("The indexed token '%s' was not dereferenced to a valid byte value.\n", token->ID->stringa);
							*error = true ;
							return NULL;
						}

						if(ter_token.var->var_ref == hvf_temporary_ref)
						{
							/*the variable was specifically created for transporting a value*/ 
							hdr_var_release_obj(ter_token.var);
							hdr_var_free(ter_token.var) ;
						}

						ter_token.var = NULL ; /*we do not use this , we have an integer as a value its an anomaly*/

					}

			   /*do the calculation*/
				if (op == 0)
				{
					/*assign the value*/
					if(ter_token.var != NULL)
					{
						/*assign the value*/
						var->type = ter_token.var->type;
						if ((ter_token.var->type == hvt_bool)||(ter_token.var->type == hvt_integer)||(ter_token.var->type == hvt_codepoint))
						var->integer = ter_token.var->integer ;
						else
						if (ter_token.var->type == hvt_float)
						var->real = ter_token.var->real ;
						else
						if((ter_token.var->type == hvt_simple_string)||(ter_token.var->type == hvt_simple_string_bcktck))
						{
							/*the string type is managed!*/
							PDX_STRING ns = dx_string_createU(NULL,((PDX_STRING)ter_token.var->obj)->stringa);
							hdr_var_set_obj(var,ns) ;
						}
						else
						if(ter_token.var->type == hvt_unicode_string) 
						{
							PDX_STRING ns = dx_string_createX(NULL,((PDX_STRING)(ter_token.var->obj))->stringx);
							hdr_var_set_obj(var,ns) ;
						}
						else
						{
						  hdr_var_set_obj(var,ter_token.var->obj) ;
						}

						if(ter_token.var->var_ref == hvf_temporary_ref)
						{
							/*the variable was specifically created for transporting a value*/ 
							if((ter_token.var->type == hvt_simple_string)||(ter_token.var->type == hvt_simple_string_bcktck))
							dx_string_free((PDX_STRING)ter_token.var->obj);/*simple strings are managed by hydra+*/

							hdr_var_release_obj(ter_token.var) ;
							hdr_var_free(ter_token.var) ;
						}
					}
					else
					{
					   /*a single codepoint, a code point is an integer , do the appropriate conversions*/
						  
						  if(token->val->type == hvt_integer)
						  {
							var->type = token->val->type;
						    var->integer = (uint32_t)token->val->integer ;
						  }
						  else 
						      if( token->val->type == hvt_codepoint)
						      {
								 var->type = hvt_simple_string ;
						         char buffer[5] ;
								 if(dxConvertUint32ToUTF8(buffer, (uint32_t)token->val->integer) == 0)
								 {
								   if (inter->warnings == true) printf("Warning -> Line %d The integer [%d] is not a valid UTF codepoint.\n",inter->current_instr->instr->line,(uint32_t)token->val->integer);
								 }
								 PDX_STRING ns = dx_string_createU(NULL,buffer) ;
								 hdr_var_set_obj(var,ns);
						      }
							  else
							  {
								 printf("Warning -> Line %d Invalid indexed variable type. Index : %d\n",inter->current_instr->instr->line,(uint32_t)token->val->integer);			  
							  }
					}
				}
				else
				{
					/*do the operation*/
					if(ter_token.var != NULL)
					{
					  if (hdr_inter_do_calcs_concat(var, ter_token.var, op) == false) 
					  {
						  *error = true ;
						  return NULL;
					  }
					  if(ter_token.var->var_ref == hvf_temporary_ref)
					  {
						/*the variable was specifically created for transporting a value*/ 
						hdr_var_release_obj(ter_token.var) ;
						hdr_var_free(ter_token.var) ;
					  }

					}
					else
					{
						  /*concat a single codepoint*/
						  if (hdr_inter_do_calcs_concat(var, token->val, op) == false) 
						  {
							  *error = true ;
							  return NULL;
						  }
					}
				}	


			}break;

			case hdr_tk_simple:
			{
				/*the simple tokens can be translated to special non negative numbers , check if the token is a special case */
				enum hdr_var_type vtype ;
				DXLONG64 ret = hdr_inter_return_special_token(token,&vtype);

				if (ret == -1)
				{
					printf("The token's '%s' type '%s' is unknown.\n", token->ID->stringa, hdr_return_token_type(token));
					*error = true ;
					return NULL;
				}

				if ((var->type == hvt_simple_string) || (var->type == hvt_simple_string_bcktck))
				{
					/*check if we have allocate memory from another operation and free it*/
					dx_string_free((PDX_STRING)var->obj);
					hdr_var_release_obj(var) ;
				}

				var->type = vtype;
				var->integer = ret;

			} break;
			default : 
			{
				/*the expression does not understand the token. Throw an error*/
				printf("The token's '%s' type '%s' is invalid in an expression.\n", token->ID->stringa, hdr_return_token_type(token));
				*error = true ;
				return NULL;
			}
		}

		node = node->right ;
	}

	/*success*/
	*error = false ; 
	return var     ;
}


enum exec_state hdr_inter_resolve_and_set(PHDR_INTERPRETER inter, PHDR_VAR var, PHDR_EXPRESSION expr)
{
	/*
	 this is a very important function. In this function we set the correct value for the var.
	 The var can be an unassigned variable that means that we will set its type and value,
	 or an already assigned variable that means that we set its value after we 
	 check for type validity
	*/
	bool error;

	PHDR_VAR result = hdr_inter_resolve_expr(inter,expr,&error);

	if (result == NULL)
	{
		printf("The expression MUST always return a value for the assigment, but this expression returned NULL.\n");
		return exec_state_error ;
	}
	else
	{
		/*
		 check the validity of the assigment. From 02-09-2024 its valid to set a variable to undefined with the undef() or with 
		 $somevar = $an_undefvar ONLY if the varible is a simple string or a trivial type 
		*/
		if(result->type != hvt_undefined)
		{
			if ((var->type != hvt_undefined) && (var->type != result->type)&&
				(hdr_inter_var_gen_type(var) != hdr_inter_var_gen_type(result)))
			{
				/*OBSOLETE COMMENT :  cannot set a complex variable type to undefined with the [undef()]*/
				printf("The expression is evaluated as a different type than the variable %s. Types : [%s] != [%s].\n",
					var->name->stringa,hdr_inter_return_variable_type(var->type),hdr_inter_return_variable_type(result->type));
				return exec_state_error;
			}
		}

		switch (hdr_inter_var_gen_type(result))
		{
		  case hdr_ivt_numeric :
		  {
			/*set the numeric values*/
			  if ((result->type == hvt_integer)||(result->type == hvt_bool))
			  {
				  if ((var->type == hvt_integer)||(var->type == hvt_undefined)||(var->type == hvt_bool))
					  var->integer = result->integer;
					else
					   if(var->type != hvt_bool)
						var->real = result->integer;
					     else
						   {
							printf("The expression is evaluated as a different type than the variable %s. Types : [%s] != [%s]\n",
							var->name->stringa,hdr_inter_return_variable_type(var->type),hdr_inter_return_variable_type(result->type));
							return exec_state_error;
						   }
			  }
			  else 
				  if (result->type == hvt_float)
				  {
					  if ((var->type == hvt_float)||(var->type == hvt_undefined))
						  var->real = result->real;
					  else
						  var->integer = result->real;
				  }

			  if (var->type == hvt_undefined) var->type = result->type;
		  }break;

		  case hdr_ivt_string :
		  {
			 /*simple strings and unicode strings are always freed and recreated in assigments but in this particular case the string has been created in the expression temp val*/
			  if (var->type == hvt_undefined) var->type = result->type;

			  /*
			    check if the variable assign it self. In that case we do not proceed to the assigment
			    as it has not any meaning BUT more importantly it will invalidate the memory and the program will crash
			  */
			  PDX_STRING str = (PDX_STRING)var->obj;

			  if((str != NULL)&&(str->stringa == ((PDX_STRING)result->obj)->stringa) ) 
			  {
				  /*release the temporary dx_string*/
				  result->obj = dx_string_free((PDX_STRING)result->obj) ;
				  goto reset ;
			  }

				  str = dx_string_free(str);
				  if(var->type != hvt_unicode_string)
				  {
                   hdr_var_set_obj(var,result->obj) ;
				  }
				  else
				  {
				   /*unicode strings needs special handling*/
				   if(result->type == hvt_unicode_string)
				   str = dx_string_createX(NULL,((PDX_STRING)result->obj)->stringx) ;
				   else
						str =dx_string_createX_u(NULL,((PDX_STRING)result->obj)->stringa) ;

				   hdr_var_set_obj(var,str);
				   dx_string_free((PDX_STRING)result->obj) ;
				  }

			  reset :			  				 
			  hdr_var_release_obj(result) ;/*reset*/
			  result->type = hvt_undefined;
		  }break;
		  default :
		  {

			  /*
			    support the undef() assigment
			  */
			  if(result->type == hvt_undefined)
			  {
			    
				  if((hdr_inter_var_gen_type(var) != hdr_ivt_numeric)&&(hdr_inter_var_gen_type(var) != hdr_ivt_string)
					  &&(var->type != hvt_undefined)&&(var->type != hvt_null))
					{
					    printf("The variable [%s] cannot be set to undefined like that. Try to use its Free() or Release() function. Variable Type : [%s].\n",
						var->name->stringa,hdr_inter_return_variable_type(var->type));
						printf("If the variable is initialized in this way and then is assigned an object in a loop or function , this means that you have not Free() or Release() the variable. Please do Release() or Free() the variable after use.\n");
						return exec_state_error;
				    }

				  /*undef the variable*/
				  if(hdr_inter_var_gen_type(var) == hdr_ivt_string)
				  {
					dx_string_free((PDX_STRING)var->obj) ;
				    hdr_var_release_obj(var)             ;
				  }

				  if(var->type != hvt_undefined) var->type = hvt_undefined ;
			  }
			  else
			  {
				  /*all the other objects are passed by reference IF the type is the same!*/
				  if((var->type != hvt_undefined)&&(var->type != result->type))
				  {
				   printf("The expression is evaluated as a different type than the variable %s. Types : [%s] != [%s]\n",
						var->name->stringa,hdr_inter_return_variable_type(var->type),hdr_inter_return_variable_type(result->type));
				   return exec_state_error;
				  }
				  if (var->type == hvt_undefined) var->type = result->type;
				  hdr_var_set_obj(var,result->obj) ;
				  var->obj     = result->obj     ;
				  var->is_ref  = result->is_ref  ;
				  result->obj = NULL			  ;/*reset*/
				  result->type = hvt_undefined   ;
			
				  /****** SPECIAL CASE!! HOORAYYYY 
					 The objects have a special variable named $_OBJ_NAME
					 this is the most suitable point in the code to insert it in the code as we have all the info we need
				  *****/
				  if(var->type == hvt_object)
				  {
					  PDX_STRING oname    = dx_string_createU(NULL,var->name->stringa) ;
					  PHDR_VAR   var_obn  = hdr_var_create(oname,"$_OBJ_NAME",hvf_block,((PHDR_OBJECT_INSTANCE)var->obj)->code) ;
					  var_obn->literal = true ; /*this make the variable read only*/
					  var_obn->type    = hvt_simple_string ;
					  hdr_var_list_add(((PHDR_OBJECT_INSTANCE)var->obj)->code->variables,var_obn) ;
				  }
			  }


		  }

		} 
		
	}

	return exec_state_ok ;
}


PHDR_COMPLEX_TOKEN hdr_return_last_token_link(PHDR_COMPLEX_TOKEN token)
{

	PHDR_COMPLEX_TOKEN tkn = token ;
	while(tkn->next_part != NULL)
	{
	  tkn = tkn->next_part ;
	}
 
	return tkn ;
}

enum exec_state hdr_inter_handle_assign(PHDR_INTERPRETER inter)
{
	/*
	 the function handles the assign of a value to a variable 
	 first we check if the variable is a simple or a complex one
	*/

	PHDR_INSTRUCTION instr = inter->current_instr->instr;
	switch (instr->left_side->type)
	{
		case hdr_tk_variable  :
		{
		  /*the variable is an easy to access variable ! :D*/
			PHDR_VAR var    = hdr_inter_retrieve_var_block(inter, instr->left_side->ID);
			if (var == NULL)
			{
				hdr_inter_print_error(inter, "The variable '%s' does not exists in the scope.\n");
				return exec_state_error;
			}

			/*
			  now resolve the value of the expression in the right
			  keep in mind that if the expression is a calculation , the 
			  returned value will be stored in the permanent numeric fields of the variable.
			  If the expression is a concatenation of strings then a buffer will be created 
			  and will be set to the variable (after the old buffer is freed).

			  In case that the expression returns a complex type then 
			  the variable will be point to the complex type that was retrieved / created 
			  in the expression.
			*/
			
			return hdr_inter_resolve_and_set(inter, var, instr->right_side);
				

		}break;
		
		case hdr_tk_expression:
		{
			/*
			 return the terminal variable for the assigment, an assigment that is applied in a complex variable (indexed or multipart)
			 has its token with the parts as the first (and only) token in the left_side->expression
			
			*/

			struct hdr_terminal_token ter_token;
			ter_token.var = NULL;
			ter_token.last_token =(PHDR_COMPLEX_TOKEN)instr->left_side->expression->tokens->start->object->obj;
			if (hdr_get_terminal_var(inter, &ter_token) == true) return exec_state_error;
			/*go to the last token because the simple strings and bytes are special cases and we will handle them diferently*/
			ter_token.last_token = hdr_return_last_token_link((PHDR_COMPLEX_TOKEN)instr->left_side->expression->tokens->start->object->obj);
			if (ter_token.var == NULL)
			{
				printf("The expression on the left side of the assigment MUST return a block or a dynamic variable but none returned.\n");
				return exec_state_error;
			}


			if ((ter_token.var->var_ref != hvf_block)&&(ter_token.var->var_ref != hvf_dynamic))
			{
				printf("The instruction returns a variable , but the variable is not a persistent one like a [block] or [dynamic] variable.\n");
				return exec_state_error;
			}

			if (ter_token.var->type == hvt_undefined)
			{
				/*the undefined variable means that a new variable was dynamically created for, most propable, a List*/
				return hdr_inter_resolve_and_set(inter, ter_token.var, instr->right_side);
			}
		

			/*
			  There is an exception that we have to account for. The [strings] and [bytes] types can be accesed by the $str[$index]
			  syntax , but the characters in it are not standalone PHDR_VAR objects (that would be a waste and slow).
			  so we will interrupt the normal logic flow and we will handle here this exceptions.
			*/

			if(((hdr_inter_var_gen_type(ter_token.var) == hdr_ivt_string) || (ter_token.var->type == hvt_bytes))
				&& ((ter_token.last_token->type == hdr_tk_ref_brackets)||(ter_token.last_token->type == hdr_tk_index))&&(ter_token.var->need_indx == true) )/*special case handle, simple strings and bytes*/
			{
				ter_token.var->need_indx = false ; /*reset*/
				return hdr_inter_handle_str_indx_asgn(inter,&ter_token,instr->right_side) ;
			}
			 else /*normal flow*/
			/*ok we have the terminal variable that we will use for the assigment*/
			return hdr_inter_resolve_and_set(inter, ter_token.var, instr->right_side);

		}break;
		
		default: 
		{ 
			hdr_inter_print_error(inter,"The assigment instruction simple token (variable name) is not of one of the valid types 'hdr_tk_variable' or 'hdr_tk_expression' ");
			return exec_state_error ; 
		}
	}

	return exec_state_error ;
}


enum hdr_bool_result {hdr_b_error,hdr_b_true,hdr_b_false};
static inline bool hdr_inter_br_to_bool(enum hdr_bool_result res)
{
	if (res == hdr_b_true) return 1;
	else
		return 0;
}


static inline DXLONG64 hdr_inter_br_add(DXLONG64 o1, enum hdr_bool_result b2)
{
	DXLONG64 o2 = 0;
	if (b2 == hdr_b_true) o2 = 1;
	return (o1 && o2);
}

static inline DXLONG64 hdr_inter_br_or(DXLONG64 o1, enum hdr_bool_result b2)
{
	DXLONG64 o2 = 0;
	if (b2 == hdr_b_true) o2 = 1;
	return (o1 || o2);
}


enum hdr_bool_result hdr_inter_resolve_comparison_exp(PHDR_INTERPRETER inter , PDXL_NODE *node)
{
	PDXL_OBJECT			 obj = (*node)->object;
	PHDR_COMPLEX_TOKEN	 token = (PHDR_COMPLEX_TOKEN)obj->obj;

	bool error;
	PHDR_VAR var1 = hdr_inter_resolve_expr(inter, token->expression,&error);
	if (var1 == NULL)
	{
		printf("The comparison expression MUST return always a value , but this expression returned NULL.\n");
		return hdr_b_error;
	}
	/*find the operator*/
	(*node) = (*node)->right	;
	if ((*node) == NULL)
	{
		printf("Internal Error. The second node (operator) of the comparison expression is NULL\n");
		return hdr_b_error;
	}
	obj     = (*node)->object				;
    token   = (PHDR_COMPLEX_TOKEN)obj->obj	;
	PDX_STRING			 op = token->ID		;

	(*node) = (*node)->right;
	if ((*node) == NULL)
	{
		printf("Internal Error. The third node (second operand) of the comparison expression is NULL\n");
		return hdr_b_error;
	}
	obj   = (*node)->object;
	token = (PHDR_COMPLEX_TOKEN)obj->obj;
	PHDR_VAR var2 = hdr_inter_resolve_expr(inter, token->expression,&error);
	if (var2 == NULL) 
	{
		printf("The comparison expression MUST return always a value , but this expression returned NULL.\n");
		return hdr_b_error;
	}


	/*do the calculation first check the types*/
	if (hdr_inter_var_gen_type(var1) != hdr_inter_var_gen_type(var2))
	{
		printf("The values that are being compared in the boolean expression are not of the same type.\n");
		return hdr_b_error;
	}
	/*well... this will be tedious....*/
	/*
	 the only thing that save as is that we can use the comparison operators only 
	 with strings (only the == and !=) and numbers (all the operators)

	*/

	if ((var1->type != hvt_float) && (var1->type != hvt_integer) 
		&& (var1->type != hvt_codepoint) && (var1->type != hvt_simple_string) 
		&& (var1->type != hvt_simple_string_bcktck)&&(var1->type != hvt_bool))
	{
		printf("The values that are being compared in the boolean expression are not one of the valid types (simple string , float and integer).\n");
		return hdr_b_error;
	}

	if (hdr_inter_fast_two(op, "==") == true)
	{

		if ((var1->type == hvt_float))
		{
			if (var2->type == hvt_float)
			{
				
				if (dx_float_compare(var1->real,var2->real) == dx_equal ) return hdr_b_true;
				else return hdr_b_false;
			}
			else
				if ((var2->type == hvt_integer)||(var2->type == hvt_codepoint)||(var1->type == hvt_bool))
				{
					if ((DXLONG64)var1->real == var2->integer) return hdr_b_true;
					else return hdr_b_false;
				}
		}
		else
		if ((var1->type == hvt_integer)||(var1->type == hvt_codepoint)||(var1->type == hvt_bool))
		{
			if (var2->type == hvt_float)
			{

				if (var1->integer == (DXLONG64)var2->real) return hdr_b_true;
				else return hdr_b_false;
			}
			else
				if ((var2->type == hvt_integer)||(var2->type == hvt_codepoint)||(var1->type == hvt_bool))
				{
					if (var1->integer == var2->integer) return hdr_b_true;
					else return hdr_b_false;
				}
		}
		else
		if ((var1->type == hvt_simple_string)||(var1->type == hvt_simple_string_bcktck))
		{
			/*trust no bug will make it in here so we will do not check for validity the str1 and str2 XD*/
			PDX_STRING str1 = (PDX_STRING)var1->obj;
			PDX_STRING str2 = (PDX_STRING)var2->obj;
			if (dx_string_native_compare(str1, str2) == dx_equal) 
			{
				str1 = dx_string_free(str1) ;
				str2 = dx_string_free(str2) ;
				hdr_var_release_obj(var1)   ;
				hdr_var_release_obj(var2)   ;
				return hdr_b_true	 ;
			}
			else
			{
				str1 = dx_string_free(str1);
				str2 = dx_string_free(str2);
				hdr_var_release_obj(var1)   ;
				hdr_var_release_obj(var2)   ;
				return hdr_b_false;
			}
		}
	}
	else
	if (hdr_inter_fast_two(op, "!=")==true)
	{
		if ((var1->type == hvt_float))
		{
			if (var2->type == hvt_float)
			{

				if (dx_float_compare(var1->real, var2->real) != dx_equal) return hdr_b_true;
				else return hdr_b_false;
			}
			else
				if ((var2->type == hvt_integer)||(var2->type == hvt_codepoint)||(var1->type == hvt_bool))
				{
					if ((DXLONG64)var1->real != var2->integer) return hdr_b_true;
					else return hdr_b_false;
				}
		}
		else
			if ((var1->type == hvt_integer)||(var1->type == hvt_codepoint)||(var1->type == hvt_bool))
			{
				if (var2->type == hvt_float)
				{

					if (var1->integer != (DXLONG64)var2->real) return hdr_b_true;
					else return hdr_b_false;
				}
				else
					if ((var2->type == hvt_integer)||(var2->type == hvt_codepoint)||(var1->type == hvt_bool))
					{
						if (var1->integer != var2->integer) return hdr_b_true;
						else return hdr_b_false;
					}
			}
			else
				if ((var1->type == hvt_simple_string) || (var1->type == hvt_simple_string_bcktck))
				{
					/*trust no bug will make it in here so we will do not check for validity for the str1 and str2 XD*/
					PDX_STRING str1 = (PDX_STRING)var1->obj;
					PDX_STRING str2 = (PDX_STRING)var2->obj;
					if (dx_string_native_compare(str1, str2) != dx_equal)
					{
						str1 = dx_string_free(str1);
						str2 = dx_string_free(str2);
						hdr_var_release_obj(var1)   ;
						hdr_var_release_obj(var2)   ;
						return hdr_b_true ;
					}
					else
					{
						str1 = dx_string_free(str1);
						str2 = dx_string_free(str2);
						hdr_var_release_obj(var1)   ;
						hdr_var_release_obj(var2)   ;
						return hdr_b_false;
					}
				}
	}else
	if (hdr_inter_fast_two(op, "<=")==true)
	{
		if(var1->type == hvt_bool)
		{
		 /*do not permit this comparison xD*/
		    printf("The operator '<=' does not being supported for boolean value\n");
			return hdr_b_error;
		}
		if ((var1->type == hvt_simple_string) || (var1->type == hvt_simple_string_bcktck))
		{
			printf("The operator '<=' does not being supported for strings\n");
			return hdr_b_error;
		}

		if ((var1->type == hvt_float))
		{
			if (var2->type == hvt_float)
			{

				if ((dx_float_compare(var1->real, var2->real) == dx_right_bigger)||
					(dx_float_compare(var1->real, var2->real) == dx_equal)) return hdr_b_true;
				else return hdr_b_false;
			}
			else
				if ((var2->type == hvt_integer)||(var2->type == hvt_codepoint))
				{
					if ((DXLONG64)var1->real <= var2->integer) return hdr_b_true;
					else return hdr_b_false;
				}
		}
		else
			if ((var1->type == hvt_integer)||(var1->type == hvt_codepoint))
			{
				if (var2->type == hvt_float)
				{

					if (var1->integer <= (DXLONG64)var2->real) return hdr_b_true;
					else return hdr_b_false;
				}
				else
					if ((var2->type == hvt_integer)||(var2->type == hvt_codepoint))
					{
						if (var1->integer <= var2->integer) return hdr_b_true;
						else return hdr_b_false;
					}
			}

	}else
	if (hdr_inter_fast_two(op, ">=")==true)
	{
		if(var1->type == hvt_bool)
		{
		 /*do not permit this comparison xD*/
		    printf("The operator '>=' does not being supported for boolean value\n");
			return hdr_b_error;
		}
		if ((var1->type == hvt_simple_string) || (var1->type == hvt_simple_string_bcktck))
		{
			printf("The operator '>=' does not being supported for strings\n");
			return hdr_b_error;
		}

		if ((var1->type == hvt_float))
		{
			if (var2->type == hvt_float)
			{

				if ((dx_float_compare(var1->real, var2->real) == dx_left_bigger) ||
					(dx_float_compare(var1->real, var2->real) == dx_equal)) return hdr_b_true;
				else return hdr_b_false;
			}
			else
				if ((var2->type == hvt_integer)||(var2->type == hvt_codepoint))
				{
					if ((DXLONG64)var1->real >= var2->integer) return hdr_b_true;
					else return hdr_b_false;
				}
		}
		else
			if ((var1->type == hvt_integer)||(var1->type == hvt_codepoint))
			{
				if (var2->type == hvt_float)
				{

					if (var1->integer >= (DXLONG64)var2->real) return hdr_b_true;
					else return hdr_b_false;
				}
				else
					if ((var2->type == hvt_integer)||(var2->type == hvt_codepoint))
					{
						if (var1->integer >= var2->integer) return hdr_b_true;
						else return hdr_b_false;
					}
			}
	}else
	if (op->stringa[0] == '<')
	{
		if(var1->type == hvt_bool)
		{
		 /*do not permit this comparison xD*/
		    printf("The operator '<' does not being supported for boolean value\n");
			return hdr_b_error;
		}

		if ((var1->type == hvt_simple_string) || (var1->type == hvt_simple_string_bcktck))
		{
			printf("The operator '<' does not being supported for strings\n");
			return hdr_b_error ;
		}

		if ((var1->type == hvt_float))
		{
			if (var2->type == hvt_float)
			{

				if (dx_float_compare(var1->real, var2->real) == dx_right_bigger) return hdr_b_true;
				else return hdr_b_false;
			}
			else
				if ((var2->type == hvt_integer)||(var2->type == hvt_codepoint))
				{
					if ((DXLONG64)var1->real < var2->integer) return hdr_b_true;
					else return hdr_b_false;
				}
		}
		else
			if ((var1->type == hvt_integer)||(var1->type == hvt_codepoint))
			{
				if (var2->type == hvt_float)
				{

					if (var1->integer < (DXLONG64)var2->real) return hdr_b_true;
					else return hdr_b_false;
				}
				else
					if ((var2->type == hvt_integer)||(var2->type == hvt_codepoint))
					{
						if (var1->integer < var2->integer) return hdr_b_true;
						else return hdr_b_false;
					}
			}
			
	}else
	if (op->stringa[0] == '>')
	{
		if(var1->type == hvt_bool)
		{
		 /*do not permit this comparison xD*/
		    printf("The operator '>' does not being supported for boolean value\n");
			return hdr_b_error;
		}

		if ((var1->type == hvt_simple_string) || (var1->type == hvt_simple_string_bcktck))
		{
			printf("The operator '>' does not being supported for strings\n");
			return hdr_b_error;
		}

		if ((var1->type == hvt_float))
		{
			if (var2->type == hvt_float)
			{

				if (dx_float_compare(var1->real, var2->real) == dx_left_bigger) return hdr_b_true;
				else return hdr_b_false;
			}
			else
				if ((var2->type == hvt_integer)||(var2->type == hvt_codepoint))
				{
					if ((DXLONG64)var1->real > var2->integer) return hdr_b_true;
					else return hdr_b_false;
				}
		}
		else
			if ((var1->type == hvt_integer)||(var1->type == hvt_codepoint))
			{
				if (var2->type == hvt_float)
				{

					if (var1->integer > (DXLONG64)var2->real) return hdr_b_true;
					else return hdr_b_false;
				}
				else
					if ((var2->type == hvt_integer)||(var2->type == hvt_codepoint))
					{
						if (var1->integer > var2->integer) return hdr_b_true;
						else return hdr_b_false;
					}
			}

	}
	else
	{
		printf("An unrecognized operator was found in the comparison expression. Operator : '%s'", op->stringa);
		return hdr_b_error;
	}

	return hdr_b_error;
}

enum hdr_bool_result hdr_inter_resolve_bool_exp(PHDR_INTERPRETER inter ,PHDR_EXPRESSION expr)
{
	PDX_STRING			op		= NULL			;
	PHDR_VAR			base_expr_var = NULL	; /*this will be used to accumulate the results*/

	switch (expr->type)
	{
		case hdr_expr_comparison:
		{
			printf("An erroneus expression type (hdr_expr_comparison) was found in the boolean expression.\n");
			return hdr_b_error;
		}break;
		case hdr_expr_boolean:
		{
			/*get all the boolean expressions / comparisons*/
			PDXL_NODE node = expr->tokens->start;
			while (node != NULL)
			{
				PDXL_OBJECT			obj		= node->object ;
				PHDR_COMPLEX_TOKEN	token   = (PHDR_COMPLEX_TOKEN)obj->obj;
				switch (token->type)
				{
					case hdr_tk_bool_expression:
					{
						enum hdr_bool_result res = hdr_inter_resolve_bool_exp(inter, token->expression) ;
						if (res == hdr_b_error) return hdr_b_error;
						/*check if the op is null meaning the value is the first one */
						if (op == NULL) expr->value->integer = hdr_inter_br_to_bool(res);
						else
						{

							if (op->stringa[0] == '&') expr->value->integer = hdr_inter_br_add(expr->value->integer, res);
							else
								if (op->stringa[0] == '|') expr->value->integer = hdr_inter_br_or(expr->value->integer, res);
							/*reset op*/
							op = NULL;
						}
						
					}break;

					case hdr_tk_comp_expression:
					{
						/*
						 if we are in there we are sure that follows one operator and one operand
						 so we will handle all the the expression in here
						 the hdr_inter_resolve_comparison_exp will nodify the node to the next one
						*/
						enum hdr_bool_result res = hdr_inter_resolve_comparison_exp(inter,&node) ;
						if (res == hdr_b_error) return hdr_b_error;
						else
							return res ;
					}break;
					case hdr_tk_operator:
					{
						/*
						  the operator is a logical operator and not a comparison operator as the comparison 
						  is handled in its own function (hdr_inter_resolve_comparison_exp) ;
						*/
						op = token->ID ;
					}break;
				}
				if(node != NULL) /*the node is modified elsewere too so check for ending*/
				node = node->right;
			}

		}break;

		default: { printf("The expression is not of a valid boolean or comparison type.\n"); return hdr_b_error; }
	}

	/*so far so good , create the right value*/

	if (expr->value->integer == 0) return hdr_b_false;
	else
		return hdr_b_true;

}

enum exec_state hdr_inter_handle_if(PHDR_INTERPRETER inter)
{
  /*
    the function branches in the right code block
    as the if instruction is self contained not actual branching
	is occured in the main code block of the interpreted and all the if...else struct 
	is handled in on instruction. 
  */

	enum exec_state exec_res = exec_state_ok;
	/*first get the expression result*/
	PHDR_INSTRUCTION     instr		 = inter->current_instr->instr  ;
	enum hdr_bool_result bool_result = hdr_inter_resolve_bool_exp(inter,instr->right_side);

	if (bool_result == hdr_b_error)
	{
		printf("Error in the resolving of the boolean expression\n");
		return exec_state_error;
	}
	
	/*check the result and execute the code */

   /*save the state as the current instruction and block  will be changed in the interpreter*/
	PHDR_INTER_STORE saved_state = hdr_inter_store(inter);
	if (saved_state == NULL)
	{
		printf("Fatal Error -> Line : %d INTERNAL ERROR : MEMORY NOT ALLOCATED.\n", inter->current_instr->instr->line);
		return exec_state_error;
	}


	if (bool_result == hdr_b_true)
	{
		/*execute main code block*/
		/*set the new code block and current instruction*/
		if (instr->code == NULL)
		{
			printf("Fatal Error -> Line : %d INTERNAL ERROR : THE 'IF' MAIN CODE BLOCK IS NULL.\n", inter->current_instr->instr->line);
			return exec_state_error;
		}
		hdr_inter_set_block(inter, instr->code);
		/*execute*/
		exec_res = hdr_inter_execute_instructions(inter);
		if (exec_res == exec_state_error)
		{
			printf("Fatal Error -> 'if' main code failed to execute. See the above messages.\n");
			return exec_state_error;
		}

	}
	else
	{
		/*execute the else code*/
		if (instr->ielse == NULL)
		{
			exec_res = exec_state_ok;
			goto end;
		}/*no else is set */

		 if(instr->ielse->code == NULL)
	 	 {
			printf("Fatal Error -> Line : %d INTERNAL ERROR : THE 'IF' IELSE INSTRUCTION CODE BLOCK IS NULL.\n", inter->current_instr->instr->line);
			return exec_state_error;
		 }
		hdr_inter_set_block(inter, instr->ielse->code);
		/*execute*/
		exec_res = hdr_inter_execute_instructions(inter) ;
		if ( exec_res == exec_state_error)
		{
			printf("Fatal Error -> 'if' [else] code failed to execute. See the above messages.\n");
			goto end;
		}
	}

	/*restore the state of the interpreter*/
	end:
	hdr_inter_restore(inter, saved_state);
	return exec_res ;
}


enum exec_state  hdr_inter_handle_switch(PHDR_INTERPRETER inter)
{
	/*
	 handle the switch instruction
	*/
	PHDR_INTER_STORE saved_state = NULL ;
	enum exec_state exec_res = exec_state_ok;
	PHDR_INSTRUCTION     instr = inter->current_instr->instr;
	bool error; 
	/*get the value that we will check*/
	PHDR_VAR value = hdr_inter_resolve_expr(inter, instr->right_side,&error) ;
	if (value != NULL)
	{
		if (value->type == hvt_undefined)
		{
			printf("Error in 'switch' expression. The type returned was 'undefined'.\n");
			exec_res = exec_state_error;
			goto exit ;
		}
	}
	else
	{
		printf("Error in 'switch' expression. A 'switch' expression MUST always return a value, but this one returned NULL.\n");
		exec_res = exec_state_error;
		goto exit ;
	}

   /*save the state as the current instruction and block  will be changed in the interpreter*/
	saved_state = hdr_inter_store(inter);
	if (saved_state == NULL)
	{
		printf("Fatal Error -> Line : %d INTERNAL ERROR : MEMORY NOT ALLOCATED.\n", inter->current_instr->instr->line);
		exec_res = exec_state_error;
		goto exit ;
	}

	/*point to first case*/
	instr = instr->ielse ;
	/*check all the cases until found one to match*/
	while (instr != NULL)
	{

		PHDR_VAR casev = NULL;
		if (instr->type != hdr_i_switch_default) /*the default value will be executed if the execution reach it in any case */
		{
			casev = hdr_inter_resolve_expr(inter, instr->right_side,&error);
			if (casev == NULL)
			{
				printf("The value type of the 'switch' expression to check return NULL.\n" );
				exec_res = exec_state_error;
			    goto exit ;
			}
			
		}

		/*
		 check for the default value. Loader has make sure that the 'default' is always the last ielse
		 so if we reach it then we execute it
		*/
		if (instr->type == hdr_i_switch_default)
		{
			hdr_inter_set_block(inter, instr->code);
			exec_res = hdr_inter_execute_instructions(inter);
			if ( exec_res == exec_state_error)
			{
				printf("Fatal Error -> 'switch' 'case' code failed to execute. See the above messages.\n");
				goto exit ;
			}
			goto exit;
		}
		else
		{
			enum hdr_inter_vt gentype = hdr_inter_var_gen_type(value);

			if (gentype == hdr_ivt_other)
			{
				printf("The type of the switch value is not one of the valid types. Valid types are [numeric] or [simple string].\n");
				exec_res = exec_state_error;
				goto exit ;
			}
			bool res = false;
			if (gentype == hdr_ivt_numeric)
			{
				if ((value->type == hvt_integer)||(value->type == hvt_codepoint))
				{
					if(casev->type == hvt_integer)
					{
					 if (value->integer == casev->integer) res = true;
					}
					else
						if (value->integer == casev->real) res = true;
				}
				else
				{
					/*float*/
					if(casev->real)
					{
					 if (value->real == casev->real) res = true;
					}
					else
						if (value->real == casev->integer) res = true;
				}
			}
			else
				if (gentype == hdr_ivt_string)
				{
					PDX_STRING strv = (PDX_STRING)value->obj;
					PDX_STRING strc = (PDX_STRING)casev->obj;
					if (dx_string_native_compare(strv, strc) == dx_equal) res = true;
					strc = dx_string_free(strc);
					hdr_var_release_obj(casev) ;
				}

			if (res == true)
			{
				hdr_inter_set_block(inter, instr->code);
				exec_res = hdr_inter_execute_instructions(inter) ; 
				if ( exec_res == exec_state_error)
				{
					printf("Fatal Error -> 'switch' 'case' code failed to execute. See the above messages.\n");
					goto exit ;
				}
				else
				{
				  goto exit;
				}
			}

		}

		instr = instr->ielse ;
	}

exit :
	if(value != NULL) dx_string_free((PDX_STRING)value->obj);
	hdr_var_release_obj(value) ;
	if(saved_state != NULL) saved_state = hdr_inter_restore(inter, saved_state);
	
	return exec_res ;
}

enum exec_state hdr_inter_handle_loop(PHDR_INTERPRETER inter)
{
	/*
	  the function handles a loop.
	  loops can be nested and the interpreter has to handle the 'break' and 'continue'
	  instructions.
	*/

	inter->loops++;
	
	/*save the state as the current instruction and block  will be changed in the interpreter*/
	PHDR_INTER_STORE saved_state = hdr_inter_store(inter);
	if (saved_state == NULL)
	{
		printf("Fatal Error -> Line : %d INTERNAL ERROR ON SAVED_STATE: MEMORY NOT ALLOCATED.\n", inter->current_instr->instr->line);
		return false;
	}

	enum exec_state exec_state = exec_state_ok ;
	/*set the block*/
	PHDR_INSTRUCTION     instr = inter->current_instr->instr;
	hdr_inter_set_block(inter, instr->code);
	/*execute the code while the break (or the continue) is not encounter*/
	while (true)
	{
		/*we will make another check into the loop to make it thread aware */
		if(inter->thread!=NULL)
		if(inter->thread->terminate == true) return exec_state_error ;
		/*******************************************************************/
		exec_state = hdr_inter_execute_instructions(inter) ;
		if ( exec_state == exec_state_error)
		{
			printf("Fatal Error -> An error occured in the 'loop' execution. See the above messages.\n");
			goto exit ;
		}
		else 
			if ((exec_state == exec_state_loop_exit)||(exec_state == exec_state_return))
			{
				if(exec_state == exec_state_return) goto exit ;
				break;
			}
			else
				if (exec_state == exec_state_loop_cont)
				{
					/*reset the loop*/
					inter->current_instr->node  = inter->current_block->instructions->start;
					inter->current_instr->instr = hdr_interpreter_ret_instr(inter->current_block->instructions->start);
				}
				else
				{
					/*reset the instruction pointer for the next loop cycle*/
					inter->current_instr->instr = hdr_interpreter_ret_instr(inter->current_block->instructions->start);
					inter->current_instr->node = inter->current_block->instructions->start;
				}
		
	}

	saved_state = hdr_inter_restore(inter, saved_state);
	inter->loops--;
	return exec_state_ok;

exit :
	saved_state = hdr_inter_restore(inter, saved_state);
	inter->loops--;
	return exec_state ;
}


enum exec_state hdr_inter_execute_instruction(PHDR_INTERPRETER inter)
{
	/*
	 the function calls the rignt functions to execute a singular instruction
	 if there is a problem in the execution the function returns false and the interpreter will 
	 emmit a fatal error
	*/
	
	if (inter->current_instr->instr == NULL) return exec_state_ok;
	PHDR_INSTRUCTION instr = inter->current_instr->instr;

	enum exec_state result = exec_state_ok;
	switch (inter->current_instr->instr->type)
	{
		case hdr_i_assign:
		{
			result = hdr_inter_handle_assign(inter) ;
			if (result == exec_state_error)
			{
				hdr_inter_print_error(inter, "Error in the assigment : ");

				_debug_print_indexed_token(inter->current_instr->instr->left_side);
				printf(" = ");
				_DEBUG_PRINT_EXPRESSION_STR(inter->current_instr->instr->right_side);
				printf("\n\n");
				return exec_state_error; 
			}
		}break;

		case hdr_i_if :
		{
			result = hdr_inter_handle_if(inter);
			if (result == exec_state_error)
			{
				hdr_inter_print_error(inter, "Error in the 'if' instruction : if");
				if (inter->last_instr!= NULL)
				{
					printf("(\n");
					_DEBUG_PRINT_EXPRESSION_STR(instr->right_side);
					printf("\n)\n\n");
				}
				return exec_state_error;
			}
		}break;

		case hdr_i_switch	 : 
		{
			result = hdr_inter_handle_switch(inter);
			if (result == exec_state_error)
			{
				hdr_inter_print_error(inter, "Error in the 'switch' instruction : switch");
				if (inter->last_instr != NULL)
				{
					printf("(\n");
					_DEBUG_PRINT_EXPRESSION_STR(instr->right_side);
					printf("\n)\n\n");
				}
				
				return exec_state_error;
			}
		}break;

		case hdr_i_loop:
		{
			result = hdr_inter_handle_loop(inter);
			if (result == exec_state_error)
			{
				hdr_inter_print_error(inter, "Error in a 'loop' instruction");
				return exec_state_error;
			}
		}break;

		case hdr_i_cmd :
		{
			/******************************************************************************************/
			PHDR_INSTRUCTION instr = inter->current_instr->instr;
			/*retrieve the first token of the instructions expression*/
			PHDR_COMPLEX_TOKEN ctoken = hdr_inter_return_list_token(instr->right_side->tokens, 0);
			/******************************************************************************************/

			/*check if the token is a function before continue*/
			if(ctoken->type == hdr_tk_function)
			{
				PHDR_VAR fres = NULL;
				bool ferror = hdr_inter_handle_function(inter, ctoken, NULL,&fres);

				if (ferror == true)
				{
					hdr_inter_print_error(inter, "Error executing the function.\n");
					return exec_state_error;
				}
				/*the fres was used just to transport the actual object data , now delete it , EXCEPT the data is a simple string, then we free the memory */
				if (fres != NULL)
				{
					if (hdr_inter_var_gen_type(fres) != hdr_ivt_string)
					{
						hdr_var_release_obj(fres) ;
					}
					hdr_var_free(fres);
				}
			}
			else
			/*check for the special instruction break , all the trivial commands will be handled by a function in the appropriate module*/
			if (hdr_inter_fast_str(ctoken->ID, "break",5) == true)
			{
				/*check if the interpreter is in a loop*/
				if (inter->loops <= 0)
				{
					hdr_inter_print_error(inter, "Stray 'break' instruction. A 'break' instruction is valid ONLY inside a loop.\n");
					return exec_state_error;
				}
				return exec_state_loop_exit ;/*signal all the functions to exit the loop*/
			}
			else
			/*check for the special instruction continue , all the trivial commands will be handled by a function in the appropriate module*/
			if (hdr_inter_fast_str(ctoken->ID, "continue", 8) == true)
			{
				/*check if the interpreter is in a loop*/
				if (inter->loops <= 0)
				{
					hdr_inter_print_error(inter, "Stray 'continue' instruction. A 'continue' instruction is valid ONLY inside a loop.\n");
					return exec_state_error;
				}
				
				/*signal the interpreter to retyurn to the first loop command*/
				return exec_state_loop_cont;

			}
			 else
				{
					/*send the instruction to the commands module and get the result*/
					result = hdr_inter_handle_single_command(inter, ctoken);
				}

		}break;

		case hdr_i_cmd_param :
		{

			PHDR_INSTRUCTION    instr	  = inter->current_instr->instr ;
			PHDR_COMPLEX_TOKEN  ctoken    = instr->left_side			;  /*the command's name*/
			PHDR_EXPRESSION		expr	  = instr->right_side			;  /*the expression*/
			if (hdr_inter_fast_str(ctoken->ID, "continue", 8) == true)
			{
				/*check if the interpreter is in a loop*/
				if (inter->loops <= 0)
				{
					hdr_inter_print_error(inter, "Stray 'continue' instruction. A 'continue' instruction is valid ONLY inside a loop.\n");
					return exec_state_error;
				}

				/*execute the command after the continue. The command is valid only if its a single function */

				if (expr->tokens->count > 1)
				{
					hdr_inter_print_error(inter, "The 'continue' instruction can be followed ONLY by a function. Example : 'continue inc($indx);'\n");
					return exec_state_error;
				}

				/*execute the function*/
				PDXL_OBJECT obj			 = expr->tokens->start->object  ;
				PHDR_COMPLEX_TOKEN token = (PHDR_COMPLEX_TOKEN)obj->obj;
				PHDR_VAR fvar = NULL;
				bool ferror = hdr_inter_handle_function(inter, token,NULL, &fvar);
				if (ferror == true)
				{
					hdr_inter_print_error(inter, "The function after the 'continue' instruction failed.'\n");
					return exec_state_error;
				}

				/*the fvar is not assigned to anything either way because the command cannot be assigned to a variable*/
				if (fvar != NULL)
				{

					if (hdr_inter_var_gen_type(fvar) != hdr_ivt_string)
					{
						hdr_var_release_obj(fvar) ;
					}
					hdr_var_free(fvar);
					printf("The [command] cannot return a value. Something is wrong in the Hydra+ core.\n");
					return exec_state_error ; 
				}
				/*return the execution to the first loop command*/
				return exec_state_loop_cont;

			}
			else
			{
				result = hdr_inter_handle_complex_command(inter, ctoken, expr);
			}

		}break;

		case hdr_i_uninit: { printf("The instruction in line %d is uninitialized. Propably a bug is at large ?\n", inter->current_instr->instr->line); return false; }break;

	}


	inter->last_instr = inter->current_instr->instr;
	if (result == exec_state_error ) hdr_inter_print_error(inter, "Error in execution. See above messages.\n");
	hdr_inter_next_instr(inter); /*next instruction*/
	return result;

}


enum exec_state  hdr_inter_execute_instructions(PHDR_INTERPRETER inter)
{
	while (true)
	{
		/*the current instruction is modified inside the execute instruction function*/
		enum exec_state exec_state = hdr_inter_execute_instruction(inter)   ;
		if (exec_state == exec_state_error )return exec_state_error 	    ;
		if(exec_state == exec_state_return) return exec_state_return		;
		if ((exec_state == exec_state_loop_exit)||(exec_state == exec_state_loop_cont))
		{
			inter->current_instr->instr = NULL  ;
			return exec_state			;
		}
		if (inter->current_instr->instr == NULL) return	exec_state_ok		;/*end of code*/
	}

	return exec_state_error;
}

enum exec_state  hdr_inter_execute_instructions_thread(PHDR_INTERPRETER inter)
{
	/*
	  this function is called in an interpreter that executes an async function.
	  the function checks the signal for termination and stops acordinally.
	*/
	while (true)
	{
		if(inter->thread->terminate == true) return exec_state_error ;
		/*the current instruction is modified inside the execute instruction function*/
		enum exec_state exec_state = hdr_inter_execute_instruction(inter)   ;
		if (exec_state == exec_state_error )return exec_state_error 	    ;
		if(exec_state == exec_state_return) return exec_state_return		;
		if ((exec_state == exec_state_loop_exit)||(exec_state == exec_state_loop_cont))
		{
			inter->current_instr->instr = NULL  ;
			return exec_state			;
		}
		if (inter->current_instr->instr == NULL) return	exec_state_ok		;/*end of code*/
	}

	return exec_state_error;
}





/*********************** UTF8 helper functions ****************************************/

bool hdr_inter_replace_utf8_character(PHDR_INTERPRETER inter ,PDX_STRING str,PHDR_VAR newval, 
	DXUTF8CHAR utf8c , char * utf8cc ,DXLONG64 iindx)
{
	/*get the utf8char position and replace it with the actual value*/
	if( (iindx < 0)||(iindx >= str->len) )
	{
		printf("The supplied index of '%d' is out of valid range [0 - %d]\n",iindx,str->len);
		return false ;
	}

	/*
		now we have some work to do. Hydra+ strings are utf8 strings, so we need to calculate the position
		and bytes of the character in the string. Needless to say that this is a VERY cpu expensive operation.
	*/
	char *char_indx = str->stringa  ; 
	char *prev_char = NULL			;
	int char_len					;
	for(int i = 0 ; i <= iindx;i++)
	{
		dx_get_utf8_char_ex2(&char_indx,&prev_char,&char_len) ;
	}

	/*
		we point to the character after the index.
		check the validity of the characters , we replace characters
		that are in the same byte count range , else we throw an error
	*/
	short bcnt = 0 ;
	if (utf8cc != NULL) bcnt = ((PDX_STRING)newval->obj)->bcount ;
	else
	bcnt = dxUtf8CpointByteCount(utf8c) ;
	if(bcnt != char_len )
	{
		printf("The replacement of the character in the position %d is not posible as the new character (utf8 codepoint) is not in the same byte range or the value is not a valid utf8 character.\n",iindx);
		return false ;
	}
	/*replace the character*/

		if (utf8cc != NULL)
		{
			/*we have a utf8 character not a codepoint*/
			for(short i = 0 ; i < bcnt;i++)
			{
				*prev_char = *utf8cc ;
				prev_char++;
				utf8cc++ ;
			}
			  
		}
		else
		{
			/*we have a number lets create a utf8 character*/
			char utftmp[5] ;
			if(dxConvertUint32ToUTF8(utftmp, utf8c) == 0) 
			{
				printf("The replacement of the character in the position %d is not posible as the new character (utf8 codepoint) is not a valid utf8 character.\n",iindx);
				return false ;
			}
			/*set the char*/
			for(short i = 0 ; i < bcnt;i++)
			{
				*prev_char = utftmp[i] ;
				prev_char++;
			}
		}

  	return true;

}

DXLONG64 hdr_inter_retrieve_utf8_character(PDX_STRING str, DXLONG64 iindx)
{
	/*get the utf8char position get the code point and return it as a DXLONG value*/
	if((iindx < 0)||(iindx >= str->len) )
	{
		printf("The supplied index of '%d' is out of valid range [0 - %d]\n",iindx,str->len);
		return -1 ;
	}

	/*
		now we have some work to do. Hydra+ strings are utf8 strings, so we need to calculate the position
		and bytes of the character in the string. Needless to say that this is a VERY cpu expensive operation.
	*/
	char *char_indx = str->stringa  ; 
	char *prev_char = NULL			;
	int char_len					;
	for(int i = 0 ; i <= iindx;i++)
	{
		dx_get_utf8_char_ex2(&char_indx,&prev_char,&char_len) ;
	}

	/*transform to int*/
   return  dxConvertUTF8ToInt32(prev_char,char_len) ; 	 /*cast to long64*/ 

}

DXLONG64 hdr_inter_return_code_point_from_indx(PHDR_INTERPRETER inter , PHDR_TERMINAL_TOKEN ter_token)
{
	 /*
	  The function gets the variable and the index in the ter_token
	  and return the code point in UTF from the character.
	  In error returns -1 
	 */
  
  bool error	= false ;
  PHDR_VAR indx = hdr_inter_resolve_expr(inter,ter_token->last_token->expression,&error) ;
  if(error == true)
  {
	printf("The supplied index for the '%s' variable was not resolved in a valid value.\n",ter_token->var->name->stringa);
	return exec_state_error ;
  }

  if((indx->type != hvt_integer)&&(indx->type != hvt_float))
  {
	printf("The supplied index for the '%s' variable MUST be of 'Numeric' type.\n",ter_token->var->name->stringa);
	return -1 ;
  }

  /*get the number*/
  DXLONG64 iindx = 0 ;
  if(indx->type == hvt_integer) iindx = indx->integer ;
  else
	  iindx = (DXLONG64)floor(indx->real) ; 

  if ((ter_token->var->type == hvt_simple_string) || (ter_token->var->type == hvt_simple_string_bcktck))
  {
    /*ok get the character*/
    return hdr_inter_retrieve_utf8_character((PDX_STRING)ter_token->var->obj,iindx) ;
	
  } else
	  if (ter_token->var->type == hvt_unicode_string)
	   {
			/*a native hydra unicode string*/
			if( (iindx < 0)||(iindx >= ((PDX_STRING)ter_token->var->obj)->len) )
			{
			 printf("The supplied index for the '%s' variable is out of range. Index : %d Range : 0 - %d.\n",
				 ter_token->var->name->stringa,iindx,((PDX_STRING)ter_token->var->obj)->len);
			 return -1 ;
			}
			return ((PDX_STRING)ter_token->var->obj)->stringx[iindx] ;
	   }

  return -1 ;
}


DXLONG64 hdr_inter_return_byte_value_from_indx(PHDR_INTERPRETER inter , PHDR_TERMINAL_TOKEN ter_token)
{
	 /*
	  The function gets the variable and the index in the ter_token
	  and return the byte in the position as integer.
	  In error returns -1 
	 */
  
  bool error	= false ;
  PHDR_VAR indx = hdr_inter_resolve_expr(inter,ter_token->last_token->expression,&error) ;
  if(error == true)
  {
	printf("The supplied index for the '%s' variable was not resolved in a valid value.\n",ter_token->var->name->stringa);
	return exec_state_error ;
  }

  if((indx->type != hvt_integer)&&(indx->type != hvt_float))
  {
	printf("The supplied index for the '%s' variable MUST be of 'Numeric' type.\n",ter_token->var->name->stringa);
	return -1 ;
  }

  /*get the number*/
  DXLONG64 iindx = 0 ;
  if(indx->type == hvt_integer) iindx = indx->integer ;
  else
	  iindx = (DXLONG64)floor(indx->real) ; 

   if( (iindx < 0)||(iindx >= ((PHDR_BYTES)ter_token->var->obj)->length) )
   {
		printf("The supplied index for the '%s' variable is out of range. Index : %d Range : 0 - %d.\n",
			ter_token->var->name->stringa,iindx,((PHDR_BYTES)ter_token->var->obj)->length);
		return -1 ;
   }
	return ((PHDR_BYTES)ter_token->var->obj)->bytes[iindx] ;
	   

  return -1 ;
}
















