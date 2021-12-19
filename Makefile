balcao:
	gcc ./src/balcao.c ./src/utils.c -o ./dist/balcao -lm && ./dist/balcao -D

cliente:
	gcc ./src/cliente.c ./src/utils.c -o ./dist/cliente -lm && ./dist/cliente Paulo -D

medico:
	gcc ./src/medico.c ./src/utils.c -o ./dist/medico -lm && ./dist/medico Paulo estomatologia -D

run:
	./dist/main