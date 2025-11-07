#ifndef PARSER_H
#define PARSER_H
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "utils/arraylist.h"
#include "tokenizer/tokens.h"

// Variable types
typedef enum {
  LIT_STRING,
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
  B_NOT_EQUAL, // !=
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
  AST_RETURN,
  AST_COMMENT,
  // expressions
  AST_IDENTIFIER,
  AST_LITERAL,
  AST_UNARY,
  AST_BINARY,
  AST_ASSIGN,
  AST_CALL,
  AST_CAST,
  AST_FUNC_PARAM,
  // types
  AST_TYPE_VAR,
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
  Node* ident;
  Node* type;
  Node* assign; 
} var_decl;

// Function delleration node
typedef struct {
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

typedef struct {
  char* comment; 
} comment_stmt;

// EXPRESSION STRUCTS

typedef struct {
  const char* name; // name of identifer
} identifier_expr;

typedef struct {
  long long int num_value;
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
  Node* inner;
} cast_expr;

typedef struct {
  Node* return_val;
} return_stmt;

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
  Node* ident;
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
    return_stmt returnStmt;
    comment_stmt commentStmt;
    // types
    func_param funcParam;
    var_t variable_t;
    func_type function_t;
  };
};

// makes a identifer expression node
// @param token - the token representing the name of the expression
// @return - the identifer expression node
Node* mk_identifer_expr(Token* token);

// creates a new function type 
// @param ret - the return expr of the func
// @param type - the type of the function param
// @param ident - the identifer expression represent the name of the function
// @return - the new function type node
Node* mk_func_t(Node* ret, ArrayList* params, Node* ident);

// creates a new function paramerters
// @param ident - identifer for this function paramerters
// @param type - the type of the function parameter
// @return - the new function param node
Node* mk_func_param(Node* ident, Node* type);

// creates a new variable type node
// @param is_adr - determines if this is an address var, or not
// @param type_adr - if this is an address type, this will hold the adr type
// @param type - if thsi is NOT and address type, this will hold the var type
Node* mk_var_type(bool is_adr, lit_adr_t type_adr, lit_t type);

// creates a new return expression
// @param return_val - the expression that dictates the return expression value
// @return - the newly created return expression
Node* mk_return_stmt(Node* return_val); 

Node* mk_comment_stmt(char* comment);

// makes a new cast expressions
// @param type - the type of the cast
// @param inner - the nodes being casted
// @return - the newly created cast expression
Node* mk_cast_expr(Node* type, Node* inner);

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
Node* mk_binary_expr(binary_expr_t op, Node* expr_left, Node* expr_right);

// makes a new unary expression based upon an operator and an expression
// @param op - the operator on the unary expression
// @param expr - the expression to apply the unary operator tokens
// @return - the newly created unary operator
Node* mk_unary_expr(unary_expr_t op, Node* expr);

// creates a new literal expression, with either a num_value or str val
// @param num_value - the number value of the literal expression
// @param str_value - the string value of the literal expression
// @return - the newly created literal expression
Node* mk_literal_expr(const char* num_value, const char* str_value);

// creates a new identifer expressions
// @param type - the type of the function
// @param block - the block of ast nodes within the function
// @return - the newly created identifer expression
Node* mk_func_decl(Node* type, Node* block);

// makes a variable declereation
// @param ident - the identifer for the variable
// @param type - the type of the param (var_t)
// @param assign - the exprssion this variable should be assigned to
// @return - the newly created variable decleration node
Node* mk_var_decl(Node* ident, Node* type, Node* assign);

typedef struct {
  ArrayList* items;
  Token* cur_tok;
  unsigned long long size;
  unsigned long long idx;
} Parser;

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
Node* parse_identifier(Parser* parser);

// finds and parsers all stmt
// @param parser - the parser to parse from
// @return - the parsed node
Node* parse_statment(Parser* parser);

// parses an equality statment
// @param parser - the parser to parse from
// @return - the parsed node
Node* parse_equal(Parser* parser);

// find and parses all statments
// @param parser - the parser to parse from
// @return - the parsed node
Node* parse_expr(Parser* parser);

// parses a literal expression
// @param parser - the parser to parse from
// @return - the parsed node
Node* parse_literal_expr(Parser* parser);

// parses primary expression (literals, identifers)
// @param parser - the parser to parse from
// @return - the parsed node
Node* parse_primary(Parser* parser);

// parses a unary expression 
// @param parser - the parser to parse from
// @return - the parsed node
Node* parse_unary_expr(Parser* parser);

// parses a factor (* or /) of a binary expression
// @param parser - the parser to parse from
// @return - the parsed node
Node* parse_factor(Parser* parser);

// parses a term (+ or -), of a binary expression
// @param parser - the parser to parse from
// @return - the parsed node
Node* parse_term(Parser* parser);

// parses a comparison expression (>, <)
// @param parser - the parser to parse from
// @return - the parsed node
Node* parse_compare(Parser* parser);

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
Node* parse_return_stmt(Parser* parser);

// parses a comment stmt
// @param parser - the parser to parse from
// @return - returns a comment node
Node* parse_comment_stmt(Parser* parser);

// parses a list of params for a function call
// @param parser - the parser to parse from
// @return - the array of arguments to the function call
ArrayList* parse_args(Parser* parser);

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

// parses the entire list of function params
// @param parser - the parser to parse from
// @return - an array of function params
ArrayList* parse_all_func_params(Parser* parser);

// parses the type of the function
// @param parser - the parser to parse from
// @return - the parsed node
Node* parse_func_type(Parser* parser);

// parses a function decleration
// @param parser - the parser to parse from
// @return - the node the function creates
Node* parse_func_decl(Parser* parser);

// main parser function, parses a list of nodes into AST 
// @param nodes - the list of nodes to parse
// @return - the head of the ast
Node* parse_program(ArrayList* nodes);

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

// checks if a given token is a type token
// @param token - the token to check 
// @return - true if the tokens is a var type token false otherwise
bool is_var_type(Token* token);

// peeks at the incoming token of the parser
// @param parser - the parser to peek into
// @return - the incoming token
Token* p_peek(Parser* parser);

// peeks at the next incoming token
// @param parser - the parser the parse from
// @return - the next peeked node
Token* p_peek_next(Parser* parser);

// checks if a given token matches a given type
// @param token - the token to match token
// @param type - the type of token to match to
// @return - true if the token matches the type, false otherwise
bool p_match(Token* token, Token_type type);

// prints out the AST
// @param nodes - the head node of the ast
void print_ast(Node* head);

#endif
