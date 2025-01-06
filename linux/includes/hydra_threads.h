/*
 Hydra+ async implementation and wrapper for the the third party library cthreads.

 Nikos Mourgis deus-ex.gr 2024

 Live Long and Prosper
*/


typedef struct hdr_async_param
{
	PHDR_INTERPRETER      inter  ;
	PHDR_THREAD           thread ;
	PHDR_THREAD*		  pos	 ; /*the position of the thread in the list*/
	PHDR_CUSTOM_FUNCTION  func	 ;
} *PHDR_ASYNC_PARAM;

void * hdr_threads_execute_inter(void *param)
{
	 PHDR_INTERPRETER		interpreter = ((PHDR_ASYNC_PARAM)param)->inter  ;
	 PHDR_THREAD			thread	    = ((PHDR_ASYNC_PARAM)param)->thread ;
	 PHDR_CUSTOM_FUNCTION	func		= ((PHDR_ASYNC_PARAM)param)->func   ;
	 PHDR_THREAD			*thr_pos    = ((PHDR_ASYNC_PARAM)param)->pos    ;
	 
	 /*remember we do not return any value from the async function except the message*/
	 thread->running = true ;
	 if (hdr_inter_execute_instructions_thread(interpreter) != exec_state_ok) 
	 {
	  thread->stop_with_error = true ;
	 }
	 thread->running = false ;
	 /*we will free everything that the main script has not access */
	 hdr_custom_functions_list_free(interpreter->funcs) ;
	 hdr_interpreter_free(interpreter,true) ;
	 hdr_custom_function_free(func);
	 free(thread->cargs)     ;
	 free(thread->cthread)   ;
	 hdr_thread_free(thread) ;
	 *thr_pos = NULL		 ;
	 free(param)			 ;
	 return NULL ;
}

/*
  the async command tries to copy the function from the interpreter list of functions 
  BUT tthe function block is prepared for the pass of the parameters (NULL variable member in the node), This make the block 
  unsuitable for regular copying and we will create a specific set of this function to use here.
*/

void hdr_async_func_var_copy(PHDR_VAR_LIST dest , PHDR_VAR_LIST source)
{
	/* The dest and source HAS to be created. 
	The variables from the source will be copied and inserted in the dest.
	*/

	for(int i = 0; i < source->list->length;i++)
	{
		/*for all the buckets*/
		PDX_LIST    sbucket = source->list->buckets[i] ;
		PDX_LIST    dbucket = dest->list->buckets[i]   ;
		PDXL_NODE   snode   = sbucket->start			 ;
		/*copy the backet contents*/
		while(snode != NULL)
		{
		PHDR_VAR svar = (PHDR_VAR)snode->object->obj ;
		if(svar != NULL)
		{
		 PHDR_VAR nvar = hdr_var_clone(svar,svar->name->stringa, dest->block) ;
	 	 hdr_var_list_add(dest,nvar) ;
		}
		else
		{
		  /*The node is reserved for a parameter , we need to create the node an set a variable object as the async needs refernces to objects not to variables*/
			PDXL_OBJECT obj = dxl_object_create() ;
			/*we will set this as a reference to the name of the original node as the node is persistend via the program lifecycle*/
			obj->key = dx_string_createU(NULL,snode->object->key->stringa); /*copy this because when the interpreter free it self it will freed all its structures*/
			obj->obj = hdr_var_create(NULL,"",hvf_block,NULL) ; /*this will be shallow copied when the parameters are resolved*/
			/*
			 we drop the flag HDR_FUNCTION_PARAM_ABS_REF of the parameters because the async function creates instances of the params
			 with object references
			*/

			/*add the object in the list*/
			dx_HashInsertObject(dest->list, obj);
		}

		snode = snode->right ; /*next variable*/
		}
	}
 

	return ;
}

PHDR_BLOCK hdr_async_func_block_copy(PHDR_BLOCK source , PHDR_BLOCK parent)
{
	if(source == NULL) return NULL ;
    /*make an identical copy of the block and returned it*/
	PHDR_BLOCK nblock = hdr_block_create(parent) ;
	/*copy the variables*/
	hdr_async_func_var_copy(nblock->variables , source->variables) ;
	/*copy the instructions*/
    hdr_instructions_list_copy(nblock->instructions,source->instructions,nblock);

    return nblock ;
}

