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
#include "lame.h"

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
    int err = 0;
    OpusDecoder *decoder = NULL;
    SNDFILE *output_file = NULL;
    unsigned char *packet = NULL;
    opus_int16 *output_buffer = NULL;
    
    FILE *input_file = fopen(input_filename, "rb");
    if (!input_file) {
        fprintf(stderr, "Could not open input file '%s'\n", input_filename);
        err = ErrOpenOpusFile;
        goto END;
    }

    unsigned char header_data[HEADER_SIZE];
    if (fread(header_data, 1, HEADER_SIZE, input_file) != HEADER_SIZE) {
        fprintf(stderr, "Failed to read header\n");
        err = ErrReadOpusHeader;
        goto END;
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
    output_file = sf_open(output_filename, SFM_WRITE, &sf_info);
    if (!output_file) {
        fprintf(stderr, "Could not open output file '%s'\n", output_filename);
        err = ErrOpenOutputFile;
        goto END;
    }

    decoder = opus_decoder_create(SAMPLE_RATE, channels, &err);
    if (err != OPUS_OK) {
        fprintf(stderr, "Failed to create decoder: %s\n", opus_strerror(err));
        err = ErrCreatDecoder;
        goto END;
    }

    packet = malloc(packet_size);
    output_buffer = malloc(frame_size * channels * sizeof(opus_int16));

    while (1) {
//        fprintf(stderr, "sequence:%d channels:%d packet_size:%d\n", sequence, channels, packet_size);
        // 读取负载数据
        if (fread(packet, 1, packet_size, input_file) != packet_size) {
            if (feof(input_file)) break; // 文件结束
            
            fprintf(stderr, "Failed to read packet\n");
            err = ErrReadOpusPayload;
            goto END;
        }

        // 解码数据帧
        int frame_size_decoded = opus_decode(decoder, packet, packet_size, output_buffer, frame_size, 0);
        if (frame_size_decoded < 0) {
            fprintf(stderr, "Decoder failed: %s\n", opus_strerror(frame_size_decoded));
            err = ErrOpusDecode;
            goto LOOP;
        }

        sf_writef_short(output_file, output_buffer, frame_size_decoded);

    LOOP:
        // 读取下一个包头
        if (fread(header_data, 1, HEADER_SIZE, input_file) != HEADER_SIZE) {
            if (feof(input_file)) break; // 文件结束
            
            fprintf(stderr, "Failed to read header\n");
            err = ErrReadOpusHeader;
            goto END;
        }

        parse_opus_header(header_data, &header);
        sequence = header.sequence;
        packet_size = header.packet_length;
        channels = header.channels;
    }

END:
    opus_decoder_destroy(decoder);
    sf_close(output_file);
    fclose(input_file);
    free(packet);
    free(output_buffer);
    return err;
}

int opus2mp3(const char *input_filename, const char *output_filename, int bitrate) {
    int err = 0; // Default success
    OpusDecoder *decoder = NULL;
    lame_t lame = NULL;
    unsigned char *packet = NULL;
    opus_int16 *output_buffer = NULL;
    unsigned char *mp3_buffer = NULL;
    FILE *output_file = NULL;
    
    FILE *input_file = fopen(input_filename, "rb");
    if (!input_file) {
        fprintf(stderr, "Could not open input file '%s'\n", input_filename);
        err = ErrOpenOpusFile;
        goto END;
    }

    unsigned char header_data[HEADER_SIZE];
    if (fread(header_data, 1, HEADER_SIZE, input_file) != HEADER_SIZE) {
        fprintf(stderr, "Failed to read header\n");
        err = ErrReadOpusHeader;
        goto END;
    }

    OpusHeader header;
    parse_opus_header(header_data, &header);

    uint16_t sequence = header.sequence;
    int channels = header.channels;
    int packet_size = header.packet_length;
    int frame_size = (SAMPLE_RATE / 1000) * FRAME_MS; // Assuming 20 ms frames
    fprintf(stdout, "channels:%d packet_size:%d\n", channels, packet_size);
    
    // 打开输出文件（MP3 格式）
    output_file = fopen(output_filename, "wb");
    
    if (!output_file) {
        fprintf(stderr, "Could not open output file '%s'\n", output_filename);
        err = ErrOpenOutputFile;
        goto END;
    }

    decoder = opus_decoder_create(SAMPLE_RATE, channels, &err);
    if (err != OPUS_OK) {
        fprintf(stderr, "Failed to create decoder: %s\n", opus_strerror(err));
        err = ErrCreatDecoder;
        goto END;
    }

    int mp3_buffer_size = frame_size * channels * 1.25 + 7200;
    packet = malloc(packet_size);
    output_buffer = malloc(frame_size * channels * sizeof(opus_int16));
    mp3_buffer = malloc(mp3_buffer_size);

    // 设置 LAME 编码器
    lame = lame_init();
    lame_set_in_samplerate(lame, SAMPLE_RATE);
    lame_set_out_samplerate(lame, SAMPLE_RATE);
    lame_set_num_channels(lame, channels);
    lame_set_brate(lame, bitrate > 0 ? bitrate : 24); //设置比特率，默认24kb

    if (lame_init_params(lame) < 0) {
        fprintf(stderr, "Error: Could not initialize LAME\n");
        err = ErrMP3EncoderInit;
        goto END;
    }
    
    while (1) {
        // 读取负载数据
        if (fread(packet, 1, packet_size, input_file) != packet_size) {
            if (feof(input_file)) break; // 文件结束
            
            fprintf(stderr, "Failed to read packet\n");
            err = ErrReadOpusPayload;
            goto END;
        }
        
        // 解码数据帧
        int frame_size_decoded = opus_decode(decoder, packet, packet_size, output_buffer, frame_size, 0);
        if (frame_size_decoded < 0) {
            fprintf(stderr, "Decoder failed: %s\n", opus_strerror(frame_size_decoded));
            err = ErrOpusDecode;
            goto LOOP;
        }

        // 使用 LAME 编码 PCM 数据
        int mp3_size;
        if (channels == 1) {
            mp3_size = lame_encode_buffer(lame, output_buffer, NULL, frame_size_decoded, mp3_buffer, mp3_buffer_size);
        } else {
            mp3_size = lame_encode_buffer_interleaved(lame, output_buffer, frame_size_decoded, mp3_buffer, mp3_buffer_size);
        }
        
        if (mp3_size < 0) {
            fprintf(stderr, "Error: LAME encoding failed\n");
            err = ErrMP3EncoderEncode;
            goto LOOP;
        }

        // 确保写入正确字节数，而不是固定的 buffer_size
        if (mp3_size != fwrite(mp3_buffer, 1, mp3_size, output_file)) {
            fprintf(stderr, "Error: fwrite failed\n");
            err = ErrMP3EncoderWrite;
            goto END;
        }
LOOP:
        // 读取下一个包头
        if (fread(header_data, 1, HEADER_SIZE, input_file) != HEADER_SIZE) {
            if (feof(input_file)) break; // 文件结束
            
            fprintf(stderr, "Failed to read header\n");
            err = ErrReadOpusHeader;
            goto END;
        }

        parse_opus_header(header_data, &header);
        sequence = header.sequence;
        packet_size = header.packet_length;
        channels = header.channels;
    }
END:
    opus_decoder_destroy(decoder);
    fclose(output_file);
    fclose(input_file);
    free(packet);
    free(output_buffer);
    free(mp3_buffer);
    lame_close(lame);
    return err;
}
