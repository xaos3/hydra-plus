/*
 This header provides functions for analyze a bool expression
 into an PHDR_EXPRESSION structure .

 Nikos Mourgis deus-ex.gr 2024

 Live Long and Prosper

*/

#define HYDRA_BOOL ;

/*
 Hydra+ can recognize bool expressions that are syntesized by 
 the logical operators || && and the comparison operators of 
 == != < > >= <= .
 All the expressions MUST be encapsulated by parenthesis '()'
 Examples
 ($test == $test1)
 (($test1 == 1)&&(test2!=8))

*/



bool hdr_bool_check_op(char* prep_expr_indx, char sep)
{
	if (sep == 0) return true;
	if (*prep_expr_indx == sep)
	{
		/*
		  check if after the valid operator , a valid character exists, i.e. we do not allow the end of the expression or another special character 
		  this is a prelimary validity check. Other validity checks will be commence in the interpreter 
		*/
		if (dxIsCharInSet(*(prep_expr_indx + 1),"+/-*^&|%1234567890") == true) return false;
		else
			return true;
	}
	return false;
}


char  *hdr_comparison_check_op(char *term_expr, char sep)
{

	/*
	 the valid operators are != , == ,< ,> , >= ,<=
	 we will do some basic validation , we will check the next character after the operator 
	 for obvious errors
	*/
	if (sep == 0) return NULL;
	switch (sep)
	{
	  case '!' :
	  {
		  if ((*(term_expr) == '=') && (dxIsCharInSet(*(term_expr + 1), "=!<>") == false))
		  {
			  char* op = malloc(3);
			  op[0] = '!';
			  op[1] = '=';
			  op[2] = 0;
			  return op;
		  }
	  }break;
	  case '<':
	  {
		  if ((*(term_expr) == '=') && (dxIsCharInSet(*(term_expr + 1), "=!<>") == false))
		  {
			  char* op = malloc(3);
			  op[0] = '<';
			  op[1] = '=';
			  op[2] = 0;
			  return op;
		  }

		  if (dxIsCharInSet(*(term_expr + 1), "!<>") == false)
		  {
			  char* op = malloc(2);
			  op[0] = '<';
			  op[1] = 0;
			  return op;
		  }

	  }break;
	  case '>':
	  {
		  if ((*(term_expr) == '=') && (dxIsCharInSet(*(term_expr + 1), "=!<>") == false))
		  {
			  char* op = malloc(3);
			  op[0] = '>';
			  op[1] = '=';
			  op[2] = 0;
			  return op;
		  }

		  if (dxIsCharInSet(*(term_expr + 1), "!<>")==false)
		  {
			  char* op = malloc(2);
			  op[0] = '>';
			  op[1] = 0;
			  return op;
		  }
	  }break;
	  case '=':
	  {
		  if ((*(term_expr) == '=') && (dxIsCharInSet(*(term_expr + 1), "=!<>") == false))
		  {
			  char* op = malloc(3);
			  op[0] = '=';
			  op[1] = '=';
			  op[2] = 0;
			  return op;
		  }
	  }break;
	}

	return NULL;
}


/*resolve algebric expression*/

PHDR_EXPRESSION  hdr_bool_h_expr_create_expr(char* instr_indx)
{
	if (instr_indx == NULL) return NULL;

	/*
	  The function creates a non boolean expression from a string that represents
	  a calculation or concatenation.
	*/
	int status = 0;
	/*
	 do the operators priority preparing for all the operations EXCEPT the expressions that are inside [] and in the functions ()
	 the above cases will be handled diferently in the code
	*/
	char* expr_str = hdr_expression_prepare_priority(instr_indx, &status);

	if (expr_str == NULL)
	{
		printf("An expression analysis failed. Instruction : '%s' Internal message : %s\n", instr_indx, hdr_error_code_to_str(status));
		goto fail;
	}

	char* expr_str_indx = expr_str;
	if (dxItsStrEncapsulated(expr_str_indx, '(', ')') == true)
	{
		/*trim the first ( and the last )*/
		expr_str_indx[strlen(expr_str_indx) - 1] = 0;
		expr_str_indx = expr_str_indx + 1;
	}



	PHDR_EXPRESSION expr = hdr_expr_create_from_str(expr_str_indx, &status);

	if (status != HDR_SUCCESS)
	{
		printf("An expression analysis failed. Instruction : '%s' Internal message : %s\n", instr_indx, hdr_error_code_to_str(status));
		goto fail;
	}
	free(expr_str);
	return expr;
fail:
	free(expr_str);
	return NULL;

}


