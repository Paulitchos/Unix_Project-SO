all:
	@gcc ./src/balcao.c ./src/utils.c -o ./dist/balcao  -lm -pthread
	@gcc ./src/cliente.c ./src/utils.c -o ./dist/cliente -lm -pthread
	@gcc ./src/medico.c ./src/utils.c -o ./dist/medico -lm -pthread

balcao:
	@gcc ./src/balcao.c ./src/utils.c -o ./dist/balcao  -lm -pthread

cliente:
	@gcc ./src/cliente.c ./src/utils.c -o ./dist/cliente -lm -pthread

medico:
	@gcc ./src/medico.c ./src/utils.c -o ./dist/medico -lm -pthread

clean:
	rm ./dist/*

runbalcaoD:
	gcc ./src/balcao.c ./src/utils.c -o ./dist/balcao  -lm -pthread && ./dist/balcao -D # -Wall -Werror -Wextra

runclienteD:
	gcc ./src/cliente.c ./src/utils.c -o ./dist/cliente -lm -pthread && ./dist/cliente Paulo -D

runmedicoD:
	gcc ./src/medico.c ./src/utils.c -o ./dist/medico -lm -pthread && ./dist/medico Paulo estomatologia -D