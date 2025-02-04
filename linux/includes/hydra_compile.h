
#define HYDRA_COMPILE

void _hdr_dummy()
{
  /*to use with the dladdr*/
}

bool hdr_compile_include_files(PHDR_LOADER loader,PDX_STRINGLIST include_list,char **main_indx)
{
    if (loader == NULL)
    {
        printf("Fatal Error : The loader is NULL");
        return false;
    }

    /*
      parse the main file and find the include directives,
      save the filenames and the last index (after the last include)
      to use for constructuing the monolithic file
    */

    if (loader->main_script == NULL)
    {
        printf("**System : Fatal Error -> The Loader needs a main script file to proceed");
        return false;
    }

    if (loader->main_script->buf == NULL)
    {
        printf("**System : Fatal Error -> The Loader needs a main script file to proceed");
        return false;
    }

    if (loader->exe_path == NULL)
    {
        printf("**System : Fatal Error -> The Loader does not have initialized the Hydra+ directory path. Why ?");
        return false;
    }

    if (loader->main_path == NULL)
    {
        printf("**System : Fatal Error -> The Loader does not have initialized the main script directory path. Why ?");
        return false;
    }

    char* indx = loader->main_script->buf->buf;
    *main_indx = indx ;
    DXLONG64 ERROR_CODE  = 0;
    DXLONG64 instr_line  = 0;
    DXLONG64 line_number = 1;
    DXLONG64 char_pos = 0;
    bool file_pending = false; /*this is true when the include instruction has been found and the filename must be aquired*/
    while (true)
    {
        /*load instruction until the instruction is not an include*/
        bool terminated;
        char* instr = hdr_parser_get_next_instruction(&indx, &instr_line, &line_number, &char_pos, &ERROR_CODE,&terminated);

        if (instr == NULL)
        {
            /*Nothing to do*/
            break;
        };

        char* tr = hdr_parser_trim(instr, "");/*trim the instruction of the excess characters*/
        if ((ERROR_CODE != HDR_SUCCESS) && (ERROR_CODE != HDR_BLOCK_OPEN) && (ERROR_CODE != HDR_BLOCK_CLOSE)
            && (ERROR_CODE != HDR_SCRIPT_END))
        {
            /*this is a fatal error as is a parsing error , the script cannot be executed*/
            printf("**System : Fatal Error -> The Loader encounters a syntax error. Code : %d  start line : %d  line : %d last character position : %d\n", ERROR_CODE, instr_line, line_number, char_pos);
            free(tr);
            if (instr != NULL) free(instr);
            return false;
        }
        else
        {
            if (ERROR_CODE == HDR_SUCCESS)
            {
                char* tri = tr;
                while (true)
                {   /*analyze the instruction*/
                    int status = 0 ;
                    PHDR_ENTITY entity = hdr_parser_get_next_entity(&tri,&status);
                    if (status != HDR_SUCCESS)
                    {
                        printf("**System : Fatal Error -> A missplaced dot '.' found in the instruction. Start line : %d  line : %d last character position : %d\n", instr_line, line_number, char_pos);
                        free(tr);
                        if (instr != NULL) free(instr);
                        return false;
                    }
                    if (entity == NULL)
                    {
                        printf("**System : Fatal Error -> The Loader failed to allocate memory for the entity. \n\n");
                        free(tr);
                        if (instr != NULL) free(instr);
                        return false;
                    }
                    else
                        if (entity->too_long_entity == true)
                        {
                            printf("**System : Fatal Error -> The Loader found a very large entity. start line : %d  line : %d last character position : %d Entity : %s\n",instr_line, line_number, char_pos, entity->entity->stringa);
                            free(tr);
                            hdr_entity_free(entity);
                            if (instr != NULL) free(instr);
                            return false;
                        }
                        else
                            if ((entity->entity->len == 0) && (entity->separator->len == 0))
                            {
                                /*the end of the instruction*/
                                hdr_entity_free(entity);
                                break;
                            }
                            else
                                if (entity->entity->len == 0)
                                {
                                    /*The absence of the entity suggest that this is not an include instruction */
                                    hdr_entity_free(entity);
                                    free(tr);
                                    if (instr != NULL) free(instr);
                                    return true;
                                }
                                else
                                {
                                    /*we have the entity , check if its the [include] instruction */
                                    enum dx_compare_res res = dx_string_native_compare(entity->entity, hdr_k_include);
                                    if ((res != dx_equal) && (file_pending == false))
                                    {
                                        /*exit , no more valid includes ! */
                                        free(tr);
                                        hdr_entity_free(entity);
                                        if (instr != NULL) free(instr);
                                        return true;
                                        }
                                    else
                                    if ((res == dx_equal) && (file_pending == true))
                                    {
                                        /*some easy to detect syntax error is here*/
                                        printf("**System : Fatal Error -> The Loader encounters a syntax error after the [include]. Start line : %d  line : %d last character position : %d\n", instr_line, line_number, char_pos);
                                        free(tr);
                                        hdr_entity_free(entity);
                                        if (instr != NULL) free(instr);
                                        return false;
                                    }
                                    else 
                                    if ((res == dx_equal) && (file_pending == false))
                                    {
                                        /*found an include. set the file pending to await for a filename*/
                                        file_pending = true;
                                        hdr_entity_free(entity);
                                    }
                                    if ((res != dx_equal) && (file_pending == true))
                                    {
                                        /*first check if the entity is a hydra+ string*/
                                        if (entity->entity->stringa[0] != entity->entity->stringa[entity->entity->bcount - 1])
                                        {
                                            printf("**System : Fatal Error -> The Loader determined that the [include] is not accompanied by a valid string (file name)  . Start line : %d  line : %d last character position : %d Entity : %s\n", instr_line, line_number, char_pos, entity->entity->stringa);
                                            free(tr);
                                            hdr_entity_free(entity);
                                            if (instr != NULL) free(instr);
                                            return false;
                                        }
                                        else
                                            if ((entity->entity->stringa[0] != '`') && (entity->entity->stringa[0] != '"'))
                                            {
                                                printf("**System : Fatal Error -> The Loader determined that the [include] is not accompanied by a valid string (file name)  . Start line : %d  line : %d last character position : %d Entity : %s\n",instr_line, line_number, char_pos, entity->entity->stringa);
                                                free(tr);
                                                hdr_entity_free(entity);
                                                if (instr != NULL) free(instr);
                                                return false;
                                            }
                                            else
                                            {
                                                /*Nice ! Now to load the actual file into the loader*/
                                                char* nfile = (char*)malloc(entity->entity->bcount + 1);
                                                /*2 bytes will be unused but i prefer to be consistent with the memory allocation*/
                                                if (nfile == NULL)
                                                {
                                                    printf("**System : Fatal Error -> The Loader failed to allocate memory for the file name  . Start line : %d  line : %d last character position : %d file : %s\n", instr_line, line_number, char_pos, entity->entity->stringa);
                                                    free(tr);
                                                    hdr_entity_free(entity);
                                                    if (instr != NULL) free(instr);
                                                    return false;
                                                }
                                               
                                                memcpy(nfile, entity->entity->stringa + 1, entity->entity->bcount - 1); /*ommit the first and last characters*/
                                                nfile[entity->entity->bcount - 2] = 0;
                                                /*Check for the special strings
                                                  [%main_script_dir%]
                                                  [%hydra_install_dir%]
                                                  only one of them can be in the string in the same time
                                                */
                                                char* nnfile = nfile;
                                                if (dxFindStringANSI(nfile, 0, "%main_script_dir%") != -1)
                                                    nnfile = dxReplaceStringANSI(nfile, "%main_script_dir%", loader->main_path->stringa,false) ;
                                                else
                                                    if (dxFindStringANSI(nfile, 0, "%hydra_install_dir%") != -1)
                                                        nnfile = dxReplaceStringANSI(nfile, "%hydra_install_dir%", loader->exe_path->stringa, false);
                                                
                                                /*add the file to the list*/
                                                dx_stringlist_add_raw_string(include_list, nnfile) ;
                                                /*update the index*/
                                                *main_indx = indx ; 
                                                if (nfile != nnfile) free(nnfile);
                                                free(nfile);
                                                file_pending = false;
                                                hdr_entity_free(entity);

                                            }

                                    }
                                       
                                }
                    } //retrieve entities


                }/*if the instruction is a normal instruction*/
        }
        if (instr != NULL) free(instr);
        free(tr);
        if (ERROR_CODE == HDR_SCRIPT_END)
        {
            /*exit the parsing*/
            break;
        }
    }/*while true*/

    *main_indx = indx ;
    return true;
} /*func end*/


