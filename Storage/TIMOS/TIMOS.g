grammar TIMOS;


options {
    language=C;
    output=AST;
    ASTLabelType=pANTLR3_BASE_TREE;
}



tokens {
    TETRA;
    TRI;
    POINT;
    TETRAS;
    POINTS;
    
    SOURCE;
    
    SOURCES;

    MATFILE;

    MATERIALS;
    
    MATERIAL;
    EXTERIOR;
    
    SOURCE_PB;
    SOURCE_VOL;
    SOURCE_FACE;
    SOURCE_POINT;
}

@parser::members {
	unsigned source_type=0;
    unsigned Np_expect=0;
    unsigned Nt_expect=0;
    unsigned Nm_expect=0;
}


sourcefile: INT '\n'! sources
	;
	
	
meshfile: Np=INT { Np_expect=atoi((const char*)$Np->getText($Np)->chars); } '\n'! Nt=INT { Nt_expect=atoi((const char*)$Nt->getText($Nt)->chars); } '\n'! points tetras
	;
	
points @init { unsigned Np=0; }: (point3 { ++Np; } '\n')* { Np==Np_expect }? -> ^(POINTS point3*)
	;
	
matfile: region=INT '\n' Nmx=INT { Nm_expect=atoi((const char*)$Nmx->getText($Nmx)->chars); } '\n' (material  '\n')* matched=INT '\n' n_ext=floatconst? -> ^(MATFILE $region material* $matched $n_ext)
	;

materials @init { unsigned Nm=0; }: material { ++Nm; } '\n' { Nm_expect==Nm }? -> ^(MATERIAL material)
    ;

matext: matched=INT '\n' n_ext=floatconst? -> ^(EXTERIOR $matched $n_ext)
    ;
	
material: floatconst floatconst floatconst floatconst -> ^(MATERIAL floatconst floatconst floatconst floatconst)
	;
	
tetras @init { unsigned Nt=0; }: (id4 INT { ++Nt; } '\n')* {Nt==Nt_expect}? -> ^(TETRAS (id4 INT)*)
	;
	
sources: (source '\n')* -> ^(SOURCES source*)
	;

source: type=INT { source_type=atoi((const char*)$type->getText($type)->chars); } sourcedetails w=INT-> ^(SOURCE $w sourcedetails)
	;
	
sourcedetails: { source_type==2 }? INT -> ^(SOURCE_VOL INT)
	| { source_type==1 }? point3 -> ^(SOURCE_POINT point3)
	| { source_type==11 }? INT point3 point3 -> ^(SOURCE_PB INT point3 point3)
	;
	
	
//facesource: type=INT { atoi((const char*)$type->getText($type))==1 }? INT point3 point3 INT
//	;
	
floatconst: (INT|FLOAT)^
	;

id4: INT INT INT INT -> ^(TETRA INT INT INT INT)
    ;

id3: INT INT INT -> ^(TRI INT INT INT)
    ;

point3: floatconst floatconst floatconst -> ^(POINT floatconst floatconst floatconst)
    ;

point2: floatconst floatconst -> ^(POINT floatconst floatconst)
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
        | '\r'
        ) {$channel=HIDDEN; }
    ;

fragment
EXPONENT : ('e'|'E') ('+'|'-')? ('0'..'9')+
	;
