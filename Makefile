#############################################################
# Makefile - build recursivo respeitando hierarquia de pastas
#############################################################

# Compilador e flags
CC      := gcc
CFLAGS  := -Wall -Wextra -g

# Incluir directorias (ajuste se tiver headers noutros locais)
INCLUDES := -I.

NAME    := analisador_lexico

## Lista explícita de ficheiros do projecto (main no root, e ficheiros em Lexer/ e utils/)
SRCS := main.c \
	Lexer/lexer.c \
	Lexer/idenumKey.c \
	Lexer/operators.c \
	Lexer/stringchar.c \
	utils/get_next_line.c \
	Lexer/numeric.c \
	utils/ft_strjoin.c \
	Lexer/processor_directive.c \
	utils/token_operating.c

# AST and Parser
SRCS += ast/ast.c \
	parser/init.c \
	parser/expressao.c \
	parser/tipo.c \
	parser/helpers.c \
	parser/declaracao_global.c \
	parser/programa.c \
	parser/statements.c


# Criar lista de .o correspondentes (preserva hierarquia: ./dir/file.o)
OBJS := $(SRCS:.c=.o)

# Headers (dependência de recompilação) — listar explicitamente se preferir
HEADERS := token.h

# Regra principal
.PHONY: all
all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $(OBJS)

# Regra geral para compilar .c -> .o (vai colocar .o ao lado do .c)
%.o: %.c $(HEADERS)
	@echo Compilando $<
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Limpeza: remove todos os .o encontrados e o executável
.PHONY: clean fclean re
clean:
	@echo "Removendo objectos..."
	rm -f $(OBJS)

fclean: clean
	@echo "Removendo executável $(NAME)..."
	rm -f $(NAME)

re: fclean all

# Alvos úteis
.PHONY: show-srcs show-objs
show-srcs:
	@printf "%s\n" $(SRCS)

show-objs:
	@printf "%s\n" $(OBJS)

# Evitar conflito com ficheiros chamados "all", "clean"…