DXLONG64 hdr_is_self_executed()
{
    /* 
     The function check the last 8 bytes of the hydra executable. 
     if the 8 bytes are the [HYDRAEXE] then the hydra has an embedded
     script to run and does not run any other script.
     Return -1 in error 0 if hydra+ its not self executed or 1 if its is
    */

    /*get the filename of this executable*/
#ifdef _WIN32

    /*paths larger than 8190 characters is not supported*/
    LPWSTR fname = (LPWSTR)malloc(8192*sizeof(WCHAR));
    DWORD ret = GetModuleFileName(NULL,fname,8192);
    if(ret == 8192) 
    {
        free(fname);
        printf("***Fatal Error. The GetModuleFileName failed.\n");
        return -1 ;
    }
    /*open the file and read the last 8 bytes*/
    HANDLE hdrf = CreateFile(fname,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL ,NULL);
    if(hdrf == INVALID_HANDLE_VALUE)
    {
        free(fname);
        printf("***Fatal Error. The CreateFile failed.\n");
        return -1 ;
    }

    /*read the bytes*/
    char sign[9] ;
    sign[8] = 0 ;
    DWORD btc = 0 ;
    SetFilePointer(hdrf,-8,NULL,FILE_END);
   if(ReadFile(hdrf,sign,8,&btc,NULL) == false) 
   {
     CloseHandle(hdrf) ;
     free(fname);
     return -1 ;
   }
    CloseHandle(hdrf) ;
    free(fname);
    /*check the bytes*/
    if((sign[0]=='H')&&(sign[1]=='Y')&&(sign[2]=='D')&&(sign[3]=='R')&&
        (sign[4]=='A')&&(sign[5]=='E')&&(sign[6]=='X')&&(sign[7]=='E')) return 1 ;

#endif

#ifdef _LINUX
  
  Dl_info  dlinfo ; 
  if (dladdr(_hdr_dummy, &dlinfo) == 0)
  {
      printf("***Fatal Error. hdr_is_self_executed -> The dladdr failed. Error : %s\n",dlerror());
      return -1 ;
  }
  
  char *fname = dlinfo.dli_fname;
  
  FILE *hdrf = fopen(fname,"rb"); 
  if(hdrf == NULL)
  {
      printf("***Fatal Error. The fopen() failed.\n");
      return -1 ;
  }

  /*read the last 8 bytes*/
  char sign[9] ;
  sign[8] = 0  ;
  fseek(hdrf,-8,SEEK_END) ;
  fread(sign,8,1,hdrf)    ;
  fclose(hdrf);

   if((sign[0]=='H')&&(sign[1]=='Y')&&(sign[2]=='D')&&(sign[3]=='R')&&
        (sign[4]=='A')&&(sign[5]=='E')&&(sign[6]=='X')&&(sign[7]=='E')) return 1 ;

#endif


    return 0 ;/*is not a self executable script*/
}


