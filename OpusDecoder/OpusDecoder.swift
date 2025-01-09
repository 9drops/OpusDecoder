//
//  OpusDecoder.swift
//  OpusDecoder
//
//  Created by Drops on 2024/6/22.
//

import Foundation

/// 将opus文件转换成wav文件
/// - Parameters:
///   - opusFilePath: opus本地文件路径
///   - outPutWavPath: 转换后生成的wav文件路径
///   返回值： 0 - 所有帧都转换成功，大于0 - 成功（但部分帧转换失败），小于0 - 转换失败
public func opus2wav(_ opusFilePath: String, _ wavFilePath: String) -> Int {
    let inPath = opusFilePath.cString(using: String.Encoding.utf8)
    let outPath = wavFilePath.cString(using: String.Encoding.utf8)
    return Int(opus2wav(inPath, outPath) )
}

/// 将opus文件转换成mp3文件
/// - Parameters:
///   - opusFilePath: opus本地文件路径
///   - mp3FilePath: 转换后生成的mp3文件路径
///   - bitrate mp3的比特率(kb)，默认24kb
///   返回值： 0 - 所有帧都转换成功，大于0 - 成功（但部分帧转换失败），小于0 - 转换失败
public func opus2mp3(_ opusFilePath: String, _ mp3FilePath: String, _ bitrate: Int32 = 24) -> Int {
    let inPath = opusFilePath.cString(using: String.Encoding.utf8)
    let outPath = mp3FilePath.cString(using: String.Encoding.utf8)
    return Int(opus2mp3(inPath, outPath, bitrate) )
}
