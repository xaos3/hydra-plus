/*
 This file has the wraper functions for the [reproc] 
 third party library : https://github.com/DaanDeMeyer/reproc/tree/main/reproc
 for manipulation of proccesses in windows and linux.

 Nikos Mourgis 
 deus-ex.gr 2024

 Live Long And Prosper.

*/


#include <reproc/run.h>

typedef reproc_t *PHDR_PROCESS ;

PDX_STRING hdr_run_exe(PDX_STRING exe_name,PDX_STRING work_dir, PDX_STRINGLIST parameters,PHDR_PROCESS *out_process,bool *error) ;
void hdr_terminate_process(PHDR_PROCESS process);
/*#
 The function runs a program (exe_name) and sets the working directory (work_dir if the work_dir is empty then
 the home dir remains the default of the system , most probably the calling procces work dir ).
 The parameters will pass as the executable parameters, every parameter HAS to be in its own line.  
 
 The [process] will set to the process handle, this is usefull in multi threading
 enviroments so the thread can return a global handle for the program to use for 
 terminating the process.It can be NULL.

 Note , free the return PDX_STRING to avoid memory leaks 

 Warning , The function is a blocking function, for not blocking capabilities run this in a thread.


#*/

PDX_STRING hdr_run_exe(PDX_STRING exe_name,PDX_STRING work_dir,PDX_STRINGLIST parameters,PHDR_PROCESS *out_process, bool *error)
{

    *error = false ;
    reproc_t *process = NULL;
    char *output = NULL;
    int r = REPROC_ENOMEM;

    char **argv = malloc(sizeof(char*)*(parameters->count+2))  ;
    /*create a string list to accumulate the output*/
    PDX_STRINGLIST strlist = dx_stringlist_create() ;
    process = reproc_new();
    *out_process = process ;
    if (process == NULL)  goto finish;
  
    /*
    construct the options
    */
    reproc_options options = {0};
    options.nonblocking           = true ;
    if(work_dir->len == 0)
    options.working_directory     = NULL        ;
    else
        options.working_directory = work_dir->stringa ;
    /*construct the parameters, the form is ["exe_name","param1","param2",...,NULL]*/
      
    argv[parameters->count+1] = NULL ;
    argv[0] = exe_name->stringa ;

    int indx = 1 ;
    PDXL_NODE node = parameters->start ;
    while(node != NULL)
    {
        argv[indx] = node->object->key->stringa ;  
        indx++ ;
        node = node->right ;
    }

    r = reproc_start(process, argv, options);
    if (r < 0) goto finish;

    r = reproc_close(process, REPROC_STREAM_IN);
    if (r < 0) goto finish;
    

    // `reproc_drain` reads from a child process and passes the output to the
    // given sinks. A sink consists of a function pointer and a context pointer
    // which is always passed to the function. reproc provides several built-in
    // sinks such as `reproc_sink_string` which stores all provided output in the
    // given string. Passing the same sink to both output streams makes sure the
    // output from both streams is combined into a single string.
    reproc_sink sink = reproc_sink_string(&output);
    // By default, reproc only redirects stdout to a pipe and not stderr so we
    // pass `REPROC_SINK_NULL` as the sink for stderr here. We could also pass
    // `sink` but it wouldn't receive any data from stderr.
    r = reproc_drain(process, sink, REPROC_SINK_NULL);
    if (r < 0) goto finish;


    dx_stringlist_add_raw_string(strlist, output);

    r = reproc_wait(process, REPROC_INFINITE);
    if (r < 0) goto finish;

finish:
    // Memory allocated by `reproc_sink_string` must be freed with `reproc_free`.
    //*out_process = NULL ; /*i will let the variable to be handled by the caller*/ 
    reproc_free(output);
    reproc_destroy(process);
    free(argv) ;

    if (r < 0)
    {
      *error = true ;
      dx_stringlist_free(strlist);
      return dx_string_createU(NULL,reproc_strerror(r)) ;
    }
          
    PDX_STRING ret = dx_stringlist_raw_text(strlist) ;
    dx_stringlist_free(strlist) ;
    return ret ;
}

void hdr_terminate_process(PHDR_PROCESS process)
{
    reproc_terminate(process);
    reproc_destroy(process);
}

