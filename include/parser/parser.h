#ifndef PARSER_H
#define PARSER_H
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "include/arraylist.h"

// Variable types
typedef enum {
  LIT_STRING,
  LIT_VOID,
  LIT_BYTE,
  LIT_WORD,
  LIT_DWORD,
  LIT_QWORD,
} lit_t;

// Variable address types
typedef enum {
  ADR_BYTE,
  ADR_WORD,
  ADR_DWORD,
  ADR_QWORD,
} lit_adr_t;

// Unary expr types
typedef enum {
  U_POS, // +
  U_NEG, // -
  U_NOT, // !
  U_ADDR, // &
} unary_expr_t;

// binary expr types
typedef enum {
  B_ADD, // + 
  B_SUB, // -
  B_MUL, // *
  B_DIV, // /
  B_LESS, // <
  B_GREATER, // >
  B_EQUAL_EQUAL, // ==
  B_GEQ, // >=
  B_LEQ, // <= 

} binary_expr_t;

// types of AST nodes
typedef enum {
  // declerations
  AST_PROGRAM,
  AST_VAR_DECL,
  AST_FUNC_DECL,
  // statments
  AST_BLOCK,
  AST_IF,
  // expressions
  AST_IDENTIFIER,
  AST_LITERAL,
  AST_UNARY,
  AST_BINARY,
  AST_ASSIGN,
  AST_CALL,
  AST_CAST,
  AST_RETURN,
  // types
  AST_TYPE_VAR,
  AST_TYPE_VAR_ADR,
  AST_TYPE_FUNC,
} ast_t;

// foward decleration
typedef struct Node Node;

// DECLARATION STRUCTS

// Starting AST node
typedef struct {

  ArrayList* nodes;
} program_decl;
 
// Variable decleration node
typedef struct {

  const char* name;
  Node* type;
  Node* assign; 
} var_decl;

// Function delleration node
typedef struct {

  const char* name;
  ArrayList* params;
  ArrayList* body;
  Node* return_t;
} func_decl;

// STATMENT STRUCTS

// Block statment node
typedef struct {

  ArrayList* nodes;
} block_stmt;

typedef struct {

  Node* cond;
  Node* then_branch;
  Node* else_branch;
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

  unary_expr_t op;
  Node* expr;
} unary_expr;

typedef struct {

  binary_expr_t op;
  Node* expr_left;
  Node* expr_right;
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
  ArrayList* params;
} func_type;


// MAIN NODE DEFENITION
struct Node {

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
    identifier_expr identifierExpr;
    literal_expr literalExpr;
    unary_expr unaryExpr;
    binary_expr binaryExpr;
    assign_expr assignExpr;
    call_expr callExpr;
    cast_expr castExpr;
    return_expr returnExpr;
    // types
    var_type varType;
    var_adr_type varAdrType;
    func_type funcType;
  };

};

#endif
