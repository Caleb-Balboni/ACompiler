#include <stdio.h>
#include <string.h>
#include <criterion/criterion.h>
#include "tokenizer/tokenizer.h"
#include "tokenizer/tokens.h"
#include "utils/arraylist.h"

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

static Token* get_token(ArrayList* tokens, int idx) {
  return (Token*)get_list(tokens, idx);
}

// ============================================================
// Tokenizer: Keywords
// ============================================================

Test(tokenizer_kw, all_keywords) {
  const char* src = "if else let call fn return BYTE WORD DWORD QWORD";
  ArrayList* tokens = tokenize_string(src);
  Token_type expected[] = {T_IF, T_ELSE, T_LET, T_CALL, T_FUNC, T_RETURN,
                           T_BYTE, T_WORD, T_DWORD, T_QWORD, T_EOF};
  for (int i = 0; i < 11; i++) {
    cr_assert(get_token(tokens, i)->type == expected[i],
              "Token %d: expected %d, got %d", i, expected[i], get_token(tokens, i)->type);
  }
}

Test(tokenizer_kw, identifier_not_keyword) {
  const char* src = "foo bar baz myVar x";
  ArrayList* tokens = tokenize_string(src);
  for (int i = 0; i < 5; i++) {
    cr_assert(get_token(tokens, i)->type == T_IDENTIFIER);
  }
}

Test(tokenizer_kw, identifier_with_digits) {
  const char* src = "var1 abc123";
  ArrayList* tokens = tokenize_string(src);
  cr_assert(get_token(tokens, 0)->type == T_IDENTIFIER);
  cr_assert_str_eq(get_token(tokens, 0)->lexeme, "var1");
  cr_assert(get_token(tokens, 1)->type == T_IDENTIFIER);
  cr_assert_str_eq(get_token(tokens, 1)->lexeme, "abc123");
}

// ============================================================
// Tokenizer: Literals
// ============================================================

Test(tokenizer_lit, number_single_digit) {
  const char* src = "0 1 9";
  ArrayList* tokens = tokenize_string(src);
  cr_assert(get_token(tokens, 0)->type == T_NUMBER_LIT);
  cr_assert_str_eq(get_token(tokens, 0)->lexeme, "0");
  cr_assert(get_token(tokens, 1)->type == T_NUMBER_LIT);
  cr_assert_str_eq(get_token(tokens, 1)->lexeme, "1");
  cr_assert(get_token(tokens, 2)->type == T_NUMBER_LIT);
  cr_assert_str_eq(get_token(tokens, 2)->lexeme, "9");
}

Test(tokenizer_lit, number_multi_digit) {
  const char* src = "42 100 999999";
  ArrayList* tokens = tokenize_string(src);
  cr_assert_str_eq(get_token(tokens, 0)->lexeme, "42");
  cr_assert_str_eq(get_token(tokens, 1)->lexeme, "100");
  cr_assert_str_eq(get_token(tokens, 2)->lexeme, "999999");
}

Test(tokenizer_lit, string_basic) {
  const char* src = "\"hello\"";
  ArrayList* tokens = tokenize_string(src);
  cr_assert(get_token(tokens, 0)->type == T_STRING_LIT);
  cr_assert_str_eq(get_token(tokens, 0)->lexeme, "hello");
}

Test(tokenizer_lit, string_with_spaces) {
  const char* src = "\"hello world\"";
  ArrayList* tokens = tokenize_string(src);
  cr_assert(get_token(tokens, 0)->type == T_STRING_LIT);
  cr_assert_str_eq(get_token(tokens, 0)->lexeme, "hello world");
}

Test(tokenizer_lit, string_empty) {
  const char* src = "\"\"";
  ArrayList* tokens = tokenize_string(src);
  cr_assert(get_token(tokens, 0)->type == T_STRING_LIT);
  cr_assert_str_eq(get_token(tokens, 0)->lexeme, "");
}

// ============================================================
// Tokenizer: Operators
// ============================================================

Test(tokenizer_ops, single_char_operators) {
  const char* src = "( ) { } , . * / : & ; ! > < = + -";
  ArrayList* tokens = tokenize_string(src);
  Token_type expected[] = {T_LEFT_PAREN, T_RIGHT_PAREN, T_LEFT_BRACE, T_RIGHT_BRACE,
                           T_COMMA, T_DOT, T_STAR, T_DIVIDE, T_COLON, T_AND,
                           T_SEMICOLON, T_NOT, T_GREATER, T_LESS, T_EQUAL,
                           T_PLUS, T_MINUS, T_EOF};
  for (int i = 0; i < 18; i++) {
    cr_assert(get_token(tokens, i)->type == expected[i],
              "Token %d: expected %d, got %d", i, expected[i], get_token(tokens, i)->type);
  }
}

