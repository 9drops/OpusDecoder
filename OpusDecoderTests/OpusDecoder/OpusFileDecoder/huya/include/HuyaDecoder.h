//
// Created by Administrator on 2017/8/25.
//

#ifndef OPUS_ANDROID_HUYADECODER_H
#define OPUS_ANDROID_HUYADECODER_H

enum {
    ErrReadOpusFile = -1,
    ErrWriteWavFile = -2,
    ErrOpusEncode   = -3,
    ErrNilPrameter  = -4,
    ErrMemMalloc    = -5,
    ErrOpenOpusFile = -6,
    ErrCreatDecoder = -99,
};

#ifdef __cplusplus
extern "C" {
#endif

#include "opus.h"
#include "Constants.h"


// setup the opus begin
#define VOICE_OPUS_BITRATE (64000)
#define VOICE_OPUS_SAMPLE_RATE (48000)
#define VOICE_FRAME_PERIOD	 (OPUS_FRAMESIZE_20_MS)  // see the OPUS_FRAMESIZE_*_MS
#define VOICE_FRAME_PERIOD_IN_MS		(20) // 80 ms
#define VOICE_SIZE_PER_SAMPLE			(2)	// 16 bits = 2*byte
#define VOICE_OPUS_CHANNEL_COUNT (1)  // 1 channel
#define OPUS_COMPARESS_RATION (VOICE_OPUS_SAMPLE_RATE*VOICE_SIZE_PER_SAMPLE*8/VOICE_OPUS_BITRATE)
#define VOICE_OPUS_ENCODED_DATA_SIZE_PER_FRAME	(VOICE_OPUS_BITRATE*VOICE_FRAME_PERIOD_IN_MS/1000/8)  // opus decode in
#define VOICE_OPUS_PCM_DATA_SIZE_PER_FRAME	(VOICE_OPUS_ENCODED_DATA_SIZE_PER_FRAME*OPUS_COMPARESS_RATION)
// setup the opus end

#define VOICE_OPUS_APP (OPUS_APPLICATION_VOIP)
#define VOICE_OPUS_BANDWIDTH (OPUS_BANDWIDTH_WIDEBAND)
#define VOICE_OPUS_USE_VBR  (0)
#define VOICE_OPUS_CONSTRAINT_USE_VBR (0)
#define VOICE_OPUS_COMPLEXITY  (0)
#define VOICE_OPUS_USE_INBANDFEC  (0)
#define VOICE_OPUS_USE_DTX (0)
#define VOICE_OPUS_PACKET_LOSS_PERC (0)

#define VOICE_OPUS_20MS_FRAME_COUNT (1)


int getFramePeriod();
int getSingleFrameInputSize();
int getTotalFrameInputSize();
int getSingleFrameOutputSize();

void setFramePeriod(int framePeriod);
void setSingleFrameInputSize(int frameSize);
void setTotalFrameInputSize(int totalFrameSize);
void setSingleFrameOutputSize(int pcmOutputSize);

void initDecoder(int framePeriod , int singleFrameInputSize , int totalFrameInputSize , int pcmOutputSize);

class HuyaDecoder {
public:
    HuyaDecoder() = default;
    void init();
    int decode(const unsigned char * in, opus_int32 in_len, opus_int16 *out, int frame_size);
    int opus2wav(const char *opusFilePath, const char *outPutWavPath);
    void destory();

private:
    OpusDecoder *opusDecoder;
    int m_channels;
};

#ifdef __cplusplus
}
#endif
#endif //OPUS_ANDROID_HUYADECODER_H