PHDR_EXPRESSION hdr_bool_create_expr(char* expr)
{
	/*trim the () if exists and all the spaces and tabs*/
	char* expr_str_indx = expr;

	dxRightTrimFastChars(expr_str_indx, "\t");
	/*trim the left spaces and tabs*/
	dxGoForwardWhileChars(&expr_str_indx, " \t");

	/*first we will check the parenthesis '()' and brackets '[]' validity*/
	if (dxCheckSectionPairing(expr_str_indx, '(', ')', "`\"") == false)
	{
		printf("The boolean expression is malformed. Some '(' or ')' is missing.\n");
		return NULL;
	}

	if (dxItsStrEncapsulated(expr_str_indx, '(', ')') == true)
	{
		/*trim the first ( and the last )*/
		expr_str_indx[strlen(expr_str_indx) - 1] = 0;
		expr_str_indx = expr_str_indx + 1;
	}

	dxRightTrimFastChars(expr_str_indx, "\t");
	/*trim the left spaces and tabs*/
	dxGoForwardWhileChars(&expr_str_indx, " \t");
	
	/*no spaces or tabs are permited in the expression*/
	char* prep_expr = dxRemoveCharsExt(expr_str_indx, " \t", "`\"");
	/*
	 so far so good. Now we will get the boolean operators and operants if exists
	*/

	PHDR_EXPRESSION expression = hdr_expression_create();
	expression->type = hdr_expr_boolean;
	char* prep_expr_indx = prep_expr;
	while (*prep_expr_indx != 0)
	{
		/*get the operands of the boolean operators*/
		char sep = 0;
		char * operand = dxGetNextWord(&prep_expr_indx, "&|", "`\"",true, false , &sep);
		if (operand == NULL)
		{
			printf("Internal error while trying to get the next operand in the boolean expression.\n");
			goto fail;
		}
		if (operand[0] == 0)break; /*no more parts*/
		/*check if the operator is valid*/
		if (hdr_bool_check_op(prep_expr_indx, sep) == false)
		{
			free(operand);
			printf("The operator '%c' is not valid. The valid form is '%c%c'. If the operator is correct, then check if the operator is missplaced.\n",sep,sep,sep);
			goto fail;
		}
		else
		{
			if(*prep_expr_indx !=0)/*there is the posibility that we have reach the end of the expression*/
			prep_expr_indx++;/*next character, the current one is the operator pair*/
		}

		/*
		  this expression can be a terminal comparison like $var1 != 3+3 
		  or a complex boolean expression like (($var!=2)||($var!=8))
		  we will check the type and we will resolve accordinally
		*/

		if (dxItsStrEncapsulated(operand, '(', ')') == true)
		{
			/*a boolean expression*/
			PHDR_COMPLEX_TOKEN token = hdr_complex_token_create(NULL,"");
			if (token == NULL){free(operand);goto fail;}
			/*so the current expression is a bool expression*/

			token->type = hdr_tk_bool_expression ;
			token->expression = hdr_bool_create_expr(operand);
			if (token->expression == NULL)
			{
				free(operand); 
				hdr_complex_token_free(token);
				goto fail;
			}
			/*add token to the expression*/
			hdr_tokens_list_add_token(expression->tokens, token);
			/*if the separator is not empty add the separator in the expression*/
			if (sep != 0)
			{
				char b_op[3];
				b_op[0] = sep;
				b_op[1] = sep;
				b_op[2] = 0;
				PHDR_COMPLEX_TOKEN token = hdr_complex_token_create(NULL, b_op);
				if (token == NULL) { free(operand); goto fail; }
				token->type = hdr_tk_operator;
				/*add token to the expression*/
				hdr_tokens_list_add_token(expression->tokens, token);
			}
		}
		else
		{
			/* a terminal comparison, we support != == < > >= <= */
			char *term_expr = operand;
			while (*term_expr != 0)
			{
				/*get the operands of the comparison operators*/
				char sep = 0;
				char* c_operand = dxGetNextWord(&term_expr, "!=<>", "`\"", true, false, &sep);
				if (c_operand == NULL)
				{
					printf("Internal error while trying to get the next operand in the boolean expression.\n");
					free(operand);
					goto fail;
				}
				if (c_operand[0] == 0)break; /*no more parts*/
				/*check if the operator is valid (an operator does not exists if we found the end of the instruction)*/
				char *c_op = hdr_comparison_check_op(term_expr, sep);
				if ( c_op != NULL)
				{
					if (strlen(c_op)>1) /*if the operator is not one character*/
					{
						if (*term_expr != 0)	/*there is the posibility that we have reach the end of the expression*/
							term_expr++;		/*next character, the current one is the operator pair*/
					}
				}

				/*now , the part that we retrieved can be an algebric expression, we will resolve it too */
				PHDR_EXPRESSION op_exp =  hdr_bool_h_expr_create_expr(c_operand) ;
				if (op_exp == NULL)
				{
					free(c_op);
					free(c_operand);
					free(operand);
					goto fail;
				}
				op_exp->type = hdr_expr_comparison ;/*special case so the interpreter knows that follows / takes part to an comparison*/

				PHDR_COMPLEX_TOKEN token = hdr_complex_token_create(NULL, "");
				if (token == NULL) { free(c_operand); free(c_op); hdr_expression_free(op_exp); goto fail; }
				token->type = hdr_tk_comp_expression;
				token->expression = op_exp;
				/*add the expression to the comparison expression*/
				hdr_tokens_list_add_token(expression->tokens, token) ;
				/*check for the operator*/
				if (c_op != NULL)
				{
					token = hdr_complex_token_create(NULL, c_op);
					if (token == NULL) { free(c_operand); free(c_op); goto fail; }
					token->type = hdr_tk_operator ;
					/*add the expression to the comparison expression*/
					hdr_tokens_list_add_token(expression->tokens, token);

				}

				free(c_op);
				free(c_operand);
			}

		}



		free(operand);

	}

	free(prep_expr);
	prep_expr = NULL;

	if (expression->tokens->count == 0)
	{
		printf("The 'if' instruction MUST have a boolean expression inside the '()'\n");
		goto fail;
	}

	return expression ;
fail:
	free(prep_expr);
	hdr_expression_free(expression);
	return NULL;
}
















