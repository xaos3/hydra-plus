/*

  This module has a collection of string manipulation routines 
  for parsing a script and retrieve its tokens.

  Hydra+
  Header file module for the script parsing routines
  Nikos Mourgis deus-ex.gr 2024
*/

#define STRING_PARSE

#define HDR_INSTRUCTION_LENGTH 102400
#define HDR_NAMED_PROP_LENGTH  128

#define HDR_BLOCK_OPEN	 101
#define HDR_BLOCK_CLOSE  102

#define HDR_STRING_OPEN_DOUBLE 103
#define HDR_STRING_OPEN_SINGLE 104
#define HDR_COMMENT_MULTI      105

#define HDR_ERROR_SEPARATOR_DOT      107

/*Hydra supports two syntax for string literals the "some string" and the `some string`*/
#define HDR_STRING_IDENT "`\""
/*The instructions are terminated by [;] OR [{]*/
#define HDR_PARSER_INSTRUCTION_TERM ";{"
/*An expression separate the operands with the following symbols*/
#define HDR_PARSER_EXPRESSION_SEPS "/*-+%^"
/*A boolean expression separates the operands with the following*/
#define HDR_PARSER_BOOL_EXPRESSION_SEPS "|| && ^"
/*A boolean operation uses the following equality operators */
#define HDR_PARSER_BOOL_EQUALITY "== != < >"
/*The indexes are confided in the following pair*/
#define HDR_PARSER_INDEXES "[]"
/*The parenthesis are .. well parenthesis*/
#define HDR_PARSER_PARENTHESIS "()"
/*The code block is encapsulated between {} */
#define HDR_PARSER_CODE_BLOCK "{}"
/*The one line comment */
#define HDR_PARSER_COMMENT "#"
/*The special one line comment */
#define HDR_SPECIAL_PARSER_COMMENT "?"
/*The multiline comment*/
#define HDR_PARSER_MULTILINE_COMMENT "[* *]"

typedef struct hdr_entity
{
	PDX_STRING separator ; /*the character(s) that seperates this entity from the sentence*/
	PDX_STRING entity    ; /*the entity*/
	bool too_long_entity ; /*if the entoty is too long to be returned this will be set to true*/
} *PHDR_ENTITY;

PHDR_ENTITY hdr_entity_create()
{
	PHDR_ENTITY ent = (PHDR_ENTITY) malloc(sizeof(struct hdr_entity));
	ent->entity    = dx_string_createU(NULL,"");
	ent->separator = dx_string_createU(NULL,"");
	ent->too_long_entity = false ;
	return ent ;
}

PHDR_ENTITY hdr_entity_free(PHDR_ENTITY entity)
{
	/*returns NULL after frees the memory*/
	if (entity == NULL) return NULL;
	dx_string_free(entity->entity)    ;
	dx_string_free(entity->separator) ;
	free(entity);
	return NULL ;
}

char *trimable_chars()
{
	char *trims = (char*)malloc(6) ;
	trims[0] = 32 ;
	trims[1] = 9  ;
	trims[2] = 10 ;
	trims[3] = 13 ;
	trims[4] = 11 ;
	trims[5] = 0  ;

	return trims;
}

bool hdr_parser_its_trimmable(char c, char* trimables)
{
	if (trimables == NULL) return false;
	while (*trimables != 0)
	{
		if (*trimables == c) return true ;
		trimables++ ;
	}
	return false;
}

/*trim undesirable characters the function returns a NEW CREATED memory block, so free it after use to not have memory leaks*/

char* hdr_parser_trim_right(char* str,char *trim_chars)
{
	DXLONG64 len = strlen(str)           ;
	if (len == 0)
	{
		char* ret = malloc(1);
		ret[0] = 0;
		return ret;
	}

	char* nstr   = (char*)malloc(len+1)  ;
	memcpy(nstr, str, len)			   ;
	nstr[len] = 0					   ;
	char *strindx = &nstr[len - 1]	   ;
	char *trims = trimable_chars()	   ;

	while (true)
	{
		if ((hdr_parser_its_trimmable(*strindx, trims) == true) || (hdr_parser_its_trimmable(*strindx, trim_chars) == true))
		{
			if (strindx == str)
			{
				/*edge case an empty characters string or a string full of trimable characters*/
				*strindx = 0;
				free(trims);
				return strindx;
			}
			*strindx = 0;
			strindx--;
		}
		else
			break;
	}
	free(trims);
	return nstr;

}

