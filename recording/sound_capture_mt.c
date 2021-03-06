#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>
#include <math.h>
#include <unistd.h>
#include <ncurses.h>
#include "portaudio.h"

#define SAMPLE_RATE             (16000)
#define FRAMES_PER_BUFFER       (512)
#define BUF_SECONDS             (10)
#define NUM_CHANNELS            (1)
#define SAMPLE_SILENCE          (0)
#define C_DATA_TYPE             short
#define PA_DATA_TYPE            paInt16
#define SPEECH_FORGET_FACTOR    (1.2)
#define SPEECH_ADJUSTMENT       (0.05)
#define SPEECH_THRESHOLD        (10.0)
#define SPEECH_FRAME_SPAN       (10)
#define NO_SPEECH_THRESHOLD     (1800)

typedef struct {
    // long          frameIndex;  /*  Index into sample array. */
    long          startIdx;
    long          endIdx;
    long          bufLen;
    short         isRecording;
    C_DATA_TYPE  *recordedSamples;
} paRecordData;

typedef struct {
    paRecordData *recordData;
    FILE *fid;
} writeThreadData;

typedef struct {
    float level;
    float background;
} checkSpeechData;

pthread_mutex_t recordMutex;
int energyThreshold;

/* 
 * non-blocking call, checks if a key is pressed
 */
int kbhit(void) {

    initscr();
    cbreak();
    noecho();
    nodelay(stdscr, TRUE);
    scrollok(stdscr, TRUE);
    // refresh();
    // keypad(stdscr, TRUE);

    int ch = getch();

    if (ch != ERR) {
        // ungetch(ch);
        refresh();
        endwin();
        return 1;
    } else {
        refresh();
        endwin();
        return 0;
    }
}

int recordCallback( const void *inputBuffer, void *outputBuffer,
                           unsigned long framesPerBuffer,
                           const PaStreamCallbackTimeInfo* timeInfo,
                           PaStreamCallbackFlags statusFlags,
                           void *userData ) {

    // lock the mutex so the write thread won't change anything
    pthread_mutex_lock(&recordMutex);

    paRecordData *data = (paRecordData *)userData;

    const C_DATA_TYPE *readPtr = (const C_DATA_TYPE*)inputBuffer;
    C_DATA_TYPE *writePtr = &data->recordedSamples[data->startIdx];
    long framesRecorded;
    long i;
    int finished;
    static int consecutiveNoSpeech = 0;
    float currentEnergy;

    framesRecorded = 0;
    finished = paContinue;

    if(inputBuffer == NULL) {
        while (framesRecorded < framesPerBuffer && (data->endIdx + 1) % data->bufLen != data->startIdx) {
            data->recordedSamples[data->endIdx] = SAMPLE_SILENCE;
            data->endIdx = (data->endIdx + 1) % data->bufLen;
            framesRecorded++;
        }
        data->isRecording = 0;
    } else {
        while (framesRecorded < framesPerBuffer && (data->endIdx + 1) % data->bufLen != data->startIdx) {
            data->recordedSamples[data->endIdx] = *readPtr++;
            data->endIdx = (data->endIdx + 1) % data->bufLen;
            framesRecorded++;

            currentEnergy = abs(data->recordedSamples[data->endIdx - 1]);

            if (currentEnergy < energyThreshold) { 
                consecutiveNoSpeech++;
            } else {
                consecutiveNoSpeech = 0;
            }
            // fprintf(stderr, "consecutiveNoSpeech: %d\n", consecutiveNoSpeech);

            if (consecutiveNoSpeech > NO_SPEECH_THRESHOLD) {
                fprintf(stderr, "Detected end-point at frame %d ... \n", data->endIdx);
                data->isRecording = 0;
                consecutiveNoSpeech = 0;
                // finished = paComplete;
                break;
            }
        }
    }
    if (!data->isRecording) {
        finished = paComplete;
    }

    pthread_mutex_unlock(&recordMutex);
    return finished;
}

int patestCheckDevice(void) {
    int numDevices;
    int i = 0;
    const PaDeviceInfo *deviceInfo;
    int pulseaudioIdx = 0;
    
    numDevices = Pa_GetDeviceCount();
    if (numDevices < 0) {
        fprintf(stderr, "Error getting devices: 0x%x\n", numDevices);
        return 3;
    }
    for (i = 0; i < numDevices; i++) {
        deviceInfo = Pa_GetDeviceInfo(i);
        fprintf(stderr, "Device %d: %s\n", i, deviceInfo->name);
        if (strcmp(deviceInfo->name, "dmix") == 0) {
            pulseaudioIdx = i;
        }
    }
    pulseaudioIdx = 5;
    return pulseaudioIdx;
}

