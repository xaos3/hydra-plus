/***************** LIST DOMAIN ***********************************/
bool hdr_domSimpleListCreate(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token, PHDR_VAR *result)
{
	 /*
	  Create a simple list. 
	 */
	 /*check the parameters*/
	 if (hdr_inter_check_param_count_error(inter, token, 0) == false) return true;

	 /*create a simple list*/
	 PDX_LIST list = dx_list_create_list() ;
	 if(list == NULL)
	 {
		 printf("MEMORY ERROR. THE PDX_LIST RETURNED NULL\n") ;
		 return true ;
	 }
	 *result		= hdr_var_create(list, "", hvf_temporary_ref, NULL); 
	 if(*result == NULL)
	 {
		 printf("MEMORY ERROR. THE PDX_VAR FOR THE RESULT RETURNED NULL\n") ;
		 return true ;
	 }
	 (*result)->type = hvt_list;

	 return false; 
}


bool hdr_domSimpleListAtPos(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token, PHDR_VAR for_var, PHDR_VAR *result)
{
   /*returns the position index of the named index or -1 if the named index does not exists*/
   PHDR_SYS_FUNC_PARAMS params = hdr_sys_func_init_params(inter,token->parameters,1) ;
   if(params == NULL)
   {
    printf("The system function List.AtPos($index : String) failed.\n");
    return true ;
   }

   bool type_error = false ;
   PDX_STRING str = hdr_inter_ret_string(params->params[0],&type_error) ; 
   if(type_error == true)
   {
	 printf("The parameter must be a simple string\n");
     goto fail ;
   }
   
   /*check the index*/
   PDX_LIST list  = (PDX_LIST)for_var->obj ;

   *result = hdr_var_create(NULL, "", hvf_temporary_ref, NULL) ;
   (*result)->type    = hvt_integer ; 
   (*result)->integer = - 1 ;
   DXLONG64 indx = 0 ;
   PDXL_NODE node = list->start ;
   while(node != NULL)
   {
       PDXL_OBJECT obj = node->object ;
	   if(obj->key!= NULL)
	   {
	    if(dx_string_native_compare(obj->key,str) == dx_equal ) 
		{
			(*result)->integer = indx ; 
			goto success ;
		}
	   
	   }
	   indx++ ;
	   node = node->right ;
   }
   
   
   success:
   hdr_sys_func_free_params(params) ;
   return false ;

   fail : 
   printf("The system function List.AtPos($index : Integer) failed.\n");
   hdr_sys_func_free_params(params) ;
   return true ;
}


bool hdr_domSimpleListNamedIndex(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token, PHDR_VAR for_var, PHDR_VAR *result)
{
   /*The function returns the named index of the position index or empty string if the position index has not any named index bound*/
   PHDR_SYS_FUNC_PARAMS params = hdr_sys_func_init_params(inter,token->parameters,1) ;
   if(params == NULL)
   {
    printf("The system function List.NamedIndex($index : Integer):String failed.\n");
    return true ;
   }

   bool type_error = false ;
   DXLONG64 indx = hdr_inter_ret_integer(params->params[0],&type_error) ; 
   if(type_error == true)
   {
	 printf("The parameter must be of a numeric type\n");
     goto fail ;
   }
   /*check the index*/
   PDX_LIST list  = (PDX_LIST)for_var->obj ;

   *result = hdr_var_create(NULL, "", hvf_temporary_ref, NULL) ;
   (*result)->type    = hvt_simple_string          ; 
   PDX_STRING ns      = dx_string_createU(NULL,"") ;
   hdr_var_set_obj(*result,ns)					   ;
   PDXL_NODE node = dx_list_go_to_pos(list,indx)   ;

   if(node == NULL)
   {
    printf("The index that was supplied is not in the valid range of [0 : %d] ",list->count-1);
    goto fail ;
   }
   
   PDXL_OBJECT obj = node->object ;
   if(obj->key != NULL) 
   {
	   PDX_STRING tstr = (PDX_STRING)(*result)->obj ; 
	   tstr = dx_string_createU(((PDX_STRING)(*result)->obj),obj->key->stringa) ;
	   hdr_var_set_obj(*result,tstr) ;
   }

   success:
    hdr_sys_func_free_params(params) ;
    return false ;

   fail : 
    printf("The system function List.NamedIndex($index : Integer):String failed.\n");
    hdr_sys_func_free_params(params) ;
    return true ;

}

bool hdr_domSimpleListSetNamedIndex(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token, PHDR_VAR for_var, PHDR_VAR *result)
{

   /*The function (re)sets the named index of the position index*/
   PHDR_SYS_FUNC_PARAMS params = hdr_sys_func_init_params(inter,token->parameters,2) ;
   if(params == NULL)
   {
    printf("The system function List.SetNamedIndex($index : Integer,$name : String) failed.\n");
    return true ;
   }

   bool type_error = false ;
   DXLONG64 indx = hdr_inter_ret_integer(params->params[0],&type_error) ; 
   if(type_error == true)
   {
	 printf("The first parameter must be of a numeric type\n");
     goto fail ;
   }

   type_error = false ;
   PDX_STRING name = hdr_inter_ret_string(params->params[1],&type_error) ; 
   if(type_error == true)
   {
	 printf("The second parameter must be of a Simple String type\n");
     goto fail ;
   }


   /*check the index*/
   PDX_LIST list  = (PDX_LIST)for_var->obj ;
   PDXL_NODE node = dx_list_go_to_pos(list,indx)   ;

   if(node == NULL)
   {
    printf("The index that was supplied is not in the valid range of [0 : %d] ",list->count-1);
    goto fail ;
   }
   
   PDXL_OBJECT obj = node->object ;
   /*set the key*/
   obj->key = dx_string_createU(obj->key,name->stringa);

   success:
    hdr_sys_func_free_params(params) ;
    return false ;

   fail : 
    printf("The system function List.SetNamedIndex($index : Integer,$name:String) failed.\n");
    hdr_sys_func_free_params(params) ;
    return true ;

}


