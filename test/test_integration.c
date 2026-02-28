#include <stdio.h>
#include <string.h>
#include <criterion/criterion.h>
#include "tokenizer/tokenizer.h"
#include "tokenizer/tokens.h"
#include "parser/parser.h"
#include "utils/arraylist.h"

// Helper: tokenize a string
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

// Helper: tokenize a file
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
// Integration: Full Pipeline (source string -> tokens -> AST)
// ============================================================

Test(integration, minimal_program) {
  const char* src = "fn DWORD main () {\n  return 0;\n}\n";
  ArrayList* tokens = tokenize_string(src);
  cr_assert(tokens->length > 0);
  Node* ast = parse_program(tokens);
  cr_assert(ast != NULL);
  cr_assert(ast->type == AST_PROGRAM);
  cr_assert(ast->programDecl.nodes->length == 1);
}

Test(integration, program_with_variables) {
  const char* src =
    "fn DWORD main () {\n"
    "  let DWORD x = 10;\n"
    "  let DWORD y = 20;\n"
    "  let DWORD z = x + y;\n"
    "  return 0;\n"
    "}\n";
  ArrayList* tokens = tokenize_string(src);
  Node* ast = parse_program(tokens);
  Node* func = (Node*)get_list(ast->programDecl.nodes, 0);
  Node* block = func->funcDecl.block;

  // 3 var decls + 1 return
  cr_assert(block->blockStmt.nodes->length == 4);

  // Verify z = x + y
  Node* var_z = (Node*)get_list(block->blockStmt.nodes, 2);
  cr_assert(var_z->type == AST_VAR_DECL);
  Node* z_assign = var_z->varDecl.assign;
  cr_assert(z_assign->type == AST_BINARY);
  cr_assert(z_assign->binaryExpr.op == B_ADD);
  cr_assert(z_assign->binaryExpr.expr_left->type == AST_IDENTIFIER);
  cr_assert_str_eq(z_assign->binaryExpr.expr_left->identifierExpr.name, "x");
  cr_assert(z_assign->binaryExpr.expr_right->type == AST_IDENTIFIER);
  cr_assert_str_eq(z_assign->binaryExpr.expr_right->identifierExpr.name, "y");
}

Test(integration, complex_expression) {
  const char* src =
    "fn DWORD main () {\n"
    "  let DWORD r = 1 + 2 * 3 - 4 / 2;\n"
    "  return 0;\n"
    "}\n";
  ArrayList* tokens = tokenize_string(src);
  Node* ast = parse_program(tokens);
  Node* func = (Node*)get_list(ast->programDecl.nodes, 0);
  Node* block = func->funcDecl.block;
  Node* var = (Node*)get_list(block->blockStmt.nodes, 0);
  Node* expr = var->varDecl.assign;

  // 1 + 2 * 3 - 4 / 2  -->  (1 + (2*3)) - (4/2)
  cr_assert(expr->type == AST_BINARY);
  cr_assert(expr->binaryExpr.op == B_SUB);

  // Left: 1 + (2 * 3)
  Node* left = expr->binaryExpr.expr_left;
  cr_assert(left->type == AST_BINARY);
  cr_assert(left->binaryExpr.op == B_ADD);
  cr_assert(left->binaryExpr.expr_left->literalExpr.num_value == 1);
  cr_assert(left->binaryExpr.expr_right->binaryExpr.op == B_MUL);

  // Right: 4 / 2
  Node* right = expr->binaryExpr.expr_right;
  cr_assert(right->type == AST_BINARY);
  cr_assert(right->binaryExpr.op == B_DIV);
}