void *writeThreadProc(void *ptr) {
    C_DATA_TYPE writeBuf[BUF_SECONDS * SAMPLE_RATE * sizeof(C_DATA_TYPE)];
    long writeIdx;
    writeThreadData *writeData = (writeThreadData *) ptr;
    paRecordData *recordData = writeData->recordData;
    FILE *fid = writeData->fid;
    short finished;

    while (1) {
        writeIdx = 0; 
        finished = 0;

        pthread_mutex_lock(&recordMutex);

        while (recordData->startIdx != recordData->endIdx) {
            writeBuf[writeIdx++] = recordData->recordedSamples[recordData->startIdx];
            recordData->startIdx = (recordData->startIdx + 1) % recordData->bufLen;
        }
        finished = !recordData->isRecording;

        pthread_mutex_unlock(&recordMutex);

        // write the recorded audio to file
        if (writeIdx > 0) {
            fwrite(writeBuf, sizeof(C_DATA_TYPE), writeIdx, fid);
            fprintf(stderr, "Wrote %d bytes to disk.\n", writeIdx * sizeof(C_DATA_TYPE));
        }
        if (finished) {
            fprintf(stderr, "We are finished.\n");
            break;
        }
        // usleep(1500000);
        sleep(2);
    }
    pthread_exit(NULL);
}

int recordAudio(char *sphinxfe_bin, char *sphinxcepview_bin, int energyThreshold) {
    PaError err;
    PaStream *stream = NULL;
    paRecordData data;
    int numSamples = BUF_SECONDS * SAMPLE_RATE;
    int totalBytes = 0;
    int i = 0;
    PaStreamParameters inputParameters;
    int targetDevId = 0;
    float currEnergy = 0.0;
    char recordFilename[32];
    char wavFilename[32];
    char mfccFilename[32];
    int recordIdx = 0;
    pthread_t writeThread;
    writeThreadData writeData;
    FILE  *fid = NULL;
    char cmdBuf[256];
    long secondsRecorded = 0;
    
    data.recordedSamples = (C_DATA_TYPE *) malloc (numSamples * sizeof(C_DATA_TYPE));
    for (i = 0; i < numSamples; i++) {
        data.recordedSamples[i] = 0;
    }

    // check device
    // targetDevId = patestCheckDevice();
    targetDevId = Pa_GetDefaultInputDevice();
    fprintf(stderr, "Using device ID %d\n", targetDevId);

    // preparing the parameters
    inputParameters.device = targetDevId;
    inputParameters.channelCount = NUM_CHANNELS;
    inputParameters.sampleFormat = PA_DATA_TYPE;
    inputParameters.suggestedLatency = Pa_GetDeviceInfo( inputParameters.device )->defaultLowInputLatency;
    inputParameters.hostApiSpecificStreamInfo = NULL;

    while (1) {
        // enable "Hit-to-talk"
        printf("Press [Enter] to start recording ... \n");
        printf("Hold [Space] to stop recording.\n");
        getchar();

        // prepare filename
        sprintf(recordFilename, "recorded_%d.raw", recordIdx);
        
        fid = fopen(recordFilename, "wb");
        if(fid == NULL) {
            fprintf(stderr, "Could not open file.\n");
            return 3;
        }

        data.startIdx = 0;
        data.endIdx = 0;
        data.bufLen = numSamples;
        data.isRecording = 1;
        secondsRecorded = 0;

        writeData.recordData = &data;
        writeData.fid = fid;

        // Opens stream
        err = Pa_OpenStream(
                  &stream,
                  &inputParameters,
                  NULL,                  /*  &outputParameters, */
                  SAMPLE_RATE,
                  paFramesPerBufferUnspecified, 
                  paClipOff,      /*  we won't output out of range samples so don't bother clipping them */
                  recordCallback,
                  &data ); 

        if (err != paNoError) {
            fprintf(stderr, "Error: %s\n", Pa_GetErrorText(err));
        }

        // start the stream
        err = Pa_StartStream(stream);

        if (err != paNoError) {
            fprintf(stderr, "Error: %s\n", Pa_GetErrorText(err));
        }

        fprintf(stderr, "\n=== Now recording!! Please speak into the microphone. ===\n"); fflush(stderr);

        // starts with write thread
        pthread_create(&writeThread, NULL, writeThreadProc, (void *) &writeData);

        if (Pa_IsStreamActive(stream) != 1) {
            fprintf(stderr, "Stream not active ... \n");
            break;
        }

        // actual recording
        while((err = Pa_IsStreamActive(stream)) == 1)
        {
            Pa_Sleep(500);
            fprintf(stderr, "startIdx = %d, endIdx = %d, second: %d\n", data.startIdx, data.endIdx, secondsRecorded); 
            fflush(stderr);
              
            /* 
            if (secondsRecorded > 30) {
                pthread_mutex_lock(&recordMutex);
                data.isRecording = 0;
                pthread_mutex_unlock(&recordMutex);
            }
            */

            if (kbhit()) {
                fprintf(stderr, "Key pressed, stop recording ... %d\n", getch());

                pthread_mutex_lock(&recordMutex);
                data.isRecording = 0;
                pthread_mutex_unlock(&recordMutex);
            }

            secondsRecorded++;
        }

        fprintf(stderr, "Waiting for write thread to join ... \n");
        pthread_join(writeThread, NULL);
        fprintf(stderr, "Write thread joined ... \n"); 

        fclose(fid);
        err = Pa_StopStream(stream);
        err = Pa_CloseStream(stream);

        // call program to do wave and MFCC conversion as needed

        sprintf(wavFilename, "recorded_%d.wav", recordIdx);
        fprintf(stderr, "Converting %s to wave format ... ", recordFilename);
        sprintf(cmdBuf, "/usr/bin/sox -t raw -b 16 -r 16000 -e signed-integer -c 1 %s %s", recordFilename, wavFilename);
        system(cmdBuf);
        fprintf(stderr, "done.\n");

        if (sphinxfe_bin != NULL) {
            fprintf(stderr, "Computing MFCC features ... ");
            sprintf(cmdBuf, "%s -i recorded_%d.wav -o recorded_%d.mfcc -mswav yes", sphinxfe_bin, recordIdx, recordIdx);
            system(cmdBuf);
            sprintf(cmdBuf, "%s -i recorded_%d.wav -o recorded_%d_nfilter25.mfcc -mswav yes -nfilt 25", sphinxfe_bin, recordIdx, recordIdx);
            system(cmdBuf);
            sprintf(cmdBuf, "%s -f recorded_%d.mfcc -d 13 | sed \"s/^ *//;s/ *$//;s/  */ /g\" > recorded_%d.mfcc.visual", sphinxcepview_bin, recordIdx, recordIdx);
            system(cmdBuf);
            sprintf(cmdBuf, "%s -f recorded_%d_nfilter25.mfcc -d 13 | sed \"s/^ *//;s/ *$//;s/  */ /g\" > recorded_%d_nfilter25.mfcc.visual", sphinxcepview_bin, recordIdx, recordIdx);
            system(cmdBuf);

            fprintf(stderr, "done.\n");

            fprintf(stderr, "Computing Logspec features ... ");
            sprintf(cmdBuf, "%s -mswav yes -i recorded_%d.wav -o recorded_%d.logspec -logspec yes", sphinxfe_bin, recordIdx, recordIdx);
            system(cmdBuf);
            sprintf(cmdBuf, "%s -f recorded_%d.logspec -d 40 -i 40 | sed \"s/^ *//;s/ *$//;s/  */ /g\" > recorded_%d.logspec.visual", sphinxcepview_bin, recordIdx, recordIdx);
            system(cmdBuf);
            fprintf(stderr, "done.\n");
        }

        recordIdx++;
    }

    free(data.recordedSamples);

    return 0;
}

