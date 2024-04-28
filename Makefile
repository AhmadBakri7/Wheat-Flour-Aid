G = gcc -g
O = -o
names = main plane sky collector splitter functions.o drawer

files:
	gcc -c functions.c -o functions.o
	$(G) plane.c functions.o $(O) plane
	$(G) collector.c functions.o $(O) collector
	$(G) splitter.c functions.o $(O) splitter
	$(G) sky.c $(O) sky
	$(G) main.c -D SLEEP=70 -D DELETE $(O) main
	gcc drawer.c $(O) drawer -lglut -lGLU -lGL -lm

clean:
	rm -f $(names)
	