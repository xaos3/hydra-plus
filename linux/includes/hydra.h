/*
 The main hydra+ header file

 Live long and prosper.

 Nikos Mourgis deus-ex.gr 2024
*/


#ifndef DXSPFUNCTIONS
#include "dxspfunctions.h"
#endif

#ifndef DXDATABASES
#include "databases.h"
#endif

#ifndef ALG_PARSE
#include "alg_parse.h"
#endif

#include "hydra_proc.h"

#ifndef HYDRA_INTERPRETER
#include "hydra_interpreter.h"
#endif

#ifndef HYDRA_COMPILE
#include "hydra_compile.h"
#endif



#define ERROR_HYDRA_IS_NULL 301 ;


/*
 The basic Hydra+ enviroment structure. In this structure all the 
 information about a hydra application are stored
*/
typedef struct hydra
{
    PHDR_VAR_LIST       defines      ; /* The special symbols that are defined in the script  */

    PHDR_LOADER         loader       ; /*The loader object */
    PHDR_INTERPRETER    interpreter  ;

    PHDR_VAR_LIST       params       ; /*the params that the command line passed to the script in the form of variables (string or numeric)*/

    PHDR_THREAD_LIST    threads      ; /*the threads the script has created*/

    PHDR_POINTERS      finalizer     ; /*this object will be initialized before the main block destruction to avoid redisposed already disposed pointers */
     
   PHDR_POINTERS_LOGGER logger       ; /*this object it will used to the special debugging state that will detect memory leaks*/

} *PHYDRA;

/*Functions for the Hydra+ script execution*/

PHYDRA HydraCreate(char *exepath, PHDR_BLOCK block);
/*#
 This function creates a new hydra object with initialized its members ready to load a script
 for execution. Returns NULL if the function failed to allocate the memory.
 The block will pass as the parent block to the code of the script
#*/

PHYDRA HydraFree(PHYDRA hydra);
/*#
 Frees the hydra object and deallocates all the memory
#*/

bool HydraLoadScript(PHYDRA hydra , char* mainscript,PHDR_RAW_BUF embedded_scr);
/*#
 Loads the mainscript and its includes
#*/

int HydraCreateCode(PHYDRA hydra);
/*#
 Invokes the loader and create the code from the scripts.
#*/

bool HydraSetParams(PHYDRA hydra);
/*sets the parameters from the command line in the script code*/


/************* Implementation **************************************************/


PHYDRA HydraCreate(char *exepath,PHDR_BLOCK block)
{
    PHYDRA hydra            = (PHYDRA)malloc(sizeof(struct hydra))  ;
    if (hydra == NULL)      return NULL                             ;

    hydra->params           = hdr_var_list_create(NULL)             ;
    hydra->defines          = hdr_define_list_create()              ;
    hydra->loader           = hdr_loader_create(exepath,block)      ;
    hydra->threads          = hdr_thread_list_create()              ;
    hydra->finalizer        = NULL                                  ;
    hydra->logger           = NULL                                  ;

    if (hydra->defines == NULL)
    {
        free(hydra);
        return NULL;
    }

    if (hydra->loader == NULL)
    {
        hdr_var_list_free(hydra->defines) ;
        free(hydra);
        return NULL;
    }

    hydra->interpreter = hdr_interpreter_create(hydra->loader,false);
    if (hydra->interpreter == NULL)
    {
        hdr_var_list_free(hydra->defines) ;
        hdr_loader_free(hydra->loader)    ;
        free(hydra);
        return NULL;
    }

    return hydra ;
}


