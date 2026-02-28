#include <stdio.h>
#include <string.h>
#include <criterion/criterion.h>
#include "tokenizer/tokenizer.h"
#include "tokenizer/tokens.h"
#include "parser/parser.h"
#include "utils/arraylist.h"

// Forward declarations for functions defined in parser.c but not in the header
extern Node* mk_var_t(bool is_adr, lit_adr_t type_adr, lit_t type);
extern Node* mk_block_stmt(ArrayList* nodes);
extern Node* mk_if_stmt(Node* cond, Node* then_branch, Node* else_branch);

// Helper: tokenize a string by writing it to a temp file
static ArrayList* tokenize_string(const char* src) {
  FILE* f = tmpfile();
  cr_assert(f != NULL);
  fwrite(src, 1, strlen(src), f);
  rewind(f);
  unsigned long len = strlen(src);
  ArrayList* tokens = tokenize(f, len);
  fclose(f);
  return tokens;
}

// Helper: tokenize a file by path
static ArrayList* tokenize_file(const char* path) {
  FILE* f = fopen(path, "r");
  cr_assert(f != NULL, "Could not open file: %s", path);
  fseek(f, 0, SEEK_END);
  unsigned long len = ftell(f);
  rewind(f);
  ArrayList* tokens = tokenize(f, len);
  fclose(f);
  return tokens;
}

// ============================================================
// AST Node Constructor Tests (mk_* functions)
// ============================================================

Test(parser_nodes, mk_literal_expr_number) {
  Node* n = mk_literal_expr("42", NULL);
  cr_assert(n != NULL);
  cr_assert(n->type == AST_LITERAL);
  cr_assert(n->literalExpr.num_value == 42);
  cr_assert(n->literalExpr.str_value == NULL);
  free(n);
}

Test(parser_nodes, mk_literal_expr_string) {
  Node* n = mk_literal_expr(NULL, "hello");
  cr_assert(n != NULL);
  cr_assert(n->type == AST_LITERAL);
  cr_assert_str_eq(n->literalExpr.str_value, "hello");
  free(n);
}

Test(parser_nodes, mk_literal_expr_negative_number) {
  Node* n = mk_literal_expr("-100", NULL);
  cr_assert(n != NULL);
  cr_assert(n->type == AST_LITERAL);
  cr_assert(n->literalExpr.num_value == -100);
  free(n);
}

Test(parser_nodes, mk_literal_expr_zero) {
  Node* n = mk_literal_expr("0", NULL);
  cr_assert(n != NULL);
  cr_assert(n->type == AST_LITERAL);
  cr_assert(n->literalExpr.num_value == 0);
  free(n);
}

Test(parser_nodes, mk_binary_expr_add) {
  Node* left = mk_literal_expr("1", NULL);
  Node* right = mk_literal_expr("2", NULL);
  Node* n = mk_binary_expr(B_ADD, left, right);
  cr_assert(n != NULL);
  cr_assert(n->type == AST_BINARY);
  cr_assert(n->binaryExpr.op == B_ADD);
  cr_assert(n->binaryExpr.expr_left == left);
  cr_assert(n->binaryExpr.expr_right == right);
  cr_assert(n->binaryExpr.expr_left->literalExpr.num_value == 1);
  cr_assert(n->binaryExpr.expr_right->literalExpr.num_value == 2);
  free(left);
  free(right);
  free(n);
}

Test(parser_nodes, mk_binary_expr_all_ops) {
  binary_expr_t ops[] = {B_ADD, B_SUB, B_MUL, B_DIV, B_LESS, B_GREATER,
                         B_EQUAL_EQUAL, B_NOT_EQUAL, B_GEQ, B_LEQ};
  Node* left = mk_literal_expr("1", NULL);
  Node* right = mk_literal_expr("2", NULL);
  for (int i = 0; i < 10; i++) {
    Node* n = mk_binary_expr(ops[i], left, right);
    cr_assert(n->type == AST_BINARY);
    cr_assert(n->binaryExpr.op == ops[i]);
    free(n);
  }
  free(left);
  free(right);
}

Test(parser_nodes, mk_unary_expr_all_ops) {
  unary_expr_t ops[] = {U_POS, U_PLUS_PLUS, U_MINUS_MINUS, U_NEG, U_NOT, U_ADDR};
  Node* inner = mk_literal_expr("5", NULL);
  for (int i = 0; i < 6; i++) {
    Node* n = mk_unary_expr(ops[i], inner);
    cr_assert(n->type == AST_UNARY);
    cr_assert(n->unaryExpr.op == ops[i]);
    cr_assert(n->unaryExpr.expr == inner);
    free(n);
  }
  free(inner);
}

Test(parser_nodes, mk_var_type_non_address) {
  lit_t types[] = {LIT_BYTE, LIT_WORD, LIT_DWORD, LIT_QWORD};
  for (int i = 0; i < 4; i++) {
    Node* n = mk_var_t(false, 0, types[i]);
    cr_assert(n->type == AST_TYPE_VAR);
    cr_assert(n->variable_t.is_adr == false);
    cr_assert(n->variable_t.type == types[i]);
    free(n);
  }
}

