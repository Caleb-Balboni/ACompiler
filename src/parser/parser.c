#include <stdlib.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include "errors/errors.h"
#include "utils/arraylist.h"
#include "parser/parser.h"
#include "tokenizer/tokens.h"

static bool block_has_ret(Node* block);
static bool stmt_has_ret(Node* block);

Node* mk_func_t(Node* ret, ArrayList* params, Node* ident) {
  Node* n = malloc(sizeof(Node));
  func_type ft = { .ident = ident, .ret_t = ret, .params = params };
  n->type = AST_TYPE_FUNC;  
  n->function_t = ft;
  return n;
}

Node* mk_func_param(Node* ident, Node* type) {
  Node* n = malloc(sizeof(Node));
  func_param fp = { .ident = ident, .type = type };
  n->type = AST_FUNC_PARAM;
  n->funcParam = fp;
  return n;
}

Node* mk_var_t(bool is_adr, lit_adr_t type_adr, lit_t type) {
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

Node* mk_return_stmt(Node* return_val) {
  Node* n = malloc(sizeof(Node));
  n->type = AST_RETURN;
  return_stmt rs = { .return_val = return_val };
  n->returnStmt = rs;
  return n;
}

Node* mk_comment_stmt(char* comment) {
  Node* n = malloc(sizeof(Node));
  n->type = AST_COMMENT;
  comment_stmt cs = { .comment = comment };
  n->commentStmt = cs;
  return n;
}

Node* mk_cast_expr(Node* type, Node* inner) {
  Node* n = malloc(sizeof(Node));
  n->type = AST_CAST;
  cast_expr ce = { .var_t = type, .inner = inner };
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
  assign_expr ae = { .target = target, .val = val };
  n->assignExpr = ae;
  return n;
}

Node* mk_binary_expr(binary_expr_t op, Node* expr_left, Node* expr_right) {
  Node* n = malloc(sizeof(Node));
  n->type = AST_BINARY;
  binary_expr be = { .op = op, .expr_left = expr_left, .expr_right = expr_right };
  n->binaryExpr = be;
  return n;
}

Node* mk_unary_expr(unary_expr_t op, Node* expr) {
  Node* n = malloc(sizeof(Node));
  n->type = AST_UNARY;
  unary_expr ue = { .op = op, .expr = expr };
  n->unaryExpr = ue;
  return n;
} 

Node* mk_literal_expr(const char* num_value, const char* str_value) {
  Node* n = malloc(sizeof(Node));
  n->type = AST_LITERAL;
  literal_expr le; 
  if (!num_value) {
    le.str_value = str_value;
    le.num_value = INT_MIN;
  } else {
    le.num_value = strtoll(num_value, NULL, 10);
    le.str_value = NULL;
  }
  n->literalExpr = le;
  return n;
} 

Node* mk_identifer_expr(Token* ident) {
  Token_type type = ident->type;
assert(type == T_IDENTIFIER);
  Node* n = malloc(sizeof(Node));
  n->type = AST_IDENTIFIER;
  identifier_expr ie = { .name = ident->lexeme };
  n->identifierExpr = ie;
  return n;
}

Node* mk_if_stmt(Node* cond, Node* then_branch, Node* else_branch) {
  Node* n = malloc(sizeof(Node));
  n->type = AST_IF;
  if_stmt is;
  is.cond = cond;
  is.then_branch = then_branch;
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
  func_decl fd = { .type = type, .block = block }; 
  n->funcDecl = fd;
  return n;
}

Node* mk_var_decl(Node* ident, Node* type, Node* assign) {
  Node* n = malloc(sizeof(Node));
  n->type = AST_VAR_DECL;
  var_decl vd = { .ident = ident, .type = type, .assign = assign };
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
    Token* temp = p_advance(parser);
    temp = p_peek(parser);
    variable.is_adr = true;
    switch(temp->type) {
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
        compile_error(t, "& requires a type");
    }
  } else {
    variable.is_adr = false;
    switch(t->type) {
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
        n->variable_t = variable;
        return n;
    }
  }
  p_advance(parser);
  n->variable_t = variable;
  return n;
}