Test(tokenizer_ops, double_char_operators) {
  const char* src = "== != <= >= ++ --";
  ArrayList* tokens = tokenize_string(src);
  cr_assert(get_token(tokens, 0)->type == T_EQUAL_EQUAL);
  cr_assert(get_token(tokens, 1)->type == T_NOT_EQUAL);
  cr_assert(get_token(tokens, 2)->type == T_LESS_EQUAL);
  cr_assert(get_token(tokens, 3)->type == T_GREATER_EQUAL);
  cr_assert(get_token(tokens, 4)->type == T_PLUS_PLUS);
  cr_assert(get_token(tokens, 5)->type == T_MINUS_MINUS);
}

Test(tokenizer_ops, operators_no_spaces) {
  const char* src = "1+2";
  ArrayList* tokens = tokenize_string(src);
  cr_assert(get_token(tokens, 0)->type == T_NUMBER_LIT);
  cr_assert(get_token(tokens, 1)->type == T_PLUS);
  cr_assert(get_token(tokens, 2)->type == T_NUMBER_LIT);
}

// ============================================================
// Tokenizer: Comments
// ============================================================

Test(tokenizer_comments, single_line_comment) {
  const char* src = "// this is a comment\nfoo";
  ArrayList* tokens = tokenize_string(src);
  cr_assert(get_token(tokens, 0)->type == T_COMMENT);
  cr_assert(get_token(tokens, 1)->type == T_IDENTIFIER);
  cr_assert_str_eq(get_token(tokens, 1)->lexeme, "foo");
}

Test(tokenizer_comments, comment_before_code) {
  const char* src = "// comment\nlet";
  ArrayList* tokens = tokenize_string(src);
  cr_assert(get_token(tokens, 0)->type == T_COMMENT);
  cr_assert(get_token(tokens, 1)->type == T_LET);
}

Test(tokenizer_comments, divide_vs_comment) {
  const char* src = "1 / 2";
  ArrayList* tokens = tokenize_string(src);
  cr_assert(get_token(tokens, 0)->type == T_NUMBER_LIT);
  cr_assert(get_token(tokens, 1)->type == T_DIVIDE);
  cr_assert(get_token(tokens, 2)->type == T_NUMBER_LIT);
}

// ============================================================
// Tokenizer: Whitespace Handling
// ============================================================

Test(tokenizer_ws, tabs_and_spaces) {
  const char* src = "  \t  foo  \t  bar  ";
  ArrayList* tokens = tokenize_string(src);
  cr_assert(get_token(tokens, 0)->type == T_IDENTIFIER);
  cr_assert(get_token(tokens, 1)->type == T_IDENTIFIER);
  cr_assert(get_token(tokens, 2)->type == T_EOF);
}

Test(tokenizer_ws, newlines_count_lines) {
  const char* src = "foo\nbar\nbaz";
  ArrayList* tokens = tokenize_string(src);
  cr_assert(get_token(tokens, 0)->line == 1);
  cr_assert(get_token(tokens, 1)->line == 2);
  cr_assert(get_token(tokens, 2)->line == 3);
}

Test(tokenizer_ws, carriage_return_ignored) {
  const char* src = "foo\r\nbar";
  ArrayList* tokens = tokenize_string(src);
  cr_assert(get_token(tokens, 0)->type == T_IDENTIFIER);
  cr_assert(get_token(tokens, 1)->type == T_IDENTIFIER);
}

// ============================================================
// Tokenizer: EOF
// ============================================================

Test(tokenizer_eof, empty_input) {
  const char* src = "";
  ArrayList* tokens = tokenize_string(src);
  cr_assert(tokens->length == 1);
  cr_assert(get_token(tokens, 0)->type == T_EOF);
}

Test(tokenizer_eof, whitespace_only) {
  const char* src = "   \t\n\n  ";
  ArrayList* tokens = tokenize_string(src);
  cr_assert(get_token(tokens, tokens->length - 1)->type == T_EOF);
}

Test(tokenizer_eof, last_token_always_eof) {
  const char* src = "fn main";
  ArrayList* tokens = tokenize_string(src);
  Token* last = get_token(tokens, tokens->length - 1);
  cr_assert(last->type == T_EOF);
}

// ============================================================
// Tokenizer: Token Lengths
// ============================================================

