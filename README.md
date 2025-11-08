# The A language

The A language is a simple compiled language, using a custom syntax developed by myself, and translated directly to x86-64 AT&T assembly.

The A language has many similarities to the C language, but with less type safety and a stronger focus on explicit low-level control.

A programs are compiled into raw assembly and assembled with `as` or linked with `ld`, making it ideal for understanding calling conventions, ABI design, and compiler construction concepts.

## Road Map

1. Develop a tokenizer for syntax types (FINSIHED)
2. Create a parser to turn the stream of tokens into an AST (FINSIHED)
3. Create an assembler to convert the AST directly to assembly (IN PROGRESS)

## Syntax

### Varibale Types:

- BYTE --> holds a single byte of information (8 bits)
- WORD --> holds two bytes of information (16 bits)
- DWORD --> holds 4 bytes of information (32 bits)
- QWORD --> holds 8 bytes of information (64 bits)

Any of these types can have a `&` appended to the beginning to turn them into an address type.
These types can be derefrenced with the `[]` operator which will grab the type specified after the `&`.
EX: let &WORD foo = 1; <-- this will hold the address 0x00000001, which can be accessed through [x] which will grab the memory at 0x00000001.

### Variable Decleration:

Variables are created using the `let` keyword followed by a type an name.
