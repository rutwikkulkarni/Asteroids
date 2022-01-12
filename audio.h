/* date = January 11th 2022 6:12 pm */
#ifndef AUDIO_H
#define AUDIO_H

#pragma pack(push, 1)
struct WAVHeader{
    u8 chunk_id[4];
    u32 file_size;
    u8 file_type_header[4];
    u8 fmt_chunk_marker[4];
    u32 sub_chunk_1_size;
    u16 audio_format;
    u16 num_channels;
    u32 sample_rate;
    u32 byte_rate;
    u16 block_align;
    u16 bits_per_sample;
    u8 sub_chunk_2_id[4];
    u32 sub_chunk_2_size;
};
#pragma pack(pop)

struct WAVFile{
    WAVHeader header;
    i16 *samples;
    u32 sample_count;
};

struct Audio{
    b32 playing;
    b32 looping;
    i16 *samples;
    u32 sample_count;
    u32 current_position;
};

#endif //AUDIO_H
