// Copyright (c) 2023-2026 Chris (boreddevnl)
// This software is released under the GNU General Public License v3.0. See LICENSE file for details.
// This header needs to maintain in any file it is present in, as per the GPL license terms.
#include <stdlib.h>
#include <syscall.h>
#include "../vm.h"

// --- Compiler Limits ---
#define MAX_SOURCE 65536
#define MAX_TOKENS 16384
#define MAX_VARS 512
#define CODE_SIZE 32768
#define STR_POOL_SIZE 16384

static int compile_error = 0;

// --- Lexer ---
typedef enum {
    TOK_EOF,
    TOK_INT,      // 123, 0xFF
    TOK_STRING,   // "hello"
    TOK_ID,       // abc
    TOK_PLUS,     // +
    TOK_MINUS,    // -
    TOK_MUL,      // *
    TOK_DIV,      // /
    TOK_ASSIGN,   // =
    TOK_LPAREN,   // (
    TOK_RPAREN,   // )
    TOK_LBRACKET, // [
    TOK_RBRACKET, // ]
    TOK_LBRACE,   // {
    TOK_RBRACE,   // }
    TOK_SEMI,     // ;
    TOK_COMMA,    // ,
    TOK_EQ,       // ==
    TOK_NEQ,      // !=
    TOK_LT,       // <
    TOK_GT,       // >
    TOK_LE,       // <=
    TOK_GE,       // >=
    TOK_IF,       // if
    TOK_ELSE,     // else
    TOK_WHILE,    // while
    TOK_INT_TYPE, // int
    TOK_CHAR_TYPE,// char
    TOK_VOID_TYPE,// void
    TOK_MAIN      // main
} TokenType;

typedef struct {
    TokenType type;
    int int_val;
    char str_val[64]; // Identifier or String Content
} Token;

static char *source_ptr;
static Token tokens[MAX_TOKENS];
static int token_count = 0;

static void lex_error(const char *msg) {
    printf("Compiler Error: %s\n", msg);
    compile_error = 1;
}

