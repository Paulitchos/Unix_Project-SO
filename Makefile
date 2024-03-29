all:
	@gcc ./src/balcao.c ./src/utils.c -o ./dist/balcao  -lm -pthread -w
	@gcc ./src/cliente.c ./src/utils.c -o ./dist/cliente -lm -pthread -w
	@gcc ./src/medico.c ./src/utils.c -o ./dist/medico -lm -pthread -w

balcao:
	@gcc ./src/balcao.c ./src/utils.c -o ./dist/balcao  -lm -pthread -w

cliente:
	@gcc ./src/cliente.c ./src/utils.c -o ./dist/cliente -lm -pthread -w

medico:
	@gcc ./src/medico.c ./src/utils.c -o ./dist/medico -lm -pthread -w

clean:
	@rm ./dist/*

runbalcaoD:
	gcc ./src/balcao.c ./src/utils.c -o ./dist/balcao  -lm -pthread && ./dist/balcao -D # -Wall -Werror -Wextra

runclienteD:
	gcc ./src/cliente.c ./src/utils.c -o ./dist/cliente -lm -pthread && ./dist/cliente Paulo -D

runmedicoD:
	gcc ./src/medico.c ./src/utils.c -o ./dist/medico -lm -pthread && ./dist/medico Paulo estomatologia -D