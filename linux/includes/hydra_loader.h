#define HYDRA_LOADER

#ifndef ALG_PARSE
#include "alg_parse.h"
#endif

#ifndef HYDRA_STRUCTS
 #include "hydra_structs.h"
#endif

#ifndef STRING_PARSE
#include "string_parse.h"
#endif

#ifndef HYDRA_EXPRESS
#include "hydra_express.h"
#endif

#ifndef HYDRA_BOOL
#include "hydra_bool.h"
#endif

/*
  This module implements the loader / parser for the Hydra+.
  Loader actions sequence : 

  1. Loads the main script. 
  2. Check if the script is obfuscated.
  3. If the script is obfuscated then the loader deobfuscated.
  4. Checks if there is any includes (the include(s) directive MUST be in the begining of the main script)
  5. If there is includes , the loader loads the scripts.
  6. For every one of the included script the loader checks for obfuscation
  7. Deobfuscate and concatenate apropriatelly the plain text of the scripts
  8. Parse the text and create the appropriate structs for the execution of the script 

  To understand if a script is obfuscated or not the loader checks the first 5 bytes of the file.
  The signature is the bytes 01 02 03 01 01 If this signature exists then it is trimmed and the 
  deobfuscation starts.

  24-06-2024 * A small change in the loader , for supporting the Gloabal Variables desired schema
  (every $__somevariable variable that is declared in the main body of the script will be visible to all the 
   functions and objects of the script and the $__ prefix is forbidden to the variables that are declared in 
   other parts of the script)
   The loader will have the ability to discern if this prefix is used in the main script body or not and will
   throw an error for the declarations , and it will not add this variables in the local scope.
   The change will be done in the [hdr_loader_create_variable] function.
   Keep in mind that ALL the globals must be declared in the top of the script ELSE the functiona and object that are declared before the 
   global will be not able to see them. I can change this ofcourse but i believe is better to have the globals
   all in one section of the script. 


  
  Hydra+
  Header file module for the loader
  Nikos Mourgis deus-ex.gr 2024
  Live Long And Prosper
    
  */

#define LOADER_TEXT_BUFFER_SIZE 102400 // 100 KB
#define OBFUSC_KEY "Th1sIS!jusAS#Imp1eKeyTo_ObfuScat4The*CodeTobeDifficultT0Read"



typedef struct hdr_raw_buf
{
    char* buf;
    DXLONG64 len;
} *PHDR_RAW_BUF;

PHDR_RAW_BUF hdr_raw_buf_free(PHDR_RAW_BUF buff)
{
    if (buff == NULL) return NULL;
    free(buff->buf);
    free(buff);
    return NULL;
}

/*********LOADER*************************************************************************/

char* hdr_loader_return_decrypt_5bytes_sign()
{
    char* sign = (char*)malloc(6);
    sign[0] = 01;
    sign[1] = 02;
    sign[2] = 03;
    sign[3] = 01;
    sign[4] = 01;
    sign[5] = 0; /*not used just a habit*/

    return sign;
}

bool hdr_loader_file_is_encrypted(PHDR_RAW_BUF text)
{
    /*
     An encrypted Hydra+ script will have the binary
    */
    if (text == NULL)       return false;
    if (text->buf == NULL)  return false;
    if (text->len <= 4)      return false;
    char* sign = hdr_loader_return_decrypt_5bytes_sign();

    /*check if the signature is there*/
    if ((text->buf[0] == sign[0]) && (text->buf[1] == sign[1]) && (text->buf[2] == sign[2]) && (text->buf[3] == sign[03])
        && (text->buf[4] == sign[4]))
    {
        free(sign);
        return true;
    }

    free(sign);
    return false;
}


char* hdr_loader_obf_text(char* text, char* key)
{
    /*
      The function gets a plain text and creates a text that is not recognizable as a script.
      A small obstacle and protection. If you want something more sofisticate
      you are welcome to create a new function to replace this one !
      The function CHANGES the actual text does not copy it , and return it
    */
    DXLONG64 len = strlen(text);
    DXLONG64 klen = strlen(key);
    for (DXLONG64 i = 0; i < len; ++i)
    {
        text[i] = text[i] ^ key[i % klen];
    }

    return text;
}

char *hdr_loader_deobf_text(char *data,DXLONG64 size,char* key)
{
    /*
     The function restore the data from an encrypted state. 
     Actually the only difference from the hdr_loader_obf_text
     is that as the encrypted text is raw binary , the 0 value is very
     likely to exist in the data , so we need the actual data length
     for the decryption

     The binary signature of the first 5 bytes it will be transform to spaces that are neutral to the script

    */
    DXLONG64 len = size;
    DXLONG64 klen = strlen(key);
    char * indx = data ;
    indx[0] = ' ';
    indx[1] = ' ';
    indx[2] = ' ';
    indx[3] = ' ';
    indx[4] = ' ';
    indx = indx+5;
    for (DXLONG64 i = 0; i < (len-5); ++i)
    {
        indx[i] = indx[i] ^ key[i % klen];
    }

    return data;
}

typedef PHDR_RAW_BUF PHDR_ENCRYPTED_BUF;
PHDR_ENCRYPTED_BUF hdr_loader_encrypt_buf_create(char *plain_text)
{
    /*
      the function encrypt the plain_text and return it in the form of the PHDR_ENCRYPT_BUF
      the PHDR_ENCRYPT_BUF POINTS to the plain_text (that now is encrypted in binary form) , so if you need this data DO NOT FREE IT!!!!!
    */
    if (plain_text == NULL) return NULL ;
    PHDR_ENCRYPTED_BUF ebuf = (PHDR_ENCRYPTED_BUF)malloc(sizeof(struct hdr_raw_buf));
    if (ebuf == NULL) return NULL;

    ebuf->len  = strlen(plain_text);
    ebuf->buf = hdr_loader_obf_text(plain_text,OBFUSC_KEY) ;

    return ebuf;
}

typedef PHDR_RAW_BUF PHDR_DECRYPTED_BUF;
PHDR_DECRYPTED_BUF hdr_loader_decrypt_buf_create(char *data,DXLONG64 size)
{
    /*
      the function decrypts the data (that are encrypted previously from the Hydra+ ) and return it in the form of the PHDR_DECRYPTED_BUF
      the PHDR_DECRYPTED_BUF POINTS to the data , so if you need this data DO NOT FREE IT!!!!!
      We will start the decryption after the first 5 bytes of the signature
    */
    if (data == NULL) return NULL;
    PHDR_DECRYPTED_BUF dbuf = (PHDR_DECRYPTED_BUF)malloc(sizeof(struct hdr_raw_buf));
    if (dbuf == NULL) return NULL;

    dbuf->len = size;
    dbuf->buf = hdr_loader_deobf_text(data, size, OBFUSC_KEY) ;

    return dbuf;
}

PHDR_RAW_BUF hdr_loader_encrypt_decrypt_buf_free(PHDR_RAW_BUF eb , bool free_data)
{
    if (free_data == true) free(eb->buf);
    free(eb)    ; 
    return NULL ;
}


char *hdr_loader_load_text_from_disk(char* filename)
{
    /*load all the text from a file returns NULL if the file is empty or another problem arises*/

    PDX_LIST list = dx_list_create_list();

    FILE* f = fopen(filename, "rb") ;
    if (f == NULL) return NULL      ;
    char *buff = (char*)malloc(LOADER_TEXT_BUFFER_SIZE+1) ;
    /*load the text in a list and concatenate later to return  all the script text*/
    DXLONG64 total_size = 0;
    while (true)
    {
        DXLONG64 res = fread(buff, 1, LOADER_TEXT_BUFFER_SIZE, f) ;
        if (res < LOADER_TEXT_BUFFER_SIZE)
        {
            /*all the data is retrieved*/
            if (res <= 0) break; // no data !

            /*load the remainder bytes*/
            buff[res] = 0 ;
            PDXL_OBJECT obj = (PDXL_OBJECT)malloc(sizeof(struct dxl_object));
            obj->key = dx_string_createA(NULL, buff);
            dx_list_add_node_direct(list,obj);
            total_size = total_size + res ;
            
            break;
        }

        /*a whole buffer was loaded! Add it to the list*/
        buff[LOADER_TEXT_BUFFER_SIZE] = 0 ; //zero terminated
        PDXL_OBJECT obj = (PDXL_OBJECT)malloc(sizeof(struct dxl_object));
        obj->key = dx_string_createA(NULL, buff)  ;
        dx_list_add_node_direct(list,obj)       ;
        total_size = total_size + res           ;

    }
    
    fclose(f);

    //buffer is ready, construct it in a continue block in memory
    free(buff);
    
    if (total_size == 0) return NULL ;
    
    buff = (char*)malloc(total_size+1);

    PDXL_NODE node = list->start ;
    char *dest_indx = buff ;
    char *src_indx  = NULL ;
    while (node != NULL)
    {
        PDXL_OBJECT obj = node->object;
        src_indx = obj->key->stringa;
        while (*src_indx != 0)
        {
            *dest_indx = *src_indx ;
            dest_indx++ ;
            src_indx++  ;
        }

        src_indx = NULL;
        dx_string_free(obj->key) ;
        free(obj);
        node = node->right;
    }

    buff[total_size] = 0;
    dx_list_delete_list(list) ;
    
    return buff;
}

PHDR_RAW_BUF hdr_loader_load_script_from_disk(char* filename)
{
    /*
      load all the binary data from a hydra script file, returns 
      NULL if the file is empty or another problem arises.
      If the script is encrypted then the script will be decrypted
    */

    PDX_LIST list = dx_list_create_list();
    if (list == NULL) return NULL;
    
    FILE* f = fopen(filename, "rb");
    if (f == NULL) return NULL;
    char* buff = (char*)malloc(LOADER_TEXT_BUFFER_SIZE + 1);
    /*load the text in a list and concatenate later to return  all the script text*/
    DXLONG64 total_size = 0;
    while (true)
    {
        DXLONG64 res = fread(buff, 1, LOADER_TEXT_BUFFER_SIZE, f);
        if (res < LOADER_TEXT_BUFFER_SIZE)
        {
            /*all the data is retrieved*/
            if (res <= 0) break; // no data !

            /*load the remainder bytes*/
            buff[res] = 0;
            PDXL_OBJECT obj = (PDXL_OBJECT)malloc(sizeof(struct dxl_object));
            char* tb = (char*)malloc(res+1) ; // this buffer must be freed when the list is destroyed
            memcpy(tb, buff, res)         ; // copy the data
            tb[res] = 0;
            
            PHDR_RAW_BUF rbuf = (PHDR_RAW_BUF)malloc(sizeof(struct hdr_raw_buf));
            rbuf->buf = tb  ;
            rbuf->len = res ;
            obj->obj = rbuf  ; // the encrypted text

            dx_list_add_node_direct(list, obj); // add to the list
            total_size = total_size + res     ; 

            break;
        }

        /*a whole buffer was loaded! Add it to the list*/

        buff[res] = 0;
        PDXL_OBJECT obj = (PDXL_OBJECT)malloc(sizeof(struct dxl_object));
        char* tb = (char*)malloc(res + 1); // this buffer must be freed when the list is destroyed
        memcpy(tb, buff, res); // copy the data
        tb[res] = 0;

        PHDR_RAW_BUF rbuf = (PHDR_RAW_BUF)malloc(sizeof(struct hdr_raw_buf));
        rbuf->buf = tb  ;
        rbuf->len = res ;
        obj->obj = rbuf ; // the encrypted text

        dx_list_add_node_direct(list, obj);
        total_size = total_size + res;

    }

    fclose(f);

    //buffer is ready, construct it in a continue block in memory
    free(buff);

    if (total_size == 0) return NULL;

    buff = (char*)malloc(total_size+1);

    PDXL_NODE node = list->start;
    char* dest_indx = buff;
    while (node != NULL)
    {
        PDXL_OBJECT obj          = node->object ;
        PHDR_RAW_BUF  ebuf = obj->obj ;

        for(DXLONG64 i = 0 ; i < ebuf->len ; i++)
        {
            *dest_indx = ebuf->buf[i];
            dest_indx++;
        }

        hdr_loader_encrypt_decrypt_buf_free(ebuf,true);
        free(obj);
        node = node->right;
    }

    dx_list_delete_list(list);

    buff[total_size] = 0;

    /* check if the file is encrypted*/
    PHDR_RAW_BUF tbf = (PHDR_RAW_BUF)malloc(sizeof(struct hdr_raw_buf)) ;
    tbf->buf = buff;
    tbf->len = total_size ;
    if (hdr_loader_file_is_encrypted(tbf) == true)
    {
        free(tbf);
        PHDR_ENCRYPTED_BUF ebuf = hdr_loader_decrypt_buf_create(buff, total_size);
        ebuf->len = total_size;
        return ebuf;
    }

    return tbf;
}

enum hdr_save_to_disk_state {hdr_sd_filename_null, hdr_sd_ebuf_null, hdr_sd_ebuf_buf_null, hdr_sd_no_open,hdr_sd_success};

enum  hdr_save_to_disk_state hdr_loader_save_binary_to_disk(char* filename, PHDR_ENCRYPTED_BUF ebuf , bool add_sign)
{
    /*
      The function writes a buffer with binary data to a file to the disk.
      If the add_sign is true then the binary file will have the encrypted sign as the first 5 bytes
    */

    if (filename == NULL)  return hdr_sd_filename_null  ;
    if (ebuf == NULL)      return hdr_sd_ebuf_null      ;
    if (ebuf->buf == NULL) return hdr_sd_ebuf_buf_null  ;
   
    FILE* f = fopen(filename, "wb");
    if (f == NULL) return hdr_sd_no_open;
    
    if (add_sign == true)
    {
        char* sign = hdr_loader_return_decrypt_5bytes_sign();
        fwrite(sign, 1, 5, f);
        free(sign);
    }
    
    fwrite(ebuf->buf, 1, ebuf->len, f);
    fclose(f);

    return hdr_sd_success;
}


/* ************************************************************************************************** */
/*
  The loader will have a list with all the scripts that the 
  hydra application will use (The first script that the Hydra will load is the main script). The mechanism that the Hydra+ uses
  is the system directive [include]. The [include] directive is used to 
  instruct the loader to load a script file to the memory for "compile".
  The scripts are being loaded based of their [include] position in the main script.
  Note that the [include] directive can be positioned ONLY as the first (except if other [include] directives 
  are pre existing ) Command of the script (Comments are not actually commands so they can be put 
  before the [include])

  Note that every script is loaded as a binary file as it can be an ecrypted or not encrypted file
  and then if it is encrypted is transformed in plain text
 */


typedef PDX_LIST PHDR_TEXT_SCRIPT_LIST   ;
typedef char            *HDR_UTF8_TEXT   ; /*Hydra+ support ONLY uft8 (or plain latin ansi characters)*/

typedef struct hdr_incl_script
{
    PHDR_DECRYPTED_BUF buf          ; /*The text data of the script*/
    PDX_STRING         filename     ; /*The actual file name of the script. Used for error displaying purposes*/
} *PHDR_INCL_SCRIPT;

PHDR_INCL_SCRIPT hdr_incl_script_create(PHDR_DECRYPTED_BUF buf, char *filename) /*does not copy the buffer only point to it*/
{
    /*create an included script*/
    PHDR_INCL_SCRIPT script = malloc(sizeof(struct hdr_incl_script)) ;
    script->buf       = buf ;
    script->filename = dx_string_createU(NULL, filename) ;
    return script ;
}

