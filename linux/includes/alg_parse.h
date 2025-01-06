/*
 This module transform an arithmetic expression  ( for example [-8+10*125/3-4] ) 
 to an arithmetic expression that has prioritize the calculations with parenthesis 
 based in the operators priority.
 For example the previous expression will become -8+((10*125)/3)-4

 This module uses the dxList object of the dxlist.h module to save the calculation 
 and create the new form

 The module supports string isolation for the expressions that are inside a string
 (`` "")

 The module isolate the expressions inside the []. The caller has
 the responsibility to extract the expression from inside the [] 
 and pass it to this module for calibration

 The module isolate the expressions inside a function , as can be multiple parameters 
 separated by an coma ','. The caller has the responsibility to pass the parameters 
 individually as expressions in the functions of this module to be proccesed

 NOTE : This module has been created for a very specific purpose for the Hydra+ Loader,
 thus it may not be usefull for other cases. The input expression can have any character 
 in it as the loader excepts the expressions to have variables and other language tokens
 in it .

 Nikos Mourgis deus-ex.gr 

*/

#ifndef DXLISTS
#include "dxlists.h"
#endif

#define ALG_PARSE

#define ALG_SUCCESS     0
#define ALG_EXPR_NULL  -1
#define ALG_EXPR_EMPTY -2
#define ALG_TOO_LONG_OPERANT -3 
#define ALG_LIST_NULL		 -4
#define ALG_MISSPLACED_OPERATOR  -5
#define ALG_SYNTAX_ERROR		 -7 
#define ALG_OBJ_NULL			 -10
#define ALG_MEM_ERROR			 -11
#define ALG_PART_MUST_EMPTY      -12
#define ALG_SECTION_INVALID      -13
#define ALG_NODES_NOT_MERGED     -14
#define ALG_BRACKETS_INVALID     -15

#define ALG_OP_EXPONENT 1 
#define ALG_OP_PRIORITY 2
#define ALG_OP_NORMAL   3
#define ALG_OP_NONE     4
#define ALG_EXPR_END    5

#define ALG_OPERAND_LENGTH 1024
#define ALG_EXPRESSION_LENGTH 2048
#define ALG_MAXIMUM_MERGED_NODES 128


typedef PDX_LIST PALG_EXPRESSION ;
enum alg_node_type {alg_expr,alg_function,alg_operator,alg_operand,alg_merged}    ;
/*
 alg_expr          : The node stores an expression as ALG_EXPRESSION (12+36*(2))
 alg_function      : The node stores a function as an ALG_EXPRESSION with a name part
 alg_operator	   : The node stores an operator <+,-,*,/,^,%>
 alg_operand       : The node stores an operand <3,"hello",$var.list[1]>
 alg_root		   : The root of the expression, this node has the expr member set it as the expression for analysis
 alg_merged		   : The node is a merged node that its value resides in the txt field
 */
enum alg_op_type   {alg_exponent,alg_priority,alg_normal,alg_none} ;
/*
 alg_exponent      : The operator is the ^
 alg_priority	   : The operator is one of the *,/,%
 alg_normal		   : The operator is one of +,-
 alg_none		   : This is not an operator
 */
typedef struct alg_node
{
	char* txt ; 
	enum alg_node_type type    ;
	enum alg_op_type   op_type ;
	PALG_EXPRESSION     expr	   ;

} *PALG_NODE;




/************DEBUG***********/
void _PRINT_EXPR_STRUCT(PALG_EXPRESSION expr);
/***************************/

/********************************/
char* AlgTransformExpression(char* expr,int *status) ;
/********************************/

bool alg_ommit_this(char c)
{
	if ((c == '\t') || (c == '\n') || (c == '\r') || (c == ' ')) return true;
	 else
		return false; 
}

int alg_its_operator(char c)
{
	/* 
	  returns ALG_OP_NONE if the character is not an operator
	  and one of the ALG_OP ... defines if it is.
	*/
	if ((c == '+') || (c == '-'))					return ALG_OP_NORMAL;
	if ((c == '/') || (c == '*') || (c == '%'))		return ALG_OP_PRIORITY;
	if (c == '^')									return ALG_OP_EXPONENT;

	return ALG_OP_NONE;

}