static void lexer(const char *source) {
    source_ptr = (char*)source;
    token_count = 0;
    compile_error = 0;

    while (*source_ptr) {
        while (*source_ptr == ' ' || *source_ptr == '\n' || *source_ptr == '\t' || *source_ptr == '\r') source_ptr++;
        if (!*source_ptr) break;

        if (*source_ptr == '/' && *(source_ptr+1) == '/') {
            while (*source_ptr && *source_ptr != '\n') source_ptr++;
            continue;
        }

        Token *t = &tokens[token_count++];
        
        if (*source_ptr == '0' && (*(source_ptr+1) == 'x' || *(source_ptr+1) == 'X')) {
            source_ptr += 2;
            t->type = TOK_INT;
            t->int_val = 0;
            int has_digits = 0;
            while ((*source_ptr >= '0' && *source_ptr <= '9') || 
                   (*source_ptr >= 'a' && *source_ptr <= 'f') ||
                   (*source_ptr >= 'A' && *source_ptr <= 'F')) {
                int digit = 0;
                if (*source_ptr >= '0' && *source_ptr <= '9') digit = *source_ptr - '0';
                else if (*source_ptr >= 'a' && *source_ptr <= 'f') digit = *source_ptr - 'a' + 10;
                else if (*source_ptr >= 'A' && *source_ptr <= 'F') digit = *source_ptr - 'A' + 10;
                t->int_val = (t->int_val << 4) | digit;
                source_ptr++;
                has_digits = 1;
            }
            if (!has_digits) { lex_error("Invalid hex literal"); return; }
        } else if (*source_ptr >= '0' && *source_ptr <= '9') {
            t->type = TOK_INT;
            t->int_val = 0;
            while (*source_ptr >= '0' && *source_ptr <= '9') {
                t->int_val = t->int_val * 10 + (*source_ptr - '0');
                source_ptr++;
            }
        } else if (*source_ptr == '"') {
            t->type = TOK_STRING;
            source_ptr++;
            int len = 0;
            while (*source_ptr && *source_ptr != '"') {
                if (*source_ptr == '\\' && *(source_ptr+1) == 'n') {
                    if (len < 63) t->str_val[len++] = '\n';
                    source_ptr += 2;
                } else {
                    if (len < 63) t->str_val[len++] = *source_ptr;
                    source_ptr++;
                }
            }
            t->str_val[len] = 0;
            if (*source_ptr == '"') source_ptr++;
        } else if (*source_ptr == '\'') {
            t->type = TOK_INT;
            source_ptr++;
            char c = 0;
            if (*source_ptr == '\\') {
                source_ptr++;
                if (*source_ptr == 'n') c = '\n';
                else if (*source_ptr == 't') c = '\t';
                else if (*source_ptr == '0') c = '\0';
                else if (*source_ptr == '\\') c = '\\';
                else if (*source_ptr == '\'') c = '\'';
                else c = *source_ptr;
                source_ptr++;
            } else {
                c = *source_ptr;
                source_ptr++;
            }
            if (*source_ptr == '\'') source_ptr++;
            else { lex_error("Expected closing '"); return; }
            t->int_val = (int)c;
        } else if ((*source_ptr >= 'a' && *source_ptr <= 'z') || (*source_ptr >= 'A' && *source_ptr <= 'Z') || *source_ptr == '_') {
            int len = 0;
            while ((*source_ptr >= 'a' && *source_ptr <= 'z') || (*source_ptr >= 'A' && *source_ptr <= 'Z') || (*source_ptr >= '0' && *source_ptr <= '9') || *source_ptr == '_') {
                if (len < 63) t->str_val[len++] = *source_ptr;
                source_ptr++;
            }
            t->str_val[len] = 0;
            if (strcmp(t->str_val, "if") == 0) t->type = TOK_IF;
            else if (strcmp(t->str_val, "else") == 0) t->type = TOK_ELSE;
            else if (strcmp(t->str_val, "while") == 0) t->type = TOK_WHILE;
            else if (strcmp(t->str_val, "int") == 0) t->type = TOK_INT_TYPE;
            else if (strcmp(t->str_val, "char") == 0) t->type = TOK_CHAR_TYPE; 
            else if (strcmp(t->str_val, "void") == 0) t->type = TOK_VOID_TYPE;
            else if (strcmp(t->str_val, "main") == 0) t->type = TOK_MAIN;
            else t->type = TOK_ID;
        } else {
            switch (*source_ptr) {
                case '+': t->type = TOK_PLUS; break;
                case '-': t->type = TOK_MINUS; break;
                case '*': t->type = TOK_MUL; break;
                case '/': t->type = TOK_DIV; break;
                case '(': t->type = TOK_LPAREN; break;
                case ')': t->type = TOK_RPAREN; break;
                case '[': t->type = TOK_LBRACKET; break;
                case ']': t->type = TOK_RBRACKET; break;
                case '{': t->type = TOK_LBRACE; break;
                case '}': t->type = TOK_RBRACE; break;
                case ';': t->type = TOK_SEMI; break;
                case ',': t->type = TOK_COMMA; break;
                case '=': 
                    if (*(source_ptr+1) == '=') { t->type = TOK_EQ; source_ptr++; } 
                    else t->type = TOK_ASSIGN; 
                    break;
                case '!':
                    if (*(source_ptr+1) == '=') { t->type = TOK_NEQ; source_ptr++; } 
                    else { lex_error("Unexpected !"); return; }
                    break;
                case '<':
                    if (*(source_ptr+1) == '=') { t->type = TOK_LE; source_ptr++; } 
                    else t->type = TOK_LT;
                    break;
                case '>':
                    if (*(source_ptr+1) == '=') { t->type = TOK_GE; source_ptr++; } 
                    else t->type = TOK_GT;
                    break;
                default: lex_error("Unknown char"); return;
            }
            source_ptr++;
        }
    }
    tokens[token_count].type = TOK_EOF;
}

