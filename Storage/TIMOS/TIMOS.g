grammar TIMOS;


options {
    language=C;
    output=AST;
    ASTLabelType=pANTLR3_BASE_TREE;
}



tokens {
    TETRA;
    ID4;
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
    
    RGBCOLOUR;
    LEGEND;
    LEGENDENTRY;
}

@parser::members {
	unsigned source_type=0;
    unsigned Np_expect=0;
    unsigned Nt_expect=0;
    unsigned Nm_expect=0;
    unsigned Ns_expect=0;
}

legendfile: (legendentry '\n')* -> ^(LEGEND legendentry*)
	;
	
legendentry: QUOTEDSTRING rgbcolour -> ^(LEGENDENTRY QUOTEDSTRING rgbcolour)
	;
	
rgbcolour: floatconst floatconst floatconst -> ^(RGBCOLOUR floatconst floatconst floatconst)
	;


sourcefile: Nsx=INT { Ns_expect=atoi((const char*)$Nsx->getText($Nsx)->chars); } '\n'! sources
	;
	
	
meshfile: Np=INT { Np_expect=atoi((const char*)$Np->getText($Np)->chars); } '\n'! Nt=INT { Nt_expect=atoi((const char*)$Nt->getText($Nt)->chars); } '\n'! points tetras
	;
	
points @init { unsigned Np=0; }: (point3 { ++Np; } '\n')* { Np==Np_expect }? -> ^(POINTS point3*)
	;
	
matfile: region=INT '\n' Nmx=INT { Nm_expect=atoi((const char*)$Nmx->getText($Nmx)->chars); } '\n' materials matext -> ^(MATFILE $region materials matext)
	;

materials @init { unsigned Nm=0; }: (material { ++Nm; } '\n')* { Nm_expect==Nm }? -> ^(MATERIALS material*)
    ;

matext: matched=INT '\n' n_ext=floatconst? -> ^(EXTERIOR $matched $n_ext)
    ;
	
material: floatconst floatconst floatconst floatconst -> ^(MATERIAL floatconst*)
	;
	
tetras @init { unsigned Nt=0; }: (tetradef { ++Nt; } '\n')* {Nt==Nt_expect}? -> ^(TETRAS tetradef*)
	;

tetradef: id4 INT -> ^(TETRA ^(ID4 id4) INT)
    ;
	
sources @init { unsigned Ns=0; }: (source { ++Ns; } '\n')* { Ns==Ns_expect }? -> ^(SOURCES source*)
	;

source: t=INT { atoi((const char*)$t->getText($t)->chars) == 12 }? id3 w=INT -> ^(SOURCE $w ^(SOURCE_FACE id3))
    |   t=INT { atoi((const char*)$t->getText($t)->chars) == 11 }? tet=INT point3 point3 w=INT -> ^(SOURCE $w ^(SOURCE_PB $tet point3 point3))
    |   t=INT { atoi((const char*)$t->getText($t)->chars) == 1  }? point3 w=INT -> ^(SOURCE $w ^(SOURCE_POINT point3))
    |   t=INT { atoi((const char*)$t->getText($t)->chars) == 2  }? tet=INT w=INT -> ^(SOURCE $w ^(SOURCE_VOL $tet))
    ;

	
floatconst: (INT|FLOAT)^
	;

id4: INT INT INT INT
    ;

id3: INT INT INT -> ^(TRI INT INT INT)
    ;

point3: floatconst floatconst floatconst -> ^(POINT floatconst floatconst floatconst)
    ;

point2: floatconst floatconst -> ^(POINT floatconst floatconst)
    ;
    
QUOTEDSTRING: '"' ('a'..'z'|'A'..'Z'|' '|'_'|'0'..'9'|'('|')')* '"'
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
    :   ('//'|'#') (~'\n')* '\n' {$channel=HIDDEN;}
    ;

WS  :   ( ' '
        | '\t'
        | '\r'
        ) {$channel=HIDDEN; }
    ;

fragment
EXPONENT : ('e'|'E') ('+'|'-')? ('0'..'9')+
	;