/*prototypes*/
PALG_EXPRESSION alg_expression_create(char* expr,int *status);
PALG_EXPRESSION alg_expression_free(PALG_EXPRESSION expr);
/*--------*/
PALG_NODE alg_create_node(enum alg_node_type type, enum alg_op_type op_type,char *txt, char *expr,int *status)
{
	/*
	  Create an alg_node
	  type     : the node type 
	  op_type  : the operator type if this node is an operator
	  txt	   : the txt representation of the operand or operator
	  expr	   : the expression that the node will analyze and store. This is used if the type is alg_expr or an alg_function
	*/
	
	PALG_NODE algn = (PALG_NODE)malloc(sizeof(struct alg_node));
	if (algn == NULL) return NULL ;

	algn->op_type = op_type ;
	algn->txt	  = txt		;
	algn->type	  = type	;
	if (type == alg_expr)
		algn->expr = alg_expression_create(expr, status);
	else
		algn->expr = NULL;

	return algn ;
 
}

PALG_NODE alg_node_free(PALG_NODE node)
{
	if (node == NULL) return NULL;
	free(node->txt);
	alg_expression_free(node->expr);
	free(node);

	return NULL;
}

PALG_NODE alg_add_node_to_expression(PALG_EXPRESSION expr, PALG_NODE node)
{
	PDXL_OBJECT obj = (PDXL_OBJECT)malloc(sizeof(struct dxl_object)) ;
	if (obj == NULL) return NULL;
	obj->obj = node ;
	dx_list_add_node_direct(expr, obj);
	return node ;
}

enum alg_op_type alg_last_node_op(PALG_EXPRESSION algexpr)
{
	PDXL_NODE node = algexpr->end;
	if (node == NULL) return alg_none ;

	PDXL_OBJECT obj = node->object ;
	PALG_NODE   anode = obj->obj   ;

	if (anode->type != alg_operator) return alg_none ;
	return anode->op_type ;
}

enum alg_op_type alg_ret_op_type(char op)
{
	switch (op)
	{
		case '+':
		case '-': return alg_normal ;
			break;
		case '/':
		case '*':
		case '%': return alg_priority ;
		case '^': return alg_exponent ;
	}

	return alg_none ;

}

PALG_EXPRESSION alg_expression_free(PALG_EXPRESSION expr)
{
	if (expr == NULL) return NULL;
	PDXL_NODE  node = expr->start;
	while (node != NULL)
	{
		PDXL_OBJECT obj    = node->object ;
		PALG_NODE   anode  = obj->obj	  ;						 
		free(obj) ;
		alg_node_free(anode);
		node = node->right ;
	}
	dx_list_delete_list(expr) ;
	return NULL ;
}

/*
 helper functions
*/

void alg_h_add_to_part(char **part_indx,char **expr_index,int *part_count)
{
	*(*part_indx) = *(*expr_index);
	(*part_indx)++;
	(*part_count)++;
	(*expr_index)++;
	return;
}