char* hdr_parser_trim_left(char* str, char* trim_chars)
{
	DXLONG64 len;
	if (str != NULL)
	{  
		len = strlen(str);
	}
	else
		len = 0;

	if (len == 0)
	{
		char* ret = malloc(1);
		ret[0] = 0;
		return ret;
	}

	char* trims = trimable_chars();

	while (true)
	{
		if ((hdr_parser_its_trimmable(*str, trims) == true) || (hdr_parser_its_trimmable(*str, trim_chars) == true))
		{
			if (str == 0)
			{ /*edge case an empty characters string or end of string of trimable characters*/
				free(trims);
				char* nstr = (char*)malloc(1);
				nstr[0] = 0;
				return nstr;
			}

			str++;
		}
		else
			break;
	}

	len = strlen(str) ;
	char* nstr = (char*)malloc(len + 1);
	memcpy(nstr, str, len);
	nstr[len] = 0;

	free(trims);
	return nstr ; 

}




char *hdr_parser_trim(char* str, char* trim_chars)
{
	char *nstr1 = hdr_parser_trim_left(str, trim_chars) ;
	char *nstr  = hdr_parser_trim_right(nstr1, trim_chars) ;
	free(nstr1);
	return nstr;
}

enum hdr_parser_parsing_state {hdr_prs_multi,hdr_prs_comment,hdr_prs_string_quotes, hdr_prs_string_singleq,hdr_prs_normal};