PHDR_INCL_SCRIPT hdr_incl_script_free(PHDR_INCL_SCRIPT script)
{
    if (script == NULL) return NULL ;

    dx_string_free(script->filename)  ;
    hdr_loader_encrypt_decrypt_buf_free(script->buf,true) ;
    free(script)                      ;

    return NULL ;
}


enum hdr_loader_parsing_state {
    hdr_single, hdr_assignment_decl, hdr_assign_expres, hdr_if, hdr_if_instr, hdr_if_bool_expres, hdr_if_open,
    hdr_else, hdr_else_instr, hdr_else_open, hdr_func, hdr_func_params , hdr_func_open,
    hdr_obj, hdr_obj_open, hdr_loop, hdr_loop_open,
    hdr_switch, hdr_switch_expres, hdr_switch_block, hdr_switch_open, hdr_pending,hdr_error
};
/*
 The state of the loader in relation with the current instruction

 hdr_single          : A simple instruction , more possible a function call or a special command as [return]

 hdr_assignment_decl : An assignment or a declaration ,( a variable to the left and an expression to the right separated by an [=] if its an assigment)

 hdr_assign_expres   : An assigment has been detected and the loader expects to find the right part expression

 hdr_if              : An [if] command (placeholder not actuay used)

 hdr_if_bool_expres  : The instruction is an [if] that excepts the next entities until the [)] to be
                       part of its logical expression.

 hdr_if_instr        : The instruction is an [if] that excepts the next entities to be
                       part of its instruction for execution.

 hdr_if_open         : An [if] command is currently analyzed and an open curly bracket [{] was found

 hdr_else            : An [else] command (placeholder not actuay used)

 hdr_else_instr      : An [else]  is currently analyzed and the loader expects to find an expression or block

 hdr_else_open       : An [else] command is currently analyzed and an open curly bracket [{] was found

 hdr_func            : The declaration of a custom function

 hdr_func_params     : The loader has found a custom function declaration and expects to find the parameter list.
 
 hdr_func_open       : The loader has found a custom function declaration and parameters and expects to find the instruction block.

 hdr_obj             : The declaration of an object

 hdr_obj_open        : An object is currently declared and an open curly bracket [{] was found

 hdr_loop            : A [loop] command

 hdr_loop            : A [loop] command is currently analyzed and an open curly bracket[{] was found

 hdr_switch          : A [switch] command

 hdr_switch_expres   : A [switch] command is analyzed and the loader expects to find its expression

 hdr_switch_block    : A [switch] command is analyzed and the loader expects to find its block

 hdr_switch_open     : A [switch] command is currently analyzed and an open curly bracket[{] was found

 hdr_pending         : The loader has not analyzed yet a command or is between commands
*/


enum hdr_loader_last_branch_state {hdr_br_none,hdr_br_if,hdr_br_else_if,hdr_br_else};
/*
 This state is used to track the state of the brancing so to not allow 
 invalid brancing like an else if after a terminating else
*/
enum hdr_loader_section_mode {hdr_s_main_code,hdr_s_block_code,hdr_s_decl_func,hdr_s_decl_obj};
/*
 this mode is used so the loader can known if its legal to declare an object or a function 
 in the current section.
 In a function declaration you cannot declare either objects or functions,
 in an object declaration you cannot declare objects but you can declare functions.
 in the main code you can do both.
*/


typedef PDX_LIST PHDR_LOADER_OBJ_PROT_LIST;
typedef struct hdr_loader
{
    /*
     the struct for the script loader 
    */
    PHDR_TEXT_SCRIPT_LIST  scripts         ;/*the plain text scripts that are included in the mainscript (this is short lived in the script)*/
    PHDR_INCL_SCRIPT       main_script     ;/*the first script to load and the main script to execute*/
    PDX_STRING             main_path       ;/*this value will be used when the script needs the main script path*/
    PDX_STRING             main_name       ;/*this value will be used when the script needs the main script name*/
    PDX_STRING             exe_path        ;/*this value will be used when the script needs the path of the hydra+ executable*/
    PDX_STRING             curr_file_name  ;/*the current filename of the script that is analyzed.*/

    PHDR_BLOCK             code            ;/*the code of the script. This struct has ALL the code ,main script and included files*/
    PHDR_BLOCK             current_block   ;/*the current block, this changes in the loader based to the instruction (if,loop,else etc can have blocks of instructions)*/
    /*temp variables to help the loading*/
    PHDR_INSTRUCTION              current_instr         ;
    PHDR_INSTRUCTION              pending_if            ;/*the if that is pending , usefull for the 'else' instruction */
    enum hdr_loader_parsing_state state                 ;

    DXLONG64 instr_line  ; /*always starts from the first line*/
    DXLONG64 line_number ; /*accummulate the lines*/
    DXLONG64 char_pos    ; /*char position of the error*/

    enum hdr_loader_last_branch_state branch_state ;

    bool in_main_file    ; /*this is used to check for erroneus includes*/

    /*temporary functions templates*/
    PHDR_INSTRUCTIONS_LIST     functions         ;
    PHDR_INSTRUCTIONS_LIST     current_func_list ; /*this member is used for storing the function in the right list (general script versus object class)*/
    /*temporary objects declarations*/
    PHDR_LOADER_OBJ_PROT_LIST       objects          ;
    enum hdr_loader_section_mode    declaration_mode ;

} *PHDR_LOADER ;

/*the loader will need to save its internal state when enters a code block*/
typedef struct hdr_loader_state
{
    enum hdr_loader_parsing_state      state             ;
    PHDR_BLOCK                         current_block     ;
    PHDR_INSTRUCTION                   current_instr     ;
    PHDR_INSTRUCTION                   pending_if        ;
    enum hdr_loader_last_branch_state  branch_state      ;
    enum hdr_loader_section_mode       declaration_mode  ;
} *PHDR_LOADER_STATE;


/*
 The following struct is used from the loader to store the object declarations
 to a compatible with the loader's structs form  
 */

typedef struct hdr_loader_obj_prot
{
    PHDR_INSTRUCTION instr           ; /*the variable declarations the assigment instructions and the construction code is stored in the code of the instruction*/
    PHDR_INSTRUCTIONS_LIST functions ;
    PDX_STRING name                  ; /*the object prototype name*/
} *PHDR_LOADER_OBJ_PROT ;

PHDR_LOADER_OBJ_PROT hdr_loader_obj_prot_create()
{
    PHDR_LOADER_OBJ_PROT obj = (PHDR_LOADER_OBJ_PROT)malloc(sizeof(struct hdr_loader_obj_prot));
    if (obj == NULL) return NULL                    ;
    obj->instr       = hdr_instruction_create()     ;
    obj->instr->code = hdr_block_create(NULL)       ;
    obj->functions = hdr_instructions_list_create() ;
    obj->name = dx_string_createU(NULL, "")         ;

    return obj ;
}

PHDR_LOADER_OBJ_PROT hdr_loader_obj_prot_free(PHDR_LOADER_OBJ_PROT obj)
{
    hdr_instruction_free(obj->instr)            ;
    hdr_instructions_list_free(obj->functions)  ;
    dx_string_free(obj->name)                   ;
    free(obj)                                   ;
    return NULL                                 ;
}

PHDR_LOADER_OBJ_PROT hdr_loader_obj_prot_list_add(PHDR_LOADER_OBJ_PROT_LIST list, PHDR_LOADER_OBJ_PROT class)
{

    if ((list == NULL) || (class == NULL)) return NULL;
    PDXL_OBJECT obj = (PDXL_OBJECT)malloc(sizeof(struct dxl_object));
    if (obj == NULL) return NULL;
    obj->obj = class;
    if (dx_list_add_node_direct(list, obj) == NULL)
    {
        free(obj);
        return NULL;
    }
    return class;
}

PHDR_LOADER_OBJ_PROT_LIST hdr_loader_obj_prot_list_create()
{
    return dx_list_create_list();
}

PHDR_LOADER_OBJ_PROT_LIST hdr_loader_obj_prot_list_free(PHDR_LOADER_OBJ_PROT_LIST list )
{
    
    PDXL_NODE node = list->start ;
    while (node != NULL)
    {
        PDXL_OBJECT obj             = node->object;
        PHDR_LOADER_OBJ_PROT class  = (PHDR_LOADER_OBJ_PROT)obj->obj;
        hdr_loader_obj_prot_free(class);
        free(obj);
        node = node->right;
    }

    dx_list_delete_list(list) ;

    return NULL ;
}



/****************************************************************************/

/************************************************/
int hdr_loader_create_code_from_string(PHDR_LOADER loader, char* text);
/***********************************************/

PHDR_LOADER_STATE hdr_loader_state_store(PHDR_LOADER loader)
{
    /*
     creates a new hdr_loader_state struct and stores the loader state
    */
    if (loader == NULL) return NULL;
    PHDR_LOADER_STATE state = (PHDR_LOADER_STATE)malloc(sizeof(struct hdr_loader_state));
    if (state == NULL) return NULL;

    state->current_block    = loader->current_block;
    state->current_instr    = loader->current_instr;
    state->pending_if       = loader->pending_if   ;
    state->state            = loader->state        ;
    state->branch_state     = loader->branch_state ;
    state->declaration_mode = loader->declaration_mode ;
    return state;

}

PHDR_LOADER_STATE hdr_loader_state_restore(PHDR_LOADER loader, PHDR_LOADER_STATE state)
{
    /*
     restore the [state] into the [loader].
     frees the state and returns NULL ;
    */

    if ( state == NULL) return NULL;
    if (loader == NULL)
    {
        free(state);
        return NULL;
    }

    loader->current_block = state->current_block;
    loader->current_instr = state->current_instr;
    loader->pending_if    = state->pending_if   ;
    loader->state         = state->state        ;
    loader->branch_state  = state->branch_state ;
    loader->declaration_mode = state->declaration_mode;
    free(state);

    return NULL;
}

/****************************************************************************************************/
/*file / include system implementation*/

/*
 create the keywords that we will need for the Hydra+ in PDX_STRING form
*/

PDX_STRING hdr_k_include = NULL;
/*keywords of the syntax for the Hydra+.*/
/*include,if,else,switch,loop,exit,return,break,continue,func,true,false,pause,async,obj,warnings*/
char* keywords = "includeifelseswitchloopexitreturnbreakcontinuefunctruefalsepauseasyncobjdwarnings";

void hdr_loader_initialize()
{
    hdr_k_include = dx_string_createU(NULL, "include") ;
    return ;
}

void hdr_loader_uninitialize()
{
    dx_string_free(hdr_k_include);
    return ;
}
/*****************************************************/



PHDR_LOADER hdr_loader_create(char *exepath,PHDR_BLOCK parent_block)
{
    /*creates and returns a loader*/
    PHDR_LOADER loader = (PHDR_LOADER)malloc(sizeof(struct hdr_loader)) ;
    if (loader == NULL) return NULL;
    
    loader->main_script   = NULL  ;
    char* bf = dxExtractPathFromFilename(exepath)      ;
    loader->exe_path      = dx_string_createU(NULL,bf) ;
    free(bf);
    loader->main_name     = dx_string_createU(NULL, "");
    loader->main_path     = dx_string_createU(NULL, "");

    loader->current_instr = NULL  ; 
    loader->pending_if    = NULL  ;
    loader->state = hdr_pending   ;
    loader->branch_state = hdr_br_none ;

    loader->scripts = dx_list_create_list();
    if (loader->scripts == NULL)
    {
        free(loader) ;
        return NULL  ;
    }

    loader->code  = hdr_block_create(parent_block);
    if (loader->code == NULL)
    {
        dx_list_delete_list(loader->scripts);
        free(loader);
        return NULL;
    }

    loader->functions = hdr_instructions_list_create();
    if (loader->functions == NULL)
    {
        dx_list_delete_list(loader->scripts)    ;
        hdr_block_free(loader->code)            ;
        free(loader);
        return NULL;
    }
    loader->current_func_list = loader->functions ;

    loader->objects = hdr_loader_obj_prot_list_create() ;
    if (loader->objects == NULL)
    {
        dx_list_delete_list(loader->scripts);
        hdr_block_free(loader->code);
        hdr_instructions_list_free(loader->functions) ;
        free(loader);
        return NULL;
    }

    loader->current_block    = loader->code  ;
    loader->in_main_file     = false         ;
    loader->declaration_mode = hdr_s_main_code;
    hdr_loader_initialize();
    return loader;
}

PHDR_LOADER hdr_loader_free(PHDR_LOADER loader)
{
    /*frees all the memory of the loader and returns NULL*/
    if (loader == NULL) return NULL;
    if (loader->scripts != NULL)
    {
        PDXL_NODE node = loader->scripts->start;
        while (node != NULL)
        {
            PDXL_OBJECT   obj = node->object;
            PHDR_INCL_SCRIPT  script = obj->obj;
            hdr_incl_script_free(script);
            free(obj);
            node = node->right;
        }
    }

    PDXL_NODE node = loader->functions->start;
    while (node != NULL)
    {
        PDXL_OBJECT       obj  = node->object;
        PHDR_INSTRUCTION  func = obj->obj;
        hdr_instruction_free(func);
        free(obj);
        node = node->right;
    }


    dx_string_free(loader->exe_path)  ;
    dx_string_free(loader->main_name) ;
    dx_string_free(loader->main_path) ;

    if(loader->scripts != NULL)
    dx_list_delete_list(loader->scripts) ;
    dx_list_delete_list(loader->functions);
    hdr_loader_obj_prot_list_free(loader->objects);

    hdr_incl_script_free(loader->main_script);

    hdr_block_free(loader->code)  ;

    hdr_loader_uninitialize();
    free(loader);

    return NULL ;
}

PHDR_VAR hdr_loader_create_variable(PHDR_LOADER loader, char* name,bool *error)
{
    
    if (name == NULL) return NULL ;
    if (strlen(name) <= 1) return NULL;

    *error = false ;
    /*check for the special global variables with the prefix $__*/
     if((strlen(name)>3)&&(loader->declaration_mode !=  hdr_s_main_code))
     {
      if((name[1]=='_')&&(name[2]=='_'))
      {
        /*
         this is a global , we have to check the main script code if this global exists. if it does we just
         ignore it and we will not add it to the local block. If does not exists we will throw an error 
         as a global cannot be declared in a block other than the main block.
        */
        /*create a temprary dx_string*/
        PDX_STRING tmp = dx_string_createU(NULL,name) ;
        if (hdr_var_list_find_var(loader->code->variables, tmp) == NULL)
        {
          dx_string_free(tmp);
          *error = true ;
          printf("The global [%s] cannot be declared in a block other than the main code block of the script. All the globals MUST be declared in the main code block.\n",name);
          return NULL ;
        }
        dx_string_free(tmp);
        /*the global exists in the main block so we silently ommit it*/
        return NULL ;

      }
     
     }

    if (hdr_parser_check_name((name+1)) == false) return NULL ; /*+1 to check from the character after the $*/
    PHDR_VAR var = hdr_var_create(NULL, name, hvf_block, NULL) ; /*it will inherit the block that will be inserted too*/
    hdr_var_list_add_inherit_block(loader->current_block->variables, var);
    if (var == NULL) *error = true ;
    return var ; /*success*/
}