/***************/
PALG_EXPRESSION alg_expression_create(char* expr,int *status)
{
	/*
	  create and store an expression from a string
	  The function get a string that represent an arithmetic expression,
	  analyze it and stored it in a ALG_EXPRESSION.

	  The ALG_EXPRESSION is a simple list of alg_node objects 
	  The expr must been a zero terminated string.
	  This means that more malloc/free operations will be 
	  executed leading to somehow lower perfomance than to have 
	  to make all the operations in the original expression.
	  But this will make the code more clean, and in reality the 
	  perfomance hit is negligible as (for Hydra+ at least) all this will be
	  executed one in the script loading time.
	*/
	*status = ALG_SUCCESS ;
	if (expr == NULL)
	{
		*status = ALG_EXPR_NULL;
		return NULL ;
	}

	if (strlen(expr) == 0)
	{
		*status = ALG_EXPR_EMPTY ;
		return NULL ;
	}

	char in_string = 0 ;
	bool in_brackets = false ;
	char* expr_index = expr  ;
	
	char part[ALG_OPERAND_LENGTH + 1]; // the maximum operand length + the terminating zero
	part[0]			  = 0;
	char*  part_indx  = part ;
	int	   part_count = 0;

	PALG_EXPRESSION algexpr = dx_list_create_list() ;
	if (algexpr == NULL)
	{
		*status = ALG_LIST_NULL;
		return NULL;
	}



	while ((*expr_index != 0)||(part[0]!=0))
	{

		if (part_count > ALG_OPERAND_LENGTH)
		{
			*status = ALG_TOO_LONG_OPERANT ;
			goto fail;
		}
		/*check if we are inside a string*/
		if (in_string != 0)
		{
			if (*expr_index == in_string) 
			{ 
				/*exit the string section*/
				alg_h_add_to_part(&part_indx, &expr_index, &part_count);
				in_string = 0;
				continue; 
			} 
		    /*still in string*/
			goto char_add;
		}
		else
		{
			/*check if we enter a string or a [] section*/
			if ((*expr_index == '"') || (*expr_index == '`')) { in_string = *expr_index; *part_indx = *expr_index; part_indx++; part_count++; expr_index++; continue; }
			if(*expr_index == '[') { in_brackets = true; *part_indx = *expr_index; part_indx++; part_count++; expr_index++; continue; }
		    if(*expr_index == ']')
			{
				/*exit the string section*/
				alg_h_add_to_part(&part_indx, &expr_index, &part_count);
				in_brackets = false ;

				/*check edge case when the expression has [] but not any operator outside*/
				if (*expr_index == 0) goto add_part ;
				continue;
			}
			if (in_brackets == true) goto char_add;
		}


		if (alg_ommit_this(*expr_index) == true)
		{
			/*a character of no consequence ommit it*/
			expr_index++;
			continue;
		}

		/*check the character as we are not inside a string section*/

		if (alg_its_operator(*expr_index) != ALG_OP_NONE)
		{
			/*
			  the character is an operator
			  maybe is a prefix (-1,+3 etc) but this will be checked later
			*/

			/*check if the operator is in the end of the expression. This is forbidden*/
			if (*(expr_index + 1) == 0) { *status = ALG_MISSPLACED_OPERATOR; goto fail; }

			char* optxt = (char*)malloc(2);
			optxt[0] = *expr_index;
			optxt[1] = 0 ;

			/*check if we have a part for saving. The part is the operand obviously :D*/

			if (part[0] != 0)
			{
				/*add the part*/
				*part_indx = 0 ; /*zero terminate it*/
				int partlen = strlen(part);
				char* txt = (char*)malloc( partlen + 1);
				if (txt == NULL){*status = ALG_MEM_ERROR;goto fail;}
				memcpy(txt, part,partlen);
				txt[partlen] = 0;

				PALG_NODE anode = alg_create_node(alg_operand, alg_none,txt , NULL, status); /*the txt will be freed later when the expression will be freed*/
				if (alg_add_node_to_expression(algexpr, anode) == NULL) { *status = ALG_OBJ_NULL; free(txt); free(optxt); goto fail; }
			    /*reset the part*/
				part[0] = 0;
				part_indx = part;
			}

			PALG_NODE anode = alg_create_node(alg_operator, alg_ret_op_type(*expr_index), optxt, NULL,status); /*the optxt will be freed later when the expression will be freed*/
			if (alg_add_node_to_expression(algexpr, anode) == NULL)
			{
				*status = ALG_OBJ_NULL ;
				free(optxt);
				goto fail;
			}
			expr_index++;
			continue ;

		}else
			if (*expr_index == '(')
			{
				/*
				  this will be treated as an isolated expression
				  copy the () section and create an expression.
				  The expr_index will point to the next character after the ')' of the section
				*/
				char* func_n = NULL;
				enum alg_node_type ntype = alg_expr;
				/*
				  if there is a part before this then the expression can be a function
				  a function has parameters in its () section and not an exppression , so we will isolate them
				*/
				if (part[0] != 0)
				{
					*part_indx = 0;/*terminate function name string*/
					int partlen = strlen(part);
					func_n = (char*)malloc(partlen + 1);
					if (func_n == NULL) { *status = ALG_MEM_ERROR; goto fail; }
					memcpy(func_n, part, partlen);
					func_n[partlen] = 0;
					ntype  = alg_function;
					/*reset the part*/
					part[0] = 0;
					part_indx = part;

				}
				char *section  = dxCopySectionFromStr(&expr_index,'(',')',"`\"", status);
				if ((*status != COPY_SECTION_SUCCESS)&&(*status != COPY_SECTION_EMPTY)) goto fail;
				
				/*if the status is COPY_SECTION_EMPTY then we will return a created BUT EMPTY expression*/

				PALG_NODE anode = NULL;
				/*check if this is a function*/
				if (ntype == alg_function)
				{					
					/*copy the name and the () section in the txt of the node as we will not change anything in the parameters*/
					int sec_len = 0;
					if (section != NULL) sec_len = strlen(section);
					int ftxt_len = strlen(func_n) + sec_len + 2;/*+2 for the ()*/
					char *ftxt = (char*)malloc(ftxt_len+1) ;
					if (ftxt == NULL)
					{
						free(section);
						free(func_n);
						goto fail;
					}
					ftxt[ftxt_len] = 0;
					
					char *ftxt_indx   = ftxt;
					dxCopyStrToStr(&ftxt_indx, func_n);
					*ftxt_indx = '(';
					ftxt_indx++;
					dxCopyStrToStr(&ftxt_indx, section);
					*ftxt_indx = ')';
					free(func_n); /*no need for this anymore!*/
					anode = alg_create_node(ntype, alg_none, ftxt, NULL, status); /*the ftxt will be freed later when the expression will be freed*/
					if (anode == NULL)
					{
						free(section);
						goto fail;
					}

					if (alg_add_node_to_expression(algexpr, anode) == NULL)
					{
						*status = ALG_EXPR_NULL;
						free(section);
						alg_node_free(anode);
						goto fail;
					}
					free(section);
					continue; /*the expr_index has been modified in the dxCopySectionFromStr*/

				}
				else 
					anode = alg_create_node(ntype, alg_none, NULL, section, status); 
				
				if (anode == NULL)
				{
					free(section);
					goto fail;
				}

				free(section);
				if (anode->expr == NULL)
				{
					alg_node_free(anode);
					goto fail;
				}

				if (alg_add_node_to_expression(algexpr, anode) == NULL)
				{
					*status = ALG_EXPR_NULL;
					alg_node_free(anode);
					goto fail;
				}
				continue ; /*the expr_index has been modified in the dxCopySectionFromStr*/
			}
		char_add :
		/*add the character into the part*/
		if(*expr_index != 0)
		alg_h_add_to_part(&part_indx, &expr_index, &part_count);
		/*SPECIAL CASE if we are in the last character of the expression we save the part and exit!*/

		if (*expr_index == 0)
		{   add_part:
			if (part[0] != 0)
			{
				/*add the part*/
				*part_indx = 0; /*zero terminate it*/
				int partlen = strlen(part);
				char* txt = (char*)malloc(partlen + 1);
				if (txt == NULL) { *status = ALG_MEM_ERROR; goto fail; }
				memcpy(txt, part, partlen);
				txt[partlen] = 0;

				PALG_NODE anode = alg_create_node(alg_operand, alg_none, txt, NULL, status); /*the txt will be freed later when the expression will be freed*/
				if (alg_add_node_to_expression(algexpr, anode) == NULL) { *status = ALG_OBJ_NULL; free(txt); goto fail; }
				/*reset the part*/
				part[0] = 0;
				part_indx = part;
			}
		}

	}


	return algexpr;
	
	fail :
	alg_expression_free(algexpr);
	return NULL;

}


