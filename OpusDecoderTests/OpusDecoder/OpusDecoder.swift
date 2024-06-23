//
//  OpusDecoder.swift
//  OpusDecoder
//
//  Created by Drops on 2024/6/22.
//

import Foundation

///用默认参数初始化一个解码器
public func createDecoder() -> UInt {
    return initOpusDecoder()
}

/// 销毁解码器
/// - Parameter decoder: [in] 解码器句柄
public func destoryDecoder(_ decoder: UInt) {
    destoryOpusDecoder(decoder)
}

///// 将opus文件转换成wav文件
///// - Parameters:
/////   - decoderHandler: decoder [in] 解码器句柄
/////   - opusFilePath: opus本地文件路径
/////   - outPutWavPath: 转换后生成的wav文件路径
/////   返回0成功，其他失败
//int opus2wav(DecoderPtr decoder, const char *opusFilePath, const char *outPutWavPath);

public func opus2wav(_ decoder: UInt, _ opusFilePath: String, _ wavFilePath: String) -> Int {
    let inPath = opusFilePath.cString(using: String.Encoding.utf8)
    let outPath = wavFilePath.cString(using: String.Encoding.utf8);
    return Int(opus2wav(decoder, inPath, outPath) )
}