bool hdr_loader_load_main_script(PHDR_LOADER loader, char* filename)
{
    if (loader == NULL) return false ;
    PHDR_DECRYPTED_BUF mains = hdr_loader_load_script_from_disk(filename) ; 
    if (mains == NULL) return false;

    loader->main_script = hdr_incl_script_create(mains, filename);
    char* bf = dxExtractPathFromFilename(filename);
    loader->main_path = dx_string_createU(loader->main_path,bf) ;
    free(bf) ;
    bf = dxExtractFilename(filename);
    loader->main_name = dx_string_createU(loader->main_name, bf);
    free(bf);
    return true ;
}

bool hdr_loader_add_script(PHDR_LOADER loader, char * filename)
{
    PHDR_DECRYPTED_BUF script = hdr_loader_load_script_from_disk(filename);
    if (script == NULL) return false ;
    /*add the script in the loader script list*/
    PDXL_OBJECT obj = (PDXL_OBJECT)malloc(sizeof(struct dxl_object));
    if (obj == NULL)
    {
        hdr_loader_encrypt_decrypt_buf_free(script, true);
        return false;
    }
    PHDR_INCL_SCRIPT iscr = hdr_incl_script_create(script, filename);
    obj->obj = iscr;
    dx_list_add_node_direct(loader->scripts, obj);
    return true ;
}