Test(integration, nested_if_structure) {
  const char* src =
    "fn DWORD main () {\n"
    "  let DWORD x = 5;\n"
    "  if (x > 0) {\n"
    "    if (x < 10) {\n"
    "      return 1;\n"
    "    }\n"
    "    return 2;\n"
    "  }\n"
    "  return 0;\n"
    "}\n";
  ArrayList* tokens = tokenize_string(src);
  Node* ast = parse_program(tokens);
  Node* func = (Node*)get_list(ast->programDecl.nodes, 0);
  Node* block = func->funcDecl.block;

  // let x, if, return 0
  cr_assert(block->blockStmt.nodes->length == 3);

  // Outer if
  Node* outer_if = (Node*)get_list(block->blockStmt.nodes, 1);
  cr_assert(outer_if->type == AST_IF);
  cr_assert(outer_if->ifStmt.cond->binaryExpr.op == B_GREATER);

  // Inner if inside outer if's then-branch
  Node* then_block = outer_if->ifStmt.then_branch;
  cr_assert(then_block->type == AST_BLOCK);
  Node* inner_if = (Node*)get_list(then_block->blockStmt.nodes, 0);
  cr_assert(inner_if->type == AST_IF);
  cr_assert(inner_if->ifStmt.cond->binaryExpr.op == B_LESS);
}

Test(integration, multi_function_program) {
  const char* src =
    "fn DWORD square (DWORD n) {\n"
    "  return n * n;\n"
    "}\n"
    "fn DWORD double (DWORD n) {\n"
    "  return n + n;\n"
    "}\n"
    "fn DWORD main () {\n"
    "  let DWORD a = call square(5);\n"
    "  let DWORD b = call double(a);\n"
    "  return 0;\n"
    "}\n";
  ArrayList* tokens = tokenize_string(src);
  Node* ast = parse_program(tokens);
  cr_assert(ast->programDecl.nodes->length == 3);

  // Verify all three are function declarations
  for (int i = 0; i < 3; i++) {
    Node* f = (Node*)get_list(ast->programDecl.nodes, i);
    cr_assert(f->type == AST_FUNC_DECL);
  }

  // Verify square function
  Node* square = (Node*)get_list(ast->programDecl.nodes, 0);
  cr_assert_str_eq(square->funcDecl.type->function_t.ident->identifierExpr.name, "square");
  cr_assert(square->funcDecl.type->function_t.params->length == 1);

  // Verify main's call to square
  Node* main_func = (Node*)get_list(ast->programDecl.nodes, 2);
  Node* main_block = main_func->funcDecl.block;
  Node* var_a = (Node*)get_list(main_block->blockStmt.nodes, 0);
  cr_assert(var_a->varDecl.assign->type == AST_CALL);
  cr_assert_str_eq(var_a->varDecl.assign->callExpr.callee->identifierExpr.name, "square");
}

Test(integration, global_and_local_vars) {
  const char* src =
    "let DWORD g = 100;\n"
    "fn DWORD main () {\n"
    "  let DWORD l = 200;\n"
    "  return 0;\n"
    "}\n";
  ArrayList* tokens = tokenize_string(src);
  Node* ast = parse_program(tokens);
  cr_assert(ast->programDecl.nodes->length == 2);

  Node* global = (Node*)get_list(ast->programDecl.nodes, 0);
  cr_assert(global->type == AST_VAR_DECL);
  cr_assert(global->varDecl.assign->literalExpr.num_value == 100);

  Node* func = (Node*)get_list(ast->programDecl.nodes, 1);
  cr_assert(func->type == AST_FUNC_DECL);
}