Node* parse_return_stmt(Parser* parser) {
  Token* temp = p_peek(parser);
  if (!p_match(temp, T_RETURN)) {
    compile_error(temp, "expected a return stmt");
  }
  p_advance(parser);
  Node* ret_val = parse_assign_expr(parser);
  return mk_return_stmt(ret_val);
}

Node* parse_if_stmt(Parser* parser) {
  assert(p_match(p_peek(parser), T_IF));
  p_advance(parser);
  if (!p_match(p_peek(parser), T_LEFT_PAREN)) {
    compile_error(p_peek(parser), "if statments require an opening paren");
  }
  p_advance(parser);
  Node* cond = parse_binary_expr(parser);
  if (!p_match(p_peek(parser), T_RIGHT_PAREN)) {
    compile_error(p_peek(parser), "if statments require a closing paren");
  }
  p_advance(parser);
  Node* then_branch = parse_block_stmt(parser);
  if (p_match(p_peek(parser), T_ELSE)) {
    p_advance(parser);
    Node* else_branch = parse_block_stmt(parser);
    return mk_if_stmt(cond, then_branch, else_branch);
  }
  return mk_if_stmt(cond, then_branch, NULL);
}

Node* parse_literal_expr(Parser* parser) {
  Token* temp = p_peek(parser);
  if (!p_match(temp, T_STRING_LIT) && !p_match(temp, T_NUMBER_LIT)) {
    compile_error(temp, "expected a literal expression");
  }
  p_advance(parser);
  if (p_match(temp, T_STRING_LIT)) {
    return mk_literal_expr(NULL, temp->lexeme);
  }
  if (p_match(temp, T_NUMBER_LIT)) {
    return mk_literal_expr(temp->lexeme, NULL);
  }
}

Node* parse_identifier_expr(Parser* parser) {
  Token* temp = p_peek(parser);
  if (!p_match(temp, T_IDENTIFIER)) {
    compile_error(temp, "expected an identifer");
  }
  Node* ident = mk_identifer_expr(p_peek(parser));
  p_advance(parser);
  return ident;
}

Node* parse_cast_expr(Parser* parser) {
  Token* temp = p_peek(parser);
  assert(temp->type == T_LEFT_PAREN);
  p_advance(parser);
  Node* type = parse_var_type(parser);
  temp = p_peek(parser);
  assert(temp->type == T_RIGHT_PAREN);
  p_advance(parser);
  Node* inner = parse_primary(parser);
  return mk_cast_expr(type, inner);
}

ArrayList* parse_args(Parser* parser) {
  ArrayList* args = init_list(10);
  Token* temp = p_peek(parser);
  if (!p_match(temp, T_LEFT_PAREN)) {
    compile_error(temp, "expected a left paren after arguments for a function");
  }
  p_advance(parser);
  temp = p_peek(parser);
  while (!p_match(temp, T_RIGHT_PAREN)) {
    Node* arg = parse_binary_expr(parser);
    add_list(args, arg);
    temp = p_peek(parser);
    if (temp && p_match(temp, T_COMMA)) {
      p_advance(parser);
      temp = p_peek(parser);
      continue;
    }
    break;
  }
  if (!p_match(temp, T_RIGHT_PAREN)) {
    compile_error(temp, "expected a right paren after arguments for a function");
  }
  p_advance(parser);
  return args;
}

Node* parse_call_expr(Parser* parser) {
  Token* temp = p_peek(parser);
  Node* callee = NULL;
  if (p_match(temp, T_CALL)) {
    p_advance(parser);
    temp = p_peek(parser);
    if (!p_match(temp, T_IDENTIFIER)) {
      compile_error(temp, "expected an identifer after the call expression");
    }
    callee = parse_identifier_expr(parser);
    temp = p_peek(parser);
    if (!p_match(temp, T_LEFT_PAREN)) {
      compile_error(temp, "expected a left paren before calling arguments");
    }
    ArrayList* args = parse_args(parser);
    return mk_call_expr(callee, args);
  }
  return NULL;
}