bool hdr_domSimpleListChangeNamedIndex(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token, PHDR_VAR for_var, PHDR_VAR *result)
{

   /*The function (re)sets the named index of the position index*/
   PHDR_SYS_FUNC_PARAMS params = hdr_sys_func_init_params(inter,token->parameters,2) ;
   if(params == NULL)
   {
    printf("The system function List.ChangeNamedIndex($index : String ,$name : String):Boolean failed.\n");
    return true ;
   }

   bool type_error = false ;
   PDX_STRING indx = hdr_inter_ret_string(params->params[0],&type_error) ; 
   if(type_error == true)
   {
	 printf("The first parameter must be of a Simple String type\n");
     goto fail ;
   }

   type_error = false ;
   PDX_STRING name = hdr_inter_ret_string(params->params[1],&type_error) ; 
   if(type_error == true)
   {
	 printf("The second parameter must be of a Simple String type\n");
     goto fail ;
   }


   /*check the index*/
   PDX_LIST list  = (PDX_LIST)for_var->obj ;
   *result = hdr_var_create(NULL, "", hvf_temporary_ref, NULL) ;
   (*result)->type    = hvt_bool ; 
   (*result)->integer = 0 ;

   PDXL_NODE node = list->start ;
   while(node != NULL)
   {
       PDXL_OBJECT obj = node->object ;
	   if(obj->key!= NULL)
	   {
	    if(dx_string_native_compare(obj->key,indx) == dx_equal ) 
		{
			(*result)->integer = 1 ; 
		    /*set the key*/
            obj->key = dx_string_createU(obj->key,name->stringa);
			goto success ;
		}
	   
	   }
	   node = node->right ;
   }


   success:
    hdr_sys_func_free_params(params) ;
    return false ;

   fail : 
    printf("The system function List.ChangeNamedIndex($index : String ,$name : String):Boolean failed.\n");
    hdr_sys_func_free_params(params) ;
    return true ;

}

bool hdr_domSimpleListCount(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token, PHDR_VAR for_var, PHDR_VAR *result)
{
   /*returns the elements count of the list*/
   PHDR_SYS_FUNC_PARAMS params = hdr_sys_func_init_params(inter,token->parameters,0) ;
   if(params == NULL)
   {
    printf("The system function List.Count() failed.\n");
    return true ;
   }
   
   /*check the index*/
   PDX_LIST list  = (PDX_LIST)for_var->obj ;

   *result = hdr_var_create(NULL, "", hvf_temporary_ref, NULL) ;
   (*result)->type    = hvt_integer ; 
   (*result)->integer = list->count ;

   success:
    hdr_sys_func_free_params(params) ;
    return false ;

   fail : 
    printf("The system function List.Count() failed.\n");
    hdr_sys_func_free_params(params) ;
    return true ;

}

bool hdr_domSimpleListExists(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token, PHDR_VAR for_var, PHDR_VAR *result)
{
   /*checks if the named index exists in the list*/
   PHDR_SYS_FUNC_PARAMS params = hdr_sys_func_init_params(inter,token->parameters,1) ;
   if(params == NULL)
   {
    printf("The system function List.Exists($name : String) failed.\n");
    return true ;
   }
   
   bool type_error = false ;
   PDX_STRING str = hdr_inter_ret_string(params->params[0],&type_error) ; 
   if(type_error == true)
   {
	 printf("The parameter must be a simple string\n");
     goto fail ;
   }
   
   /*check the index*/
   PDX_LIST list  = (PDX_LIST)for_var->obj ;

   *result = hdr_var_create(NULL, "", hvf_temporary_ref, NULL) ;
   (*result)->type    = hvt_bool ; 
   (*result)->integer = 0 ;

   PDXL_NODE node = list->start ;
   while(node != NULL)
   {
       PDXL_OBJECT obj = node->object ;
	   if(obj->key!= NULL)
	   {
	    if(dx_string_native_compare(obj->key,str) == dx_equal ) 
		{
			(*result)->integer = 1 ; 
			goto success ;
		}
	   
	   }
	   node = node->right ;
   }
   

   success:
    hdr_sys_func_free_params(params) ;
    return false ;

   fail : 
    printf("The system function List.Exists($string : String) failed.\n");
    hdr_sys_func_free_params(params) ;
    return true ;
}


bool hdr_domSimpleListDelete(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token, PHDR_VAR for_var, PHDR_VAR *result)
{
	/*
	 The function removes and deletes an element of the list , the parameter can be a string index or a numeric index
	*/

   PHDR_SYS_FUNC_PARAMS params = hdr_sys_func_init_params(inter,token->parameters,1) ;
   if(params == NULL)
   {
    printf("The system function List.Delete($indx : Integer | String):Boolean failed.\n");
    return true ;
   }
   
   PDX_LIST list  = (PDX_LIST)for_var->obj ;

   /*returns true if the index exists and is removed or false */
   *result = hdr_var_create(NULL, "", hvf_temporary_ref, NULL) ;
   (*result)->type    = hvt_bool ; 
   (*result)->integer = 0 ;

   if((params->params[0]->type == hvt_integer)||(params->params[0]->type == hvt_float))
   {
      /*remove the element in the numeric index*/
	   bool type_error ;
	   PDXL_NODE node = dx_list_go_to_pos(list,  hdr_inter_ret_integer(params->params[0],&type_error)) ;
	   if(node == NULL)
	   {
		printf("The index that was supplied is not in the valid range of [0 : %d] ",list->count-1);
		goto fail ;
	   }
   
	   PDXL_OBJECT obj = node->object ;
	   hdr_var_free((PHDR_VAR)obj->obj) ;
	   free(obj) ;
	   dx_list_delete_node(node) ;
	   (*result)->integer = 1 ;
	   goto success;
   }
   else
	   if((params->params[0]->type == hvt_simple_string) || (params->params[0]->type == hvt_simple_string_bcktck))
	   {
	     bool type_error = false ;
		 PDX_STRING str = hdr_inter_ret_string(params->params[0],&type_error) ; 
		 
		  PDXL_NODE node = list->start ;
		  while(node != NULL)
		  {
			   PDXL_OBJECT obj = node->object ;
			   if(obj->key!= NULL)
			   {
				if(dx_string_native_compare(obj->key,str) == dx_equal ) 
				{
				   /*remove it*/
				   hdr_var_free((PHDR_VAR)obj->obj) ;
				   dx_string_free(obj->key) ;
				   free(obj) ;
				   dx_list_delete_node(node) ;

					(*result)->integer = 1 ; 
					goto success ;
				}
	   
			   }
			   node = node->right ;
		  }

		 
		goto success ;
	   }
	   else
	   {
	     printf("The parameter must be a simple string as a named index or a numeric index\n");
		 goto fail ;
	   }

   success:
    hdr_sys_func_free_params(params) ;
    return false ;

   fail : 
    printf("The system function List.Delete($indx : Numeric | String):Boolean failed.\n");
    hdr_sys_func_free_params(params) ;
    return true ;
}

