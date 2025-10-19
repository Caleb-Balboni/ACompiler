GRAMMER:



Program         ::= { TopDecl }

TopDecl         ::= FuncDecl | VarDecl

FuncDecl        ::= "let" [ CastExpr ] Ident '(' [ ParamList ] ')' Block

ParamList       ::= Param { ',' Param }

Param           ::= [ CastExpr ] Ident

Block           ::= '{' { Stmt } '}'

Stmt            ::= Block | IfStmt | ReturnStmt | VarDecl | ExprStmt

IfStmt          ::= "if" '(' Expr ')' Block [ "else" Block ]

ReturnStmt      ::= "return" '(' Expr ')' ';'

VarDecl         ::= "let" [ CastExpr ] Ident [ '=' [ CastExpr ] Expr ] ';'

ExprStmt        ::= Expr ';'

Type            ::= '(' PrimType | AdrType ')'

PrimType        ::= "VOID" | "BYTE" | "WORD" | "DWORD" | "QWORD"

AdrType         ::= "VOID&" | "BYTE&" | "WORD&" | "DWORD&" | "QWORD&"

Expr            ::= AddExpr

CastExpr        ::= '(' Type ')'

AssignExpr      ::= CondExpr [ AssignOp ]

AssignOp        ::= RelExpr [ '=' AssignExpr ]

RelExpr         ::= AddExpr ( '<' | '<=' | '>' | '>=' | '==' ) Expr

AddExpr         ::= MulExpr { ( '+' | '-' ) MulExpr }

MulExpr         ::= UnaryExpr { ( '*' | '/' ) UnaryExpr }

UnaryExpr       ::= ( '+' | '-' | '!' | '&' | '*' ) UnaryExpr | Primary

Primay          ::= CallExpr | Ident

CallExpr        ::= Ident '(' Expr [ { ',' Expr } ] ')'

Iden            ::= 'A...Z, a...z' | 0 - 9_
