all:
	gcc src/balcao.c -o dist/main -lm

run:
	./dist/main