Test(tokenizer_len, lexeme_length_tracking) {
  const char* src = "let foo";
  ArrayList* tokens = tokenize_string(src);
  cr_assert(get_token(tokens, 0)->length == 3); // "let"
  cr_assert(get_token(tokens, 1)->length == 3); // "foo"
}

// ============================================================
// Tokenizer: Complete Program Tokenization
// ============================================================

Test(tokenizer_program, simple_function) {
  const char* src = "fn DWORD main () {\n  return 0;\n}\n";
  ArrayList* tokens = tokenize_string(src);
  Token_type expected[] = {
    T_FUNC, T_DWORD, T_IDENTIFIER, T_LEFT_PAREN, T_RIGHT_PAREN,
    T_LEFT_BRACE, T_RETURN, T_NUMBER_LIT, T_SEMICOLON, T_RIGHT_BRACE, T_EOF
  };
  for (int i = 0; i < 11; i++) {
    cr_assert(get_token(tokens, i)->type == expected[i],
              "Token %d: expected %d, got %d", i, expected[i], get_token(tokens, i)->type);
  }
}

Test(tokenizer_program, var_declaration) {
  const char* src = "let DWORD x = 42;";
  ArrayList* tokens = tokenize_string(src);
  Token_type expected[] = {T_LET, T_DWORD, T_IDENTIFIER, T_EQUAL, T_NUMBER_LIT, T_SEMICOLON, T_EOF};
  for (int i = 0; i < 7; i++) {
    cr_assert(get_token(tokens, i)->type == expected[i],
              "Token %d: expected %d, got %d", i, expected[i], get_token(tokens, i)->type);
  }
  cr_assert_str_eq(get_token(tokens, 2)->lexeme, "x");
  cr_assert_str_eq(get_token(tokens, 4)->lexeme, "42");
}

Test(tokenizer_program, call_expression) {
  const char* src = "call foo(1, 2)";
  ArrayList* tokens = tokenize_string(src);
  Token_type expected[] = {T_CALL, T_IDENTIFIER, T_LEFT_PAREN, T_NUMBER_LIT,
                           T_COMMA, T_NUMBER_LIT, T_RIGHT_PAREN, T_EOF};
  for (int i = 0; i < 8; i++) {
    cr_assert(get_token(tokens, i)->type == expected[i],
              "Token %d: expected %d, got %d", i, expected[i], get_token(tokens, i)->type);
  }
}

Test(tokenizer_program, if_else_statement) {
  const char* src = "if (x > 0) { } else { }";
  ArrayList* tokens = tokenize_string(src);
  cr_assert(get_token(tokens, 0)->type == T_IF);
  cr_assert(get_token(tokens, 1)->type == T_LEFT_PAREN);
  cr_assert(get_token(tokens, 2)->type == T_IDENTIFIER);
  cr_assert(get_token(tokens, 3)->type == T_GREATER);
  cr_assert(get_token(tokens, 4)->type == T_NUMBER_LIT);
  cr_assert(get_token(tokens, 5)->type == T_RIGHT_PAREN);
  cr_assert(get_token(tokens, 6)->type == T_LEFT_BRACE);
  cr_assert(get_token(tokens, 7)->type == T_RIGHT_BRACE);
  cr_assert(get_token(tokens, 8)->type == T_ELSE);
}

Test(tokenizer_program, address_type_declaration) {
  const char* src = "let &DWORD ptr = &x;";
  ArrayList* tokens = tokenize_string(src);
  cr_assert(get_token(tokens, 0)->type == T_LET);
  cr_assert(get_token(tokens, 1)->type == T_AND);
  cr_assert(get_token(tokens, 2)->type == T_DWORD);
  cr_assert(get_token(tokens, 3)->type == T_IDENTIFIER);
  cr_assert(get_token(tokens, 4)->type == T_EQUAL);
  cr_assert(get_token(tokens, 5)->type == T_AND);
  cr_assert(get_token(tokens, 6)->type == T_IDENTIFIER);
  cr_assert(get_token(tokens, 7)->type == T_SEMICOLON);
}

Test(tokenizer_program, cast_expression) {
  const char* src = "(QWORD)x";
  ArrayList* tokens = tokenize_string(src);
  cr_assert(get_token(tokens, 0)->type == T_LEFT_PAREN);
  cr_assert(get_token(tokens, 1)->type == T_QWORD);
  cr_assert(get_token(tokens, 2)->type == T_RIGHT_PAREN);
  cr_assert(get_token(tokens, 3)->type == T_IDENTIFIER);
}