void alg_merge_nodes(PDXL_NODE *tnode,int * status)
{
	/*
	  the function merges three nodes (two operands and one operator) into the left node ,
	  sets the txt , set it to alg_merged and deletes the memory of the two nodes that we do not need 
	  anymore.
	  Keep in mind that the nodes must being ready for consumption , this means that the nodes with expressions
	  must being transformed and the result must being in the txt member.
	*/
	PDXL_NODE node = *tnode;
	if ((node->left == NULL) || (node->right == NULL))
	{
		*status = ALG_NODES_NOT_MERGED ;
		return ;
	}

	PDXL_NODE left   = node->left  ;
	PDXL_NODE right  = node->right ;

	PDXL_OBJECT obj;

	obj     = left->object	;
	PALG_NODE  algnode_left = obj->obj		;
	char* left_txt = algnode_left->txt;

	obj = node->object;
	PALG_NODE  algnode_op = obj->obj;
	char* optxt = algnode_op->txt;
	free(obj);

	obj = right->object;
	PALG_NODE  algnode_right = obj->obj;
	char* right_txt = algnode_right->txt;
	free(obj);

	int lt_c, opt_c, rt_c;
	lt_c  = strlen(left_txt)  ;
	opt_c = strlen(optxt)     ;
	rt_c  = strlen(right_txt) ;
	int txt_len = lt_c + opt_c + rt_c ;

	char *merged_txt = (char*)malloc(txt_len + 2 + 1); /*2 for () and 1 for terminating zero*/
	merged_txt[txt_len+2] = 0 ;
	char *merg_indx = merged_txt;
	*merg_indx = '(';
	merg_indx++;
	while (*left_txt != 0)
	{
		*merg_indx = *left_txt;
		merg_indx++;
		left_txt++;
	}
	while (*optxt != 0)
	{
		*merg_indx = *optxt;
		merg_indx++;
		optxt++;
	}
	while (*right_txt != 0)
	{
		*merg_indx = *right_txt;
		merg_indx++;
		right_txt++;
	}
	*merg_indx = ')';

	/*set the correct chain links*/
	left->right = right->right;
	if(right->right!=NULL)
	right->right->left = left;
	/*
	  check for edge cases. If the right node is the end of the list , then 
	  the end of the list must point to the left node as the right node was absorbed
	*/
	if (right == right->lst->end) right->lst->end = left;

	/*set the correct count*/
	left->lst->count = left->lst->count - 2;


	/*set the new string*/
	obj = left->object ;
	algnode_left = obj->obj ;
	free(algnode_left->txt) ;
	algnode_left->txt = merged_txt;
	algnode_left->op_type = alg_none ;
	algnode_left->type = alg_merged  ;
	*tnode = left;/*set this a the current node*/
	*status = ALG_SUCCESS ;

	/*release the memory*/
	free(right);
	free(node);
	alg_node_free(algnode_op);
	alg_node_free(algnode_right);

}

