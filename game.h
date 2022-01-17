#ifndef GAME
#define GAME

/* 
TO DO:
- SOUND            []
-   - Sound API    [X]
 -   - Load a sound [X]
-   - Play a sound [X]
-   - Play multiple sounds []
- ANIMATION SYSTEM [X]
- ALL ANIMATIONS
 -   - PLAYER [X]
  -   - ASTEROID [X]
-   - ENEMY  []
  -   - BULLET []
- RENDERING 
-   - BATCH RENDERER[]
- CONTROLLER INPUT []
- SCORE SYSTEM     [X]
- ENEMY AI         []
- FONTS            [X]
- MENU SCREEN      []
  - GAMEPLAY         []
    -   - BIG ASTEROID: 20PTS
  -   - MEDIUM ASTEROID: 50PTS
-   - SMALL ASTEROID: 100PTS

- __BUGS__
- Audio: clicking at the start of every sound, not sure if this is just the wav file but most likely my code
- Frame drops every 5-10 seconds - see if batch renderer fixes this because there are a lot of draw calls atm
- sometimes score gets counted incorrectly, there should be 1000 score after every round 

*/


#define MAX_SPEED 5

#define MAX_BULLETS 32
#define BULLET_SPEED 1000
#define TIME_BETWEEN_SHOOTING 0.1f
#define BULLET_LIFETIME 0.5f

#define MAX_ASTEROIDS 64
#define BIG_ASTEROID V4(64.0f, 0.0f, 64.0f, 64.0f)
#define MEDIUM_ASTEROID_BL V4(128.0f, 0.0f, 32.0f, 32.0f)
#define MEDIUM_ASTEROID_BR V4(160.0f, 0.0f, 32.0f, 32.0f)
#define MEDIUM_ASTEROID_TL V4(128.0f, 32.0f, 32.0f, 32.0f)
#define MEDIUM_ASTEROID_TR V4(160.0f, 32.0f, 32.0f, 32.0f)
#define SMALL_ASTEROID_BL V4(196.0f, 16.0f, 16.0f, 16.0f)
#define SMALL_ASTEROID_BR V4(196+16.0f, 16.0f, 16.0f, 16.0f)
#define ASTEROID_EXPLOSION_1 V4(0.0f, 0.0f, 64.0f, 64.0f)
#define ASTEROID_EXPLOSION_2 V4(192.0f, 64.0f, 64.0f, 64.0f)
#define ASTEROID_EXPLOSION_3 V4(192.0f - 64.0f, 64.0f, 64.0f, 64.0f)
#define ASTEROID_EXPLOSION_4 V4(192.0f - 128.0f, 64.0f, 64.0f, 64.0f)
#define PLAYER_FIRE_1 V4(64.0f, 192.0f, 32.0f, 32.0f)
#define PLAYER_FIRE_2 V4(96.0f, 192.0f, 32.0f, 32.0f)
#define PLAYER_FIRE_3 V4(128.0f, 192.0f, 32.0f, 32.0f)
#define PLAYER_FIRE_4 V4(160.0f, 192.0f, 32.0f, 32.0f)

#define ASTEROID_EXPLOSION_5 V4(0.0f, 64.0f, 64.0f, 64.0f)
#define ASTEROID_EXPLOSION_6 V4(192.0f, 128.0f, 64.0f, 64.0f)
#define ASTEROID_EXPLOSION_7 V4(192.0f - 64.0f, 128.0f, 64.0f, 64.0f)
#define ASTEROID_EXPLOSION_8 V4(192.0f - 128.0f, 128.0f, 64.0f, 64.0f)


#define MAX_ANIMATION_FRAMES 8

struct Player{
    Vec2 pos;
    Vec2 scale;
    Vec2 collision_box;
    f32 direction;
    Vec2 velocity;
    Vec2 acceleration;
    i32 asteroids_killed;
};

struct Texture{
    u32 id;
    i32 width;
    i32 height;
    i32 channels;
};

enum ShaderType{
    FRAGMENT,
    VERTEX
};

struct Shader{
    u32 id;
};

struct Bullet{
    Vec2 pos;
    Vec2 velocity;
    Vec2 scale;
    f32 lifetime;
};

struct Asteroid{
    Vec2 pos;
    Vec2 velocity;
    Vec2 scale;
    Vec2 collision_box;
    u32 type; // 0 is big asteroid, 1 is medium, 2 is small
};

struct Animation{
    bool playing;
    bool looping;
    Vec2 pos;
    Vec2 scale;
    f32 rotation;
    Vec4 uv[MAX_ANIMATION_FRAMES];
    i32 frames;
    i32 current_frame;
    f32 duration;
    f32 time_left;
    f32 frame_timer;
};

struct Game{
    Platform *platform;
    
    Player player;
    //todo: maybe move this to player struct ?
    u32 score;
    u32 lives;
    b32 has_invincibility;
    f32 invincibility_time;
    
    /* Rendering */
    GLuint quad_vao;
    GLuint quad_vbo;
    Shader quad_shader;
    Texture game_texture;
    
    GLuint text_vao;
    GLuint text_vbo;
    Shader text_shader;
    Texture text_texture;
    
    GLuint fbo;
    GLuint fbo_texture;
    GLuint rbo;
    GLuint fbo_vao;
    GLuint fbo_vbo;
    Shader fbo_shader;
    /* --------- */
    
    Bullet bullets[MAX_BULLETS];
    i32 bullets_pointer;
    f32 bullet_timer;
    Asteroid asteroids[MAX_ASTEROIDS];
    i32 asteroids_pointer;
    b32 reset_asteroids;
    
    Audio shoot_sound;
    
    Animation asteroid_anim;
    Animation player_anim;
};

Game game = {};
#endif /* GAME */
