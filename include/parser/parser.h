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
  AST_FUNC_PARAM,
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
  Node* type;
  Node* block;
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
  const char* name; // name of identifer
} identifier_expr;

typedef struct {
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
  Node* var_t;
} cast_expr;

typedef struct {
  Node* return_val;
} return_expr;

// TYPE STRUCTS
typedef struct {
  bool is_adr;
  lit_adr_t type_adr;
  lit_t type;
} var_t;

typedef struct {
  Node* ident;
  Node* type; // will be a var_t
} func_param;

typedef struct {
  Node* ret_t; // will be a var_type
  ArrayList* params; // will be func_param
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
    var_type variable_t;
    func_type function_t;
  };
};

// creates a new function type 
// @param ret - the return expr of the func
// @param type - the type of the function param
Node* mk_func_t(Node* ret, ArrayList* params);
// creates a new function paramerters
// @param ident - identifer for this function paramerters
// @param type - the type of the function parameter
Node* mk_func_param(Node* ident, Node* type);

// creates a new variable type node
// @param is_adr - determines if this is an address var, or not
// @param type_adr - if this is an address type, this will hold the adr type
// @param type - if thsi is NOT and address type, this will hold the var type
Node* mk_var_t(bool is_adr, lit_adr_t type_adr, lit_t type);

// creates a new return expression
// @param return_val - the expression that dictates the return expression value
// @return - the newly created return expression
Node* mk_return_expr(Node* return_val); 

// makes a new cast expressions
// @param type - the type of the cast
// @return - the newly created cast expression
Node* mk_cast_expr(Node* type);

// makes a new call expression
// @param callee - the function being called
// @param args - the arguments to the function call
// @return - the newly created call expression
Node* mk_call_expr(Node* callee, ArrayList* args);

// makes a new assign expr
// @param target - the target expression to assign tokens
// @param val - the value to assign the target
// @return - the newly created assign expression
Node* mk_assign_expr(Node* target, Node* val);

// makes a new binary operator
// @param op - the operator to apply to the binary expression
// @param expr_left - the expression on the left of the operator
// @param expr_right - the expression on the right of the operator
// @return - the newly created binary expr
Node* mk_binary_expr(binary_expr_t op, Node* expr_left, Node* expr_right)

// makes a new unary expression based upon an operator and an expression
// @param op - the operator on the unary expression
// @param expr - the expression to apply the unary operator tokens
// @return - the newly created unary operator
Node* mk_unary_expr(unary_expr_t op, Node* expr);

// creates a new literal expression, with either a num_value or str val
// @param num_value - the number value of the literal expression
// @param str_value - the string value of the literal expression
// @return - the newly created literal expression
Node* mk_literal_expr(long num_value, const char* str_value);

// creates a new identifer expressions
// @param name - the name of the identifer
// @return - the newly created identifer expression
Node* mk_identifer_expr(const char* name);

// creates a new if statment node
// @param cond - the conditional statment of the node
// @param then_branch - the expression that execute if cond is true
// @param else_branch - the expressions that occur if the if statment is not true
// @return - the newly created if statment
Node* mk_if_stmt(Node* cond, Node* then_branch, Node* else_branch);

// makes a block statment
// @param nodes - every expression within the block 
// @return - the newly created function block
Node* mk_block_stmt(ArrayList* nodes);

// makes a function declerations
// @param name - the name of the function
// @param type - the functions paramerters, and return type
// @param block - the body of the function
// @return - the newly created function defenition node
Node* mk_func_decl(const char* name, Node* type, Node* block);

// makes a variable declereation
// @param name - the name of the newly created variable
// @param type - the type of the param (var_t)
// @param assign - the exprssion this variable should be assigned to
// @return - the newly created variable decleration node
Node* mk_var_decl(const char* name, Node* type, Node* assign);

// makes initial node for the program
// @param nodes - the list of nodes in the program
// @return - the newly created program node
Node* mk_program_decl(ArrayList* nodes);

// parses function paramerters
// @param parser - the parser to parse from
// @return - the parsed node
Node* parse_func_param(Parser* parser);

// parses a variable type node
// @param parser - the parser to parse from
// @return - the parsed node
Node* parse_var_t(Parser* parser);

// parses an identifer expression
// @param parser - the parser to parse from
// @return - the parsed node
Node* parse_indentifer_expr(Pasrser* parser);

// parses a literal expression
// @param parser - the parser to parse from
// @return - the parsed node
Node* parse_literal_expr(Parser* parser);

// parses a unary expression 
// @param parser - the parser to parse from
// @return - the parsed node
Node* parse_unary_expr(Parser* parser);

// parses a binary expression
// @param parser - the parser to parse from
// @return - the parsed node
Node* parse_binary_expr(Parser* parser);

// parses a cast expression
// @param parser - the parser to parse from
// @return - the parsed node
Node* parse_cast_expr(Parser* parser);

// parses an assign expression
// @param parser - the parser to parse from
// @return - the parsed node
Node* parse_assign_expr(Parser* parser);

// parses a return expression
// @param parser - the parser to parse from
// @return - the newly created node
Node* parse_return_expr(Parser* parser);

// parses a call expression
// @param parser - the parser to parse from
// @return - the parsed node
Node* parse_call_expr(Parser* parser);

// parses an if statment
// @param parser - the parser to parse from
// @return - the parsed node
Node* parse_if_stmt(Parser* parser);

// parses a block statment
// @param parser - the parser to parse from
// @return - the parsed node
Node* parse_block_stmt(Parser* parser);

// parses a variable declerations
// @param parser - the parser to parse from
// @return - the parsed node
Node* parse_var_decl(Parser* parser);

// parses a function decleration
// @param parser - the parser to parse from
// @return - the node the function creates
Node* parse_func_decl(Parser* parser);

// main parser function, parses a list of nodes into AST 
// @param nodes - the list of nodes to parse
// @return - the head of the ast
Node* parse_program(ArrayList* nodes);

typedef struct {
  ArrayList* items;
  Token* cur_tok;
  unsigned long long size;
  unsigned long long idx;
} Parser;

// inits the parser with an array of tokens
// @param tokens - the list of tokens to init the parser with
// @return - the newly created parser
Parser* init_parser(ArrayList* tokens);

// returns whether or not the parser has gone through all tokens
// @param parser - the parser to checks
// @return - true if parser has gone through all tokens, false otherwise
bool p_is_end(Parser* parser);

// advances the given parser foward one token 
// @param parser - the parser to advances
Token* p_advance(Parser* parser);

// peeks at the incoming token of the parser
// @param parser - the parser to peek into
// @return - the incoming token
Token* p_peek(Parser* parser);

// checks if a given token matches a given type
// @param token - the token to match token
// @param type - the type of token to match to
// @return - true if the token matches the type, false otherwise
bool p_match(Token* token, Token_type type);

// inits the parser with an array of tokens
// @param tokens - the array of tokens to init the parser with
void init_parser(ArrayList* tokens);

#endif
