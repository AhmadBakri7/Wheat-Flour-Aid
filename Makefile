G = gcc -g
O = -o
names = main plane sky collector splitter functions.o

files:
	gcc -c functions.c -o functions.o
	$(G) plane.c functions.o $(O) plane
	$(G) collector.c functions.o $(O) collector
	$(G) splitter.c functions.o $(O) splitter
	$(G) sky.c $(O) sky
	$(G) main.c -D SLEEP=70 -D DELETE $(O) main

clean:
	rm -f $(names)