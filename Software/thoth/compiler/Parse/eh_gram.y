
%token	ID 1 CONSTANT 2 STRING 3
%token	AUTO 4 EXTRN 5
%token	ELSE 6 IF 7 FOR 8 REPEAT 9 SELECT 10 WHILE 11
%token	BREAK 12 GOTO 13 NEXT 14 RETURN 15
%token	CASE 16 DEFAULT 17
%token	ENABLE 18 DISABLE 19 TWIT 20 ACTIVE 21
%token	'(' '[' '{' ',' ';' ')' ']' '}'

%token	ASSIGNOP 22 '=' 
%token  '?' ':'
%token	LOGOR 23
%token	LOGAND 24
%token	RELATIONALOP 25
%token	'+' '-'
%token	DIVOP 26 '*'
%token	BITWISEOP 27 '&'
%token	PREUNIOP 28 POSTUNIOP 29
%token	COLCOLN 30
%token	'#'

%right	ASSIGNOP '='
%right	'?' ':'

%left	LOGOR
%left	LOGAND
%left	RELATIONALOP
%left	'+' '-'
%left	DIVOP '*'
%left	BITWISEOP '&'
%left	PREUNIOP POSTUNIOP

%{

#include <stdio.h> 

#include "parse.h"
#include "il_opc.h"

#define YYSTYPE int

int Nxt_label;
int Brk_label;

void Enter_loop(void);
void Leave_loop(void);

void Do_relop_case(int relop, int value);

int In_twit;

%}

%%

IL_program		:	list_modules
			;
list_modules		:	module
			|	list_modules module
			;
module			:	function
			|	external ';'
			|	error
			;
function		:	func_name func_header fstatement
				{ Put('R'); Put(0); Put(0); Ref_clear(); }
			;
func_header		:	list_of_arguments ')' rbr
				{ Put('P'); PutW($1); Put($3); }
			;
rbr			:
				{ $$ = 255; }
			|	'#' '(' con_expr ')'
				{ $$ = 255; }
			|	'#' '(' con_expr con_expr ')'
				{ $$ = 255; }
			;
func_name		:	ID '('
				{ Put('('); PutW($1); }
			;
list_of_arguments	:
			|	'?'
                                { $$ = 255; }
			|	argument_list
                                { $$ = ($1<<8) | $1; }
			|	argument_list ';'
                                { $$ = ($1<<8) | $1; }
                        |	';' argument_list
                                { $$ = $2; }
			|	argument_list ';' argument_list
                                { $$ = ($1<<8) | ($1 + $3); }
			|	argument_list ';' '?'
                                { $$ = ($1<<8) | 255; }
			;
argument_list		:	ID
				{ Ref_ident($1, 'a', NEW); $$ = 1; }
			|	argument_list ',' ID
				{ Ref_ident($3, 'a', NEW); $$ = $1 + 1; }
			;
external		:	external_word initial_word
			|	external_ptr_word list_word_data
			|	external_ptr_byte initial_byte
			;
external_word		:	ID rbr
				{ Put(':'); PutW($1); DPut(':'); DPutW($1); DPut($2); }
			;
external_ptr_word	:	ID '[' constant... ']' rbr
				{ Put(':'); PutW($1); DPut('['); DPutW($1); DPut($5); DPutC($3); }
			;
external_ptr_byte	:	ID '{' constant... '}' rbr
				{ Put(':'); PutW($1); DPut('"'); DPutW($1); DPut($5); DPutC($3); }
			;
con_expr		:	expr
				{ $$ = Expr_const($1); }
			;
constant...		:	con_expr
			;
list_word_data		:
				{ DPutC(0); DPut(0); }
			|	':' list_words
				{ DPut(0); }
			;
initial_byte		:	
				{ DPutW(0); }
			|	':' STRING
				{ DPutW($2); }
			;
initial_word		:
				{ DPutC(0); }
			|	':' expr
				{ Expr_extrn($2); }
			;
list_words		:	list_words ',' word_list_element
			|	word_list_element
			;
hook			:
				{ DPut('['); }
			;
word_list_element	:	expr
				{ Expr_extrn($1); }
			|	'[' hook list_words ']'
				{ DPut(']'); }
			;
statement		:	label ':' statement
			|	'{' stat_list '}'
			|	compound_statement
			|	simple_statement ';'
			|	';'
			|	error
			;
label			:	ID
				{ $$ = Ref_ident($1, 'l', MAY); Put('l'); PutR($$); }
			;
fstatement		:	hend compound_statement
			|	hend simple_statement ';'
			|	hend ';'
			|	'{' fstat_list '}'
			|	error
			;
fstat_list		:	scope_stmt_list hend stat_list
			|	hend stat_list
			;
hend			:
				{ Put('H'); }
			;
