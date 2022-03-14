# paths
LIB = ../lib
INCLUDE = ../include
MODULES = ../modules

# compiler
CC = gcc

# Compile options. Το -I<dir> λέει στον compiler να αναζητήσει εκεί include files
CFLAGS = -Wall -g -I$(INCLUDE)
LDFLAGS = -lm

# Αρχεία .o
OBJS1 = state_test.o $(MODULES)/state_alt.o $(MODULES)/set_utils.o $(LIB)/k08.a $(LIB)/libraylib.a
OBJS2 = set_utils_test.o $(MODULES)/set_utils.o $(LIB)/k08.a $(LIB)/libraylib.a

# Το εκτελέσιμο πρόγραμμα
EXEC1 = state_test
EXEC2 = set_utils_test

# Παράμετροι για δοκιμαστική εκτέλεση
ARGS =

# Παράμετροι της βιβλιοθήκης raylib
include $(LIB)/libraylib.mk

$(EXEC1): $(OBJS1)
	$(CC) $(OBJS1) -o $(EXEC1) $(LDFLAGS)

clean:
	rm -f $(OBJS1) $(EXEC1)

run: $(EXEC1)
	./$(EXEC1) $(ARGS)

$(EXEC2): $(OBJS2)
	$(CC) $(OBJS2) -o $(EXEC2) $(LDFLAGS)

clean_test:
	rm -f $(OBJS2) $(EXEC2)

run_test: $(EXEC2)
	./$(EXEC2) $(ARGS)

# Για να φτιάξουμε τα k08.a/libraylib.a τρέχουμε το make στο lib directory.
$(LIB)/%.a:
	$(MAKE) -C $(LIB) $*.a