bool hdr_domSimpleListDeleteAll(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token, PHDR_VAR for_var, PHDR_VAR *result)
{
	/*
	 The function removes and deletes all elements of the list
	*/

   PHDR_SYS_FUNC_PARAMS params = hdr_sys_func_init_params(inter,token->parameters,0) ;
   if(params == NULL)
   {
    printf("The system function List.DeleteAll() failed.\n");
    return true ;
   }
   
    PDX_LIST list  = (PDX_LIST)for_var->obj ;
	 		 
	PDXL_NODE node = list->start ;
	while(node != NULL)
	{
		PDXL_OBJECT obj = node->object ;
		/*remove it*/
		hdr_var_free((PHDR_VAR)obj->obj) ;
		dx_string_free(obj->key) ;
		free(obj) ;	   
		node = node->right ;
	}

	dx_list_empty_list(list) ;
    hdr_sys_func_free_params(params) ;
    return false ;
}


bool hdr_domSimpleListRemove(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token, PHDR_VAR for_var, PHDR_VAR *result)
{
	/*
	 The function removes the variable BUT do not delete the actual object of the variable except if its a simple string.
	 The parameter can be a string index or a numeric index
	*/

   PHDR_SYS_FUNC_PARAMS params = hdr_sys_func_init_params(inter,token->parameters,1) ;
   if(params == NULL)
   {
    printf("The system function List.Remove($indx : Integer | String) failed.\n");
    return true ;
   }
   
   PDX_LIST list  = (PDX_LIST)for_var->obj ;

   /*returns true if the index exists and is removed or false */
   *result = hdr_var_create(NULL, "", hvf_temporary_ref, NULL) ;
   (*result)->type    = hvt_bool ; 
   (*result)->integer = 0 ;

   if((params->params[0]->type == hvt_integer)||(params->params[0]->type == hvt_float))
   {
      /*remove the element in the numeric index*/
	   bool type_error ;
	   PDXL_NODE node = dx_list_go_to_pos(list,  hdr_inter_ret_integer(params->params[0],&type_error)) ;
	   if(node == NULL)
	   {
		printf("The index that was supplied is not in the valid range of [0 : %d] ",list->count-1);
		goto fail ;
	   }
   
	   PDXL_OBJECT obj = node->object ;
	   PHDR_VAR var = (PHDR_VAR)obj->obj ;
	   if((var->type == hvt_simple_string)||(var->type == hvt_simple_string_bcktck))
        var->obj = dx_string_free((PDX_STRING)var->obj) ;

	   /*if not a string we will not delete the object*/
	   hdr_var_release_obj(var) ;

	   hdr_var_free((PHDR_VAR)obj->obj) ;
	   free(obj) ;
	   dx_list_delete_node(node) ;
	   (*result)->integer = 1 ;
	   goto success;
   }
   else
	   if((params->params[0]->type == hvt_simple_string) || (params->params[0]->type == hvt_simple_string_bcktck))
	   {
	     bool type_error = false ;
		 PDX_STRING str = hdr_inter_ret_string(params->params[0],&type_error) ; 
		 
		  PDXL_NODE node = list->start ;
		  while(node != NULL)
		  {
			   PDXL_OBJECT obj = node->object ;
			   if(obj->key!= NULL)
			   {
				if(dx_string_native_compare(obj->key,str) == dx_equal ) 
				{
				   /*remove it*/
				   PHDR_VAR var = (PHDR_VAR)obj->obj ;
				   if((var->type == hvt_simple_string)||(var->type == hvt_simple_string_bcktck))
					   var->obj = dx_string_free((PDX_STRING)var->obj) ;
				   
					hdr_var_release_obj(var) ; /*the object will not be destroyed*/

				    hdr_var_free((PHDR_VAR)obj->obj) ;
				    free(obj) ;
				    dx_list_delete_node(node) ;
				    (*result)->integer = 1 ;

					goto success ;
				}
	   
			   }
			   node = node->right ;
		  }

		 
		goto success ;
	   }
	   else
	   {
	     printf("The parameter must be a simple string as a named index or a numeric index\n");
		 goto fail ;
	   }


   success:
    hdr_sys_func_free_params(params) ;
    return false ;

   fail : 
    printf("The system function List.Remove($index : Numeric | String) failed.\n");
    hdr_sys_func_free_params(params) ;
    return true ;
}


