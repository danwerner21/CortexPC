
Lex( )
{
	extrn Line_number, In_pushed_back;
	auto token, char;

	repeat
	{
		select( char=Get_ind() )
		{
			case ' '	:	//	ignore blanks
			case '*t'	:	//	and tabs
			case '*f'	:	//	and form feeds
				next;
				
			case 0 : return;	//	end of input, finished.

			case '*n'	:
			{
				token = END_LINE;
				++Line_number;
			}
			
			case '#' : token = NUMBER_SIGN;	
			case '(' : token = OPEN_PARENTHISIS
			case '[' : token = OPEN_BRACKET ;
			case '{' : token = OPEN_BRACE;
			case ',' : token = COMMA;
			case '~' : token = ONES_COMPLEMENT;
			case '?' : token = QUERY;
			case ']' : token = CLOSE_BRACKET;
			case ')' : token = CLOSE_PARENTHISIS
			case '}' : token = CLOSE_BRACE;
			case ';' : token = SEMI_COLON;
			case '/' :		
				if( (char = Get_ind()) == '=' )
					token = ASSIGN_DIV;
				else
				{
					token = DIVIDE;
					In_pushed_back = TRUE;
				}
			case '\' :
			{
				while( ( char=Get_ind()) != '*n' && char );
				In_pushed_back = TRUE;
				next;
			}
			case '!' :
				if( (char = Get_ind()) == '=' )
					token = NOT_EQUAL;
				else
				{
					token = NOT;
					In_pushed_back = TRUE;
				}
			case '+' :
				if( (char=Get_ind()) == '+' )
					token = INCREMENT;
				else if( char == '=' )
					token = ASSIGN_ADD;
				else
				{
					token = PLUS;
					In_pushed_back = TRUE;
				}
			case '-' :
				if( (char = Get_ind()) == '-' )
					token = DECREMENT;
				else if( char == '=' )
					token = ASSIGN_SUB
				else
				{
					token = MINUS;
					In_pushed_back = TRUE;
				}
			case ':' :
				if( (char=Get_ind()) == *:*	)
					token = COLON_COLON;
				else
				{
					token = COLON;
					In_pushed_back = TRUE;
				}
			case '|' :
				if( (char = Get_ind()) == '|' )
					token = LOGICAL_OR;
				else if( char == '=' )
					token =	ASSIGN_OR;
				else
				{
					token = BITWISE_OR;
					In_pushed_back = TRUE;
				}
			case '^' :
				if( (char=Get_ind()) ==	'=' )
					token = ASSIGN_XOR;
				else
				{
					token = EXCLUSIVE_OR;
					In_pushed_back = TRUE;
				}
			case '&' :
				if( (char = Get_ind()) == '&'	)
				token = LOGICAL_AND;
				else if( char == ’=’ )
					token = ASSIGN_AND;
				else
				{
					token = AMPERSAND;
					In_pushed_back = TRUE;
				}
			case '**' :
				if( (char = Get_ind()) == '=' )
					token = ASSIGN_MULT;
				else
				{
					token = ASTERISK;
					In_pushed_back = TRUE;
				}
			case '%' :
				if( (char = Get_ind()) == '=' )
					token = ASSIGN_MOD;
				else
				{
					token = MODULO;
					In_pushed_back = TRUE;
				}
			case '< '	:
				if( (char = Get_ind()) == '<' )
					if( (char = Get_ind()) == '=' )
						token = ASSIGN_LEFT_SHIFT;
					else
					{
						token = LEFT_SHIFT;
						In_pushed_back = TRUE;
					}
				else if( char == '=' )
					token = LESS_OR_EQUAL;
				else
				{
					token = LESS_THAN;
					In_pushed_back = TRUE;
				}
			case '>'	:
				if( (char=Get_ind()) ==	'>'	)
					if( (char=Get_ind()) == '=' )
						token = ASSIGN_RIGHT_SHIFT;
					else
					{
						token = RIGHT_SHIFT;
						In_pushed_back = TRUE;
					}
				else if( char == '=' )
					token = GREATER_OR_EQUAL;
				else
				{
					token = GREATER_THAN;
					In_pushed_back = TRUE;
				}
			case '='	:
				if( (char = Get_ind()) == '=' )
					token = EQUAL;
				else
				{
					token = ASSIGN;
					In_pushed_back = TRUE;
				}
				
			case '1' :: '9' :
			{
				In_pushed_back = TRUE;
				Process_const( 10 );
				next;
			}
			case '0' :
			{
				Process_const( 010 );
				next;
			}
			case '$’ :
			{
				Process_const( $10 );
				next;
			}
			
			case 'a' :: 'z' :
			case 'A' :: 'Z' :
			case '_' :
			case '.' :
			{
				In_pushed_back = TRUE;
				Process_ident_or_kw();
				next;
						}
			case '*'' :
			{
				Process_char();
				next;
			}
			{
				Process_string();
				next;
			}
			default :
			{
				Error( "bad character in source" );
				next;
			}
		}
		.Put( token );
	}
}