bool hdr_loader_load_include_files(PHDR_LOADER loader)
{
    if (loader == NULL)
    {
        printf("Fatal Error : The loader is NULL");
        return false;
    }

    /*
      parse the main file and find the include directives.
      We will check if the include directives are in the top of the file.
      If the parser finds any other instruction then the loading of the files is terminated and
      the loader assumes that there is no more files for inclusion.
      Keep in mind that the files are loaded by the showing order.
      The [include] directive can use two special keywords in the filename string 
      that are unique to the [include]. The [%main_script_dir%] that returns the 
      path component of the full main script filename , and the  [%hydra_install_dir%]
      that returns the path component of the hydra executable full name
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
                                                //printf("%s\n",nnfile);
                                                if (hdr_loader_add_script(loader, nnfile) == false)
                                                {
                                                    printf("**System : Fatal Error -> The Loader failed to load the included file  . Start line : %d  line : %d last character position : %d file : %s\n", instr_line, line_number, char_pos, nnfile);
                                                    free(tr);
                                                    hdr_entity_free(entity);
                                                    if (nfile != nnfile) free(nnfile);
                                                    free(nfile);
                                                    if (instr != NULL) free(instr);
                                                    return false;
                                                }

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


    return true;
} /*func end*/



/************** load the code of the scripts ***************************/




/******************helper functions*/

PHDR_ENTITY hdr_loader_get_entity( DXLONG64 cline,char** instr_index, bool *fatal_error)
{

    /*
      retrieve the next entity and makes all the preliminary validity checks. 
      Any error in this function is a fatal error.
      The return value is a valid entity or the NULL value

      The instr_index will be point to the next character for analyze

    */

       /*retrieve entity*/
        int status = 0;
        PHDR_ENTITY entity = hdr_parser_get_next_entity(instr_index,&status);
        
        if (entity == NULL)
        {
            printf("**System : Fatal Error -> The Loader failed to allocate memory for the entity. Line : %d Instruction : %s\n\n",
                cline, *instr_index);
            *fatal_error = true ;
            return NULL;
        }
        
        if (status != HDR_SUCCESS)
        {
            
            printf("**System : Fatal Error -> Line : %d %s : %s\n\n",
                cline, hdr_error_code_to_str(status),*instr_index);
            hdr_entity_free(entity) ; 
            *fatal_error = true ;
            return NULL;
        }
        
        if (entity->too_long_entity == true)
        {
            printf("**System : Fatal Error -> The Loader found a very large entity. Line : %d Entity : %s\n",  cline,  entity->entity->stringa);
            hdr_entity_free(entity);
            *fatal_error = true ;
            return NULL;
        }
        else
            if ((entity->entity->len == 0) && (entity->separator->len == 0))
            {
                /*the end of the instruction*/
                hdr_entity_free(entity);
                return NULL ;
            }
          
        return entity;
}



int hdr_loader_check_state(enum hdr_loader_parsing_state loader_state, DXLONG64 instr_line, PDX_STRING prev_i)
{

    if (loader_state == hdr_assign_expres)
    {
        printf("Fatal Error -> Line : %d The instruction %s needs an expression to be assigned \n", instr_line, prev_i->stringa);
        return HDR_ERROR_LOADER_EXPR_MISSING;
    }

    if (loader_state == hdr_if_bool_expres)
    {
        printf("Fatal Error -> Line : %d The [if] (%s) needs a boolean expresion for it to be valid. \n", instr_line, prev_i->stringa);
        return HDR_ERROR_LOADER_EXPR_MISSING;
    }

    if (loader_state == hdr_if_instr)
    {
        printf("Fatal Error -> Line : %d The [if] (%s) needs an expresion or instruction to execute for it to be valid. \n", instr_line, prev_i->stringa);
        return HDR_ERROR_LOADER_EXPR_MISSING;
    }

    if (loader_state == hdr_else_instr)
    {
        printf("Fatal Error -> Line : %d The [else] (%s) needs an expresion or instruction to execute for it to be valid. \n", instr_line, prev_i->stringa);
        return HDR_ERROR_LOADER_EXPR_MISSING;
    }

    if (loader_state == hdr_switch_expres)
    {
        printf("Fatal Error -> Line : %d The [switch] (%s) needs an expression that returns a value for it to be valid. \n", instr_line, prev_i->stringa);
        return HDR_ERROR_LOADER_EXPR_MISSING;
    }

    return HDR_SUCCESS;

}

/*the function set the loader state based of the previous state and the new entity*/
int hdr_loader_set_state(char * instr,PHDR_ENTITY entity, PHDR_LOADER loader)
{

    char* indx = entity->entity->stringa;
    char* token = hdr_parser_get_next_token(&indx) ;

    if (token == NULL)
    {
        free(token);
        return HDR_ERROR_LOADER_TOKEN_IS_NULL;
    }

    if (loader->state == hdr_assign_expres)
    {
        /*the loader expects an expresion for the right side of the assigment , do nothing and return!*/    
        goto exit  ;
    }

    if (token[0] == '$')
    {
        /*check if the state is not in a pending state, the if token commands a new instruction*/
        if (loader->state != hdr_pending) return HDR_ERROR_LOADER_NEW_INSTR_WHILE_PENDING;
        /*Ok setup a new assigment/variable declaration!*/
        if (dxCharExistsInStr(instr, '=', "`\"") == -1)
        {
            loader->state = hdr_single;
            goto exit;
        }
        /*set the state to the assigment or declaration */
        loader->state = hdr_assignment_decl ;
        goto exit;
    }

    /*check for token type and set the state*/
    if (dxIsStrEqual("if", token) == true)
    {
        /*check if the state is not in a pending state, the if token commands a new instruction*/
        if (loader->state != hdr_pending) return HDR_ERROR_LOADER_NEW_INSTR_WHILE_PENDING;
        /*Ok setup a new if!*/
        loader->state = hdr_if_bool_expres ;
        goto exit;
    }

    if (dxIsStrEqual("loop", token) == true)
    {
        /*check if the state is not in a pending state, the if token commands a new instruction*/
        if (loader->state != hdr_pending) return HDR_ERROR_LOADER_NEW_INSTR_WHILE_PENDING;
        /*Ok setup a new loop!*/
        loader->state = hdr_loop ;
        goto exit;
    }

    if (dxIsStrEqual("else", token) == true)
    {
        /*check if the state is not in a pending state, the if token commands a new instruction*/
        if (loader->state != hdr_pending) return HDR_ERROR_LOADER_NEW_INSTR_WHILE_PENDING;
        /*Ok setup a new else!*/
        loader->state = hdr_else_instr;
        goto exit;
    }

    if (dxIsStrEqual("switch", token) == true)
    {
        /*check if the state is not in a pending state, the if token commands a new instruction*/
        if (loader->state != hdr_pending) return HDR_ERROR_LOADER_NEW_INSTR_WHILE_PENDING;
        /*Ok setup a new else!*/
        loader->state = hdr_switch ;
        goto exit;
    }

    if (dxIsStrEqual("func", token) == true)
    {
        /*check if the state is not in a pending state, the if token commands a new instruction*/
        if (loader->state != hdr_pending) return HDR_ERROR_LOADER_NEW_INSTR_WHILE_PENDING;
        /*Ok setup a new function!*/
        loader->state = hdr_func;
        goto exit;
    }

    if (dxIsStrEqual("obj", token) == true)
    {
        /*check if the state is not in a pending state, the if token commands a new instruction*/
        if (loader->state != hdr_pending) return HDR_ERROR_LOADER_NEW_INSTR_WHILE_PENDING;
        /*Ok setup a new object class!*/
        loader->state = hdr_obj ;
        goto exit;
    }

   loader->state = hdr_single ;

   exit :
   free(token);
   return HDR_SUCCESS;

}




PHDR_INSTRUCTION hdr_loader_h_create_asgn_i(PHDR_LOADER loader,char *var_name,bool complex,char *fullname)
{
    PHDR_INSTRUCTION instr = hdr_instruction_create();
    if (instr == NULL) return NULL;
    instr->parent = loader->current_block;
    instr->type = hdr_i_assign; /*assigment*/
    instr->left_side = hdr_complex_token_create(NULL, var_name);
    instr->left_side->type = hdr_tk_variable;
    if (complex == true)
    {
        int status;
        instr->left_side->expression = hdr_expr_create_from_str(fullname, &status);
        if (status != HDR_SUCCESS)
        {
            hdr_instruction_free(instr);
            printf("Error in the expression of the complex variable. Internal Error : %s",hdr_error_code_to_str(status));
            return NULL;
        }
        /*
          Hydra+ do not permits left sides in an asigment in the form of $var+1 = ...  so we will check for the 
          returned expression ,  it MUST have only one token for parsing as we permits indexed and multipart tokens
        */
        if (instr->left_side->expression->tokens->count > 1)
        {
            hdr_instruction_free(instr);
            printf("Error in the expression of the complex variable. A complex variable can have indexes and multiple parts like $var[1][2] = 8, but syntax like $var+1 = 4 is invalid.\n");
            return NULL;
        }

        instr->left_side->type = hdr_tk_expression;
    }
    
    instr->line = loader->instr_line;
    dx_string_createU(instr->filename, loader->curr_file_name->stringa);
    loader->current_instr = instr; /*we expect the right part in the next entity retrieval*/
    if (loader->pending_if != NULL) loader->pending_if = NULL;
    /*set the new state */
    loader->state = hdr_assign_expres;
    return instr;
}


PHDR_INSTRUCTION hdr_loader_h_create_var_cmd(PHDR_LOADER loader, char *var_name,char* instr_str)
{
    PHDR_INSTRUCTION instr = hdr_instruction_create();
    if (instr == NULL) return NULL;
    instr->parent = loader->current_block;
    instr->type = hdr_i_var_cmd ;
    instr->left_side = hdr_complex_token_create(NULL, var_name);

    int status;
    /*trim the instruction*/
    dxGoForwardWhileChars(&instr_str, " \t") ;
    dxRightTrimFastChars(instr_str, " \t")   ;
    instr->left_side->expression = hdr_expr_create_from_str(instr_str, &status);
    if (status != HDR_SUCCESS)
    {
        hdr_instruction_free(instr);
        printf("Error in expression of the complex variable token. Internal Error : %s", hdr_error_code_to_str(status));
        return NULL;
    }
  
    instr->line = loader->instr_line;
    dx_string_createU(instr->filename, loader->curr_file_name->stringa);
    loader->current_instr = instr;
    if (loader->pending_if != NULL) loader->pending_if = NULL;
    /*set the state as pending we have nothing more to do */
    loader->state = hdr_pending;
    return instr;

}

PHDR_INSTRUCTION hdr_loader_h_create_if_i(PHDR_LOADER loader)
{
    PHDR_INSTRUCTION instr = hdr_instruction_create();
    if (instr == NULL) return NULL;
    instr->parent = loader->current_block;
    instr->type = hdr_i_if ; /*if*/
    /*create the code structure for the 'true' evaluation */
    instr->code = hdr_block_create(loader->current_block);
    dx_string_createU(instr->filename, loader->curr_file_name->stringa);
    instr->line = loader->instr_line;

    loader->current_instr = instr   ; /*we will fill the expression in the caller*/
    loader->pending_if    = instr   ;
    /*set the state for the pending expression */
    loader->state = hdr_if_instr ;
    return instr;
}

PHDR_INSTRUCTION hdr_loader_h_create_loop_i(PHDR_LOADER loader)
{
    PHDR_INSTRUCTION instr = hdr_instruction_create();
    if (instr == NULL) return NULL;
    instr->parent = loader->current_block;
    instr->type = hdr_i_loop; /*if*/
    /*create the code structure for the loop*/
    instr->code = hdr_block_create(loader->current_block);
    dx_string_createU(instr->filename, loader->curr_file_name->stringa);
    instr->line = loader->instr_line;

    loader->current_instr = instr; /*we will fill the expression in the caller*/
    if (loader->pending_if != NULL) loader->pending_if = NULL;
    /*set the state for the pending expression */
    loader->state = hdr_loop;
    return instr;
}


PHDR_INSTRUCTION hdr_loader_h_create_switch_i(PHDR_LOADER loader, char* expr)
{
    PHDR_INSTRUCTION instr = hdr_instruction_create();
    if (instr == NULL) return NULL;
    instr->parent = loader->current_block;
    instr->type = hdr_i_switch; /*the switch is in actuality an if with many else if but has different structure from an if so has a different type*/
    instr->line = loader->instr_line;
//    instr->code = hdr_block_create(loader->current_block);  not used
    /*the expression of the switch that will be checked against the values*/
    instr->right_side = hdr_expr_create_expr(loader->instr_line, expr);
    if (instr->right_side == NULL)
    {
        hdr_instruction_free(instr);
        return NULL;
    }

    dx_string_createU(instr->filename, loader->curr_file_name->stringa);
    loader->current_instr = instr; /*we will fill the expression in the caller*/
    if (loader->pending_if != NULL) loader->pending_if = NULL ;
    /*set the state for the block */
    loader->state = hdr_switch_block;
    return instr;
}

PHDR_INSTRUCTION hdr_loader_h_create_single_token_i(PHDR_LOADER loader, char* token)
{
    PHDR_INSTRUCTION instr = hdr_instruction_create();
    if (instr == NULL) return NULL;
    instr->parent           = loader->current_block ;
    instr->type             = hdr_i_cmd             ; /*a command*/
    instr->left_side        = NULL                  ;
    instr->line = loader->instr_line                ;
    dx_string_createU(instr->filename, loader->curr_file_name->stringa);
    int status;
    instr->right_side = hdr_expr_create_from_str(token, &status);
    if (status != HDR_SUCCESS)
    {
        hdr_instruction_free(instr);
        printf("Error in analyzing the expression : %s . Internal Error : %s",token,hdr_error_code_to_str(status));
        return NULL;
    }

    loader->current_instr = instr; /*we expect the right part in the next entity retrieval*/
    if (loader->pending_if != NULL) loader->pending_if = NULL;
    /*set the state for the pending expression */
    loader->state = hdr_pending ; /*this is handed in one cycle*/
    return instr;
}

PHDR_INSTRUCTION hdr_loader_h_create_param_token_i(PHDR_LOADER loader, char* word , char *param)
{
    if (word == NULL)
    {
        printf("The word token is NULL\n");
        return NULL;
    }
    if (param == NULL)
    {
        printf("The param token is NULL\n");
        return NULL;
    }

    /*check if the word is a variable. A variable cannot have any parameters*/
    if (word[0] == '$')
    {
        printf("The variable '%s' (and general the variables) cannot have parameters.\n",word);
        return NULL;
    }

    PHDR_INSTRUCTION instr = hdr_instruction_create();
    if (instr == NULL) return NULL;
    instr->parent = loader->current_block;
    instr->type = hdr_i_cmd_param ; /*a command*/
    int status;
    instr->left_side = hdr_token_create_simple_token(word,&status);
    if (status != HDR_SUCCESS)
    {
        hdr_instruction_free(instr);
        printf("Error in creating the token : %s . Internal Error : %s", word, hdr_error_code_to_str(status));
        return NULL;
    }

    instr->line = loader->instr_line;
    dx_string_createU(instr->filename, loader->curr_file_name->stringa);
    instr->right_side = hdr_expr_create_from_str(param, &status);
    if (status != HDR_SUCCESS)
    {
        hdr_instruction_free(instr);
        printf("Error in analyzing the expression : %s . Internal Error : %s", param, hdr_error_code_to_str(status));
        return NULL;
    }

    loader->current_instr = instr; /*we expect the right part in the next entity retrieval*/
    if (loader->pending_if != NULL) loader->pending_if = NULL;
    /*set the state for the pending expression */
    loader->state = hdr_pending; /*this is handed in one cycle*/
    return instr;
}



bool hdr_loader_h_construct_switch(PHDR_LOADER loader, char *blk)
{

    /*
      the text must be like  [value] [:] ([{}]or[instruction;]) 
      a small problem is that as we do handle all the switch block in this 
      function , we have to update the loaders instruction lines number.

      Every switch case is stored in the previous (starting with the switch instruction itself)
      ielse instruction and its literal is stored to the the right_part member.
      the first instruction (the switch) has stored the actuall value for check in the switch 
    */

    char sep;
    DXLONG64 cp;
    DXLONG64 ERROR_CODE;
    bool terminated;
    PHDR_INSTRUCTION cur_instr = loader->current_instr;
    while (*blk != 0)
    {
        
        /*get the next full switch component every loop iteration*/
        char* instr = hdr_parser_get_next_instruction(&blk, &loader->instr_line, &loader->line_number, &cp, &ERROR_CODE, &terminated);

        char* instr_indx = instr;
        dxGoForwardWhileChars(&instr_indx, " \t");
        dxRightTrimFastChars(instr_indx, " \t");
        /*check if its an empty string*/
        if (*instr_indx == 0)
        {
            free(instr);
            continue; /*the instr_indx can be empty but the section can still have commands so we will check again in the loop*/
        }

        if ((ERROR_CODE != HDR_SUCCESS) && (ERROR_CODE != HDR_BLOCK_OPEN) && (ERROR_CODE != HDR_BLOCK_CLOSE)
            && (ERROR_CODE != HDR_SCRIPT_END))
        {
            printf("Error while retrieving the instruction of the [switch]. Internal Error : %s\n", hdr_error_code_to_str(ERROR_CODE));
            free(instr);
            return false;
        }

        /*the switch cannot have any other case after the default case (if exists)*/
        if (cur_instr->type == hdr_i_switch_default)
        {
            printf("A [switch] cannot have any case value after the [default] case. The [default] case has to be the last one in the switch.");
            free(instr);
            return false;
        }
        /*get the value from the instruction*/
        char *value = dxGetNextWord(&instr_indx, ":", "`\"", false, false, &sep);
        if (sep == 0)
        {  
            printf("The [switch] value '%s' is malformed. No ':' separator character was found.\n", value);
            free(instr)  ;
            free(value)  ;
            return false ;
        }
        /*
          the value cannot be a regular Hydra+ expression, but only a number , a string or the special [default] token
          the [default] token MUST be the last case of the switch. Internally is represented by 
          an ielse instruction with its right_side set to NULL.
         */
       
         /*trim the value*/
        char* val_indx = value;
        dxGoForwardWhileChars(&val_indx, " \t");
        dxRightTrimFastChars(val_indx, " \t");
        /***************/

        if(dxIsStrReal(val_indx) == false)
         if(dxIsTextAString(val_indx,'`') == false)
             if (dxIsTextAString(val_indx, '"') == false)
             if(dxIsStrNameSafe(val_indx) == false) /*10-07-2024 support for simple tokens*/
                 if (dxIsStrEqual(val_indx, "default") == false)
                 {
                     printf("The [switch] value '%s' is not valid. Valid values are only literal numbers and strings.\n", val_indx);
                     free(instr);
                     free(value);
                     return false;
                 }

        int status;
        /*store the value for comparison*/
        /*we will store the value to the right_part of the new ielse instruction*/
        cur_instr->ielse = hdr_instruction_create();
        cur_instr->ielse->type = hdr_i_switch_case;
        if (cur_instr->ielse == NULL)
        {
            free(instr);
            free(value);
            printf("**System : Fatal Error -> The Loader failed to allocate memory for the ielse\n");
            return false;
        }
        if (dxIsStrEqual(val_indx, "default") == false)
        {
            cur_instr->ielse->right_side = hdr_expr_create_from_str(val_indx, &status);
            if (status != HDR_SUCCESS)
            {
                free(instr);
                free(value);
                printf("Error in expression : %s\n", hdr_error_code_to_str(status));
                return false;
            }
        }
        else
        {
            /*default case*/
            cur_instr->ielse->right_side = NULL; 
            cur_instr->ielse->type = hdr_i_switch_default;
        }
        /*check if a new code block opens or the instruction is a standalone instruction*/
        if (terminated == true)
        {
            /*trim the instruction*/
            dxGoForwardWhileChars(&instr_indx, " \t") ;
            dxRightTrimFastChars(instr_indx, " \t")  ;
            if (strlen(instr_indx) == 0)
            {
                free(instr);
                free(value);
                printf("It is forbidden the cases in a [switch] instruction to have no code to run. For an empty code block please use the 'nop' instruction.\n");
                return false;
            }
            /*add the code*/
            /*save the state of the loader*/
            PHDR_LOADER_STATE lstate = hdr_loader_state_store(loader);
            cur_instr->ielse->code = hdr_block_create(loader->current_block);
            loader->current_block = cur_instr->ielse->code;
            int succ = hdr_loader_create_code_from_string(loader, instr_indx);
            if (succ != HDR_SUCCESS)
            {
                free(instr);
                free(value);
                printf("Error while creating the code block. Internal Error : %s\n",hdr_error_code_to_str(succ));
                return false;
            }
            /*restore the loader*/
            lstate = hdr_loader_state_restore(loader, lstate);
        }
        else
        {
            free(instr);
            /*check for {*/
            instr = hdr_parser_get_next_instruction(&blk, &loader->instr_line, &loader->line_number, &cp, &ERROR_CODE, &terminated);
          
            if ((ERROR_CODE != HDR_SUCCESS) && (ERROR_CODE != HDR_BLOCK_OPEN) && (ERROR_CODE != HDR_BLOCK_CLOSE))
            {
                printf("Error while retrieving the instruction of the [switch]. Internal Error : %s\n", hdr_error_code_to_str(ERROR_CODE));
                free(instr);
                free(value);
                return false;
            }

            /*check if the code has end */
            if (ERROR_CODE == HDR_SCRIPT_END)
            {
                free(instr);
                free(value);
                return true;
            }

            if (dxIsStrEqual(instr, "{") == false)
            {
                printf("The value '%s' of the switch has not a code block.\n",value);
                free(instr);
                free(value);
                return false;
            }

            /*get the section */
            char* sect = dxCopySectionFromStrSmart(&blk, '{', '}', "`\"",1, &status);
            if (sect == NULL)
            {
                printf("The code block of a 'switch' case cannot be empty. For an empty code block please use the 'nop' instruction. \n");
                free(instr);
                free(value);
                return false;
            }
            /*add the code*/
            /*save the state of the loader*/
            PHDR_LOADER_STATE lstate = hdr_loader_state_store(loader);
            
            cur_instr->ielse->code = hdr_block_create(loader->current_block);
            loader->current_block = cur_instr->ielse->code;
            int succ = hdr_loader_create_code_from_string(loader, sect);
            if (succ != HDR_SUCCESS)
            {
                free(instr);
                free(value);
                free(sect);
                free(lstate);
                printf("Error while creating the code block. Internal Error : %s\n", hdr_error_code_to_str(succ));
                return false;
            }

            /*restore the loader*/
            lstate = hdr_loader_state_restore(loader, lstate);
            free(sect);
        }

        cur_instr = cur_instr->ielse ; /*the current instruction is the new formed ielse*/

        free(instr) ;
        free(value) ;
    }


    return true;
}

/**********************************/


/**********DECLARATIONS***********/

bool hdr_loader_h_func_exists(PHDR_LOADER loader, char *func_name)
{

    PDXL_NODE node = loader->functions->start;
    while (node != NULL)
    {
        PDXL_OBJECT      obj   = node->object;
        PHDR_INSTRUCTION instr = (PHDR_INSTRUCTION)obj->obj;
        if (dxIsStrEqual(instr->left_side->ID->stringa, func_name) == true) return true;
        node = node->right;
    }

    return false ;
}

PHDR_INSTRUCTION hdr_loader_h_create_func_decl(PHDR_LOADER loader)
{
    /*
      the declaration does not modify the code of the loader in any 
      way. The declaration of the function is stored in the loader
      functions list
    */
    PHDR_INSTRUCTION instr = hdr_instruction_create();
    if (instr == NULL) return NULL;
    instr->parent = loader->current_block;
    instr->type = hdr_i_func_decl ; /*func declaration we do not use this*/
    /*create the code structure for the function declaration*/
    instr->code = hdr_block_create(loader->current_block);
    instr->code->belongs_to_func = instr ;
    dx_string_createU(instr->filename, loader->curr_file_name->stringa);
    instr->line = loader->instr_line;

    loader->current_instr = instr; /*we will fill the expression in the caller*/
    if (loader->pending_if != NULL) loader->pending_if = NULL;
    /*set the state for the pending expression */
    loader->state = hdr_func_params;/*wait to get the params*/
    return instr;
}

bool hdr_h_class_exist(PHDR_LOADER loader, char *class_name)
{
    PDXL_NODE node = loader->objects->start;
    while (node != NULL)
    {
        PDXL_OBJECT obj             = node->object ;
        PHDR_LOADER_OBJ_PROT class  = (PHDR_LOADER_OBJ_PROT)obj->obj;
        if (dxIsStrEqual(class->name->stringa, class_name) == true) return true;
        node = node->right ;
    }
    return false ;
}

PHDR_LOADER_OBJ_PROT hdr_loader_h_create_o_class(PHDR_LOADER loader, char* class_name)
{
    /*
     the declaration does not modify the code of the loader in any
     way. The declaration of the object is stored in the loader
     objects list.
     Now this is some how tricky as the objects are complex structures.
     The loader is aware of them but it will handle them with the typical 
     for instruction structures.
   */
    PHDR_LOADER_OBJ_PROT class = hdr_loader_obj_prot_create();
    if (class == NULL) return NULL;
    /*create the code structure for the object declaration*/
    dx_string_createU(class->name, class_name) ; 
    dx_string_createU(class->instr->filename , loader->curr_file_name->stringa);
    class->instr->line = loader->instr_line ;
    class->instr->type = hdr_i_obj_decl     ;

    /*the following have special meaning in objects as the object prototypes can have code and functions by themselves (but not object declarations)*/
    loader->current_instr     = class->instr        ; 
    loader->current_block     = class->instr->code  ;
    loader->current_func_list = class->functions    ;
    /***********************************************/
    if (loader->pending_if != NULL) loader->pending_if = NULL;
    /*set the state for the pending code block */
    loader->state = hdr_obj_open;
    return class ;
}


/********************************/

char* hdr_h_return_loader_state_str(enum hdr_loader_parsing_state state)
{
    switch (state)
    {
    case hdr_single: { return "A simple instruction is pending."; }break;
        case hdr_assignment_decl: { return "The loader expects an assigment declaration."; }break;
        case hdr_assign_expres: { return "The loader expects the expression for the assigment."; }break;
        case hdr_if: { return "An 'if' instruction is pending."; }break;
        case hdr_if_instr:{ return "An 'if' is pending, an instruction or a code block is required."; }break;
        case hdr_if_bool_expres:{ return "The expression of the 'if' instruction is required."; }break;
        case hdr_if_open:{return "A code block that belongs to an if is still open.";}break;
        case hdr_else:{ return "An else instruction is pending."; }break;
        case hdr_else_instr: { return "An instruction or code block is required after an 'else' instruction."; }break;
        case hdr_else_open: { return "A code block that belongs to an 'else' is still open."; }break;
        case hdr_func: { return "A function declaration is pending."; }break;
        case hdr_func_params: { return "A function declaration must have a pair of '()' to be valid."; }break;
        case hdr_func_open: { return "A code block that belongs to a function declaration is still open."; }break;
        case hdr_obj: { return "An object declaration is pending."; }break;
        case hdr_obj_open: { return "A block that belongs to an object declaration is still open."; }break;
        case hdr_loop: { return "A 'loop' instruction is pending."; }break;
        case hdr_loop_open: { return "A code block that belongs to a loop is still open."; }break;
        case hdr_switch: { return "A 'switch' instruction is pending."; }break;
        case hdr_switch_expres: { return "A 'switch' instruction needs an expression to evaluate, and its missing."; }break;
        case hdr_switch_open: { return "A code block that belongs to a switch is still open."; }break;
        case hdr_pending: { return NULL; }break;
    }
    
    return NULL;
}

bool hdr_loader_h_handle_block(PHDR_LOADER loader,PDX_STRING prev_i,char **index)
{
    /*
      the function sets the appropriate flags for the loader to load the code block
      and handles the block.
      The index points to the next valid character after the '{' and it will be set to point after the closing '}' of the block
    */

    /*save the states*/
    PHDR_LOADER_STATE tstate = hdr_loader_state_store(loader);
    if (tstate == NULL)
    {
        printf("The instruction '%s' have a code block attached to it when its not valid, or a stray '{' is in the code. Check if you have forgot the [func]. \n", prev_i->stringa);
        return false;

    }

    if (dxCheckSectionPairingSmart(*index, '{', '}',"`\"", 1) == false)
    {
        printf("The code block after the instruction '%s' is been malformed. Check for misplaced '{' or '}'. (Alternative check for not closed strings.) \n", prev_i->stringa);
        return false;
    }

    /*get the  section*/
    int status;
    char* codeb = dxCopySectionFromStrSmart(index, '{', '}', "`\"", 1, &status);

    if ((status != COPY_SECTION_SUCCESS) && (status != COPY_SECTION_NULL_P))
    {
        printf("The code block after the instruction '%s' was not retrieved. Check if the code block is empty. hydra+ does not support empty code blocks. If you need an empty block use the instruction 'nop' Internal error : '%s' \n", prev_i->stringa, hdr_error_code_to_str(status));
        return false;
    }

    
    switch (loader->state)
    {
        case hdr_if_instr: 
        { 
            /*we reset the pending_if*/
            loader->state = hdr_if_open; /*for debuging purposes*/
            loader->pending_if   = NULL;
            loader->branch_state = hdr_br_none;
            /*set the current block as the block of the if instruction*/
            loader->current_block = loader->current_instr->code;
            /*analyze and store the block*/
            if (hdr_loader_create_code_from_string(loader, codeb) != HDR_SUCCESS)
            {
                goto fail;
            }

        }break;
        case hdr_else_instr: 
        { 
            loader->state = hdr_else_open;
            /*set the loader ready to store the block in the pending_if instruction*/

            /*create the else instruction*/
            loader->pending_if->ielse = hdr_instruction_create();
            loader->pending_if->ielse->code = hdr_block_create(loader->current_block);
            loader->pending_if->ielse->parent = loader->current_block;
            loader->pending_if->ielse->iparent = loader->pending_if;

            loader->current_block = loader->pending_if->ielse->code;
            loader->current_instr = NULL;
            loader->pending_if = NULL;
            loader->branch_state = hdr_br_none;
            /*analyze and store the block*/
            if (hdr_loader_create_code_from_string(loader, codeb) != HDR_SUCCESS)
            {
                goto fail;
            }
          
        }break;
        case hdr_func_open: 
        {
           /*
             create a code block with the code of the function.
             this code block in actuality it will never be executed and it will 
             be used as the template for creating the actual function code. 
           */
            loader->current_block    = loader->current_instr->code;
            loader->declaration_mode = hdr_s_decl_func;
            /*analyze and store the block*/
            if (hdr_loader_create_code_from_string(loader, codeb) != HDR_SUCCESS)
            {
                goto fail;
            }

        }break;
        case hdr_obj_open  : 
        { 
            /*we will handle the object class declaration totally in here*/
            loader->declaration_mode = hdr_s_decl_obj ; 
            /*analyze and store the block*/
            if (hdr_loader_create_code_from_string(loader, codeb) != HDR_SUCCESS)
            {
                goto fail;
            }

            tstate = hdr_loader_state_restore(loader, tstate);
            /*all good set the loader for the next instruction*/
            loader->state = hdr_pending                   ;
            loader->current_instr = NULL                  ;
            loader->current_block = loader->code          ;
            loader->current_func_list = loader->functions ;
            goto success; 

        }break;
        case hdr_loop : 
        { 
            loader->state = hdr_loop_open ; /*for debuging purposes*/
            /*set the current block as the block of the if instruction*/
            loader->current_block = loader->current_instr->code;
            /*analyze and store the block*/
            if (hdr_loader_create_code_from_string(loader, codeb) != HDR_SUCCESS)
            {
                goto fail;
            }
        }break;
        case hdr_switch_block: { loader->state = hdr_switch_open; }break;
        default: {
                    printf("The instruction '%s' have a code block attached to it when its not valid, or a stray '{' exists. Check if you have forgot the [func].\n", prev_i->stringa);
                    goto fail;
                };
    }

    tstate = hdr_loader_state_restore(loader, tstate);
    /*check the banching state if ends the [if] chain*/
    if(loader->state == hdr_else_instr)
    if (loader->branch_state == hdr_br_else) loader->branch_state = hdr_br_none;
    /*all good set the loader for the next instruction*/
    loader->state = hdr_pending;
    loader->current_instr = NULL;
success:
    free(codeb);
    return true;

fail :
    free(tstate);
    free(codeb);
    return false;
}

int hdr_loader_create_code_from_string(PHDR_LOADER loader, char* text)
{
    /*
      The function parses the text and creates the appropriate structs and data.
      The data are being stored to the current block of the loader 
    */

    char *indx = text;
    PDX_STRING curr_file_name = loader->curr_file_name;
    int ret = HDR_SUCCESS;

    DXLONG64 ERROR_CODE = 0;

    PDX_STRING prev_i = dx_string_createU(NULL, ""); /*keeps the previous valid instruction for debug purposes*/
    DXLONG64 block_header_ln = 0; /*This temporary stores the line number of the header of a block e.g if ,else ,loop,switch,func,obj */
    bool first_instr = false; /*for find erroneus include directives. If this is true (is set to true when an
                                        instruction other than the include is found) then if an include is found in the main file
                                        then a fatal error is produced.*/
    loader->state = hdr_pending;/*the state of the loader in any point of time*/
    /*the loader current instruction must be set to NULL , as the text is an isolated code block*/
    loader->current_instr = NULL;
    while (true)
    {
        /*load instructions until end*/
        bool terminated;
        char* instr = hdr_parser_get_next_instruction(&indx, &(loader->instr_line), &(loader->line_number), &(loader->char_pos), &ERROR_CODE,&terminated);
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
            printf("**System : [%s] Fatal Error -> The Loader encounters a syntax error. Code : %d  start line : %d  line : %d last character position : %d\n", curr_file_name->stringa, ERROR_CODE, (int)loader->instr_line, (int)loader->line_number, (int)loader->char_pos);
            free(tr);
            if (instr != NULL) free(instr);
            return false;
        }
        else
        {
            char* instr_indx = tr;
            if (ERROR_CODE == HDR_SUCCESS)
            {
                /*The instruction was retrieved successfuly , now it must be handled appropriately*/
                /*Get the entities*/
                while (*instr_indx != 0)
                {
                    bool fatal_error = false ;
                    PHDR_ENTITY entity = hdr_loader_get_entity(loader->instr_line, &instr_indx,&fatal_error);
                    if(fatal_error == true)
                    {
                        ret = HDR_ERROR_LOADER_SYNTAX_ERROR;
                        goto fail;
                    }
                    if (entity == NULL)
                    {

                        /*
                          check the states if the absence of an entity is what we expect
                          The states that warrand an empty entity are the states that they do not
                          expect a mandatory field after it.
                        */
                        ret = hdr_loader_check_state(loader->state, loader->instr_line, prev_i);
                        if (ret != HDR_SUCCESS)
                        {
                            free(tr);
                            if (instr != NULL) free(instr);
                            dx_string_free(prev_i);
                            return ret;
                        }
                        break;

                    }

                    /*check the includes*/
                    if (dxIsStrEqual(entity->entity->stringa, "include") == false)
                    {
                        if (first_instr == false) first_instr = true;
                    }
                    else
                    {
                        if ((first_instr == true)||(loader->in_main_file == false))
                        {
                            printf("[%s] Fatal Error -> Line : %d:%d The 'include' directive is valid only in the top of the main file . \n", curr_file_name->stringa, loader->instr_line, loader->line_number);
                            ret = HDR_ERROR_LOADER_SYNTAX_ERROR;
                            goto fail;
                        }
                        else
                        {
                            /*we have already handle the include , ignore it*/
                            hdr_entity_free(entity);
                           // dxGoForwardToCh(&indx, ';');
                            dxGoForwardToCh(&instr_indx, 0);
                         //   if (*indx != 0) indx++; /*next valid char*/
                            continue;
                        }
                    }
                    /*
                      check what the entity symbolizes and set the state
                    */
                    ret = hdr_loader_set_state(instr,entity, loader); 
                    if (ret == HDR_SUCCESS)
                    {
                        /*
                          the state was changed succesfully
                          Check the actual state that the loader is
                        */
                        /*first check if we have to clear the branch state */
                        if ((loader->state != hdr_if_bool_expres) && (loader->state != hdr_if_instr) &&
                            (loader->state != hdr_else_instr) && (loader->state != hdr_else_open))
                            loader->branch_state = hdr_br_none; 
                        switch (loader->state)
                        {
                            case hdr_assignment_decl:
                            {
                                /*
                                  check if the assigment is an assigment or a declaration ( = exists ?)
                                  as the left expression can be very complicated we will check all the instruction
                                  for the = because the entity can be just the left part of a multipart tolen
                                */
                                instr_indx = instr; /*reset the instruction position*/
                                bool its_assign = dxCharExists(&instr_indx, '=' , "`\"");
                                if (its_assign == true)
                                {
                                    /*check for the == operator */
                                   
                                    if (dxIsCharInSet(*(instr_indx + 1),"=/*%<>!") == true)
                                    {
                                        printf("[%s] Fatal Error -> Line : %d:%d Expected an [=] for the assigment in [%s] but an invalid character is following. %s\n", curr_file_name->stringa, loader->instr_line, loader->line_number, entity->entity->stringa, instr);
                                        ret = HDR_ERROR_LOADER_SYNTAX_ERROR;
                                        goto fail;
                                    }
                                    instr_indx++; /*next character*/
                                    /*check if the entity is indeed a variable*/
                                    if (entity->entity->stringa[0] != '$')
                                    {
                                        printf("[%s] Fatal Error -> Line : %d:%d Expected a variable (a token prefixed with $) for the assigment in [%s] but a the following token [%s] was encouter. \n", curr_file_name->stringa, loader->instr_line, loader->line_number, entity->entity->stringa, entity->separator->stringa);
                                        ret = HDR_ERROR_LOADER_SYNTAX_ERROR;
                                        goto fail;
                                    }

                                    /*
                                      this is an assigment. We will check if the variable already exists in the block
                                      and we will create an instruction with the assigment
                                    */
                                    bool global = true ;
                                    /*The global will become false if we are in an function or object declaration*/
                                    if((loader->declaration_mode == hdr_s_decl_func)||(loader->declaration_mode == hdr_s_decl_obj)) 
                                        global = false ;

                                    if (hdr_var_list_find_var_scope(loader->current_block->variables, entity->entity,global) == false)
                                    {
                                        bool error_v = true ; 
                                        hdr_loader_create_variable(loader, entity->entity->stringa,&error_v) ;
                                        if ( error_v == true)
                                        {
                                            printf("[%s] Fatal Error -> Line : %d:%d The variable name is too long or invalid in this instruction or block [%s] .\n", curr_file_name->stringa, loader->instr_line, loader->line_number, entity->entity->stringa);
                                            ret = HDR_ERROR_LOADER_SYNTAX_ERROR;
                                            goto fail;

                                        }
                                    }

                                    /*ok, now to check if the variable is a single one or a multipart /indexed token*/
                                    bool complex = false   ;
                                    char* full_name = NULL ;
                                    char* full_name_indx = NULL ;
                                    if ((entity->separator->stringa[0] == '[') || (entity->separator->stringa[0] == '.'))
                                    {
                                        complex = true;
                                        /*get full name*/
                                        instr_indx = instr;
                                        full_name = dxCopyStrToChar(&instr_indx, '=', "`\"");
                                        instr_indx++; /*after =*/
                                        dxGoForwardWhileChars(&instr_indx, " \t");
                                        full_name_indx = full_name ;
                                        dxGoForwardWhileChars(&full_name_indx, " \t") ;
                                        dxRightTrimFastChars(full_name_indx," \t")   ;
                                    }

                                    /*create an assigment instruction*/
                                    if (hdr_loader_h_create_asgn_i(loader, entity->entity->stringa,complex,full_name_indx) == NULL)
                                    {
                                        printf("[%s] Fatal Error -> Line : %d:%d The instruction was not created. Failed to allocate memory. Instruction : [%s] .\n", curr_file_name->stringa, loader->instr_line, loader->line_number, entity->entity->stringa);
                                        ret = HDR_ERROR_LOADER_INTERNAL_ERROR;
                                        goto fail;

                                    }
                                    free(full_name);

                                    /*go to do the assigment*/
                                    goto create_the_assigment;
                                }
                                else
                                {
                                    /*if this is a multipart or/and indexed variable then we have to handled too*/

                                    /*check if this is a complex variable*/
                                    if ((entity->separator->stringa[0] == '[') || (entity->separator->stringa[0] == '.'))
                                    {
                                        bool global = true ;
                                        if((loader->declaration_mode == hdr_s_decl_func)||(loader->declaration_mode == hdr_s_decl_obj))
                                            global = false ;
                                        if (hdr_var_list_find_var_scope(loader->current_block->variables, entity->entity,global) == false)
                                        {
                                            bool error_v = true ;
                                            hdr_loader_create_variable(loader, entity->entity->stringa,&error_v) ;
                                            if ( error_v == true)
                                            {
                                                printf("[%s] Fatal Error -> Line : %d:%d The variable name is too long or invalid in instruction [%s] .\n", curr_file_name->stringa, loader->instr_line, loader->line_number, entity->entity->stringa);
                                                ret = HDR_ERROR_LOADER_SYNTAX_ERROR;
                                                goto fail;

                                            }

                                            /*create the expression*/

                                            /*reset the state , a new instruction is expected*/
                                            loader->state = hdr_pending;
                                        }

                                        /*create the instruction*/
                                        hdr_loader_h_create_var_cmd(loader,entity->entity->stringa ,instr);
                                        /*add the instruction*/
                                        if (hdr_instructions_list_add(loader->current_block->instructions, loader->current_instr) == NULL)
                                        {
                                            printf("[%s] Fatal Error -> Line : %d:%d The loader was unabled to add the instruction in the block. Memory allocation failed. '%s' .\n", curr_file_name->stringa, loader->instr_line, loader->line_number, entity->entity->stringa);
                                            ret = HDR_ERROR_LOADER_INTERNAL_ERROR;
                                            hdr_instruction_free(loader->current_instr);
                                            goto fail;
                                        }
                                        loader->current_instr = NULL;
                                        /*the state is already been reset in the hdr_loader_h_create_var_cmd */
                                        /*go to the end , all of the expression has been handled in here*/
                                        dxGoForwardToCh(&instr_indx, 0);
                                    }
                                    else
                                    {
                                        /*as it seems this is a simple declaration. Add the variable in the block , we will not add any instruction*/
                                        bool global = true ;
                                        if((loader->declaration_mode == hdr_s_decl_func)||(loader->declaration_mode == hdr_s_decl_obj))
                                            global = false ;
                                        if (hdr_var_list_find_var_scope(loader->current_block->variables, entity->entity,global) == false)
                                        {
                                            bool error_v = true;
                                            hdr_loader_create_variable(loader, entity->entity->stringa,&error_v) ;
                                            if ( error_v == true)
                                            {
                                                printf("[%s] Fatal Error -> Line : %d:%d The variable name is too long or invalid in instruction [%s] .\n", curr_file_name->stringa, loader->instr_line, loader->line_number, entity->entity->stringa);
                                                ret = HDR_ERROR_LOADER_SYNTAX_ERROR;
                                                goto fail;

                                            }

                                            /*reset the state , a new instruction is expected*/
                                            loader->state = hdr_pending;
                                        }
                                        else
                                        {
                                            printf("[%s],Fatal Error -> Line : %d:%d The variable has been already declared. Is prohibited to redeclare a variable. [%s] .\n", curr_file_name->stringa, loader->instr_line, loader->line_number, entity->entity->stringa);
                                            ret = HDR_ERROR_LOADER_SYNTAX_ERROR;
                                            goto fail;
                                        }
                                    }
                                }
                            }
                            break;
                            case hdr_assign_expres:
                            {
                                /*
                                 Nice , we need to get the expression , analyze it and save it in the right part of the instruction
                                */
                            create_the_assigment: /*jump from the hdr_assignment_decl*/
                                if (loader->current_instr == NULL)
                                {
                                    printf("[%s] Fatal Error -> Line : %d:%d WTF ! The loader->current_instruction is NULL. Why? [%s] .\n", curr_file_name->stringa, loader->instr_line, loader->line_number, entity->entity->stringa);
                                    ret = HDR_ERROR_LOADER_INTERNAL_ERROR;
                                    goto fail;
                                }

                                /*
                                 hydra support operands up to 1024 characters , but its very common
                                 to have strings of many kilobytes or even megabytes in a script,
                                 so only for the assigment of strings will  permit an arbritary 
                                 size of bytes. BUT only in a pure assigment ,else the hard limit stands 
                                */

                                /*check if the operand is a string*/
                                dxGoForwardWhileChars(&instr_indx, " \t")   ;
                                dxRightTrimFastChars(instr_indx, " \t")     ;
                                if (dxIsStringValidEncaps(instr_indx, "`\"") == true)
                                {
                                    int status;
                                    /*create an expression with a single string*/
                                    loader->current_instr->right_side = hdr_expression_create();
                                    if (hdr_expr_h_add_simple_and_op(loader->current_instr->right_side, instr_indx, 0, &status) == false)
                                    {
                                        printf("[%s] Fatal Error -> Line : %d:%d The simple token '%s' was not inserted in the expression. Internal Error : %s\n", 
                                            curr_file_name->stringa, loader->instr_line, loader->line_number, instr_indx,hdr_error_code_to_str(status));
                                        ret = HDR_ERROR_LOADER_INTERNAL_ERROR;
                                        goto fail;
                                    }
                                   

                                } else
                                /*create the expression*/
                                loader->current_instr->right_side = hdr_expr_create_expr(loader->instr_line, instr_indx);
                                /*go to the end , all of the expression has been handled in here*/
                                dxGoForwardToCh(&instr_indx, 0);

                                if (loader->current_instr->right_side == NULL)
                                {
                                    printf("[%s] Fatal Error -> Line : %d:%d An expression was expected for the assigment but an error occured . '%s' .\n", curr_file_name->stringa, loader->instr_line, loader->line_number, instr);
                                    ret = HDR_ERROR_LOADER_INTERNAL_ERROR;
                                    hdr_instruction_free(loader->current_instr);
                                    goto fail;
                                }
                                if (hdr_instructions_list_add(loader->current_block->instructions, loader->current_instr) == NULL)
                                {
                                    printf("[%s] Fatal Error -> Line : %d:%d The loader was unabled to add the instruction in the block. Memory allocation failed. '%s' .\n", curr_file_name->stringa, loader->instr_line, loader->line_number, entity->entity->stringa);
                                    ret = HDR_ERROR_LOADER_INTERNAL_ERROR;
                                    hdr_instruction_free(loader->current_instr);
                                    goto fail;
                                }
                                loader->current_instr = NULL;
                                loader->state = hdr_pending;
                            }break;

                            case hdr_if_bool_expres:
                            {
                                /*found an if keyword, create the instruction */

                                /*check for the validity of the if*/
                                if (entity->separator->stringa[0] != '(')
                                {

                                    printf("[%s] Fatal Error -> Line : %d:%d A boolean expression after an 'if' MUST be encapsulated by '()' . '%s' .\n", curr_file_name->stringa, loader->instr_line, loader->line_number, instr);
                                    ret = HDR_ERROR_LOADER_INTERNAL_ERROR;
                                    hdr_instruction_free(loader->current_instr);
                                    goto fail;
                                }

                                if (loader->branch_state == hdr_br_else)
                                {
                                    printf("[%s] Fatal Error -> Line : %d:%d An 'if' instruction cannot follows a terminal else INTERNAL ERROR THIS IS NOT SUPPOSED TO HAPPEN  . '%s' .\n", curr_file_name->stringa, loader->instr_line, loader->line_number, instr);
                                    ret = HDR_ERROR_LOADER_INTERNAL_ERROR;
                                    hdr_instruction_free(loader->current_instr);
                                    goto fail;

                                }

                                hdr_loader_h_create_if_i(loader);
                                if (hdr_instructions_list_add(loader->current_block->instructions, loader->current_instr) == NULL)
                                {
                                    printf("[%s] Fatal Error -> Line : %d:%d The loader was unabled to add the instruction in the block. Memory allocation failed. '%s' .\n", curr_file_name->stringa, loader->instr_line, loader->line_number, entity->entity->stringa);
                                    ret = HDR_ERROR_LOADER_INTERNAL_ERROR;
                                    hdr_instruction_free(loader->current_instr);
                                    goto fail;
                                }
                                
                                /*set the brancing state*/
                                loader->branch_state = hdr_br_if;

                                /*copy the section of the if expression */
                                int status;
                                char* if_expr = dxCopySectionFromStrSmart(&instr_indx, '(', ')', "`\"",1, &status);
                                if (status != COPY_SECTION_SUCCESS)
                                {
                                    printf("[%s] Fatal Error -> Line : %d:%d An error occured in the expression of the 'if' . '%s' .\n", curr_file_name->stringa, loader->instr_line, loader->line_number, instr);
                                    ret = HDR_ERROR_LOADER_INTERNAL_ERROR;
                                    goto fail;
                                }
                                /*analyze the expression*/
                                loader->current_instr->right_side = hdr_bool_create_expr(if_expr);
                                free(if_expr);
                                //_DEBUG_PRINT_EXPRESSION_STRUCT(loader->current_instr->right_side, 0, true);

                                if (loader->current_instr->right_side == NULL)
                                {
                                    printf("[%s] Fatal Error -> Line : %d:%d A boolean expression was expected for the assigment but an error occured . '%s' .\n", curr_file_name->stringa, loader->instr_line, loader->line_number, instr);
                                    ret = HDR_ERROR_LOADER_INTERNAL_ERROR;
                                    goto fail;
                                }
                                /*
                                  the if instruction has been created and the boolean expression is in place
                                  now we have to retrieve the instructions that will be executed when the if
                                  is true. The instructions can be in a block or a singular standalone instruction
                                */
                                /*check for the standalone instruction*/
                                if (*instr_indx != 0)
                                {
                                    /*
                                      so we have a singular instruction to handle,we will run this function for the instruction
                                      after we set the loaders current block as the block of the 'if'  
                                    */


                                    PHDR_LOADER_STATE tstate = hdr_loader_state_store(loader);
                                    if (tstate == NULL)
                                    {
                                        printf("[tstate] is NULL. Memory allocation Error");
                                        goto fail;
                                    }
                                    loader->current_block = loader->current_instr->code;
                                    loader->current_instr    = NULL;
                                   
                                    /*
                                     check if the next instruction is a switch.
                                     we do not support inline switch. It must be inside a code block
                                    */
                                    char* tindx = instr_indx;
                                    char sep;
                                    dxGoForwardWhileChars(&tindx, " \t");
                                    char* token = dxGetNextWord(&tindx, " ({", "`\"", false, false, &sep);
                                    if (dxIsStrEqual(token, "switch") == true)
                                    {   
                                        printf("[%s] Fatal Error -> Line : %d:%d The 'switch' instruction MUST be inside a code block '{}' for it to be valid.\n", curr_file_name->stringa, loader->instr_line, loader->line_number);
                                        ret = HDR_ERROR_LOADER_INTERNAL_ERROR;
                                        free(token);
                                        free(tstate);
                                        goto fail;
                                    }
                                    free(token);

                                    /*the code*/
                                    int res = hdr_loader_create_code_from_string(loader, instr_indx);
                                    if (res != HDR_SUCCESS)
                                    {
                                        ret = res ;
                                        free(tstate);
                                        goto fail ;
                                    }
                                    
                                    tstate = hdr_loader_state_restore(loader, tstate);

                                    /*go to the end , all of the expression has been handled in here*/
                                    dxGoForwardToCh(&instr_indx, 0);
                                    /*reset the loader state*/
                                    loader->state = hdr_pending ;
                                }
                                else
                                {
                                    /*check if the 'if' is invalid*/
                                    if (terminated == true)
                                    {
                                        printf("[%s] Fatal Error -> Line : %d:%d The 'if' instruction MUST have an instruction or a code block after its expression. '%s' .\n", curr_file_name->stringa, loader->instr_line, loader->line_number, instr);
                                        ret = HDR_ERROR_LOADER_INTERNAL_ERROR;
                                        goto fail;
                                    }
                                }

                               

                            }break;

                            case hdr_else_instr :
                            {
                                /*check if there is an if pending in the loader*/
                                if (loader->pending_if == NULL)
                                {
                                    printf("[%s] Fatal Error -> Line : %d:%d The 'else' instruction MUST follow an 'if' instruction.\n", curr_file_name->stringa, loader->instr_line, loader->line_number);
                                    ret = HDR_ERROR_LOADER_INTERNAL_ERROR;
                                    goto fail;
                                }

                                /*check if its position is correct*/
                                if (loader->branch_state == hdr_br_else)
                                {
                                    printf("[%s] Fatal Error -> Line : %d:%d The standalone 'else'(terminal) structure cannot follow another standalone (terminal) 'else' instruction .\n", curr_file_name->stringa, loader->instr_line, loader->line_number);
                                    ret = HDR_ERROR_LOADER_INTERNAL_ERROR;
                                    goto fail;

                                }

                                /*retrieve the next instruction if its a standalone one*/
                                if (*instr_indx != 0)
                                {
                                    /*
                                      so we have a singular instruction to handle,we will run this function for the instruction
                                      after we set the loaders current block as the block of the 'if'
                                    */


                                    /*
                                     check if the standalone instruction is an if or an else because we have to check for 
                                     branching errors
                                    */

                                    char* tindx = instr_indx ;
                                    char sep;
                                    dxGoForwardWhileChars(&tindx, " \t");
                                    char* token = dxGetNextWord(&tindx, " ({" ,"`\"", false ,false, &sep);
                                    if (dxIsStrEqual(token, "if"))
                                    {
                                        /*
                                         the instruction is an [if]. If the branching state is an if or an else if
                                         then this is a valid branching
                                         */
                                        if (loader->branch_state == hdr_br_if)
                                        {
                                            /*this is an else if*/
                                            loader->branch_state = hdr_br_else_if;
                                        }
                                        else
                                            if (loader->branch_state == hdr_br_else_if)
                                            {
                                                /*nothing to do , this is a  string of else if's*/
                                            }
                                            else
                                                if(loader->branch_state == hdr_br_else)
                                                {
                                                    printf("[%s] Fatal Error -> Line : %d:%d The 'else if' structure cannot follow a standalone (terminal) 'else' instruction .\n", curr_file_name->stringa, loader->instr_line,loader->line_number);
                                                    ret = HDR_ERROR_LOADER_INTERNAL_ERROR;
                                                    free(token);
                                                    goto fail;
                                                }
                                                else
                                                    if (loader->branch_state == hdr_br_none)
                                                    {
                                                        printf("[%s] Fatal Error -> Line : %d:%d The 'else' must follows an 'if' or 'else if'.\n", curr_file_name->stringa, loader->instr_line, loader->line_number);
                                                        ret = HDR_ERROR_LOADER_INTERNAL_ERROR;
                                                        free(token);
                                                        goto fail;
                                                    }

                                    }
                                    else  
                                    if (dxIsStrEqual(token, "switch")) /*we do not support inline switch. the switch has to be in a code block*/
                                    {
                                        printf("[%s] Fatal Error -> Line : %d:%d The 'switch' instruction MUST be inside a code block '{}' for it to be valid.\n", curr_file_name->stringa, loader->instr_line, loader->line_number);
                                        ret = HDR_ERROR_LOADER_INTERNAL_ERROR;
                                        free(token);
                                        goto fail;
                                    }
                                    else
                                    {
                                        /*this is a terminal else, clear the state*/
                                        loader->branch_state = hdr_br_else;

                                    }
                                    free(token);

                                    PHDR_BLOCK tblock = loader->current_block;
                                    /*set the current block as the 'else' in the if */
                                    loader->pending_if->ielse          = hdr_instruction_create();
                                    loader->pending_if->ielse->iparent = loader->pending_if ;
                                    loader->pending_if->ielse->code = hdr_block_create(loader->current_block);
                                    loader->current_block           = loader->pending_if->ielse->code ;
                                    loader->current_instr = NULL;

                                    /*run the code*/
                                    int res = hdr_loader_create_code_from_string(loader, instr_indx);
                                    if (res != HDR_SUCCESS)
                                    {
                                        ret = res;
                                        goto fail;
                                    }

                                    /*
                                     the loader state must retains the values from the hdr_loader_create_code_from_string as the 
                                     code flow is still in the base code block , but the current block has to being reset in the one before else
                                    */
                                    loader->current_block = tblock;
 
                                    /*go to the end , all of the expression has been handled in here*/
                                    dxGoForwardToCh(&instr_indx, 0);
                                }
                                else
                                {
                                    /*
                                     the loader will excpect an open block for the else.
                                     set this else as the terminating else
                                    */
                                    loader->branch_state = hdr_br_else;
                                }

                            }break;

                            case hdr_switch :
                            {
                                /*
                                 the loader has states for the 'switch' instruction but i will handle the switch 
                                 completelly in there without continue with the loader flow. I will left the 
                                 switch flags in place though. 
                                */
                                /*get the expression, check for the validity of the section () */
                                if (dxCheckSectionPairingSmart(instr_indx, '(', ')', "`\"", 1) == false)
                                {
                                    printf("[%s] Fatal Error -> Line : %d:%d The expression of the 'switch(expression)' is invalid. Check for erroneus pairing of '()'.\n", curr_file_name->stringa, loader->instr_line, loader->line_number);
                                    ret = HDR_ERROR_LOADER_INTERNAL_ERROR;
                                    goto fail;
                                }

                                /*get the expression*/
                                int status ;
                                char* expr = dxCopySectionFromStrSmart(&instr_indx, '(', ')', "`\"", 1, &status);
                                if ((status != COPY_SECTION_SUCCESS) && (status != COPY_SECTION_NULL_P))
                                {
                                    printf("[%s] Fatal Error -> Line : %d:%d Error while retrieving the expression of the 'switch' instruction. Internal Error : %s\n", curr_file_name->stringa, loader->instr_line, loader->line_number, hdr_error_code_to_str(status));
                                    ret = HDR_ERROR_LOADER_INTERNAL_ERROR;
                                    goto fail;
                                }

                                /*create the switch instruction*/
                                if (hdr_loader_h_create_switch_i(loader, expr) == NULL)
                                {
                                    free(expr);
                                    printf("[%s] Fatal Error -> Line : %d:%d The instruction was not created. Failed to allocate memory. Instruction : [%s] .\n", curr_file_name->stringa, loader->instr_line, loader->line_number, entity->entity->stringa);
                                    ret = HDR_ERROR_LOADER_INTERNAL_ERROR;
                                    goto fail;

                                }
                                free(expr);
                                /*add the instruction*/
                                if (hdr_instructions_list_add(loader->current_block->instructions, loader->current_instr) == NULL)
                                {
                                    printf("[%s] Fatal Error -> Line : %d:%d The loader was unabled to add the instruction in the block. Memory allocation failed. '%s' .\n", curr_file_name->stringa, loader->instr_line, loader->line_number, entity->entity->stringa);
                                    ret = HDR_ERROR_LOADER_INTERNAL_ERROR;
                                    hdr_instruction_free(loader->current_instr);
                                    goto fail;
                                }
                                /*get the switch block */

                                char *blk = dxCopySectionFromStr(&indx, '{', '}', "`\"", &status);
                                if (blk == NULL)
                                {
                                    printf("[%s] Fatal Error -> Line : %d:%d Malformed [switch]. The [switch] MUST have a valid block. Internal Error : %s\n", curr_file_name->stringa, loader->instr_line, loader->line_number,hdr_error_code_to_str(status));
                                    ret = HDR_ERROR_LOADER_INTERNAL_ERROR;
                                    goto fail;
                                }
                                /*loaders current instruction is the switch*/
                                if (hdr_loader_h_construct_switch(loader,blk)== false)
                                { 
                                    printf("[%s] Fatal Error -> Line : %d:%d Malformed [switch]. Block : [%s] .\n", curr_file_name->stringa, loader->instr_line, loader->line_number, blk);
                                    free(blk);
                                    ret = HDR_ERROR_LOADER_INTERNAL_ERROR;
                                    goto fail;
                                }
                                free(blk);
                                loader->state = hdr_pending;

                            }break ;
                            case hdr_single:
                            {
                                /*check if the separator is a = because is forbiden a token to be assigned with something*/
                                if (entity->separator->stringa[0] == '=')
                                {
                                    printf("[%s] Fatal Error -> Line : %d:%d The simple token '%s' cannot be assigned with anything.\n", curr_file_name->stringa, loader->instr_line, loader->line_number, entity->entity->stringa);
                                    ret = HDR_ERROR_LOADER_INTERNAL_ERROR;
                                    goto fail;
                                }

                                /*the single token must be handled as an instruction and not per entity*/
                                /*the token can be a word with a following expression or a single word that can be anything*/
                                instr_indx = instr;/*reset the instruction*/

                                /*trim the instruction*/
                                dxGoForwardWhileChars(&instr_indx, " \t");
                                dxRightTrimFastChars(instr_indx, " \t");

                                char sep = 0;
                                char * word = dxGetNextWord(&instr_indx," \t","`\"",true,true,&sep);
                                if (*instr_indx == 0)
                                {
                                    /*a single token , we will save it as an expression in a instruction*/
                                    PHDR_INSTRUCTION pinstr = hdr_loader_h_create_single_token_i(loader, word);
                                    if (pinstr == NULL)
                                    {
                                        printf("[%s] Fatal Error -> Line : %d:%d Expression error.\n", curr_file_name->stringa, loader->instr_line, loader->line_number);
                                        ret = HDR_ERROR_LOADER_INTERNAL_ERROR;
                                        free(word);
                                        goto fail;
                                    }
                                    /*add the instruction in the block*/
                                    if (hdr_instructions_list_add(loader->current_block->instructions, loader->current_instr) == NULL)
                                    {
                                        printf("[%s] Fatal Error -> Line : %d:%d The loader was unabled to add the instruction in the block. Memory allocation failed. '%s' .\n", curr_file_name->stringa, loader->instr_line, loader->line_number, entity->entity->stringa);
                                        ret = HDR_ERROR_LOADER_INTERNAL_ERROR;
                                        hdr_instruction_free(loader->current_instr);
                                        goto fail;
                                    }
                                    free(word);
                                    hdr_entity_free(entity);
                                    continue;
                                }
                                /*the instruction is a word with a type of parameter*/

                                PHDR_INSTRUCTION pinstr = hdr_loader_h_create_param_token_i(loader, word,instr_indx);
                                if (pinstr == NULL)
                                {
                                    printf("[%s] Fatal Error -> Line : %d:%d Expression error.\n", curr_file_name->stringa, loader->instr_line, loader->line_number);
                                    ret = HDR_ERROR_LOADER_INTERNAL_ERROR;
                                    free(word);
                                    goto fail;
                                }
                                /*add the instruction in the block*/
                                if (hdr_instructions_list_add(loader->current_block->instructions, loader->current_instr) == NULL)
                                {
                                    printf("[%s] Fatal Error -> Line : %d:%d The loader was unabled to add the instruction in the block. Memory allocation failed. '%s' .\n", curr_file_name->stringa, loader->instr_line, loader->line_number, entity->entity->stringa);
                                    ret = HDR_ERROR_LOADER_INTERNAL_ERROR;
                                    hdr_instruction_free(loader->current_instr);
                                    goto fail;
                                }
                                free(word);
                                hdr_entity_free(entity);
                                dxGoForwardToCh(&instr_indx, 0);/*go to the end of the instruction, we are done*/
                                continue;
                                
                            }break;
                            case hdr_loop :
                            {
                                /*set the right state in the loader*/
                                loader->state = hdr_loop_open;/*we expect a block opening*/
                                /*check if the loop instruction is valid*/
                                if (terminated == true)
                                {
                                    printf("[%s] Fatal Error -> Line : %d:%d The 'loop' instruction cannot be terminated by an ';'. A code block has to follow. Instruction : %s.\n", curr_file_name->stringa, loader->instr_line, loader->line_number, instr);
                                    ret = HDR_ERROR_LOADER_INTERNAL_ERROR;
                                    goto fail;
                                }
                                if (*instr_indx != 0)
                                {
                                    printf("[%s] Fatal Error -> Line : %d:%d The 'loop' instruction cannot be followed by any other instruction or token : %s.\n", curr_file_name->stringa, loader->instr_line, loader->line_number, instr);
                                    ret = HDR_ERROR_LOADER_INTERNAL_ERROR;
                                    goto fail;
                                }

                                /*create the instruction and set the loader state*/
                                hdr_loader_h_create_loop_i(loader);
                                if (hdr_instructions_list_add(loader->current_block->instructions, loader->current_instr) == NULL)
                                {
                                    printf("[%s] Fatal Error -> Line : %d:%d The loader was unabled to add the 'loop' instruction in the block. Memory allocation failed. '%s' .\n", curr_file_name->stringa, loader->instr_line, loader->line_number, entity->entity->stringa);
                                    hdr_instruction_free(loader->current_instr);
                                    ret = HDR_ERROR_LOADER_INTERNAL_ERROR;
                                    goto fail;
                                }

                                /*the loader is setup to load the block*/


                            }break;
                            /***************************DECLARATIONS******************************/
                            case hdr_func :
                            {
                                /*check if its valid to declare a function*/
                                if (loader->declaration_mode == hdr_s_decl_func)
                                {
                                    printf("[%s] Fatal Error -> Line : %d:%d The declaration of a function inside another function is not valid. : %s.\n", curr_file_name->stringa, loader->instr_line, loader->line_number, instr);
                                    ret = HDR_ERROR_LOADER_SYNTAX_ERROR;
                                    goto fail;
                                }
                                /*check if the separator is valid*/
                                if (entity->separator->stringa[0] != '(')
                                {
                                    printf("[%s] Fatal Error -> Line : %d:%d A function is required to have a valid parameter section '()' : %s.\n", curr_file_name->stringa, loader->instr_line, loader->line_number, instr);
                                    ret = HDR_ERROR_LOADER_SYNTAX_ERROR;
                                    goto fail;
                                }
                                /*get and validate the function name*/
                                char *ent_indx  = entity->entity->stringa;
                                dxGoForwardToCh(&ent_indx, ' ');
                                dxGoForwardWhileChars(&ent_indx, " \t");
                                char sep = 0;
                                char* func_name = dxGetNextWord(&ent_indx, "(", "`\"", false, false, &sep);
                                if (func_name == NULL)
                                {
                                    printf("[%s] Fatal Error -> Line : %d:%d A function is required to have a valid name : %s.\n", curr_file_name->stringa, loader->instr_line, loader->line_number, instr);
                                    ret = HDR_ERROR_LOADER_SYNTAX_ERROR;
                                    goto fail;
                                }

                                if (func_name[0] == 0)
                                {
                                    printf("[%s] Fatal Error -> Line : %d:%d A function is required to have a valid name, none found : %s.\n", curr_file_name->stringa, loader->instr_line, loader->line_number, instr);
                                    ret = HDR_ERROR_LOADER_SYNTAX_ERROR;
                                    free(func_name);
                                    goto fail;
                                }
                                /*check the name for validity*/
                                if (dxIsStrNameSafe(func_name) == false)
                                {
                                    printf("[%s] Fatal Error -> Line : %d:%d A function is required to have a valid name : %s.\n", curr_file_name->stringa, loader->instr_line, loader->line_number, func_name);
                                    free(func_name);
                                    ret = HDR_ERROR_LOADER_SYNTAX_ERROR;
                                    goto fail;
                                }

                                /*check if the function is already declared*/

                                if (hdr_loader_h_func_exists(loader, func_name) == true)
                                {
                                    printf("[%s] Fatal Error -> Line : %d:%d The function '%s' is already declared.\n", curr_file_name->stringa, loader->instr_line, loader->line_number, func_name);
                                    free(func_name);
                                    ret = HDR_ERROR_LOADER_SYNTAX_ERROR;
                                    goto fail;
                                }

                                /*set the right state in the loader*/
                                loader->state = hdr_func_params;/*we expect a () section opening*/
                                /*check if the function instruction is valid*/
                                if (terminated == true)
                                {
                                    printf("[%s] Fatal Error -> Line : %d:%d A function declaration cannot be terminated by an ';'. A code block has to follow. Instruction : %s.\n", curr_file_name->stringa, loader->instr_line, loader->line_number, instr);
                                    ret = HDR_ERROR_LOADER_SYNTAX_ERROR;
                                    goto fail;
                                }

                                /*create the instruction and set the appropriate loader state*/
                                if (hdr_loader_h_create_func_decl(loader) == NULL)
                                {
                                    printf("[%s] Fatal Error -> Line : %d:%d The instruction for the function declaration was not created. %s.\n", curr_file_name->stringa, loader->instr_line, loader->line_number, instr);
                                    ret = HDR_ERROR_LOADER_SYNTAX_ERROR;
                                    goto fail;
                                }

                                /*add the name*/
                                loader->current_instr->left_side = hdr_complex_token_create(NULL, func_name);
                                free(func_name);
                                /*get the parameters*/
                                int status;
                                char* tsect = dxCopySectionFromStrSmart(&instr_indx, '(', ')', "`\"",1,&status);
                                if ((tsect == NULL)&&(status != COPY_SECTION_EMPTY))
                                {
                                    printf("[%s] Fatal Error -> Line : %d:%d The function does not have a parameters section. %s. Internal Error : %s\n", curr_file_name->stringa, loader->instr_line, loader->line_number, instr,hdr_error_code_to_str(status));
                                    ret = HDR_ERROR_LOADER_SYNTAX_ERROR;
                                    hdr_instruction_free(loader->current_instr);
                                    goto fail;
                                }

                                loader->current_instr->left_side->parameters = hdr_expression_list_create();
                                if (status != COPY_SECTION_EMPTY)
                                {
                                    char* sect = tsect;
                                    while (*sect != 0)
                                    {
                                        char sep = 0;
                                        char* var = dxGetNextWord(&sect, ",", "`\"", true, true, &sep);
                                        if (var[0] == 0)
                                        {
                                            free(var);
                                            var = NULL;
                                        }

                                        if (var == NULL)
                                        {
                                            /*free the instruction and exit*/
                                            printf("[%s] Fatal Error -> Line : %d:%d The parameters section of the function is malformed. Check if there is more ',' than needed %s.\n", curr_file_name->stringa, loader->instr_line, loader->line_number, instr);
                                            ret = HDR_ERROR_LOADER_SYNTAX_ERROR;
                                            hdr_instruction_free(loader->current_instr);
                                            goto fail;
                                        }

                                        /*check for validity*/
                                        if (var[0] != '$')
                                        {
                                            /*free the instruction and exit*/
                                            printf("[%s] Fatal Error -> Line : %d:%d The parameter %s of the function is invalid. Do you have prefixed the token with a '$' ?.\n", curr_file_name->stringa, loader->instr_line, loader->line_number, instr);
                                            ret = HDR_ERROR_LOADER_SYNTAX_ERROR;
                                            hdr_instruction_free(loader->current_instr);
                                            free(var);
                                            goto fail;
                                        }

                                        if (hdr_parser_check_name_permit_dollar(var) == false)
                                        {
                                            /*free the instruction and exit*/
                                            printf("[%s] Fatal Error -> Line : %d:%d The parameter %s of the function is invalid. Do you have use any weird character?. %s\n", curr_file_name->stringa, loader->instr_line, loader->line_number, var, instr);
                                            ret = HDR_ERROR_LOADER_SYNTAX_ERROR;
                                            hdr_instruction_free(loader->current_instr);
                                            free(var);
                                            goto fail;
                                        }

                                        /*create the parameter*/
                                        PHDR_EXPRESSION expr = hdr_expression_create();
                                        if (expr == NULL)
                                        {
                                            /*free the instruction and exit*/
                                            printf("[%s] Fatal Error -> Line : %d:%d Internal Error. Memory allocation error in expression creation. %s.\n", curr_file_name->stringa, loader->instr_line, loader->line_number, instr);
                                            ret = HDR_ERROR_LOADER_SYNTAX_ERROR;
                                            hdr_instruction_free(loader->current_instr);
                                            free(var);
                                            goto fail;

                                        }

                                        dx_string_createU(expr->value->name, var);
                                        free(var);
                                        hdr_expression_list_add(loader->current_instr->left_side->parameters, expr);
                                    }
                                }
                                free(tsect);
                                /*the loader has red all the parameters , set it up for the block*/
                                /*add the instruction to the functions*/
                                hdr_instructions_list_add(loader->current_func_list, loader->current_instr) ;
                                loader->state = hdr_func_open; /*loader expects to find a {} with instructions*/
                            }break;
                            case hdr_obj :
                            {
                                
                                /*check if its valid to declare an object*/
                                if (loader->declaration_mode != hdr_s_main_code)
                                {
                                    printf("[%s] Fatal Error -> Line : %d:%d An object prototype (class) is valid only in the main script code. check if you try to declare your object inside another object or a function : %s.\n", curr_file_name->stringa, loader->instr_line, loader->line_number, instr);
                                    ret = HDR_ERROR_LOADER_SYNTAX_ERROR;
                                    goto fail;
                                }

                                if (terminated == true)
                                {
                                    printf("[%s] Fatal Error -> Line : %d:%d An object prototype (class) declaration cannot be terminated by an ';'. A code block has to follow. Instruction : %s.\n", curr_file_name->stringa, loader->instr_line, loader->line_number, instr);
                                    ret = HDR_ERROR_LOADER_SYNTAX_ERROR;
                                    goto fail;
                                }


                                /*
                                  an object declaration. First retrieve the name.
                                  The entity has all the components of the declaration e.g obj myname 
                                */

                                char *class_name = entity->entity->stringa ;
                                class_name = class_name + 3                ;
                                if(*class_name == 0) 
                                {
                                    printf("[%s] Fatal Error -> Line : %d:%d The object declaration needs a class name ( obj somename{....} ). None was supplied.\n", curr_file_name->stringa, loader->instr_line, loader->line_number);
                                    ret = HDR_ERROR_LOADER_SYNTAX_ERROR;
                                    goto fail;
                                }

                                dxGoForwardWhileChars(&class_name, " \t")  ;
                                dxRightTrimFastChars(class_name, " \t")    ;
                                /*check the validity of the name*/
                                if (dxIsStrNameSafe(class_name) == false)
                                {
                                    printf("[%s] Fatal Error -> Line : %d:%d The object declaration class name is not valid. %s.\n", curr_file_name->stringa, loader->instr_line, loader->line_number, instr);
                                    ret = HDR_ERROR_LOADER_SYNTAX_ERROR;
                                    goto fail;
                                }

                                /*check if the object has been redeclared*/
                                if (hdr_h_class_exist(loader, class_name) == true)
                                {
                                    printf("[%s] Fatal Error -> Line : %d:%d The object declaration class name already exist. %s.\n", curr_file_name->stringa, loader->instr_line, loader->line_number, instr);
                                    ret = HDR_ERROR_LOADER_SYNTAX_ERROR;
                                    goto fail;
                                }
                                /*create the class object and add it in the list*/
                                if (hdr_loader_obj_prot_list_add(loader->objects, hdr_loader_h_create_o_class(loader, class_name)) == NULL)
                                {
                                    printf("[%s] Fatal Error -> Line : %d:%d Memory allocation error while creating object class. %s.\n", curr_file_name->stringa, loader->instr_line, loader->line_number, instr);
                                    ret = HDR_ERROR_LOADER_SYNTAX_ERROR;
                                    goto fail;
                                }
                                                            
                                /*the next step is for the object to store its own code and functions (in the block opening)*/

                            }break;
                            /********************************************************************/
                            /*hdr_ error is reserved for future use (or not! :D )*/
                            case hdr_error :
                            {
                                printf("[%s] Fatal Error -> Line : %d:%d Unknown token : %s.\n", curr_file_name->stringa, loader->instr_line, loader->line_number,entity->entity->stringa);
                                ret = HDR_ERROR_LOADER_INTERNAL_ERROR;
                                goto fail;
                            }break;
                        }
                    }
                    else
                    {
                        switch (ret)
                        {
                        case HDR_ERROR_LOADER_NEW_INSTR_WHILE_PENDING:
                        {
                            printf("[%s] Fatal Error -> Line : %d:%d Expected the completion of the instruction  '%s' but a new instruction '%s' was encountered. \n", curr_file_name->stringa, loader->instr_line, loader->line_number, prev_i->stringa, entity->entity->stringa);
                        }
                        break;

                        }
                        /*failed*/
                    fail:
                        free(tr);
                        if (instr != NULL) free(instr);
                        /*the loader->current_instr will be freed in the block deletion*/
                       // if (loader->current_instr != NULL) loader->current_instr = hdr_instruction_free(loader->current_instr);
                        hdr_entity_free(entity);
                        dx_string_free(prev_i);
                        return ret;
                    }


                    dx_string_createU(prev_i, entity->entity->stringa);
                    hdr_entity_free(entity);
                }//while

            }
            else
                if (ERROR_CODE == HDR_BLOCK_OPEN)
                {
                    /*
                      a block has been open. For this to be valid the instruction must be one of the following :
                      if , else , obj , func , switch , loop , obj
                    */

                    if (hdr_loader_h_handle_block(loader,prev_i,&indx) == false)
                    {
                        free(tr);
                        if (instr != NULL) free(instr);
                        printf("[%s] Fatal Error -> Line : %d:%d \n", curr_file_name->stringa, loader->instr_line,loader->line_number);
                        dx_string_free(prev_i);
                        return HDR_ERROR_LOADER_INTERNAL_ERROR;
                    }


                }
                else
                    if (ERROR_CODE == HDR_BLOCK_CLOSE)
                    {
                        /*reserved not used as the opening / closing of a block is handled in the opening '{'*/
                    }
                    else
                        if (ERROR_CODE == HDR_SCRIPT_END)
                        {
                            /*The script was ended , no more instructions to analyze!*/
                            free(tr);
                            if (instr != NULL) free(instr);
                            break;
                        }
            free(tr);
            dx_string_createU(prev_i, instr);
            if (instr != NULL) free(instr);
        }/*NO ERROR*/

    } /*while true*/

    dx_string_free(prev_i);

    return HDR_SUCCESS;

}

