/*
 * token.h — tipos de token para o analisador léxico de C
 * Cobre C89/C90 completo. Extensões C99/C11 marcadas com comentário.
 */

#ifndef TOKEN_H
#define TOKEN_H

typedef enum TokenType {

    /* ── keywords (32 em C89) ──────────────────────────────── */
    KW_AUTO,         /* auto        */
    KW_BREAK,        /* break       */
    KW_CASE,         /* case        */
    KW_CHAR,         /* char        */
    KW_CONST,        /* const       */
    KW_CONTINUE,     /* continue    */
    KW_DEFAULT,      /* default     */
    KW_DO,           /* do          */
    KW_DOUBLE,       /* double      */
    KW_ELSE,         /* else        */
    KW_ENUM,         /* enum        */
    KW_EXTERN,       /* extern      */
    KW_FLOAT,        /* float       */
    KW_FOR,          /* for         */
    KW_GOTO,         /* goto        */
    KW_IF,           /* if          */
    KW_INT,          /* int         */
    KW_LONG,         /* long        */
    KW_REGISTER,     /* register    */
    KW_RETURN,       /* return      */
    KW_SHORT,        /* short       */
    KW_SIGNED,       /* signed      */
    KW_SIZEOF,       /* sizeof      */
    KW_STATIC,       /* static      */
    KW_STRUCT,       /* struct      */
    KW_SWITCH,       /* switch      */
    KW_TYPEDEF,      /* typedef     */
    KW_UNION,        /* union       */
    KW_UNSIGNED,     /* unsigned    */
    KW_VOID,         /* void        */
    KW_VOLATILE,     /* volatile    */
    KW_WHILE,        /* while       */
    /* C99+ */
    KW_INLINE,       /* inline      (C99) */
    KW_RESTRICT,     /* restrict    (C99) */
    KW_BOOL,         /* _Bool       (C99) */
    KW_COMPLEX,      /* _Complex    (C99) */
    KW_NORETURN,     /* _Noreturn   (C11) */
    KW_STATIC_ASSERT,/* _Static_assert (C11) */

    /* ── literais ─────────────────────────────────────────── */
    LIT_INT,         /* 42  077  0xFF  42U  42L  42ULL  */
    LIT_FLOAT,       /* 3.14  1e10  .5f  2.0L           */
    LIT_CHAR,        /* 'a'  '\n'  '\x41'  L'ç'      */
    LIT_STRING,      /* "hello"  "with\tescape"  L"w"  */

    /* ── identificador ─────────────────────────────────────── */
    TOK_IDENT,       /* myVar  _count  __func__          */

    /* ── operadores aritméticos ────────────────────────────── */
    OP_PLUS,         /* +   */
    OP_MINUS,        /* -   */
    OP_STAR,         /* *   (também ponteiro/deref) */
    OP_SLASH,        /* /   */
    OP_PERCENT,      /* %   */
    OP_INC,          /* ++  */
    OP_DEC,          /* --  */

    /* ── operadores relacionais ────────────────────────────── */
    OP_EQ,           /* ==  */
    OP_NEQ,          /* !=  */
    OP_LT,           /* <   */
    OP_GT,           /* >   */
    OP_LTE,          /* <=  */
    OP_GTE,          /* >=  */

    /* ── operadores lógicos ────────────────────────────────── */
    OP_AND,          /* &&  */
    OP_OR,           /* ||  */
    OP_NOT,          /* !   */

    /* ── operadores bit a bit ──────────────────────────────── */
    OP_BAND,         /* &   (também endereço-de) */
    OP_BOR,          /* |   */
    OP_BXOR,         /* ^   */
    OP_BNOT,         /* ~   */
    OP_SHL,          /* <<  */
    OP_SHR,          /* >>  */

    /* ── atribuição ────────────────────────────────────────── */
    OP_ASSIGN,       /* =   */
    OP_PLUS_ASSIGN,  /* +=  */
    OP_MINUS_ASSIGN, /* -=  */
    OP_STAR_ASSIGN,  /* *=  */
    OP_SLASH_ASSIGN, /* /=  */
    OP_PCT_ASSIGN,   /* %=  */
    OP_AND_ASSIGN,   /* &=  */
    OP_OR_ASSIGN,    /* |=  */
    OP_XOR_ASSIGN,   /* ^=  */
    OP_SHL_ASSIGN,   /* <<= */
    OP_SHR_ASSIGN,   /* >>= */

    /* ── acesso a membros / ponteiros ──────────────────────── */
    OP_DOT,          /* .   struct.membro */
    OP_ARROW,        /* ->  ptr->membro   */

    /* ── outros operadores ─────────────────────────────────── */
    OP_TERNARY_Q,    /* ?   (operador ternário) */
    OP_TERNARY_C,    /* :   (também em switch/label) */

    /* ── pontuação ─────────────────────────────────────────── */
    TOK_LPAREN,      /* (   */
    TOK_RPAREN,      /* )   */
    TOK_LBRACE,      /* {   */
    TOK_RBRACE,      /* }   */
    TOK_LBRACKET,    /* [   */
    TOK_RBRACKET,    /* ]   */
    TOK_SEMICOLON,   /* ;   */
    TOK_COMMA,       /* ,   */
    TOK_ELLIPSIS,    /* ... (variadic: printf(fmt, ...)) */

    /* ── tokens especiais ──────────────────────────────────── */
    TOK_EOF,         /* fim do ficheiro de entrada       */
    TOK_ERROR,       /* caracter inválido (ex: @, $, `) */
    TOK_NEWLINE,     /* \n — útil para contar linhas    */
    TOK_COMMENT,     /* // ou /* */

} TokenType;

/* Tabela de lookup: texto → keyword token
 * Usada após reconhecer um identificador: se estiver
 * na tabela, é keyword; caso contrário, é TOK_IDENT. */
typedef struct { const char *word; TokenType type; } Keyword;

extern const Keyword keywords[];
extern const int    num_keywords;

/* Converte TokenType → string legível (para debug) */
const char *token_type_name(TokenType t);

#endif /* TOKEN_H */