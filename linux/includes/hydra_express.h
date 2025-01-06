/*
 This header provides functions for analyze a Hydra+ expression 
 into an PHDR_EXPRESSION structure .

 Nikos Mourgis deus-ex.gr 2024

 Live Long and Prosper

*/

#define HYDRA_EXPRESS



/******************************************/
PHDR_EXPRESSION hdr_expr_create_from_str(char* expr,int *status);
PHDR_COMPLEX_TOKEN hdr_token_construct_multipart_token(char *sentence,int *status);
//PHDR_COMPLEX_TOKEN hdr_token_construct_function_token(char* sentence, int* status);
PHDR_COMPLEX_TOKEN hdr_token_create_simple_token(char* token, int* status);
bool hdr_expr_h_add_function(PHDR_EXPRESSION expression, char* sentence, char op, int* status) ;
/*****************************************/

/*
 helper function to transform an expression with operators based to the operators priority
 creates internally and returns a new string that must be freed by the caller  
 */


char* hdr_express_prepare_alg_expr(char* expression, int* status)
{
    char* calibrated_expr = AlgTransformExpression(expression, status);
    return calibrated_expr;
}

bool hdr_expr_check_alg_validity(int status, char* or_sentence)
{
    if (status != ALG_SUCCESS)
    {
        switch (status)
        {

        case  ALG_EXPR_NULL:
        {
            printf("Fatal Error -> Expression is NULL : [%s]   .\n", or_sentence);
            return false;
        }
        break;
        case  ALG_EXPR_EMPTY:
        {
            printf("Fatal Error -> Expression is Empty : [%s]   .\n", or_sentence);
            return false;
        }
        break;
        case  ALG_TOO_LONG_OPERANT:
        {
            printf("Fatal Error -> One or more operands are too long : [%s]   .\n", or_sentence);
            return false;
        }
        break;
        case  ALG_LIST_NULL:
        {
            printf("Fatal Error -> alg_parse list is NULL (why?) : [%s]   .\n", or_sentence);
            return false;
        }
        break;
        case  ALG_MISSPLACED_OPERATOR:
        {
            printf("Fatal Error -> A stray operator was found in the expression : [%s]   .\n", or_sentence);
            return false;
        }
        break;
        case  ALG_SYNTAX_ERROR:
        {
            printf("Fatal Error -> Malformed expression : [%s]   .\n", or_sentence);
            return false;
        }
        break;


        }

    }

    return true;
}

char* hdr_expression_prepare_priority(char* expr, int* status)
{
    char* expr_str = hdr_express_prepare_alg_expr(expr, status);/*the function checks for the validity of the () and [] too*/

    if (expr_str == NULL)
    {
        printf("\n%s", hdr_error_code_to_str(*status));
        return NULL;
    }

    if (hdr_expr_check_alg_validity(*status, expr_str) == false)
    {
        free(expr_str);
        return NULL;
    }

    return expr_str;
}
/*
 Helper function to retrieve entities
*/

PHDR_ENTITY hdr_expr_get_entity(DXLONG64 cline, char** instr_index)
{

    /*
      retrieve the next entity and makes all the preliminary validity checks.
      Any error in this function is a fatal error.
      The return value is a valid entity or the NULL value

      The instr_index will be point to the next character for analyze
    */

    /*retrieve entity*/
    int status;
    PHDR_ENTITY entity = hdr_parser_get_next_entity(instr_index,&status);

     
    if (status != HDR_SUCCESS)
    {
        printf("**System : Fatal Error -> Line : %d A misplased dot (.) was found in the instruction. Instruction : %s\n", cline, *instr_index);
        hdr_entity_free(entity);
        return NULL;
    }

    if (entity == NULL)
    {
        printf("**System : Fatal Error -> The Loader failed to allocate memory for the entity. Line : %d Instruction : %s\n\n",
        cline, *instr_index);
        return NULL;
    }
    else
        if (entity->too_long_entity == true)
        {
            printf("**System : Fatal Error -> The Loader found a very large entity. Line : %d Entity : %s\n", cline, entity->entity->stringa);
            hdr_entity_free(entity);
            return NULL;
        }
        else
            if ((entity->entity->len == 0) && (entity->separator->len == 0))
            {
                /*the end of the instruction*/
                hdr_entity_free(entity);
                return NULL;
            }
           

    return entity;
}




bool hdr_expr_check_section_validity(DXLONG64 cline, char* or_sentence,char* sep,int status)
{
    /*returns true if the status is not for an error or false if the status is for an error*/
    switch (status)
    {
        case COPY_SECTION_STRAY_OPEN:
        {
            printf("Fatal Error -> Line : %d  A stray '%s' was found in the expression : '%s'   .\n", cline,sep,or_sentence);
            return false;
        }break;
        case COPY_SECTION_STRAY_CLOSE:
        {
            printf("Fatal Error -> Line : %d  A stray '%s' was found in the expression : '%s'   .\n", cline, sep, or_sentence);
            return false;
        }break;
        case COPY_SECTION_NULL_P:
        {
            printf("Fatal Error -> Line : %d  Internal error : COPY_SECTION_NULL : '%s'   .\n", cline, or_sentence);
            return false;
        }break;
    }

    return true;
}