Test(parser_nodes, mk_var_type_address) {
  lit_adr_t adr_types[] = {ADR_BYTE, ADR_WORD, ADR_DWORD, ADR_QWORD};
  for (int i = 0; i < 4; i++) {
    Node* n = mk_var_t(true, adr_types[i], 0);
    cr_assert(n->type == AST_TYPE_VAR);
    cr_assert(n->variable_t.is_adr == true);
    cr_assert(n->variable_t.type_adr == adr_types[i]);
    free(n);
  }
}

Test(parser_nodes, mk_var_decl_with_assign) {
  Token tok = { .type = T_IDENTIFIER, .lexeme = "x" };
  Node* ident = mk_identifer_expr(&tok);
  Node* type = mk_var_t(false, 0, LIT_DWORD);
  Node* val = mk_literal_expr("42", NULL);
  Node* decl = mk_var_decl(ident, type, val);

  cr_assert(decl->type == AST_VAR_DECL);
  cr_assert(decl->varDecl.ident == ident);
  cr_assert(decl->varDecl.type == type);
  cr_assert(decl->varDecl.assign == val);
  free(ident);
  free(type);
  free(val);
  free(decl);
}

Test(parser_nodes, mk_var_decl_without_assign) {
  Token tok = { .type = T_IDENTIFIER, .lexeme = "y" };
  Node* ident = mk_identifer_expr(&tok);
  Node* type = mk_var_t(false, 0, LIT_QWORD);
  Node* decl = mk_var_decl(ident, type, NULL);

  cr_assert(decl->type == AST_VAR_DECL);
  cr_assert(decl->varDecl.assign == NULL);
  free(ident);
  free(type);
  free(decl);
}

Test(parser_nodes, mk_assign_expr_test) {
  Token tok = { .type = T_IDENTIFIER, .lexeme = "x" };
  Node* target = mk_identifer_expr(&tok);
  Node* val = mk_literal_expr("10", NULL);
  Node* n = mk_assign_expr(target, val);

  cr_assert(n->type == AST_ASSIGN);
  cr_assert(n->assignExpr.target == target);
  cr_assert(n->assignExpr.val == val);
  free(target);
  free(val);
  free(n);
}

Test(parser_nodes, mk_call_expr_test) {
  Token tok = { .type = T_IDENTIFIER, .lexeme = "foo" };
  Node* callee = mk_identifer_expr(&tok);
  ArrayList* args = init_list(2);
  add_list(args, mk_literal_expr("1", NULL));
  add_list(args, mk_literal_expr("2", NULL));
  Node* n = mk_call_expr(callee, args);

  cr_assert(n->type == AST_CALL);
  cr_assert(n->callExpr.callee == callee);
  cr_assert(n->callExpr.args->length == 2);
  free(callee);
  free(n);
}

Test(parser_nodes, mk_call_expr_no_args) {
  Token tok = { .type = T_IDENTIFIER, .lexeme = "bar" };
  Node* callee = mk_identifer_expr(&tok);
  ArrayList* args = init_list(2);
  Node* n = mk_call_expr(callee, args);

  cr_assert(n->type == AST_CALL);
  cr_assert(n->callExpr.args->length == 0);
  free(callee);
  free(n);
}

Test(parser_nodes, mk_cast_expr_test) {
  Node* type = mk_var_t(false, 0, LIT_QWORD);
  Node* inner = mk_literal_expr("42", NULL);
  Node* n = mk_cast_expr(type, inner);

  cr_assert(n->type == AST_CAST);
  cr_assert(n->castExpr.var_t == type);
  cr_assert(n->castExpr.inner == inner);
  free(type);
  free(inner);
  free(n);
}

Test(parser_nodes, mk_if_stmt_with_else) {
  Node* cond = mk_literal_expr("1", NULL);
  ArrayList* then_nodes = init_list(1);
  add_list(then_nodes, mk_return_stmt(mk_literal_expr("1", NULL)));
  Node* then_branch = mk_block_stmt(then_nodes);
  ArrayList* else_nodes = init_list(1);
  add_list(else_nodes, mk_return_stmt(mk_literal_expr("0", NULL)));
  Node* else_branch = mk_block_stmt(else_nodes);

  Node* n = mk_if_stmt(cond, then_branch, else_branch);
  cr_assert(n->type == AST_IF);
  cr_assert(n->ifStmt.cond == cond);
  cr_assert(n->ifStmt.then_branch == then_branch);
  cr_assert(n->ifStmt.else_branch == else_branch);
  free(n);
}

Test(parser_nodes, mk_if_stmt_without_else) {
  Node* cond = mk_literal_expr("1", NULL);
  ArrayList* then_nodes = init_list(1);
  add_list(then_nodes, mk_return_stmt(mk_literal_expr("1", NULL)));
  Node* then_branch = mk_block_stmt(then_nodes);

  Node* n = mk_if_stmt(cond, then_branch, NULL);
  cr_assert(n->type == AST_IF);
  cr_assert(n->ifStmt.else_branch == NULL);
  free(n);
}

Test(parser_nodes, mk_return_stmt_test) {
  Node* val = mk_literal_expr("0", NULL);
  Node* n = mk_return_stmt(val);
  cr_assert(n->type == AST_RETURN);
  cr_assert(n->returnStmt.return_val == val);
  free(val);
  free(n);
}

