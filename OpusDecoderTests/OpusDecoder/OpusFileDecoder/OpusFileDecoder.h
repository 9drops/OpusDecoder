//
//  OpusFileDecoder.h
//  OpusFileDecoder
//
//  Created by drops on 2024/6/16.
//

#ifndef OpusFileDecoder_h
#define OpusFileDecoder_h

typedef unsigned long DecoderPtr;

#ifdef __cplusplus
extern "C" {
#endif

///用默认参数初始化一个解码器
DecoderPtr initOpusDecoder();

/// 初始化解码器
/// - Parameters:
///   - framePeriod: framePeriod , -1使用默认值
///   - singleFrameInputSize: singleFrameInputSize，-1使用默认值
///   - totalFrameInputSize: totalFrameInputSize，-1使用默认值
///   - pcmOutputSize: pcmOutputSize ，-1使用默认值
///   - errCode: 调用后错误码
///   返回非NULL成功，其他失败
DecoderPtr initOpusDecoder2(int framePeriod, int singleFrameInputSize, int totalFrameInputSize, int pcmOutputSize, int *errCode);

/// 销毁解码器
/// - Parameter decoder: [in] 解码器句柄
void destoryOpusDecoder(DecoderPtr decoder);

/// 将opus文件转换成wav文件
/// - Parameters:
///   - decoderHandler: decoder [in] 解码器句柄
///   - opusFilePath: opus本地文件路径
///   - outPutWavPath: 转换后生成的wav文件路径
///   返回0成功，其他失败
int opus2wav(DecoderPtr decoder, const char *opusFilePath, const char *outPutWavPath);

#ifdef __cplusplus
}
#endif


#endif /* OpusDecoder_h */