PHDR_COMPLEX_TOKEN hdr_token_create_simple_token(char* token, int* status)
{

    /*
      this function creates a simple named token , checks if it is a variable or a single name
      and setup the appropriate token
    */
    *status = HDR_SUCCESS;
    PHDR_COMPLEX_TOKEN ctoken = hdr_complex_token_create(NULL, token);
    if (ctoken == NULL)
    {
        *status = HDR_ERROR_TOKEN_NULL_P;
        return NULL;
    }
    /*
     check the type
    */

    if (token[0] == '$')
    {
        /*this is a variable, we will do the bare minimum validation*/
        if (hdr_parser_check_name((token + 1)) == false)
        {
            *status = HDR_ERROR_TOKEN_NAME_ERROR;
            return NULL;
        }

        ctoken->type = hdr_tk_variable;

        return ctoken;
    }
    else
        if ((token[0] == '`') || (token[0] == '"')) /*check for literal string*/
        {
            ctoken->type = hdr_tk_literal;
            /*set the val value*/
            if (token[0] == '`')
                ctoken->val->type = hvt_simple_string_bcktck;
            else
                ctoken->val->type = hvt_simple_string;

            ctoken->val->literal = true; /*this is not usefull in this context but i set it to be consistent*/
            /*remove string encapsulation*/
            token[strlen(token)-1] = 0;
            char* tmpstr = &(token[1]);
            ctoken->val->obj = (void*)dx_string_createU(NULL, tmpstr);

            /***********************************************************************************/
            /*we do not need the ID for a literal , free some of the memory*/
            dx_string_createU(ctoken->ID, "");
            return ctoken;
        }
        else
            if (dxIsStrReal(token) == true)/*check for literal number*/
            {
                ctoken->type = hdr_tk_literal;
                ctoken->val->type = hvt_float;
                ctoken->val->literal = true;
                char* cindx = NULL;
                ctoken->val->real = strtod(token, &cindx);
                /*we do not need the ID for a literal , free some of the memory*/
                dx_string_createU(ctoken->ID, "");
                return ctoken;
            }
            else
            {
                /*this is a simple word like [break]*/
                if (hdr_parser_check_name((token + 1)) == false)
                {
                    *status = HDR_ERROR_TOKEN_NAME_ERROR;
                    return NULL;
                }

                ctoken->type = hdr_tk_simple;

                return ctoken;
            }

    return NULL;
}


/*helping functions and enumerations*/
enum hdr_expr_sentence_type { hdr_s_literal_simple, hdr_s_function, hdr_s_complx_expr, hdr_s_indexed, hdr_s_multipart,hdr_s_error};
/*
hdr_s_literal_simple  : A literal , simple word , or variable token 
hdr_s_function        : A function
hdr_s_complx_expr     : An expression inside a parenthesis
hdr_s_indexed         : A token that is indexed like $var[`index`][0]
hdr_s_multipart       : A token that has multy parts
hdr_s_error           : Implies an error (reserved)
*/

enum hdr_expr_sentence_type hdr_return_strtoken_type(char* token)
{
    /*
     returns the type of the token 
    */

    /*first check for real numbers as they have the dot [.] in them and can confuse the function*/
    if(dxIsStrReal(token) == true) return hdr_s_literal_simple ;

    /*check first for multipart as it is a token that can have every other type of tokens (almost) in it */

    /*we have to check for dots in sections like () and []*/
    if (dxCharExistsInStrExt(token, '.', "`\"") != -1) return hdr_s_multipart;

    if (dxItsStrEncapsulatedExt(token, '(', ')',"`\"") == true) return hdr_s_complx_expr ;   
    
    /*
      check if the token is a function, check if there is a valid section of () and then we will copy the
      characters before the (). If its a valid hydra name then this is a function
    */
    if (dxCharExistsInStr(token, '(', "`\"") != -1)
    {
        /*there is a posibility this to be a function , check the name, the name CANNOT being a numbers only token and cannot starts with a number either*/
        if (dxIsCharNumber(*token) == true) goto not_a_func; /*nop this is not a function*/
        char *tok = token;
        while (*tok != '(')
        {
            if (dxCharIsLatinNumberOr_(*tok) == false) goto not_a_func ; /*nop this is not a function*/
            tok++;
        }

        return hdr_s_function ;
    }
    
    not_a_func: /*you fot to love the goto in C :D*/
    /*check for an indexed token*/

    if (dxCharExistsInStr(token, '[', "`\"") != -1)
    {
        /*
          there is a posibility this to be an indexed token.
        */
        
        char* tok = token;
        while (*tok != '[')
        {
            if ((dxCharIsLatinOrNumber(*tok) == false))
            {
                /*some times is valid the first character of the token to be the $ (in a fisrt part or stand alone variable)*/
                if ((tok == token) && (*tok == '&')) goto next ;
                return hdr_s_indexed ;
            }
            next:
            tok++;
        }

        return hdr_s_indexed ;
    }

    return hdr_s_literal_simple ;
}


bool hdr_expr_h_add_simple_and_op(PHDR_EXPRESSION expr , char *sentence, char op, int *status)
{
    /*simple token , check first if it is a literal*/
    if ((dxIsStrReal(sentence) == false) && (sentence[0] != '`') && (sentence[0] != '"'))
    if (hdr_parser_check_name_permit_dollar(sentence) == false)
    {
        *status = HDR_ERROR_TOKEN_NAME_ERROR;
        return false;
    }

    /*there is cases that the sentence is empty string , we will not add it to the expression*/
    PHDR_COMPLEX_TOKEN token = NULL;
    if (sentence[0] != 0)
    {
        PHDR_COMPLEX_TOKEN token = hdr_token_create_simple_token(sentence, status);
        if (token == NULL) return false;
        /*add it to the expression*/
        hdr_tokens_list_add_token(expr->tokens, token);
    }
    /*check if there is an operator*/
    if (op != 0)
    {
        char sop[2];
        sop[0] = op;
        sop[1] = 0;
        PHDR_COMPLEX_TOKEN tokensep = hdr_complex_token_create(NULL, sop);
        if (tokensep == NULL)
        {
            *status = HDR_ERROR_TOKEN_NULL_P;
            hdr_complex_token_free(token);
            return false;
        }
        tokensep->type = hdr_tk_operator;
        /*add it to the expression*/
        hdr_tokens_list_add_token(expr->tokens, tokensep);
    }
    
    return true;
}

