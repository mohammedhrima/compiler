// c headers
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdarg.h>
#include <math.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>
#include <fcntl.h>
#include <stdarg.h>

#ifndef DEBUG
#define DEBUG 1
#endif

// structs, enums
typedef struct Token Token;
typedef struct Node Node;
typedef enum Type Type;

// stupid implicit declaration error
Token *get_var(char *name);
Token *evaluate(Node *node);

enum Type
{
    none_,
    eof_,
    // operators
    add_,
    sub_,
    mul_,
    div_,
    mod_,
    // logic operator
    and_,
    or_,
    // comparision
    // TODO: expect expression after operator
    // TODO: handle not
    not_,
    not_equal_,
    equal_,
    less_than_,
    grea_than_,
    less_than_equal_,
    grea_than_equal_,
    // parents
    lparent_,
    rparent_,
    // assignement
    assign_,
#if 0
    // TODO: handle logic operators
    add_assign_,
    sub_assign_,
    mul_assign_,
    div_assign_,
#endif
    // memory
    fix_,
    dyn_,
    // Data types
    char_,
    int_,
    float_,
    bool_,
    identifier_,
    neg_,
    // function
    func_dec_,
    func_call_,
    coma_,
    // statements / loops
    if_,
    elif_,
    else_,
    // TODO: expect new line after :
    dots_,
};

struct Token
{
    Type type;
    Type sub_type;
    char *name;
    uintptr_t ptr;
    size_t level;
    union
    {
        long long int_;
        uint32_t float_;
        struct
        {
            char *char_;
            size_t index_;
        };
        struct
        {
            bool bool_;
            char c;
            size_t Label;
        };
    };
};

struct Node
{
    Node *left;
    Node *right;
    Token *token;
};

struct
{
    Type type;
    char *name;
} DataTypes[] = {
    // chars
    {char_, "char"},
    {char_, "fix:char"},
    {char_, "dyn:char"},
    // int
    {int_, "int"},
    {int_, "fix:int"},
    {int_, "dyn:int"}, /* big Int */
    // float
    {float_, "float"},
    {float_, "fix:float"},
    {float_, "fix:float"}, /* big Float */
    // bool
    {bool_, "bool"},
    // {bool_, "fix:bool"},
    // {bool_, "dyn:bool"},
    {0, 0},
};

struct
{
    Type type;
    char *name;
} Symbols[] = {
    // operators
    {add_, "+"},
    {sub_, "-"},
    {mul_, "*"},
    {div_, "/"},
    // parents, coma
    {lparent_, "("},
    {rparent_, ")"},
    {coma_, ","},
    // comparision
    {less_than_equal_, "<="},
    {grea_than_equal_, ">="},
    {not_equal_, "!="},
    {not_, "!"},
    {equal_, "=="},
    {equal_, "is "},
    {less_than_, "<"},
    {grea_than_, ">"},
    // assign
    {assign_, "="},
#if 0
    {add_assign_, "+="},
    {sub_assign_, "-="},
    {mul_assign_, "*="},
    {div_assign_, "/="},
#endif
    // logic
    {and_, "&&"},
    {or_, "||"},
    {dots_, ":"},
    {0, 0},
};

struct
{
    Type type;
    char *name;
} Random[] = {
    {eof_, "EOF"},
    {identifier_, "identifier"},
    // function
    {func_dec_, "function declaration"},
    {func_call_, "function call"},
    {neg_, "negative"},
    {fix_, "fix"},
    {dyn_, "dynamic"},
    {if_, "if"},
    {else_, "else"},
    {elif_, "elif"},
    {none_, "none"},
    {0, 0},
};