char* hdr_parser_get_next_instruction(char** string_pos, DXLONG64* instr_line, DXLONG64* line_number, DXLONG64* char_pos, DXLONG64* ERROR_CODE,bool *terminated)
{
	/*
	  This function is one of the most important of the loader / parse,
	  The string_pos is the position in the script file.
	  After the instruction is retrieved, the string_pos is pointing to the
	  [{] if the instruction itself has a block , or the next character after the lf that follows the [;]
	  The function returns NULL if an error occurs or if the parsing is done. The ERROR_CODE will have the
	  HDR_SUCCESS if the retrieving was succesfull (when the return value is NULL and the ERROR_CODE is HDR_SUCCESS then 
	  the script has been parsed whole.)
	  The instr_line is the line number that the instruction starts (an instruction can span in multilines).
	  The line_number is the current line in the script(based of the current value of the string_pos).
	  The char_pos will be set to the last character parsed.
	  If the instruction is terninated by an [;] then the terminated flag is true else is set to false 
	*/
	*terminated = false;
 	char *indx  = *string_pos ;
	char *instr = (char*)malloc(HDR_INSTRUCTION_LENGTH+1); /*The maximum length for an istruction*/
	*instr = 0;/*empty string*/
	char *instr_indx = instr ;
	*instr_line = (*line_number); /*the new instructions line as the line_number has the previous line number*/
	 /*
	  if the state is in a multi line comment the function will ommit the characters until the end of the comment. BUT the lf will be counted
	  if the state is in a single line comment the function will ommit the characters until the next lf
	  if the state is inside a string then we will ommit the characters until the string closing
	  */
	enum hdr_parser_parsing_state state = hdr_prs_normal ;
	while (*indx != 0)
	{
		(*char_pos)++ ;
		/*the LF must always be calculated */
	
		if (*indx == 10)
		{
			(*line_number)++;
			/*check if the state is in a single comment because a new line exits the state*/
			if (state == hdr_prs_comment) state = hdr_prs_normal;
			/*if we are in a string the lf must be include in the instruction*/
			if ((state == hdr_prs_string_quotes) || (state == hdr_prs_string_singleq))
			{
				*instr_indx = *indx;
				instr_indx++;
			}
			else
			{
				/*transform the character to a space*/
				*instr_indx = ' ';
				instr_indx++;
			}
			indx++; //next character
			continue ;
		}

		/*Check if we are in a state of ignoring*/
		if (state != hdr_prs_normal)
		{
			if ((state == hdr_prs_string_quotes)||(state == hdr_prs_string_singleq))
			{
				/*check if the character is to close the string*/
				if((state == hdr_prs_string_quotes)&&(*indx == '"'))
				{
					state = hdr_prs_normal;
					*instr_indx = *indx;
					instr_indx++;
					indx++   ;
					continue ;
				}
				 else
					if ((state == hdr_prs_string_singleq) && (*indx == '`'))
					{
						state = hdr_prs_normal;
						*instr_indx = *indx;
						instr_indx++;
						indx++;
						continue;
					}

				/*the string is part of the instruction so the characters has to be inserted in*/
				*instr_indx = *indx;
				instr_indx++;
			}
			else
				if (state == hdr_prs_multi)
				{
					/*check if this is a multiline comment end , no character will be returned */
					if(*indx == '*')
					if (*(indx + 1) == ']')
					{
						state = hdr_prs_normal;
						/*pass the *] characters */
						indx = indx + 2;
						continue;
					}
				}

			indx++;
			continue;
		}

		/*check the current char for special characters*/
		switch (*indx)
		{
		 case ';':
				 {
					/*
					  The instruction ends. We have already check for states that ommit it.
					  Point to the next valid character.
					  The actual [;] will not be added
					*/
					indx++ ; /*next character*/
					/*find the next valid character*/
					 while ((*indx != 0) && ((*indx == 13) ||(*indx == 10)))
					 {
						 /*if we found a line feed then update the line_number*/
						 if (*indx == 10) (*line_number)++;
						 indx++;
					 }
					
					 /*return the instruction string*/
					 *instr_indx = 0		   ; /*terminate the instruction*/
					 *string_pos = indx		   ;
					 *ERROR_CODE = HDR_SUCCESS ; 
					 *terminated = true		   ;
					 return instr			   ;

				 }
			 break;
		 case '{':
				 {
					/*
					  the open bracket is a special case. If we have accumulate more characters than 0
					  we will return all the string until now as an instruction BUT the index will be not 
					  advanced to the next character. If we have accumulate only th { character
					  will return it , advance the index and the ERROR_CODE will be HDR_BLOCK_OPEN
					*/

					/*return the instruction string*/
					 *instr_indx = 0; /*terminate the instruction*/
					 if (strlen(instr) > 0)
					 {
						 *ERROR_CODE = HDR_SUCCESS ;
						 *string_pos = indx ;
						 return instr ;
					 }

					 indx++; /*after the {*/
					 /*find the next valid character*/
					 while ((*indx != 0) && ((*indx == 13) || (*indx == 10)))
					 {
						 /*if we found a line feed then update the line_number*/
						 if (*indx == 10) (*line_number)++;
						 indx++;
					 }

					 *string_pos = indx ;/*next character after { we have already find the next valid character after*/
					 *ERROR_CODE = HDR_BLOCK_OPEN;
					 *instr_indx = '{';
					 *(instr_indx+1) = 0 ;
					 return instr;
				 }
			 break;
		 case '}':
				 {
					 /*
					   the close bracket is a special case. The function return all the string until now
					   as an instruction and the ERROR_CODE will be HDR_BLOCK_CLOSE
					 */

					 /*return the instruction string*/
					 *instr_indx		= '}'	; 
					 *(instr_indx + 1)	= 0		; /*terminate the instruction*/
					 /*go after }*/
					 indx++;
					 /*find the next valid character*/
					 while ((*indx != 0) && ((*indx == 13) || (*indx == 10)))
					 {
						 /*if we found a line feed then update the line_number*/
						 if (*indx == 10) (*line_number)++;
						 indx++;
					 }

					 *string_pos = indx; /*next character after } we have already find the nexr valid character after }*/
		 
					 *ERROR_CODE = HDR_BLOCK_CLOSE;
					 return instr;
				 }
			 break;
		 case '[': 
				 {
					/*check if this is a multiline comment start , no character will be returned */
					if (*(indx + 1) == '*') 
					{ 
						state = hdr_prs_multi;
						indx = indx + 2 ;
						continue        ;
					}
					*instr_indx = *indx;
					instr_indx++;

				 }
			 break;
		 case '#':
				 {
					/*this is a comment*/
					state = hdr_prs_comment ;
					/*no need for [continue] we will proceed normally in the string after the switch*/
				 }
			 break;
		 case '?':
				 {
					/*25-10-2024 this is a comment, i have add this as special comment for readability in the 
					functions return value, for example func somef()?Boolean*/
					state = hdr_prs_comment ;
					/*no need for [continue] we will proceed normally in the string after the switch*/
				 }
			 break;
		 case '"':
				 {
					/*enter the state of the string*/
					state = hdr_prs_string_quotes ;
					*instr_indx = *indx;
					instr_indx++;
				 }
			 break;
		 case '`':
				 {
					/*enter the state of the string*/
					state = hdr_prs_string_singleq ;
					*instr_indx = *indx;
					instr_indx++;
				 }
			 break;
		 default: {/*add the char to the instruction*/ 
					if (*indx != 13) 
					   { 
						 *instr_indx = *indx;
					     instr_indx++ ; 
					   } 
				  }
				  break;
		}

		indx++ ; /*next character*/
	}

	/*check for errors when the string is ending*/

	switch (state)
	{
	case hdr_prs_multi			  : { *ERROR_CODE = HDR_COMMENT_MULTI; free(instr); return NULL; }
		  break;
	  case hdr_prs_comment		  : { 
										/*A single line comment can terminate just with the end of the file and not with an LF*/
										*ERROR_CODE = HDR_SUCCESS; 
										free(instr); 
										return NULL;
									}
		  break;
	  case hdr_prs_string_quotes  : { *ERROR_CODE = HDR_STRING_OPEN_DOUBLE; free(instr); return NULL; }
		  break;
	  case hdr_prs_string_singleq : { *ERROR_CODE = HDR_STRING_OPEN_SINGLE; free(instr); return NULL; }
		  break;
	  case hdr_prs_normal		  :	
									{
										if (instr[0] == 0)
										{
											*ERROR_CODE = HDR_SCRIPT_END;
											free(instr) ;
											return NULL ;
										}
										*instr_indx = 0;
										*ERROR_CODE = HDR_SUCCESS; 
										*string_pos = indx;
										return instr; 
									}
		  break;
	}

	return NULL ; /*never reach this*/

}

