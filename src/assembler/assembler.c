#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>
#include "assembler/assembler.h"
#include "errors/errors.h"

static const regid arg_regs[] = { REG_RDI, REG_RSI, REG_RDX, REG_RCX, REG_R8, REG_R9 };

static void asm_emit(asm_ctx* ctx, const char* fmt, ...) {
  va_list args;
  va_start(args, fmt);
  if (ctx->emitter->indent > 0) {
    fprintf(ctx->emitter->file, "%*c", ctx->emitter->indent, ' ');
  }
  vfprintf(ctx->emitter->file, fmt, args);
  fprintf(ctx->emitter->file, "\n");
  va_end(args);
}

static void asm_raw(asm_ctx* ctx, const char* fmt, ...) {
  va_list args;
  va_start(args, fmt);
  vfprintf(ctx->emitter->file, fmt, args);
  fprintf(ctx->emitter->file, "\n");
  va_end(args);
}

static unsigned int type_size(var_t* t) {
  if (t->is_adr) { return 8; }
  switch (t->type) {
    case LIT_BYTE:  return 1;
    case LIT_WORD:  return 2;
    case LIT_DWORD: return 4;
    case LIT_QWORD: return 8;
    default: return 8;
  }
}

static void push_scope(asm_ctx* ctx) {
  scope_t* s = malloc(sizeof(scope_t));
  s->symbols = create_ht(64);
  s->base_offset = ctx->cur_offset;
  push_stack(ctx->scope_stk, s);
}

static void pop_scope(asm_ctx* ctx) {
  scope_t* s = (scope_t*)pop_stack(ctx->scope_stk);
  destroy_ht(s->symbols);
  ctx->cur_offset = s->base_offset;
  free(s);
}

static symbol_t* find_symbol(asm_ctx* ctx, const char* name) {
  stack_node* n = ctx->scope_stk->head;
  while (n) {
    scope_t* s = (scope_t*)n->val;
    symbol_t* sym = (symbol_t*)get_ht(s->symbols, name);
    if (sym) { return sym; }
    n = n->next;
  }
  return NULL;
}

static symbol_t* define_local(asm_ctx* ctx, const char* name, var_t type) {
  unsigned int sz = type_size(&type);
  if (sz < 8) { sz = 8; }
  ctx->cur_offset -= sz;
  symbol_t* sym = malloc(sizeof(symbol_t));
  sym->name = name;
  sym->stack_offset = ctx->cur_offset;
  sym->type = type;
  sym->is_global = false;
  scope_t* s = (scope_t*)peek_stack(ctx->scope_stk);
  add_ht(s->symbols, name, sym);
  return sym;
}

static symbol_t* define_global(asm_ctx* ctx, const char* name, var_t type) {
  symbol_t* sym = malloc(sizeof(symbol_t));
  sym->name = name;
  sym->stack_offset = 0;
  sym->type = type;
  sym->is_global = true;
  scope_t* s = (scope_t*)peek_stack(ctx->scope_stk);
  add_ht(s->symbols, name, sym);
  return sym;
}

static unsigned int new_label(asm_ctx* ctx) {
  return ctx->label_count++;
}

static long count_locals(Node* node) {
  if (!node) { return 0; }
  switch (node->type) {
    case AST_BLOCK: {
      long sum = 0;
      ArrayList* nodes = node->blockStmt.nodes;
      for (int i = 0; i < nodes->length; i++) {
        sum += count_locals((Node*)get_list(nodes, i));
      }
      return sum;
    }
    case AST_VAR_DECL:
      return 8;
    case AST_IF: {
      long t = count_locals(node->ifStmt.then_branch);
      long e = count_locals(node->ifStmt.else_branch);
      return t + e;
    }
    default:
      return 0;
  }
}

static long compute_frame_size(Node* func_decl) {
  long size = count_locals(func_decl->funcDecl.block);
  ArrayList* params = func_decl->funcDecl.type->function_t.params;
  size += 8 * params->length;
  if (size % 16 != 0) { size += 16 - (size % 16); }
  return size;
}

static void push_rax(asm_ctx* ctx) {
  operand_t* rax = mk_register(REG_RAX, SZ_64);
  emit_push(ctx->emitter, rax);
  free(rax);
  ctx->push_depth++;
}

static void pop_into(asm_ctx* ctx, regid id) {
  operand_t* r = mk_register(id, SZ_64);
  emit_pop(ctx->emitter, r);
  free(r);
  ctx->push_depth--;
}

static void load_imm(asm_ctx* ctx, long long val) {
  operand_t* imm = mk_immutable(val);
  operand_t* rax = mk_register(REG_RAX, SZ_64);
  emit_mov(ctx->emitter, imm, rax);
  free(imm); free(rax);
}

