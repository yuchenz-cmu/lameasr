#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>
#include <math.h>
#include "portaudio.h"

#define SAMPLE_RATE             (16000)
#define FRAMES_PER_BUFFER       (512)
#define BUF_SECONDS             (5)
#define NUM_CHANNELS            (1)
#define SAMPLE_SILENCE          (0)
#define C_DATA_TYPE             short
#define PA_DATA_TYPE            paInt16
#define SPEECH_FORGET_FACTOR    (1.2)
#define SPEECH_ADJUSTMENT       (0.05)
#define SPEECH_THRESHOLD        (10.0)
#define SPEECH_FRAME_SPAN       (10)

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

float frameEnergy(paRecordData *recordData, long frameIdx, int frameSpan) {
    long startFrame;
    long endFrame;
    float energy;
    long i;
    
    // calculates the energy starting frameIdx backwards
    // startFrame = frameIdx - frameSpan;
    // if (startFrame < 0) {
    //     startFrame += recordData->bufLen;
    //     if (startFrame < recordData->startIdx) {
    //         startFrame = recordData->startIdx;
    //     }
    // }
    startFrame = frameIdx - frameSpan;
    if (startFrame < 0) {
        startFrame = 0;
    }
    endFrame = frameIdx;
    
    energy = 0.0;
    // for (i = startFrame; i < endFrame; i++) {
    // while (startFrame != endFrame) {
    //     energy += recordData->recordedSamples[startFrame] ^ 2;
    //     startFrame = (startFrame + 1) % recordData->bufLen;
    //     counter++;
    // }
    // energy = 10 * log10(energy) / (float)counter;

    for (i = startFrame; i < endFrame; i++) {
        energy += recordData->recordedSamples[i] * recordData->recordedSamples[i];
    }
    energy = sqrt(energy) / (float) (endFrame - startFrame);

    return energy;
}

int checkSpeech(paRecordData *recordedData, long frameIdx) {
    static checkSpeechData prevSpeechData;

    int frameOffset = frameIdx - recordedData->startIdx;
    if (frameOffset < 0) {
        frameOffset += recordedData->bufLen;
    }

    // assume first several frames are speech
    if (frameOffset < SPEECH_FRAME_SPAN - 1) {
        // fprintf(stderr, "value: %d\n", recordedData->recordedSamples[frameIdx]);
        return 1;
    } else if (frameOffset == SPEECH_FRAME_SPAN - 1) {
        prevSpeechData.background = frameEnergy(recordedData, frameIdx, SPEECH_FRAME_SPAN);
        // prevSpeechData.level = prevSpeechData.background + 5;
        prevSpeechData.level = recordedData->recordedSamples[frameIdx] * recordedData->recordedSamples[frameIdx];
        // fprintf(stderr, "value: %d, frameIdx: %d, level: %f, background: %f, threshold: %f\n",\
        //     recordedData->recordedSamples[frameIdx], frameIdx, prevSpeechData.level, prevSpeechData.background, SPEECH_THRESHOLD);

        return 1;
    }

    float currentEnergy = frameEnergy(recordedData, frameIdx, SPEECH_FRAME_SPAN);
    fprintf(stderr, "currentEnergy: %f\n", currentEnergy);
    
    prevSpeechData.level = ((prevSpeechData.level * SPEECH_FORGET_FACTOR) + currentEnergy) / (float)(SPEECH_FORGET_FACTOR + 1);
    
    if (currentEnergy < prevSpeechData.background) {
        prevSpeechData.background = currentEnergy;
    } else {
        prevSpeechData.background += (currentEnergy - prevSpeechData.background) * SPEECH_ADJUSTMENT;
    }

    if (prevSpeechData.level < prevSpeechData.background) {
        prevSpeechData.level = prevSpeechData.background;
    }
    
    // fprintf(stderr, "value: %d, frameIdx: %d, level: %f, background: %f, threshold: %f\n",\
    //        recordedData->recordedSamples[frameIdx], frameIdx, prevSpeechData.level, prevSpeechData.background, SPEECH_THRESHOLD);
    return (prevSpeechData.level - prevSpeechData.background > SPEECH_THRESHOLD);
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

    framesRecorded = 0;
    finished = paContinue;

    if(inputBuffer == NULL) {
        while (framesRecorded < framesPerBuffer && (data->endIdx + 1) % data->bufLen != data->startIdx) {
            data->recordedSamples[data->endIdx] = SAMPLE_SILENCE;
            data->endIdx = (data->endIdx + 1) % data->bufLen;
            framesRecorded++;
        }
        finished = paComplete;
        data->isRecording = 0;
    } else {
        while (framesRecorded < framesPerBuffer && (data->endIdx + 1) % data->bufLen != data->startIdx) {
            data->recordedSamples[data->endIdx] = *readPtr++;
            data->endIdx = (data->endIdx + 1) % data->bufLen;
            framesRecorded++;

            if (!checkSpeech(data, data->endIdx - 1)) {
                consecutiveNoSpeech++;
            } else {
                consecutiveNoSpeech = 0;
            }
            fprintf(stderr, "consecutiveNoSpeech: %d\n", consecutiveNoSpeech);

            if (consecutiveNoSpeech > 150) {
                fprintf(stderr, "Detected end-point at frame %d ... \n", data->endIdx);
                data->isRecording = 0;
                finished = paComplete;
                break;
            }
        }
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
            fprintf(stderr, "Wrote %d bytes to 'recorded.raw'\n", writeIdx * sizeof(C_DATA_TYPE));
        }
        if (finished) {
            fprintf(stderr, "We are finished.\n");
            break;
        }
        // usleep(1500000);
        sleep(1);
    }
    pthread_exit(NULL);
}

