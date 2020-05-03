CC      = gcc
AR      = ar
ARFLAGS = cr
override LDFLAGS = -lm
override CFLAGS += -Wall -fwrapv -march=native
#override CFLAGS += -DUSE_SIMD

ifeq ($(OS),Windows_NT)
	override CFLAGS += -D_WIN32
	RM = del
else
	override LDFLAGS += -pthread
	#RM = rm
endif

.PHONY : all debug libcubiomes clean

all: CFLAGS += -O3 -march=native
all: god clean

debug: CFLAGS += -DDEBUG -O0 -ggdb3
debug: god clean

libcubiomes: CFLAGS += -O3 -fPIC
libcubiomes: layers.o generator.o finders.o util.o
	$(AR) $(ARFLAGS) libcubiomes.a $^

find_compactbiomes: find_compactbiomes.o layers.o generator.o finders.o
	$(CC) -o $@ $^ $(LDFLAGS)

find_compactbiomes.o: find_compactbiomes.c
	$(CC) -c $(CFLAGS) $<

find_quadhuts: find_quadhuts.o layers.o generator.o finders.o
	$(CC) -o $@ $^ $(LDFLAGS)

find_quadhuts.o: find_quadhuts.c
	$(CC) -c $(CFLAGS) $<

god: CFLAGS += -O3 -march=native
god: Gods_seedfinder.o layers.o generator.o finders.o util.o
	$(CC) -o $@ $^ $(LDFLAGS)

Gods_seedfinder.o: Gods_seedfinder.c
	$(CC) -c $(CFLAGS) $<

searcher: CFLAGS += -O3 -march=native
searcher: searcher.o layers.o generator.o finders.o
	$(CC) -o $@ $^ $(LDFLAGS)

searcher.o: searcher.c
	$(CC) -c $(CFLAGS) $<

server: CFLAGS += -O3 -march=native
server: server.o layers.o generator.o finders.o
	$(CC) -o $@ $^ $(LDFLAGS)

server.o: server.c
	$(CC) -c $(CFLAGS) $<

file_verifier: CFLAGS += -O3 -march=native
file_verifier: file_verifier.o layers.o generator.o finders.o
	$(CC) -o $@ $^ $(LDFLAGS)

file_verifier.o: file_verifier.c
	$(CC) -c $(CFLAGS) $<

gen_image.wasm: finders.c layers.c generator.c util.c gen_image.c
	emcc -o $@ -Os $^ -s WASM=1

xmapview.o: xmapview.c xmapview.h
	$(CC) -c $(CFLAGS) $<

finders.o: finders.c finders.h
	$(CC) -c $(CFLAGS) $<

generator.o: generator.c generator.h
	$(CC) -c $(CFLAGS) $<

layers.o: layers.c layers.h
	$(CC) -c $(CFLAGS) $<

util.o: util.c util.h
	$(CC) -c $(CFLAGS) $<

clean:
	$(RM) *.o