Test(parser_nodes, mk_comment_stmt_test) {
  char comment[] = "this is a comment";
  Node* n = mk_comment_stmt(comment);
  cr_assert(n->type == AST_COMMENT);
  cr_assert_str_eq(n->commentStmt.comment, "this is a comment");
  free(n);
}

Test(parser_nodes, mk_block_stmt_test) {
  ArrayList* nodes = init_list(3);
  add_list(nodes, mk_return_stmt(mk_literal_expr("0", NULL)));
  Node* n = mk_block_stmt(nodes);
  cr_assert(n->type == AST_BLOCK);
  cr_assert(n->blockStmt.nodes == nodes);
  cr_assert(n->blockStmt.nodes->length == 1);
  free(n);
}

Test(parser_nodes, mk_func_decl_test) {
  Token tok = { .type = T_IDENTIFIER, .lexeme = "main" };
  Node* ident = mk_identifer_expr(&tok);
  Node* ret = mk_var_t(false, 0, LIT_DWORD);
  ArrayList* params = init_list(1);
  Node* ft = mk_func_t(ret, params, ident);

  ArrayList* body_nodes = init_list(1);
  add_list(body_nodes, mk_return_stmt(mk_literal_expr("0", NULL)));
  Node* block = mk_block_stmt(body_nodes);

  Node* n = mk_func_decl(ft, block);
  cr_assert(n->type == AST_FUNC_DECL);
  cr_assert(n->funcDecl.type == ft);
  cr_assert(n->funcDecl.block == block);
  free(n);
}

Test(parser_nodes, mk_func_t_test) {
  Token tok = { .type = T_IDENTIFIER, .lexeme = "add" };
  Node* ident = mk_identifer_expr(&tok);
  Node* ret = mk_var_t(false, 0, LIT_DWORD);
  ArrayList* params = init_list(2);

  Token p1_tok = { .type = T_IDENTIFIER, .lexeme = "a" };
  Token p2_tok = { .type = T_IDENTIFIER, .lexeme = "b" };
  add_list(params, mk_func_param(mk_identifer_expr(&p1_tok), mk_var_t(false, 0, LIT_DWORD)));
  add_list(params, mk_func_param(mk_identifer_expr(&p2_tok), mk_var_t(false, 0, LIT_DWORD)));

  Node* n = mk_func_t(ret, params, ident);
  cr_assert(n->type == AST_TYPE_FUNC);
  cr_assert(n->function_t.ident == ident);
  cr_assert(n->function_t.ret_t == ret);
  cr_assert(n->function_t.params->length == 2);
  free(n);
}

Test(parser_nodes, mk_func_param_test) {
  Token tok = { .type = T_IDENTIFIER, .lexeme = "n" };
  Node* ident = mk_identifer_expr(&tok);
  Node* type = mk_var_t(false, 0, LIT_DWORD);
  Node* n = mk_func_param(ident, type);

  cr_assert(n->type == AST_FUNC_PARAM);
  cr_assert(n->funcParam.ident == ident);
  cr_assert(n->funcParam.type == type);
  free(ident);
  free(type);
  free(n);
}

Test(parser_nodes, mk_program_decl_test) {
  ArrayList* nodes = init_list(2);
  Node* n = mk_program_decl(nodes);
  cr_assert(n->type == AST_PROGRAM);
  cr_assert(n->programDecl.nodes == nodes);
  cr_assert(n->programDecl.nodes->length == 0);
  free(n);
}

Test(parser_nodes, mk_identifer_expr_test) {
  Token tok = { .type = T_IDENTIFIER, .lexeme = "myVar" };
  Node* n = mk_identifer_expr(&tok);
  cr_assert(n->type == AST_IDENTIFIER);
  cr_assert_str_eq(n->identifierExpr.name, "myVar");
  free(n);
}

// ============================================================
// Parser Utility Function Tests
// ============================================================

Test(parser_utils, init_parser_test) {
  ArrayList* tokens = init_list(10);
  Token* t = malloc(sizeof(Token));
  t->type = T_EOF;
  add_list(tokens, t);
  Parser* p = init_parser(tokens);
  cr_assert(p != NULL);
  cr_assert(p->items == tokens);
  cr_assert(p->size == 1);
  cr_assert(p->idx == 0);
  free(p);
  destroy_list(tokens);
}

Test(parser_utils, p_is_end_false) {
  ArrayList* tokens = init_list(2);
  Token* t = malloc(sizeof(Token));
  t->type = T_EOF;
  add_list(tokens, t);
  Parser* p = init_parser(tokens);
  cr_assert(p_is_end(p) == false);
  free(p);
  destroy_list(tokens);
}

Test(parser_utils, p_is_end_true) {
  ArrayList* tokens = init_list(2);
  Token* t = malloc(sizeof(Token));
  t->type = T_EOF;
  add_list(tokens, t);
  Parser* p = init_parser(tokens);
  p_advance(p);
  cr_assert(p_is_end(p) == true);
  free(p);
  destroy_list(tokens);
}

