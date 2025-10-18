#include <stdlib.h>
#include <stdio.h>

typedef enum {
  AST_STRING,
  AST_VOID,
  AST_BYTE,
  AST_WORD,
  AST_DWORD,
  AST_QWORD,
  AST_BYTE_ADR,
  AST_WORD_ADR,
  AST_DWORD_ADR,
  AST_QWORD_ADR,
} lit_t;

typedef enum {
  AST_BYTE_ADR,
  AST_WORD_ADR,
  AST_DWORD_ADR,
  AST_QWORD_ADR,
} lit_adr_t;

typedef enum {
  U_POS, // +
  U_NEG, // -
  U_NOT, // !
  U_ADDR, // &
} u_expr_t;

typedef enum {
  B_ADD, // + 
  B_SUB, // -
  B_MUL, // *
  B_DIV, // /
  B_LESS, // <
  B_GREATER, // >
  B_EQUAL, // =
  B_EQUAL_EQUAL, // ==
  B_GEQ, // >=
  B_LEQ, // <=
} b_expr_t;

typedef enum {
  B_EQUAL, // =
  B_EQUAL_EQUAL, // ==
  B_GEQ, // >=
  B_LEQ, 
} b_cond_expr_t;

typedef enum {
  // declerations
  AST_PROGRAM,
  AST_VAR_DECL,
  AST_FUNC_DECL,
  // statments
  AST_BLOCK,
  AST_IF,
  // expressions
  AST_IDENTIFER,
  AST_LITERAL,
  AST_UNARY,
  AST_BINARY,
  AST_ASSIGN,
  AST_CALL,
  AST_CAST,
  AST_COND,
  AST_RETURN,
  // types
  AST_TYPE_VAR,
  AST_TYPE_VAR_ADR,
  AST_TYPE_FUNC,
} ast_t;

// DECLARATION STRUCTS
typedef struct {
  ArrayList* nodes;
} program_decl;
  
typedef struct {
  const char* name;
  Node* type;
  Node* assign; 
} var_decl;

typedef struct {
  const char* name;
  ArrayList* args;
  ArrayList* block;
  Node* return_t;
} func_decl;

// STATMENT STRUCTS
typedef struct {
  ArrayList* nodes;
} block_stmt;

typedef struct {
  Node* cond;
  Node* block;
  Node* else_cond;
} if_stmt;

// EXPRESSION STRUCTS

typedef struct {
  const char* name;
} identifier_expr;

typedef struct {
  lit_t type;
  long num_value;
  const char* str_value; 
} literal_expr;

typedef struct {
  u_expr_t op;
  Node* block;
} unary_expr;

typedef struct {
  b_expr_t op;
  Node* block_left;
  Node* block_right;
} binary_expr;

typedef struct {
  Node* target;
  Node* val;
} assign_expr;

typedef struct {
  Node* callee;
  ArrayList* args;
} call_expr;

typedef struct {
  Node* cast_type;
  Node* expr;
} cast_expr;

typedef struct {
  b_cond_expr_t cond;
  Node* left;
  Node* right;
} cond_expr;

typedef struct {
  Node* return_val;
} return_expr;

// TYPE STRUCTS

typedef struct {
  lit_t type;

} var_type;

typedef struct {
  lit_adr_t type;
} var_adr_type;

typedef struct {
  bool isAdr;
  lit_t lit_ret_type;
  lit_adr_t lit_adr_ret_type;
  Arraylist* params;
} func_type;


// MAIN NODE DEFENITION
typedef struct {
  ast_t type;
  
  union {
    // declerations
    program_decl programDecl;
    var_decl varDecl;
    func_decl funcDecl; 
    // statments
    block_stmt blockStmt;
    if_stmt ifStmt;
    // expressions
    identifer_expr identiferExpr;
    literal_expr literalExpr;
    unary_expr unaryExpr;
    binary_expr binaryExpr;
    assign_expr assignExpr;
    call_expr callExpr;
    cast_expr castExpr;
    cond_expr condExpr;
    return_expr returnExpr;
    // types
    var_type varType;
    var_adr_type varAdrType;
    func_type funcType;
  };

} Node;


