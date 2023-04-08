run:
	./Bin/Inicializador "buffer1" 21 5
	./Bin/Emisor "Auto" "buffer1" 21	
	
compile:
	gcc Sources/Inicializador.c -o Bin/Inicializador -lrt -lpthread
	gcc Sources/Emisor.c -o Bin/Emisor -lrt -lpthread
	gcc Sources/Receptor.c -o Bin/Receptor -fsanitize=address -lrt -lpthread 
	gcc Sources/Finalizador.c -o Bin/Finalizador -lrt -lpthread

xd:
	./Bin/Receptor "Auto" "buffer1" 21	
	./Bin/Emisor "Auto" "buffer1" 21
	./Bin/Finalizador