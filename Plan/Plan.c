/*
Planning:
    Objetivo:
        - Sistema para atendimento de cllientes em establecimentos médicos (MEDICALso)
        - Feito para unix, devem ser usadas funções especificas para unix (read() e write() em vez de fread() e fwrite())

    Sistema(MEDICALso):
        -cliente
            -interface (consola)
            -tantas instâncias do cliente como utentes
        -balcão 
            - recebe pedidos de atendimento pelo cliente e encaminha para a especialidade que lhes corresponde
            - dados para escolha: 
                .tipo de pedido
                .prioridade
                .etc
            - Apenas 1 instância
        -classificador
            - é o programa que recebe o pedido oriundo dos clientes, encaminhado pelo balcão, e os classifica quanto a especialidade e prioridade, encaminhando-o de volta para o balcão para encaminhamento posterior a um médico da especialidade.
        -médico
            - interface para médicos (consola)
            - tantas instâncias como médicos

    Utilizadores:
    (pode receber o Nº de utente de saúde como identificador)
        Utente:
            -Interge a partir do cliente
        Administrador:
            -Interage a partir do balcão
            - pode consultar info
            - pode expulsar clientes ou médicos
        Especialista:
            - Profissional de saúde

    Programas funcionamento:
        - Cliente:
            -recebe do especialista "adeus" - termina o cliente
            Utilização:
                ./cliente <nome>
                Ex: ./cliente Manuel
        - Médico:
            -recebe do utete "adeus" - termina a interação e informa o balcão que está disponível para outra consulta
            -recebe "sair" - termina o medico
            Utilização:
                ./medico <nome> <especialidade>
                Ex: ./medico Jorge oftalmologia
                (qualquer especialidade é permitida, se não reconhecida, não receberá utentes)
        - Balcão
            - Lançado pelo administrador, começa o sistema MEDICALso
            - Informa o administrador:
                - novo utente (quem)
                - sua especialidade (quem e qual)
                - utente começou a ser atendido (quem e por quem)
                - especialista saiu (quem e qual especialidade)
                - qual a ocupação das filas de espera (para cada especialidade, quantos utentes em espera)
            - updates info cada 30segundos (pode ser alterado pelo administrador)
            Comandos:
                - utentes
                - especialistas
                - delutt X
                - delesp X
                - freq N
                - encerra
            Utilização:
                ./balcao        

    Classificador:
        - dados pelos professores
        - reconhece especialidades
            - oftalmologia
            - neurologia
            - estomatologia
            - ortopedia
            - geral
        - Recebe o seu input pelo stdin e envia o seu output pelo stdout
        -  O input e o output serão sempre texto em minúsculas, não acentuado e sem pontuação
        - grau prioridade 1 a 3
        Utilização:
            Ex input: estomago arde muito
            Ex output: estomatologia 2
    
    Datas de entrega:
    21 de Novembro:
        - fazer estruturas de dados para gerir definições de funcionoamento do cliente médico e serviço
        - definir os .h files com constantes simbolicasque registem os valores por omissão comuns e específicos do cliente e servidor bem como as estruturas de dados relevantes.
        - Implementar a parte do balcão relativa à classificação da especialidade e respetiva prioridade.
        - Implementar no balcão a obtenção dos valores das variáveis de ambiente e concretizar as estruturas de dados do balcão.
        - Na meta intermédia deverá ser entregue um documento (pdf, incluído no ficheiro zip) com duas páginas descrevendo os pormenores da implementação e principais opções tomadas.

    16 de Janeiro:
        - Todos os requisitos expostos no enunciado.
        - makefile que possua os targets de compilação “all” (compilação de todos os programas), 
          “cliente” (compilação do programa cliente), 
          “balcao” (compilação do programa balcão), 
          “medico” (compilação do programa médico) e 
          “clean” (eliminação de todos os ficheiros temporários de apoio à compilação e dos executáveis).
        - Na meta final deverá ser entregue um relatório (pdf, também no zip). 
        O relatório compreenderá o conteúdo que for relevante para justificar o trabalho feito, deverá ser da exclusiva autoria dos membros do grupo. 
        Caso venha a ser divulgado, entretanto um guia de elaboração do relatório, então este deverá seguir as indicações dadas.
*/