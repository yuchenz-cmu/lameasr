all: sound_capture sound_capture_mt

sound_capture_mt.o: sound_capture_mt.c
	gcc -g -c sound_capture_mt.c -o sound_capture_mt.o

sound_capture_mt: sound_capture_mt.o
	gcc sound_capture_mt.o -lportaudio -lrt -lasound -lpthread -o sound_capture_mt

sound_capture.o: sound_capture.c
	gcc -g -c sound_capture.c -o sound_capture.o

sound_capture: sound_capture.o
	gcc sound_capture.o -lportaudio -lrt -lasound -lpthread -o sound_capture

clean: 
	-rm sound_capture.o
	-rm sound_capture
	-rm sound_capture_mt.o
	-rm sound_capture_mt
