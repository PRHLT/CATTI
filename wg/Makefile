NAME= wg
CXX = g++
CC = gcc

LIB= lib$(NAME).a
LIBS= -L /home/$(USER)/software/local/lib 
INC= -I /home/$(USER)/software/local/include
LIBS= -L /home/$(USER)/software/local/lib 
INC= -I /home/$(USER)/software/local/include

OBJS= entropy.o auxiliar.o zipio.o heap.o dynamic_list.o dynamic_ordered_list.o hash.o strop.o vector.o dictionary.o word_graph.o ecparser.o nbestsuffix.o CAT.o librefx.o sfst.o entropy.o

CFLAGS += $(INC) $(LIBS) -fPIC -Wall -Wextra -g3 -g -std=c99 -D_POSIX_C_SOURCE=200112L -Werror-implicit-function-declaration -Wc++-compat -Wunreachable-code

all: wg $(LIB) 

# Clean

clean:
	rm -f wg $(ALL) $(LIB) *.o *~

wg: main.c $(OBJS) lat-parser/liblat-parser.a
	$(CC) $(CFLAGS) -o $@ $^ -lm -lstdc++

server:  auxiliar.o zipio.o heap.o dynamic_list.o dynamic_ordered_list.o hash.o strop.o vector.o dictionary.o word_graph.o ecparser.o nbestsuffix.o CAT.o librefx.o sfst.o server.c lat-parser/liblat-parser.a
	$(CC) $(CFLAGS) -o $@ $^ -lm -lstdc++ -lfl -lpthread -lcat_api_nonshared -lstrokes_nonshared

$(LIB): $(OBJS)
	ar rvu $(LIB) $(OBJS)
	ranlib $(LIB)