bool hdr_domSimpleListRemoveAll(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token, PHDR_VAR for_var, PHDR_VAR *result)
{
	/*
	 The function removes  all elements of the list , the parameter can be a string index or a numeric index
	*/

   PHDR_SYS_FUNC_PARAMS params = hdr_sys_func_init_params(inter,token->parameters,0) ;
   if(params == NULL)
   {
    printf("The system function List.RemoveAll() failed.\n");
    return true ;
   }
   
    PDX_LIST list  = (PDX_LIST)for_var->obj ;
	 		 
	PDXL_NODE node = list->start ;
	while(node != NULL)
	{
		PDXL_OBJECT obj = node->object ;
		/*remove it*/
		 PHDR_VAR var = (PHDR_VAR)obj->obj ;
		if((var->type == hvt_simple_string)||(var->type == hvt_simple_string_bcktck))
			var->obj = dx_string_free((PDX_STRING)var->obj) ;
		
			hdr_var_release_obj(var) ; /*the object will not be destroyed*/

		hdr_var_free((PHDR_VAR)obj->obj) ;
		free(obj) ;
	   
		node = node->right ;
	}

	dx_list_empty_list(list);

    hdr_sys_func_free_params(params) ;
    return false ;
}



bool hdr_domSimpleListReleaseMem(PHDR_VAR for_var)
{
   if(for_var->obj == NULL) return false ;
   /*the simple list is a collection of objects*/
   hdr_simple_list_free((PDX_LIST)for_var->obj) ;
   hdr_var_release_obj(for_var) ;
   for_var->type = hvt_undefined ;

   return false ;
}


bool hdr_domSimpleListInsert(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token, PHDR_VAR for_var)
{
   /*returns the position index of the named index or -1 if the named index does not exists*/
   PHDR_SYS_FUNC_PARAMS params = hdr_sys_func_init_params(inter,token->parameters,1) ;
   if(params == NULL)
   {
    printf("The system function List.Insert($index : Integer) failed.\n");
    return true ;
   }

   bool type_error = false ;
   DXLONG64 indx = hdr_inter_ret_integer(params->params[0],&type_error) ; 
   if(type_error == true)
   {
	 printf("The parameter must be an Integer\n");
     goto fail ;
   }
   
   /*get the node to the index and add a new variable*/
   PDX_LIST list  = (PDX_LIST)for_var->obj ;
   PDXL_NODE node = dx_list_go_to_pos(list,indx) ;
   PHDR_VAR var = hdr_var_create(NULL,"",hvf_dynamic,NULL) ; /*the variable is a dynamic variable */
   if(var == NULL)
   {
	  printf("MEMORY ERROR. THE PHDR_VAR IS NULL.\n");
	  goto fail ;
   }
   PDXL_NODE    nnode = (PDXL_NODE)malloc(sizeof(struct dxl_node)) ;
   PDXL_OBJECT  obj   = dxl_object_create() ;
   obj->obj			  = var					;
   if(nnode == NULL)
   {
      hdr_var_free(var) ;
	  printf("MEMORY ERROR. THE PHDR_VAR IS NULL.\n");
	  goto fail ;
   }

   nnode->object = obj ;
   dx_list_insert_node(list,node,nnode) ;
   
   success:
   hdr_sys_func_free_params(params) ;
   return false ;

   fail : 
   printf("The system function List.AtPos($index : Integer) failed.\n");
   hdr_sys_func_free_params(params) ;
   return true ;
}


/********************* STRINGLIST ******************************/

bool hdr_domStringListCreate(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token, PHDR_VAR *result)
{
	 /*
	  Create a stringlist. 
	 */
	 /*check the parameters*/
	 if (hdr_inter_check_param_count_error(inter, token, 0) == false) return true;

	 /*create a simple list*/
	 PDX_STRINGLIST list = dx_stringlist_create() ;
	 if(list == NULL)
	 {
		 printf("MEMORY ERROR. THE PDX_STRINGLIST RETURNED NULL\n") ;
		 return true ;
	 }
	 *result = hdr_var_create(list, "", hvf_temporary_ref, NULL); 
	 if(*result == NULL)
	 {
		 printf("MEMORY ERROR. THE PDX_VAR FOR THE RESULT RETURNED NULL\n") ;
		 return true ;
	 }
	 (*result)->type = hvt_string_list ;

	 return false; 
}

bool hdr_domStringListCount(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token, PHDR_VAR for_var, PHDR_VAR *result)
{
   /*returns the strings elements count of the list*/
   PHDR_SYS_FUNC_PARAMS params = hdr_sys_func_init_params(inter,token->parameters,0) ;
   if(params == NULL)
   {
    printf("The system function Stringlist.Count() failed.\n");
    return true ;
   }
   
   /*check the index*/
   PDX_STRINGLIST list  = (PDX_STRINGLIST)for_var->obj ;

   *result = hdr_var_create(NULL, "", hvf_temporary_ref, NULL) ;
   (*result)->type    = hvt_integer ; 
   (*result)->integer = list->count ;

   success:
    hdr_sys_func_free_params(params) ;
    return false ;

   fail : 
    printf("The system function Stringlist.Count() failed.\n");
    hdr_sys_func_free_params(params) ;
    return true ;

}


bool hdr_domStringListDelete(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token, PHDR_VAR for_var, PHDR_VAR *result)
{
	/*
	 The function deletes an element of the list , the parameter must be a numeric index
	*/

   PHDR_SYS_FUNC_PARAMS params = hdr_sys_func_init_params(inter,token->parameters,1) ;
   if(params == NULL)
   {
    printf("The system function Stringlist.Delete($indx : Integer) failed.\n");
    return true ;
   }
   
   PDX_STRINGLIST list  = (PDX_STRINGLIST)for_var->obj ;

   /*returns true if the index exists and is removed or false */
   *result = hdr_var_create(NULL, "", hvf_temporary_ref, NULL) ;
   (*result)->type    = hvt_bool ; 
   (*result)->integer = 0 ;



   if(params->params[0]->type == hvt_integer)
   {
      /*remove the element in the numeric index*/
	   bool type_error ;	   
	   DXLONG64 indx = hdr_inter_ret_integer(params->params[0],&type_error) ;
	   PDXL_NODE node = dx_list_go_to_pos(list, indx) ;
	   if(node == NULL)
	   {
		printf("The index that was supplied is not in the valid range of [0 : %d] ",list->count-1);
		goto fail ;
	   }
   
	   dx_stringlist_remove_str(list,indx) ;
	   (*result)->integer = 1 ;
	   goto success;
   }
	else
	   {
	     printf("The parameter must be a numeric index\n");
		 goto fail ;
	   }

   success:
    hdr_sys_func_free_params(params) ;
    return false ;

   fail : 
    printf("The system function Stringlist.Delete($indx : Numeric) failed.\n");
    hdr_sys_func_free_params(params) ;
    return true ;
}