void alg_merge_sign(PDXL_NODE *sign_n, int* status)
{
	/*
	  the function merges two nodes (the sign and the operand) into the operand node ,
	  sets the txt , and deletes the memory of the sign node that we do not need
	  anymore.
	  Keep in mind that the nodes must being ready for consumption , this means that the nodes with expressions
	  must being transformed and the result must being in the txt member.
	*/
	PDXL_NODE sign_node = *sign_n ; 
	if (sign_node->right == NULL)
	{
		*status = ALG_NODES_NOT_MERGED;
		return;
	}

	PDXL_NODE right = sign_node->right;

	PDXL_OBJECT obj;

	obj = sign_node->object;
	PALG_NODE algnode_sign = obj->obj;
	char* sign_txt = algnode_sign->txt;
	free(obj);

	obj = right->object;
	PALG_NODE algnode_operand = obj->obj;
	char* operand_txt = algnode_operand->txt;
	
	/*NOTE the last change is the  +4 it was +1 an the () and 0 was not added. This was changed to support easily the expresions like +12 * -10*/
	int txt_len = strlen(operand_txt)+4; /*+4 for the operator the () and the 0 as the form we will construct is the for example from +3 -> (0+3)*/

	char* merged_txt = (char*)malloc(txt_len + 1); 
	merged_txt[txt_len] = 0;
	char* merg_indx = merged_txt;
	*merg_indx = '(';
	merg_indx++;
	*merg_indx = '0';
	merg_indx++;
	*merg_indx = *sign_txt;
	merg_indx++;

	while (*operand_txt != 0)
	{
		*merg_indx = *operand_txt;
		merg_indx++;
		operand_txt++;
	}
	*merg_indx = ')';
	/*set the corect chain link*/
	if(sign_node->left != NULL)
	 sign_node->left->right = right;
	right->left = sign_node->left;
	/*check edge cases*/
	if (sign_node == sign_node->lst->start)sign_node->lst->start = right;
	/*set the correct count*/
	right->lst->count = right->lst->count - 1;

	/*release the memory of the sign node*/
	free(sign_node);
	/*set the new string*/
	obj = right->object;
	algnode_operand = obj->obj;
	free(algnode_operand->txt);
	algnode_operand->txt = merged_txt;

	*sign_n = right; /*set this node as this is now the curent one*/
	*status = ALG_SUCCESS;

	alg_node_free(algnode_sign);
}

