int checkSpeech(const paRecordData *recordedData, const long frameIdx) {
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
    // fprintf(stderr, "currentEnergy: %f\n", currentEnergy);
    
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

float frameEnergy(const paRecordData *recordData, const long frameIdx, const int frameSpan) {
    long startFrame;
    long endFrame;
    float energy;
    long i;

    startFrame = frameIdx - frameSpan;
    if (startFrame < 0) {
        startFrame = 0;
    }
    endFrame = frameIdx;

    energy = 0.0;
    for (i = startFrame; i < endFrame; i++) {
        energy += (recordData->recordedSamples[i] * recordData->recordedSamples[i]);
    }
    energy = sqrt(energy) / (float) (endFrame - startFrame);

    return energy;
}

}
}
}
