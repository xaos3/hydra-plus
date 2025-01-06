/*
 This file has the implementation for the date/time support.

 Nikos Mourgis deus-ex.gr 2024

 Live Long and Prosper !
*/


#include "dxdates.h"



bool hdr_funcDateToPOSIX(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token,PHDR_VAR *result)
{
   /*accepts a datetime of the format YYYY-MM-DD 00:00:00 and returns a posix time of the seconds after since 00:00, Jan 1 1970 UTC 
    returns -1 if the date is malformed and did not converted
   */
   PHDR_SYS_FUNC_PARAMS params = hdr_sys_func_init_params(inter,token->parameters,1) ;
   if(params == NULL)
   {
    printf("The system function dateToPOSIX($date : String as [YYYY-MM-DD 00:00:00]) : POSIX time (Integer) failed.\n");
    return true ;
   }

   bool type_error = false ;

   PDX_STRING date = hdr_inter_ret_string(params->params[0],&type_error) ; 
   if(type_error == true)
   {
	 printf("The first parameter must be a String.\n");
     goto fail ;
   }

   *result = hdr_var_create(NULL, "", hvf_temporary_ref, NULL) ;
   (*result)->type        = hvt_integer ;  
   (*result)->integer     = dx_convert_to_time(date->stringa)  ;

   success:
    hdr_sys_func_free_params(params) ;
    return false ;

   fail : 
    printf("The system function dateToPOSIX($date : String as [YYYY-MM-DD 00:00:00]) : POSIX time (Integer) failed.\n");
    hdr_sys_func_free_params(params) ;
    return true ;
}


bool hdr_funcDateNowPOSIX(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token,PHDR_VAR *result)
{
   /*
   * returns this moment as POSIX time_t
   */
   PHDR_SYS_FUNC_PARAMS params = hdr_sys_func_init_params(inter,token->parameters,0) ;
   if(params == NULL)
   {
    printf("The system function nowPOSIX() : POSIX time failed.\n");
    return true ;
   }

   *result = hdr_var_create(NULL, "", hvf_temporary_ref, NULL) ;
   (*result)->type        = hvt_integer ;  
   (*result)->integer     = dx_Now()  ;

   success:
    hdr_sys_func_free_params(params) ;
    return false ;

   fail : 
    printf("The system function nowPOSIX() : POSIX time failed.\n");
    hdr_sys_func_free_params(params) ;
    return true ;
}


bool hdr_funcTimeToDate(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token,PHDR_VAR *result)
{
   /*
    Returns the time as a string based to the format 
   */
   PHDR_SYS_FUNC_PARAMS params = hdr_sys_func_init_params(inter,token->parameters,2) ;
   if(params == NULL)
   {
    printf("The system function timeToDate($time : POSIX Time,$format : String) : String failed.\n");
    return true ;
   }

   bool type_error = false ;

   DXLONG64 tme = hdr_inter_ret_integer(params->params[0],&type_error) ; 
   if(type_error == true)
   {
	 printf("The first parameter must be an Integer.\n");
     goto fail ;
   }

   PDX_STRING format = hdr_inter_ret_string(params->params[1],&type_error) ; 
   if(type_error == true)
   {
	 printf("The second parameter must be a String.\n");
     goto fail ;
   }

   *result = hdr_var_create(NULL, "", hvf_temporary_ref, NULL) ;
   (*result)->type        = hvt_simple_string ;  
   PDX_STRING ns          = dx_convert_to_time_ex(format->stringa,tme) ; 
   hdr_var_set_obj(*result,ns);

   success:
    hdr_sys_func_free_params(params) ;
    return false ;

   fail : 
    printf("The system function timeToDate($time : POSIX Time,$format : String) : String failed.\n");
    hdr_sys_func_free_params(params) ;
    return true ;
}

bool hdr_funcNow(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token,PHDR_VAR *result)
{
   /*
    Returns the current moment as a string in the form YYYY-MM-DD (optional) ->  00:00:00 
   */
   PHDR_SYS_FUNC_PARAMS params = hdr_sys_func_init_params(inter,token->parameters,1) ;
   if(params == NULL)
   {
    printf("The system function Now($add_time : boolean) : String failed.\n");
    return true ;
   }

   bool type_error = false ;

   bool add_time = hdr_inter_ret_bool(params->params[0],&type_error) ; 
   if(type_error == true)
   {
	 printf("The first parameter must be a Boolean.\n");
     goto fail ;
   }

   *result = hdr_var_create(NULL, "", hvf_temporary_ref, NULL) ;
   (*result)->type        = hvt_simple_string ;  
   PDX_STRING ns          = dx_time_to_date_str(dx_Now(),add_time)  ;
   hdr_var_set_obj(*result,ns); 

   success:
    hdr_sys_func_free_params(params) ;
    return false ;

   fail : 
    printf("The system function Now($add_time : boolean) : String failed.\n");
    hdr_sys_func_free_params(params) ;
    return true ;
}