Node* parse_primary(Parser* parser) {
  Token* temp = p_peek(parser);
  if (!temp) { compile_error(temp, "expected an expression"); }
  if (p_match(temp, T_LEFT_PAREN) && is_var_type(p_peek_next(parser))) {
    Node* cast = parse_cast_expr(parser);
    return cast;
  }
  if (p_match(temp, T_CALL)) {
    return parse_call_expr(parser);
  }
  if (p_match(temp, T_IDENTIFIER)) {
    return parse_identifier_expr(parser);
  }
  if (p_match(temp, T_NUMBER_LIT) || p_match(temp, T_STRING_LIT)) {
    return parse_literal_expr(parser);
  }
  if (p_match(temp, T_LEFT_PAREN)) {
    p_advance(parser);
    Node* inner = parse_assign_expr(parser);
    if (!p_match(p_peek(parser), T_RIGHT_PAREN)) {
      compile_error(p_peek(parser), "expected a closing ) in a () expression");
    }
    p_advance(parser);
    return inner;
  }
  compile_error(temp, "expected a primary expression");
  return NULL;
}

Node* parse_unary_expr(Parser* parser) {
  Token* temp = p_peek(parser);
  if (p_match(temp, T_PLUS)) {
    p_advance(parser);
    return mk_unary_expr(U_POS, parse_unary_expr(parser));
  }
  if (p_match(temp, T_PLUS_PLUS)) {
    p_advance(parser);
    return mk_unary_expr(U_PLUS_PLUS, parse_unary_expr(parser));
  }
  if (p_match(temp, T_MINUS)) {
    p_advance(parser);
    return mk_unary_expr(U_NEG, parse_unary_expr(parser));
  }
  if (p_match(temp, T_MINUS_MINUS)) {
    p_advance(parser);
    return mk_unary_expr(U_MINUS_MINUS, parse_unary_expr(parser));
  }
  if (p_match(temp, T_NOT)) {
    p_advance(parser);
    return mk_unary_expr(U_NOT, parse_unary_expr(parser));
  }
  if (p_match(temp, T_AND)) {
    p_advance(parser);
    return mk_unary_expr(U_ADDR, parse_unary_expr(parser));
  }
  return parse_primary(parser);
}

Node* parse_factor(Parser* parser) {
  Node* left = parse_unary_expr(parser);
  Token* temp = p_peek(parser);
  while (true) {
    temp = p_peek(parser);
    if (p_match(temp, T_STAR)) {
      p_advance(parser);
      left = mk_binary_expr(B_MUL, left, parse_unary_expr(parser));
      continue;
    }
    if (p_match(temp, T_DIVIDE)) {
      p_advance(parser);
      left = mk_binary_expr(B_DIV, left, parse_unary_expr(parser));
      continue;
    }
    break;
  }
  return left;
}

Node* parse_term(Parser* parser) {
  Node* left = parse_factor(parser);
  Token* temp = p_peek(parser);
  while (true) {
    temp = p_peek(parser);
    if (p_match(temp, T_PLUS)) {
      p_advance(parser);
      left = mk_binary_expr(B_ADD, left, parse_factor(parser));
      continue;
    }
    if (p_match(temp, T_MINUS)) {
      p_advance(parser);
      left = mk_binary_expr(B_SUB, left, parse_factor(parser));
      continue;
    }
    break;
  }
  return left;
}

Node* parse_compare(Parser* parser) {
  Node* left = parse_term(parser);
  Token* temp = p_peek(parser);
  while (true) {
    temp = p_peek(parser);
    if (p_match(temp, T_GREATER)) {
      p_advance(parser);
      left = mk_binary_expr(B_GREATER, left, parse_term(parser)); 
      continue;
    }
    if (p_match(temp, T_LESS)) {
      p_advance(parser);
      left = mk_binary_expr(B_LESS, left, parse_term(parser));
      continue;
    }
    if (p_match(temp, T_GREATER_EQUAL)) {
      p_advance(parser);
      left = mk_binary_expr(B_GEQ, left, parse_term(parser));
      continue;
    }
    if (p_match(temp, T_LESS_EQUAL)) {
      p_advance(parser);
      left = mk_binary_expr(B_LEQ, left, parse_term(parser));
      continue;
    } 
    break;
  }
  return left;
}