typedef struct {
    const char *name;
    int syscall_id;
} Builtin;

static const Builtin builtins[] = {
    {"exit", VM_SYS_EXIT},
    {"print_int", VM_SYS_PRINT_INT},
    {"print_char", VM_SYS_PRINT_CHAR},
    {"print_str", VM_SYS_PRINT_STR},
    {"print", VM_SYS_PRINT_INT},
    {"pritc", VM_SYS_PRINT_CHAR},
    {"puts", VM_SYS_PRINT_STR},
    {"nl", VM_SYS_NL},
    {"cls", VM_SYS_CLS},
    {"getchar", VM_SYS_GETCHAR},
    {"strlen", VM_SYS_STRLEN},
    {"strcmp", VM_SYS_STRCMP},
    {"strcpy", VM_SYS_STRCPY},
    {"strcat", VM_SYS_STRCAT},
    {"memset", VM_SYS_MEMSET},
    {"memcpy", VM_SYS_MEMCPY},
    {"malloc", VM_SYS_MALLOC},
    {"free", VM_SYS_FREE},
    {"rand", VM_SYS_RAND},
    {"srand", VM_SYS_SRAND},
    {"abs", VM_SYS_ABS},
    {"min", VM_SYS_MIN},
    {"max", VM_SYS_MAX},
    {"pow", VM_SYS_POW},
    {"sqrt", VM_SYS_SQRT},
    {"sleep", VM_SYS_SLEEP},
    {"fopen", VM_SYS_FOPEN},
    {"fclose", VM_SYS_FCLOSE},
    {"fread", VM_SYS_FREAD},
    {"fwrite", VM_SYS_FWRITE},
    {"fseek", VM_SYS_FSEEK},
    {"remove", VM_SYS_REMOVE},
    {"draw_pixel", VM_SYS_DRAW_PIXEL},
    {"draw_rect", VM_SYS_DRAW_RECT},
    {"draw_line", VM_SYS_DRAW_LINE},
    {"draw_text", VM_SYS_DRAW_TEXT},
    {"get_width", VM_SYS_GET_WIDTH},
    {"get_height", VM_SYS_GET_HEIGHT},
    {"get_time", VM_SYS_GET_TIME},
    {"kb_hit", VM_SYS_KB_HIT},
    {"mouse_x", VM_SYS_MOUSE_X},
    {"mouse_y", VM_SYS_MOUSE_Y},
    {"mouse_state", VM_SYS_MOUSE_STATE},
    {"play_sound", VM_SYS_PLAY_SOUND},
    {"atoi", VM_SYS_ATOI},
    {"itoa", VM_SYS_ITOA},
    {"peek", VM_SYS_PEEK},
    {"poke", VM_SYS_POKE},
    {"exec", VM_SYS_EXEC},
    {"system", VM_SYS_SYSTEM},
    {"strchr", VM_SYS_STRCHR},
    {"memcmp", VM_SYS_MEMCMP},
    {"isalnum", VM_SYS_ISALNUM},
    {"isalpha", VM_SYS_ISALPHA},
    {"isdigit", VM_SYS_ISDIGIT},
    {"tolower", VM_SYS_TOLOWER},
    {"toupper", VM_SYS_TOUPPER},
    {"strncpy", VM_SYS_STRNCPY},
    {"strncat", VM_SYS_STRNCAT},
    {"strncmp", VM_SYS_STRNCMP},
    {"strstr", VM_SYS_STRSTR},
    {"strrchr", VM_SYS_STRRCHR},
    {"memmove", VM_SYS_MEMMOVE},
    {NULL, 0}
};

static int find_builtin(const char *name) {
    for (int i = 0; builtins[i].name != NULL; i++) {
        if (strcmp(builtins[i].name, name) == 0) return builtins[i].syscall_id;
    }
    return -1;
}

static uint8_t code[CODE_SIZE];
static int code_pos = 0;
static int cur_token = 0;
static uint8_t str_pool[STR_POOL_SIZE];
static int str_pool_pos = 0;

