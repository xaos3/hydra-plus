/*


 Usage : 
 Hydra+ mainscript.hydra     : Runs the mainscript.hydra as an executable script
 Hydra+ -o somescript.hydra  : Reads the somescript.hydra and creates a new obfuscated file in the same dir, with the same name but with the added
							   extension [.obf] . e.g somescript.hydra.obf 
Hydra+ -c somescript.hydra	 : "Compiles" a script and its includes to a large obfuscated file in the same directory with the main file.  

 Pass parameters from the command as variables in the script.
 Hydra+ can take values from the command line and fill variables in the script mid execution.
 Set in the main script the variables that you want , for example 
 $param1 = "" ;
 $param2 = 0 ;

 In the command prompt run Hydra+ as :
 Windows -> Hydra+ mainscript.hydra -p [$param1=`hello`,$param2=5]
 Linux   -> Hydra+ mainscript.hydra -p [\$param1=\"hello\",\$param2=5]

 Use the fill_params to fill the variables in the script code with the values from the command line parameters


 Hydra+ 
 Nikos Mourgis 
 deus-ex.gr 2024

 Live Long and Prosper

*/

#define _LINUX

#define _HTTP_DEBUG
#ifndef UNICODE
#define UNICODE
#endif
#ifdef _LINUX
#define _GNU_SOURCE
#include <signal.h>
#include <ctype.h>
#include <wctype.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h> 
#endif // _LINUX

#include <stdlib.h>


#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <direct.h>
#endif

#include <stdint.h>
#include <stdbool.h>
#include <errno.h>
#include <stdio.h>
#include <time.h>
#include <math.h>
#include <wchar.h>
#include <string.h>
#include <locale.h>
#include <float.h>
#include <hydra.h>


#ifdef _WIN32
#define CP_UTF8  65001
#endif


/*
 handle the broken pipe signal in linux. If we do not handle this , when the tcp socket is closed and we try
 to write or read from it the proccess is terminated . We need to recover for this error so we will create a dummy handler  
*/
#ifdef _LINUX
void sigpipe_handler_dummy(int notused)
{
	return ;
}
#endif