bool hdr_expr_h_add_expression(PHDR_EXPRESSION expr, char *sentence, char op, int *status)
{
    /*an expression*/

    PHDR_COMPLEX_TOKEN token = hdr_complex_token_create(NULL, "expression");
    if (token == NULL) return false;
    /*strip the () else we wil have an infinite loop that will crash the machine */
    sentence[strlen(sentence)-1] = 0;
    sentence++;
    /*analyze and create the expression*/
    token->expression = hdr_expr_create_from_str(sentence , status) ;
    if (token->expression == NULL)
    {
        hdr_complex_token_free(token);
        return false ;
    }
    token->type = hdr_tk_expression ;

    /*add it to the expression*/
    hdr_tokens_list_add_token(expr->tokens, token) ;
    /*check if there is an operator*/
    if (op != 0)
    {
        char sop[2];
        sop[0] = op;
        sop[1] = 0;
        PHDR_COMPLEX_TOKEN tokensep = hdr_complex_token_create(NULL, sop);
        if (tokensep == NULL)
        {
            *status = HDR_ERROR_TOKEN_NULL_P;
            hdr_complex_token_free(token);
            return false;
        }
        tokensep->type = hdr_tk_operator;
        /*add it to the expression*/
        hdr_tokens_list_add_token(expr->tokens, tokensep);
    }

    return true;
}

PHDR_COMPLEX_TOKEN hdr_token_create_function_token(char* sentence, int* status)
{

    char* fname = dxCopyStrToChar(&sentence, '(', "`\"");
    if (fname == NULL)
    {
        *status = HDR_ERROR_NAME_NULL;
        return false;
    }

    PHDR_COMPLEX_TOKEN func = hdr_complex_token_create(NULL, fname);
    if (func == NULL)
    {
        free(fname);
        return false;
    }
    free(fname);
    func->type = hdr_tk_function;

    /*a function can have zero or multiple parameters. Every parameter will be handled as an expression*/

    func->parameters = hdr_expression_list_create();
    if (func->parameters == NULL)
    {
        hdr_complex_token_free(func);
        *status = HDR_ERROR_TOKEN_NULL_P;
        return false;
    }

    /*trim the ()*/
    char *sent_end = &sentence[strlen(sentence)];
    /*check if there is more encapsulation and remove it*/
    while (dxItsStrEncapsulatedExt(sentence, '(', ')',"`\"") == true)
    {
        if (sent_end == sentence) break; /*no string*/
        sent_end--;
        *sent_end = 0;
        sentence++;
    }

    /*get all the parameters and store them*/
    while (*sentence != 0)
    {
        char sep;/*reserved not used for this function*/
        char* expr = dxGetNextWord(&sentence, ",", "`\"", true, true, &sep);
        if (expr == NULL)
        {
            *status = HDR_ERROR_EXPR_STR_NULL;
            goto fail;

        }

        if (expr[0] == 0)
        {
            free(expr);
            *status = HDR_ERROR_EXPR_STR_EMPTY;
            goto fail;
        }


        char* prepared_expr = hdr_expression_prepare_priority(expr, status);
        if (prepared_expr == NULL)
        {
            free(expr);
            goto fail;
        }


        PHDR_EXPRESSION expression = hdr_expr_create_from_str(prepared_expr, status);
        if (*status != HDR_SUCCESS)
        {
            free(expr);
            free(prepared_expr);
            goto fail;
        }

        hdr_expression_list_add(func->parameters, expression);
        free(expr);
        free(prepared_expr);
    }

  
    return func;
fail:
    hdr_complex_token_free(func);
    return NULL;

}

bool hdr_expr_h_add_function(PHDR_EXPRESSION expression, char* sentence, char op, int* status)
{
    /*a function*/

    /*get the function name*/
    char* fname = dxCopyStrToChar(&sentence, '(', "`\"") ;
    if (fname == NULL)
    {
        *status = HDR_ERROR_NAME_NULL;
        return false ;
    }

    PHDR_COMPLEX_TOKEN func = hdr_complex_token_create(NULL, fname);
    if (func == NULL)
    {
        free(fname) ;
        return false;
    }
    free(fname);
    func->type = hdr_tk_function;

   /*a function can have zero or multiple parameters. Every parameter will be handled as an expression*/

    func->parameters = hdr_expression_list_create();
    if (func->parameters == NULL)
    {
        hdr_complex_token_free(func);
        *status = HDR_ERROR_TOKEN_NULL_P;
        return false ;
    }

    /*trim the ()*/
    sentence[strlen(sentence) - 1] = 0;
    sentence++;

    /*get all the parameters and store them*/
    while (*sentence != 0)
    {
        char sep ;/*reserved not used for this function*/
        char* expr = dxGetNextWord(&sentence, ",", "`\"", true, true, &sep);
        if (expr == NULL)
        {
            *status = HDR_ERROR_EXPR_STR_NULL;
            goto fail;
            
        }

        if (expr[0] == 0)
        {
            free(expr);
            *status = HDR_ERROR_EXPR_STR_EMPTY;
            goto fail;
        }

        char* prepared_expr = hdr_expression_prepare_priority(expr, status);
        if (prepared_expr == NULL)
        {
            free(expr);
            goto fail;
        }

        PHDR_EXPRESSION expression = hdr_expr_create_from_str(prepared_expr, status);
        if (*status != HDR_SUCCESS)
        {
            free(expr) ;
            free(prepared_expr);
            goto fail  ;
        }

        hdr_expression_list_add(func->parameters, expression);
        free(expr);
        free(prepared_expr);
    }

    /*add the function in the expression*/
    hdr_tokens_list_add_token(expression->tokens, func);
    /*check if there is an operator to add*/
    if (op != 0)
    {
        char sop[2];
        sop[0] = op;
        sop[1] = 0;
        PHDR_COMPLEX_TOKEN tokensep = hdr_complex_token_create(NULL, sop);
        if (tokensep == NULL)
        {
            *status = HDR_ERROR_TOKEN_NULL_P;
            goto fail;
        }
        tokensep->type = hdr_tk_operator;
        /*add it to the expression*/
        hdr_tokens_list_add_token(expression->tokens, tokensep);
    }
    return true;
    fail:
        hdr_complex_token_free(func);
        return false;
}

