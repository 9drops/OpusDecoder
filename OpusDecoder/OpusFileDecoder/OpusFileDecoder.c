//
//  OpusFileDecoder.cpp
//  OpusFileDecoder
//
//  Created by drops on 2024/6/16.
//

#include "OpusFileDecoder.h"
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "opus.h"
#include "sndfile.h"

#define SAMPLE_RATE 16000
#define HEADER_SIZE 4
#define FRAME_MS 20 //（20 ms） frame

// 包头结构
typedef struct {
    uint8_t packet_length;
    uint8_t channels;
    uint16_t sequence;
} OpusHeader;

void parse_opus_header(const unsigned char *header_data, OpusHeader *header) {
    header->packet_length = header_data[0];
    header->channels = header_data[1];
    header->sequence = header_data[2] | (header_data[3] << 8);
}

int opus2wav(const char *input_filename, const char *output_filename) {
    FILE *input_file = fopen(input_filename, "rb");
    if (!input_file) {
        fprintf(stderr, "Could not open input file '%s'\n", input_filename);
        return ErrOpenOpusFile;
    }

    unsigned char header_data[HEADER_SIZE];
    if (fread(header_data, 1, HEADER_SIZE, input_file) != HEADER_SIZE) {
        fprintf(stderr, "Failed to read header\n");
        fclose(input_file);
        return ErrReadOpusHeader;
    }

    OpusHeader header;
    parse_opus_header(header_data, &header);

    uint16_t sequence = header.sequence;
    int channels = header.channels;
    int packet_size = header.packet_length;
    int frame_size = (SAMPLE_RATE / 1000) * FRAME_MS; // Assuming 20 ms frames
    fprintf(stdout, "channels:%d packet_size:%d\n", channels, packet_size);
    
    SF_INFO sf_info;
    sf_info.samplerate = SAMPLE_RATE;
    sf_info.channels = channels;
    sf_info.format = SF_FORMAT_WAV | SF_FORMAT_PCM_16;
    SNDFILE *output_file = sf_open(output_filename, SFM_WRITE, &sf_info);
    if (!output_file) {
        fprintf(stderr, "Could not open output file '%s'\n", output_filename);
        fclose(input_file);
        return ErrOpenWavFile;
    }

    int err;
    OpusDecoder *decoder = opus_decoder_create(SAMPLE_RATE, channels, &err);
    if (err != OPUS_OK) {
        fprintf(stderr, "Failed to create decoder: %s\n", opus_strerror(err));
        fclose(input_file);
        sf_close(output_file);
        return ErrCreatDecoder;
    }

    unsigned char *packet = malloc(packet_size);
    opus_int16 *output_buffer = malloc(frame_size * channels * sizeof(opus_int16));

    while (1) {
//        fprintf(stderr, "sequence:%d channels:%d packet_size:%d\n", sequence, channels, packet_size);
        // 读取负载数据
        if (fread(packet, 1, packet_size, input_file) != packet_size) {
            if (feof(input_file)) break; // 文件结束
            fprintf(stderr, "Failed to read packet\n");
            err = ErrReadOpusPayload;
            goto cleanup;
        }

        // 解码数据帧
        int frame_size_decoded = opus_decode(decoder, packet, packet_size, output_buffer, frame_size, 0);
        if (frame_size_decoded < 0) {
            fprintf(stderr, "Decoder failed: %s\n", opus_strerror(frame_size_decoded));
            err = ErrOpusDecode;
            goto cleanup;
        }

        sf_writef_short(output_file, output_buffer, frame_size_decoded);

        // 读取下一个包头
        if (fread(header_data, 1, HEADER_SIZE, input_file) != HEADER_SIZE) {
            if (feof(input_file)) break; // 文件结束
            fprintf(stderr, "Failed to read header\n");
            err = ErrReadOpusHeader;
            goto cleanup;
        }

        parse_opus_header(header_data, &header);
        sequence = header.sequence;
        packet_size = header.packet_length;
        channels = header.channels;
    }

    err = 0; // Success

cleanup:
    opus_decoder_destroy(decoder);
    sf_close(output_file);
    fclose(input_file);
    free(packet);
    free(output_buffer);
    return err;
}