Node* parse_equal(Parser* parser) {
  Node* left = parse_compare(parser);
  Token* temp = p_peek(parser);
  while (true) {
    temp = p_peek(parser);
    if (p_match(temp, T_EQUAL_EQUAL)) {
      p_advance(parser);
      left = mk_binary_expr(B_EQUAL_EQUAL, left, parse_compare(parser));
      continue;
    }
    if (p_match(temp, T_NOT_EQUAL)) {
      p_advance(parser);
      left = mk_binary_expr(B_NOT_EQUAL, left, parse_compare(parser));
      continue;
    }
    break;
  }
  return left;
}

Node* parse_binary_expr(Parser* parser) {
  return parse_equal(parser);
}

Node* parse_assign_expr(Parser* parser) {
  Node* left = parse_binary_expr(parser);
  Token* temp = p_peek(parser);
  if (p_match(temp, T_EQUAL)) {
    p_advance(parser);
    if (!left || left->type != AST_IDENTIFIER) {
      compile_error(p_peek(parser), "left side of = must be assignable");
    } 
    Node* value = parse_assign_expr(parser);
    return mk_assign_expr(left, value);
  }
  return left;
}

Node* parse_expr(Parser* parser) {
  Token* temp = p_peek(parser);
  if (!temp) { std_compile_error("expected an expression"); }
  if (p_match(temp, T_CALL)) {
    return parse_call_expr(parser);
  }
  if (p_match(temp, T_LEFT_BRACE)) {
    return parse_block_stmt(parser);
  }
  return parse_assign_expr(parser);
}

Node* parse_comment_stmt(Parser* parser) {
  Token* temp = p_peek(parser);
  assert(temp->type == T_COMMENT);
  p_advance(parser);
  return mk_comment_stmt(temp->lexeme);
}

Node* parse_statment(Parser* parser) {
  Token* temp = p_peek(parser);
  if (p_match(temp, T_IF)) {
    return parse_if_stmt(parser);
  }
  if (p_match(temp, T_COMMENT)) {
    return parse_comment_stmt(parser);
  }
  if (p_match(temp, T_RETURN)) {
    Node* ret_stmt = parse_return_stmt(parser);
    if (!p_match(p_peek(parser), T_SEMICOLON)) {
      compile_error(p_peek(parser), "expected a semicolon after return stmt");
    }
    p_advance(parser);
    return ret_stmt;
  }
  Node* expr = parse_expr(parser);
  if (!p_match(p_peek(parser), T_SEMICOLON)) {
    compile_error(p_peek(parser), "expected a semicolon after expression");
  }
  p_advance(parser);
  return expr;
  compile_error(temp, "not a valid statment");
}

Node* parse_block_stmt(Parser* parser) {
  assert(p_match(p_peek(parser), T_LEFT_BRACE));
  ArrayList* nodes = init_list(100);
  p_advance(parser);
  Token* t = p_peek(parser);
  while (!p_match(t, T_RIGHT_BRACE)) {
    Node* temp = NULL;
    if (p_match(t, T_LET)) { 
      temp = parse_var_decl(parser); 
    } else if (p_match(t, T_COMMENT)) {
      temp = parse_comment_stmt(parser); 
    } else {
      temp = parse_statment(parser);
    }
    if  (!temp) {
      compile_error(p_peek(parser), "not a valid expression within the block");
    }
    add_list(nodes, temp);
    t = p_peek(parser);
  }
  Node* block = mk_block_stmt(nodes);
  assert(block != NULL);
  p_advance(parser);
  return block;
}