PHDR_COMPLEX_TOKEN hdr_create_indexed_token(char *sentence,int *status)
{
    /*
      a token with indexes.
      A token can have many complicate indexes. Example : $var[5+6/2][func1(`param`)][0]
      The function assumes that the '[]' sections are matched correctly
     */
    *status = HDR_SUCCESS;
    /*get the token*/
    char* tname = dxCopyStrToChar(&sentence, '[', "`\"");
    if (tname == NULL)
    {
        *status = HDR_ERROR_NAME_NULL;
        return NULL;
    }

    PHDR_COMPLEX_TOKEN token = hdr_complex_token_create(NULL, tname);
    if (token == NULL)
    {
        free(tname);
        return NULL;
    }
    free(tname);
    token->type = hdr_tk_ref_brackets;

    /*add the expression of the brackets*/
    char* section = dxCopySectionFromStr(&sentence, '[', ']', "`\"", status);
    if ((*status != COPY_SECTION_SUCCESS) && (*status != COPY_SECTION_EMPTY)) goto fail;
    char* prepared_expr = NULL ;

    if(*status != COPY_SECTION_EMPTY)
    {

        prepared_expr = hdr_expression_prepare_priority(section, status);
        if (prepared_expr == NULL)
        {
            free(section);
            goto fail;
        }

        token->expression = hdr_expr_create_from_str(prepared_expr, status);
    }
    else
    {
         token->expression = NULL ;
    }

    free(section);
    free(prepared_expr);

    /*get every index*/
    PHDR_COMPLEX_TOKEN current_index = token;
    while (*sentence != 0)
    {
        /*make a syntax error check , in an indexed token we cannot have any string prefixed the []*/
        if (*sentence != '[')
        {
            *status = HDR_ERROR_STRAY_CHARS_BEFORE_INDEX;
            goto fail;
        }
        char* section = dxCopySectionFromStr(&sentence, '[', ']', "`\"", status);
        if ((*status != COPY_SECTION_SUCCESS) && (*status != COPY_SECTION_EMPTY)) goto fail;
        /*
          do a syntax error check, if the previous index has NULL in the expression then its illegal
          to have another index after. So the Hydra+ does not supports $var[][][][] = 3 ;
          you have to $var[] = List.Create();
          $var[0][] = List.Create(); e.t.c
        */

        if (current_index->type == hdr_tk_index)
        {
            if (current_index->expression == NULL)
            {
                /*yap, not supported*/
                free(section);
                *status = HDR_ERROR_EMPTY_INDEX_BEFORE_CURRENT;
                goto fail;
            }
        }

        /*create the token for the index*/
        current_index->next_part = hdr_complex_token_create(token, "indx[]");
        if (current_index->next_part == NULL)
        {
            free(section);
            goto fail;
        }
        current_index->next_part->previous_part = current_index;/*set the chain links*/

        current_index = current_index->next_part;/*set the current index to the new created token*/
        /*set the type*/
        current_index->type = hdr_tk_index;
        /*check if the index is in fact empty or with an expression*/
        if (section != NULL)
        {

            char* prepared_expr = hdr_expression_prepare_priority(section, status);
            if (prepared_expr == NULL)
            {
                free(section);
                goto fail;
            }

            current_index->expression = hdr_expr_create_from_str(prepared_expr, status);
            if (current_index->expression == NULL)
            {
                free(section);
                free(prepared_expr);
                /*The current_index will be freed in the hdr_complex_token_free(token) after the fail*/
                goto fail;
            }

            free(prepared_expr);
        }
        free(section);
    }

    return token;

fail:
    hdr_complex_token_free(token);
    return NULL;

}

bool hdr_expr_h_add_indexed(PHDR_EXPRESSION expr,char* sentence, char op, int* status)
{
    /*
     a token with indexes.
     A token can have many complicate indexes. Example : $var[5+6/2][func1(`param`)][0] 
     The function assumes that the '[]' sections are matched correctly 
    */
    *status = HDR_SUCCESS ;
    /*get the token*/
    char* tname = dxCopyStrToChar(&sentence, '[', "`\"");
    if (tname == NULL)
    {
        *status = HDR_ERROR_NAME_NULL;
        return false;
    }

    PHDR_COMPLEX_TOKEN token = hdr_complex_token_create(NULL, tname);
    if (token == NULL)
    {
        free(tname);
        return false;
    }
    free(tname);
    token->type = hdr_tk_ref_brackets ;
    /*add the expression of the brackets*/
    char* section = dxCopySectionFromStr(&sentence, '[', ']', "`\"", status);
    if ((*status != COPY_SECTION_SUCCESS) && (*status != COPY_SECTION_EMPTY)) goto fail;
    
    char *prepared_expr = NULL ;

    if (section == NULL)
    {
        /*the expression is empty*/
        prepared_expr = (char*)malloc(1) ;
        prepared_expr[0] = 0 ;
    }
    else prepared_expr =  hdr_expression_prepare_priority(section, status);
    
    if (prepared_expr == NULL)
    {
        free(section);
        goto fail;
    }

    token->expression = hdr_expr_create_from_str(prepared_expr, status) ;
    free(section) ;
    free(prepared_expr);
    if(*status != HDR_SUCCESS )
    {
         goto fail;
    }
    /*get every index*/
    PHDR_COMPLEX_TOKEN current_index = token ;
    while (*sentence != 0)
    {
        /*make a syntax error check , in an indexed token we cannot have any string prefixed the []*/
        if (*sentence != '[')
        {
            *status = HDR_ERROR_STRAY_CHARS_BEFORE_INDEX;
            goto fail;
        }

        char* section = dxCopySectionFromStr(&sentence, '[', ']', "`\"", status);
        if ((*status != COPY_SECTION_SUCCESS)&&(*status != COPY_SECTION_EMPTY)) goto fail ;
       /*
         do a syntax error check, if the previous index has NULL in the expression then its illegal
         to have another index after. So the Hydra+ does not supports $var[][][][] = 3 ;
         you have to $var[] = List.Create();
         $var[0][] = List.Create(); e.t.c
       */
        
        if (current_index->type == hdr_tk_index)
        {
            if (current_index->expression == NULL)
            {
                /*yap, not supported*/
                free(section);
                *status = HDR_ERROR_EMPTY_INDEX_BEFORE_CURRENT;
                goto fail;
            }
        }

        /*create the token for the index*/
        current_index->next_part = hdr_complex_token_create(token, "indx[]");
        if (current_index->next_part == NULL)
        {
            free(section);
            goto fail;
        }
        current_index->next_part->previous_part = current_index;/*set the chain links*/

        current_index = current_index->next_part;/*set the current index to the new created token*/
        /*set the type*/
        current_index->type = hdr_tk_index;
        /*check if the index is in fact empty or with an expression*/
        if (section != NULL)
        {
            char* prepared_expr = hdr_expression_prepare_priority(section, status);
            if (prepared_expr == NULL)
            {
                free(section);
                goto fail;
            }

            current_index->expression = hdr_expr_create_from_str(prepared_expr, status);
            if (current_index->expression == NULL)
            {
                free(section);
                free(prepared_expr);
                /*The current_index will be freed in the hdr_complex_token_free(token) after the fail*/
                goto fail ;
            }
            free(prepared_expr);
        }
        free(section);
    }

    /*indexes was created add the token in the expression*/
    hdr_tokens_list_add_token(expr->tokens, token);
    /*check if there is an operator to add*/
    if (op != 0)
    {
        char sop[2];
        sop[0] = op;
        sop[1] = 0;
        PHDR_COMPLEX_TOKEN tokensep = hdr_complex_token_create(NULL, sop);
        if (tokensep == NULL)
        {
            *status = HDR_ERROR_TOKEN_NULL_P;
            goto fail;
        }
        tokensep->type = hdr_tk_operator;
        /*add it to the expression*/
        hdr_tokens_list_add_token(expr->tokens, tokensep);
    }

    return true;
    
fail:
    hdr_complex_token_free(token);
    return false;
}

