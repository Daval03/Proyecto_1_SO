run:
	./Bin/Inicializador "buffer1" 21 5
	./Bin/Emisor "m" "buffer1" 21 10
	
compile:
	gcc Sources/Inicializador.c -o Bin/Inicializador -fsanitize=address -lrt -lpthread
	gcc Sources/Emisor.c -o Bin/Emisor -fsanitize=address -lrt -lpthread
	gcc Sources/Receptor.c -o Bin/Receptor -fsanitize=address -lrt -lpthread 
	gcc Sources/Finalizador.c -o Bin/Finalizador -fsanitize=address -lrt -lpthread

i:
	./Bin/Inicializador "buffer1" 21 80
e:
	./Bin/Emisor "a" "buffer1" 21 10
r:
	./Bin/Receptor "a" "buffer1" 21 1
f:
	./Bin/Finalizador