typedef struct {
    char name[32];
    int addr;
} Symbol;

static Symbol symbols[MAX_VARS];
static int symbol_count = 0;
static int next_var_addr = 32768;

static int find_symbol(const char *name) {
    for (int i = 0; i < symbol_count; i++) {
        if (strcmp(symbols[i].name, name) == 0) return symbols[i].addr;
    }
    return -1;
}

static int add_symbol(const char *name) {
    int existing = find_symbol(name);
    if (existing != -1) return existing;
    if (symbol_count >= MAX_VARS) return -1;
    strcpy(symbols[symbol_count].name, name);
    symbols[symbol_count].addr = next_var_addr;
    next_var_addr += 4;
    return symbol_count++;
}

static void emit(uint8_t b) {
    if (code_pos < CODE_SIZE) code[code_pos++] = b;
    else { printf("Error: Code buffer overflow\n"); compile_error = 1; }
}

static void emit32(int v) {
    emit(v & 0xFF); emit((v >> 8) & 0xFF); emit((v >> 16) & 0xFF); emit((v >> 24) & 0xFF);
}

static int add_string(const char *str) {
    int start = str_pool_pos;
    int len = strlen(str);
    if (str_pool_pos + len + 1 >= STR_POOL_SIZE) { printf("Error: String pool overflow\n"); compile_error = 1; return 0; }
    for(int i=0; i<len; i++) str_pool[str_pool_pos++] = str[i];
    str_pool[str_pool_pos++] = 0;
    return start;
}

static void match(TokenType t) {
    if (compile_error) return;
    if (tokens[cur_token].type == t) cur_token++;
    else { printf("Syntax Error: Expected token %d got %d\n", t, tokens[cur_token].type); compile_error = 1; }
}

static void expression();
static void statement();
static void block();

static void function_call(int syscall_id) {
    if (compile_error) return;
    cur_token++;
    match(TOK_LPAREN);
    if (tokens[cur_token].type != TOK_RPAREN) {
        expression();
        while (tokens[cur_token].type == TOK_COMMA) { cur_token++; expression(); }
    }
    match(TOK_RPAREN);
    emit(OP_SYSCALL);
    emit32(syscall_id);
}

static void factor() {
    if (compile_error) return;
    if (tokens[cur_token].type == TOK_INT) { emit(OP_IMM); emit32(tokens[cur_token].int_val); cur_token++; }
    else if (tokens[cur_token].type == TOK_STRING) { int offset = add_string(tokens[cur_token].str_val); emit(OP_PUSH_PTR); emit32(offset); cur_token++; }
    else if (tokens[cur_token].type == TOK_ID) {
        int syscall = find_builtin(tokens[cur_token].str_val);
        if (syscall != -1 && tokens[cur_token+1].type == TOK_LPAREN) function_call(syscall);
        else {
            int addr = find_symbol(tokens[cur_token].str_val);
            if (addr == -1) { printf("Error: Undefined variable: %s\n", tokens[cur_token].str_val); compile_error = 1; }
            emit(OP_LOAD); emit32(addr); cur_token++;
        }
    } else if (tokens[cur_token].type == TOK_LPAREN) { cur_token++; expression(); match(TOK_RPAREN); }
    else { printf("Syntax Error: Unexpected token in factor\n"); compile_error = 1; }
}

static void term() {
    if (compile_error) return;
    factor();
    while (tokens[cur_token].type == TOK_MUL || tokens[cur_token].type == TOK_DIV) {
        TokenType op = tokens[cur_token].type; cur_token++; factor();
        if (op == TOK_MUL) emit(OP_MUL); else emit(OP_DIV);
    }
}

static void additive() {
    if (compile_error) return;
    term();
    while (tokens[cur_token].type == TOK_PLUS || tokens[cur_token].type == TOK_MINUS) {
        TokenType op = tokens[cur_token].type; cur_token++; term();
        if (op == TOK_PLUS) emit(OP_ADD); else emit(OP_SUB);
    }
}