Test(parser_utils, p_advance_test) {
  ArrayList* tokens = init_list(3);
  Token* t1 = malloc(sizeof(Token));
  t1->type = T_LET;
  strcpy(t1->lexeme, "let");
  Token* t2 = malloc(sizeof(Token));
  t2->type = T_SEMICOLON;
  strcpy(t2->lexeme, ";");
  Token* t3 = malloc(sizeof(Token));
  t3->type = T_EOF;
  strcpy(t3->lexeme, "eof");
  add_list(tokens, t1);
  add_list(tokens, t2);
  add_list(tokens, t3);
  Parser* p = init_parser(tokens);

  Token* advanced = p_advance(p);
  cr_assert(advanced->type == T_LET);
  cr_assert(p->idx == 1);

  advanced = p_advance(p);
  cr_assert(advanced->type == T_SEMICOLON);
  cr_assert(p->idx == 2);

  advanced = p_advance(p);
  cr_assert(advanced->type == T_EOF);
  cr_assert(p->idx == 3);

  // Past end
  advanced = p_advance(p);
  cr_assert(advanced == NULL);

  free(p);
  destroy_list(tokens);
}

Test(parser_utils, p_peek_test) {
  ArrayList* tokens = init_list(2);
  Token* t1 = malloc(sizeof(Token));
  t1->type = T_IDENTIFIER;
  strcpy(t1->lexeme, "x");
  Token* t2 = malloc(sizeof(Token));
  t2->type = T_EOF;
  add_list(tokens, t1);
  add_list(tokens, t2);
  Parser* p = init_parser(tokens);

  Token* peeked = p_peek(p);
  cr_assert(peeked->type == T_IDENTIFIER);
  // peek shouldn't advance
  cr_assert(p->idx == 0);

  p_advance(p);
  peeked = p_peek(p);
  cr_assert(peeked->type == T_EOF);

  free(p);
  destroy_list(tokens);
}

Test(parser_utils, p_peek_next_test) {
  ArrayList* tokens = init_list(3);
  Token* t1 = malloc(sizeof(Token));
  t1->type = T_LET;
  Token* t2 = malloc(sizeof(Token));
  t2->type = T_DWORD;
  Token* t3 = malloc(sizeof(Token));
  t3->type = T_EOF;
  add_list(tokens, t1);
  add_list(tokens, t2);
  add_list(tokens, t3);
  Parser* p = init_parser(tokens);

  Token* next = p_peek_next(p);
  cr_assert(next->type == T_DWORD);
  // idx shouldn't change
  cr_assert(p->idx == 0);

  free(p);
  destroy_list(tokens);
}

Test(parser_utils, p_peek_next_at_end) {
  ArrayList* tokens = init_list(1);
  Token* t1 = malloc(sizeof(Token));
  t1->type = T_EOF;
  add_list(tokens, t1);
  Parser* p = init_parser(tokens);

  Token* next = p_peek_next(p);
  cr_assert(next == NULL);

  free(p);
  destroy_list(tokens);
}

Test(parser_utils, p_match_true) {
  Token t = { .type = T_IDENTIFIER };
  cr_assert(p_match(&t, T_IDENTIFIER) == true);
}

Test(parser_utils, p_match_false) {
  Token t = { .type = T_IDENTIFIER };
  cr_assert(p_match(&t, T_LET) == false);
}

Test(parser_utils, p_match_null) {
  cr_assert(p_match(NULL, T_IDENTIFIER) == false);
}

Test(parser_utils, is_var_type_true) {
  Token_type var_types[] = {T_BYTE, T_WORD, T_DWORD, T_QWORD, T_AND};
  for (int i = 0; i < 5; i++) {
    Token t = { .type = var_types[i] };
    cr_assert(is_var_type(&t) == true, "Expected %d to be a var type", var_types[i]);
  }
}

Test(parser_utils, is_var_type_false) {
  Token_type non_types[] = {T_IDENTIFIER, T_LET, T_IF, T_ELSE, T_FUNC,
                            T_SEMICOLON, T_LEFT_PAREN, T_NUMBER_LIT};
  for (int i = 0; i < 8; i++) {
    Token t = { .type = non_types[i] };
    cr_assert(is_var_type(&t) == false, "Expected %d to not be a var type", non_types[i]);
  }
}

Test(parser_utils, is_var_type_null) {
  cr_assert(is_var_type(NULL) == false);
}

// ============================================================
// Parser: Parsing Full Programs from Source Strings
// ============================================================

Test(parser_parse, simple_func) {
  ArrayList* tokens = tokenize_file("../test/testprograms/simple_func.av");
  cr_assert(tokens != NULL);
  Node* program = parse_program(tokens);
  cr_assert(program != NULL);
  cr_assert(program->type == AST_PROGRAM);
  cr_assert(program->programDecl.nodes->length == 1);

  Node* func = (Node*)get_list(program->programDecl.nodes, 0);
  cr_assert(func->type == AST_FUNC_DECL);

  // Check function type
  Node* ft = func->funcDecl.type;
  cr_assert(ft->type == AST_TYPE_FUNC);
  cr_assert_str_eq(ft->function_t.ident->identifierExpr.name, "main");
  cr_assert(ft->function_t.ret_t->variable_t.type == LIT_DWORD);
  cr_assert(ft->function_t.params->length == 0);

  // Check block has return
  Node* block = func->funcDecl.block;
  cr_assert(block->type == AST_BLOCK);
  cr_assert(block->blockStmt.nodes->length == 1);

  Node* ret = (Node*)get_list(block->blockStmt.nodes, 0);
  cr_assert(ret->type == AST_RETURN);
  cr_assert(ret->returnStmt.return_val->type == AST_LITERAL);
  cr_assert(ret->returnStmt.return_val->literalExpr.num_value == 0);
}

