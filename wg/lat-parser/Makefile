YACC=bison
LEX=flex

NAME= lat
UPPERNAME= $(shell echo $(NAME) | tr '[a-z]' '[A-Z]')
DIR= $(shell basename $(PWD))

LIB= $(NAME)-parser
SOURCES= $(NAME)-parser.c $(NAME)-scanner.c $(NAME)-flex.c
LIBS=
OBJS= $(SOURCES:.c=.o)

INC=

CFLAGS += $(INC) $(LIBS) -Wall -O3 -fPIC 

all: $(LIB)

%.c: %.y
	$(YACC) -o$(@:%.o=%.d) $<

%.c: %.l
	$(LEX) -o$(@:%.o=%.d) $<

$(LIB): $(OBJS)
	ar rvu lib$@.a $(OBJS)
	ranlib lib$@.a

test: test.c $(LIB) 
	$(CC) test.c lib$(LIB).a $(CFLAGS) -o test -ly -lfl

# Clean
clean:
	$(RM) -R $(DEPEND_DIR) $(BIN) $(OBJS) lib$(LIB).a $(NAME)-scanner.c $(NAME)-parser.h $(NAME)-parser.c test

distclean: clean
	$(RM) *~