int hdr_loader_create_code_from_script(PHDR_LOADER loader, PHDR_INCL_SCRIPT script, bool is_main_script)
{
    /*
     Analyzes the text of the script and creates the appropriate structures
    */

    if (loader == NULL)  return HDR_ERROR_LOADER_IS_NULL;
    if (script == NULL)  return HDR_ERROR_INCL_SCRIPT_IS_NULL;

    char* indx = NULL ;
    if (is_main_script == true)
    {
        loader->curr_file_name = loader->main_name;
        loader->instr_line  = 0; /*always starts from the first line*/
        loader->line_number = 1; /*accummulate the lines*/
        loader->char_pos    = 0;
    }
    else
    {
        loader->curr_file_name = script->filename ;
        loader->instr_line  = 0; /*always starts from the first line*/
        loader->line_number = 1; /*accummulate the lines*/
        loader->char_pos    = 0;
    }

    loader->in_main_file = is_main_script ;

    return hdr_loader_create_code_from_string(loader,script->buf->buf);

    /*check if the script is ending with a invalid state*/
    if (loader->state != hdr_pending)
    {
        printf("[%s] Fatal Error -> Line %d An instruction was not terminated correctly. Specific loader state : %s.\n", script->buf->buf, loader->instr_line, hdr_h_return_loader_state_str(loader->state));
        return HDR_ERROR_LOADER_INTERNAL_ERROR;
    }

}