bool hdr_domStringListDeleteAll(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token, PHDR_VAR for_var, PHDR_VAR *result)
{
	/*
	 The function deletes all elements of the list 
	*/

   PHDR_SYS_FUNC_PARAMS params = hdr_sys_func_init_params(inter,token->parameters,0) ;
   if(params == NULL)
   {
    printf("The system function Stringlist.DeletaAll() failed.\n");
    return true ;
   }
   
    PDX_STRINGLIST list  = (PDX_STRINGLIST)for_var->obj ;	 		 
	dx_stringlist_clear(list) ;

    hdr_sys_func_free_params(params) ;
    return false ;
}

bool hdr_domStringListAdd(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token, PHDR_VAR for_var)
{
	/*
	 The function adds an element in the list , the parameter must be a simple string
	*/

   PHDR_SYS_FUNC_PARAMS params = hdr_sys_func_init_params(inter,token->parameters,1) ;
   if(params == NULL)
   {
    printf("The system function Stringlist.Add($string : String) failed.\n");
    return true ;
   }
   
   PDX_STRINGLIST list  = (PDX_STRINGLIST)for_var->obj ;

   if((params->params[0]->type == hvt_simple_string)||(params->params[0]->type == hvt_simple_string_bcktck))
   {
      /*add the string */
	   bool type_error ;	   
	   PDX_STRING str = hdr_inter_ret_string(params->params[0],&type_error) ;
   
	   dx_stringlist_add_raw_string(list,str->stringa) ;
	   goto success;
   }
	else
	   {
	     printf("The parameter must be a simple string\n");
		 goto fail ;
	   }

   success:
    hdr_sys_func_free_params(params) ;
    return false ;

   fail : 
    printf("The system function Stringlist.Add($string : String) failed.\n");
    hdr_sys_func_free_params(params) ;
    return true ;
}

bool hdr_domStringListLoadFromFile(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token, PHDR_VAR for_var)
{
	/*
	 The function clear the list and loads text from a file and stored per lines {cr lf} 
	*/

   PHDR_SYS_FUNC_PARAMS params = hdr_sys_func_init_params(inter,token->parameters,1) ;
   if(params == NULL)
   {
    printf("The system function Stringlist.LoadFromFile($fileName : String) failed.\n");
    return true ;
   }
   
   bool type_error = false ;
   PDX_STRING fname = hdr_inter_ret_string(params->params[0],&type_error) ; 
   if(type_error == true)
   {
	 printf("The first parameter must be a String variable.\n");
     goto fail ;
   }

   PDX_STRINGLIST list  = (PDX_STRINGLIST)for_var->obj ;
   /*load the file*/
   if(dxFileExists(fname) == false) 
   {
     printf("The file [%s] does not exist\n",fname->stringa);
	 goto fail ;
   }

   bool error = false ;
   PDX_STRING text = dx_LoadUTF8TextFromFile(fname,&error) ;
   if(error == true)
   {
     dx_string_free(text) ;
	 printf("The file [%s] was not opened succesfully.\n",fname->stringa);
	 goto fail ;
   }

   /*empty the list*/

   dx_stringlist_clear(list) ;
   
   DXLONG64 indx = 0 ;
   char * tochar = "\n" ;
   while(indx < text->len)
   {
     /*get the line*/
     char * line = utf8CopyToCh(text->stringa,&indx,tochar) ;
	 if(line == NULL) break ;
	 /*check if the line is windows ending (crlf)*/
	 if(line[strlen(line)-1] == '\r') line[strlen(line)-1] = 0 ; /*terminate the line effectivelly trim the \r*/
	 PDX_STRING str = dx_string_create_bU(line) ;
	 dx_stringlist_add_string(list,str) ;

	 indx++ ; /*go after the character*/
	 if(indx == -1) break ;
   }


   dx_string_free(text) ;

   success:
    hdr_sys_func_free_params(params) ;
    return false ;

   fail : 
    printf("The system function Stringlist.LoadFromFile($fileName : String) failed.\n");
    hdr_sys_func_free_params(params) ;
    return true ;
}

bool hdr_domStringListSaveToFile(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token, PHDR_VAR for_var)
{
	/*
	 The function saves the stringlists contents as a text file. Every line of the string list will emmit 
	 an crlf or lf character based on the unixStyle parameter

	*/

   PHDR_SYS_FUNC_PARAMS params = hdr_sys_func_init_params(inter,token->parameters,2) ;
   if(params == NULL)
   {
    printf("The system function Stringlist.SaveToFile($fileName : String,$unixStyle : Boolean) failed.\n");
    return true ;
   }
   
   bool type_error = false ;
   PDX_STRING fname = hdr_inter_ret_string(params->params[0],&type_error) ; 
   if(type_error == true)
   {
	 printf("The first parameter must be a String variable.\n");
     goto fail ;
   }

   type_error = false ;
   bool unix_style = hdr_inter_ret_bool(params->params[1],&type_error) ; 
   if(type_error == true)
   {
	 printf("The second parameter must be a Boolean variable.\n");
     goto fail ;
   }

   PDX_STRINGLIST list  = (PDX_STRINGLIST)for_var->obj ;

   /*open the file*/
   FILE * f = fopen(fname->stringa,"wb+");
   if(f == NULL)
   {
     printf("Error opening the file : %s.\n",fname->stringa);
	 goto fail ;
   }


   PDXL_NODE node = list->start ;
   while(node!=NULL)
   {
		PDXL_OBJECT obj = node->object ;
   
		DXLONG64 bcount =  fwrite(obj->key->stringa,obj->key->bcount,1,f) ;
		if(bcount < 1)
		{
		 printf("Error writing in the file : %s.\n",fname->stringa);
		 fclose(f);
		 goto fail ;
		}

		char terml[2] ;
		if(unix_style == true)
		{
		  terml[0] = 10 ;
		  fwrite(terml,1,1,f) ;
		}
		else
		{
		  terml[0] = 13 ;
		  terml[1] = 10 ;
		  fwrite(terml,1,1,f) ;
		}

    node = node->right ;
   }

   fclose(f);

   success:
    hdr_sys_func_free_params(params) ;
    return false ;

   fail : 
    printf("The system function Stringlist.SaveToFile($fileName : String,$unixStyle) failed.\n");
    hdr_sys_func_free_params(params) ;
    return true ;
}