static void load_local(asm_ctx* ctx, long offset) {
  operand_t* mem = mk_mem(REG_RBP, SZ_64, offset);
  operand_t* rax = mk_register(REG_RAX, SZ_64);
  emit_mov(ctx->emitter, mem, rax);
  free(mem); free(rax);
}

static void store_local(asm_ctx* ctx, long offset) {
  operand_t* mem = mk_mem(REG_RBP, SZ_64, offset);
  operand_t* rax = mk_register(REG_RAX, SZ_64);
  emit_mov(ctx->emitter, rax, mem);
  free(mem); free(rax);
}

static void gen_expr(asm_ctx* ctx, Node* node);
static void gen_stmt(asm_ctx* ctx, Node* node);
static void gen_block(asm_ctx* ctx, Node* node);

static void gen_literal(asm_ctx* ctx, Node* node) {
  load_imm(ctx, node->literalExpr.num_value);
}

static void gen_identifier(asm_ctx* ctx, Node* node) {
  const char* name = node->identifierExpr.name;
  symbol_t* sym = find_symbol(ctx, name);
  if (!sym) { std_compile_error("undefined identifier"); }
  if (sym->is_global) {
    asm_emit(ctx, "movq %s(%%rip), %%rax", name);
  } else {
    load_local(ctx, sym->stack_offset);
  }
}

static void gen_unary(asm_ctx* ctx, Node* node) {
  unary_expr ue = node->unaryExpr;
  if (ue.op == U_ADDR) {
    if (ue.expr->type != AST_IDENTIFIER) {
      std_compile_error("can only take address of an identifier");
    }
    symbol_t* sym = find_symbol(ctx, ue.expr->identifierExpr.name);
    if (!sym) { std_compile_error("undefined identifier"); }
    if (sym->is_global) {
      asm_emit(ctx, "leaq %s(%%rip), %%rax", sym->name);
    } else {
      asm_emit(ctx, "leaq %ld(%%rbp), %%rax", sym->stack_offset);
    }
    return;
  }
  gen_expr(ctx, ue.expr);
  switch (ue.op) {
    case U_NEG:
      asm_emit(ctx, "negq %%rax");
      break;
    case U_NOT:
      asm_emit(ctx, "cmpq $0, %%rax");
      asm_emit(ctx, "sete %%al");
      asm_emit(ctx, "movzbq %%al, %%rax");
      break;
    case U_POS:
      break;
    default:
      std_compile_error("unsupported unary op");
  }
}

static void gen_binary(asm_ctx* ctx, Node* node) {
  binary_expr be = node->binaryExpr;
  gen_expr(ctx, be.expr_left);
  push_rax(ctx);
  gen_expr(ctx, be.expr_right);
  operand_t* rax = mk_register(REG_RAX, SZ_64);
  operand_t* rcx = mk_register(REG_RCX, SZ_64);
  emit_mov(ctx->emitter, rax, rcx);
  pop_into(ctx, REG_RAX);
  switch (be.op) {
    case B_ADD: emit_add(ctx->emitter, rcx, rax); break;
    case B_SUB: emit_sub(ctx->emitter, rcx, rax); break;
    case B_MUL: emit_imul(ctx->emitter, rcx, rax); break;
    case B_DIV:
      asm_emit(ctx, "cqto");
      emit_idiv(ctx->emitter, rcx, NULL);
      break;
    case B_LESS:
    case B_GREATER:
    case B_EQUAL_EQUAL:
    case B_NOT_EQUAL:
    case B_GEQ:
    case B_LEQ: {
      emit_cmp(ctx->emitter, rcx, rax);
      const char* setcc = "sete";
      switch (be.op) {
        case B_LESS:        setcc = "setl"; break;
        case B_GREATER:     setcc = "setg"; break;
        case B_EQUAL_EQUAL: setcc = "sete"; break;
        case B_NOT_EQUAL:   setcc = "setne"; break;
        case B_GEQ:         setcc = "setge"; break;
        case B_LEQ:         setcc = "setle"; break;
        default: break;
      }
      asm_emit(ctx, "%s %%al", setcc);
      asm_emit(ctx, "movzbq %%al, %%rax");
      break;
    }
    default:
      std_compile_error("unsupported binary op");
  }
  free(rax); free(rcx);
}