static void relation() {
    if (compile_error) return;
    additive();
    if (tokens[cur_token].type >= TOK_EQ && tokens[cur_token].type <= TOK_GE) {
        TokenType op = tokens[cur_token].type; cur_token++; additive();
        switch (op) {
            case TOK_EQ: emit(OP_EQ); break;
            case TOK_NEQ: emit(OP_NEQ); break;
            case TOK_LT: emit(OP_LT); break;
            case TOK_GT: emit(OP_GT); break;
            case TOK_LE: emit(OP_LE); break;
            case TOK_GE: emit(OP_GE); break;
            default: break;
        }
    }
}

static void expression() { if (compile_error) return; relation(); }

static void statement() {
    if (compile_error) return;
    if (tokens[cur_token].type == TOK_INT_TYPE || tokens[cur_token].type == TOK_CHAR_TYPE) {
        cur_token++;
        while (tokens[cur_token].type == TOK_MUL) cur_token++;
        if (tokens[cur_token].type == TOK_ID) {
            add_symbol(tokens[cur_token].str_val);
            cur_token++;
            if (tokens[cur_token].type == TOK_LBRACKET) {
                cur_token++; if (tokens[cur_token].type == TOK_INT) cur_token++;
                if (tokens[cur_token].type == TOK_RBRACKET) cur_token++;
                else printf("Error: Expected ]\n");
            }
            if (tokens[cur_token].type == TOK_ASSIGN) {
                int addr = find_symbol(tokens[cur_token-1].str_val);
                cur_token++; expression(); emit(OP_STORE); emit32(addr);
            }
            match(TOK_SEMI);
        } else { printf("Syntax Error: Expected identifier\n"); compile_error = 1; }
    } else if (tokens[cur_token].type == TOK_ID) {
        int syscall = find_builtin(tokens[cur_token].str_val);
        if (syscall != -1 && tokens[cur_token+1].type == TOK_LPAREN) {
            function_call(syscall); match(TOK_SEMI); emit(OP_POP); 
        } else {
            int addr = find_symbol(tokens[cur_token].str_val);
            if (addr == -1) { printf("Error: Undefined variable assignment: %s\n", tokens[cur_token].str_val); compile_error = 1; return; }
            cur_token++; match(TOK_ASSIGN); expression(); match(TOK_SEMI); emit(OP_STORE); emit32(addr);
        }
    } else if (tokens[cur_token].type == TOK_IF) {
        cur_token++; match(TOK_LPAREN); expression(); match(TOK_RPAREN);
        emit(OP_JZ); int jz_addr_pos = code_pos; emit32(0); block();
        if (tokens[cur_token].type == TOK_ELSE) {
            emit(OP_JMP); int jmp_addr_pos = code_pos; emit32(0); 
            int else_start = code_pos;
            code[jz_addr_pos] = else_start & 0xFF; code[jz_addr_pos+1] = (else_start >> 8) & 0xFF; code[jz_addr_pos+2] = (else_start >> 16) & 0xFF; code[jz_addr_pos+3] = (else_start >> 24) & 0xFF;
            cur_token++; block();
            int end_addr = code_pos;
            code[jmp_addr_pos] = end_addr & 0xFF; code[jmp_addr_pos+1] = (end_addr >> 8) & 0xFF; code[jmp_addr_pos+2] = (end_addr >> 16) & 0xFF; code[jmp_addr_pos+3] = (end_addr >> 24) & 0xFF;
        } else {
            int end_addr = code_pos;
            code[jz_addr_pos] = end_addr & 0xFF; code[jz_addr_pos+1] = (end_addr >> 8) & 0xFF; code[jz_addr_pos+2] = (end_addr >> 16) & 0xFF; code[jz_addr_pos+3] = (end_addr >> 24) & 0xFF;
        }
    } else if (tokens[cur_token].type == TOK_WHILE) {
        int start_addr = code_pos; cur_token++; match(TOK_LPAREN); expression(); match(TOK_RPAREN);
        emit(OP_JZ); int jz_addr_pos = code_pos; emit32(0); block();
        emit(OP_JMP); emit32(start_addr);
        int end_addr = code_pos;
        code[jz_addr_pos] = end_addr & 0xFF; code[jz_addr_pos+1] = (end_addr >> 8) & 0xFF; code[jz_addr_pos+2] = (end_addr >> 16) & 0xFF; code[jz_addr_pos+3] = (end_addr >> 24) & 0xFF;
    } else cur_token++;
}

