CC = gcc
CXX = g++
CFLAGS = -std=c++14 -march=native -Wall -Wextra -pedantic -Wno-write-strings -fsanitize=address -O1 -fno-omit-frame-pointer


ODIR = obj
LDIR = lib
OUT = $(LDIR)/xavierlib.a
SDIR = src
INC = -Iinclude

_OBJS = score.o \
		seed.o \
		aligner.o \
		trace.o \
		vectors.o \
		xavier.o

OBJS = $(patsubst %,$(ODIR)/%,$(_OBJS))


$(ODIR)/%.o: $(SDIR)/%.cpp
	mkdir -p $(ODIR)
	$(CXX) $< -c -o $@ $(INC) $(CFLAGS)

$(OUT): $(OBJS)
	mkdir -p $(LDIR)
	ar rvs $(OUT) $^

.PHONY: clean

clean:
	rm -rf $(ODIR) $(LDIR) $(OUT)