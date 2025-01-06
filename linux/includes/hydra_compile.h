
#define HYDRA_COMPILE


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











