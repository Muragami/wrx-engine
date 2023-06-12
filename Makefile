CC = clang
CCP = clang++
CFLAGS = -std=c99 -m64 -Wall -I./include
CPFLAGS = -m64 -Wall -I./include
OPTFLAGS ?= -o3
IFLAGS = -I/usr/local/Cellar/lua/5.4.6/include/lua5.4/
OBJS = ./obj/
SRCS = ./src/

WLIBS = -lws2_32 -lopengl32 -lgdi32 -llua -ljson-c -lphysfs -lportaudio -lopus -lcairo
MLIBS = -framework OpenGL -framework Cocoa -llua -ljson-c -lphysfs -lopus -lcairo
LLIBS = -lpthread -lGLU -lGL -lX11 -llua -ljson-c -lphysfs -lopus -lcairo

windows: $(OBJS)wwrx.exe

$(OBJS)wwrx.exe: $(OBJS)main.w.o $(OBJS)tigr.w.o $(OBJS)data.w.o $(OBJS)core.w.o $(OBJS)lua.w.o $(OBJS)xthread.w.o $(OBJS)socket.w.o $(OBJS)audio.w.o
	clang $(CFLAGS) $(OPTFLAGS) -o $(OBJS)wwrx.exe $(OBJS)main.w.o $(OBJS)tigr.w.o $(OBJS)data.w.o $(OBJS)core.w.o \
	$(OBJS)lua.w.o $(OBJS)xthread.w.o $(OBJS)socket.w.o $(OBJS)audio.w.o $(WLIBS)

$(OBJS)main.w.o: $(SRCS)main.c
	$(CC) $(CFLAGS) $(OPTFLAGS) -c $(SRCS)main.c -o $(OBJS)main.w.o

$(OBJS)tigr.w.o: $(SRCS)tigr.c
	$(CC) $(CFLAGS) $(OPTFLAGS) -c $(SRCS)tigr.c -o $(OBJS)tigr.w.o

$(OBJS)data.w.o: $(SRCS)data.cpp
	$(CCP) $(CPFLAGS) $(OPTFLAGS) -c $(SRCS)data.cpp -o $(OBJS)data.w.o

$(OBJS)core.w.o: $(SRCS)core.c
	$(CC) $(CFLAGS) $(OPTFLAGS) -c $(SRCS)core.c -o $(OBJS)core.w.o	

$(OBJS)lua.w.o: $(SRCS)lua.c
	$(CC) $(CFLAGS) $(OPTFLAGS) -c $(SRCS)lua.c -o $(OBJS)lua.w.o	

$(OBJS)xthread.w.o: $(SRCS)xthread.c
	$(CC) $(CFLAGS) $(OPTFLAGS) -c $(SRCS)xthread.c -o $(OBJS)xthread.w.o	

$(OBJS)socket.w.o: $(SRCS)socket.c
	$(CC) $(CFLAGS) $(OPTFLAGS) -c $(SRCS)socket.c -o $(OBJS)socket.w.o

$(OBJS)audio.w.o: $(SRCS)audio.c
	$(CC) $(CFLAGS) $(OPTFLAGS) -c $(SRCS)audio.c -o $(OBJS)audio.w.o

macos: $(OBJS)mwrx

$(OBJS)mwrx: $(OBJS)main.m.o $(OBJS)tigr.m.o $(OBJS)data.m.o $(OBJS)core.m.o $(OBJS)lua.m.o $(OBJS)xthread.m.o $(OBJS)socket.m.o $(OBJS)audio.m.o
	clang $(CFLAGS) $(OPTFLAGS) $(IFLAGS) -o $(OBJS)mwrx $(OBJS)main.m.o $(OBJS)tigr.m.o $(OBJS)data.m.o $(OBJS)core.m.o \
	$(OBJS)lua.m.o $(OBJS)xthread.m.o $(OBJS)socket.m.o $(MLIBS)

$(OBJS)main.m.o: $(SRCS)main.c
	$(CC) $(CFLAGS) $(OPTFLAGS) $(IFLAGS) -c $(SRCS)main.c -o $(OBJS)main.m.o

$(OBJS)tigr.m.o: $(SRCS)tigr.c
	$(CC) $(CFLAGS) $(OPTFLAGS) $(IFLAGS) -c $(SRCS)tigr.c -o $(OBJS)tigr.m.o

$(OBJS)data.m.o: $(SRCS)data.cpp
	$(CCP) $(CPFLAGS) $(OPTFLAGS) $(IFLAGS) -c $(SRCS)data.cpp -o $(OBJS)data.m.o	

$(OBJS)core.m.o: $(SRCS)core.c
	$(CC) $(CFLAGS) $(OPTFLAGS) $(IFLAGS) -c $(SRCS)core.c -o $(OBJS)core.m.o	

$(OBJS)lua.m.o: $(SRCS)lua.c
	$(CC) $(CFLAGS) $(OPTFLAGS) $(IFLAGS) -c $(SRCS)lua.c -o $(OBJS)lua.m.o	

$(OBJS)xthread.m.o: $(SRCS)xthread.c
	$(CC) $(CFLAGS) $(OPTFLAGS) $(IFLAGS) -c $(SRCS)xthread.c -o $(OBJS)xthread.m.o

$(OBJS)socket.m.o: $(SRCS)socket.c
	$(CC) $(CFLAGS) $(OPTFLAGS) $(IFLAGS) -c $(SRCS)socket.c -o $(OBJS)socket.m.o

$(OBJS)audio.m.o: $(SRCS)audio.c
	$(CC) $(CFLAGS) $(OPTFLAGS) -c $(SRCS)audio.c -o $(OBJS)audio.m.o

clean:
	rm $(OBJS)*
