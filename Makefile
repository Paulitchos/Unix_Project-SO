balcao:
	gcc ./src/balcao.c -o ./dist/balcao -lm && ./dist/balcao

cliente:
	gcc ./src/cliente.c -o ./dist/cliente -lm && ./dist/cliente Paulo

run:
	./dist/main