void printUsage() {
    fprintf(stderr, "Usage: ./sound_capture_mt --sphinx-bin ./sphinx_fe --threshold 1500\n");
}

int main(int argc, char** argv) {
    char *sphinxfe_bin = NULL;
    char *sphinxcepview_bin = NULL;
    energyThreshold = 1400;
    int i;

    // initialize ncurses
    // initscr();
    // cbreak();
    // noecho();
    // nodelay(stdscr, TRUE);
    // scrollok(stdscr, TRUE);
    // refresh();
    // keypad(stdscr, TRUE);

    i = 1;
    while (i < argc) {
        if (!strcmp(argv[i], "--sphinx-bin")) {
            if (i + 2 >= argc) {
                fprintf(stderr, "Must specify sphinx_fe's location.\n");
                printUsage();
                return 4;
            }
            sphinxfe_bin = argv[i + 1];
            sphinxcepview_bin = argv[i + 2];
            fprintf(stderr, "Using sphinx_fe binary from %s, %s ... \n", sphinxfe_bin, sphinxcepview_bin);
            i += 3;
        } else if (!strcmp(argv[i], "--threshold")) {
            if (i + 1 >= argc) {
                fprintf(stderr, "Must specify the threshold.\n");
                printUsage();
                return 5;
            }
            energyThreshold = atoi(argv[i + 1]);
            i += 2;
        } else if (!strcmp(argv[i], "--help")) {
            printUsage();
            return 0;
        } else {
            fprintf(stderr, "Invalid argument: %s\n", argv[i]);
            printUsage();
            return 6;
        }
    }
    fprintf(stderr, "Using threshold: %d\n", energyThreshold);

    pthread_mutex_init(&recordMutex, NULL);

    fprintf(stderr, "Initializing PortAudio ... \n");
    int err = Pa_Initialize();
    int returnValue = 0;

    if (err != paNoError) {
        fprintf(stderr, "Can't create PortAudio ... error msg: %s\n", Pa_GetErrorText(err)); 
    } else {
        fprintf(stderr, "PortAudio initialized ... \n");
    }

    // Here we go ... 
    returnValue = recordAudio(sphinxfe_bin, sphinxcepview_bin, energyThreshold);

    err = Pa_Terminate();
    if (err == paNoError) {
        fprintf(stderr, "PortAudio terminated.\n");
    }

    // endwin();
    pthread_mutex_destroy(&recordMutex);
    return returnValue;
}