Test(parser_parse, var_decl_all_types) {
  ArrayList* tokens = tokenize_file("../test/testprograms/var_decl_types.av");
  Node* program = parse_program(tokens);
  cr_assert(program->type == AST_PROGRAM);

  Node* func = (Node*)get_list(program->programDecl.nodes, 0);
  Node* block = func->funcDecl.block;
  // 6 var decls + 1 return = 7 statements
  cr_assert(block->blockStmt.nodes->length == 7);

  // let BYTE a = 1;
  Node* var_a = (Node*)get_list(block->blockStmt.nodes, 0);
  cr_assert(var_a->type == AST_VAR_DECL);
  cr_assert_str_eq(var_a->varDecl.ident->identifierExpr.name, "a");
  cr_assert(var_a->varDecl.type->variable_t.is_adr == false);
  cr_assert(var_a->varDecl.type->variable_t.type == LIT_BYTE);
  cr_assert(var_a->varDecl.assign->literalExpr.num_value == 1);

  // let WORD b = 2;
  Node* var_b = (Node*)get_list(block->blockStmt.nodes, 1);
  cr_assert(var_b->varDecl.type->variable_t.type == LIT_WORD);
  cr_assert(var_b->varDecl.assign->literalExpr.num_value == 2);

  // let DWORD c = 3;
  Node* var_c = (Node*)get_list(block->blockStmt.nodes, 2);
  cr_assert(var_c->varDecl.type->variable_t.type == LIT_DWORD);
  cr_assert(var_c->varDecl.assign->literalExpr.num_value == 3);

  // let QWORD d = 4;
  Node* var_d = (Node*)get_list(block->blockStmt.nodes, 3);
  cr_assert(var_d->varDecl.type->variable_t.type == LIT_QWORD);
  cr_assert(var_d->varDecl.assign->literalExpr.num_value == 4);

  // let &DWORD e = &c;
  Node* var_e = (Node*)get_list(block->blockStmt.nodes, 4);
  cr_assert(var_e->varDecl.type->variable_t.is_adr == true);
  cr_assert(var_e->varDecl.type->variable_t.type_adr == ADR_DWORD);
  // The assignment is a unary addr expression
  cr_assert(var_e->varDecl.assign->type == AST_UNARY);
  cr_assert(var_e->varDecl.assign->unaryExpr.op == U_ADDR);

  // let x = 99; (implicit QWORD)
  Node* var_x = (Node*)get_list(block->blockStmt.nodes, 5);
  cr_assert(var_x->varDecl.type->variable_t.type == LIT_QWORD);
  cr_assert(var_x->varDecl.assign->literalExpr.num_value == 99);
}

Test(parser_parse, binary_expr_precedence) {
  // 1 + 2 * 3  should parse as  1 + (2 * 3)
  ArrayList* tokens = tokenize_file("../test/testprograms/binary_expr.av");
  Node* program = parse_program(tokens);
  Node* func = (Node*)get_list(program->programDecl.nodes, 0);
  Node* block = func->funcDecl.block;
  Node* var = (Node*)get_list(block->blockStmt.nodes, 0);
  cr_assert(var->type == AST_VAR_DECL);
  Node* expr = var->varDecl.assign;

  // Top level should be ADD
  cr_assert(expr->type == AST_BINARY);
  cr_assert(expr->binaryExpr.op == B_ADD);

  // Left should be literal 1
  cr_assert(expr->binaryExpr.expr_left->type == AST_LITERAL);
  cr_assert(expr->binaryExpr.expr_left->literalExpr.num_value == 1);

  // Right should be MUL(2, 3)
  Node* right = expr->binaryExpr.expr_right;
  cr_assert(right->type == AST_BINARY);
  cr_assert(right->binaryExpr.op == B_MUL);
  cr_assert(right->binaryExpr.expr_left->literalExpr.num_value == 2);
  cr_assert(right->binaryExpr.expr_right->literalExpr.num_value == 3);
}

Test(parser_parse, if_else_stmt) {
  ArrayList* tokens = tokenize_file("../test/testprograms/if_else.av");
  Node* program = parse_program(tokens);
  Node* func = (Node*)get_list(program->programDecl.nodes, 0);
  Node* block = func->funcDecl.block;

  // let DWORD x = 5; then if/else
  cr_assert(block->blockStmt.nodes->length == 2);

  Node* if_node = (Node*)get_list(block->blockStmt.nodes, 1);
  cr_assert(if_node->type == AST_IF);

  // Condition: x > 3
  Node* cond = if_node->ifStmt.cond;
  cr_assert(cond->type == AST_BINARY);
  cr_assert(cond->binaryExpr.op == B_GREATER);
  cr_assert(cond->binaryExpr.expr_left->type == AST_IDENTIFIER);
  cr_assert(cond->binaryExpr.expr_right->literalExpr.num_value == 3);

  // Then branch
  Node* then_b = if_node->ifStmt.then_branch;
  cr_assert(then_b->type == AST_BLOCK);
  cr_assert(then_b->blockStmt.nodes->length == 1);
  Node* then_ret = (Node*)get_list(then_b->blockStmt.nodes, 0);
  cr_assert(then_ret->type == AST_RETURN);

  // Else branch
  Node* else_b = if_node->ifStmt.else_branch;
  cr_assert(else_b != NULL);
  cr_assert(else_b->type == AST_BLOCK);
}

