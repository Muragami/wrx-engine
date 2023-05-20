CC = clang
CFLAGS = -std=c99 -m64 -Wall
OPTFLAGS ?= -o3
OBJS = ./obj/
SRCS = ./src/

WLIBS = -lws2_32 -lopengl32 -lgdi32 -llua -ljson-c -lphysfs -lportaudio
MLIBS = -framework OpenGL -framework Cocoa -llua -ljson-c
LLIBS = -lpthread -lGLU -lGL -lX11 -llua -ljson-c

windows: $(OBJS)wwrx.exe

$(OBJS)wwrx.exe: $(OBJS)main.w.o $(OBJS)tigr.w.o $(OBJS)data.w.o $(OBJS)core.w.o $(OBJS)lua.w.o $(OBJS)xthread.w.o $(OBJS)socket.w.o
	clang $(CFLAGS) $(OPTFLAGS) -o $(OBJS)wwrx.exe $(OBJS)main.w.o $(OBJS)tigr.w.o $(OBJS)data.w.o $(OBJS)core.w.o \
	$(OBJS)lua.w.o $(OBJS)xthread.w.o $(OBJS)socket.w.o $(WLIBS)

$(OBJS)main.w.o: $(SRCS)main.c
	$(CC) $(CFLAGS) $(OPTFLAGS) -c $(SRCS)main.c -o $(OBJS)main.w.o

$(OBJS)tigr.w.o: $(SRCS)tigr.c
	$(CC) $(CFLAGS) $(OPTFLAGS) -c $(SRCS)tigr.c -o $(OBJS)tigr.w.o

$(OBJS)data.w.o: $(SRCS)data.c
	$(CC) $(CFLAGS) $(OPTFLAGS) -c $(SRCS)data.c -o $(OBJS)data.w.o	

$(OBJS)core.w.o: $(SRCS)core.c
	$(CC) $(CFLAGS) $(OPTFLAGS) -c $(SRCS)core.c -o $(OBJS)core.w.o	

$(OBJS)lua.w.o: $(SRCS)lua.c
	$(CC) $(CFLAGS) $(OPTFLAGS) -c $(SRCS)lua.c -o $(OBJS)lua.w.o	

$(OBJS)xthread.w.o: $(SRCS)xthread.c
	$(CC) $(CFLAGS) $(OPTFLAGS) -c $(SRCS)xthread.c -o $(OBJS)xthread.w.o	

$(OBJS)socket.w.o: $(SRCS)socket.c
	$(CC) $(CFLAGS) $(OPTFLAGS) -c $(SRCS)socket.c -o $(OBJS)socket.w.o

macos: $(OBJS)mwrx

$(OBJS)mwrx: $(OBJS)main.m.o $(OBJS)tigr.m.o $(OBJS)data.m.o $(OBJS)core.m.o $(OBJS)lua.m.o $(OBJS)xthread.m.o $(OBJS)socket.m.o
	clang $(CFLAGS) $(OPTFLAGS) -o $(OBJS)mwrx $(OBJS)main.m.o $(OBJS)tigr.m.o $(OBJS)data.m.o $(OBJS)core.m.o \
	$(OBJS)lua.m.o $(OBJS)xthread.m.o $(OBJS)socket.m.o $(MLIBS)

$(OBJS)main.m.o: $(SRCS)main.c
	$(CC) $(CFLAGS) $(OPTFLAGS) -c $(SRCS)main.c -o $(OBJS)main.m.o

$(OBJS)tigr.m.o: $(SRCS)tigr.c
	$(CC) $(CFLAGS) $(OPTFLAGS) -c $(SRCS)tigr.c -o $(OBJS)tigr.m.o

$(OBJS)data.m.o: $(SRCS)data.c
	$(CC) $(CFLAGS) $(OPTFLAGS) -c $(SRCS)data.c -o $(OBJS)data.m.o	

$(OBJS)core.m.o: $(SRCS)core.c
	$(CC) $(CFLAGS) $(OPTFLAGS) -c $(SRCS)core.c -o $(OBJS)core.m.o	

$(OBJS)lua.m.o: $(SRCS)lua.c
	$(CC) $(CFLAGS) $(OPTFLAGS) -c $(SRCS)lua.c -o $(OBJS)lua.m.o	

$(OBJS)xthread.m.o: $(SRCS)xthread.c
	$(CC) $(CFLAGS) $(OPTFLAGS) -c $(SRCS)xthread.c -o $(OBJS)xthread.m.o

$(OBJS)socket.m.o: $(SRCS)socket.c
	$(CC) $(CFLAGS) $(OPTFLAGS) -c $(SRCS)socket.c -o $(OBJS)socket.m.o

clean:
	rm $(OBJS)*