int hdr_loader_create_code(PHDR_LOADER loader)
{
 /*
   Load all the instructions from every script , create the function templates ,
   defines , object classes and construct the main code block
 */

    if (loader == NULL) return HDR_ERROR_LOADER_IS_NULL;
 /*
  first analyze the included files and then the main script file 
 */


    PDXL_NODE node = loader->scripts->start ;
    while (node != NULL)
    {
        PDXL_OBJECT  obj    = node->object ;
        PHDR_INCL_SCRIPT script = obj->obj     ;
        int status = hdr_loader_create_code_from_script(loader,script,false) ;
        if (status != HDR_SUCCESS) return status ;
        node = node->right ;
    }

    int status = hdr_loader_create_code_from_script(loader,loader->main_script,true) ;
   
    /*free the scripts, we do not need them anymore so we will free the memory*/

    node = loader->scripts->start;
    while (node != NULL)
    {
        PDXL_OBJECT   obj = node->object;
        PHDR_INCL_SCRIPT  script = obj->obj;
        hdr_incl_script_free(script);
        free(obj);
        node = node->right;
    }
    loader->scripts = dx_list_delete_list(loader->scripts);
    

    return status;
}





/**********************************************************************/




/****************************************************************************************************/

/********************DEBUG**************************************************************************/

