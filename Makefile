balcao:
	gcc ./src/balcao.c ./src/utils.c -o ./dist/balcao  -lm -pthread && ./dist/balcao -D # -Wall -Werror -Wextra

cliente:
	gcc ./src/cliente.c ./src/utils.c -o ./dist/cliente -lm -pthread && ./dist/cliente Paulo -D

medico:
	gcc ./src/medico.c ./src/utils.c -o ./dist/medico -lm -pthread && ./dist/medico Paulo estomatologia -D

runcliente:
	./dist/cliente Paulo -D

runmedico:
	./dist/medico Paulo estomatologia -D