bool hdr_domStringListToSimple(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token, PHDR_VAR for_var, PHDR_VAR *result)
{
	/*
	 The function returns all the elements of the list as a simple text.
	*/

   PHDR_SYS_FUNC_PARAMS params = hdr_sys_func_init_params(inter,token->parameters,0) ;
   if(params == NULL)
   {
    printf("The system function Stringlist.ToSimple() failed.\n");
    return true ;
   }
   
    PDX_STRINGLIST list  = (PDX_STRINGLIST)for_var->obj ;	 		 
    *result = hdr_var_create(NULL, "", hvf_temporary_ref, NULL) ;
    (*result)->type    = hvt_simple_string ; 
    PDX_STRING ns = dx_stringlist_raw_text(list) ;
    hdr_var_set_obj(*result,ns) ;
  
    hdr_sys_func_free_params(params) ;
    return false ;
}

bool hdr_domStringListToSimpleLn(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token, PHDR_VAR for_var, PHDR_VAR *result)
{
	/*
	 The function returns all the elements of the list as a simple text but every line will add a \n character (appropriate for unix or windows).
	*/

   PHDR_SYS_FUNC_PARAMS params = hdr_sys_func_init_params(inter,token->parameters,1) ;
   if(params == NULL)
   {
    printf("The system function Stringlist.ToSimpleLn($unixStyle : Boolean) failed.\n");
    return true ;
   }

   bool unix_flag = false ;
   if(params->params[0]->type == hvt_bool)
   {
	   if (params->params[0]->integer == 1) unix_flag = true ;
   }
   else
   {
		printf("The Stringlist.ToSimpleLn($unixStyle : Boolean) needs one boolean parameter.");
		return true ;
   }

   
    PDX_STRINGLIST list  = (PDX_STRINGLIST)for_var->obj ;	 		 
   *result = hdr_var_create(NULL, "", hvf_temporary_ref, NULL) ;
   (*result)->type    = hvt_simple_string ; 
   PDX_STRING ns      = dx_stringlist_text(list,unix_flag) ; 
   hdr_var_set_obj(*result,ns);

    hdr_sys_func_free_params(params) ;
    return false ;
}

bool hdr_domStringListClear(PHDR_VAR for_var)
{
   if(for_var->obj == NULL) return false ;
   dx_stringlist_clear((PDX_STRINGLIST)for_var->obj) ;
   return false ;
}


bool hdr_domStringListReleaseMem(PHDR_VAR for_var)
{
   if(for_var->obj == NULL) return false ;
   dx_stringlist_free((PDX_STRINGLIST)for_var->obj) ;
   hdr_var_release_obj(for_var);
   for_var->type = hvt_undefined ;

   return false ;
}


/********************** FAST LIST ********************************/


bool hdr_domFastListCreate(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token, PHDR_VAR *result)
{
	 /*Create a fast list ! */
	 /*check the parameters*/
   PHDR_SYS_FUNC_PARAMS params = hdr_sys_func_init_params(inter,token->parameters,1) ;
   if(params == NULL)
   {
    printf("The system function List.CreateFastList($buckets : Integer) failed.\n");
    return true ;
   }
   
   bool type_error = false ;
   DXLONG64 len = hdr_inter_ret_integer(params->params[0],&type_error) ; 
   if(type_error == true)
   {
	 printf("The parameter must be an integer.\n");
     goto fail ;
   }

     if(len <1) len = 1 ;
	 /*create a hash table*/
	 PDX_HASH_TABLE list = dx_HashCreateTable(len);
	 if(list == NULL)
	 {
		 printf("MEMORY ERROR. THE PDX_HASH_TABLE RETURNED NULL\n") ;
		 return true ;
	 }
	 *result		= hdr_var_create(list, "", hvf_temporary_ref, NULL); 
	 if(*result == NULL)
	 {
		 printf("MEMORY ERROR. THE PDX_VAR FOR THE RESULT RETURNED NULL\n") ;
		 return true ;
	 }
	 (*result)->type = hvt_fast_list ;

	 success:
    hdr_sys_func_free_params(params) ;
    return false ;

    fail : 
    printf("The system function List.CreateFastList($buckets : Integer) failed.\n");
    hdr_sys_func_free_params(params) ;
    return true ;
}


/************************************************/

void hdr_fast_list_free_var_obj(PDXL_OBJECT obj)
{
	PHDR_VAR var = obj->obj			;
	obj->obj = hdr_var_free(var)	;
	return;
}

PDX_HASH_TABLE hdr_fast_list_free(PDX_HASH_TABLE list)
{
	dx_HashDestroyTable(list, hdr_fast_list_free_var_obj);
	return NULL;
}

bool hdr_domFastListReleaseMem(PHDR_VAR for_var)
{
   if(for_var->obj == NULL) return false ;
   
   hdr_fast_list_free((PDX_HASH_TABLE)for_var->obj) ;
   hdr_var_release_obj(for_var) ;
   for_var->type = hvt_undefined ;
   return false ;	 
}

/***********************************************/

