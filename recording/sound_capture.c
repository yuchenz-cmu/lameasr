#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "portaudio.h"

#define SAMPLE_RATE (16000)
#define FRAMES_PER_BUFFER (512)
#define BUF_SECONDS     (1)
#define NUM_CHANNELS    (1)
#define SAMPLE_SILENCE  (0)

#define C_DATA_TYPE     short
#define PA_DATA_TYPE    paInt16

typedef struct
{
    long          frameIndex;  /*  Index into sample array. */
    long          maxFrameIndex;
    C_DATA_TYPE  *recordedSamples;
} 
paTestData;

float frameEnergy(C_DATA_TYPE *samples, long sampleLen, long frameIdx, int frameSpan) {
    long startFrame;
    long endFrame;
    float energy;
    long i;
    
    startFrame = frameIdx - (frameSpan / 2);
    endFrame = frameIdx + (frameSpan / 2);
    
    if (startFrame < 0) {
        startFrame = 0;
    }
    if (endFrame > sampleLen) {
        endFrame = sampleLen;
    }
    
    energy = 0.0;
    for (i = startFrame; i < endFrame; i++) {
        energy += samples[i];
    }
    energy = 10 * log10(energy);

    return energy;
}

int recordCallback( const void *inputBuffer, void *outputBuffer,
                           unsigned long framesPerBuffer,
                           const PaStreamCallbackTimeInfo* timeInfo,
                           PaStreamCallbackFlags statusFlags,
                           void *userData ) {

    paTestData *data = (paTestData *)userData;

    const C_DATA_TYPE *readPtr = (const C_DATA_TYPE*)inputBuffer;
    C_DATA_TYPE *writePtr = &data->recordedSamples[data->frameIndex];
    unsigned long framesLeft = data->maxFrameIndex - data->frameIndex;
    long framesToRecord;
    long i;
    int finished;

    framesToRecord = framesPerBuffer;
    finished = paContinue;

    // not enough buffer left, grow
    if(framesLeft < framesPerBuffer) {
        framesToRecord = framesLeft;
        // finished = paComplete;
    }

    if(inputBuffer == NULL)
    {
        for(i = 0; i < framesPerBuffer; i++)
        {
            *writePtr++ = SAMPLE_SILENCE;  // left
        }
    }
    else
    {
        for(i = 0; i < framesPerBuffer; i++)
        {
            *writePtr++ = *readPtr++;  // left
        }
    }
    data->frameIndex += framesToRecord;
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

int patestMain(void) {
    PaError err;
    PaStream *stream;
    paTestData data;
    // int totalFrames;
    int numSamples;
    int totalBytes;
    int i;
    PaStreamParameters inputParameters;
    int targetDevId = 0;
    float currEnergy = 0.0;

    FILE  *fid;
    fid = fopen("recorded.raw", "ab");
    if( fid == NULL )
    {
        fprintf(stderr, "Could not open file.\n");
        return 3;
    }

    // preparing the buffer
    data.maxFrameIndex = numSamples = BUF_SECONDS * SAMPLE_RATE;
    // totalBytes = numSamples * sizeof(C_DATA_TYPE);
    
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
    fprintf(stderr, "\n=== Now recording!! Please speak into the microphone. ===\n"); fflush(stderr);


    while (1) {
        data.frameIndex = 0;

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

        if (Pa_IsStreamActive(stream) != 1) {
            fprintf(stderr, "Stream not active ... \n");
        }

        while((err = Pa_IsStreamActive(stream)) == 1)
        {
            Pa_Sleep(100);
            fprintf(stderr, "index = %d\n", data.frameIndex ); fflush(stderr);
        }

        // write the recorded audio to file
        fwrite(data.recordedSamples, sizeof(C_DATA_TYPE), data.maxFrameIndex, fid);
        fprintf(stderr, "Wrote %d bytes to 'recorded.raw'\n", data.maxFrameIndex * sizeof(C_DATA_TYPE));

        // compute the energy of each frame
        // for (i = 0; i < numSamples; i+=100) {
            // float frameEnergy(C_DATA_TYPE *samples, long sampleLen, long frameIdx, int frameSpan) {
            // currEnergy = frameEnergy(data.recordedSamples, numSamples, i, 10);
            // fprintf(stderr, "Energy at frame %d: %f\n", i, currEnergy);
        // }
    }

    fclose( fid );
    err = Pa_StopStream(stream);
    err = Pa_CloseStream(stream);
    free(data.recordedSamples);

    return 0;
}

int main(int argc, char** argv) {
    printf("Hello World!\n");

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

    return returnValue;
}