bool hdr_funcDaysDiff(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token,PHDR_VAR *result)
{
   /*
    Returns the days difference between two dates 
   */
   PHDR_SYS_FUNC_PARAMS params = hdr_sys_func_init_params(inter,token->parameters,2) ;
   if(params == NULL)
   {
    printf("The system function daysDiff($date1 : String , $date2 : String) : Integer failed.\n");
    return true ;
   }

   bool type_error = false ;

   PDX_STRING date1 = hdr_inter_ret_string(params->params[0],&type_error) ; 
   if(type_error == true)
   {
	 printf("The first parameter must be a String.\n");
     goto fail ;
   }

   PDX_STRING date2 = hdr_inter_ret_string(params->params[1],&type_error) ; 
   if(type_error == true)
   {
	 printf("The second parameter must be a String.\n");
     goto fail ;
   }

   *result = hdr_var_create(NULL, "", hvf_temporary_ref, NULL) ;
   (*result)->type        = hvt_integer ;  
   (*result)->integer     = dx_days_diff(date1->stringa,date2->stringa)  ;

   success:
    hdr_sys_func_free_params(params) ;
    return false ;

   fail : 
    printf("The system function daysDiff($date1 : String , $date2 : String) : Integer failed.\n");
    hdr_sys_func_free_params(params) ;
    return true ;
}


bool hdr_funcSecDiff(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token,PHDR_VAR *result)
{
   /*
    Returns the days difference between two dates 
   */
   PHDR_SYS_FUNC_PARAMS params = hdr_sys_func_init_params(inter,token->parameters,2) ;
   if(params == NULL)
   {
    printf("The system function secsDiff($date1 : String , $date2 : String) : Integer failed.\n");
    return true ;
   }

   bool type_error = false ;

   PDX_STRING date1 = hdr_inter_ret_string(params->params[0],&type_error) ; 
   if(type_error == true)
   {
	 printf("The first parameter must be a String.\n");
     goto fail ;
   }

   PDX_STRING date2 = hdr_inter_ret_string(params->params[1],&type_error) ; 
   if(type_error == true)
   {
	 printf("The second parameter must be a String.\n");
     goto fail ;
   }

   *result = hdr_var_create(NULL, "", hvf_temporary_ref, NULL) ;
   (*result)->type        = hvt_integer ;  
   (*result)->integer     = dx_sec_diff(date1->stringa,date2->stringa)  ;

   success:
    hdr_sys_func_free_params(params) ;
    return false ;

   fail : 
    printf("The system function secsDiff($date1 : String , $date2 : String) : Integer failed.\n");
    hdr_sys_func_free_params(params) ;
    return true ;
}


bool hdr_funcAddDays(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token,PHDR_VAR *result)
{
   /*
    Add the days (or subtract them if the days are a negative number) 
   */
   PHDR_SYS_FUNC_PARAMS params = hdr_sys_func_init_params(inter,token->parameters,2) ;
   if(params == NULL)
   {
    printf("The system function addDays($date : String , $days : Integer(+/-)) : POSIX time failed.\n");
    return true ;
   }

   bool type_error = false ;

   PDX_STRING date = hdr_inter_ret_string(params->params[0],&type_error) ; 
   if(type_error == true)
   {
	 printf("The first parameter must be a String.\n");
     goto fail ;
   }

   DXLONG64 days = hdr_inter_ret_integer(params->params[1],&type_error) ; 
   if(type_error == true)
   {
	 printf("The second parameter must be an Integer.\n");
     goto fail ;
   }

   *result = hdr_var_create(NULL, "", hvf_temporary_ref, NULL) ;
   (*result)->type        = hvt_integer ;  
   (*result)->integer     = dx_add_days(date->stringa,days)  ;

   success:
    hdr_sys_func_free_params(params) ;
    return false ;

   fail : 
    printf("The system function secsDiff($date : String , $days : Integer(+/-)) : Integer failed.\n");
    hdr_sys_func_free_params(params) ;
    return true ;
}