bool hdr_expr_h_add_multipart(PHDR_EXPRESSION expr, char* sentence, char op, int* status)
{
    /*
     a token with multiple parts
    */
    *status = HDR_SUCCESS;
    /*the token can have many types of token. First find the base token*/
    char sep ;
    char* tname = dxGetNextWord(&sentence, ".", "`\"", true, true, &sep);
    /*
     check if the token is an indexed token or a simple token
    */
    enum hdr_expr_sentence_type ttype = hdr_return_strtoken_type(tname);
    PHDR_COMPLEX_TOKEN  root        = NULL ;
    PHDR_COMPLEX_TOKEN  curr_token  = NULL ;
    
    if (ttype == hdr_s_indexed)
    {
        root = hdr_create_indexed_token(tname, status);
        free(tname);
        if (root == NULL) return false ;
        /*find the last index to the chain*/
        curr_token = root;
        while (curr_token->next_part != NULL)
        {
            curr_token = curr_token->next_part ;
        }
    }
    else
        if (ttype == hdr_s_literal_simple)
        {
            /*check if the name is valid for a token*/
            if (hdr_parser_check_name_permit_dollar(tname) == false)
            {
                free(tname) ;
                *status = HDR_ERROR_TOKEN_NAME_ERROR ;
                return false ;
            }
            root = hdr_token_create_simple_token(tname, status);
            free(tname);
            if (root == NULL) return false ;
            curr_token = root ;
        }
        else
        {
            /*Hydra+ does not suports syntax like $var.(func(15)).do_something() or func1($var1).printHello() ;*/
            free(tname);
            *status = HDR_ERROR_TOKEN_TYPE_FOR_PART_ROOT ; 
            return false ;
        }
  
    root->type = hdr_tk_multipart;
    /*root is ready , keep in mind that to an indexed token the '.some_part' belongs TO THE LAST INDEX in the chain */
    /*now that the curr_token is set we will retrieve all the parts and add them in the token*/
    while (*sentence != 0)
    {
        /*
          we do not allow a syntax as $var.func1().a_token as this would lead to memory leaks because Hydra+ 
          does not have a memory manager and the programmer is responsible to deallocate the memory after allocated it.
          So we will check the type of the curr_token 
        */

        if (curr_token->type == hdr_tk_function)
        {
            *status = HDR_ERROR_TOKEN_FUNCTION_CANNOT_HAVE_PART;
            goto fail ;
        }

        char sep;
        char* tname = dxGetNextWord(&sentence, ".", "`\"", true, true, &sep);
        /*we do not permit the syndax of $var.[$index] so we have to check for it*/
        if (tname == NULL)
        {
            *status = HDR_ERROR_NAME_NULL ;
            goto fail;
        }

        if (tname[0] == '[')
        {
            *status = HDR_ERROR_BRACKET_AFTER_DOT;
            goto fail;
        }

        enum hdr_expr_sentence_type ttype = hdr_return_strtoken_type(tname);
        if (ttype == hdr_s_indexed)
        {
            PHDR_COMPLEX_TOKEN part_token = hdr_create_indexed_token(tname, status);
            free(tname);
            if (part_token == NULL) goto fail ;
            /*attach it to the current token*/
            part_token->root = root    ; /*the root of it is the first part of the multipart token*/
            part_token->previous_part = curr_token ;
            curr_token->next_part     = part_token ;
            curr_token = part_token ;
            while (curr_token->next_part != NULL)
            {
                curr_token = curr_token->next_part;
                curr_token->root = root; /*even the parts of the indexed token will have as root the first part token and not the indexed token*/
            }
        }
        else
            if (ttype == hdr_s_literal_simple)
            {
                /*check if the name is valid for a token*/
                if (hdr_parser_check_name_permit_dollar(tname) == false)
                {
                    free(tname);
                    *status = HDR_ERROR_TOKEN_NAME_ERROR;
                    goto fail;
                }
                PHDR_COMPLEX_TOKEN part_token = hdr_token_create_simple_token(tname, status);
                free(tname);
                if (part_token == NULL) goto fail  ;
                part_token->root = root; /*the root of it is the first part of the multipart token*/
                part_token->previous_part = curr_token;
                curr_token->next_part = part_token;
                curr_token = part_token;
            }
            else
                if (ttype == hdr_s_function)
                {
                    PHDR_COMPLEX_TOKEN part_token = hdr_token_create_function_token(tname, status);
                    free(tname);
                    if (part_token == NULL) goto fail ;
                    part_token->root = root; /*the root of it is the first part of the multipart token*/
                    part_token->previous_part = curr_token;
                    curr_token->next_part = part_token;
                    curr_token = part_token;
                }
            else
            {
                /*Hydra+ does not suports syntax like $var.(func(15)).do_something() or func1($var1).printHello() ;*/
                free(tname);
                *status = HDR_ERROR_TOKEN_TYPE_FOR_PART_ROOT;
                goto fail;
            }
    }


    /*add the token in the expression*/
    hdr_tokens_list_add_token(expr->tokens, root);
    /*check if there is an operator to add*/
    if (op != 0)
    {
        char sop[2];
        sop[0] = op;
        sop[1] = 0;
        PHDR_COMPLEX_TOKEN tokensep = hdr_complex_token_create(NULL, sop);
        if (tokensep == NULL)
        {
            *status = HDR_ERROR_TOKEN_NULL_P;
            goto fail;
        }
        tokensep->type = hdr_tk_operator;
        /*add it to the expression*/
        hdr_tokens_list_add_token(expr->tokens, tokensep);
    }
    return true;

    fail :
    hdr_complex_token_free(root);
    return false;

}