// Debuging
void error(char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    fprintf(stderr, "\033[0;31m");
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\033[0m\n");
    exit(1);
};

char *type_to_string(Type type)
{
    for (int i = 0; DataTypes[i].name; i++)
        if (DataTypes[i].type == type)
            return DataTypes[i].name;
    for (int i = 0; Symbols[i].name; i++)
        if (Symbols[i].type == type)
            return Symbols[i].name;
    for (int i = 0; Random[i].name; i++)
        if (Random[i].type == type)
            return Random[i].name;
    error("error unkown type (%d | %c)\n", type, type);
    return NULL;
}

void debug(char *conv, ...)
{
#if DEBUG
    size_t len = strlen(conv);
    size_t i = 0;

    va_list args;
    va_start(args, conv);
    while (i < len)
    {
        if (conv[i] == '%')
        {
            i++;
            if (strncmp(conv + i, "zu", 2) == 0)
            {
                fprintf(stdout, "%zu", va_arg(args, size_t));
                i++;
            }
            else
            {
                switch (conv[i])
                {
                case 'c':
                    fprintf(stdout, "%c", va_arg(args, int));
                    break;
                case 's':
                    fprintf(stdout, "%s", va_arg(args, char *));
                    break;
                case 'p':
                    fprintf(stdout, "%p", (size_t)(va_arg(args, void *)));
                    break;
                case 'x':
                    fprintf(stdout, "%x", (size_t)va_arg(args, void *));
                    break;
                case 'X':
                    fprintf(stdout, "%X", (size_t)va_arg(args, void *));
                    break;
                case 'd':
                    fprintf(stdout, "%d", (int)va_arg(args, int));
                    break;
                case 'f':
                    fprintf(stdout, "%f", va_arg(args, double));
                    break;
                case '%':
                    fprintf(stdout, "%%");
                    break;
                case 'k':
                {
                    Token *token = (Token *)va_arg(args, Token *);
                    if (token)
                    {
                        fprintf(stdout, "%s ", type_to_string(token->type));
#if 0
                    if (token->sub_type)
                        fprintf(stdout, "subtype: %s, ", type_to_string(token->sub_type));
#endif
                        if (token->name)
                            fprintf(stdout, "<name:%5s> ", token->name);
#if 1
                        else
#endif
                            switch (token->type)
                            {
                            case char_:
                                fprintf(stdout, "'%s' ", token->char_);
                                if (token->index_)
                                    fprintf(stdout, " STR%zu, ", token->index_);
                                break;
                            case int_:
                                fprintf(stdout, "%d ", token->int_);
                                break;
                            case float_:
                                fprintf(stdout, "%zu (%.2f) ", token->float_,
                                        *(float *)(&token->float_));
                                if (token->index_)
                                    fprintf(stdout, "FLT%zu ", token->index_);
                                break;
                            case bool_:
                                fprintf(stdout, "%s ", token->bool_ ? "True" : "False");
                                break;
                            default:
                                // ft_putstr(stdout, "Unkown");
                                break;
                            }
                        fprintf(stdout, " in LVL%zu", token->level);
                    }
                    else
                        fprintf(stdout, "(null)");
                    break;
                }
                default:
                    error("in debug function");
                    break;
                }
            }
        }
        else
            fprintf(stdout, "%c", conv[i]);
        i++;
    }
#endif
}

// file
FILE *file;
char *text;

// tokens
Token **tokens;
int tk_len;
int tk_pos;
int exe_pos;

// variables
Token **variables;
int var_len;
int var_pos;
uintptr_t ptr;
size_t index_;
/*
TODO:
    protect Label index, putstr and putnbr ...
    may cause problems
*/
// built ins

size_t Label;
char *Label_name;

size_t rsp;
FILE *asm_fd;
void print_asm(char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vfprintf(asm_fd, fmt, ap);
}

void output(Token *token)
{
    // TODO: handle bool
    debug("output %k\n", token);
    switch (token->type)
    {
    case char_:
    {
        print_asm("   /* call _putstr */\n");
        if (token->ptr)
        {
            print_asm("   mov     rax, QWORD PTR -%zu[rbp]\n", token->ptr);
            print_asm("   mov     rdi, rax\n", token->ptr);
            print_asm("   call    _putstr\n");
        }
        else if (token->index_)
        {
            print_asm("   lea     rax, STR%zu[rip]\n", token->index_);
            print_asm("   mov     rdi, rax\n", token->ptr);
            print_asm("   call    _putstr\n");
        }
        else
            error("output char");
        break;
    }
    case int_:
    {
        print_asm("   /* call _putnbr */\n");
        if (token->ptr)
        {
            print_asm("   mov   rax, QWORD PTR -%zu[rbp]\n", token->ptr);
            print_asm("   mov   rdi, rax\n");
            print_asm("   call  _putnbr\n");
        }
        else if (token->index_)
        {
            token->ptr = (ptr += 8);
            print_asm("   mov   QWORD PTR -%zu[rbp], %lld\n", token->ptr, token->int_);
            print_asm("   mov   rax, QWORD PTR -%zu[rbp]\n", token->ptr);
            print_asm("   mov   rdi, rax\n");
            print_asm("   call  _putnbr\n");
        }
        else
            error("output int");
        break;
    }
    case bool_:
    {
        print_asm("   /* call _putbool */\n");
        if (token->ptr)
            print_asm("   movzx   eax, BYTE PTR -%zu[rbp]\n", token->ptr);
        else if (token->c)
            print_asm("   movzx   eax, %cl\n", token->c);
        else
            error("output bool");
        print_asm("   mov	  edi, eax\n");
        print_asm("   call	  _putbool\n");
        break;
    }
    case float_:
        // TODO: handle float
        error("Error in output float not handled yet");
        break;
    case identifier_:
        break;
    default:
        error("Error in output unknown type");
        break;
    }
}

// utils
char *strjoin(char *left, char *right)
{
    char *res = calloc(strlen(left) + strlen(right) + 1, sizeof(char));
    strcpy(res, left);
    strcpy(res + strlen(res), right);
    return res;
}

// Tokens
void free_token(Token *token)
{
    debug("free token has type %s\n", type_to_string(token->type));
    if (token->name)
        free(token->name);
    if (token->type == char_)
        free(token->char_);
    free(token);
}

Token *new_token(int s, int e, Type type, Type sub_type, size_t level)
{
    Token *token = calloc(1, sizeof(Token));
    if (tk_pos + 10 > tk_len)
    {
        Token **tmp = calloc(tk_len * 2, sizeof(Token *));
        memcpy(tmp, tokens, tk_len * sizeof(Token *));
        free(tokens);
        tokens = tmp;
        tk_len *= 2;
    }
    token->type = type;
    token->sub_type = sub_type;
    token->level = level;
    switch (type)
    {
    case identifier_:
        token->name = calloc(e - s + 1, sizeof(char));
        strncpy(token->name, text + s, e - s);
        if (strcmp(token->name, "True") == 0)
        {
            token->type = bool_;
            token->bool_ = true;
        }
        else if (strcmp(token->name, "False") == 0)
        {
            token->type = bool_;
            token->bool_ = false;
        }
        else if (strcmp(token->name, "and") == 0)
            token->type = and_;
        else if (strcmp(token->name, "or") == 0)
            token->type = or_;
        else if (strcmp(token->name, "if") == 0)
            token->type = if_;
        else if (strcmp(token->name, "else") == 0)
            token->type = else_;
        else if (strcmp(token->name, "elif") == 0)
            token->type = elif_;
        break;
    case char_:
        token->index_ = index_;
        index_++;
        token->char_ = calloc(e - s + 1, sizeof(char));
        strncpy(token->char_, text + s, e - s);
        break;
    case int_:
        while (s < e)
            token->int_ = 10 * token->int_ + text[s++] - '0';
        break;
    case float_:
        float f = 0.0;
        token->index_ = index_;
        index_++;
        while (s < e && isdigit(text[s]))
            f = 10 * f + text[s++] - '0';
        s++;
        while (s < e && isdigit(text[s]))
            f = f + (float)(text[s++] - '0') / 10;
        token->float_ = *(uint32_t *)(&f);
        // exit(0);
        break;
    default:
        break;
    }
    debug("token %k \n", token);
    return (tokens[tk_pos++] = token);
}

Token *new_variable(Token *token)
{
    // TODO: check if you can remove initilize instruction
    switch (token->type)
    {
    case char_:
        token->ptr = (ptr += 8);
        // print_asm("\n");
        break;
    case int_:
        token->ptr = (ptr += 8);
        print_asm("   mov     QWORD PTR -%zu[rbp], %d /* declare %s */\n",
                  token->ptr, 0, token->name);
        break;
    case float_:
        token->ptr = (ptr += 4);
        print_asm("   mov     DWORD PTR -%zu[rbp], %d /* declare %s */\n",
                  token->ptr, 0, token->name);
        break;
    case bool_:
        token->ptr = (ptr += 1);
        print_asm("   mov     BYTE PTR -%zu[rbp], %d /* declare %s */\n",
                  token->ptr, 0, token->name);
    default:
        break;
    }
    if (var_pos + 10 > var_len)
    {
        Token **tmp = calloc(var_len * 2, sizeof(Token *));
        memcpy(tmp, variables, var_len * sizeof(Token *));
        free(variables);
        variables = tmp;
        var_len *= 2;
    }
    debug("new variable %k in ptr: %zu\n", token, token->ptr);
    return (variables[var_pos++] = token);
}

void build_tokens()
{
    int e = 0;
    // Expect main label
    // while (isspace(text[e]))
    //     e++;
    // TODO: main could any where
    size_t level = 0;
    if (strncmp(&text[e], "main:\n", strlen("main:\n")))
        error("main label with new line is required\n");
    e += strlen("main:");
    while (text[e])
    {
        Token *token = NULL;
        Type sub_type = fix_;
        int s = e;
        // TODO: if level < 1 throw error for non functions
        if (text[e] == '\n')
        {
            e++;
            level = 0;
            while (text[e] != '\n' && isspace(text[e]))
                e++;
            debug("found space with lenght %d\n", e - s);
            level = e - s;
            continue;
        }
        if (isspace(text[e]))
        {
            while (isspace(text[e]))
                e++;
            continue;
        }
        if (strlen(text + e) > 1 && strncmp(text + e, "/*", 2) == 0)
        {
            e += 2;
            while (text[e + 1] && strncmp(text + e, "*/", 2))
                e++;
            if (!text[e + 1])
                error("Expected end of comment");
            e += 2;
            continue;
        }
        // TODO: protect it from stupid parsing errors
        if (strncmp(text + e, "fix:", 4) == 0)
        {
            sub_type = fix_;
            e += 4;
        }
        else if (strncmp(text + e, "dyn:", 4) == 0)
        {
            sub_type = dyn_;
            e += 4;
        }
        for (int i = 0; Symbols[i].name; i++)
        {
            if (strncmp(Symbols[i].name, text + e, strlen(Symbols[i].name)) == 0)
            {
                token = new_token(s, e + strlen(Symbols[i].name), Symbols[i].type,
                                  sub_type, level);
                e += strlen(Symbols[i].name);
                break;
            }
        }
        if (token && token->type)
        {
            token = NULL;
            continue;
        }
        if (isalpha(text[e]))
        {
            while (isalnum(text[e]))
                e++;
            if (e > s)
            {
                new_token(s, e, identifier_, sub_type, level);
                continue;
            }
        }
        if (isdigit(text[e]))
        {
            Type type = int_;
            while (isdigit(text[e]))
                e++;
            if (text[e] == '.')
            {
                type = float_;
                e++;
            }
            while (isdigit(text[e]))
                e++;
            new_token(s, e, type, sub_type, level);
            continue;
        }
        // TODO: update error message
        if (strchr("\"\'", text[e]))
        {
            char quote = text[e++];
            while (text[e] && text[e] != quote)
                e++;
            if (text[e++] != quote)
                error("Syntax");
            new_token(s + 1, e - 1, char_, sub_type, level);
            continue;
        }
        error("tokenizing: %s", text + s);
    }
    new_token(0, 0, eof_, none_, level);
}

// Tree
void free_node(Node *node)
{
    if (node)
    {
        free_node(node->left);
        free_node(node->right);
        free(node);
    }
}

void print_node(Node *node, int level)
{
    if (node)
    {
        int curr = level;
        while (curr)
        {
            debug("    ");
            curr--;
        }
        debug("%k\n", node->token);
        print_node(node->left, level + 1);
        print_node(node->right, level + 1);
    }
}

Node *new_node(Token *token)
{
    Node *new = calloc(1, sizeof(Node));
    new->token = token;
    debug("new node %k\n", new->token);
    return new;
}

Node *expr();
Node *assign();     // =
Node *logic();      // || or && and
Node *equality();   // ==  !=
Node *comparison(); // < > <= >=
Node *add_sub();    // + -
Node *mul_div();    // * /
Node *unary();      // -
Node *prime();

Token *check(Type type, ...)
{
    va_list ap;
    va_start(ap, type);
    while (type)
    {
        if (type == tokens[exe_pos]->type)
            return tokens[exe_pos++];
        type = va_arg(ap, Type);
    }
    return NULL;
}

Token *expect(Type type, ...)
{
    va_list ap;
    va_start(ap, type);
    while (type)
    {
        if (type == tokens[exe_pos]->type)
            return tokens[exe_pos++];
        type = va_arg(ap, Type);
    }
    error("Unexpected %s\n", type_to_string(type));
    return NULL;
}

Node *expr()
{
    return assign();
}

Node *assign()
{
    Node *left = logic();
    Token *token;
    if (token = check(assign_, 0))
    {
        Node *node = new_node(token);
        node->left = left;
        node->right = assign();
        // left = node;
        return node;
    }
    return left;
}
Node *logic()
{
    Node *left = equality();
    Token *token;
#if 1
    if (token = check(or_, and_, 0))
    {
        Node *node = new_node(token);
        node->token->index_ = Label++;
        node->left = left;
        node->right = logic();
        return node;
    }

#else
    if (token = check(or_, and_, 0))
    {
        Node *node = new_node(token);
        node->token->index_ = Label++;
        node->left = left;
        node->right = logic();
        left = node;
        // return node;
    }
#endif
    return left;
}

Node *equality()
{
    Node *left = comparison();
    Token *token;
    if (token = check(equal_, not_equal_, 0))
    {
        Node *node = new_node(token);
        node->left = left;
        node->right = equality();
        left = node;
    }
    return left;
}

Node *comparison()
{
    Node *left = add_sub();
    Token *token;
    if (token = check(less_than_, grea_than_,
                      less_than_equal_, grea_than_equal_, 0))
    {
        Node *node = new_node(token);
        node->left = left;
        node->right = comparison();
        left = node;
    }
    return left;
}

Node *add_sub()
{
    Node *left = mul_div();
    Token *token;
#if 0
    while (token = check(add_, sub_, 0))
    {
        Node *node = new_node(token);
        node->right = left;
        node->left = mul_div();
        left = node;
    }
#else
    if (token = check(add_, sub_, 0))
    {
        Node *node = new_node(token);
        node->left = left;
        node->right = add_sub();
        return node;
    }
#endif
    return left;
}

Node *mul_div()
{
    Node *left = unary();
    Token *token;
#if 0
    while(token = check(mul_, div_, 0))
    {
        Node *node = new_node(token);
        node->right = left;
        node->left = unary();
        left = node;
    }
#else
    if (token = check(mul_, div_, 0))
    {
        Node *node = new_node(token);
        node->left = left;
        node->right = mul_div();
        return node;
    }
#endif
    return left;
}

// TODO: handle negative number / be carefull of casting
Node *unary()
{
    Token *token = check(add_, sub_, 0);
    // Node *left = else_statement();
    Node *left = prime();
    if (token && token->type == sub_)
    {
        // if (minus_int == NULL)
        // {
        //     minus_int = calloc(1, sizeof(Token));
        //     minus_int->type = int_;
        //     minus_int->int_ = -1;
        //     minus_float = calloc(1, sizeof(Token));
        //     minus_float->type = float_;
        //     float f = -1.0;
        //     minus_float->float_ = *(uint32_t *)(&f);
        //     minus_float->index_ = index_++;
        // }
        Node *node = new_node(new_token(0, 0, neg_, none_, left->token->level));
        node->left = left;
        return node;
    }
    return left;
}

Node *prime()
{
    Node *node = NULL;
    Token *token = NULL;
    if (token = check(identifier_, 0))
    {
        for (int i = 0; DataTypes[i].name; i++)
        {
            if (strcmp(DataTypes[i].name, token->name) == 0)
            {
                // TODO: update this code , try using the approach in new token function
                Type type = DataTypes[i].type;
                Type sub_type = token->sub_type;
                debug("find %s\n", type_to_string(DataTypes[i].type));
                node = new_node(expect(identifier_, 0));
                node->token->type = type;
                node->token->sub_type = sub_type;
                if (get_var(node->token->name))
                    error("redefinition of variable");
                new_variable(node->token);
                return node;
            }
        }
        Token *tmp_token;
        if (tmp_token = check(lparent_, 0))
        {
            debug("found function\n");
            /*
                TODO:
                    + function call
                    + function declaration
            */
            node = new_node(token);
            node->token->type = func_call_;
            node->left = expr();
            Node *tmp_node = node;
            while (tmp_token = check(coma_))
            {
                tmp_node->right = new_node(tmp_token);
                tmp_node->right->left = expr();
                tmp_node = tmp_node->right;
            }
            expect(rparent_);
            return node;
        }
        char *name = token->name;
        token = get_var(name);
        if (!token)
            error("Undeclared variable '%s'", name);
        node = new_node(token);
    }
    else if (token = check(lparent_, 0))
    {
        // expect(lparent_);
        node = expr();
        expect(rparent_);
    }
    else if (token = check(char_, int_, float_, eof_, bool_, 0))
        node = new_node(token);

    else if (token = check(if_, 0))
    {
        /*
            if:
                left:
                    left: condition
                    right: 
                        left:  code 
                        right: (next node)
                right: (next statement)
                    elif:
                        left:
                            left: condition
                            right:
                                left:  code
                                right: (next node)
                        right:
                            else:
                                left:  code
                                right: (next node)
        */
        size_t end_label = Label++;
        node = new_node(token);
        node->left = new_node(new_token(0, 0, none_, none_, node->token->level));
        node->token->index_ = Label++;
        node->left->token->index_ = end_label;

        // the condition bloc
        node->left->left = expr();
        expect(dots_);
        // statement bloc
        Node *tmp = node->left;
        while (
            tokens[exe_pos]->level > node->token->level &&
            tokens[exe_pos]->type != eof_)
        {
            tmp->right = new_node(NULL);
            tmp = tmp->right;
            tmp->left = expr();
        }
        // elif
        Node *tmp0 = node;
        while (token = check(elif_, 0))
        {
            tmp = tmp0;
            tmp->right = new_node(token);
            tmp0 = tmp->right; 
            tmp->right->token->index_ = Label++;
            tmp->right->left = new_node(NULL);
            tmp = tmp->right->left;

            // condition
            tmp->left = expr();
            expect(dots_);
            // bloc of code
            while (
                tokens[exe_pos]->level > token->level &&
                tokens[exe_pos]->type != eof_)
            {
                tmp->right = new_node(NULL);
                tmp = tmp->right;
                tmp->left = expr();
            }
        }
        // else
        tmp = tmp0;
        if (token = check(else_, 0))
        {
            expect(dots_);
            int i = 0;
            while (
                tokens[exe_pos]->level > token->level &&
                tokens[exe_pos]->type != eof_)
            {
                // debug("loop \n");
                if (i == 0)
                {
                    tmp->right = new_node(token);
                    tmp->right->token->index_ = Label++;
                    i++;
                }
                else
                    tmp->right = new_node(NULL);
                tmp = tmp->right;
                tmp->left = expr();
            }

        }

    }
    else // TODO: add Unexpected error message here
        error("%s in prime", token ? type_to_string(token->type) : "(null)");

    return node;
}

void initialize()
{
    // TODO: verify all label then set start label
    // write the assembly
    print_asm(".intel_syntax noprefix\n");
    print_asm(".include \"import/header.s\"\n\n");
    print_asm(".text\n");
    print_asm(".globl	main\n\n");
    print_asm("main:\n");
    print_asm("   push    rbp\n");
    print_asm("   mov     rbp, rsp\n");
    print_asm("   sub     rsp, %zu\n", rsp);

    Node *curr = NULL;
    while (tokens[exe_pos]->type != eof_)
    {
        Label_name = "main";
        curr = expr();
        print_node(curr, 0);
        evaluate(curr);
        free_node(curr);
    }
}

void finalize()
{
    // TODO: check exit status if changed
#if 0
    print_asm("   mov     rax, 0\n");
#else
    print_asm("\n");
#endif
    print_asm("   leave\n");
    print_asm("   ret\n\n");
    for (int i = 0; i < tk_pos; i++)
    {
        // test char variable before making any modification
        if (!tokens[i]->name && tokens[i]->index_ && tokens[i]->type == char_)
            print_asm("STR%zu: .string \"%s\"\n", tokens[i]->index_, tokens[i]->char_);
        if (!tokens[i]->name && tokens[i]->index_ && tokens[i]->type == float_)
            print_asm("FLT%zu: .long %zu /* %f */\n", tokens[i]->index_,
                      *((float *)(&tokens[i]->float_)),
                      tokens[i]->float_);
    }
    print_asm(".section	.note.GNU-stack,\"\",@progbits\n");
}

Token *get_var(char *name)
{
    for (int i = 0; i < var_pos; i++)
        if (variables[i]->name && strcmp(variables[i]->name, name) == 0)
            return variables[i];
    return NULL;
}

Token *evaluate(Node *node)
{
    // debug("Evaluate %k\n", node->token);
    Token *left = NULL, *right = NULL;
    Type type = node->token->type;
    switch (type)
    {
    case identifier_:
    case char_:
    case float_:
    case int_:
    case bool_:
        break;
    case neg_:
    {
        // TODO: negative float has a different behaviour !!!
        left = evaluate(node->left);
        if (left->type != int_ && left->type != float_)
            error("Invalid unary operation 0");

        if (!left->name)
        {
            node->token->type = left->type;
            switch (left->type)
            {
            case int_:
                // TODO: protect INT_MIN
                node->token->int_ = -1 * left->int_;
                break;
            case float_:
                float f = -1 * (*(float *)(&left->float_));
                left->float_ = *(uint32_t *)(&f);
                node->token = left;
                break;
            default:
                break;
            }
        }
        else
        {
            Node *curr = new_node(new_token(0, 0, mul_, none_, left->level));
            curr->left = new_node(left);
            if (left->type == int_)
            {
                curr->right = new_node(new_token(0, 0, int_, none_, left->level));
                curr->right->token->int_ = -1;
            }
            else if (left->type == float_)
            {
                curr->right = new_node(new_token(0, 0, float_, none_, left->level));
                float f = -1.0;
                curr->right->token->float_ = *(uint32_t *)(&f);
                curr->right->token->index_ = index_++;
            }
            else
                error("Invalid unary operation 1");
            print_node(curr, 0);
            node->token = evaluate(curr);
            free_node(curr);
        }
        break;
    }
    case assign_:
    {
        /*
            TODOS:
                + assign / initializing
                + deep / shallow copy
        */
        left = evaluate(node->left);
        right = evaluate(node->right);
        debug("assign %k and %k \n", left, right);
        if (!left->name || left->type != right->type || !left->ptr)
            error("Invalid assignement");
        // print_asm(" %0s/* assign  %s */\n", "", left->name);
        node->token = left;
        switch (left->type)
        {
        case int_:
            if (right->ptr)
            {
                print_asm("   mov     rax, QWORD PTR -%zu[rbp]\n", right->ptr);
                print_asm("   mov     QWORD PTR -%zu[rbp], rax /* assign  %s */\n",
                          left->ptr, left->name);
            }
            else
                print_asm("   mov     QWORD PTR -%zu[rbp], %d /* assign  %s */\n\n",
                          left->ptr, right->int_, left->name);
            break;
        case float_:
            // TODO: check xmms, with multiple variables
            if (right->ptr)
            {
                print_asm("   movss   xmm1, DWORD PTR -%zu[rbp]\n", right->ptr);
                print_asm("   movss   DWORD PTR -%zu[rbp], xmm1 /* assign  %s */\n",
                          left->ptr, left->name);
            }
            else
            {
                print_asm("   movss   xmm1, DWORD PTR FLT%zu[rip]\n", right->index_);
                print_asm("   movss   DWORD PTR -%zu[rbp], xmm1 /* assign  %s */\n",
                          left->ptr, left->name);
            }
            break;
        case char_:
            /*
                TODO:
                    + check right has ptr, then load value from there
                    + strdup from right
            */
            if (left->sub_type == fix_)
            {
                if (right->index_)
                    print_asm("   lea     rax, STR%zu[rip]\n", right->index_);
                else
                    print_asm("   mov     rax, QWORD PTR -%zu[rbp]\n", right->ptr);
            }
            else if (left->sub_type == dyn_)
            {
                if (right->index_)
                    print_asm("   lea     rax, STR%zu[rip]\n", right->index_);
                else
                    print_asm("   mov     rax, QWORD PTR -%zu[rbp]\n", right->ptr);
                print_asm("   mov     rdi, rax\n");
                print_asm("   call    _strdup\n");
            }
            else
                error("in assign char\n");

            print_asm("   mov     QWORD PTR -%zu[rbp], rax /* assign  %s */\n", left->ptr,
                      left->name);
            break;
        case bool_:
            if (right->ptr)
            {
                // TODO: test this one
                print_asm("   mov     al, BYTE PTR -%zu[rbp]\n", right->ptr);
                print_asm("   mov     BYTE PTR -%zu[rbp], al /* assign  %s */\n", left->ptr,
                          left->name);
            }
            else if (right->c)
                print_asm("   mov     BYTE PTR -%zu[rbp], %cl\n", left->ptr, right->c);
            else
                print_asm("   mov     BYTE PTR -%zu[rbp], %d /* assign  %s */\n", left->ptr,
                          right->bool_, left->name);
            break;
        default:
            error("add assembly for this one 0");
            break;
        }
        break;
    }
    // arithmetic operators
    case add_:
    case sub_:
    case mul_:
    case div_:
    {
        left = evaluate(node->left);
        right = evaluate(node->right);
        if (left->type != right->type)
            error("Uncompatible type in math operation");
        node->token->type = left->type;
        // has no name // optimization
        if (!left->ptr && !right->ptr)
        {
            debug("0. do %s between %k with %k\n", type_to_string(type), left, right);
            left->index_ = 0;
            right->index_ = 0;
            switch (node->token->type)
            {
            case int_:
                if (type == add_)
                    node->token->int_ = left->int_ + right->int_;
                else if (type == sub_)
                    node->token->int_ = left->int_ - right->int_;
                else if (type == mul_)
                    node->token->int_ = left->int_ * right->int_;
                else if (type == div_)
                {
                    if (right->int_ == 0)
                        error("can't devide by 0 (int)");
                    node->token->int_ = left->int_ / right->int_;
                }
                break;
            case float_:
                node->token->index_ = index_++;
                float l = *(float *)(&left->float_);
                float r = *(float *)(&right->float_);
                float res;
                if (type == add_)
                    res = l + r;
                else if (type == sub_)
                    res = l - r;
                else if (type == mul_)
                    res = l * r;
                else if (type == div_)
                {
                    if (r == 0)
                        error("can't devide by 0 (float)");
                    res = l / r;
                }
                node->token->float_ = *(uint32_t *)(&res);
                break;
            // TODO: handle strings that get concatinated in run time
            case char_:
                node->token->index_ = index_++;
                node->token->sub_type = dyn_;
                if (type == add_)
                    node->token->char_ = strjoin(left->char_, right->char_);
                else
                    error("invalid math operation for characters");
                break;
            default:
                error("math operation 0");
                break;
            }
        }
        else
        {
            // TODO: addition for dynamic strings
            debug("1. do %s between %k with %k\n", type_to_string(type), left, right);
            char *str;
            switch (node->token->type)
            {
            case int_:
                node->token->ptr = (ptr += 8);
                // set left
                print_asm("   mov     rax, ");
                if (left->ptr)
                    print_asm("QWORD PTR -%zu[rbp]\n", left->ptr);
                else
                    print_asm("%d\n", left->int_);
                // set right
                print_asm("   %s", type == add_   ? "add     rax, "
                                   : type == sub_ ? "sub     rax, "
                                   : type == mul_ ? "imul    rax, "
                                   : type == div_ ? "cdq\n   mov     rbx, "
                                                  : NULL);
                if (right->ptr)
                    print_asm("QWORD PTR -%zu[rbp]\n", right->ptr);
                else
                    print_asm("%d\n", right->int_);
                if (type == div_)
                    print_asm("   idiv    rbx\n");
                print_asm("   mov     QWORD PTR -%zu[rbp], rax\n", node->token->ptr);
                break;
            case float_:
                node->token->ptr = (ptr += 4);
                // set left
                print_asm("   movss   xmm1, ");
                if (left->ptr)
                    print_asm("DWORD PTR -%zu[rbp]\n", left->ptr);
                else if (left->index_)
                    print_asm("DWORD PTR FLT%zu[rip]\n", left->index_);
                else
                    print_asm("%zu\n", left->float_);
                // set right
                print_asm("   %s", type == add_   ? "addss   xmm1, "
                                   : type == sub_ ? "subss   xmm1, "
                                   : type == mul_ ? "mulss   xmm1, "
                                   : type == div_ ? "divss   xmm1, "
                                                  : NULL);
                if (right->ptr)
                    print_asm("DWORD PTR -%zu[rbp]\n", right->ptr);
                else if (right->index_)
                    print_asm("DWORD PTR FLT%zu[rip]\n", right->index_);
                else
                    print_asm("%zu\n", right->float_);
                print_asm("   movss   DWORD PTR -%zu[rbp], xmm1\n", node->token->ptr);
                break;
            case char_:
                if (type != add_)
                    error("math operation 2");
                node->token->sub_type = dyn_;
                node->token->ptr = (ptr += 8);
                if (left->ptr)
                    print_asm("   mov     rdi, QWORD PTR -%zu[rbp]\n", left->ptr);
                else if (left->index_)
                {
                    print_asm("   lea     rax, STR%zu[rip]\n", left->index_);
                    print_asm("   mov     rdi, rax\n");
                }
                else
                    error("in char joining 1");

                if (right->ptr)
                    print_asm("   mov     rsi, QWORD PTR -%zu[rbp]\n", right->ptr);
                else if (right->index_)
                {
                    print_asm("   lea     rax, STR%zu[rip]\n", right->index_);
                    print_asm("   mov     rsi, rax\n");
                }
                else
                    error("in char joining 2");
                print_asm("   call	  _strjoin\n");
                print_asm("   mov     QWORD PTR -%zu[rbp], rax\n", node->token->ptr);

                break;
            default:
                error("math operation 1");
                break;
            }
        }
        break;
    }
    // logic operators
    case not_:
        // TODO:
        error("handle not logic operator");
        break;
    case not_equal_:
    case equal_:
    case less_than_:
    case grea_than_:
    case less_than_equal_:
    case grea_than_equal_:
    {
        node->token->type = bool_;
        left = evaluate(node->left);
        right = evaluate(node->right);
        if (left->type != right->type)
            error("Uncompatible type in logic operation");
        // has no name // optimization
        if (!left->ptr && !right->ptr)
        {
            debug("0. do %s between %k with %k\n", type_to_string(type), left, right);
            left->index_ = 0;
            right->index_ = 0;
            switch (left->type)
            {
            case int_:
                if (type == equal_)
                    node->token->bool_ = (left->int_ == right->int_);
                else if (type == not_equal_)
                    node->token->bool_ = (left->int_ != right->int_);
                else if (type == less_than_)
                    node->token->bool_ = (left->int_ < right->int_);
                else if (type == grea_than_)
                    node->token->bool_ = (left->int_ > right->int_);
                else if (type == less_than_equal_)
                    node->token->bool_ = (left->int_ <= right->int_);
                else if (type == grea_than_equal_)
                    node->token->bool_ = (left->int_ >= right->int_);

                break;
            case float_:
                if (type == equal_)
                    node->token->bool_ = (left->float_ == right->float_);
                else if (type == not_equal_)
                    node->token->bool_ = (left->float_ != right->float_);
                else if (type == less_than_)
                    node->token->bool_ = (left->float_ < right->float_);
                else if (type == grea_than_)
                    node->token->bool_ = (left->float_ > right->float_);
                else if (type == less_than_equal_)
                    node->token->bool_ = (left->float_ <= right->float_);
                else if (type == grea_than_equal_)
                    node->token->bool_ = (left->float_ >= right->float_);
                break;
            /*
                TODO:
                    + handle strings that get concatinated in run time
                    + strcmp shoud return boolean value
            */
            case char_:
                if (type == equal_)
                    node->token->bool_ = (strcmp(left->char_, right->char_) == 0);
                else if (type == not_equal_)
                    node->token->bool_ = (strcmp(left->char_, right->char_) != 0);
                else
                    error("Invalid logic operation on char");
                break;
            default:
                error("logic operation 0");
                break;
            }
        }
        else
        {
            debug("1. do %s between %k with %k\n", type_to_string(type), left, right);
            // TODO: nested if statement maybe can cause problem ???
#define BOOL_PTR 0
#if BOOL_PTR
            node->token->ptr = (ptr += 1);
#else
            node->token->c = 'a';
#endif
            char *str;
            switch (left->type)
            {
            // TODO: handle heap allocated variables
            case int_:
                // set left
                print_asm("   mov     rax, ");
                if (left->ptr)
                    print_asm("QWORD PTR -%zu[rbp]\n", left->ptr);
                else
                    print_asm("%d\n", left->int_);

                print_asm("   cmp     rax, ");
                if (right->ptr)
                    print_asm("QWORD PTR -%zu[rbp]\n", right->ptr);
                else
                    print_asm("%d\n", right->int_);

                print_asm("   %s  al\n", type == equal_             ? "sete  "
                                         : type == equal_           ? "setne"
                                         : type == less_than_       ? "setl "
                                         : type == less_than_equal_ ? "setle"
                                         : type == grea_than_       ? "setg "
                                         : type == grea_than_equal_ ? "setge"
                                                                    : NULL);

#if BOOL_PTR
                print_asm("   mov     BYTE PTR -%zu[rbp], al\n", node->token->ptr);
#endif
                break;
            case float_:
                if ((type == less_than_ && (type = grea_than_)) ||
                    (type == less_than_equal_ && (type = grea_than_equal_)))
                {
                    Token *tmp = left;
                    left = right;
                    right = tmp;
                }
                print_asm("   movss   xmm0, ");
                if (left->ptr)
                    print_asm("DWORD PTR -%zu[rbp]\n", left->ptr);
                else if (left->index_)
                    print_asm("DWORD PTR FLT%zu[rip]\n", left->index_);
                else
                    print_asm("%zu\n", left->float_);

                print_asm("   %s xmm0, ", type != equal_ ? "comiss " : "ucomiss");
                if (right->ptr)
                    print_asm("DWORD PTR -%zu[rbp]\n", right->ptr);
                else if (right->index_)
                    print_asm("DWORD PTR FLT%zu[rip]\n", right->index_);
                else
                    print_asm("%zu\n", right->float_);

                print_asm("   %s   al\n",
                          type == grea_than_         ? "seta "
                          : type == grea_than_equal_ ? "setnb"
                          : type == equal_           ? "setnp"
                          : type == not_equal_       ? "setp "
                                                     : NULL);
                if (type == equal_)
                {
                    print_asm("   mov	  edx, 0\n");
                    print_asm("   movss	  xmm0, ");
                    if (left->ptr)
                        print_asm("DWORD PTR -%zu[rbp]\n", left->ptr);
                    else if (left->index_)
                        print_asm("DWORD PTR FLT%zu[rip]\n", left->index_);
                    else
                        print_asm("%zu\n", left->float_);
                    print_asm("   ucomiss xmm0, ");
                    if (right->ptr)
                        print_asm("DWORD PTR -%zu[rbp]\n", right->ptr);
                    else if (right->index_)
                        print_asm("DWORD PTR FLT%zu[rip]\n", right->index_);
                    else
                        print_asm("%zu\n", right->float_);
                    print_asm("   cmovne  eax, edx\n");
                }
#if BOOL_PTR
                print_asm("   mov     BYTE PTR -%zu[rbp], al\n", node->token->ptr);
#endif
                break;
            case char_:
                if (type != equal_)
                    error("logic operation 3");
                if (left->ptr)
                    print_asm("   mov     rsi, QWORD PTR -%zu[rbp]\n", left->ptr);
                else if (left->index_)
                {
                    print_asm("   lea     rax, STR%zu[rip]\n", left->index_);
                    print_asm("   mov     rsi, rax\n");
                }
                else
                    error("in char equal");

                if (right->ptr)
                    print_asm("   mov     rdi, QWORD PTR -%zu[rbp]\n", right->ptr);
                else if (right->index_)
                {
                    print_asm("   lea     rax, STR%zu[rip]\n", right->index_);
                    print_asm("   mov     rdi, rax\n");
                }
                else
                    error("in char equal 2");
                print_asm("   call	  _strcmp\n");
#if BOOL_PTR
                print_asm("   mov     BYTE PTR -%zu[rbp], al\n", node->token->ptr);
#endif
                break;
            default:
                error("logic operation 4");
                break;
            }
        }
        break;
    }
    case and_:
    case or_:
    {
        int i = 0;
        Node *tmp = node;
        Node *tmp0 = tmp;

        // tmp = node;
        while (tmp->token->type == or_ || tmp->token->type == and_)
        {
            left = evaluate(tmp->left);
            if (left->type != bool_)
                error("0.Expected boolean value");
            print_asm("   /* %s operation %d */\n", type_to_string(tmp->token->type), i++);
            if (left->ptr)
                print_asm("   cmp     BYTE PTR -%zu[rbp], 1\n", left->ptr);
            else if (left->c)
                print_asm("   cmp     %cl, 1\n", left->c);
            else // TODO: handle if has value is True or False
            {
                print_asm("   mov     al, %d\n", left->bool_);
                print_asm("   cmp     al, 1\n");
            }
            if (tmp->token->type == or_)
                print_asm("   je      %s%zu\n", Label_name, node->token->index_);
            else if (tmp->token->type == and_)
                print_asm("   jne      %s%zu\n", Label_name, node->token->index_);
            tmp0 = tmp;
            tmp = tmp->right;
        }

        left = evaluate(tmp);
        if (left->type != bool_)
            error("0.Expected boolean value");
        print_asm("   /* %s operation %d */\n", type_to_string(tmp0->token->type), i++);
        if (left->ptr)
            print_asm("   cmp     BYTE PTR -%zu[rbp], 1\n", left->ptr);
        else if (left->c)
            print_asm("   cmp     %cl, 1\n", left->c);
        else // TODO: handle if has value is True or False
        {
            print_asm("   mov     al, %d\n", left->bool_);
            print_asm("   cmp     al, 1\n");
        }
        if (tmp0->token->type == or_)
            print_asm("   je      %s%zu\n", Label_name, node->token->index_);
        else if (tmp0->token->type == and_)
            print_asm("   jne      %s%zu\n", Label_name, node->token->index_);
        print_asm("%s%zu:\n", Label_name, node->token->index_);
        node->token->type = bool_;
        break;
    }
    case if_:
    {
        printf("evaluate if\n");
        Node *curr = node->left;

        left = evaluate(curr->left);
        // size_t if_ptr = left->ptr;
        if (left->type != bool_)
            error("Expected a valid condition in if statment");

        print_asm("%s%zu: %43s\n", Label_name, node->token->index_, "/* if statement */");
        if (left->ptr)
            print_asm("   cmp     BYTE PTR -%zu[rbp], 1\n", left->ptr);
        else if (left->c)
            print_asm("   cmp     %cl, 1\n", left->c);
        else
        {
            print_asm("   mov     al, %d\n", left->bool_);
            print_asm("   cmp     al, 1\n");
        }
        if (node->right)
            print_asm("   jne     %s%zu %39s\n", Label_name, node->right->token->index_, "/* jmp next statement */");
        else
            print_asm("   jne     %s%zu %15s\n", Label_name, node->left->token->index_, "/* jmp end statemnt */");
        curr = curr->right;
        // if statment bloc
        while (curr)
        {
            evaluate(curr->left);
            curr = curr->right;
        }
        if (node->right)
            print_asm("   jmp     %s%zu %38s\n", Label_name, node->left->token->index_, "/* jmp end statement */");

        // elif / else statement
        curr = node->right;
        while (curr)
        {
            if (curr->token->type == elif_)
            {
                // evaluate elif
                print_asm("%s%zu: %45s\n", Label_name, curr->token->index_, "/* elif statement */");
                Node *tmp = curr->left;

                left = evaluate(tmp->left);
                // size_t if_ptr = left->ptr;
                if (left->type != bool_)
                    error("Expected a valid condition in if statment");
                if (left->ptr)
                    print_asm("   cmp     BYTE PTR -%zu[rbp], 1\n", left->ptr);
                else if (left->c)
                    print_asm("   cmp     %cl, 1\n", left->c);
                else
                {
                    print_asm("   mov     al, %d\n", left->bool_);
                    print_asm("   cmp     al, 1\n");
                }
                if (curr->right)
                    print_asm("   jne     %s%zu %39s\n", Label_name, curr->right->token->index_, "/* jmp next statement */");
                else
                    print_asm("   jne     %s%zu %38s\n", Label_name, node->left->token->index_, "/* jmp end statemnt */");
                tmp = tmp->right;
                while (tmp)
                {
                    evaluate(tmp->left);
                    tmp = tmp->right;
                }
                if (curr->right)
                    print_asm("   jmp     %s%zu %38s\n", Label_name, node->left->token->index_, "/* jmp end statement */");
                curr = curr->right;
            }
            else if (curr->token->type == else_)
            {
                print_asm("%s%zu: %45s\n", Label_name, curr->token->index_, "/* else statement */");
                while (curr)
                {
                    evaluate(curr->left);
                    curr = curr->right;
                }
            }
        }

        // end statement bloc
        print_asm("%s%zu: %44s\n", Label_name, node->left->token->index_, "/* end statement */");

        break;
    }

    case func_call_:
    {
        debug("found function call has name '%s'\n", node->token->name);
        if (strncmp("output", node->token->name, strlen("output")) == 0)
        {
            debug("found output\n");
            Node *tmp = node;

            while (tmp)
            {
                output(evaluate(tmp->left));
                // debug("%k\n",tmp->token);
                // printf("loop\n");
                tmp = tmp->right;
            }
            // exit(0);
        }
        break;
    }
    default:
        error("in evaluate %s", type_to_string(type));
        break;
    }
    if (ptr + 10 > rsp)
    {
        // TODO: protect this line from being printed in wrong place
        // after label for example
        rsp += 30;
        print_asm("   sub     rsp, 30\n");
    }
    return node->token;
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        // TODO: check if file ends with .hr
        error("require one file.hr as argument\n");
    }
#if 1
    file = fopen(argv[1], "r");
#else
    file = fopen("file.hr", "r");
#endif

#if 0
    char *filename = strdup(argv[1]);
    filename[strlen(filename) - 2] = 's';
    filename[strlen(filename) - 1] = 0;
    
    // opening file
    debug("open %s\n", argv[1]);
    debug("write to %s\n", filename);
    asm_fd = fopen(filename, "w");
    free(filename);
#else
    asm_fd = fopen("file.s", "w");
#endif
    if (!file || !asm_fd)
        error("Opening file");
    fseek(file, 0, SEEK_END);
    size_t txt_len = ftell(file);
    if (!(text = calloc(txt_len + 1, sizeof(char))))
        error("Allocation");
    fseek(file, 0, SEEK_SET);
    fread(text, txt_len, sizeof(char), file);
    fclose(file);
    debug("%s\n\n", text);
    Label = 1;
    index_ = 1;
    tk_len = var_len = 100;
    tokens = calloc(tk_len, sizeof(Token *));
    variables = calloc(tk_len, sizeof(Token *));
    build_tokens();
    debug("\n");
    free(text);
    // TODO: verify rsp position
    rsp = 30;
    initialize();
    finalize();
    debug("\nresult: \n");
}
