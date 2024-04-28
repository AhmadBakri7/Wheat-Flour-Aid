G = gcc -g
O = -o
names = main plane sky collector splitter occupation functions.o

files:
	gcc -c functions.c -o functions.o
	$(G) plane.c functions.o $(O) plane
	$(G) collector.c functions.o $(O) collector
	$(G) splitter.c functions.o $(O) splitter
	$(G) occupation.c functions.o $(O) occupation
	$(G) sky.c functions.o $(O) sky
	$(G) main.c -D SLEEP=70 -D DELETE $(O) main

clean:
	rm -f $(names)