/*helper function*/
char alg_return_op_from_node(PDXL_NODE node)
{
	if (node == NULL) return 0;
	PDXL_OBJECT obj = node->object ;
	PALG_NODE anode = obj->obj;
	if (anode->type == alg_operator) return anode->txt[0];
	if (anode->type == alg_expr) return 'e';
	if (anode->type == alg_function) return 'f';
	return 0;
}

void alg_merge_signed(PALG_EXPRESSION expr, int* status)
{
	/*
	 now we will do the validity checks . We will check if there is operators in pairs like ** or +*
	 the only pairs that are valid are the ones that describe a signed number like 8+-3 or 3*+4
	 if the operator is a sign the we will merge the two nodes
	*/
	*status = ALG_SUCCESS ;
	PDXL_NODE node = expr->start;
	while (node != NULL)
	{
		PDXL_OBJECT obj  = node->object     ;
		PALG_NODE   anode = obj->obj		;
		
		if (anode->type == alg_operator)
		{
			/*check if the operator is the first node. if it is, then it can be only +/- and after it MUST follow an operand*/
			if (node == expr->start)
			{
				if ((anode->txt[0] != '-') && (anode->txt[0] != '+')) { *status = ALG_MISSPLACED_OPERATOR; return; }
				if (node == expr->end) { *status = ALG_MISSPLACED_OPERATOR; return; }
				if (alg_return_op_from_node(node->right) != 0){/*this pairing is illegal*/*status = ALG_MISSPLACED_OPERATOR ;return;}
				/*ok merge the nodes as this is a sign*/
				alg_merge_sign(&node, status);
				if (*status != ALG_SUCCESS) return;
			}
			else
				if (node == expr->end) { *status = ALG_MISSPLACED_OPERATOR; return; }
				else
				{
					/*
					  the operator is not in the edges
					  check for validity
					*/
					char left_part = alg_return_op_from_node(node->left);
					if ((left_part != 0) && (left_part != 'e') && (left_part != 'f'))
					{
						if ((anode->txt[0] != '+') && (anode->txt[0] != '-')) {*status=ALG_MISSPLACED_OPERATOR;return;}
						/*check if the right part is an operator or expression or function and if it is then exit , else do the merge*/
						char right_part = alg_return_op_from_node(node->right);
						if ((right_part != 0) || (right_part == 'e') || (right_part == 'f')) { *status = ALG_MISSPLACED_OPERATOR; return; }
						else
						{
							alg_merge_sign(&node, status);
							if(*status != ALG_SUCCESS) return ;
						}
					}

				}

		}

		node = node->right ;
	}

}

void alg_resolve_exponents(PALG_EXPRESSION bexpr, int* status)
{
	/*find the exponents*/
	PDXL_NODE node = bexpr->start;
	while (node != NULL)
	{
		PDXL_OBJECT obj  = node->object  ;
		PALG_NODE   anode = obj->obj     ;
		if (anode->op_type == alg_exponent)
		{
			alg_merge_nodes(&node, status)		;
			if (*status != ALG_SUCCESS) return	;
		}

		node = node->right ;
	}

	return ;
}