bool hdr_is_entity_sep(char *current_pos,char *start_str_pos, int *status)
{
	/*
	  checks if the current_pos character is a separator
	  The status can be HDR_ERROR_SEPARATOR_DOT 
	  or HDR_SUCCES
	*/

	*status = HDR_SUCCESS ; 
	char* seps   = "=/*-+%^(),[]\"`.";

	int len = strlen(seps) ;
	char *sindx = seps ;
	while ( *sindx != 0 )
	{
		if (*sindx == *current_pos)
		{
			/*the == and !=  are special cases , we need to check them so we will not use the = as a separator*/
			if (*current_pos == '=')
			{
				/*a[=] found , check if its a bool operator*/
				if (*(current_pos+1) == '=')
				{
					return false  ;
				}

				if (current_pos != start_str_pos)
				if (*(current_pos - 1) == '=')
				{
					return false;
				}

				if(current_pos != start_str_pos)
					if (*(current_pos - 1) == '!')
					{
						return false;
					}

			}
			else 
				if (*current_pos == '.')
				{
					/*
					  the dot can be an entity separator if its used to separates a part of a multipart token
					  or can be used for the decimal separator if the token is a literal real number 
					*/

					/*check if the left character and the right character is a number*/
					if (current_pos == start_str_pos)
					{
						/*if the dot is the first character then the next character cannot be a number*/
						if (dxIsCharNumber(*(current_pos + 1)) == true)
						{
							*status = HDR_ERROR_SEPARATOR_DOT;
							return false ;
						}
					}

				/*22-06-2024 i comment the following lines as a variable can end with a number AND be a multipart*/
				/*	if ((dxIsCharNumber(*(current_pos - 1)) == true) || (dxIsCharNumber(*(current_pos + 1) == true)))
					{
						
						if ((dxIsCharNumber(*(current_pos - 1)) == false) || (dxIsCharNumber(*(current_pos + 1)) == false))
						{
							*status = HDR_ERROR_SEPARATOR_DOT ;
							return false;
						}

					
						return false ;

					}
					*/

					
				}

			return true;
		}
		sindx++;
	}

	return false ;
}

bool hdr_is_a_sign(char *indx, char* start_str_pos)
{

	/*check if the char is a sign to a number like +3 or -4 */
	if ((*indx == '-') || (*indx == '+'))
	{
		/*chek if its the first character of the sentense*/
		if (indx == start_str_pos) return true; /*ok lets say its a sign*/
		/*check if the sign is following another arithmetic operator and if after it exists a number*/
		char* c_indx = indx;
		while ((*c_indx != *start_str_pos) && (*c_indx != ' ') && (*c_indx != '\t')) c_indx--;
		if ((*c_indx != '/') && (*c_indx != '*') && (*c_indx != '%') && (*c_indx != '^') && (*c_indx != '+') && (*c_indx != '-'))
			if((*c_indx!=' ')&&(*c_indx != '\t'))
			return false;

		c_indx = indx+1 ; /*next character*/
		while ((*c_indx != 0) && (*c_indx == ' ') && (*c_indx == '\t'))
		{
			c_indx++;
		}
		if (*c_indx == 0) return false; /*this is an error in syntax b ut leave it for now , we will catch ot later*/
		/*check for a number*/
		if (dxIsCharNumber(*c_indx) == true) return true;
		else
			return false;


	}

	return false ;
}