/************************************/

PHDR_EXPRESSION hdr_expr_create_from_str(char* expr, int *status)
{
   /*
     This function accepts a string that describes an expression (not boolean)
     and creates a PHDR_EXPRESSION object.
     A valid expression can have the following operators :
     -*+/%^ AND a special operator the [@] that is used into async and other special cases
     and as operands can have literals (strings and numbers), tokens (single or multipart),expressions in parenthesis,
     or functions.
     
     Remarks: The function expects the expr to be calibrated with parenthesis for the operators priority 
   */

   /*prepare the expression*/
    *status = HDR_SUCCESS;
    char* expr_str = expr;
    PHDR_EXPRESSION expression = hdr_expression_create();

    while (*expr_str != 0)
    {
        /*
         copy next valid sentence
         the @ is used for the async amd the < is use for the detach, thats why are in the operators

        */
        char sep;
        char* tsentence = dxGetNextWord(&expr_str, "+-/*%^@<", "`\"", true, true, &sep);/*free the sentence in this operation to avoid mem leaks*/
        /*fast trim sentence */
        char *sentence = tsentence;
        dxGoForwardWhileChars(&sentence, " \t");
        dxRightTrimFastChars(sentence, " \t");
        /*check for NULL responses*/
        if ((sentence == NULL) && (sep == 0)) break;
        /*
         The sentence can be anything , from a literal to a function or a complex expression (some expression).
        */

        switch (hdr_return_strtoken_type(sentence))
        {
           case hdr_s_literal_simple:
           {
                /*a simple token , create it and add it to the expression*/
               if (hdr_expr_h_add_simple_and_op(expression, sentence, sep, status) == false)
               {
                   free(tsentence);
                   goto fail;
               }
              
           }
               break;
           case hdr_s_function      :
           {
               /*a function , create it as a token and add it to the expression*/
               if (hdr_expr_h_add_function(expression, sentence, sep, status) == false)
               {
                   free(tsentence);
                   goto fail;
               }
           }
               break;
           case hdr_s_complx_expr   :
           {
               /*an expression inside parenthesis , create it as a token and add it to the expression*/
               if (hdr_expr_h_add_expression(expression, sentence, sep, status) == false)
               {
                   free(tsentence);
                   goto fail;
               }
           }
               break;
           case hdr_s_indexed       :
           {
               /*a token that has indexes */
               if (hdr_expr_h_add_indexed(expression, sentence, sep, status) == false)
               {
                   free(tsentence);
                   goto fail;
               }
           }
               break;
           case hdr_s_multipart     :
           {
               /*a multipart token */
               if (hdr_expr_h_add_multipart(expression, sentence, sep, status) == false)
               {
                   free(tsentence);
                   goto fail;
               }
           }
               break;

        }

        free(tsentence);

    }

    *status = HDR_SUCCESS;
    return expression;

    fail :
    hdr_expression_free(expression) ; 
    return NULL;
}


/*
 This function creates an expression from a string .
*/
PHDR_EXPRESSION  hdr_expr_create_expr( DXLONG64 cline , char* instr_indx)
{
    if (instr_indx == NULL) return NULL ;

	/*
      The function creates a non boolean expression from a string that represents
      a calculation or concatenation.
    */
    int status = 0 ; 
    /*
     do the operators priority preparing for all the operations EXCEPT the expressions that are inside [] and in the functions ()
     the above cases will be handled diferently in the code
    */
    char* expr_str = hdr_expression_prepare_priority(instr_indx, &status);

    if (expr_str == NULL)
    {
        printf("Fatal Error -> Line : %d An expression analysis failed. Instruction : '%s' Internal message : %s\n", cline, instr_indx, hdr_error_code_to_str(status));
        goto fail;
    }

    char *expr_str_indx = expr_str;
    if (dxItsStrEncapsulatedExt(expr_str_indx, '(', ')',"`\"") == true)
    {
        /*trim the first ( and the last )*/
        expr_str_indx[strlen(expr_str_indx) - 1] = 0;
        expr_str_indx = expr_str_indx + 1;
    }
   


    PHDR_EXPRESSION expr = hdr_expr_create_from_str(expr_str_indx,&status) ;

    if (status != HDR_SUCCESS)
    {
        printf("Fatal Error -> Line : %d An expression analysis failed. Instruction : '%s' Internal message : %s\n", cline, instr_indx, hdr_error_code_to_str(status));
        goto fail;
    }
    free(expr_str);
    return expr;
fail:
   free(expr_str);
    return NULL;

}