Node* parse_var_decl(Parser* parser) {
  if (p_match(p_peek(parser), T_LET)) {
    p_advance(parser);
    Node* type = parse_var_type(parser);
    assert(type != NULL);
    Node* ident = parse_identifier_expr(parser);
    assert(ident != NULL);
    if (p_match(p_peek(parser), T_EQUAL)) {
      p_advance(parser);
      Node* binexpr = parse_binary_expr(parser);
      if (!p_match(p_peek(parser), T_SEMICOLON)) {
        compile_error(p_peek(parser), "variable decls require a trailing semi");
      } 
      p_advance(parser);
      if (!binexpr) {
        return mk_var_decl(ident, type, NULL);
      }
      return mk_var_decl(ident, type, binexpr);
    } else if (p_match(p_peek(parser), T_SEMICOLON)) {
      p_advance(parser);
      return mk_var_decl(ident, type, NULL);
    } else {
      compile_error(p_peek(parser), "variable decls require a trailing semi");
    }
  } else {
    return NULL;
  }
}

Node* parse_func_param(Parser* parser) {
  Node* type = parse_var_type(parser);
  Node* ident = parse_identifier_expr(parser);
  Node* param = mk_func_param(ident, type);
  Token* temp = p_peek(parser);
  if (!p_match(temp, T_COMMA) && !p_match(temp, T_RIGHT_PAREN)) {
    compile_error(temp, "expected a comma after a function param");
  } else if (p_match(temp, T_COMMA)) {
    p_advance(parser);
  }
  return param;
}

ArrayList* parse_all_func_params(Parser* parser) {
  ArrayList* params = init_list(32);
  if (p_match(p_peek(parser), T_LEFT_PAREN)) {
    p_advance(parser);
    Token* temp = p_peek(parser);
    while (!p_match(temp, T_RIGHT_PAREN)) {
      if (!(is_var_type(temp) || p_match(p_peek(parser), T_IDENTIFIER))) {
        compile_error(temp, "expected a trailing paren after function params");
      }
      Node* p = parse_func_param(parser);
      assert(p != NULL);
      add_list(params, p);
      temp = p_peek(parser);
    }
  } else {
    compile_error(p_peek(parser), "function params require a leading open paren");
  }
  p_advance(parser);
  return params;
}

Node* parse_func_type(Parser* parser) {
  Node* ret = parse_var_type(parser);
  Node* ident;
  if (!p_match(p_peek(parser), T_IDENTIFIER)) {
    compile_error(p_peek(parser), "functions require identifers");
  }
  ident = parse_identifier_expr(parser);
  ArrayList* params = parse_all_func_params(parser);
  Node* func_type = mk_func_t(ret, params, ident);
  return func_type;
}

static bool stmt_has_ret(Node* stmt) {
  if (!stmt) { return false; }
  switch (stmt->type) {
    case AST_RETURN:
      return true;
    case AST_BLOCK:
      return block_has_ret(stmt);
    case AST_IF:
      Node* t_stmt = stmt->ifStmt.then_branch;
      Node* e_stmt = stmt->ifStmt.else_branch;
      if (t_stmt && !e_stmt) {
        return stmt_has_ret(t_stmt);
      } else if (t_stmt && e_stmt) {
        return stmt_has_ret(t_stmt) && stmt_has_ret(e_stmt);
      } 
    default:
      return false;
  }
}

static bool block_has_ret(Node* block) {
  assert(block && block->type == AST_BLOCK);
  ArrayList* nodes = block->blockStmt.nodes;
  for (int i = 0; i < nodes->length; i++) {
    Node* cur = (Node*)get_list(nodes, i);
    if (stmt_has_ret(cur)) {
      return true;
    }
  }
  return false;
}

Node* parse_func_decl(Parser* parser) {
  if (p_match(p_peek(parser), T_FUNC)) {
    p_advance(parser);
    Node* ft = parse_func_type(parser);
    Node* block = parse_block_stmt(parser);
    if (!block_has_ret(block)) {
      std_compile_error("functions require a return stmt");
    } 
    return mk_func_decl(ft, block);
  }
  return NULL;
}

