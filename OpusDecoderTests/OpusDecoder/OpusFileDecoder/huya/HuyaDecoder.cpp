//
// Created by Administrator on 2017/8/25.
//

#include "include/HuyaDecoder.h"
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include <stdio.h>

extern "C" {


int mFramePeriod = -1 ;
int mSingleFrameInputSize = -1 ;
int mTotalFrameInputSize = -1 ;
int mSingnleFrameOutputSize = -1 ;

int getFramePeriod(){
    if(mFramePeriod == -1){
        mFramePeriod = VOICE_FRAME_PERIOD ;
    }
    return mFramePeriod ;
}
int getSingleFrameInputSize(){
    if(mSingleFrameInputSize == -1){
        mSingleFrameInputSize = VOICE_OPUS_ENCODED_DATA_SIZE_PER_FRAME ;
    }
    return mSingleFrameInputSize ;
}
int getTotalFrameInputSize(){
    if(mTotalFrameInputSize == -1){
        mTotalFrameInputSize = VOICE_OPUS_ENCODED_DATA_SIZE_PER_FRAME * VOICE_OPUS_20MS_FRAME_COUNT;
    }
    return mTotalFrameInputSize ;
}
int getSingleFrameOutputSize(){
    if(mSingnleFrameOutputSize == -1){
        mSingnleFrameOutputSize = VOICE_OPUS_PCM_DATA_SIZE_PER_FRAME ;
    }
    return mSingnleFrameOutputSize ;
}

void setFramePeriod(int framePeriod){
    mFramePeriod = framePeriod ;
}
void setSingleFrameInputSize(int frameSize){
    mSingleFrameInputSize = frameSize ;
}
void setTotalFrameInputSize(int totalFrameSize){
    mTotalFrameInputSize = totalFrameSize ;
}
void setSingleFrameOutputSize(int pcmOutputSize){
    mSingnleFrameOutputSize = pcmOutputSize ;
}

void initDecoder(int framePeriod , int singleFrameInputSize , int totalFrameInputSize , int pcmOutputSize){
    if(framePeriod == 0 || singleFrameInputSize == 0 || totalFrameInputSize == 0 || pcmOutputSize == 0){
//        __android_log_print(ANDROID_LOG_ERROR, "HuyaDecoder", "init_error framePeriod == 0 || singleFrameInputSize == 0 || totalFrameInputSize == 0 || pcmOutputSize == 0");
    }
    setFramePeriod(framePeriod);
    setSingleFrameInputSize(singleFrameInputSize);
    setTotalFrameInputSize(totalFrameInputSize);
    setSingleFrameOutputSize(pcmOutputSize);
}

static void put_le32(unsigned char *dst,opus_uint32 _x){
  dst[0]=(unsigned char)(_x&0xFF);
  dst[1]=(unsigned char)(_x>>8&0xFF);
  dst[2]=(unsigned char)(_x>>16&0xFF);
  dst[3]=(unsigned char)(_x>>24&0xFF);
}

/*Make a header for a 48 kHz, stereo, signed, 16-bit little-endian PCM WAV.*/
static void make_wav_header(unsigned char dst[44], int64_t dataSize){
  /*The chunk sizes are set to 0x7FFFFFFF by default.
    Many, though not all, programs will interpret this to mean the duration is
     "undefined", and continue to read from the file so long as there is actual
     data.*/
  static const unsigned char WAV_HEADER_TEMPLATE[44]={
    'R','I','F','F',0xFF,0xFF,0xFF,0x7F,
    'W','A','V','E','f','m','t',' ',
    0x10,0x00,0x00,0x00,0x01,0x00,0x02,0x00,
    0x80,0xBB,0x00,0x00,0x00,0xEE,0x02,0x00,
    0x04,0x00,0x10,0x00,'d','a','t','a',
    0xFF,0xFF,0xFF,0x7F
  };

  memcpy(dst,WAV_HEADER_TEMPLATE,sizeof(WAV_HEADER_TEMPLATE));
  if(dataSize > 0){
    if(dataSize > 0x1FFFFFF6) {
      printf("WARNING: WAV output would be larger than 2 GB.\n");
      printf("Writing non-standard WAV header with invalid chunk sizes.\n");
    } else {
      opus_uint32 audio_size;
      audio_size=(opus_uint32)dataSize;//(_duration * 0.001 * 4800 * 2 * 16 / 8);
      put_le32(dst+4,audio_size+36);
      put_le32(dst+40,audio_size);
    }
  }
}

int readFile(const char *inPath, char **buf, size_t *bufSize) {
    int ret = 0;
    if (NULL == inPath || NULL == buf || NULL == bufSize) {
        return ErrNilPrameter;
    }
    
    int fd = open(inPath, O_RDONLY);
    if (fd <= 0) {
        return ErrOpenOpusFile;
    }
    
    struct stat st;
    ret = fstat(fd, &st);
    if (ret) {
        ret = ErrReadOpusFile;
        goto END;
    }
    
    {
        size_t fileSize = st.st_size;
        *buf = (char *)malloc(fileSize);
        *bufSize = read(fd, *buf, fileSize);
        ret = *bufSize != fileSize;
    }
    
END:
    close(fd);
    return ret;
}

int writeFile(const char *path, char *buf, size_t bufSize) {
    if (NULL == path || NULL == buf || 0 == bufSize) {
        return ErrNilPrameter;
    }
    
    int fd = open(path, O_RDWR | O_CREAT | O_APPEND | O_TRUNC, 0644);
    if (fd <= 0) {
        return ErrWriteWavFile;
    }
    
    unsigned char dst[44];
    make_wav_header(dst, bufSize);
    size_t written = write(fd, dst, sizeof(dst));
    written += write(fd, buf, bufSize);
    close(fd);
    return bufSize + sizeof(dst)  != written;
}

void HuyaDecoder::init() {
//    __android_log_print(ANDROID_LOG_ERROR, "HuyaDecoder", "start");
    int error;
    m_channels = VOICE_OPUS_CHANNEL_COUNT;
    opusDecoder = opus_decoder_create(VOICE_OPUS_SAMPLE_RATE, VOICE_OPUS_CHANNEL_COUNT, &error);

    if (error != OPUS_OK) {
//        __android_log_print(ANDROID_LOG_ERROR, "HuyaDecoder", "init_error:%d", error);
    }
    opus_int32 skip;
    opus_decoder_ctl(opusDecoder, OPUS_SET_BITRATE(VOICE_OPUS_BITRATE));
    opus_decoder_ctl(opusDecoder, OPUS_SET_BANDWIDTH(VOICE_OPUS_BANDWIDTH));
    opus_decoder_ctl(opusDecoder, OPUS_SET_VBR(VOICE_OPUS_USE_VBR));
    opus_decoder_ctl(opusDecoder, OPUS_SET_VBR_CONSTRAINT(VOICE_OPUS_CONSTRAINT_USE_VBR));
    opus_decoder_ctl(opusDecoder, OPUS_SET_COMPLEXITY(VOICE_OPUS_COMPLEXITY));
    opus_decoder_ctl(opusDecoder, OPUS_SET_INBAND_FEC(VOICE_OPUS_USE_INBANDFEC));
    opus_decoder_ctl(opusDecoder, OPUS_SET_FORCE_CHANNELS(VOICE_OPUS_CHANNEL_COUNT));
    opus_decoder_ctl(opusDecoder, OPUS_SET_DTX(VOICE_OPUS_USE_DTX));
    opus_decoder_ctl(opusDecoder, OPUS_SET_PACKET_LOSS_PERC(VOICE_OPUS_PACKET_LOSS_PERC));
    opus_decoder_ctl(opusDecoder, OPUS_SET_SIGNAL(OPUS_SIGNAL_VOICE));
    opus_decoder_ctl(opusDecoder, OPUS_SET_EXPERT_FRAME_DURATION(getFramePeriod()));
    opus_decoder_ctl(opusDecoder, OPUS_GET_LOOKAHEAD(&skip));
}

int
HuyaDecoder::decode(const unsigned char *in, opus_int32 in_len, opus_int16 *out, int frame_size) {
//    __android_log_print(ANDROID_LOG_ERROR, "HuyaDecoder", "decode_frame_size:%d", frame_size);
    int size = opus_decode(opusDecoder, in, in_len, out, frame_size, 0);
//    __android_log_print(ANDROID_LOG_ERROR, "HuyaDecoder", "decode_size:%d", size);
    if (size < 0) {
//        __android_log_print(ANDROID_LOG_ERROR, "HuyaDecoder", "decode_error:%s",
//                            opus_strerror(size));
    }
    return size * m_channels; // 返回采样size ，需要 * 声道数
}

int HuyaDecoder::opus2wav(const char *opusFilePath, const char *outPutWavFilePath) {
    char *outLeftChannelBytes = NULL; //左声道PCM
    char *outRightChannelBytes = NULL; //右声道PCM
    char *pcmFrame = NULL;
    char *composChannelsBytes = NULL; //左右双声道
    
    int opusSingleFrameLen = getSingleFrameInputSize(); //单声道opus playload大小:160
    int oneOpusPacketLen = 2 * (opusSingleFrameLen + 4); //一个双声道opus帧大小: 2 * (4 + 160)
    
    char *opusBytes = NULL;
    size_t opusSize = 0;
    int ret = readFile(opusFilePath, &opusBytes, &opusSize);
    if (ret) {
        goto END;
    }
    
    //双声道：head(4 bytes) payload(160 bytes) 4 160
    if(opusBytes == NULL || opusSize % oneOpusPacketLen != 0) {
        ret = ErrOpusEncode;
        goto END;
    }

    {
        int pcmFrameSize = getSingleFrameOutputSize(); //单声道opus playload解码出的pcm帧大小
        size_t singleChannelFrameNum = (opusSize / opusSingleFrameLen) / 2;
        size_t outSize = singleChannelFrameNum * pcmFrameSize;
        outLeftChannelBytes = (char *)malloc(outSize);
        outRightChannelBytes = (char *)malloc(outSize);
        pcmFrame = (char *)malloc(pcmFrameSize * sizeof(short));
        composChannelsBytes = (char *)malloc(outSize * 2);
        
        if (!(outLeftChannelBytes && outRightChannelBytes && pcmFrame &&composChannelsBytes)) {
            ret = ErrMemMalloc;
            goto END;
        }
        
        //分别解码右声道，左右声道，最后合成双声道PCM
        //opus data: 4 160 4 160 | 4 160 4 160 |
        for (int i = 0, iloop = 0; i < opusSize; i += oneOpusPacketLen, iloop++){
            char *fbytes = outLeftChannelBytes + pcmFrameSize * iloop;
            size_t decodedSamples = decode((const unsigned char *)opusBytes + i + 4, opusSingleFrameLen, (short *)pcmFrame, pcmFrameSize);
            if (decodedSamples < 0) {
                fprintf(stderr, "left channel, decode error:%zu\n", decodedSamples);
            }
            
            for(int j = 0; j < pcmFrameSize; j++) {
                short s;
                s = pcmFrame[j];
                fbytes[2*j] = s&0xFF;
                fbytes[2*j+1] = (s>>8)&0xFF;
            }
            
            //右声道
            bzero(pcmFrame, pcmFrameSize * 2);
            fbytes = outRightChannelBytes + pcmFrameSize * iloop;
            decodedSamples = decode((const unsigned char *)opusBytes + (opusSingleFrameLen + 4) * (2 * iloop + 1), opusSingleFrameLen, (short *)pcmFrame, pcmFrameSize);
            if (decodedSamples < 0) {
                fprintf(stderr, "right channel, decode error:%zu\n", decodedSamples);
            }
            
            for(int j = 0; j < pcmFrameSize; j++) {
                short s;
                s = pcmFrame[j];
                fbytes[2*j] = s&0xFF;
                fbytes[2*j+1] = (s>>8)&0xFF;
            }
        }
        
       //左右声道合成
        for (int i = 0; i < outSize / 2; i++) {
            composChannelsBytes[i * 4]     = outLeftChannelBytes[i * 2];
            composChannelsBytes[i * 4 + 1] = outLeftChannelBytes[i * 2 + 1];
            composChannelsBytes[i * 4 + 2] = outRightChannelBytes[i * 2];
            composChannelsBytes[i * 4 + 3] = outRightChannelBytes[i * 2 + 1];
        }
        
        ret = writeFile(outPutWavFilePath, composChannelsBytes, outSize * 2);
    }
        
END:
    free(opusBytes);
    free(composChannelsBytes);
    free(outLeftChannelBytes);
    free(outRightChannelBytes);
    free(pcmFrame);
    return ret;
}

void HuyaDecoder::destory() {
    opus_decoder_destroy(opusDecoder);
}



}
