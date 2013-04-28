all: decoder show_align train_continuous edit_hmm

edit_hmm: edit_hmm.o hmm.o feat.o gmm.o topo.o utils.o align.o
	gcc edit_hmm.o hmm.o feat.o gmm.o topo.o utils.o align.o -o edit_hmm -lm

edit_hmm.o: edit_hmm.c
	gcc -g -std=c99 -c edit_hmm.c -o edit_hmm.o

train_continuous: hmm.o topo.o train_continuous.o database.o gmm.o feat.o utils.o
	gcc train_continuous.o hmm.o database.o gmm.o feat.o utils.o topo.o -o train_continuous -lm
	
train_continuous.o: train_continuous.c
	gcc -g -std=c99 -c train_continuous.c -o train_continuous.o

show_align: show_align.o topo.o align.o
	gcc show_align.o topo.o align.o -o show_align -lm

show_align.o: show_align.c
	gcc -g -std=c99 -c show_align.c -o show_align.o

decoder: decoder.o hmm.o feat.o gmm.o topo.o utils.o align.o
	gcc hmm.o feat.o decoder.o gmm.o topo.o utils.o align.o -o decoder -lm

decoder.o: decoder.c
	gcc -g -std=c99 -c decoder.c -o decoder.o

train_single_word: train_single_word.o hmm.o feat.o gmm.o utils.o
	gcc hmm.o feat.o train_single_word.o gmm.o utils.o -o train_single_word -lm

train_single_word.o: train_single_word.c
	gcc -g -std=c99 -c train_single_word.c -o train_single_word.o

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

align.o: align.c
	gcc -g -std=c99 -c align.c -o align.o

hmm: topo.o hmm.o gmm.o utils.o feat.o
	gcc topo.o hmm.o gmm.o utils.o feat.o -o hmm -lm

hmm.o: hmm.c
	gcc -g -std=c99 -c hmm.c -o hmm.o

utils.o: utils.c
	gcc -g -std=c99 -c utils.c -o utils.o

topo: topo.o 
	gcc topo.o -o topo

topo.o: topo.c 
	gcc -g -std=c99 -c topo.c -o topo.o

clean: 
	-rm gmm.o
	-rm feat.o
	-rm controller.o
	-rm hmm.o
	-rm align.o
	-rm train_single_word.o
	-rm show_align.o
	-rm train_continuous.o
	-rm database.o
	-rm utils.o
	-rm edit_hmm.o
	-rm edit_hmm
	-rm gmm 
	-rm feat
	-rm controller
	-rm hmm
	-rm train_single_word
	-rm show_align
	-rm train_continuous