/************************DEBUG**************************/

void _DEBUG_PRINT_EXPRESSION_STR(PHDR_EXPRESSION expr);
void _DEBUG_PRINT_EXPRESSION_STRUCT(PHDR_EXPRESSION expr, int level,bool add_brackets );



void _debug_print_indexed_token(PHDR_COMPLEX_TOKEN token)
{
    /*print root */
    printf("%s[", token->ID->stringa);
    _DEBUG_PRINT_EXPRESSION_STR(token->expression);
    printf("]");
    /*the free indexes if exists will be printed in the caller as all the tokens are in serial form*/
    return;
}

void _debug_print_literal(PHDR_COMPLEX_TOKEN token)
{
    PHDR_VAR val = token->val ;
    
    if (val->type == hvt_float)
    {
        printf("%.8f", val->real);
    } else
        if (val->type == hvt_simple_string)
        {
            PDX_STRING str = (PDX_STRING)val->obj ;
            printf("\"%s\"",str->stringa);
        }else
            if (val->type == hvt_simple_string_bcktck)
            {
                PDX_STRING str = (PDX_STRING)val->obj;
                printf("`%s`", str->stringa);
            }

}

void _debug_print_function(PHDR_COMPLEX_TOKEN token)
{

    printf("%s(", token->ID->stringa);

    PDXL_NODE node  = token->parameters->start;
    while (node != NULL)
    {
        PDXL_OBJECT     obj  = node->object;
        PHDR_EXPRESSION expr = (PHDR_EXPRESSION)obj->obj;
        _DEBUG_PRINT_EXPRESSION_STR(expr);
        printf(",");
        node = node->right;
    }

    printf(")");

    return;
}

void _debug_print_multipart(PHDR_COMPLEX_TOKEN token)
{
    printf(token->ID->stringa);
    /*check if the multipart token is an indexed one*/
    if (token->expression != NULL)
    {
        /*an indexed token*/
        printf("[");
        _DEBUG_PRINT_EXPRESSION_STR(token->expression);
        printf("]");
    }
  
    /*print the parts*/
    PHDR_COMPLEX_TOKEN curr_token = token ;
    while (curr_token != NULL)
    {
        /*the parts can be simple tokens ,functions or indexed tokens */
        switch (curr_token->type)
        {
            case hdr_tk_function :
            {
                printf(".");
                _debug_print_function(curr_token);
            }break;
            case hdr_tk_ref_brackets :
            {
                printf(".");
                _debug_print_indexed_token(curr_token);
            }break;
            case hdr_tk_index:
            {
                /*a "free" index of the token like $test[3]'[2]'*/
                printf("[");
                _DEBUG_PRINT_EXPRESSION_STR(curr_token->expression);
                printf("]");
            }break;
            case hdr_tk_simple : 
            {
                printf(".");
                printf(curr_token->ID->stringa);
            }break;
        }


        curr_token = curr_token->next_part;
    }


    return;
}

void _DEBUG_PRINT_EXPRESSION_STR(PHDR_EXPRESSION expr )
{
    /*parse all the nodes of an expression and print them*/
    if (expr == NULL)
    {
        printf("NULL");
        return ;
    }
   
            PDXL_NODE node = expr->tokens->start;
            while (node != NULL)
            {
                PDXL_OBJECT obj = node->object;
                PHDR_COMPLEX_TOKEN token = obj->obj;
                switch (token->type)
                {
                   case hdr_tk_uninit: { printf("'UNDEFINED'"); } break;
                   case hdr_tk_ref_brackets: { _debug_print_indexed_token(token); }break;
                   case hdr_tk_multipart: { _debug_print_multipart(token); }break;
                   case hdr_tk_simple: { printf(token->ID->stringa); }break;
                   case hdr_tk_function: { _debug_print_function(token); }break;
                   case hdr_tk_literal: { _debug_print_literal(token); }break;
                   case hdr_tk_operator: { printf(token->ID->stringa); }break;
                   case hdr_tk_expression: { printf("("); _DEBUG_PRINT_EXPRESSION_STR(token->expression); printf(")"); }break;
                   case hdr_tk_variable: { printf(token->ID->stringa); }break;
                   case hdr_tk_bool_expression: { _DEBUG_PRINT_EXPRESSION_STR(token->expression); }break;
                   case hdr_tk_comp_expression: { _DEBUG_PRINT_EXPRESSION_STR(token->expression); }break;
                }
                node = node->right;
            }


    return;
}
/****************************************************************/
void _debug_add_spaces(int num)
{
    for (int i = 0; i < num; i++)printf(" ");
    return;
}

char *_debug_return_token_type_str(PHDR_COMPLEX_TOKEN token)
{
    switch (token->type)
    {
        case hdr_tk_uninit:         { return "uninitialized"; } break;
        case hdr_tk_ref_brackets:   { return "indexed"; }break;
        case hdr_tk_multipart:      { return "multipart"; }break;
        case hdr_tk_simple:         { return "simple"; }break;
        case hdr_tk_function:       { return "function"; }break;
        case hdr_tk_literal:        { return "literal"; }break;
        case hdr_tk_operator:       { return "operator"; }break;
        case hdr_tk_expression:     { return "expression"; }break;
        case hdr_tk_variable:       { return "variable"; }break;
        default: { return "not supported"; }break;
    }
    return NULL;
}

