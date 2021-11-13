all:
	gcc src/*.c -o dist/main -lm

run:
	./dist/main