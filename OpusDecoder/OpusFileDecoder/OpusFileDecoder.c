//
//  OpusFileDecoder.cpp
//  OpusFileDecoder
//
//  Created by drops on 2024/6/16.
//

#include "OpusFileDecoder.h"
#include <stdio.h>
#include <stdint.h>
#include "opus.h"
#include "sndfile.h"

#define SAMPLE_RATE 48000
#define CHANNELS 2
#define FRAME_SIZE 960 // 20 ms frame size at 48 kHz
#define PACKET_SIZE 160
#define HEADER_SIZE 4


int opus2wav(const char *input_filename, const char *output_filename) {
    FILE *input_file = fopen(input_filename, "rb");
    if (!input_file) {
        fprintf(stderr, "Could not open input file '%s'\n", input_filename);
        return ErrOpenOpusFile;
    }

    SF_INFO sf_info;
    sf_info.samplerate = SAMPLE_RATE;
    sf_info.channels = CHANNELS;
    sf_info.format = SF_FORMAT_WAV | SF_FORMAT_PCM_16;
    SNDFILE *output_file = sf_open(output_filename, SFM_WRITE, &sf_info);
    if (!output_file) {
        fprintf(stderr, "Could not open output file '%s'\n", output_filename);
        fclose(input_file);
        return ErrOpenWavFile;
    }

    int err;
    OpusDecoder *decoder = opus_decoder_create(SAMPLE_RATE, CHANNELS, &err);
    if (err != OPUS_OK) {
        fprintf(stderr, "Failed to create decoder: %s\n", opus_strerror(err));
        fclose(input_file);
        sf_close(output_file);
        return ErrCreatDecoder;
    }

    uint32_t header;
    unsigned char packet[PACKET_SIZE] = {0};
    opus_int16 output_buffer[FRAME_SIZE * CHANNELS] = {0};

    while (1) {
        // 读取自定义包头
        if (fread(&header, HEADER_SIZE, 1, input_file) != 1) {
            if (feof(input_file)) break; // 文件结束
            fprintf(stderr, "Failed to read header\n");
            err = ErrReadOpusHeader;
            goto cleanup;
        }

        // 读取负载数据
        if (fread(packet, 1, PACKET_SIZE, input_file) != PACKET_SIZE) {
            if (feof(input_file)) break; // 文件结束
            fprintf(stderr, "Failed to read packet\n");
            err = ErrReadOpusPayload;
            goto cleanup;
        }

        // 解码数据帧
        int frame_size = opus_decode(decoder, packet, PACKET_SIZE, output_buffer, FRAME_SIZE, 0);
        if (frame_size < 0) {
            fprintf(stderr, "Decoder failed: %s\n", opus_strerror(frame_size));
            err = ErrOpusDecode;
            goto cleanup;
        }

        if (frame_size != FRAME_SIZE) {
            fprintf(stderr, "Unexpected frame size: %d\n", frame_size);
            err = ErrDecodedDataSize;
            goto cleanup;
        }

        // 将PCM数据写入输出文件
        sf_writef_short(output_file, output_buffer, frame_size);
    }

    err = 0; // Success

cleanup:
    opus_decoder_destroy(decoder);
    sf_close(output_file);
    fclose(input_file);
    return err;
}