static void gen_call(asm_ctx* ctx, Node* node) {
  call_expr ce = node->callExpr;
  ArrayList* args = ce.args;
  int n = args->length;
  if (n > 6) { std_compile_error("more than 6 args not supported"); }
  bool pad = (ctx->push_depth % 2) != 0;
  if (pad) {
    asm_emit(ctx, "subq $8, %%rsp");
    ctx->push_depth++;
  }
  for (int i = 0; i < n; i++) {
    gen_expr(ctx, (Node*)get_list(args, i));
    push_rax(ctx);
  }
  for (int i = n - 1; i >= 0; i--) {
    pop_into(ctx, arg_regs[i]);
  }
  operand_t* lbl = mk_label(ce.callee->identifierExpr.name);
  emit_call(ctx->emitter, lbl);
  free(lbl);
  if (pad) {
    asm_emit(ctx, "addq $8, %%rsp");
    ctx->push_depth--;
  }
}

static void gen_assign(asm_ctx* ctx, Node* node) {
  assign_expr ae = node->assignExpr;
  if (ae.target->type != AST_IDENTIFIER) {
    std_compile_error("only identifier assignment supported");
  }
  gen_expr(ctx, ae.val);
  const char* name = ae.target->identifierExpr.name;
  symbol_t* sym = find_symbol(ctx, name);
  if (!sym) { std_compile_error("undefined identifier in assignment"); }
  if (sym->is_global) {
    asm_emit(ctx, "movq %%rax, %s(%%rip)", name);
  } else {
    store_local(ctx, sym->stack_offset);
  }
}

static void gen_cast(asm_ctx* ctx, Node* node) {
  gen_expr(ctx, node->castExpr.inner);
}

static void gen_expr(asm_ctx* ctx, Node* node) {
  switch (node->type) {
    case AST_LITERAL:    gen_literal(ctx, node); break;
    case AST_IDENTIFIER: gen_identifier(ctx, node); break;
    case AST_UNARY:      gen_unary(ctx, node); break;
    case AST_BINARY:     gen_binary(ctx, node); break;
    case AST_CALL:       gen_call(ctx, node); break;
    case AST_ASSIGN:     gen_assign(ctx, node); break;
    case AST_CAST:       gen_cast(ctx, node); break;
    default:
      std_compile_error("unsupported expression");
  }
}

static void gen_var_decl(asm_ctx* ctx, Node* node) {
  var_decl vd = node->varDecl;
  symbol_t* sym = define_local(ctx, vd.ident->identifierExpr.name, vd.type->variable_t);
  if (vd.assign) {
    gen_expr(ctx, vd.assign);
    store_local(ctx, sym->stack_offset);
  }
}

static void gen_return(asm_ctx* ctx, Node* node) {
  return_stmt rs = node->returnStmt;
  if (rs.return_val) { gen_expr(ctx, rs.return_val); }
  asm_emit(ctx, "jmp %s", ctx->epilogue_label);
}

static void gen_if(asm_ctx* ctx, Node* node) {
  if_stmt is = node->ifStmt;
  unsigned int else_lbl = new_label(ctx);
  unsigned int end_lbl = new_label(ctx);
  gen_expr(ctx, is.cond);
  asm_emit(ctx, "cmpq $0, %%rax");
  asm_emit(ctx, "je .L%u", else_lbl);
  gen_stmt(ctx, is.then_branch);
  asm_emit(ctx, "jmp .L%u", end_lbl);
  asm_raw(ctx, ".L%u:", else_lbl);
  if (is.else_branch) { gen_stmt(ctx, is.else_branch); }
  asm_raw(ctx, ".L%u:", end_lbl);
}

static void gen_block(asm_ctx* ctx, Node* node) {
  push_scope(ctx);
  ArrayList* nodes = node->blockStmt.nodes;
  for (int i = 0; i < nodes->length; i++) {
    gen_stmt(ctx, (Node*)get_list(nodes, i));
  }
  pop_scope(ctx);
}

static void gen_stmt(asm_ctx* ctx, Node* node) {
  switch (node->type) {
    case AST_BLOCK:    gen_block(ctx, node); break;
    case AST_IF:       gen_if(ctx, node); break;
    case AST_RETURN:   gen_return(ctx, node); break;
    case AST_VAR_DECL: gen_var_decl(ctx, node); break;
    case AST_COMMENT:  break;
    default:           gen_expr(ctx, node);
  }
}