bool hdr_domFastListCount(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token, PHDR_VAR for_var, PHDR_VAR *result)
{
   /*returns the elements count of the list*/
   PHDR_SYS_FUNC_PARAMS params = hdr_sys_func_init_params(inter,token->parameters,0) ;
   if(params == NULL)
   {
    printf("The system function List.Count() failed.\n");
    return true ;
   }
   
   /*check the index*/
   PDX_HASH_TABLE list  = (PDX_HASH_TABLE)for_var->obj ;

   *result = hdr_var_create(NULL, "", hvf_temporary_ref, NULL) ;
   (*result)->type    = hvt_integer ; 
   (*result)->integer = list->count ;

   success:
    hdr_sys_func_free_params(params) ;
    return false ;

   fail : 
    printf("The system function List.Count() failed.\n");
    hdr_sys_func_free_params(params) ;
    return true ;
}

bool hdr_domFastListExists(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token,PHDR_VAR for_var, PHDR_VAR *result)
{
   /*checks if the named index exists in the list*/
   PHDR_SYS_FUNC_PARAMS params = hdr_sys_func_init_params(inter,token->parameters,1) ;
   if(params == NULL)
   {
    printf("The system function List.Exists($name : String):Boolean failed.\n");
    return true ;
   }
   
   bool type_error = false ;
   PDX_STRING str = hdr_inter_ret_string(params->params[0],&type_error) ; 
   if(type_error == true)
   {
	 printf("The parameter must be a simple string\n");
     goto fail ;
   }
   
   /*check the index*/
   PDX_HASH_TABLE list  = (PDX_HASH_TABLE)for_var->obj ;

   *result = hdr_var_create(NULL, "", hvf_temporary_ref, NULL) ;
   (*result)->type    = hvt_bool ; 
   (*result)->integer = 0 ;

   PDXL_OBJECT obj = dx_HashReturnItem(list,str,true) ;
   if (obj != NULL) (*result)->integer = 1 ;

   success:
    hdr_sys_func_free_params(params) ;
    return false ;

   fail : 
    printf("The system function List.Exists($string : String):Boolean failed.\n");
    hdr_sys_func_free_params(params) ;
    return true ;
}


bool hdr_domFastListDelete(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token, PHDR_VAR for_var, PHDR_VAR *result)
{
    /*
	 The function removes and deletes an element of the list
	*/

   PHDR_SYS_FUNC_PARAMS params = hdr_sys_func_init_params(inter,token->parameters,1) ;
   if(params == NULL)
   {
    printf("The system function List.Delete($indx : String):Boolean failed.\n");
    return true ;
   }
   
   bool type_error = false ;
   PDX_STRING str = hdr_inter_ret_string(params->params[0],&type_error) ; 
   if(type_error == true)
   {
	 printf("The parameter must be a simple string\n");
     goto fail ;
   }

   PDX_HASH_TABLE list  = (PDX_HASH_TABLE)for_var->obj ;

   /*returns true if the index exists and is removed or false */
   *result = hdr_var_create(NULL, "", hvf_temporary_ref, NULL) ;
   (*result)->type    = hvt_bool ; 
   (*result)->integer = 0 ;

	PDXL_OBJECT obj = dx_HashReturnItem(list,str,true) ;
	if(obj == NULL) goto success ;

	/*exists , remove it*/
	obj = dx_HashRemoveItem(list,str) ;

	hdr_var_free((PHDR_VAR)obj->obj) ;
	dx_string_free(obj->key) ;
	free(obj) ;

	(*result)->integer = 1 ; 

   success:
    hdr_sys_func_free_params(params) ;
    return false ;

   fail : 
    printf("The system function List.Delete($indx : String):Boolean failed.\n");
    hdr_sys_func_free_params(params) ;
    return true ;
}

bool hdr_domFastListDeleteAll(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token, PHDR_VAR for_var, PHDR_VAR *result)
{
	/*
	 The function removes and deletes all elements of the list 
	*/
   PHDR_SYS_FUNC_PARAMS params = hdr_sys_func_init_params(inter,token->parameters,0) ;
   if(params == NULL)
   {
    printf("The system function List.DeleteAll() failed.\n");
    return true ;
   }
   
    PDX_HASH_TABLE list  = (PDX_HASH_TABLE)for_var->obj ;
	 		 
	for(int i = 0; i < list->length;i++)
	{
	  PDX_LIST bucket = list->buckets[i] ;
	  
	  PDXL_NODE node = bucket->start ;
	  while(node != NULL)
	  {
		PDXL_OBJECT obj = node->object ;
		/*remove it*/
		hdr_var_free((PHDR_VAR)obj->obj) ;
		dx_string_free(obj->key) ;
		free(obj) ;	   
		node = node->right ;
	  } 
	  dx_list_empty_list(bucket) ;
	}
	
	list->count = 0 ; /*reset all was deleted*/

    hdr_sys_func_free_params(params) ;
    return false ;
}

bool hdr_domFastListRemove(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token, PHDR_VAR for_var, PHDR_VAR *result)
{
	/*
	 The function removes and deletes the variable BUT do not delete the actual object of the variable except if its a simple string.
	*/

   PHDR_SYS_FUNC_PARAMS params = hdr_sys_func_init_params(inter,token->parameters,1) ;
   if(params == NULL)
   {
    printf("The system function List.Remove($indx : String) failed.\n");
    return true ;
   }
   
   PDX_HASH_TABLE list  = (PDX_HASH_TABLE)for_var->obj ;

   /*returns true if the index exists and is removed or false */
   *result = hdr_var_create(NULL, "", hvf_temporary_ref, NULL) ;
   (*result)->type    = hvt_bool ; 
   (*result)->integer = 0 ;
 
   bool type_error = false ;
   PDX_STRING str = hdr_inter_ret_string(params->params[0],&type_error) ; 
   if(type_error == true)
   {
		printf("The parameter must be a simple string\n");
		goto fail ;
   }

   PDXL_OBJECT obj = dx_HashReturnItem(list,str,true) ;
   if(obj == NULL) goto success ;

	/*exists , remove it*/
	obj = dx_HashRemoveItem(list,str) ;

	PHDR_VAR var = (PHDR_VAR)obj->obj ;
	if((var->type != hvt_simple_string)&&(var->type != hvt_simple_string_bcktck))
	hdr_var_release_obj(var) ;/*only the simple string type is managed so its a copy*/

	hdr_var_free(var) ;
	dx_string_free(obj->key) ;
	free(obj) ;

	(*result)->integer = 1 ; 	

   success:
    hdr_sys_func_free_params(params) ;
    return false ;

   fail : 
    printf("The system function List.Remove($index : String) failed.\n");
    hdr_sys_func_free_params(params) ;
    return true ;
}