PHDR_ENTITY hdr_parser_get_next_entity(char** strindx,int *status)
{
	/*
	  This function is in the heart of the parser.
	  It gets the next logical entity of the script 
	  and returns it. The return value is created in the function and it must 
	  be freed by the caller.
	  The strindx will point to the next character after the retrieved entity 
      The token separators are =/*-+%^(),[]"`.
	  The function is aware of the strings identation of [`] and ["] 
	  so it will return a string as a whole entity.

	  Note : The dot [.] can be either a separator for an entity in the multipart
	  tokens ($var.some_property) OR as a decimal point for the real numbers (3.3).
	  The function can understand this, if the dot is a part of a real number then 
	  it is ignored , if it is a separator for a part then the function use it
	  as a separator
	  
	  For the strings the behavior of the function differs slightly.
	  If a string separator is found to open a string , then the entity is returned , the separator
	  is as usuall returned BUT the string index will point to the string itendation
	  character and not in the character after it.
	  Remarks : We "trim" the string of the prefixed trimmable characters so to detect the string type ["][`] 

	  The return entity will have the actual entity text and the separator.
	  If the separator was the string end , then the entity->sep will be empty.
	  
	  If the entity->entity and the entity->sep is empty then the string has ended 

	  Remarks : The strings are special entities , when a string entity is returned then the separator is EMPTY
	  as the actuall separator is the string identation that is returned as a part of the entity .
	  So a string entity has the form of "HELLO" or `HELLO`

	  Another special case is when a number is signed, for example +18 or -4 , the number will be 
	  returned as a single entity. To detect an entity like it we will check if the sign is 
	  found in the start of the expression or immediatelly after another separator

	  ELSE : the keyword 'else' is a special case as it has not a distinct separator from the next 
	  instruction so we check specifically in here and return only the 'else' word of the instruction  


	*/
	PHDR_ENTITY entity = hdr_entity_create() ;
	char *indx  = *strindx;
	char *accum = (char*)malloc(HDR_INSTRUCTION_LENGTH+1) ;
	accum[0] = 0;
	int  accum_cnt = 0;
	char sep[2] ;
	sep[0] = 0  ;
	sep[1] = 0  ;
	DXLONG64 acc_indx = 0 ;

	bool in_string = false ;
	char str_ident = '`'   ;

	*status = HDR_SUCCESS;

	/*"trim" the tab and space characters*/

	while ((*indx == ' ') || (*indx == '\t')) indx++;

	/*check if this is an else instruction*/
	char* sindx = indx;
	char *els = "else ";
	int i = 0;
	for (; i < 5; i++)
	{
		if (*sindx == 0) break;
		if (els[i] != dxConvertChar('\t', ' ', *sindx)) break;
		sindx++;
	}
	if (i == 5)
	{
		*strindx = *strindx + 5;
		free(accum);
		dx_string_createU(entity->entity, "else");
		return entity;
	}

	/*check if we are about to return a string*/
	if (*indx == '`')
	{
		in_string = true;
		str_ident = '`';
	}
	else
		if (*indx == '"')
		{
			in_string = true;
			str_ident = '"';
		}

	while (*indx != 0)
	{
		/*check if we are in a string*/
		if (in_string == true)
		{
			/*check if the string ends*/
			if (*indx == str_ident)
			{
				//return the string 
				if (accum_cnt > HDR_INSTRUCTION_LENGTH - 2)
				{
					entity->too_long_entity = true;
					accum[accum_cnt - 1] = 0;
					char* trimmed = hdr_parser_trim_right(accum," ");
					dx_string_createU(entity->entity, trimmed);
					*strindx = indx; // set the string pointer to the next character
					free(accum);
					free(trimmed);
					return entity;
				}

				/*check if its the string opening*/
				if (accum[0] == 0)
				{
					accum[acc_indx] = *indx;
					acc_indx++ ;
					indx++     ;
					continue; /*add the character , is the opening of the string*/
				}

				accum[acc_indx]      = *indx ;
				accum[acc_indx + 1]  = 0     ;
				*strindx = indx+1; // set the string pointer to the next character
				char* trimmed = hdr_parser_trim_right(accum," ");
				dx_string_createU(entity->entity, trimmed)  ;
				dx_string_createU(entity->separator, sep) ;
				free(accum);
				free(trimmed);
				return entity;
			}
			/*else accumulate the characters*/
			accum[acc_indx] = *indx;
			acc_indx++;
		} else /*we are not in a string ,check for separators*/
		if ((hdr_is_entity_sep(indx,*strindx,status) == false)||(hdr_is_a_sign(indx,*strindx)==true))
		{
			if (*status != HDR_SUCCESS) goto e_error;
			if (hdr_parser_its_trimmable(*indx, "\r\n") == false)
			{
				if ((accum[0] == 0) && (*indx == ' '))/*ommit prefixed spaces*/
				{
                  /*do not add the character*/
				} else
				{
					accum[acc_indx] = dxConvertChar('\t', ' ', *indx) ;
					acc_indx++;
				}
			}
		}
		else
		{
			if (*status != HDR_SUCCESS) goto e_error;
			sep[0] = *indx;
			indx++		;
			break;
		}
		indx++ ;
		accum_cnt++ ;
		if (accum_cnt == HDR_INSTRUCTION_LENGTH)
		{
			entity->too_long_entity = true;
			accum[acc_indx - 1] = 0;
			char* trimmed = hdr_parser_trim_right(accum, " ");
			dx_string_createU(entity->entity, trimmed);
			*strindx = indx; // set the string pointer to the next character
			free(accum);
			free(trimmed);
			return entity;
		}
	}
	
	e_error:

	accum[acc_indx] = 0 ;
	/*check if the separator is a string identation character*/
	if ((sep[0] == '`') || (sep[0] == '"')) *strindx = indx - 1; /*point to the start of the string entity*/
	else
	*strindx = indx; /*next character*/

	char* trimmed = hdr_parser_trim_right(accum," ");
	dx_string_createU(entity->entity, trimmed) ;
	dx_string_createU(entity->separator, sep);
	free(accum);
	free(trimmed);
	return entity ;
}