bool hdr_funcDatesCompare(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token,PHDR_VAR *result)
{
   /*
    Compares the two dates , the result is applied to the base date , meaning that if the result is bigger then the base date
    is bigger than compare date
   */
   PHDR_SYS_FUNC_PARAMS params = hdr_sys_func_init_params(inter,token->parameters,2) ;
   if(params == NULL)
   {
    printf("The system function datesCompare($base_date : String , $compare_date : String) : [bigger | smaller | equal] failed.\n");
    return true ;
   }

   bool type_error = false ;

   PDX_STRING date1 = hdr_inter_ret_string(params->params[0],&type_error) ; 
   if(type_error == true)
   {
	 printf("The first parameter must be a String.\n");
     goto fail ;
   }

   PDX_STRING date2 = hdr_inter_ret_string(params->params[1],&type_error) ; 
   if(type_error == true)
   {
	 printf("The second parameter must be a String.\n");
     goto fail ;
   }

   *result = hdr_var_create(NULL, "", hvf_temporary_ref, NULL) ;
   (*result)->type        = hvt_integer ;  
   enum dx_date_comp ret  = dx_dates_compare(date1->stringa,date2->stringa) ;
   if(ret == dx_date_smaller) (*result)->integer = HDR_INTER_SMALLER;
   else
    if(ret == dx_date_bigger) (*result)->integer = HDR_INTER_BIGGER;
    else
        if(ret == dx_date_equal) (*result)->integer = HDR_INTER_EQUAL;

   success:
    hdr_sys_func_free_params(params) ;
    return false ;

   fail : 
    printf("The system function datesCompare($date1 : String , $date2 : String) : [bigger | smaller | equal] failed.\n");
    hdr_sys_func_free_params(params) ;
    return true ;
}


void dx_func_hlp_create_list_int_var(PDX_LIST list,DXLONG64 val,char *indx_name)
{
   PDXL_OBJECT obj   = dxl_object_create() ;
   PHDR_VAR    tvar  = hdr_var_create(NULL,"",hvf_dynamic,NULL) ;
   tvar->type      = hvt_integer ;
   tvar->integer   = val         ;
   obj->obj = (void*)tvar		 ;
   obj->key = dx_string_createU(NULL,indx_name) ;
   dx_list_add_node_direct(list,obj) ;
}

bool hdr_funcDateInfo(PHDR_INTERPRETER inter, PHDR_COMPLEX_TOKEN token,PHDR_VAR *result)
{
   /*
    Returns the info of the date in a list 
   */
   PHDR_SYS_FUNC_PARAMS params = hdr_sys_func_init_params(inter,token->parameters,1) ;
   if(params == NULL)
   {
     printf("The system function dateInfo($date : String as a YYYY-MM-DD 00:00:00 string) : List failed.\n");
     return true ;
   }

   bool type_error = false ;

   PDX_STRING bdate = hdr_inter_ret_string(params->params[0],&type_error) ; 
   if(type_error == true)
   {
	 printf("The first parameter must be a String.\n");
     goto fail ;
   }

   struct tm tim ;
   time_t posixt = dx_convert_to_time_struct(bdate->stringa,&tim) ;
   /*construct the list*/
   PDX_LIST list = dx_list_create_list() ;
   
   dx_func_hlp_create_list_int_var(list,posixt,"posix time") ;
   dx_func_hlp_create_list_int_var(list,tim.tm_sec,"seconds") ;
   dx_func_hlp_create_list_int_var(list,tim.tm_min,"minutes") ; 
   dx_func_hlp_create_list_int_var(list,tim.tm_hour,"hours") ;
   dx_func_hlp_create_list_int_var(list,tim.tm_mday,"month day") ; 
   dx_func_hlp_create_list_int_var(list,tim.tm_mon+1,"month") ;
   dx_func_hlp_create_list_int_var(list,tim.tm_year+1900,"year") ;
   dx_func_hlp_create_list_int_var(list,tim.tm_wday+1,"week day")  ;
   dx_func_hlp_create_list_int_var(list,tim.tm_yday+1,"year day")  ;
   dx_func_hlp_create_list_int_var(list,tim.tm_isdst,"daylight")   ;/*<0 no available info , >0 is in effect , 0 is not in effect*/

   *result = hdr_var_create(NULL, "", hvf_temporary_ref, NULL) ;
   (*result)->type        = hvt_list ;  
   hdr_var_set_obj(*result,list);

   success:
    hdr_sys_func_free_params(params) ;
    return false ;

   fail : 
    printf("The system function dateInfo($date : String as a YYYY-MM-DD 00:00:00 string) : List failed.\n");
    hdr_sys_func_free_params(params) ;
    return true ;
}







