balcao:
	gcc ./src/balcao.c ./src/utils.c -o ./dist/balcao -lm && ./dist/balcao -D

cliente:
	gcc ./src/cliente.c ./src/utils.c -o ./dist/cliente -lm && ./dist/cliente Paulo -D

run:
	./dist/main