scope_stmt_list		:	scope_stmt_list scope_stmt ';'
			|	scope_stmt ';'
			;
scope_stmt		:	AUTO list_auto_defns
			|	EXTRN list_extrn
			;
list_auto_defns		:	auto_defn
			|	list_auto_defns	',' auto_defn
			;
auto_defn		:	ID
				{ Ref_ident($1, 'A', NEW); }
			|	ID '[' con_expr ']'
				{ Ref_ident($1, 'V', NEW); PutW($3); }
			|	ID '{' con_expr '}'
				{ Ref_ident($1, 'v', NEW); PutW($3); }
			;
list_extrn		:	extrn_var
			|	list_extrn ',' extrn_var
			;
extrn_var		:	ID
				{ Ref_ident($1, 'g', NEW); }
			;
stat_list		:	statement
			|	stat_list statement
			;
simple_statement	:	transfer_stmt
			|	interrupt_control_stmt
			|	twit_stmt
			|	expr
				{ Expr_check($1); Put('E'); Expr_put($1); }
			;
transfer_stmt		:	BREAK
				{ if( Brk_label == 0 ) yyerror("break outside loop"); Put('J'); PutR(Brk_label); } 
			|	NEXT
				{ if( Nxt_label == 0 ) yyerror("next outside loop");  Put('J'); PutR(Nxt_label); } 
			|	RETURN '(' expr ')'
				{ Expr_check($3); Put('R'); Expr_put($3); }
			|	RETURN
				{ Put('R'); Put(0); }
			|	GOTO ID
				{ Put('J'); PutR(Ref_ident($2, 'l', MAY)); }
			;
interrupt_control_stmt	:	ENABLE
				{ Put('e'); }
			|	DISABLE
				{ Put('d'); }
			;
twit_kwd 		:	TWIT
				{ In_twit = TRUE; }
			;
twit_stmt		:	twit_kwd '(' parm_list ')'
				{ Expr_check($3); Put('t'); Expr_put($3); In_twit = FALSE; }
			;
compound_statement	:	then_clause statement
				{ Put('l'); PutR($1); }
			|	if_then_else statement
				{ Put('l'); PutR($1); }
			|	while_clause statement
				{ Leave_loop(); }				
			|	for_clause statement
				{ Leave_loop(); }
			|	repeat_clause statement
				{ Leave_loop(); }
			|	w_sel_clause '{' w_case_s.list '}'
				{ Put('w'); }
			|	b_sel_clause '{' b_case_s.list '}'
				{ Put('s'); }
			;
then_clause		:	IF '(' expr ')'
				{ Expr_check($3); $$ = Ref_label(); Put('F'); PutR($$); Expr_put($3); }
			;
if_then_else		:	then_clause statement ELSE
				{ $$ = Ref_label(); Put('J'); PutR($$); Put('l'); PutR($1); }
			;
repeat_clause		:	REPEAT
				{ Enter_loop(); }
			;
while_clause		:	WHILE '(' expr ')'
				{ Expr_check($3); Enter_loop(); Put('F'); PutR(Brk_label); Expr_put($3); }
			;
for_clause		:	FOR '(' exp. ';' exp. ';' exp. ')'
				{ int l = Ref_label(); Expr_check($3); Put('E'); Expr_put($3);
				  Put('J'); PutR(l);
				  Enter_loop(); 
				  Expr_check($7); Put('E'); Expr_put($7); Put('l'); PutR(l);
				  Expr_check($5); Put('F'); PutR(Brk_label); Expr_put($5); }
			;
exp.			:
				{ $$ = 0; }
			|	expr
			;
w_sel_clause		:	SELECT '(' expr ')'
				{ Expr_check($3); Put('W'); Expr_put($3); }
			;
b_sel_clause		:	SELECT '{' expr '}'
				{ Expr_check($3); Put('S'); Expr_put($3); }
			;
w_case_s.list		:	w_case_stat w_case_s.list
			|	w_case_stat
			;
b_case_s.list		:	b_case_stat b_case_s.list
			|	b_case_stat
			;
w_case_stat		:	wocase_list statement
			;
b_case_stat		:	bycase_list statement
			;
wocase_list		:	case_const_wo ':'
			|	wocase_list case_const_wo ':'
			;
bycase_list		:	case_const_by ':'
			|	bycase_list case_const_by ':'
			;
case_const_wo		:	CASE con_expr
				{ Put('i'); PutW($2); }
			|	CASE con_expr COLCOLN con_expr
				{ Put('r'); PutW($2); PutW($4); }
			|	CASE RELATIONALOP con_expr
				{ Do_relop_case($2, $3); }
			|	DEFAULT
				{ Put('D'); }
			;
case_const_by		:	CASE STRING
				{ Put('b'); PutW($2); }
			|	DEFAULT
				{ Put('D'); }
			;
