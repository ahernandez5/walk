CFLAGS = -I ./include
LFLAGS = -lrt -lX11 -lGLU -lGL -pthread -lm #-lXrandr

all: walk

walk: walk.cpp lab3http.cpp ppm.cpp log.cpp
	g++ $(CFLAGS) $(LFLAGS) walk.cpp lab3http.cpp ppm.cpp log.cpp libggfonts.a -Wall -Wextra $(LFLAGS) -owalk -D UNIT_TEST
	

clean:
	rm -f walk