void alg_resolve_priority(PALG_EXPRESSION bexpr, int* status)
{
	/*find the priorities*/
	PDXL_NODE node = bexpr->start;
	while (node != NULL)
	{
		PDXL_OBJECT obj = node->object;
		PALG_NODE   anode = obj->obj;
		if (anode->op_type == alg_priority)
		{
			alg_merge_nodes(&node, status);
			if (*status != ALG_SUCCESS) return;
		}
		node = node->right;
	}

	return ;
}


void alg_resolve_expr(PALG_NODE bexpr,int *status)
{
	/*
	  The function finds the expressions (parenthesis) and creates the actual string from it 
	*/
	if (bexpr->type != alg_expr) return ;
	*status = ALG_SUCCESS ;	
	alg_merge_signed(bexpr->expr, status);
	if (*status != ALG_SUCCESS) return;

	PDXL_NODE node = bexpr->expr->start;
	while (node != NULL)
   	{
		PDXL_OBJECT obj   = node->object ;
		PALG_NODE   anode = obj->obj     ;
		if (anode->type == alg_expr)
		{
			alg_resolve_expr(anode,status) ;
		}
		node = node->right ;
	}

	/*resolve the priorities*/
	alg_resolve_exponents(bexpr->expr,status);
	if (*status != ALG_SUCCESS) return;
	alg_resolve_priority(bexpr->expr,status);
	if (*status != ALG_SUCCESS) return;

	/*store the txt's of the merged nodes*/
	char* merged[ALG_MAXIMUM_MERGED_NODES];/*the maximum merged nodes to merge*/
	for (int i = 0; i < ALG_MAXIMUM_MERGED_NODES; i++) merged[i] = NULL;

	/*store the function name if this is a function*/
	char* func_n = NULL;
	if (bexpr->type == alg_function)
	{
		int slen = strlen(bexpr->txt) ;
		func_n = (char*)malloc(slen+1);
		if (func_n == NULL)
		{
			*status = ALG_MEM_ERROR ;
			return;
		}
		func_n[slen] = 0;
		memcpy(func_n, bexpr->txt, slen); 
	}

	if (bexpr->txt != NULL) free(bexpr->txt) ;
	node = bexpr->expr->start;
	int i = 0;
	while (node != NULL)
	{
		PDXL_OBJECT obj   = node->object;
		PALG_NODE   anode = obj->obj;

		merged[i] = anode->txt;
		i++;
		node = node->right;
	}
	/*get the byte count*/
	int bcount = 0;
	for (int i = 0; i < ALG_MAXIMUM_MERGED_NODES; i++)
	{
		if (merged[i] == NULL) break ;
		bcount = bcount + strlen(merged[i]);
	}

	/*if this is a function add the function name in the calculation*/
	if(bexpr->type == alg_function)
	bcount = bcount + strlen(func_n);
	/*alocate all the memory we need*/
	bexpr->txt = (char*)malloc(bcount + 2 + 1) ; /*+2 for the () and +1 for the terminating zero*/
	if (bexpr->txt == NULL)
	{
		*status = ALG_MEM_ERROR;
		return;
	}

	bexpr->txt[bcount + 2] = 0 ;

	/*concat the nodes*/
	char* expr_indx = bexpr->txt ;
	/*if this is a function add the function name*/
	if (bexpr->type == alg_function)
	{
		char* fn_indx = func_n;
		while (*fn_indx != 0)
		{
			*expr_indx = *fn_indx;
			fn_indx++   ;
			expr_indx++ ;
		}
	}
	free(func_n); /*free the memory of the function name*/
	*expr_indx = '(';
	expr_indx++;
	for (int i = 0; i < ALG_MAXIMUM_MERGED_NODES; i++)
	{
		if (merged[i] == NULL) break;
		char* mtxt = merged[i];
		while (*mtxt != 0)
		{
			*expr_indx = *mtxt;
			mtxt++;
			expr_indx++;
		}
	}
	*expr_indx = ')';
	bexpr->type = alg_merged ; /*the node is merged */

	return;
}

