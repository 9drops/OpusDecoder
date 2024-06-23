//
//  OpusFileDecoder.cpp
//  OpusFileDecoder
//
//  Created by drops on 2024/6/16.
//

#include "OpusFileDecoder.h"
#include "HuyaDecoder.h"
#include <stdio.h>

#define Debug printf

DecoderPtr initOpusDecoder2(int framePeriod, int singleFrameInputSize, int totalFrameInputSize, int pcmOutputSize, int *errCode) {
    initDecoder(framePeriod, singleFrameInputSize, totalFrameInputSize, pcmOutputSize);
    Debug("init framePeriod = %d , singleFrameInputSize = %d , totalFrameInputSize = %d , pcmOutputSize = %d ", getFramePeriod() , getSingleFrameInputSize() , getTotalFrameInputSize() , getSingleFrameOutputSize());
    HuyaDecoder *huyaDecoder = new HuyaDecoder();
    huyaDecoder->init();
    return (DecoderPtr)huyaDecoder;
}

DecoderPtr initOpusDecoder() {
    return initOpusDecoder2(-1, -1, -1, -1, NULL);
}

/// 销毁解码器
/// - Parameter decoderHandler: decoderHandler [in] 解码器句柄
void destoryOpusDecoder(DecoderPtr decoderHandler) {
    HuyaDecoder *huyaDecoder = (HuyaDecoder *)decoderHandler;
    huyaDecoder->destory();
}

/// 将opus文件转换成wav文件
/// - Parameters:
///   - decoderHandler: decoderHandler [in] 解码器句柄
///   - opusFilePath: opus本地文件路径
///   - outPutWavPath: 转换后生成的wav文件路径
///   返回0成功，其他失败
int opus2wav(DecoderPtr decoderHandler, const char *opusFilePath, const char *outPutWavPath) {
    HuyaDecoder *huyaDecoder = (HuyaDecoder *)decoderHandler;
    if (!huyaDecoder) {
        return ErrCreatDecoder;
    }
    
    return huyaDecoder->opus2wav(opusFilePath, outPutWavPath);
}