void gen_func_decl(asm_ctx* ctx, Node* node) {
  func_decl fd = node->funcDecl;
  func_type ft = fd.type->function_t;
  const char* name = ft.ident->identifierExpr.name;
  long frame = compute_frame_size(node);

  ctx->emitter->indent = 0;
  asm_emit(ctx, ".globl %s", name);
  asm_raw(ctx, "%s:", name);
  ctx->emitter->indent = 4;
  asm_emit(ctx, "pushq %%rbp");
  asm_emit(ctx, "movq %%rsp, %%rbp");
  if (frame > 0) {
    asm_emit(ctx, "subq $%ld, %%rsp", frame);
  }

  unsigned int el = new_label(ctx);
  char buf[32];
  snprintf(buf, sizeof(buf), ".L%u", el);
  ctx->epilogue_label = strdup(buf);
  ctx->cur_offset = 0;
  ctx->push_depth = 0;

  push_scope(ctx);
  ArrayList* params = ft.params;
  for (int i = 0; i < params->length; i++) {
    Node* p = (Node*)get_list(params, i);
    symbol_t* sym = define_local(ctx, p->funcParam.ident->identifierExpr.name, p->funcParam.type->variable_t);
    operand_t* reg = mk_register(arg_regs[i], SZ_64);
    operand_t* mem = mk_mem(REG_RBP, SZ_64, sym->stack_offset);
    emit_mov(ctx->emitter, reg, mem);
    free(reg); free(mem);
  }

  ArrayList* nodes = fd.block->blockStmt.nodes;
  for (int i = 0; i < nodes->length; i++) {
    gen_stmt(ctx, (Node*)get_list(nodes, i));
  }

  asm_raw(ctx, "%s:", ctx->epilogue_label);
  ctx->emitter->indent = 4;
  asm_emit(ctx, "movq %%rbp, %%rsp");
  asm_emit(ctx, "popq %%rbp");
  asm_emit(ctx, "ret");

  pop_scope(ctx);
  free((void*)ctx->epilogue_label);
  ctx->epilogue_label = NULL;
}

static void gen_global(asm_ctx* ctx, Node* node) {
  var_decl vd = node->varDecl;
  const char* name = vd.ident->identifierExpr.name;
  var_t type = vd.type->variable_t;
  define_global(ctx, name, type);
  unsigned int sz = type_size(&type);
  long long val = 0;
  if (vd.assign && vd.assign->type == AST_LITERAL) {
    val = vd.assign->literalExpr.num_value;
  }
  const char* directive = ".quad";
  switch (sz) {
    case 1: directive = ".byte"; break;
    case 2: directive = ".word"; break;
    case 4: directive = ".long"; break;
    case 8: directive = ".quad"; break;
  }
  ctx->emitter->indent = 0;
  asm_raw(ctx, "%s:", name);
  ctx->emitter->indent = 4;
  asm_emit(ctx, "%s %lld", directive, val);
  ctx->emitter->indent = 0;
}

void gen_program(asm_ctx* ctx, Node* program) {
  push_scope(ctx);
  ArrayList* nodes = program->programDecl.nodes;
  bool has_data = false;
  for (int i = 0; i < nodes->length; i++) {
    Node* n = (Node*)get_list(nodes, i);
    if (n->type == AST_VAR_DECL) { has_data = true; break; }
  }
  if (has_data) {
    ctx->emitter->indent = 0;
    asm_emit(ctx, ".data");
    for (int i = 0; i < nodes->length; i++) {
      Node* n = (Node*)get_list(nodes, i);
      if (n->type == AST_VAR_DECL) { gen_global(ctx, n); }
    }
  }
  ctx->emitter->indent = 0;
  asm_emit(ctx, ".text");
  for (int i = 0; i < nodes->length; i++) {
    Node* n = (Node*)get_list(nodes, i);
    if (n->type == AST_FUNC_DECL) { gen_func_decl(ctx, n); }
  }
  pop_scope(ctx);
}

asm_ctx* asm_init(const char* output_file) {
  asm_ctx* ctx = malloc(sizeof(asm_ctx));
  ctx->emitter = emitter_init(output_file);
  ctx->scope_stk = init_stack();
  ctx->cur_offset = 0;
  ctx->label_count = 0;
  ctx->push_depth = 0;
  ctx->epilogue_label = NULL;
  return ctx;
}

asm_ctx* asm_init_file(FILE* file) {
  asm_ctx* ctx = malloc(sizeof(asm_ctx));
  ctx->emitter = emitter_init2(file);
  ctx->scope_stk = init_stack();
  ctx->cur_offset = 0;
  ctx->label_count = 0;
  ctx->push_depth = 0;
  ctx->epilogue_label = NULL;
  return ctx;
}

static void drain_scopes(asm_ctx* ctx) {
  while (!stack_is_empty(ctx->scope_stk)) {
    scope_t* s = (scope_t*)pop_stack(ctx->scope_stk);
    destroy_ht(s->symbols);
    free(s);
  }
  delete_stack(ctx->scope_stk);
}

void asm_free(asm_ctx* ctx) {
  drain_scopes(ctx);
  fclose(ctx->emitter->file);
  free(ctx->emitter);
  free(ctx);
}

void asm_free_keep_file(asm_ctx* ctx) {
  drain_scopes(ctx);
  free(ctx->emitter);
  free(ctx);
}