int main(int argc, char** argv)
{

/*UTF8 SUPPORT IN CONSOLE*/

	#ifdef _LINUX
	  sigaction(SIGPIPE,&(struct sigaction){sigpipe_handler_dummy},NULL) ;
	  setlocale(LC_ALL, "");
	#endif

	#ifdef _WIN32
		SetConsoleOutputCP(CP_UTF8);
		SetConsoleCP(CP_UTF8);
	#endif
/*************************/

    char *main_script   = NULL  ;
	bool params_exists  = false ;
	/*check parameters and mode of operation*/
	if( argc < 2 )
	{
	  printf("***Fatal Error : The minimum parameters that Hydra+ needs to execute a script is the full file name of the script\n");
      printf("Examples :\n");
	  printf("hydra+ /home/scripts/mainscript.hydra\n");
	  printf("hydra+ \"C:\\Program Files\\scripts\\mainscript.hydra\" \n");
	  return 0 ;
	}


	if(argv[1][0] != '-')
	{
	   /*retrieve next param as the scripts file name*/
	   main_script = argv[1] ;
	   /*check if command line sets any variables*/
	   if(argc > 2 )
	   {
	     if((argv[2][0] == '-')&&(argv[2][1] == 'p'))
		 {
		   if(argc < 4 ) 
		   {
		    printf("***Fatal Error : After the -p flag a string of variable assigments for Hydra+ must be set. Example : -p [$var1=0,$var2=`hello`]\n");
		   }

		   /*set the flag to signal Hydra+ to load the parameters */
		   params_exists = true ;
		 }
		 else
		 {
		  printf("***Fatal Error : The second parameter for the Hydra+ MUST be the -p flag\n");
		  return 0 ;
		 }
	   
	   }

	}
	else
	 if((argv[1][0] == '-')&&(argv[1][1] == 'o'))
	 {
		 /*load the file*/
		 char * text = hdr_loader_load_text_from_disk(argv[2]) ;
	     /*obfuscate the file*/
		 PHDR_ENCRYPTED_BUF buf = hdr_loader_encrypt_buf_create(text) ;
		 /*write the file in the disk*/
		 PDX_STRING temps = dx_string_createU(NULL,argv[2]);
		 PDX_STRING ext   = dx_string_createU(NULL,".obf") ;
		 PDX_STRING newf  = dx_string_concat(temps,ext)	   ;
		 enum  hdr_save_to_disk_state state = hdr_loader_save_binary_to_disk(newf->stringa, buf , true) ;
		 switch(state)
		 {
			 case hdr_sd_filename_null : {printf("***Fatal Error : The file name for the obfuscated file is NULL\n");}break;
			 case hdr_sd_ebuf_null	   : {printf("***Fatal Error : The [ebuf] is NULL\n");}break;
			 case hdr_sd_ebuf_buf_null : {printf("***Fatal Error : The ebuffer is NULL\n");}break;
			 case hdr_sd_no_open	   : {printf("***Fatal Error : Unable to open the file [%s]\n",newf->stringa);}break;
			 case hdr_sd_success	   : {printf("Success! A new obfuscated file was created . File name : %s\n",newf->stringa);}break;
		 }
		 /*do not free any memory the program exits and its lifetime ends! :'( */
		 return 0 ;
	 }
	 else
	 if((argv[1][0] == '-')&&(argv[1][1] == 'c'))
	 {

		PHYDRA hydra = HydraCreate(argv[0], NULL); /*create the hydra object and set the executable's directory*/
		if (hydra == NULL)
		{
			printf("***FATAL ERROR : Hydra object does not created.\n");
			return 0 ;
		}
		main_script = argv[2] ;

		 /*setup the loader with the main script file*/
		 if(HydraLoadScript(hydra , main_script) == false)
		 {
		   printf("***Fatal error. The operation failed.");
		   return ;
		 }
		 
		 PDX_STRINGLIST include_list = dx_stringlist_create() ;
		 char *main_indx = NULL;
		 if(hdr_compile_include_files(hydra->loader,include_list,&main_indx) == false)
		 {
		   printf("***Fatal error. The operation failed.");
		   return ;
		 }

		 /*now we will create the monolithic file*/

		 PDX_STRING temps = dx_string_createU(NULL,argv[2]);
		 PDX_STRING ext   = dx_string_createU(NULL,".comp") ;
		 PDX_STRING newf  = dx_string_concat(temps,ext)	   ;

		 

		 FILE *f = fopen(newf->stringa,"wb") ;

		 PDXL_NODE node = include_list->start ;
		 while(node!=NULL)
		 {
		   PDX_STRING file_name = node->object->key ; 
           FILE *inf = fopen(file_name->stringa,"rb");
		   DXLONG64 fsize = dx_GetFileSize(inf) ;
		   char * buff = (char*)malloc(fsize)   ;
		   fread(buff,fsize,1,inf);
		   fwrite(buff,fsize,1,f) ;
		   node = node->right ;
		 }

		 /*write the monolithic file*/
		 fwrite(main_indx,strlen(main_indx),1,f) ;

		 fclose(f);

		 PDX_STRING monolithic = dx_string_createU(NULL,newf->stringa) ;
		 newf = dx_string_free(newf);
		
		 /*load the file*/
		 char * text = hdr_loader_load_text_from_disk(monolithic->stringa) ;

		 PHDR_ENCRYPTED_BUF buf = hdr_loader_encrypt_buf_create(text) ;
		 /*write the file in the disk*/
		 temps = dx_string_createU(NULL,argv[2]);
		 ext   = dx_string_createU(NULL,".obf") ;
		 newf  = dx_string_concat(temps,ext)	   ;
		 enum  hdr_save_to_disk_state state = hdr_loader_save_binary_to_disk(newf->stringa, buf , true) ;
		 switch(state)
		 {
			 case hdr_sd_filename_null : {printf("***Fatal Error : The file name for the obfuscated file is NULL\n");}break;
			 case hdr_sd_ebuf_null	   : {printf("***Fatal Error : The [ebuf] is NULL\n");}break;
			 case hdr_sd_ebuf_buf_null : {printf("***Fatal Error : The ebuffer is NULL\n");}break;
			 case hdr_sd_no_open	   : {printf("***Fatal Error : Unable to open the file [%s]\n",newf->stringa);}break;
			 case hdr_sd_success	   : {printf("Success! A new monolithic obfuscated file was created . File name : %s\n",newf->stringa);}break;
		 }
		 /*do not free any memory the program exits and its lifetime ends! :'( */
		 return 0 ;
	 }
	 else
	 {
	  printf("***Fatal Error : The flag [%s] is not a valid flag for the Hydra+ command line. valid flags : [-o,-c]\n",argv[1]);
	  return 0 ;
	 }	

	PHYDRA hydra = HydraCreate(argv[0], NULL); /*create the hydra object and set the executable's directory*/
	if (hydra == NULL)
	{
		printf("***FATAL ERROR : Hydra object does not created.\n");
		return 0 ;
	}

	if(params_exists == true)
	{
	  if(HydraLoadParams(hydra,argv[4]) == false) return 0 ; /*the function has print the appropriate error code*/
	}

	if (HydraLoadScript(hydra, main_script) == false)
	{
		printf("***Fatal Error : The script was not loaded. Script : %s\n",argv[2]);
	}
	else
	{

		if (HydraCreateCode(hydra) != HDR_SUCCESS)
		{
			printf("***Fatal Error : Error while loading and creating the internal code structures.\n");
			return 0;
		}

		HydraExecute(hydra);
	}
	
	end:

	HydraFree(hydra);

	return 0;
}