PHDR_CUSTOM_FUNCTION hdr_async_custom_function_copy(PHDR_INTERPRETER inter,PHDR_CUSTOM_FUNCTION func)
{
  /*
  copy the function for the thread to execute. Note that the parameters of an async function are not absolute variable references
  */
  PHDR_CUSTOM_FUNCTION nfunc = hdr_custom_function_create(inter,func->name->stringa) ;
  if(nfunc == NULL) return NULL ;
  nfunc->code				 = hdr_async_func_block_copy(func->code,inter->code) ;/*init with the main code block*/
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
 We need to pass the parameters in the function block, the following
 function setups the parameters. The parameters in the async functions
 are not the same as the functions in the main thread.
 In the main thread the parameters are the actual variables of the current block (except if the variable is a temporay one).
 In the async functions the parameters makes a copy of the object of the variable 
 of the current block. This is done because the threads cann change the actual variable for their own purpose
 and this can lead to memory access errors. For example in this way you can easily pass to a thread a parameter
 and then .Release() this variable to be ready to be set to another value (in a loop for example)

*/

bool hdr_async_setup_params(PHDR_INTERPRETER caller_inter,PHDR_CUSTOM_FUNCTION func,
	PHDR_COMPLEX_TOKEN token,PDX_LIST params)
{
	/*returns false if fails , true in success*/
	/*
	   now , the parameters setup is tricky, we will get the name of the first parameter , and we will retrieve the 
	   first actual parameter. We will find the variable with the same name as the parameter in the block's variables
	   and we will set the pointer of the object of the variable to the local variable. For variables that are 
	   of numeric type we will do a copy.
	*/

	/* REMEMBER another part that needs special handling is that the parameters can be temporary created 
	   variables AND MUST be FREED before the thread terminates. 
	*/
	if(params == NULL) 
	{
	 
		printf("Fatal Error -> Line : %d INTERNAL ERROR ON ASYNC -> PARAM LIST. NULL OBJECT\n", caller_inter->current_instr->instr->line);
		return false ;
	}

	PDXL_NODE node = func->parameters->start  ;
	int indx = 0 ;
	while(node != NULL)
	{
	  PDX_STRING  pname    = node->object->key ;
	  PHDR_VAR    rparam   = hdr_inter_resolve_extract_var_from_expr(caller_inter, token->parameters, indx);
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
		   The ppointer has a key that is the actual name of the parameter(but for the threads is a reference!) and an empty obj that 
		   for the threads must be set to a variable that will be shallow copied
		  */
		  PHDR_VAR pvar = (PHDR_VAR)ppointer->obj ;
		  hdr_var_copy_shallow(rparam,pvar) ;
		  /*THIS VARIABLE MUST NOT RELEASE ITS OBJECT. IS A REFERENCE!*/
		  pvar->is_ref = true ;
	  }
	  indx++ ;
	  node = node->right ;
	}

	return true ;
}


PDX_LIST hdr_async_release_params(PDX_LIST params)
{
	/**************clean up******************************/
	return hdr_user_func_h_release_params(params) ;	
}



enum exec_state hdr_async_run_function(PHDR_INTERPRETER inter , PHDR_CUSTOM_FUNCTION func ,PHDR_COMPLEX_TOKEN token, PDX_STRING thread_id) 
{
	PHDR_CUSTOM_FUNCTION async_func = hdr_async_custom_function_copy(inter,func) ; 
	/*setup the parameters for the function*/
	PDX_LIST async_params = dx_list_create_list() ; 
	if (hdr_async_setup_params(inter,async_func,token,async_params) == false) 
	{
	  printf("Error creating setting up the parameters for the async function : %s\n",token->ID->stringa) ;
	  hdr_custom_function_free(async_func) ;
	  dx_list_delete_list(async_params)    ;
	  return exec_state_error ;
	}

	/*create a new interpreter*/
	PHDR_INTERPRETER async_inter = hdr_interpreter_create(NULL,true) ;
	if(async_inter == NULL) 
	{
      printf("Error creating the interpreter for the async.\n") ;
	  hdr_custom_function_free(async_func) ;
	  return exec_state_error ;
	}

	async_func->interpreter	    = async_inter ;
	async_inter->code		    = async_func->code ;
	async_inter->current_block  = async_inter->code ; 

	async_inter->current_instr->instr = hdr_interpreter_ret_instr(async_inter->code->instructions->start);
	async_inter->last_instr			  = async_inter->current_instr->instr;
	async_inter->current_instr->node  = async_inter->code->instructions->start;
	async_inter->current_block		  = async_inter->code		  ;
	async_inter->loader				  = inter->loader			  ;

	/*
	 this interpreter inherits the  objects classes and thread list
	 and COPY the functions templates
	*/
	