PHYDRA HydraFree(PHYDRA hydra)
{
    if (hydra == NULL) return NULL;
    /*mem managemnt*/
    hydra->finalizer        = hdr_mem_pointers_create()                         ;
    _POINTERS = hydra->finalizer ;
    /**************/
    hydra->params           = hdr_var_list_free(hydra->params)                  ;
    hydra->interpreter      = hdr_interpreter_free(hydra->interpreter,false)    ;
    hydra->defines          = hdr_define_list_free(hydra->defines)              ;
    hydra->loader           = hdr_loader_free(hydra->loader)                    ;
    /*send signal to all the threads to terminate and wait*/
    hydra->threads          = hdr_thread_list_free(hydra->threads)              ;
    hydra->finalizer        = hdr_mem_pointers_free(hydra->finalizer) ;
    _POINTERS = NULL ;
    
     /*check if the logger is active*/
    if(hydra->logger != NULL)
    {
      /*Print all the variables that are not freed*/
      hdr_mem_pointers_logger_print(hydra->logger) ;
    }
    
    free(hydra);
    /*check and cleanup the network*/
#ifdef _WIN32
    if (IS_SOCKETS_INIT == true) WSACleanup();
#endif
    if(IS_WOLF_INIT == true) wolfSSL_Cleanup();
    return NULL ;

}

bool HydraLoadScript(PHYDRA hydra , char *mainscript,PHDR_RAW_BUF embedded_scr)
{
    if (hydra == NULL)
    {
        printf("**System : Fatal Error -> [hydra] is NULL\n");
        return false;
    }
    if ((mainscript == NULL)&&(embedded_scr == NULL))
    {
        printf("**System : Fatal Error -> [mainscript] is NULL\n");
        return false;
    }

    if (hdr_loader_load_main_script(hydra->loader, mainscript,embedded_scr) == false)
    {
        printf("**System : Fatal Error -> Main Script was not loaded. File name : %s\n",mainscript);
        return false ; 
    }

    if (hdr_loader_load_include_files(hydra->loader) == false)
    {
        printf("**System : Fatal Error -> Some included file were not loaded. File name : %s\n", mainscript);
        return false ;
    }

    return true ;
}

int HydraCreateCode(PHYDRA hydra)
{
    if (hydra == NULL) return ERROR_HYDRA_IS_NULL ;
    return hdr_loader_create_code(hydra->loader)  ;
}

enum exec_state  HydraExecute(PHYDRA hydra)
{
    if (hydra == NULL) return ERROR_HYDRA_IS_NULL;
    /*set the memory logger*/
    if(_ON_MEMORY_DETECT == true )
    {
      hydra->logger = hdr_mem_pointers_Logger_create() ;
      _LOGGER       = hydra->logger ;  

    }
    /*start the interpreter and the code execution*/
    if(hdr_interpreter_init(hydra->interpreter) == false) return exec_state_error; /*call this to setup the current block and the current instruction*/
   	/*set the parameters list*/
    hydra->interpreter->params  = hydra->params  ;
    /*set the thread list*/
    hydra->interpreter->threads = hydra->threads ;
    /*set the command line variables in the code*/
    return hdr_inter_execute_instructions(hydra->interpreter);

}

