all: controller

controller: controller.o feat.o gmm.o hmm.o utils.o
	gcc feat.o gmm.o controller.o hmm.o utils.o -o controller -lm

controller.o: controller.c
	gcc -g -std=c99 -c controller.c -o controller.o

gmm: gmm.o
	gcc gmm.o -o gmm -lm

gmm.o: gmm.c
	gcc -g -std=c99 -c gmm.c -o gmm.o

feat.o: feat.c
	gcc -g -std=c99 -c feat.c -o feat.o

feat: feat.o
	gcc feat.o -o feat

hmm: hmm.o gmm.o utils.o
	gcc hmm.o gmm.o utils.o -o hmm -lm

hmm.o: hmm.c
	gcc -g -std=c99 -c hmm.c -o hmm.o

utils.o: utils.c
	gcc -g -std=c99 -c utils.c -o utils.o

clean: 
	-rm gmm.o
	-rm feat.o
	-rm controller.o
	-rm hmm.o
	-rm gmm 
	-rm feat
	-rm controller
	-rm hmm
