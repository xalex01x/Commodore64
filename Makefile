all:
	g++ -I../src/include -L../src/lib -o a main.cpp -lmingw32 -lSDL2main -lSDL2