Test(integration, assignment_and_return) {
  const char* src =
    "fn DWORD main () {\n"
    "  let DWORD x = 0;\n"
    "  x = 42;\n"
    "  return x;\n"
    "}\n";
  ArrayList* tokens = tokenize_string(src);
  Node* ast = parse_program(tokens);
  Node* func = (Node*)get_list(ast->programDecl.nodes, 0);
  Node* block = func->funcDecl.block;

  // let, assign, return
  cr_assert(block->blockStmt.nodes->length == 3);

  Node* assign = (Node*)get_list(block->blockStmt.nodes, 1);
  cr_assert(assign->type == AST_ASSIGN);
  cr_assert_str_eq(assign->assignExpr.target->identifierExpr.name, "x");
  cr_assert(assign->assignExpr.val->literalExpr.num_value == 42);

  Node* ret = (Node*)get_list(block->blockStmt.nodes, 2);
  cr_assert(ret->type == AST_RETURN);
  cr_assert(ret->returnStmt.return_val->type == AST_IDENTIFIER);
  cr_assert_str_eq(ret->returnStmt.return_val->identifierExpr.name, "x");
}

Test(integration, comments_preserved) {
  const char* src =
    "// top-level comment\n"
    "fn DWORD main () {\n"
    "  // inside function\n"
    "  return 0;\n"
    "}\n";
  ArrayList* tokens = tokenize_string(src);
  Node* ast = parse_program(tokens);

  // comment + func
  cr_assert(ast->programDecl.nodes->length == 2);
  Node* comment = (Node*)get_list(ast->programDecl.nodes, 0);
  cr_assert(comment->type == AST_COMMENT);

  Node* func = (Node*)get_list(ast->programDecl.nodes, 1);
  Node* block = func->funcDecl.block;
  Node* inner_comment = (Node*)get_list(block->blockStmt.nodes, 0);
  cr_assert(inner_comment->type == AST_COMMENT);
}

Test(integration, pointer_operations) {
  const char* src =
    "fn DWORD main () {\n"
    "  let DWORD x = 42;\n"
    "  let &DWORD p = &x;\n"
    "  return 0;\n"
    "}\n";
  ArrayList* tokens = tokenize_string(src);
  Node* ast = parse_program(tokens);
  Node* func = (Node*)get_list(ast->programDecl.nodes, 0);
  Node* block = func->funcDecl.block;

  // let x, let p, return
  cr_assert(block->blockStmt.nodes->length == 3);

  Node* ptr = (Node*)get_list(block->blockStmt.nodes, 1);
  cr_assert(ptr->varDecl.type->variable_t.is_adr == true);
  cr_assert(ptr->varDecl.type->variable_t.type_adr == ADR_DWORD);
  cr_assert(ptr->varDecl.assign->type == AST_UNARY);
  cr_assert(ptr->varDecl.assign->unaryExpr.op == U_ADDR);
}

Test(integration, cast_in_expression) {
  const char* src =
    "fn QWORD main () {\n"
    "  let DWORD x = 5;\n"
    "  let QWORD y = (QWORD)x + 1;\n"
    "  return 0;\n"
    "}\n";
  ArrayList* tokens = tokenize_string(src);
  Node* ast = parse_program(tokens);
  Node* func = (Node*)get_list(ast->programDecl.nodes, 0);
  Node* block = func->funcDecl.block;

  Node* var_y = (Node*)get_list(block->blockStmt.nodes, 1);
  Node* expr = var_y->varDecl.assign;
  // (QWORD)x + 1 should be ADD(CAST(x), 1)
  cr_assert(expr->type == AST_BINARY);
  cr_assert(expr->binaryExpr.op == B_ADD);
  cr_assert(expr->binaryExpr.expr_left->type == AST_CAST);
  cr_assert(expr->binaryExpr.expr_right->literalExpr.num_value == 1);
}

