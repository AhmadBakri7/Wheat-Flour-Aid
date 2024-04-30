G = gcc -g
O = -o
names = main plane sky collector splitter occupation functions.o drawer distributor families sorter

files:
	gcc -c functions.c -o functions.o
	$(G) plane.c functions.o $(O) plane
	$(G) collector.c functions.o $(O) collector
	$(G) splitter.c functions.o $(O) splitter
	$(G) occupation.c functions.o $(O) occupation
	$(G) sky.c functions.o $(O) sky
	$(G) distributor.c functions.o $(O) distributor
	$(G) families.c functions.o $(O) families
	$(G) sorter.c $(O) sorter -lm
	$(G) main.c -D SLEEP=70 -D DELETE $(O) main
	gcc drawer.c $(O) drawer -lglut -lGLU -lGL -lm

clean:
	rm -f $(names)
	