int patestMain(void) {
    PaError err;
    PaStream *stream;
    paRecordData data;
    // int totalFrames;
    int numSamples;
    int totalBytes;
    int i;
    PaStreamParameters inputParameters;
    int targetDevId = 0;
    float currEnergy = 0.0;
    pthread_t writeThread;
    writeThreadData writeData;

    FILE  *fid;
    fid = fopen("recorded.raw", "ab");
    if( fid == NULL )
    {
        fprintf(stderr, "Could not open file.\n");
        return 3;
    }

    writeData.recordData = &data;
    writeData.fid = fid;
    
    data.startIdx = 0;
    data.endIdx = 0;
    data.bufLen = BUF_SECONDS * SAMPLE_RATE;
    data.isRecording = 1;

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
        getchar();


        // Opens stream
        err = Pa_OpenStream(
                  &stream,
                  &inputParameters,
                  NULL,                  /*  &outputParameters, */
                  SAMPLE_RATE,
                  // FRAMES_PER_BUFFER,
                  paFramesPerBufferUnspecified, 
                  paClipOff,      /*  we won't output out of range samples so don't bother clipping them */
                  recordCallback,
                  &data ); 

        if (err != paNoError) {
            fprintf(stderr, "Error: %s\n", Pa_GetErrorText(err));
        }

        // start the stream
        err = Pa_StartStream( stream );

        fprintf(stderr, "\n=== Now recording!! Please speak into the microphone. ===\n"); fflush(stderr);

        // starts with write thread
        pthread_create(&writeThread, NULL, writeThreadProc, (void *) &writeData);

        // actual recording
        while (1) {
            if (Pa_IsStreamActive(stream) != 1) {
                fprintf(stderr, "Stream not active ... \n");
                break;
            }

            while((err = Pa_IsStreamActive(stream)) == 1)
            {
                Pa_Sleep(500);
                fprintf(stderr, "startIdx = %d, endIdx = %d\n", data.startIdx, data.endIdx ); 
                fflush(stderr);
            }
        }

        fprintf(stderr, "Waiting for write thread to join ... \n");
        pthread_join(writeThread, NULL);
        fprintf(stderr, "Write thread joined ... \n"); 

        fclose( fid );
        err = Pa_StopStream(stream);
        err = Pa_CloseStream(stream);
    }

    free(data.recordedSamples);

    return 0;
}

int main(int argc, char** argv) {
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
    returnValue = patestMain();

    err = Pa_Terminate();
    if (err == paNoError) {
        fprintf(stderr, "PortAudio terminated.\n");
    }

    pthread_mutex_destroy(&recordMutex);
    return returnValue;
}