Test(parser_parse, func_with_params_and_call) {
  ArrayList* tokens = tokenize_file("../test/testprograms/func_call.av");
  Node* program = parse_program(tokens);
  cr_assert(program->programDecl.nodes->length == 2);

  // First function: fn DWORD add (DWORD a, DWORD b)
  Node* add_func = (Node*)get_list(program->programDecl.nodes, 0);
  cr_assert(add_func->type == AST_FUNC_DECL);
  Node* add_type = add_func->funcDecl.type;
  cr_assert_str_eq(add_type->function_t.ident->identifierExpr.name, "add");
  cr_assert(add_type->function_t.params->length == 2);

  // Check params
  Node* param_a = (Node*)get_list(add_type->function_t.params, 0);
  cr_assert(param_a->type == AST_FUNC_PARAM);
  cr_assert_str_eq(param_a->funcParam.ident->identifierExpr.name, "a");
  cr_assert(param_a->funcParam.type->variable_t.type == LIT_DWORD);

  Node* param_b = (Node*)get_list(add_type->function_t.params, 1);
  cr_assert_str_eq(param_b->funcParam.ident->identifierExpr.name, "b");

  // Check body: return a + b;
  Node* add_block = add_func->funcDecl.block;
  Node* ret_stmt = (Node*)get_list(add_block->blockStmt.nodes, 0);
  cr_assert(ret_stmt->type == AST_RETURN);
  cr_assert(ret_stmt->returnStmt.return_val->type == AST_BINARY);
  cr_assert(ret_stmt->returnStmt.return_val->binaryExpr.op == B_ADD);

  // Second function: main with call expression
  Node* main_func = (Node*)get_list(program->programDecl.nodes, 1);
  Node* main_block = main_func->funcDecl.block;
  Node* var_x = (Node*)get_list(main_block->blockStmt.nodes, 0);
  cr_assert(var_x->type == AST_VAR_DECL);
  // The assignment should be a call expression
  cr_assert(var_x->varDecl.assign->type == AST_CALL);
  cr_assert_str_eq(var_x->varDecl.assign->callExpr.callee->identifierExpr.name, "add");
  cr_assert(var_x->varDecl.assign->callExpr.args->length == 2);
}

Test(parser_parse, unary_expressions) {
  ArrayList* tokens = tokenize_file("../test/testprograms/unary_expr.av");
  Node* program = parse_program(tokens);
  Node* func = (Node*)get_list(program->programDecl.nodes, 0);
  Node* block = func->funcDecl.block;

  // let DWORD x = -5;
  Node* var_x = (Node*)get_list(block->blockStmt.nodes, 0);
  cr_assert(var_x->varDecl.assign->type == AST_UNARY);
  cr_assert(var_x->varDecl.assign->unaryExpr.op == U_NEG);
  cr_assert(var_x->varDecl.assign->unaryExpr.expr->literalExpr.num_value == 5);

  // let DWORD y = !0;
  Node* var_y = (Node*)get_list(block->blockStmt.nodes, 1);
  cr_assert(var_y->varDecl.assign->type == AST_UNARY);
  cr_assert(var_y->varDecl.assign->unaryExpr.op == U_NOT);
}

Test(parser_parse, assign_expression) {
  ArrayList* tokens = tokenize_file("../test/testprograms/assign_expr.av");
  Node* program = parse_program(tokens);
  Node* func = (Node*)get_list(program->programDecl.nodes, 0);
  Node* block = func->funcDecl.block;

  // x = 42; is the second statement
  Node* assign = (Node*)get_list(block->blockStmt.nodes, 1);
  cr_assert(assign->type == AST_ASSIGN);
  cr_assert_str_eq(assign->assignExpr.target->identifierExpr.name, "x");
  cr_assert(assign->assignExpr.val->literalExpr.num_value == 42);
}

Test(parser_parse, cast_expression) {
  ArrayList* tokens = tokenize_file("../test/testprograms/cast_expr.av");
  Node* program = parse_program(tokens);
  Node* func = (Node*)get_list(program->programDecl.nodes, 0);
  Node* block = func->funcDecl.block;

  // let QWORD y = (QWORD)x;
  Node* var_y = (Node*)get_list(block->blockStmt.nodes, 1);
  cr_assert(var_y->type == AST_VAR_DECL);
  Node* cast = var_y->varDecl.assign;
  cr_assert(cast->type == AST_CAST);
  cr_assert(cast->castExpr.var_t->variable_t.type == LIT_QWORD);
  cr_assert(cast->castExpr.inner->type == AST_IDENTIFIER);
  cr_assert_str_eq(cast->castExpr.inner->identifierExpr.name, "x");
}