void _DEBUG_PRINT_BLOCK(PHDR_BLOCK block, int ident);

char *_debug_ret_instr_name(PHDR_INSTRUCTION instr)
{
    if (instr == NULL) return "";
    switch (instr->type)
    {
    case hdr_i_uninit     : return "UNINITIALIZED";
    case hdr_i_if         : return "IF";
    case hdr_i_loop       : return "LOOP";
    case hdr_i_assign     : return "ASSIGN";
    case hdr_i_cmd        : return "CMD";
    case hdr_i_cmd_param  : return "CMD PARAM";
    case hdr_i_var_cmd    : return "CVAR";
    case hdr_i_code_block : return "BLOCK";
    case hdr_i_switch     : return "SWITCH";
    case hdr_i_switch_default: return "DEFAULT";
    case hdr_i_switch_case: return "CASE";
    case hdr_i_func_decl  : return "FUNC";
    }

    return NULL;
}

void _debug_print_ident(int ident)
{
    for (int i = 0; i < (ident + 4); i++)printf(" ");
}

void _DEBUG_PRINT_INSTRUCTION(PHDR_INSTRUCTION instr, int ident)
{
  /*prints the structure of an instruction */
  printf("\n");
  if (instr == NULL)
  {
      printf("NULL");
      return;
  }
  _debug_print_ident(ident);
  printf("%s ", _debug_ret_instr_name(instr));
  switch (instr->type)
  {
  case hdr_i_uninit: { printf(" ;\n"); }break;
  case hdr_i_if: 
  { 
      printf("("); 
      _DEBUG_PRINT_EXPRESSION_STR(instr->right_side);
      printf(")");
      printf("\n");
      _debug_print_ident(ident);
      printf("{\n");
      _debug_print_ident(ident);
      _DEBUG_PRINT_BLOCK( instr->code, ident + 2);
      printf("\n");
      _debug_print_ident(ident);
      printf("}\n");
      if (instr->ielse != NULL)
      {
          _debug_print_ident(ident+4);
          printf("else\n");
          _debug_print_ident(ident + 4);
          printf("{\n");
          _DEBUG_PRINT_BLOCK(instr->ielse->code, ident + 4);
          printf("\n");
          _debug_print_ident(ident + 4);
          printf("}\n");
          printf("\n");
      }
      printf("\n");

  }break;
  case hdr_i_switch:
  {
      printf("(");
      _DEBUG_PRINT_EXPRESSION_STR(instr->right_side);
      printf(")");
      printf("\n");
      _debug_print_ident(ident);
      printf("{\n");
      _debug_print_ident(ident);
      _DEBUG_PRINT_INSTRUCTION(instr->ielse, ident + 2);
      printf("\n");
      _debug_print_ident(ident);
      printf("}\n");

  }break;
  case hdr_i_switch_case:
  {
      printf(":");
      _DEBUG_PRINT_EXPRESSION_STR(instr->right_side);
      printf(" ");
      printf("\n");
      _debug_print_ident(ident);
      printf("{\n");
      _debug_print_ident(ident);
      _DEBUG_PRINT_BLOCK(instr->code, ident + 2);
      printf("\n");
      _debug_print_ident(ident);
      printf("}\n");
      _debug_print_ident(ident);
      _DEBUG_PRINT_INSTRUCTION(instr->ielse, ident + 2);
      printf("\n");

  }break;
  case hdr_i_switch_default:
  {
      printf("(");
      _DEBUG_PRINT_EXPRESSION_STR(instr->right_side);
      printf(")");
      printf("\n");
      _debug_print_ident(ident);
      printf("{\n");
      _debug_print_ident(ident);
      _DEBUG_PRINT_BLOCK(instr->code, ident + 2);
      printf("\n");
      _debug_print_ident(ident);
      printf("}\n");
      if (instr->ielse != NULL)
      {
          _debug_print_ident(ident + 4);
          printf("else\n");
          _debug_print_ident(ident + 4);
          printf("{\n");
          _DEBUG_PRINT_BLOCK(instr->ielse->code, ident + 4);
          printf("\n");
          _debug_print_ident(ident + 4);
          printf("}\n");
          printf("\n");
      }
      printf("\n");

  }break;
  case hdr_i_loop: 
  {
      printf("\n");
      _debug_print_ident(ident);
      printf("{\n");
      _debug_print_ident(ident);
      _DEBUG_PRINT_BLOCK(instr->code, ident + 2);
      printf("\n");
      _debug_print_ident(ident);
      printf("}");
  }break;
  case hdr_i_assign: 
  {
      printf("\n");
      _debug_print_ident(ident);
      printf("%s : ",instr->left_side->ID->stringa);
      if (instr->left_side->expression != NULL)
      {
          _debug_print_ident(ident);
          _DEBUG_PRINT_EXPRESSION_STR(instr->left_side->expression);
      }
      printf(" = ");
      _DEBUG_PRINT_EXPRESSION_STR(instr->right_side);
  }break;
  case hdr_i_cmd: 
  {
      printf("\n");
      _debug_print_ident(ident);
      _DEBUG_PRINT_EXPRESSION_STR(instr->right_side);
  }break;
  case hdr_i_var_cmd:
  {
      printf("\n");
      _debug_print_ident(ident);
      printf("%s : ", instr->left_side->ID->stringa);
      _debug_print_ident(ident);
      _DEBUG_PRINT_EXPRESSION_STR(instr->left_side->expression);
  }break;
  case hdr_i_cmd_param: 
  {
      printf("\n");
      _debug_print_ident(ident);
      printf(instr->left_side->ID->stringa);
      printf(" ");
      _DEBUG_PRINT_EXPRESSION_STR(instr->right_side);
  }break;
  case hdr_i_code_block: { printf("{reserved}\n"); }break;
  case hdr_i_func_decl :
  {
      printf(instr->left_side->ID->stringa);
      printf("(");
      PDXL_NODE node = instr->left_side->parameters->start;
      while (node != NULL)
      {
          PDXL_OBJECT obj = node->object;
          PHDR_EXPRESSION expr = (PHDR_EXPRESSION)obj->obj;
          printf("%s,",expr->value->name->stringa);
          node = node->right;
      }
      printf(")\n");
      _debug_print_ident(ident);
      printf("{");
      _DEBUG_PRINT_BLOCK(instr->code, ident+1);
      printf("\n");
      _debug_print_ident(ident);
      printf("}\n\n");
  }break;

  case hdr_i_obj_decl :
  {
      _DEBUG_PRINT_BLOCK(instr->code, ident+1);
  }break;

  }

  return;
}

