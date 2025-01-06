/*
 Special System Functions

 Nikos Mourgis deus-ex.gr 2024

 Live Long and Prosper
*/

bool hdr_domHydraScriptPath(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token, PHDR_VAR *result)
{
   PHDR_SYS_FUNC_PARAMS params = hdr_sys_func_init_params(inter,token->parameters,0) ;
   if(params == NULL)
   {
    printf("The system function Hydra.ScriptPath() failed.\n");
    return true ;
   }

   *result = hdr_var_create(NULL, "", hvf_temporary_ref, NULL) ;
   (*result)->type    = hvt_simple_string ; 
   PDX_STRING ns = dx_string_createU(NULL,inter->loader->main_path->stringa);
   hdr_var_set_obj(*result,ns) ;
   if((*result)->obj == NULL) goto fail ;

    success:
    hdr_sys_func_free_params(params) ;
    return false ;

    fail : 
    printf("The system function Hydra.ScriptPath() failed.\n");
    hdr_sys_func_free_params(params) ;
    return true ;
}

bool hdr_domHydraPath(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token, PHDR_VAR *result)
{
   PHDR_SYS_FUNC_PARAMS params = hdr_sys_func_init_params(inter,token->parameters,0) ;
   if(params == NULL)
   {
    printf("The system function Hydra.Path() failed.\n");
    return true ;
   }

   *result = hdr_var_create(NULL, "", hvf_temporary_ref, NULL) ;
   (*result)->type    = hvt_simple_string ; 
   PDX_STRING ns = dx_string_createU(NULL,inter->loader->exe_path->stringa);
   hdr_var_set_obj(*result , ns) ;
   if((*result)->obj == NULL) goto fail ;

    success:
    hdr_sys_func_free_params(params) ;
    return false ;

    fail : 
    printf("The system function Hydra.Path() failed.\n");
    hdr_sys_func_free_params(params) ;
    return true ;
}

bool hdr_domHydraScriptName(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token, PHDR_VAR *result)
{
   PHDR_SYS_FUNC_PARAMS params = hdr_sys_func_init_params(inter,token->parameters,0) ;
   if(params == NULL)
   {
    printf("The system function Hydra.ScriptName() failed.\n");
    return true ;
   }

   *result = hdr_var_create(NULL, "", hvf_temporary_ref, NULL) ;
   (*result)->type    = hvt_simple_string ; 
   PDX_STRING ns = dx_string_createU(NULL,inter->loader->main_name->stringa);
   hdr_var_set_obj(*result , ns) ;
   if((*result)->obj == NULL) goto fail ;

    success:
    hdr_sys_func_free_params(params) ;
    return false ;

    fail : 
    printf("The system function Hydra.ScriptName() failed.\n");
    hdr_sys_func_free_params(params) ;
    return true ;
}