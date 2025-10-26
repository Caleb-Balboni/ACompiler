#ifndef PARSER_H
#define PARSER_H

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <assert.h>
#include "include/arraylist.h"
#include "include/parser.h"
#include "include/tokens.h"

Node* mk_func_t(Node* ret, ArrayList* params, Node* ident) {
  Node* n = malloc(sizeof(Node));
  func_type ft = { .ident = ident .ret_t = ret .params = params };
  n->type = AST_TYPE_FUNC;  
  n->function_t = ft;
  return n;
}

Node* mk_func_param(Node* ident, Node* type) {
  Node* n = malloc(sizeof(Node));
  func_param = { .ident = ident .type = type };
  n->type = AST_FUNC_PARAM;
  n->function_t = func_param;
  return n;
}

Node* mk_var_t(bool, is_adr, lit_adr_t type_adr, lit_t type) {
  Node* n = malloc(sizeof(Node));
  var_t vt;
  n->type = AST_TYPE_VAR;
  vt.is_adr = is_adr;
  if (is_adr) {
    vt.type_adr = type_adr;
  } else {
    vt.type = type;
  }
  n->variable_t = vt; 
  return n;
}

Node* mk_return_expr(Node* return_val) {
  Node* n = malloc(sizeof(Node));
  n->type = AST_RETURN;
  return_expr = { .return_val = return_val };
  n->callExpr = return_expr;
  return n;
}

Node* mk_cast_expr(Node* type) {
  Node* n = malloc(sizeof(Node));
  n->type = AST_CAST;
  cast_expr ce = { .var_t = type };
  n->castExpr = ce;
  return n;
} 

Node* mk_call_expr(Node* callee, ArrayList* args) {
  Node* n = malloc(sizeof(Node));
  n->type = AST_CALL;
  call_expr ce = { .callee = callee, .args = args };
  n->callExpr = ce;
  return n;
}

Node* mk_assign_expr(Node* target, Node* val) {
  Node* n = malloc(sizeof(Node));
  n->type = AST_ASSIGN;
  assign_expr ae = { .target = target .val = val };
  n->assignExpr = ae;
  return n;
}

Node* mk_binary_expr(binary_expr_t op, Node* expr_left, Node* expr_right) {
  Node* n = malloc(sizeof(Node));
  n->type = AST_BINARY;
  binary_expr be = { .op = op .expr_left = expr_left .expr_right = expr_right };
  n->binaryExpr = be;
  return n;
}

Node* mk_unary_expr(unary_expr_t op, Node* expr) {
  Node* n = malloc(sizeof(Node));
  n->type = AST_UNARY;
  unary_expr ue = { .op = op .expr = expr };
  n->unaryExpr = ue;
  return n;
} 

Node* mk_literal_expr(long num_value, const char* str_value) {
  Node* n = malloc(sizeof(Node));
  n->type = AST_LITERAL;
  literal_Expr le = { .num_value = num_value .str_value = str_value }; 
  n->literalExpr = le;
  return n;
} 

Node* mk_identifer_expr(Token* ident) {
  Token_type type = ident->type;
  assert(type == T_IDENTIFIER);
  Node* n = malloc(sizeof(Node));
  n->type = AST_IDENTIFER;
  identifer_expr ie = { .name = ident->lexeme };
  n->identiferExpr = ie;
  return n;
}

Node* mk_if_stmt(Node* cond, Node* then_branch, Node* else_branch) {
  Node* n = malloc(sizeof(Node));
  n->type = AST_IF;
  if_stmt is;
  is.cond = cond;
  is.then_branch;
  if (else_branch) {
    is.else_branch = else_branch;
  } else {
    is.else_branch = NULL;
  }
  n->ifStmt = is;
  return n;
} 

Node* mk_block_stmt(ArrayList* nodes) {
  Node* n = malloc(sizeof(Node));
  n->type = AST_BLOCK;
  block_stmt bs = { .nodes = nodes };
  n->blockStmt = bs;
  return n;
}

Node* mk_func_decl(Node* type, Node* block) {
  Node* n = malloc(sizeof(Node));
  n->type = AST_FUNC_DECL;
  func_decl fd = { .type = type .block = block }; 
  n->funcDecl = fd;
  return n;
}

Node* mk_var_decl(const char* name, Node* type, Node* assign) {
  Node* n = malloc(sizeof(Node));
  n->type = AST_VAR_DECL;
  var_decl vd = { .name = name .type = type .assign = assign };
  n->varDecl = vd;
  return n;
}

Node* mk_program_decl(ArrayList* nodes) {
  Node* n = malloc(sizeof(Node));
  n->type = AST_PROGRAM;
  program_decl pd = { .nodes = nodes };
  n->programDecl = pd;
  return n;
}