FILE *hdr_open_self()
{
    /* 
     The function opens this executable file and returns it as an open file 
    */

    /*get the filename of this executable*/
#ifdef _WIN32

    /*paths larger than 8190 characters is not supported*/
    LPWSTR fname = (LPWSTR)malloc(8192*sizeof(WCHAR));
    DWORD ret = GetModuleFileName(NULL,fname,8192);
    if(ret == 8192) 
    {
        free(fname);
        printf("***Fatal Error. hdr_open_self() -> The GetModuleFileName failed.\n");
        return NULL ;
    }

    PDX_STRING str = dx_string_create_bW(fname) ;
    PDX_STRING ufn = dx_string_convertU(str) ;
    FILE *hdrf = fopen(ufn->stringa,"rb"); 
    dx_string_free(str) ;
    dx_string_free(ufn) ;
    return hdrf ;
#endif

#ifdef _LINUX
  
  Dl_info  dlinfo ; 
  if (dladdr(_hdr_dummy, &dlinfo) == 0)  
  {
      printf("***Fatal Error. hdr_open_self() -> The hdr_open_self() -> dladdr failed. Error : %s\n",dlerror());
      return NULL ;
  }
  
  char *fname = dlinfo.dli_fname;

  FILE *hdrf = fopen(fname,"rb"); 
  return hdrf ;
 
#endif

}


