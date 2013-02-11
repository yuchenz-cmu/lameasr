wavfile = './recorded_3.wav'
logspecfile='./recorded_3.logspec.visual'
mfccfile='./recorded_3.mfcc.visual'

[d, sr] = wavread(wavfile);
subplot(311)
specgram(d, 256, sr);

logspec_mat = dlmread(logspecfile, ' ');
subplot(312)
imagesc(logspec_mat');
axis xy

mfcc_mat = dlmread(mfccfile, ' ')
subplot(313)
imagesc(mfcc_mat(:,2:13)');
axis xy