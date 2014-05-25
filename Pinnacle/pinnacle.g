grammar pinnacle;

options {
	language=C; 
	output=AST;
	ASTLabelType=pANTLR3_BASE_TREE;
}

tokens {
	POINT;
	PROP;
}



start
	:	file_stamp^ roi*
	;

file_stamp
	:	FILESTAMP^ '='! '{'! prop* '}'! ';'!
	;
	
surface_mesh
	:	SURFACEMESH^ '='! '{'! (prop|vertices|triangles)* '}'! ';'!
	;
	
triangles 
	:	TRIANGLES^ '='! '{'! '}'! ';'!
	;
	
vertices:	VERTICES^ '='! '{'! '}'! ';'!
	;
	
	
mean_mesh
	:	MEANMESH^ '='! '{'! (prop|vertices|triangles)* '}'! ';'!
	;
	
roi
	: TOK_ROI^ '='! '{'! (prop | curve|surface_mesh|mean_mesh)* '}'! ';'!
	;
	
curve	:	CURVE^ '='! '{'! prop* points '}'! ';'!
	;
	
//	 { printf("  \%s -> \%s\n",$ID->getText($ID)->chars,$ID->getText($ID)->chars); }
//  { printf("  \%s -> \%s\n",$ID->getText($ID)->chars,$ID->getText($INT)->chars); }
// { printf("  \%s -> \%s\n",$ID->getText($ID)->chars,$ID->getText($LITERAL)->chars); }

prop	:	ID '=' FLOAT ';' -> ^(PROP ID FLOAT)
	|	ID '=' INT ';' -> ^(PROP ID INT)
	|	ID LITERAL -> ^(PROP ID LITERAL)
	;
	
points	:	POINTS! '='! '{'! point3* '}'! ';'!
	;
	
point3	:	FLOAT FLOAT FLOAT -> ^(POINT FLOAT FLOAT FLOAT)
	;
	
TRIANGLES
	:	't' 'r' 'i' 'a' 'n' 'g' 'l' 'e' 's'
	;
	
VERTICES:	'v' 'e' 'r' 't' 'i' 'c' 'e' 's'
	;
	
MEANMESH
	:	'm' 'e' 'a' 'n' '_' 'm' 'e' 's' 'h'
	;
	
SURFACEMESH
	:	's' 'u' 'r' 'f' 'a' 'c' 'e' '_' 'm' 'e' 's' 'h'
	;

POINTS	:	'p' 'o' 'i' 'n' 't' 's'
	;

FILESTAMP
	:	'f' 'i' 'l' 'e' '_' 's' 't' 'a' 'm' 'p'
	;

TOK_ROI	:	'r' 'o' 'i'
	;
	
CURVE	:	'c' 'u' 'r' 'v' 'e'
	;
	
LITERAL	:	':' (' '|'\t')* (~('\n'|' '|'\t') ~'\n'*)? '\n'
	;

ID  :	('a'..'z'|'A'..'Z'|'_') ('a'..'z'|'A'..'Z'|'0'..'9'|'_')*
    ;

INT :	'-'? '0'..'9'+
    ;

FLOAT
    :   '-'? ('0'..'9')+ '.' ('0'..'9')* EXPONENT?
    |   '-'? '.' ('0'..'9')+ EXPONENT?
    |   '-'? ('0'..'9')+ EXPONENT
    ;

COMMENT
    :   '//' (~'\n')* '\n' {$channel=HIDDEN;}
    ;

WS  :   ( ' '
        | '\t'
        | '\n'
        | '\r'
        ) {$channel=HIDDEN; }
    ;

fragment
EXPONENT : ('e'|'E') ('+'|'-')? ('0'..'9')+
	;

