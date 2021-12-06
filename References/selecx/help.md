
Exit by typing "sair"
Will print (Estou a espera....) every 30 seconds



Run with `make all` or `gcc selecx.c -o /tmp/selecx && /tmp/selecx`  
Selecx will be watching three different sources: pipe_a, pipe_b and user input.
You can send it info from any of them:  
- Through user input by simply typing in the same terminal selecx was run  
- Through pipe_a/b by sending info to the pipe, Ex: `echo hello > pipe_a`