bool hdr_domFastListRemoveAll(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token, PHDR_VAR for_var, PHDR_VAR *result)
{
	
	/*
	  The function removes and deletes all the variables BUT do not deletes
	  the actual object of the variable except if its a simple string.
	*/
   PHDR_SYS_FUNC_PARAMS params = hdr_sys_func_init_params(inter,token->parameters,0) ;
   if(params == NULL)
   {
    printf("The system function List.RemoveAll() failed.\n");
    return true ;
   }
   
    PDX_HASH_TABLE list  = (PDX_HASH_TABLE)for_var->obj ;
	 		 
	for(int i = 0; i < list->length;i++)
	{
	  PDX_LIST bucket = list->buckets[i] ;
	  
	  PDXL_NODE node = bucket->start ;
	  while(node != NULL)
	  {
		PDXL_OBJECT obj = node->object ;
		/*remove it*/
		PHDR_VAR var = (PHDR_VAR)obj->obj;
		if((var->type != hvt_simple_string)&&(var->type != hvt_simple_string_bcktck))
		hdr_var_release_obj(var);

		hdr_var_free(var) ;
		dx_string_free(obj->key) ;
		free(obj) ;	   
		node = node->right ;
	  } 
	  dx_list_empty_list(bucket) ;
	}
	
	list->count = 0 ; /*reset all was deleted*/

    hdr_sys_func_free_params(params) ;
    return false ;

}


/********************** JSON SUPPORT *********************************/

bool hdr_domJsonListToString(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token, PHDR_VAR for_var, PHDR_VAR *result)
{
	/*
	 The function returns a json object that has been created by the .ToJson as a string or try
	 to Jsonfy the simple list
	*/

   PHDR_SYS_FUNC_PARAMS params = hdr_sys_func_init_params(inter,token->parameters,2) ;
   if(params == NULL)
   {
    printf("The system function List.ToJson($isJsonObj:Boolean, out-> $error:String):String failed.\n");
    return true ;
   }

   bool type_error = false ;

   bool is_json = hdr_inter_ret_bool(params->params[0],&type_error) ; 
   if(type_error == true)
   {
		printf("The parameter must be a bool\n");
		goto fail ;
   }

   PDX_STRING error = hdr_inter_ret_string(params->params[1],&type_error) ; 
   if(type_error == true)
   {
		printf("The parameter must be a simple string\n");
		goto fail ;
   }
   
    PDX_LIST root  = (PDX_LIST)for_var->obj ;
	 
	PDX_STRING str =  NULL ;
	
	if(is_json == true) str = hdrJSONToString(root,error) ;
	else
		str = hdrArrayToString(root,error) ;
	
	if(str == NULL) str = dx_string_createU(NULL,"")    ;

	*result = hdr_var_create(str,"",hvf_temporary_ref,NULL) ;
	(*result)->type = hvt_simple_string						;

    hdr_sys_func_free_params(params) ;
    return false ;

fail:
	 printf("The system function List.ToJson($isJsonObj:Boolean, out-> $error:String):String failed.\n");
	return true ;
}



bool hdr_domJsonObjToString(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token, PHDR_VAR for_var, PHDR_VAR *result)
{
	/*
	 The function try to Jsonfy the fast list
	*/

   PHDR_SYS_FUNC_PARAMS params = hdr_sys_func_init_params(inter,token->parameters,1) ;
   if(params == NULL)
   {
    printf("The system function List.ToJson(out-> $error:String) failed.\n");
    return true ;
   }

   bool type_error = false ;

   PDX_STRING error = hdr_inter_ret_string(params->params[0],&type_error) ; 
   if(type_error == true)
   {
		printf("The parameter must be a simple string\n");
		goto fail ;
   }
   
    PDX_HASH_TABLE root  = (PDX_HASH_TABLE)for_var->obj ;
	 
	PDX_STRING str =  NULL ;
	
	str = hdrObjectToString(root,error) ;
	
	if(str == NULL) str = dx_string_createU(NULL,"")    ;

	*result = hdr_var_create(str,"",hvf_temporary_ref,NULL) ;
	(*result)->type = hvt_simple_string						;

    hdr_sys_func_free_params(params) ;
    return false ;

fail:
	printf("The system function List.ToJson(out-> $error:String) failed.\n");
	return true ;
}

/*XML SUPPORT*/

bool hdr_domListToXmlStr(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token,PHDR_VAR for_var,PHDR_VAR *result)
{
	/*
	 the function returns a properly constructed list as an xml formated string
	*/

   PHDR_SYS_FUNC_PARAMS params = hdr_sys_func_init_params(inter,token->parameters,1) ;
   if(params == NULL)
   {
	 printf("The system function List.ListToXmlStr($error:String):String failed.\n");
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

   PDX_LIST list  = (PDX_LIST)for_var->obj ;

   

   *result   = hdrListToXmlStr(list,error) ;
   (*result)->type = hvt_simple_string ;

    success:
    hdr_sys_func_free_params(params) ;
    return false ;

    fail : 
    printf("The system function  List.ListToXmlStr($error:String):String failed.\n");
    hdr_sys_func_free_params(params) ;
    return true ;
}











