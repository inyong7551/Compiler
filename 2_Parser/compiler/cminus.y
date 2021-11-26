/****************************************************/
/* File: tiny.y                                     */
/* The TINY Yacc/Bison specification file           */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/
%{
#define YYPARSER /* distinguishes Yacc output from other code files */

#include "globals.h"
#include "util.h"
#include "scan.h"
#include "parse.h"

#define YYSTYPE TreeNode *
static int savedLineNo;  /* ditto */
static int savedNum;
static TreeNode * savedTree; /* stores syntax tree for later return */
static int yylex(void); // added 11/2/11 to ensure no conflict with lex

%}

%token IF ELSE WHILE RETURN INT VOID
%token ID NUM 
%token LPAREN RPAREN LBRACE RBRACE LCURLY RCURLY SEMI
%left TIMES OVER
%left PLUS MINUS
%left LT LE GT GE
%left EQ NE
%right ASSIGN
%left COMMA

%token ERROR 

%% /* Grammar for C-minus */

program     : dec_list
                { savedTree = $1; } 
            ;

dec_list    : dec_list dec
	    	{ YYSTYPE t = $1;
		  if (t != NULL)
		  { while (t->sibling != NULL)
		      t = t->sibling;
		    t->sibling = $2;
		    $$ = $1; }
		  else $$ = $2;
		}   
	    | dec { $$ = $1; }
	    ;

dec	    : var_dec { $$ = $1; }
	    | fun_dec { $$ = $1; }
	    ;

var_dec     : type_spec id SEMI
		{ $$ = newDecNode(VarK);
		  $$->attr.name = $2->attr.name;
		  $$->lineno = savedLineNo;
		  $$->type = $1->type;
		  free($1); 
	 	  free($2);
		}	     
	    | type_spec id LBRACE num RBRACE SEMI
		{ $$ = newDecNode(VarK);
		  $$->attr.name = $2->attr.name;
		  $$->lineno = savedLineNo;
		  $$->type = $1->type;
		  $$->child[0] = newExpNode(ConstK);
		  $$->child[0]->attr.val = savedNum;
		  free($1);
	 	  free($2);
		}
	    ;

type_spec   : INT
	    	{ $$ = newTypeNode(IntK);
		  $$->type = Integer;
		}
	    | VOID
		{ $$ = newTypeNode(VoidK);
		  $$->type = Void;
		}
	    ;

fun_dec	    : type_spec id LPAREN params RPAREN comp_stmt
		{ $$ = newDecNode(FunK);
		  $$->attr.name = $2->attr.name;
		  $$->lineno = savedLineNo;
		  $$->type = $1->type;
		  $$->child[0] = $4;
		  $$->child[1] = $6;
		  free($1);
		  free($2);
	 	}
	    ;

params 	    : param_list { $$ = $1; }
	    | VOID
		{ $$ = newPrmNode(SingleK);
		  $$->type = Void;
		}
	    ;

param_list  : param_list COMMA param
	        { YYSTYPE t = $1;
		  if (t != NULL)
		  { while (t->sibling != NULL)
		      t = t->sibling;
		    t->sibling = $3;
		    $$ = $1; }
		  else $$ = $3;
		} 
	    | param { $$ = $1; }
	    ;

param       : type_spec id
	    	{ $$ = newPrmNode(SingleK);
		  $$->attr.name = $2->attr.name;
		  $$->type = $1->type;
		  free($1);
		  free($2);
		}
	    | type_spec id LBRACE RBRACE
		{ $$ = newPrmNode(ArrayK);
		  $$->attr.name = $2->attr.name;
   		  $$->type = $1->type;
		  free($1);
		  free($2);
		}
	    ;

comp_stmt   : LCURLY local_dec stmt_list RCURLY
	    	{ $$ = newStmtNode(CompoundK);
		  $$->child[0] = $2;
		  $$->child[1] = $3;
		}
	    ;

local_dec   : local_dec var_dec
	    	{ YYSTYPE t = $1;
		  if (t != NULL)
		  { while (t->sibling != NULL)
		      t = t->sibling;
		    t->sibling = $2;
		    $$ = $1; }
		  else $$ = $2;
		}
	    | { $$ = NULL; }
	    ;

stmt_list   : stmt_list stmt
	    	{ YYSTYPE t = $1;
		  if (t != NULL)
		  { while (t->sibling != NULL)
		      t = t->sibling;
		    t->sibling = $2;
		    $$ = $1; }
	  	  else $$ = $2;
		}
	    | { $$ = NULL; }
	    ;

stmt 	    : exp_stmt { $$ = $1; }
	    | comp_stmt { $$ = $1; }
	    | select_stmt { $$ = $1; }
	    | iter_stmt { $$ = $1; }
	    | ret_stmt { $$ = $1; }
            ;

exp_stmt    : exp SEMI
	    	{ $$ = $1; }
	    | SEMI
		{ $$ = NULL; }
	    ;