Test(integration, fib_file_full_parse) {
  ArrayList* tokens = tokenize_file("../fib.av");
  cr_assert(tokens != NULL);
  cr_assert(tokens->length > 10);  // sanity: nontrivial number of tokens

  Node* ast = parse_program(tokens);
  cr_assert(ast != NULL);
  cr_assert(ast->type == AST_PROGRAM);

  // Find function declarations
  int func_count = 0;
  int comment_count = 0;
  for (int i = 0; i < (int)ast->programDecl.nodes->length; i++) {
    Node* n = (Node*)get_list(ast->programDecl.nodes, i);
    if (n->type == AST_FUNC_DECL) func_count++;
    if (n->type == AST_COMMENT) comment_count++;
  }
  cr_assert(func_count == 2, "Expected 2 functions (fib + main), got %d", func_count);
  cr_assert(comment_count >= 2, "Expected at least 2 top-level comments, got %d", comment_count);

  // Find fib function and verify it has an if-else
  for (int i = 0; i < (int)ast->programDecl.nodes->length; i++) {
    Node* n = (Node*)get_list(ast->programDecl.nodes, i);
    if (n->type == AST_FUNC_DECL) {
      const char* name = n->funcDecl.type->function_t.ident->identifierExpr.name;
      if (strcmp(name, "fib") == 0) {
        // fib should have a parameter
        cr_assert(n->funcDecl.type->function_t.params->length == 1);
        Node* param = (Node*)get_list(n->funcDecl.type->function_t.params, 0);
        cr_assert_str_eq(param->funcParam.ident->identifierExpr.name, "n");
        break;
      }
    }
  }
}

Test(integration, equality_and_comparison_mixed) {
  const char* src =
    "fn DWORD main () {\n"
    "  let DWORD x = 1 + 2 == 3;\n"
    "  return 0;\n"
    "}\n";
  ArrayList* tokens = tokenize_string(src);
  Node* ast = parse_program(tokens);
  Node* func = (Node*)get_list(ast->programDecl.nodes, 0);
  Node* block = func->funcDecl.block;
  Node* var = (Node*)get_list(block->blockStmt.nodes, 0);
  Node* expr = var->varDecl.assign;

  // 1 + 2 == 3 should be EQUAL_EQUAL(ADD(1,2), 3)
  cr_assert(expr->type == AST_BINARY);
  cr_assert(expr->binaryExpr.op == B_EQUAL_EQUAL);
  cr_assert(expr->binaryExpr.expr_left->binaryExpr.op == B_ADD);
  cr_assert(expr->binaryExpr.expr_right->literalExpr.num_value == 3);
}

Test(integration, call_with_complex_args) {
  const char* src =
    "fn DWORD foo (DWORD a, DWORD b) {\n"
    "  return a + b;\n"
    "}\n"
    "fn DWORD main () {\n"
    "  let DWORD r = call foo(1 + 2, 3 * 4);\n"
    "  return 0;\n"
    "}\n";
  ArrayList* tokens = tokenize_string(src);
  Node* ast = parse_program(tokens);
  Node* main_func = (Node*)get_list(ast->programDecl.nodes, 1);
  Node* block = main_func->funcDecl.block;
  Node* var = (Node*)get_list(block->blockStmt.nodes, 0);

  Node* call = var->varDecl.assign;
  cr_assert(call->type == AST_CALL);
  cr_assert(call->callExpr.args->length == 2);

  // First arg: 1 + 2
  Node* arg0 = (Node*)get_list(call->callExpr.args, 0);
  cr_assert(arg0->type == AST_BINARY);
  cr_assert(arg0->binaryExpr.op == B_ADD);

  // Second arg: 3 * 4
  Node* arg1 = (Node*)get_list(call->callExpr.args, 1);
  cr_assert(arg1->type == AST_BINARY);
  cr_assert(arg1->binaryExpr.op == B_MUL);
}

Test(integration, tokenize_count_consistency) {
  // Verify tokenizer produces consistent results
  const char* src = "fn DWORD main () { return 0; }";
  ArrayList* t1 = tokenize_string(src);
  ArrayList* t2 = tokenize_string(src);
  cr_assert(t1->length == t2->length);

  for (int i = 0; i < (int)t1->length; i++) {
    Token* tok1 = (Token*)get_list(t1, i);
    Token* tok2 = (Token*)get_list(t2, i);
    cr_assert(tok1->type == tok2->type, "Token %d type mismatch", i);
  }
}