expr			:	expr ASSIGNOP expr
				{ $$ = Expr_node( $2, 0, $3, $1 ); }
			|	expr '=' expr
				{ $$ = Expr_node( IL_ASSIGN, 0, $3, $1 ); }
			|	expr '?' expr ':' expr
				{ $$ = Expr_node( IL_QUERY, 0, $1, Expr_node(IL_COLON, 0, $3, $5)); }
			|	expr LOGOR expr
				{ $$ = Expr_node( IL_LOGOR, 0, $1, $3 ); }
			|	expr LOGAND expr
				{ $$ = Expr_node( IL_LOGAND, 0, $1, $3 ); }
			|	expr RELATIONALOP expr
				{ $$ = Expr_node( $2, 0, $1, $3 ); }
			|	expr BITWISEOP expr
				{ $$ = Expr_node( $2, 0, $1, $3 ); }
			|	expr '&' expr
				{ $$ = Expr_node( IL_BITAND, 0, $1, $3 ); }
			|	expr '+' expr
				{ $$ = Expr_node( IL_ADD, 0, $1, $3 ); }
			|	expr '-' expr
				{ $$ = Expr_node( IL_SUB, 0, $1, $3 ); }
			|	expr '*' expr
				{ $$ = Expr_node( IL_MUL, 0, $1, $3 ); }
			|	expr DIVOP expr
				{ $$ = Expr_node( $2, 0, $1, $3 ); }
			|	primary
			;
primary			:	var
			|	PREUNIOP primary
				{ $$ = Expr_node( $1, 0, $2, 0 ); }
			|	'&' primary
				{ $$ = Expr_node( IL_RVAL, 0, $2, 0 ); }
			|	'*' primary
				{ $$ = Expr_node( IL_LVAL, 0, $2, 0 ); }
			|	'-' primary
				{ $$ = Expr_node( IL_NEG, 0, $2, 0 ); }
			|	POSTUNIOP primary
				{ $$ = Expr_node( ($1==IL_POSTINC ? IL_PREINC : IL_PREDEC), 0, $2, 0 ); }
			;
var			:	ID
				{ $$ = Expr_leaf($1, 0, TNAME); } 
			|	CONSTANT
				{ $$ = Expr_leaf($1, 0, TCONST); } 
			|	STRING
				{ $$ = Expr_leaf($1, 0, TSTR); } 
			|	'(' expr ')'
				{ $$ = $2; }
			|	var POSTUNIOP
				{ $$ = Expr_node($2, 0, $1, 0 ); }
			|	var '[' expr ']'
				{ $$ = Expr_node( IL_WIDX, 0, $3, $1 ); };
			|	var '{' expr '}'
				{ $$ = Expr_node( IL_BIDX, 0, $3, $1 ); };
			|	var '(' parm_list ')'
				{ $$ = Expr_node( IL_NFCTN_CALL, 0, $3, $1 ); };
			;
parm_list		:
				{ $$ = Expr_node( IL_MARK, 0, 0, 0 ); }
			|	parm_list_nn
			;
parm_list_nn		:	expr
				{ $$ = Expr_node( IL_COMMA, 0, Expr_node( IL_MARK, 0, 0, 0 ), $1); }
			|	parm_list_nn ',' expr
				{ $$ = Expr_node( IL_COMMA, 0, $1, $3); }
			;
%%

#define MAXLOOP 20

int Nxt_stk[MAXLOOP];
int Brk_stk[MAXLOOP];
int Loop_lvl;

void
Enter_loop(void)
{
	if( Loop_lvl == MAXLOOP ) {
		yyerror("loop nested too many levels");
	}
	Nxt_stk[Loop_lvl] = Nxt_label;
	Brk_stk[Loop_lvl] = Brk_label;
	Loop_lvl++;
	Nxt_label = Ref_label();
	Brk_label = Ref_label();
	
	Put('l'); PutR(Nxt_label);
}

void
Leave_loop(void)
{
	if( Loop_lvl == 0 ) {
		yyerror("loop nesting error");
	}
	
	Put('J'); PutR(Nxt_label);
	Put('l'); PutR(Brk_label);
	
	Loop_lvl--;
	Nxt_label = Nxt_stk[Loop_lvl];
	Brk_label = Brk_stk[Loop_lvl];
}

int minint = 0x8000;
int maxint = 0x7fff;

void
Do_relop_case(int relop, int value)
{
	switch( relop ) {
	case IL_LT:	Put('r'); PutW(minint);    PutW(value - 1); break; 
	case IL_LTE:	Put('r'); PutW(minint);    PutW(value);     break;
	case IL_GT:	Put('r'); PutW(value + 1); PutW(maxint);    break;
	case IL_GTE:	Put('r'); PutW(value);     PutW(maxint);    break;
	default:
		yyerror("bad relational op in case clause");
	}
}