void _DEBUG_PRINT_BLOCK(PHDR_BLOCK block,int ident)
{
    if (block == NULL)
    {
        _debug_print_ident(ident + 2);
        printf("NULL\n");
        return;
    }

    int pid = -1;
    if (block->parent != NULL)
    {
        pid = block->parent->ID;
    }

    printf("\n");
    _debug_print_ident(ident+2);
    printf("parent ID : %d ID : %d \n",pid,block->ID);
    PDXL_NODE node = block->instructions->start;
    while (node != NULL)
    {
        PDXL_OBJECT      obj    = node->object  ;
        PHDR_INSTRUCTION instr  = obj->obj      ;
        _DEBUG_PRINT_INSTRUCTION(instr, ident + 2);
        node = node->right;
    }

    return;
}

void _DEBUG_PRINT_CODE(PHDR_LOADER loader)
{
    if (loader == NULL)
    {
        printf("NULL Loader\n");
    }

    printf("---------- Code Dump. Total blocks : %d ----------\n\n",GLID);
    
    PDXL_NODE node = loader->code->instructions->start;
    while (node != NULL)
    {
        PDXL_OBJECT      obj = node->object;
        PHDR_INSTRUCTION instr = obj->obj;
        _DEBUG_PRINT_INSTRUCTION(instr, 1);
        node = node->right;
    }

    printf("\n\n--------- Function Declaration Dump -----------\n\n");
    node = loader->functions->start ;
    while (node != NULL)
    {
        PDXL_OBJECT      obj = node->object;
        PHDR_INSTRUCTION instr = obj->obj;
        _DEBUG_PRINT_INSTRUCTION(instr, 1);
        node = node->right;
    }

    printf("\n\n--------- Objects Declaration Dump -----------\n\n");
    node = loader->objects->start;
    while (node != NULL)
    {
        PDXL_OBJECT          obj   = node->object     ;
        PHDR_LOADER_OBJ_PROT class = obj->obj         ;
        _debug_print_ident(2);
        printf("obj %s\n", class->name->stringa);
        _debug_print_ident(2);
        printf("{\n");
        _debug_print_ident(2);
        _DEBUG_PRINT_INSTRUCTION(class->instr,3)      ;

        PDXL_NODE fnode = class->functions->start;
        while (fnode != NULL)
        {
            PDXL_OBJECT      obj = fnode->object;
            PHDR_INSTRUCTION instr = obj->obj;
            _DEBUG_PRINT_INSTRUCTION(instr, 6);
            fnode = fnode->right;
        }
        
        
        printf("\n");
        _debug_print_ident(2);
        printf("}\n\n\n");

        node = node->right;
    }

   

    return ;
}