Node* parse_var_type(Parser* parser) {
  Node* n = malloc(sizeof(Node));
  Token* t = p_peek(parser);
  var_t variable;
  n->type = AST_TYPE_VAR;
  if (p_match(t, T_AND)) {
    Token_type type = p_advance(parser);
    variable.is_adr = true;
    switch(type) {
      case T_BYTE:
        variable.type_adr = ADR_BYTE;
        break;
      case T_WORD:
        variable.type_adr = ADR_WORD;
        break;
      case T_DWORD:
        variable.type_adr = ADR_DWORD;
        break;
      case T_QWORD:
        variable.type_adr = ADR_QWORD;
        break;
      default:
        return NULL;
    }
  } else {
    variable.is_adr = false;
    switch(t->type) {
      case T_VOID:
        variable.type = LIT_VOID;
        break;
      case T_BYTE:
        variable.type = LIT_BYTE;
        break;
      case T_WORD:
        variable.type = LIT_WORD;
        break;
      case T_DWORD:
        variable.type = LIT_DWORD;
        break;
      case T_QWORD:
        variable.type = LIT_QWORD;
        break;
      default:
        variable.type = LIT_QWORD;
        break;
    }
  }
  p_advance(parser);
  n->variable_t = variable;
  return n;
}

Node* parse_identifer_expr(Parser* parser) {
  assert(0 && "TODO: this function is unimplemented");
}

Node* parse_literal_expr(Parser* parser) {
  assert(0 && "TODO: this function is unimplemented");
}

Node* parse_unary_expr(Parser* parser) {
  assert(0 && "TODO: this function is unimplemented");
}

Node* parse_binary_expr(Parser* parser) {
  assert(0 && "TODO: this function is unimplemented");
}

Node* parse_cast_expr(Parser* parser) {
  assert(0 && "TODO: this function is unimplemented");
}

Node* parse_assign_expr(Parser* parser) {
  assert(0 && "TODO: this function is unimplemented");
}

Node* parse_return_expr(Parser* parser) {
  assert(0 && "TODO: this function is unimplemented");
}

Node* parse_call_expr(Parser* parser) {
  assert(0 && "TODO: this function is unimplemented");
}

Node* parse_if_stmt(Parser* parser) {
  assert(0 && "TODO: this function is unimplemented");
}

Node* parse_block_stmt(Parser* parser) {
  assert(0 && "TODO: this function is unimplemented");
}

Node* parse_var_decl(Parser* parser) {
  assert(0 && "TODO: this function is unimplemented");
}

Node* parse_func_param(Parser* parser) {
  Node* type = parse_var_t(parser);
  Node* ident = mk_ident_expr(p_peek(parser));
  Node* param = mk_func_param(ident, type);
  return param;
}

ArrayList* parse_all_func_params(Parser* parser) {
  ArrayList* params = init_list(32);
  if (p_match(p_peek(parser), T_LEFT_PAREN) {
    Token* temp = p_peek(parser);
    while ((is_var_type(temp) || p_match(temp, T_IDENTIFIER)) && !p_match(temp, T_RIGHT_PAREN)) {
      Node* p = parse_func_param(parser);
      assert(p != NULL);
      add_list(params, p);
      temp = p_peek(parser);
    }
  } else {
    return NULL;
  }
  return params;
}

Node* parse_func_type(Parser* parser) {
  Node* ret = parse_var_type(parser);
  Token* ident;
  if (p_match(p_peek(parser), T_IDENTIFIER)) {
    ident = mk_identifer_expr(p_peek(parser)) 
  }
  ArrayList* params = parse_all_func_params(parser);
  Node* func_type = mk_func_t(ret, params, ident);
  return func_type;
}

Node* parse_func_decl(Parser* parser) {
  if (p_match(p_peek(parser), T_FUNC)) {
    p_advance(parser);
    Node* ft = parse_func_type(parser);
    Node* block = parse_block(parser):
    return mk_func_decl(ft, block);
  } else {
    return parse_var_decl(parser);
  }
}

Node* parse_program(Parser* parser) {
  ArrayList* nodes = init_list(128);
  while(!p_is_end(parser)) {
    Node* temp = parse_func_decl(parser);
    assert(temp != NULL);
    add_list(nodes, temp);
  }
  Node* program = mk_program_decl(nodes);
  assert(program != NULL);
  return program;
}

Parser* init_parser(ArrayList* tokens) {
  assert(tokens != NULL);
  Parser* p = malloc(sizeof(Parser));
  p->items = tokens;
  if (tokens->length > 0) {
    p->cur_tok = (Token*)get_list(tokens, 0);
  }
  p->size = p->items->length; 
  p->idx = 0;
  return p; 
}

bool p_is_end(Parser* parser) {
  return parser->idx >= parser->size;
}

Token* p_advance(Parser* parser) {
  if (p_is_end(parser)) {
    return NULL;
  } 
  Token* temp = parser->cur_tok;
  parser->cur_tok = get_list(parser->items, ++parser->idx); 
  return temp;
}

Token* p_peek(Parser* parser) {
  if (p_is_end(parser)) {
    return NULL;
  }
  return parser->cur_tok;
}

bool p_match(Token* token, Token_type type) {
  return token->type == type;
}

bool is_var_type(Token* token) {
  Token_type tt = token->type;
  return tt == T_AND || tt == T_BYTE || tt == T_WORD || tt == T_DWORD || tt == T_QWORD;
}

#endif
