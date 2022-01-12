#ifndef PLATFORM
#define PLATFORM

enum Keys{
    KEY_NONE, 
    KEY_W,
    KEY_A,
    KEY_S,
    KEY_D,
    KEY_SPACE,
    KEY_MAX
};

struct FileString{
    const char *data;
    u32 size;
};

struct File{
    void *data;
    u32 size;
};

struct Platform{
    b32 running;
    i32 window_width;
    i32 window_height;
    
    i16 *samples;
    i16 samples_to_write;
    u32 samples_per_second;
    
    b32 key_down[KEY_MAX];
    f32 dt;
    
    FileString (*OSReadFileToString)(const char *filename);
    File (*OSReadFile)(const char *filename);
    void (*OSDisplayError)(const char *title, const char *message);
};

#endif /* PLATFORM */