/*
  The function returns the next token of the sentense , the tokens are separated by spaces or tabs
  This function is used for the func <name> obj <name> syntax
*/
char* hdr_parser_get_next_token(char** strindx)
{

	char *indx = *strindx ;
	char* accum = (char*)malloc(HDR_NAMED_PROP_LENGTH + 1) ;
	if (accum == NULL) return NULL ;
	char* accum_i = accum;

	/*"trim" spaces and tab*/
	while (*indx != 0)
	{
		if ((*indx != ' ') && (*indx != '\t')) break;
		indx++;
	}

	DXLONG64 cnt = 0;

	while (*indx != 0)
	{
		if ((*indx == ' ')||(*indx == '\t')) break;
		*accum_i = *indx ;
		accum_i++ ;
		indx++  ;
		cnt++   ;
		if (cnt > HDR_NAMED_PROP_LENGTH)
		{
			free(accum);
			return NULL ;
		}
	}

	*accum_i = 0;
	*strindx = indx ; /*set the position to the next valid character*/

	return accum;
}

bool hdr_parser_check_name(char* name)
{
	/*
	 the function checks if a variable name or other token comforms to the 
	 hydra naming standard. That is that the length must be smaller or equal to HDR_NAME_PROP_LENGTH
	 and to be constructed only with the latin upper and lower case leters, and numbers.

	 Remarks : The caller has to trim the $ symbol from name if it is a variable 
	*/


	if(strlen(name) > HDR_NAMED_PROP_LENGTH) return false ;

	if (dxIsStrNameSafe(name) == false) return false;
	return true ;

}

bool hdr_parser_check_name_permit_dollar(char* name)
{
	/*
	 the function checks if a variable name or other token comforms to the
	 hydra naming standard. That is that the length must be smaller or equal to HDR_NAME_PROP_LENGTH
	 and to be constructed only with the latin upper and lower case leters, and numbers.

	 Remarks : The $ symbol is valid in the start of the name
	*/

	int len = strlen(name);
	if ( len > HDR_NAMED_PROP_LENGTH) return false;
	if (len < 2) return false;
	if (*name == '$') name++; /*first $ is valid in variable names*/
	while(*name!=0)	
	{ 
		if (dxIsCharInSet(*name,"1234567890qwertyuioplkjhgfdsazxcvbnmQWERTYUIOPLKJHGFDSAZXCVBNM-_") == false) return false;
		name++;
	}

	return true;

}