	if(hdr_interpreter_set_functions_from_loader(async_inter, inter->loader) == false)
	{
	  printf("Error creating the new thread interpeter functions list for the async.\n") ;
	  hdr_custom_function_free(async_func) ;
	  hdr_interpreter_free(async_inter,true)     ;
	  return exec_state_error ;
	}

	hdr_object_class_list_free(async_inter->object_classes)		  ;       
	async_inter->object_classes		  = inter->object_classes	  ;
	async_inter->threads			  = inter->threads			  ;
	
	/*create a new thread entry*/
	PHDR_THREAD nthread = hdr_thread_create(thread_id) ;
	if(nthread == NULL)
	{
	  printf("Error creating the new thread object for the async.\n") ;
	  hdr_custom_function_free(async_func) ;
	  hdr_interpreter_free(async_inter,true)     ;
	  return exec_state_error ;
	}
	
	struct cthreads_thread *cthread = (struct cthreads_thread*)malloc(sizeof(struct cthreads_thread)) ;
	nthread->cthread = cthread ;
	struct cthreads_args *args	    = (struct cthreads_args*)malloc(sizeof(struct cthreads_args)) ;
	
	PHDR_ASYNC_PARAM param = (PHDR_ASYNC_PARAM)malloc(sizeof(struct hdr_async_param)) ;
	param->func          = async_func  ;
	param->inter         = async_inter ;
	param->thread        = nthread     ;
	param->thread->cargs = args		   ;
	
	param->pos = hdr_thread_list_add_thread(inter->threads,nthread);
	if(param->pos == NULL)
	{
	  printf("All the slots for the threads are filled. Something is not right as the Hydra+ allows %d threads simultaneously.\n",HYDRA_MAXIMUM_THREADS) ;
	  hdr_custom_function_free(async_func) ;
	  hdr_interpreter_free(async_inter,true)     ;
	  free(cthread);
	  free(param)  ;
	  return exec_state_error ;
	}
	
	async_inter->thread        = nthread ;
	async_inter->async_params  = async_params ;
	int error_code = cthreads_thread_create(cthread, NULL,hdr_threads_execute_inter, param, args) ;
	if (error_code!=0)
	{
	  printf("Error creating the cthread for the async. Error code : %d\n",error_code) ;
	  hdr_custom_function_free(async_func) ;
	  hdr_interpreter_free(async_inter,true)     ;
	  free(cthread);
	  free(param)  ;
	  return exec_state_error ;
	}
	
  return exec_state_ok ;
}


bool hdr_terminate_thread(PHDR_INTERPRETER inter , PDX_STRING thread_id)
{
	/*finds the first thread with the given id and terminate it*/
	if(thread_id == NULL) return true;

	/*
	  check if the terminate has as a parameter the main thread and terminate the program 
	*/
	PDX_STRING main = dx_string_createU(NULL,"main") ;
	if(dx_string_native_compare(main,thread_id) == dx_equal)
	{
	  /*terminate Hydra+*/
	  exit(0);
	} 
	dx_string_free(main);

	PHDR_THREAD thread = hdr_thread_list_return_thread(inter->threads,thread_id) ;
	if(thread == NULL)
	{
	 if(inter->warnings == true) printf("The thread list does not contain a thread with the given id : %s\n",thread_id->stringa);
	 return true;
	}

	/*terminate thread*/
	if(thread->cthread == NULL)
	{
	 if(inter->warnings == true) printf("The thread has been terminated or deallocated previously. Id : %s\n",thread_id->stringa);
	 return true;
	}

	/*set the flag for the interpreter of the thread to stop*/
	thread->terminate = true ;

	return true ;
}


bool hdr_threads_term_and_wait(PHDR_INTERPRETER inter , int seconds) 
{
   for(int i = 0; i < HYDRA_MAXIMUM_THREADS;i++)
   {
      if(inter->threads->threads[i]!=NULL)
	  {
        if (inter->threads->threads[i]->terminate == false)
	    inter->threads->threads[i]->terminate = true ;
	  }
   
   }

   /*wait seconds*/
   DXLONG64 time_point = dxGetTickCount() ;

   while(true)
   {
     if(((dxGetTickCount()-time_point)/1000) > seconds) break ;
   }

   /*
     if the threads do not have terminate then its possible to have run time errors as the structs that the threads will be released in the 
     main thread. So we will just set it to NULL. May some memory leaks be created but this function logically is 
	 called when hydra terminates so whateveeeeeeer.
   */

   for(int i = 0; i < HYDRA_MAXIMUM_THREADS;i++)
   {
      if(inter->threads->threads[i]!=NULL)
	  {
        if(inter->threads->threads[i] != NULL)
	    inter->threads->threads[i] = NULL ;
	  }
   
   }

   return true ;

}
