bool HydraLoadParams(PHYDRA hydra,char * cmd_line)
{
  /*
   the function gets a command line in the form of [$param1="test",$param2=0...] 
   and creates the apropriate variables in the hydra->params
  */
   if(cmd_line == NULL) return true ; /*ewmpty string is not an error, just no parameters*/

   /*check if the string is encapsulated in []*/
   if (dxItsStrEncapsulatedExt(cmd_line,'[',']',"`\"") == false)
   {
       printf("***Fatal Error : The command line variables string is not correctly encapsulated in '[]'\n");
       return false ;
   }
   /*retrieve every sentence between [,] */
   /*trim the [] of the string*/
   cmd_line[strlen(cmd_line)-1] = 0 ;
   char *strindx = &cmd_line[1] ;
   while(*strindx != 0)
   {
     char sep = 0 ;
     char *sentence = dxGetNextWord(&strindx,",","`\"",false ,false, &sep);
     /*we have the sentence , check if its a valid assigment*/
     char *sentenceindx = sentence ;
     char * var     = dxCopyStrToChar(&sentenceindx,'=',"`\"") ; 
     if((*sentenceindx == 0)||(var == NULL)) 
     {
         printf("***Fatal Error : Malformed command line variable. Token : %s",sentence);
         free(sentence) ;
         goto fail ;
     }
   
     if(var[0] != '$')
     {
         printf("***Fatal Error : Malformed command line variable (Forgotten '$' ?). Token : %s",sentence);
         free(sentence) ;
         free(var);
         goto fail ;
     }
     
     DXLONG64 pos = dxCharExistsInStr(sentenceindx,'=',"`\"") ;
     if(pos == -1) 
     {
         printf("***Fatal Error : Malformed command line variable, value pair (The [=] was not found). Token : %s",sentence);
         free(sentence) ;
         free(var)      ;
         goto fail ;
     }
     sentenceindx = &sentenceindx[pos+1];
     char *value = dxCopyStrToChar(&sentenceindx,0,"`\"") ;

     if(value == NULL)
     {
         printf("***Fatal Error : Malformed command line variable value. Token : %s",sentence);
         free(sentence) ;
         free(var)      ;
         goto fail ;
     }

     /*the value can be only string or numeric*/
     
     if(dxIsStringValidEncaps(value,"`\"") == true)
     {
       /*create a string variable*/
       PHDR_VAR   rvar = hdr_var_create(NULL,var,hvf_system,NULL) ; 
       if(value[0] == '`') rvar->type = hvt_simple_string_bcktck ;
       else
           rvar->type = hvt_simple_string ;
       
       /*trim the string encaps*/
       value[strlen(value)-1] = 0 ;
       char *valindx = &value[1] ;

       PDX_STRING str = dx_string_createU(NULL,valindx) ;
       hdr_var_set_obj(rvar,str) ;

       /*add the variable to the list*/
       if(hdr_var_list_find_var(hydra->params,rvar->name) != NULL)
       {
           hdr_var_free(rvar) ;
           free(sentence);
           free(value);
           free(var);
           printf("***Fatal Error : The variable is already being set in the command line. Token : %s",sentence);
           goto fail ;
       }

       hdr_var_list_add(hydra->params,rvar) ;

     }
     else
     {
           /*check if the value is numeric*/
           if(dxIsStrInteger(value) == true)
           {
               bool error = false ;
               /*create an integer variable*/
               PHDR_VAR   rvar = hdr_var_create(NULL,var,hvf_system,NULL) ; 
               rvar->type      = hvt_integer ;
               PDX_STRING strval = dx_string_createU(NULL,value) ;
               rvar->integer   = dx_StringToInt(strval,&error); /*do not check for error as the dxIsStrInteger has returned true*/
               dx_string_free(strval) ;

               /*add the variable to the list*/
               if(hdr_var_list_find_var(hydra->params,rvar->name) != NULL)
               {
                   hdr_var_free(rvar) ;
                   free(sentence);
                   free(value);
                   free(var);
                   printf("***Fatal Error : The variable is already being set in the command line. Token : %s",sentence);
                   goto fail ;
               }

               hdr_var_list_add(hydra->params,rvar) ;
         
       }
       else
           if(dxIsStrReal(value) == true)
           {
              /*create a float variable*/
               bool error = false; 
               PHDR_VAR   rvar = hdr_var_create(NULL,var,hvf_system,NULL) ; 
               rvar->type      = hvt_float ;
               PDX_STRING strval = dx_string_createU(NULL,value) ; 
               rvar->real        = dx_StringToReal(strval,&error);
               dx_string_free(strval) ;
               /*add the variable to the list*/
               if(hdr_var_list_find_var(hydra->params,rvar->name) != NULL)
               {
                   hdr_var_free(rvar) ;
                   free(sentence);
                   free(value);
                   free(var);
                   printf("***Fatal Error : The variable is already being set in the command line. Token : %s",sentence);
                   goto fail ;
               }

               hdr_var_list_add(hydra->params,rvar) ;
           }
           else
           {
            printf("***Fatal Error : Malformed command line variable value. Only hydra+ strings and numeric values are permited. Token : %s",sentence);
            free(sentence);
            free(value);
            free(var);
            goto fail ;
           }

     }

     free(sentence);
     free(value);
     free(var);
   }


   return true ;
fail:

   return false ;

}