Test(parser_parse, comparison_operators) {
  ArrayList* tokens = tokenize_file("../test/testprograms/comparison.av");
  Node* program = parse_program(tokens);
  Node* func = (Node*)get_list(program->programDecl.nodes, 0);
  Node* block = func->funcDecl.block;

  // Verify each comparison operator is correctly parsed
  binary_expr_t expected_ops[] = {B_LESS, B_GREATER, B_LEQ, B_GEQ, B_EQUAL_EQUAL, B_NOT_EQUAL};
  for (int i = 0; i < 6; i++) {
    Node* var = (Node*)get_list(block->blockStmt.nodes, i);
    cr_assert(var->type == AST_VAR_DECL);
    cr_assert(var->varDecl.assign->type == AST_BINARY);
    cr_assert(var->varDecl.assign->binaryExpr.op == expected_ops[i],
              "Expected op %d at index %d, got %d",
              expected_ops[i], i, var->varDecl.assign->binaryExpr.op);
  }
}

Test(parser_parse, global_var_decl) {
  ArrayList* tokens = tokenize_file("../test/testprograms/global_var.av");
  Node* program = parse_program(tokens);
  cr_assert(program->programDecl.nodes->length == 2);

  Node* global_var = (Node*)get_list(program->programDecl.nodes, 0);
  cr_assert(global_var->type == AST_VAR_DECL);
  cr_assert_str_eq(global_var->varDecl.ident->identifierExpr.name, "x");
  cr_assert(global_var->varDecl.assign->literalExpr.num_value == 42);

  Node* func = (Node*)get_list(program->programDecl.nodes, 1);
  cr_assert(func->type == AST_FUNC_DECL);
}

Test(parser_parse, var_decl_no_init) {
  const char* src = "fn DWORD main () {\n  let DWORD x;\n  return 0;\n}\n";
  ArrayList* tokens = tokenize_string(src);
  Node* program = parse_program(tokens);
  Node* func = (Node*)get_list(program->programDecl.nodes, 0);
  Node* block = func->funcDecl.block;

  Node* var = (Node*)get_list(block->blockStmt.nodes, 0);
  cr_assert(var->type == AST_VAR_DECL);
  cr_assert_str_eq(var->varDecl.ident->identifierExpr.name, "x");
  cr_assert(var->varDecl.assign == NULL);
}

Test(parser_parse, nested_binary_left_assoc) {
  // 1 - 2 - 3 should parse as (1 - 2) - 3 (left-associative)
  const char* src = "fn DWORD main () {\n  let DWORD x = 1 - 2 - 3;\n  return 0;\n}\n";
  ArrayList* tokens = tokenize_string(src);
  Node* program = parse_program(tokens);
  Node* func = (Node*)get_list(program->programDecl.nodes, 0);
  Node* block = func->funcDecl.block;
  Node* var = (Node*)get_list(block->blockStmt.nodes, 0);
  Node* expr = var->varDecl.assign;

  // Top: (1 - 2) - 3
  cr_assert(expr->type == AST_BINARY);
  cr_assert(expr->binaryExpr.op == B_SUB);
  cr_assert(expr->binaryExpr.expr_right->literalExpr.num_value == 3);

  // Left: 1 - 2
  Node* left = expr->binaryExpr.expr_left;
  cr_assert(left->type == AST_BINARY);
  cr_assert(left->binaryExpr.op == B_SUB);
  cr_assert(left->binaryExpr.expr_left->literalExpr.num_value == 1);
  cr_assert(left->binaryExpr.expr_right->literalExpr.num_value == 2);
}

Test(parser_parse, mul_div_left_assoc) {
  // 6 / 2 * 3 should parse as (6 / 2) * 3
  const char* src = "fn DWORD main () {\n  let DWORD x = 6 / 2 * 3;\n  return 0;\n}\n";
  ArrayList* tokens = tokenize_string(src);
  Node* program = parse_program(tokens);
  Node* func = (Node*)get_list(program->programDecl.nodes, 0);
  Node* block = func->funcDecl.block;
  Node* var = (Node*)get_list(block->blockStmt.nodes, 0);
  Node* expr = var->varDecl.assign;

  cr_assert(expr->binaryExpr.op == B_MUL);
  cr_assert(expr->binaryExpr.expr_right->literalExpr.num_value == 3);
  cr_assert(expr->binaryExpr.expr_left->binaryExpr.op == B_DIV);
}

Test(parser_parse, comment_in_function) {
  const char* src = "fn DWORD main () {\n  // hello world\n  return 0;\n}\n";
  ArrayList* tokens = tokenize_string(src);
  Node* program = parse_program(tokens);
  Node* func = (Node*)get_list(program->programDecl.nodes, 0);
  Node* block = func->funcDecl.block;

  cr_assert(block->blockStmt.nodes->length == 2);
  Node* comment = (Node*)get_list(block->blockStmt.nodes, 0);
  cr_assert(comment->type == AST_COMMENT);
}

Test(parser_parse, fib_program) {
  // Parse the full fibonacci example
  ArrayList* tokens = tokenize_file("../fib.av");
  Node* program = parse_program(tokens);
  cr_assert(program->type == AST_PROGRAM);

  // Should have: comment, fib func, comment, main func
  int count = program->programDecl.nodes->length;
  cr_assert(count >= 2, "Expected at least 2 top-level nodes, got %d", count);

  // Find the function declarations
  int func_count = 0;
  for (int i = 0; i < count; i++) {
    Node* n = (Node*)get_list(program->programDecl.nodes, i);
    if (n->type == AST_FUNC_DECL) func_count++;
  }
  cr_assert(func_count == 2, "Expected 2 function declarations, got %d", func_count);
}