Node* parse_program(ArrayList* nodes) {
  Parser* parser = init_parser(nodes);
  ArrayList* p_nodes = init_list(128);
  Node* temp = NULL;
  while(!p_is_end(parser) && !p_match(p_peek(parser), T_EOF)) {
    if (p_match(p_peek(parser), T_FUNC)) {
      temp = parse_func_decl(parser);
    } else if (p_match(p_peek(parser), T_LET)) {
      temp = parse_var_decl(parser);

    } else if (p_match(p_peek(parser), T_COMMENT)) {
      temp = parse_comment_stmt(parser);
    } else {
      compile_error(p_peek(parser), "only varibles and function can be declared in global scope");
    }
    add_list(p_nodes, temp);
  }
  Node* program = mk_program_decl(p_nodes);
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

Token* p_peek_next(Parser* parser) {
  if (parser->idx + 1 >= parser->size) { return NULL; }
  return (Token*)get_list(parser->items, parser->idx + 1);
}

bool p_match(Token* token, Token_type type) {
  if (!token) { return false; }
  return token->type == type;
}

bool is_var_type(Token* token) {
  if (!token) { return false; }
  Token_type tt = token->type;
  return tt == T_AND || tt == T_BYTE || tt == T_WORD || tt == T_DWORD || tt == T_QWORD;
}

static void print_block_stmt(Node* node, const int depth);
static void print_assign_expr(Node* node, const int depth);
static void print_call_expr(Node* node, const int depth);
static void print_binary_expr(Node* node, const int depth);
static void print_primary_types(Node* node, const int depth);
static void print_comment_stmt(Node* node, const int depth);

static void print_with_indent_arr(const char** to_print, size_t bytes, const int depth) {
  size_t elements = bytes / sizeof(char*);
  const int spacing = 4;
  printf("%*c", spacing * depth, '{');
  for (int i = 0; i < elements; i++) {
    if (!to_print[i]) { continue; }
    printf("%s ", to_print[i]);
  }
  printf("%c", '}');
  printf("%c",'\n');
}

static void print_with_indent_s(const char* to_print, const int depth) {
  const int spacing = 4;
  printf("%*c", spacing * depth, ' ');
  printf("%s", to_print);
  printf("%c", '\n');
}

static const char* get_ident(Node* node) {
  assert(node->type == AST_IDENTIFIER);
  identifier_expr ident = node->identifierExpr;
  return ident.name;
}

static const char* get_var_t(Node* node) {
  assert(node->type == AST_TYPE_VAR);
  var_t type = node->variable_t;
  if (type.is_adr) {
    switch(type.type_adr) {
      case ADR_BYTE:
        return "&BYTE";
      case ADR_WORD:
        return "&WORD";
      case ADR_DWORD:
        return "&DWORD";
      case ADR_QWORD:
        return "&QWORD";
    }
  } else {
    switch(type.type) {
      case LIT_BYTE:
        return "BYTE";
      case LIT_WORD:
        return "WORD";
      case LIT_DWORD:
        return "DWORD";
      case LIT_QWORD:
        return "QWORD";
    }
  }
  return NULL;
}

static void print_var_decl(Node* node, const int depth) {
  assert(node->type == AST_VAR_DECL);
  var_decl var = node->varDecl;
  const char* var_p[4] = { "VAR DECL:", get_ident(var.ident), "TYPE:", get_var_t(var.type) };
  print_with_indent_arr(var_p, sizeof(var_p), depth);
  if (var.assign) {
    print_binary_expr(var.assign, 1 + depth);
  }
}

static const char* get_func_params(ArrayList* params) {
  char* ret = NULL;
  unsigned int format_size = 7; // "param: " 
  unsigned int ident_size = 2;
  unsigned int alloc_size = 0;
  for (int i = 0; i < params->length; i++) {
    Node* temp = (Node*)get_list(params, i);
    func_param param = temp->funcParam;
    const char* ident = get_ident(param.ident);
    const char* vartype = get_var_t(param.type);
    alloc_size += strlen(ident) + strlen(vartype) + format_size + 1 + 2;
    ret = realloc(ret, alloc_size);
    if (i == 0) {
      strcpy(ret, "Param: ");
    } else {
      strcat(ret, "Param: ");
    }
    strcat(ret, vartype);
    strcat(ret, " ");
    strcat(ret, ident);
    strcat(ret, ", ");
  }
  return ret;
}

static void print_cast_expr(Node* node, const int depth) {
  assert(node->type == AST_CAST);
  cast_expr castexpr = node->castExpr;
  const char* type[2] = { "CAST:", get_var_t(castexpr.var_t) };
  print_with_indent_arr(type, sizeof(type), depth);
  print_binary_expr(castexpr.inner, 1 + depth);
}

static void print_primary_types(Node* node, const int depth) {
  switch(node->type) {
    case AST_CAST:
      print_cast_expr(node, depth);
      break;
    case AST_CALL:
      print_call_expr(node, depth);
      break;
    case AST_IDENTIFIER:
      const char* identifier[2] = { "IDENT:", get_ident(node) };
      print_with_indent_arr(identifier, sizeof(identifier), depth);
      break;
    case AST_LITERAL:
      literal_expr lit = node->literalExpr;
      const char* literal[2];
      literal[0] = "LITERAL:";
      if (lit.str_value) {
        literal[1] = lit.str_value;
      } else {
        literal[1] = "number value";
      }
      print_with_indent_arr(literal, sizeof(literal), depth);
      break;
  }
} 

static const char* get_unary_op(Node* node) {
  assert(node->type == AST_UNARY);
  unary_expr ue = node->unaryExpr;
  unary_expr_t op = ue.op;
  switch(op) {
    case U_POS:
      return "+";
    case U_NEG:
      return "-";
    case U_NOT:
      return "!";
    case U_ADDR:
      return "&";
    default:
      std_compile_error("obtained an unusable unary expression");
  }
}

static void print_unary_expr(Node* node, const int depth) {
  if (node->type != AST_UNARY) {
    print_primary_types(node, depth);
    return;
  }
  const char* unarytype[2] = { "UNARY:", get_unary_op(node) };
  print_with_indent_arr(unarytype, sizeof(unarytype), depth);
  unary_expr unaryexpr = node->unaryExpr;
  print_binary_expr(unaryexpr.expr, 1 + depth);
}

static const char* get_binary_op(Node* node) {
  assert(node->type == AST_BINARY);
  binary_expr be = node->binaryExpr;
  binary_expr_t op = be.op;
  switch (op) {
    case B_ADD:
      return "+";
    case B_SUB:
      return "-";
    case B_MUL:
      return "*";
    case B_DIV:
      return "/";
    case B_LESS:
      return "<";
    case B_GREATER:
      return ">";
    case B_EQUAL_EQUAL:
      return "==";
    case B_NOT_EQUAL:
      return "!=";
    case B_GEQ:
      return ">=";
    case B_LEQ:
      return "<=";
    default:
      std_compile_error("obtained an unusable bin expression");
  } 
}

static void print_binary_expr(Node* node, const int depth) {
  if (node->type != AST_BINARY) {
    print_unary_expr(node, depth);
    return;
  }
  binary_expr be = node->binaryExpr;
  const char* bintype[2] = { "BINARY:", get_binary_op(node) };
  print_with_indent_arr(bintype, sizeof(bintype), depth);
  print_binary_expr(be.expr_left, 1 + depth);
  print_binary_expr(be.expr_right, 1 + depth);
}

static void print_call_expr(Node* node, const int depth) {
  call_expr func = node->callExpr;
  const char* func_name[2] = { "FUNC CALL:", get_ident(func.callee) };
  print_with_indent_arr(func_name, sizeof(func_name), depth);
  ArrayList* args = func.args;
  for (int i = 0; i < args->length; i++) {
    Node* arg = (Node*)get_list(args, i);
    print_binary_expr(arg, 1 + depth);
  }
}

static void print_return_stmt(Node* node, const int depth) {
  print_with_indent_s("RETURN:", depth);
  return_stmt returnstmt = node->returnStmt;
  print_binary_expr(returnstmt.return_val, 1 + depth);
}

static void print_assign_expr(Node* node, const int depth) {
  assert(node->type == AST_ASSIGN);
  assign_expr assignexpr = node->assignExpr;
  const char* assigntype[3] = { "ASSIGN:", get_ident(assignexpr.target), "<--" };
  print_with_indent_arr(assigntype, sizeof(assigntype), depth);
  print_binary_expr(assignexpr.val, 1 + depth);
}

static void print_expr(Node* node, const int depth) {
  switch(node->type) {
    case AST_CALL:
      print_call_expr(node, depth);
      break;
    case AST_BLOCK:
      print_block_stmt(node, 1 + depth);
      break;
    case AST_ASSIGN:
      print_assign_expr(node, depth);
    default:
      print_binary_expr(node, depth);
  }
}

static void print_if_stmt(Node* node, const int depth) {
  assert(node->type == AST_IF);
  if_stmt ifStmt = node->ifStmt;
  print_with_indent_s("IFSTMT:", depth);
  print_binary_expr(ifStmt.cond, depth + 1);
  print_block_stmt(ifStmt.then_branch, depth + 1);
  if (ifStmt.else_branch) {
    print_with_indent_s("ELSESTMT:", depth);
    print_block_stmt(ifStmt.else_branch, depth + 1);
  }
}

static void print_stmt(Node* node, const int depth) {
  switch(node->type) {
    case AST_IF:
      print_if_stmt(node, 1 + depth);
      break;
    case AST_RETURN:
      print_return_stmt(node, 1 + depth);
      break;
    case AST_COMMENT:
      print_comment_stmt(node, 1 + depth);
    default:
      print_expr(node, 1 + depth); 
  }
}

static void print_block_stmt(Node* node, const int depth) {
  assert(node->type == AST_BLOCK);
  ArrayList* nodes = node->blockStmt.nodes;
  print_with_indent_s("BlOCK:", depth);
  for (int i = 0; i < nodes->length; i++) {
    Node* temp = (Node*)get_list(nodes, i);
    switch(temp->type) {
      case AST_VAR_DECL:
        print_var_decl(temp, 1 + depth);
        break;
      default:
        print_stmt(temp, depth);
    }
  }
}

static void print_comment_stmt(Node* node, const int depth) {
  assert(node->type == AST_COMMENT);
  comment_stmt commentstmt = node->commentStmt;
  const char* comment[2] = { "COMMENT:", commentstmt.comment };
  print_with_indent_arr(comment, sizeof(comment), depth);
}

static void print_func_type(Node* node, const int depth) {
  assert(node->type == AST_TYPE_FUNC);
  func_type f_type = node->function_t;
  const char* func_sig[4] = { "FUNCTION:", get_ident(f_type.ident), "RETURN TYPE:", get_var_t(f_type.ret_t) };
  print_with_indent_arr(func_sig, sizeof(func_sig), depth);
  const char* func_param[2] = { "PARAMS:", get_func_params(f_type.params) };
  print_with_indent_arr(func_param, sizeof(func_param), depth);
}

static void print_func_decl(Node* node, const int depth) {
  assert(node->type == AST_FUNC_DECL);
  func_decl header = node->funcDecl;
  print_func_type(header.type, depth);
  print_block_stmt(header.block, 1 + depth);
}

void print_ast(Node* head) {
  assert(head->type == AST_PROGRAM);
  program_decl program = head->programDecl;
  ArrayList* nodes = program.nodes;
  for (int i = 0; i < nodes->length; i++) {
    Node* node = (Node*)get_list(nodes, i);
    switch(node->type) {
      case AST_VAR_DECL:
        print_var_decl(node, 0);
        break;
      case AST_FUNC_DECL:
        print_func_decl(node, 0);
        break;
      case AST_COMMENT:
        
    }
  }
}
