#ifndef PANIC_MODE_H
#define PANIC_MODE_H

typedef struct Parser Parser;

extern const char *const SYNC_DECLARACAO[];
extern const char *const SYNC_BLOCO[];
extern const char *const SYNC_INSTRUCAO[];
extern const char *const SYNC_EXPRESSAO[];
extern const char *const SYNC_HEADER[];


void recover(Parser *p, const char *const sync_set[]);
void syntax_error_recover(Parser *p, const char *msg, const char *const sync_set[]);
int parser_error_count(void);
void parser_reset_error_count(void);
void parser_sincronizar_geral(Parser *p);
int parser_expect(Parser *p, const char *valor_esperado, const char *mensagem_erro, const char *const sync_set[]);

#endif