char* _debug_return_token_val_str(PHDR_COMPLEX_TOKEN token)
{
    PHDR_VAR val = token->val;

    if (val->type == hvt_float)
    {
        PDX_STRING str = dx_FloatToStr(val->real, 4);
        char *retstr = (char*)malloc(str->bcount+1) ;
        retstr[str->bcount] = 0;

        for (int i = 0; i < str->bcount; i++)
            retstr[i] = str->stringa[i];
        dx_string_free(str);
        return retstr;

    }
    else
        if (val->type == hvt_simple_string)
        {
            PDX_STRING str = (PDX_STRING)val->obj;
            char *retstr = (char*)malloc(str->bcount + 1+2);
            retstr[str->bcount + 1] = '\'';
            retstr[str->bcount+2] = 0;

            retstr[0] = '\'';
            for (int i = 0; i < str->bcount; i++)
                retstr[i+1] = str->stringa[i];

            return retstr;
        }
        else
            if (val->type == hvt_simple_string_bcktck)
            {
                PDX_STRING str = (PDX_STRING)val->obj;

                char *retstr = (char*)malloc(str->bcount + 1 + 2);
                retstr[str->bcount + 1] = '`';
                retstr[str->bcount + 2] = 0;

                retstr[0] = '`';
                for (int i = 0; i < str->bcount; i++)
                    retstr[i + 1] = str->stringa[i];

                return retstr;
            }

    return NULL;
}


void _debug_print_indexed_token_struct(PHDR_COMPLEX_TOKEN token, int level)
{
    /*print root */
    level++;
    _debug_add_spaces(level);
    printf("\"indexed\":{\"name\" : \"%s\",\"indx\" : \n ", token->ID->stringa);
    _DEBUG_PRINT_EXPRESSION_STRUCT(token->expression,level,true);
    printf("},");
    return;
}

void _debug_print_function_struct(PHDR_COMPLEX_TOKEN token, int level)
{
    level++;
    printf("\"func\":{ \"name\" : \"%s\" , \"parameters\" : {", token->ID->stringa);

    PDXL_NODE node = token->parameters->start;
    while (node != NULL)
    {
        printf("\n\"param\":");
        PDXL_OBJECT     obj = node->object;
        PHDR_EXPRESSION expr = (PHDR_EXPRESSION)obj->obj;
        _DEBUG_PRINT_EXPRESSION_STRUCT(expr,level,true);
        printf(",");
        node = node->right;
    }

    printf("}}\n");

    return;
}

void _debug_print_multipart_struct(PHDR_COMPLEX_TOKEN token, int level)
{
    level++;
    _debug_add_spaces(level);
    printf("\"multipart\" : { \"name\" : \"%s\"," , token->ID->stringa);
    /*check if the multipart token is an indexed one*/
    if (token->expression != NULL)
    {
        /*an indexed token*/
        printf(" ,\"index\" : ");
        _DEBUG_PRINT_EXPRESSION_STRUCT(token->expression,level,true);
    }

    /*print the parts*/
    PHDR_COMPLEX_TOKEN curr_token = token;
    while (curr_token != NULL)
    {
        /*the parts can be simple tokens ,functions or indexed tokens */
        switch (curr_token->type)
        {
        case hdr_tk_function:
        {
            _debug_print_function_struct(curr_token,level);
            printf("\n");
        }break;
        case hdr_tk_ref_brackets:
        {
            _debug_print_indexed_token_struct(curr_token,level);
            printf("\n");
        }break;
        case hdr_tk_index:
        {
            /*a "free" index of the token like $test[3]'[2]'*/
            printf(",\"index\" : ");
            _DEBUG_PRINT_EXPRESSION_STRUCT(curr_token->expression,level,true);
            printf("\n");
        }break;
        case hdr_tk_simple:
        {
            printf(",\"simple\":\"%s\"",curr_token->ID->stringa);
            printf("\n");
        }break;
        }


        curr_token = curr_token->next_part;
    }
    printf("}\n");
    return ;
}


void _DEBUG_PRINT_EXPRESSION_STRUCT(PHDR_EXPRESSION expr,int level, bool add_brackets)
{
    printf("\n");
    level = level + 2;
    _debug_add_spaces(level);
    if(add_brackets == true)
    printf("{\n");
    /*parse all the nodes of an expression and print them*/
    if (expr == NULL)
    {
        printf("NULL");
        return;
    }

    PDXL_NODE node = expr->tokens->start;
    while (node != NULL)
    {
        PDXL_OBJECT obj = node->object;
        PHDR_COMPLEX_TOKEN token = obj->obj;
        switch (token->type)
        {
            case hdr_tk_uninit: { printf(" ,\"undefined\":{}\n"); } break;
            case hdr_tk_ref_brackets: { _debug_print_indexed_token_struct(token,level); }break;
            case hdr_tk_multipart: { _debug_print_multipart_struct(token,level); }break;
            case hdr_tk_simple: 
            { 
                _debug_add_spaces(level + 1);
                printf("\"token\" : { \"name\" : \"%s\" }\n", token->ID->stringa);
            }break;
            case hdr_tk_function: { _debug_print_function_struct(token,level); }break;
            case hdr_tk_literal: 
            {
                _debug_add_spaces(level + 1);
               // char* ttypes = _debug_return_token_type_str(token);/*this do not needs freeing*/
                char* tvalues = _debug_return_token_val_str(token);
                printf("\"literal\": \"%s\",\n", tvalues);
                free(tvalues);
            }break;
            case hdr_tk_operator: 
            {
                _debug_add_spaces(level + 1);
                printf("\"operator\" : \"%s\",\n", token->ID->stringa);
            }break;
            case hdr_tk_expression: { printf("\"expr\":"); _DEBUG_PRINT_EXPRESSION_STRUCT(token->expression, level, true); }break;
            case hdr_tk_bool_expression: { printf("\"expr\":"); _DEBUG_PRINT_EXPRESSION_STRUCT(token->expression, level, true); }break;
            case hdr_tk_comp_expression: { printf("\"expr\":"); _DEBUG_PRINT_EXPRESSION_STRUCT(token->expression, level, true); }break;
            case hdr_tk_variable: 
            {
                    _debug_add_spaces(level+1);
                    printf("\"variable\" : { \"name\" : \"%s\" }\n",token->ID->stringa);
            }break;
        }
        node = node->right;
    }

    if (add_brackets == true)
        printf("}\n");
    return;

}