Test(parser_parse, if_without_else) {
  const char* src = "fn DWORD main () {\n  if (1 > 0) {\n    return 1;\n  }\n  return 0;\n}\n";
  ArrayList* tokens = tokenize_string(src);
  Node* program = parse_program(tokens);
  Node* func = (Node*)get_list(program->programDecl.nodes, 0);
  Node* block = func->funcDecl.block;

  Node* if_node = (Node*)get_list(block->blockStmt.nodes, 0);
  cr_assert(if_node->type == AST_IF);
  cr_assert(if_node->ifStmt.else_branch == NULL);
  cr_assert(if_node->ifStmt.then_branch != NULL);
}

Test(parser_parse, call_no_args) {
  const char* src = "fn DWORD foo () {\n  return 1;\n}\nfn DWORD main () {\n  let DWORD x = call foo();\n  return 0;\n}\n";
  ArrayList* tokens = tokenize_string(src);
  Node* program = parse_program(tokens);
  Node* main_func = (Node*)get_list(program->programDecl.nodes, 1);
  Node* block = main_func->funcDecl.block;
  Node* var = (Node*)get_list(block->blockStmt.nodes, 0);

  cr_assert(var->varDecl.assign->type == AST_CALL);
  cr_assert(var->varDecl.assign->callExpr.args->length == 0);
}

Test(parser_parse, nested_call_in_binary) {
  const char* src = "fn DWORD id (DWORD x) {\n  return x;\n}\nfn DWORD main () {\n  let DWORD r = call id(1) + call id(2);\n  return 0;\n}\n";
  ArrayList* tokens = tokenize_string(src);
  Node* program = parse_program(tokens);
  Node* main_func = (Node*)get_list(program->programDecl.nodes, 1);
  Node* block = main_func->funcDecl.block;
  Node* var = (Node*)get_list(block->blockStmt.nodes, 0);

  Node* expr = var->varDecl.assign;
  cr_assert(expr->type == AST_BINARY);
  cr_assert(expr->binaryExpr.op == B_ADD);
  cr_assert(expr->binaryExpr.expr_left->type == AST_CALL);
  cr_assert(expr->binaryExpr.expr_right->type == AST_CALL);
}

Test(parser_parse, multiple_params_func) {
  const char* src = "fn DWORD add3 (DWORD a, DWORD b, DWORD c) {\n  return a + b + c;\n}\nfn DWORD main () {\n  return 0;\n}\n";
  ArrayList* tokens = tokenize_string(src);
  Node* program = parse_program(tokens);
  Node* func = (Node*)get_list(program->programDecl.nodes, 0);
  Node* ft = func->funcDecl.type;

  cr_assert(ft->function_t.params->length == 3);
  Node* p1 = (Node*)get_list(ft->function_t.params, 0);
  Node* p2 = (Node*)get_list(ft->function_t.params, 1);
  Node* p3 = (Node*)get_list(ft->function_t.params, 2);
  cr_assert_str_eq(p1->funcParam.ident->identifierExpr.name, "a");
  cr_assert_str_eq(p2->funcParam.ident->identifierExpr.name, "b");
  cr_assert_str_eq(p3->funcParam.ident->identifierExpr.name, "c");
}

Test(parser_parse, string_literal) {
  const char* src = "fn DWORD main () {\n  let QWORD s = \"hello\";\n  return 0;\n}\n";
  ArrayList* tokens = tokenize_string(src);
  Node* program = parse_program(tokens);
  Node* func = (Node*)get_list(program->programDecl.nodes, 0);
  Node* block = func->funcDecl.block;
  Node* var = (Node*)get_list(block->blockStmt.nodes, 0);

  cr_assert(var->varDecl.assign->type == AST_LITERAL);
  cr_assert(var->varDecl.assign->literalExpr.str_value != NULL);
  cr_assert_str_eq(var->varDecl.assign->literalExpr.str_value, "hello");
}

Test(parser_parse, address_type_all_variants) {
  const char* src = "fn DWORD main () {\n  let DWORD a = 1;\n  let &BYTE b = &a;\n  let &WORD c = &a;\n  let &DWORD d = &a;\n  let &QWORD e = &a;\n  return 0;\n}\n";
  ArrayList* tokens = tokenize_string(src);
  Node* program = parse_program(tokens);
  Node* func = (Node*)get_list(program->programDecl.nodes, 0);
  Node* block = func->funcDecl.block;

  lit_adr_t expected[] = {ADR_BYTE, ADR_WORD, ADR_DWORD, ADR_QWORD};
  for (int i = 0; i < 4; i++) {
    Node* var = (Node*)get_list(block->blockStmt.nodes, i + 1);
    cr_assert(var->varDecl.type->variable_t.is_adr == true,
              "Expected address type at index %d", i);
    cr_assert(var->varDecl.type->variable_t.type_adr == expected[i],
              "Expected adr type %d at index %d", expected[i], i);
  }
}