static void block() {
    if (compile_error) return;
    match(TOK_LBRACE);
    while (tokens[cur_token].type != TOK_RBRACE && tokens[cur_token].type != TOK_EOF && !compile_error) statement();
    match(TOK_RBRACE);
}

static void program() {
    if (tokens[cur_token].type == TOK_INT_TYPE || tokens[cur_token].type == TOK_VOID_TYPE) cur_token++;
    if (tokens[cur_token].type == TOK_MAIN) cur_token++;
    match(TOK_LPAREN); match(TOK_RPAREN); block(); emit(OP_HALT);
}

int main(int argc, char **argv) {
    if (argc < 2) { printf("Usage: cc <filename.c>\n"); return 1; }

    int fh = sys_open(argv[1], "r");
    if (fh < 0) { printf("Error: Cannot open source file.\n"); return 1; }
    
    char *source = (char*)malloc(MAX_SOURCE);
    if (!source) { printf("Error: Out of memory for source buffer.\n"); sys_close(fh); return 1; }

    int len = sys_read(fh, source, MAX_SOURCE - 1);
    source[len] = 0;
    sys_close(fh);

    lexer(source);
    free(source);

    if (compile_error) return 1;
    
    code_pos = 0; symbol_count = 0; cur_token = 0; str_pool_pos = 0; next_var_addr = 32768; 
    
    const char* magic = VM_MAGIC;
    for(int i=0; i<7; i++) emit(magic[i]);
    emit(1); 
    program();
    
    if (compile_error) { printf("Compilation Failed.\n"); return 1; }
    
    int pool_start_addr = code_pos;
    for(int i=0; i<str_pool_pos; i++) emit(str_pool[i]);
    
    int pc = 8;
    while (pc < pool_start_addr) {
        uint8_t op = code[pc++];
        switch (op) {
            case OP_HALT: break; case OP_IMM: case OP_LOAD: case OP_STORE: case OP_LOAD8: case OP_STORE8: case OP_JMP: case OP_JZ: case OP_SYSCALL: pc += 4; break;
            case OP_ADD: case OP_SUB: case OP_MUL: case OP_DIV: case OP_PRINT: case OP_PRITC: case OP_EQ: case OP_NEQ: case OP_LT: case OP_GT: case OP_LE: case OP_GE: case OP_POP: break;
            case OP_PUSH_PTR: {
                int offset = 0;
                offset |= code[pc]; offset |= code[pc+1] << 8; offset |= code[pc+2] << 16; offset |= code[pc+3] << 24;
                int abs_addr = pool_start_addr + offset;
                code[pc] = abs_addr & 0xFF; code[pc+1] = (abs_addr >> 8) & 0xFF; code[pc+2] = (abs_addr >> 16) & 0xFF; code[pc+3] = (abs_addr >> 24) & 0xFF;
                pc += 4; code[pc-5] = OP_IMM; break;
            }
            default: break;
        }
    }
    
    char out_name[64]; int i = 0;
    while(argv[1][i] && argv[1][i] != '.') { out_name[i] = argv[1][i]; i++; }
    out_name[i] = 0;

    int out_fh = sys_open(out_name, "w");
    if (out_fh >= 0) {
        sys_write_fs(out_fh, code, code_pos);
        sys_close(out_fh);
        printf("Compilation successful. Output: %s\n", out_name);
    } else { printf("Error: Cannot write output file.\n"); }
    
    return 0;
}