void alg_resolve_all_expr(PALG_EXPRESSION bexpr, int* status)
{
	*status = ALG_SUCCESS;
	PDXL_NODE node = bexpr->start;
	while (node != NULL)
	{
		PDXL_OBJECT obj = node->object;
		PALG_NODE   anode = obj->obj;
		if (anode->type == alg_expr)
		{
			alg_resolve_expr(anode,status);
			if (*status != ALG_SUCCESS) return ;
		}
		node = node->right;
	}

	return;
}

void alg_calc_buff_expr(PALG_EXPRESSION expr,int *bcount)
{
	/*calculate the buffer size*/
	PDXL_NODE node = expr->start;
	while (node != NULL)
	{

		PDXL_OBJECT obj = node->object;
		PALG_NODE anode = obj->obj;
		if (anode->type != alg_expr) *bcount = *bcount + strlen(anode->txt);
		else
		{
			if (anode->txt != NULL)
				*bcount = *bcount + strlen(anode->txt) + 2;/*the 2 bytes for ()*/
			else
				*bcount = *bcount + 2;
			
		}
	
		node = node->right;
	}

	return;
}

void alg_construct_expr(PALG_EXPRESSION expr, char* buff)
{
	PDXL_NODE node = expr->start;
	while (node != NULL)
	{

		PDXL_OBJECT obj = node->object;
		PALG_NODE anode = obj->obj;
		
			char* an_indx = anode->txt;
			if(an_indx!=NULL)
			while (*an_indx != 0)
			{
				*buff = *an_indx;
				buff++;
				an_indx++;
			}
		

		node = node->right;
	}


	return ;
}

char* AlgTransformExpression(char* expr, int* status)
{
	char* buf = NULL;
	/*
	 first check for the validity of the "()"
	*/

	if (dxCheckSectionPairing(expr,'(',')',"`\"") == false)
	{
		*status = ALG_SECTION_INVALID;
		return NULL;
	}

	if (dxCheckSectionPairing(expr, '[', ']', "`\"") == false)
	{
		*status = ALG_BRACKETS_INVALID;
		return NULL;
	}

	/*resolve the expression to data structures*/
	PALG_EXPRESSION bexpr = alg_expression_create(expr, status);

	if (bexpr == NULL) return NULL ;
	/*traverse the plane of numbers and rules and create a NEW EXPRESSIONNN (yes im dizzy and sleepless. Time : 1:22)*/
	alg_merge_signed(bexpr, status);
	if (*status != ALG_SUCCESS) goto end;
	alg_resolve_all_expr(bexpr,status)	   ;
	if (*status != ALG_SUCCESS) goto end;
	alg_resolve_exponents(bexpr,status)    ;
	if (*status != ALG_SUCCESS) goto end;
	alg_resolve_priority(bexpr,status)	   ;
	if (*status != ALG_SUCCESS) goto end;
	/*construct the string*/
	int bcount = 0;
	alg_calc_buff_expr(bexpr, &bcount);
	buf = (char*)malloc(bcount+1);
	if (buf == NULL)
	{
		*status = ALG_MEM_ERROR ;
		goto end;
	}
	buf[bcount] = 0;
	alg_construct_expr(bexpr,buf);

	end:
	 alg_expression_free(bexpr) ;
	 return buf;

}


/******DEBUG*******/

void _PRINT_EXPR_STRUCT(PALG_EXPRESSION expr)
{

	PDXL_NODE node = expr->start;
	while (node != NULL)
	{

		PDXL_OBJECT obj = node->object ;
		PALG_NODE anode = obj->obj     ;
		if (anode->type != alg_expr) printf(anode->txt);
		else
		{
			printf("(");
			_PRINT_EXPR_STRUCT(anode->expr);
			printf(")");
		}
		node = node->right;
	}

	return;
}