select_stmt : IF LPAREN exp RPAREN stmt
	    	{ $$ = newStmtNode(IfK);
		  $$->child[0] = $3;
		  $$->child[1] = $5;
		}
	    | IF LPAREN exp RPAREN stmt ELSE stmt
		{ $$ = newStmtNode(IfelseK);
		  $$->child[0] = $3;
		  $$->child[1] = $5;
		  $$->child[2] = $7;
		}
	    ;

iter_stmt   : WHILE LPAREN exp RPAREN stmt
	    	{ $$ = newStmtNode(WhileK);
		  $$->child[0] = $3;
		  $$->child[1] = $5;
		}
	    ;

ret_stmt    : RETURN SEMI
	    	{ $$ = newStmtNode(ReturnK); }
	    | RETURN exp SEMI
		{ $$ = newStmtNode(ReturnK);
		  $$->child[0] = $2;
		}
	    ;

exp	    : var ASSIGN exp
		{ $$ = newExpNode(AssignK);
		  $$->child[0] = $1;
		  $$->child[1] = $3;
		}
	    | simple_exp { $$ = $1; }
	    ;

var 	    : id
	 	{ $$ = newExpNode(VariableK);
		  $$->attr.name = $1->attr.name;
		  free($1);
		}
	    | id LBRACE exp RBRACE
		{ $$ = newExpNode(VariableK); 
		  $$->attr.name = $1->attr.name;
		  $$->child[0] = $3;
		  free($1);
		}
	    ;

simple_exp  : add_exp relop add_exp
	    	{ $$ = newExpNode(OpK);
		  $$->child[0] = $1;
		  $$->child[1] = $3;
		  $$->attr.op = $2->attr.op;
		  free($2);
		}
	    | add_exp { $$ = $1; }
	    ;

relop	    : LE
	  	{ $$ = newOpNode(RelopK);
		  $$->attr.op = LE;
	  	}
	    | LT
		{ $$ = newOpNode(RelopK);
		  $$->attr.op = LT;
		}
	    | GT
		{ $$ = newOpNode(RelopK);
		  $$->attr.op = GT;
		}
	    | GE
		{ $$ = newOpNode(RelopK);
		  $$->attr.op = GE;
		}
	    | EQ
		{ $$ = newOpNode(RelopK);
		  $$->attr.op = EQ;
		}
	    | NE
		{ $$ = newOpNode(RelopK);
		  $$->attr.op = NE;
		}
	    ;
		

add_exp     : add_exp addop term
	    	{ $$ = newExpNode(OpK);
		  $$->child[0] = $1;
		  $$->child[1] = $3;
		  $$->attr.op = $2->attr.op;
		  free($2);
	 	}
	    | term { $$ = $1; }
	    ;

addop	    : PLUS
	  	{ $$ = newOpNode(AddopK);
		  $$->attr.op = PLUS; }
	    | MINUS
		{ $$ = newOpNode(AddopK);
		  $$->attr.op = MINUS; }
	    ;

term 	    : term mulop factor
	  	{ $$ = newExpNode(OpK);
		  $$->child[0] = $1;
		  $$->child[1] = $3;
		  $$->attr.op = $2->attr.op;
		  free($2);
		}
	    | factor { $$ = $1; }
	    ;

mulop	    : TIMES
	  	{ $$ = newOpNode(MulopK);
		  $$->attr.op = TIMES;
		}
	    | OVER
		{ $$ = newOpNode(MulopK);
		  $$->attr.op = OVER;
		}
	    ;

factor	    : LPAREN exp RPAREN { $$ = $2; }
	    | var { $$ = $1; }
	    | call { $$ = $1; }
	    | num
		{ $$ = newExpNode(ConstK);
		  $$->attr.val = savedNum;
		}
	    ;

call	    : id LPAREN args RPAREN
	 	{ $$ = newExpNode(CallK);
		  $$->attr.name = $1->attr.name;
		  $$->child[0] = $3;
		  free($1);
		}
	    ;

args	    : arg_list { $$ = $1; }
	    | { $$ = NULL; }
	    ;

arg_list    : arg_list COMMA exp
	    	{ YYSTYPE t = $1;
		  if (t != NULL)
		  { while (t->sibling != NULL)
		      t = t->sibling;
		    t->sibling = $3;
		    $$ = $1; }
		  else $$ = $3; 
		}
	    | exp { $$ = $1; }
	    ;

id	    : ID
       		{ $$ = newExpNode(IdK);
		  $$->attr.name = copyString(tokenString);
		  savedLineNo = lineno;
		}
	    ;

num	    : NUM
	        { savedNum = atoi(tokenString);
		  savedLineNo = lineno;
		}
	    ;
%%

int yyerror(char * message)
{ fprintf(listing,"Syntax error at line %d: %s\n",lineno,message);
  fprintf(listing,"Current token: ");
  printToken(yychar,tokenString);
  Error = TRUE;
  return 0;
}

/* yylex calls getToken to make Yacc/Bison output
 * compatible with ealier versions of the TINY scanner
 */
static int yylex(void)
{ return getToken(); }

TreeNode * parse(void)
{ yyparse();
  return savedTree;
}