uint32_t hdr_decode_char_to_uint32(unsigned char buffer[4])
{
    return ((uint32_t)buffer[3] << 24) | ((uint32_t)buffer[2] << 16) | ((uint32_t)buffer[1] << 8) | (uint32_t)buffer[0];
}

PHDR_RAW_BUF hdr_load_stored_script()
{
 /*
   the function read the uint32_t value that begins from the filesize -12
   this is the script size in bytes.
   Then reads this bytes and convert them to a PHDR_RAW_BUF
 */

 FILE *f = hdr_open_self();
 uint32_t ssize = 0       ;
 fseek(f,-12,SEEK_END)    ;
 char intbytes[4] ;
 fread(intbytes,4,1,f);

 ssize = hdr_decode_char_to_uint32(intbytes);

 if(ssize == 0 ) return NULL ;

 PHDR_RAW_BUF rawb = (PHDR_RAW_BUF)malloc(sizeof(struct hdr_raw_buf)) ;
 rawb->buf = (char*)malloc(ssize+1) ;
 rawb->buf[ssize] = 0     ;
 rawb->len        = ssize ;
 /*go to the position*/
 long tsize = ssize ;
 tsize = 0 - tsize ;
 fseek(f,(-12)+tsize,SEEK_END)    ;
 fread(rawb->buf,ssize,1,f);    
 fclose(f) ;

 return rawb ;
}

bool hdr_create_executable(char *script_name)
{
    /*
     the function copy this executable and adds the script (compiled and obfuscated) to the 
     new copy. Adds the script size and the signature HYDRAEXE in the end.
    */

     FILE *f = hdr_open_self();
     if(f == NULL) return false ;

     DXLONG64 hdrs = dx_GetFileSize(f);
     /*open the script*/
     FILE *script = fopen(script_name,"rb") ;
     if(script == NULL)
     {
       printf("*Fatal Error. The script file failed to open. File : %s\n",script_name);
       return false ;
     }

     uint32_t scrs = dx_GetFileSize(script)  ;
     DXLONG64 tot_size = hdrs + scrs + 4 + 8 ; /*4 for the uint32_t fir the size and 8 for the signature*/
     char * buff = (char*)malloc(tot_size)   ;
     char *buffindx = buff                   ;

     /*read the hydra+ bytes*/
     DXLONG64 bcnt = fread(buffindx,hdrs,1,f) ;
     buffindx = &(buff[hdrs]) ;

     /*read the scripts bytes*/
     bcnt = fread(buffindx,scrs,1,script) ;
     buffindx = &(buff[hdrs+scrs]) ;
     /*add the scriptsize*/
     char *ss = (char*)(&scrs) ;
     *buffindx = ss[0] ;
     buffindx++;
     *buffindx = ss[1] ;
     buffindx++ ;
     *buffindx = ss[2] ;
     buffindx++;
     *buffindx = ss[3] ;
     buffindx++;

     /*add the signature*/
     *buffindx = 'H' ;
     buffindx++;
     *buffindx = 'Y' ;
     buffindx++;
     *buffindx = 'D' ;
     buffindx++;
     *buffindx = 'R' ;
     buffindx++;
     *buffindx = 'A' ;
     buffindx++;
     *buffindx = 'E' ;
     buffindx++;
     *buffindx = 'X' ;
     buffindx++;
     *buffindx = 'E' ;
     buffindx++;

     /*save the file*/
     dx_string_createU(NULL,"") ;
     PDX_STRING scrname = dx_string_create_bU(script_name) ;
     PDX_STRING ext     = dx_string_createU(NULL,".exe")   ; 
     PDX_STRING newname = dx_string_concat(scrname,ext)    ;

     dx_string_free(scrname) ;
     dx_string_free(ext)     ;
     fclose(f)      ;
     fclose(script) ;

     /*open the new file*/

     f = fopen(newname->stringa,"wb");
     if(f == NULL)
     {
       printf("*Fatal Error. The new file was not created. File : %s\n",newname->stringa);
       dx_string_free(newname) ; 
       return false ;
     }

     dx_string_free(newname)     ;

     if(fwrite(buff,tot_size,1,f) != 1) 
     {
       printf("*Fatal Error. The new file is not valid. Error in fwrite(). File : %s\n",newname->stringa);
       fclose(f) ; 
       return false ;
     }
     fclose(f);


     return true ;
}





















