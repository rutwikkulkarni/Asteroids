void ResetAsteroids(){
    game.asteroids_pointer = 0;
    for(int i = 0; i < 4; i++){
        Asteroid asteroid = {};
        i32 r_y = rand();
        i32 r_x = rand();
        //only on bounds of screen
        if(r_x % 2 == 0){
            r_x = r_x % 20;
        }else{
            r_x = (r_x % 20) + (game.platform->initial_window_width - 200);
        }
        if(r_y % 2 == 0){
            r_y = r_y % 20;
        }else{
            r_y = (r_y % 20) + (game.platform->initial_window_height - 200);
        }
        
        asteroid.pos = V2(r_x, r_y);
        i32 sign_x = rand() % 20 + 80;
        if(sign_x % 2 == 0) sign_x = -sign_x;
        i32 sign_y = -sign_x;
        asteroid.velocity = V2(sign_x * game.platform->dt, sign_y * game.platform->dt);
        asteroid.scale = V2(128, 128);
        asteroid.type = 0;
        game.asteroids[game.asteroids_pointer++] = asteroid;
    }
}

Texture CreateTexture(char *path){
    Texture texture = {};
    glGenTextures(1, &texture.id);
    glBindTexture(GL_TEXTURE_2D, texture.id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    stbi_set_flip_vertically_on_load(true);
    u8 *image = stbi_load(path, &texture.width, &texture.height, &texture.channels, STBI_rgb_alpha);
    if(image){
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture.width, texture.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
        stbi_image_free(image);
    }
    return texture;
}

Shader LoadShader(char *path, ShaderType type){
    Shader shader = {};
    if(type == FRAGMENT){
        shader.id = glCreateShader(GL_FRAGMENT_SHADER);
    }else if(type == VERTEX){
        shader.id = glCreateShader(GL_VERTEX_SHADER);
    }else{
        game.platform->OSDisplayError("Incorrect shader", "Error: incorrect shader format or shader not supported.");
        return shader;
    }
    FileString source = game.platform->OSReadFileToString(path);
    glShaderSource(shader.id, 1, &source.data, NULL);
    glCompileShader(shader.id);
    GLint successful;
    glGetShaderiv(shader.id, GL_COMPILE_STATUS, &successful);
    if(successful == GL_FALSE){
        if(type == FRAGMENT){
            game.platform->OSDisplayError("OpenGL Shader Compilation Error", "Failed to compile fragment shader");
        }else if(type == VERTEX){
            game.platform->OSDisplayError("OpenGL Shader Compilation Error", "Failed to compile vertex shader");
            char log[256];
        }
        char log[512];
        glGetShaderInfoLog(shader.id, 512, NULL, log);
        game.platform->OSDisplayError("OpenGL Shader Compilation Error", log);
    }
    return shader;
}

Shader CreateShaderProgram(Shader vertex, Shader fragment){
    Shader shader_program = {};
    shader_program.id = glCreateProgram();
    glAttachShader(shader_program.id, vertex.id);
    glAttachShader(shader_program.id, fragment.id);
    glLinkProgram(shader_program.id);
    glUseProgram(shader_program.id);
    return shader_program;
}

void PlayAnimation(Animation *anim, Vec2 pos, Vec2 scale){
    anim->playing = true;
    anim->pos = pos;
    anim->scale = scale;
}

void GameInit(Platform *platform){
    game.platform = platform;
    
    {   
        game.player = {};
        game.player.scale.x = 48;
        game.player.scale.y = 48;
        game.player.collision_box.x = 32;
        game.player.collision_box.y = 32;
        game.player.pos.x = game.platform->initial_window_width / 2 - 16;
        game.player.pos.y = game.platform->initial_window_height / 2 - 16;
        game.player.asteroids_killed = 0;
        game.score = 0;
        game.lives = 3;
        game.has_invincibility = false;
        game.invincibility_time = 1.0f;
    }
    
    {
        game.bullets_pointer = 0;
        game.bullet_timer = 0.0f;
    }
    
    {
        game.asteroid_anim.playing = false;
        game.asteroid_anim.looping = false;
        game.asteroid_anim.uv[0] = ASTEROID_EXPLOSION_1;
        game.asteroid_anim.uv[1] = ASTEROID_EXPLOSION_2;
        game.asteroid_anim.uv[2] = ASTEROID_EXPLOSION_3;
        game.asteroid_anim.uv[3] = ASTEROID_EXPLOSION_4;
        game.asteroid_anim.frames = 4;
        game.asteroid_anim.rotation = 0;
        game.asteroid_anim.duration = 0.20f;
        game.asteroid_anim.time_left = game.asteroid_anim.duration;
        
        game.player_anim.playing = false;
        game.player_anim.looping = true;
        game.player_anim.uv[0] = PLAYER_FIRE_1;
        game.player_anim.uv[1] = PLAYER_FIRE_2;
        game.player_anim.uv[2] = PLAYER_FIRE_3;
        game.player_anim.uv[3] = PLAYER_FIRE_4;
        game.player_anim.frames = 4;
        game.player_anim.rotation = 0;
        game.player_anim.duration = 0.35f;
        game.player_anim.time_left = game.player_anim.duration;
    }
    
    InitAudio();
    
    ResetAsteroids();
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    { /* Quad initialisation */
        glGenVertexArrays(1, &game.quad_vao);
        glBindVertexArray(game.quad_vao);
        GLfloat quad_vertices[] = {
            // vertices  textures
            0, 0, 0,     0, 0,
            0, 1, 0,     0, 0.125f,
            1, 1, 0,     0.125f, 0.125f,
            0, 0, 0,     0, 0,
            1, 0, 0,     0.125f, 0,
            1, 1, 0,     0.125f, 0.125f
        };
        glGenBuffers(1, &game.quad_vbo);
        glBindBuffer(GL_ARRAY_BUFFER, game.quad_vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quad_vertices), quad_vertices, GL_STATIC_DRAW);
        
        game.game_texture = CreateTexture("../data/asteroids-arcade.png");
        
        Shader quad_vertex_shader = LoadShader("../data/quad_vertex.glsl", VERTEX);
        Shader quad_fragment_shader = LoadShader("../data/quad_fragment.glsl", FRAGMENT);
        
        game.quad_shader = CreateShaderProgram(quad_vertex_shader, quad_fragment_shader);
        
        glVertexAttribPointer(0, 3, GL_FLOAT, false, 5*sizeof(f32), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 2, GL_FLOAT, false, 5*sizeof(f32), (void*)(3 * sizeof(f32)));
        glEnableVertexAttribArray(1);
        
        /* Matrix initialisation */
        Mat4 ortho = Orthographic(0, game.platform->initial_window_width, game.platform->initial_window_height, 0, 0, 10);
        GLuint ortho_location = glGetUniformLocation(game.quad_shader.id, "ortho");
        glUniformMatrix4fv(ortho_location, 1, true, &ortho.elements[0][0]);
    }
    
    { /* Text Initialisation */ // doesn't have to be on a different vao as this text is just a bitmap, but maybe do some cool effects after everything's working with ttf files
        glGenVertexArrays(1, &game.text_vao);
        glBindVertexArray(game.text_vao);
        GLfloat text_vertices[] = {
            // vertices  textures
            0, 0, 0,     0, 0,
            0, 1, 0,     0, 1.0f,
            1, 1, 0,     1.0f, 1.0f,
            0, 0, 0,     0, 0,
            1, 0, 0,     1.0f, 0,
            1, 1, 0,     1.0f, 1.0f
        };
        glGenBuffers(1, &game.text_vbo);
        glBindBuffer(GL_ARRAY_BUFFER, game.text_vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(text_vertices), text_vertices, GL_STATIC_DRAW);
        
        game.text_texture = CreateTexture("../data/font.bmp");
        
        Shader text_vertex_shader = LoadShader("../data/text_vertex.glsl", VERTEX);
        Shader text_fragment_shader = LoadShader("../data/text_fragment.glsl", FRAGMENT);
        
        game.text_shader = CreateShaderProgram(text_vertex_shader, text_fragment_shader);
        
        glVertexAttribPointer(0, 3, GL_FLOAT, false, 5*sizeof(f32), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 2, GL_FLOAT, false, 5*sizeof(f32), (void*)(3 * sizeof(f32)));
        glEnableVertexAttribArray(1);
        
        /* Matrix initialisation */
        Mat4 ortho = Orthographic(0, game.platform->initial_window_width, game.platform->initial_window_height, 0, 0, 10);
        GLuint ortho_location = glGetUniformLocation(game.text_shader.id, "ortho");
        glUniformMatrix4fv(ortho_location, 1, true, &ortho.elements[0][0]);
    }
    
    { //opengl fbo initialisation - for post processing stuff
        //1. Create Framebuffer
        //2. Attach Color/depth/stencil buffer
        //3. Check the buffer is valid
        //4. Ready for launch
        glGenFramebuffers(1, &game.fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, game.fbo);
        
        //create texture to attach to fbo
        glGenTextures(1, &game.fbo_texture);
        glBindTexture(GL_TEXTURE_2D, game.fbo_texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1280, 720, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);  
        
        //attach colour to fbo
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, game.fbo_texture, 0);
        
        //attach depth/stencil to fbo - note: render buffers are good for when we don't need to read from the storage eg stencil and depth buffer, but most of the time with a colour buffer you are sampling those values so you should use a texture for a colour buffer.
        glGenRenderbuffers(1, &game.rbo);
        glBindRenderbuffer(GL_RENDERBUFFER, game.rbo);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 1280, 720);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, game.rbo);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);
        
        if(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE){
            OutputDebugString("Created framebuffer");
        }else{
            OutputDebugString("Failed to create frame buffer.");
        }
        
        //now set up the vao/vbo of the texture we attach to the screen.
        GLfloat fbo_vertices[] = {
            // vertices  textures
            -1, -1, 0,     0, 0,
            -1, 1, 0,     0, 1.0f,
            1, 1, 0,     1.0f, 1.0f,
            -1, -1, 0,     0, 0,
            1, -1, 0,     1.0f, 0,
            1, 1, 0,     1.0f, 1.0f
        };
        glGenVertexArrays(1, &game.fbo_vao);
        glBindVertexArray(game.fbo_vao);
        glGenBuffers(1, &game.fbo_vbo);
        glBindBuffer(GL_ARRAY_BUFFER, game.fbo_vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(fbo_vertices), fbo_vertices, GL_STATIC_DRAW);
        
        Shader fbo_vertex_shader = LoadShader("../data/fbo_vertex.glsl", VERTEX);
        Shader fbo_fragment_shader = LoadShader("../data/fbo_fragment.glsl", FRAGMENT);
        
        game.fbo_shader = CreateShaderProgram(fbo_vertex_shader, fbo_fragment_shader);
        
        glVertexAttribPointer(0, 3, GL_FLOAT, false, 5*sizeof(f32), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 2, GL_FLOAT, false, 5*sizeof(f32), (void*)(3 * sizeof(f32)));
        glEnableVertexAttribArray(1);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
}

void DrawQuadFromTexture(Texture texture, Vec2 pos, Vec2 size, Vec4 crop, f32 rot_z){
    /* Update Vertex Data per frame */
    // x = x offset, y = y offset, z = width, w = height
    Vec2 bottom_left  = V2(crop.x / 256.0f, (crop.y / 256.0f));
    Vec2 bottom_right = V2((crop.x + crop.z) / 256.0f, crop.y / 256.0f);
    Vec2 top_left     = V2(crop.x / 256.0f, (crop.y + crop.w) / 256.0f);
    Vec2 top_right    = V2((crop.x + crop.z) / 256.0f, (crop.y + crop.w) / 256.0f);
    glBindTexture(GL_TEXTURE_2D, texture.id);
    glBindVertexArray(game.quad_vao);
    glBindBuffer(GL_ARRAY_BUFFER, game.quad_vbo);
    GLfloat quad_vertices[] = {
        // vertices  textures
        0, 0, 0,     bottom_left.x, bottom_left.y,
        0, 1, 0,     top_left.x, top_left.y,
        1, 1, 0,     top_right.x, top_right.y,
        0, 0, 0,     bottom_left.x, bottom_left.y,
        1, 0, 0,     bottom_right.x, bottom_right.y,
        1, 1, 0,     top_right.x, top_right.y
    };
    //translate to centre for rotation first
    Vec2 centre = V2(pos.x + (size.x / 2.0f), pos.y + (size.y / 2.0f));
    Mat4 translate = M4Translate(M4I(), V3(centre.x, centre.y, 0));
    Mat4 translate_rotate = translate * M4RotateZ(M4I(), rot_z);
    translate_rotate = translate_rotate * M4Translate(M4I(), V3(pos.x - centre.x, pos.y - centre.y, 0));
    Mat4 translate_rotate_scale = translate_rotate * M4Scale(M4I(), V3(size.x, size.y, 1));
    for(int i = 0; i < 6; i++){
        Vec4 trans = V4(quad_vertices[i * 5], quad_vertices[i * 5 + 1], quad_vertices[i * 5 + 2], 1.0f);
        Vec4 vertex_pos = M4MultV4(translate_rotate_scale, trans);
        quad_vertices[i * 5] = vertex_pos.x;
        quad_vertices[i * 5 + 1] = vertex_pos.y;
        quad_vertices[i * 5 + 2] = vertex_pos.z;
    }
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(quad_vertices), quad_vertices);
    glUseProgram(game.quad_shader.id);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void DrawChar(char c, Vec2 pos, Vec2 size){
    glBindTexture(GL_TEXTURE_2D, game.text_texture.id);
    glBindVertexArray(game.text_vao);
    glBindBuffer(GL_ARRAY_BUFFER, game.text_vbo);
    
    //convert lower case to upper case
    if(c >= 'a' && c <= 'z'){
        c -= 32; 
    }
    
    //todo: make these configurable
    f32 texture_height = (f32)game.text_texture.height;
    f32 texture_width  = (f32)game.text_texture.width;
    f32 char_width = 256/8;
    f32 char_height = 256/8;
    i32 position = 96 - c;
    
    //start with coord position from (0, 7)
    i32 row_pos = (64 - position) % 8;
    i32 col_pos = (position % 8 == 0) ? (position - 1) / 8 : position / 8;
    //then multiply it to get the pixel value
    row_pos *= char_width;
    col_pos *= char_height;
    //..then divide by texture values to get uv 
    f32 final_row = (f32)row_pos / texture_width;
    f32 final_column = (f32)col_pos / texture_height;
    
    Vec2 bottom_left = V2(final_row, final_column);
    Vec2 bottom_right = V2(final_row + (char_width / texture_width), final_column);
    Vec2 top_left = V2(final_row, final_column + (char_height / texture_height));
    Vec2 top_right = V2(final_row + (char_width / texture_width), final_column + (char_height / texture_height));
    GLfloat text_vertices[] = {
        // vertices  textures
        0, 0, 0,     bottom_left.x, bottom_left.y,
        0, 1, 0,     top_left.x, top_left.y,
        1, 1, 0,     top_right.x, top_right.y,
        0, 0, 0,     bottom_left.x, bottom_left.y,
        1, 0, 0,     bottom_right.x, bottom_right.y,
        1, 1, 0,     top_right.x, top_right.y
    };
    Mat4 translate = M4Translate(M4I(), V3(pos.x, pos.y, 0));
    Mat4 translate_scale = translate * M4Scale(M4I(), V3(size.x, size.y, 0));
    for(int i = 0; i < 6; i++){
        Vec4 trans = V4(text_vertices[i * 5], text_vertices[i * 5 + 1], text_vertices[i * 5 + 2], 1.0f);
        Vec4 vertex_pos = M4MultV4(translate_scale, trans);
        text_vertices[i * 5] = vertex_pos.x;
        text_vertices[i * 5 + 1] = vertex_pos.y;
        text_vertices[i * 5 + 2] = vertex_pos.z;
    }
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(text_vertices), text_vertices);
    glUseProgram(game.text_shader.id);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void DrawText(char *text, Vec2 pos, Vec2 size){
    char *c = text;
    u32 offset = 0;
    while(*c != '\0'){
        DrawChar(*c++, V2(pos.x + (offset * size.x* 0.5f), pos.y), size);
        offset++;
    }
}

void DeleteBullet(int position){
    if(game.bullets_pointer > 0){
        for(int i = position; i < game.bullets_pointer; i++){
            game.bullets[i] = game.bullets[i+1];
        }
        game.bullets_pointer--;
    }
}

void DeleteAsteroid(int position){
    game.player.asteroids_killed++;
    if(game.asteroids_pointer > 0 && game.asteroids[position].type == 5){
        PlayAnimation(&game.asteroid_anim, game.asteroids[position].pos, game.asteroids[position].scale * 2);
        for(int i = position; i < game.asteroids_pointer; i++){
            game.asteroids[i] = game.asteroids[i+1];
        }
        game.score += 50;
        game.asteroids_pointer--;
    }else{
        if(game.asteroids[position].type == 0){
            for(int i = 0; i < 1; i++){
                for(int j = 0; j < 2; j++){
                    Asteroid asteroid = {};
                    asteroid.pos = V2(game.asteroids[position].pos.x + 32, game.asteroids[position].pos.y + 32);
                    i32 r_x = (rand() % 2 == 0 ? 1 : -1);
                    i32 r_y = -r_x;
                    asteroid.velocity = V2(game.asteroids[position].velocity.x * r_x * 1.25f, game.asteroids[position].velocity.y * r_y * 1.25f);
                    asteroid.scale = V2(64, 64);
                    asteroid.type = (i * 2) + j + 1;
                    if(i == 0 && j == 0){
                        game.asteroids[position] = asteroid;
                    }else{
                        game.asteroids[game.asteroids_pointer++] = asteroid;
                    }
                    
                }
            }
            game.score += 10;
            PlayAnimation(&game.asteroid_anim, game.asteroids[position].pos, game.asteroids[position].scale * 2);
        }else if(game.asteroids[position].type > 0 && game.asteroids[position].type < 5){
            for(int i = 0; i < 1; i++){
                for(int j = 0; j < 2; j++){
                    Asteroid asteroid = {};
                    asteroid.pos = V2(game.asteroids[position].pos.x + 16, game.asteroids[position].pos.y + 16);
                    i32 r_x = ((rand() % 2 == 0 ? 1 : -1));
                    i32 r_y = -r_x;
                    asteroid.velocity = V2(game.asteroids[position].velocity.x * r_x * 1.25f, game.asteroids[position].velocity.y * r_y * 1.25f);
                    asteroid.scale = V2(32, 32);
                    asteroid.type = 5;
                    if(i == 0 && j == 0){
                        game.asteroids[position] = asteroid;
                    }else{
                        game.asteroids[game.asteroids_pointer++] = asteroid;
                    }
                }
            }
            game.score += 20;
            PlayAnimation(&game.asteroid_anim, game.asteroids[position].pos, game.asteroids[position].scale * 2);
        }
    }
}

void PlayerShoot(){
    game.bullet_timer = TIME_BETWEEN_SHOOTING;
    Bullet b = {};
    b.pos = game.player.pos + V2(-32.0f * sin(Radians(game.player.direction)) + 8.0f, 32.0f * cos(Radians(game.player.direction)));
    b.velocity = V2(-BULLET_SPEED*sin(Radians(game.player.direction))*game.platform->dt, BULLET_SPEED*cos(Radians(game.player.direction))*game.platform->dt);
    b.scale = V2(32, 32);
    b.lifetime = BULLET_LIFETIME;
    if(game.bullets_pointer < MAX_BULLETS){
        game.bullets[game.bullets_pointer++] = b;
    }
}

void CheckBounds(Vec2 *pos, Vec2 scale){
    if(pos->y < -scale.y){
        pos->y = game.platform->initial_window_height;
    }
    if(pos->y > game.platform->initial_window_height){
        pos->y = -scale.y;
    }
    if(pos->x > game.platform->initial_window_width){
        pos->x = -scale.x;
    }
    if(pos->x < -scale.x){
        pos->x = game.platform->initial_window_width;
    }
}

void UpdateBullets(){
    if(game.bullet_timer > 0.0f){
        game.bullet_timer -= game.platform->dt;
        if(game.bullet_timer < 0.0f){
            game.bullet_timer = 0.0f;
        }
    }
    
    i32 bullets_to_delete[MAX_BULLETS];
    i32 bullets_to_delete_pointer = 0;
    i32 asteroids_to_delete[MAX_ASTEROIDS];
    i32 asteroids_to_delete_pointer = 0;
    
    for(int i = 0; i < game.bullets_pointer; i++){
        game.bullets[i].pos = game.bullets[i].pos + game.bullets[i].velocity;
        CheckBounds(&game.bullets[i].pos, game.bullets[i].scale);
        DrawQuadFromTexture(game.game_texture, game.bullets[i].pos, game.bullets[i].scale, V4(0, 126 - 16, 16.0f, 16.0f), 0.0f);
        game.bullets[i].lifetime -= game.platform->dt;
        if(game.bullets[i].lifetime < 0){
            DeleteBullet(i);
        }
        Vec4 bounding_box_bullet = V4(game.bullets[i].pos.x, game.bullets[i].pos.y, game.bullets[i].scale.x, game.bullets[i].scale.y);
        
        for(int j = 0; j < game.asteroids_pointer; j++){
            Vec4 bounding_box_asteroid = V4(game.asteroids[j].pos.x, game.asteroids[j].pos.y, game.asteroids[j].scale.x, game.asteroids[j].scale.y);
            if(AABB(bounding_box_bullet, bounding_box_asteroid)){
                bullets_to_delete[bullets_to_delete_pointer++] = i;
                asteroids_to_delete[asteroids_to_delete_pointer++] = j;
            }
        }
    }
    
    for(int i = 0; i < bullets_to_delete_pointer; i++){
        DeleteBullet(bullets_to_delete[i]);
    }
    
    for(int i = 0; i < asteroids_to_delete_pointer; i++){
        DeleteAsteroid(asteroids_to_delete[i]);
    }
}

void PlayerLoseLife(){
    if(game.lives > 0){
        game.lives--;
        game.has_invincibility = true;
        game.player.pos = V2(-500, -500); //teleport off screen
    }
}

void UpdateAsteroids(){
    for(int i = 0; i < game.asteroids_pointer; i++){
        game.asteroids[i].pos = game.asteroids[i].pos + game.asteroids[i].velocity;
        CheckBounds(&game.asteroids[i].pos, game.asteroids[i].scale);
        
        if(game.asteroids[i].type == 0){
            DrawQuadFromTexture(game.game_texture, game.asteroids[i].pos, game.asteroids[i].scale, BIG_ASTEROID, 0);
        }else if(game.asteroids[i].type == 1){
            DrawQuadFromTexture(game.game_texture, game.asteroids[i].pos, game.asteroids[i].scale, MEDIUM_ASTEROID_BL, 0);
        }
        else if(game.asteroids[i].type == 2){
            DrawQuadFromTexture(game.game_texture, game.asteroids[i].pos, game.asteroids[i].scale, MEDIUM_ASTEROID_BR, 0);
        }
        else if(game.asteroids[i].type == 3){
            DrawQuadFromTexture(game.game_texture, game.asteroids[i].pos, game.asteroids[i].scale, MEDIUM_ASTEROID_TL, 0);
        }
        else if(game.asteroids[i].type == 4){
            DrawQuadFromTexture(game.game_texture, game.asteroids[i].pos, game.asteroids[i].scale, MEDIUM_ASTEROID_TR, 0);
        }
        else if(game.asteroids[i].type == 5){
            DrawQuadFromTexture(game.game_texture, game.asteroids[i].pos, game.asteroids[i].scale, SMALL_ASTEROID_BL, 0);
        }
        
        Vec4 bounding_box_asteroid = V4(game.asteroids[i].pos.x, game.asteroids[i].pos.y, game.asteroids[i].scale.x, game.asteroids[i].scale.y);
        Vec4 bounding_box_player = V4(game.player.pos.x + ((game.player.scale.x - game.player.collision_box.x) / 2.0f), game.player.pos.y + ((game.player.scale.y - game.player.collision_box.y) / 2.0f), game.player.collision_box.x, game.player.collision_box.y);
        if(AABB(bounding_box_player, bounding_box_asteroid)){
            
            PlayerLoseLife();
        }
    }
}

void UpdatePlayer(){
    game.player.acceleration = V2(0, 0);
    bool player_pressed = false;
    if(game.platform->key_down[KEY_W]){
        game.player.acceleration = V2(-15*sin(Radians(game.player.direction)), 15*cos(Radians(game.player.direction)));
        player_pressed = true;
    } 
    if(game.platform->key_down[KEY_S]){
        game.player.acceleration = V2(15*sin(Radians(game.player.direction)), -15*cos(Radians(game.player.direction)));
        player_pressed = true;
    } 
    
    if(game.platform->key_down[KEY_A]){
        game.player.direction += 3;
    }
    if(game.platform->key_down[KEY_D]){
        game.player.direction -= 3;
    }
    if(game.platform->key_down[KEY_SPACE] && game.bullet_timer == 0.0f){
        AudioPlayFromStart(&game.shoot_sound);
        PlayerShoot();
    }
    
    game.player.velocity = game.player.velocity + game.player.acceleration * game.platform->dt;
    game.player.velocity.x -= game.player.velocity.x * 1.5f * game.platform->dt;
    game.player.velocity.y -= game.player.velocity.y * 1.5f * game.platform->dt;
    
    if(game.player.velocity.x >= MAX_SPEED){
        game.player.velocity.x = MAX_SPEED;
    }
    if(game.player.velocity.x <= -MAX_SPEED){
        game.player.velocity.x = -MAX_SPEED;
    }
    if(game.player.velocity.y >= MAX_SPEED){
        game.player.velocity.y = MAX_SPEED;
    }
    if(game.player.velocity.y <= -MAX_SPEED){
        game.player.velocity.y = -MAX_SPEED;
    }
    
    game.player.pos = ((game.player.acceleration * 0.5f) * game.platform->dt * game.platform->dt) + game.player.velocity + game.player.pos;
    CheckBounds(&game.player.pos, game.player.scale);
    
    if(game.player.asteroids_killed == 28){
        game.player.asteroids_killed = 0;
        game.reset_asteroids = true;
    }
    
    DrawQuadFromTexture(game.game_texture, game.player.pos, game.player.scale, V4(0.0f, 256.0f - 32.0f, 32.0f, 32.0f), game.player.direction);
    if(player_pressed){
        game.player_anim.rotation = game.player.direction;
        PlayAnimation(&game.player_anim, V2(game.player.pos.x + 16*sin(Radians(game.player.direction)), game.player.pos.y - 16*cos(Radians(game.player.direction))), V2(48, 48));
    }
    else{
        game.player_anim.playing = false;
    }
}

void UpdateAnimation(Animation *anim){
    if(anim->playing){
        if(anim->looping){
            f32 seconds_per_frame = anim->duration / (f32)anim->frames;
            DrawQuadFromTexture(game.game_texture, anim->pos, anim->scale, anim->uv[anim->current_frame], anim->rotation);
            if(anim->frame_timer > seconds_per_frame){
                if(anim->current_frame < anim->frames - 1) anim->current_frame++;
                else anim->current_frame = 0;
                anim->frame_timer = 0;
            }
            anim->frame_timer += game.platform->dt;
        } else if(anim->time_left > 0.0f){
            f32 seconds_per_frame = anim->duration / (f32)anim->frames;
            DrawQuadFromTexture(game.game_texture, anim->pos, anim->scale, anim->uv[anim->current_frame], anim->rotation);
            if(anim->frame_timer > seconds_per_frame){ anim->current_frame++;
                anim->frame_timer = 0;
            }
            anim->frame_timer += game.platform->dt;
            anim->time_left -= game.platform->dt;
        }else{
            anim->playing = false;
            anim->time_left = anim->duration;
            anim->frame_timer = 0;
            anim->current_frame = 0;
        }
    }
}

void CheckRestart(){
    if(game.reset_asteroids == true){
        //not game over - just if the asteroids have been killed
        ResetAsteroids();
        game.reset_asteroids = false;
    }
    //game over check
    if(game.lives == 0){
        ResetAsteroids();
        game.score = 0;
        game.lives = 3;
    }
}

void DrawUI(){
    char score_string[8];
    snprintf(score_string, 8, "%i", game.score);
    DrawText("Score: ", V2(12, 720 - 54), V2(48, 48));
    DrawText(score_string, V2(156, 720 - 54), V2(48, 48));
    
    DrawText("Lives: ", V2(1280 - 180, 720 - 54), V2(48, 48));
    char lives_string[2];
    snprintf(lives_string, 2, "%i", game.lives);
    DrawText(lives_string, V2(1280 - 40, 720 - 54), V2(48, 48));
}

void GameUpdateAndRender(Platform *platform){
    static bool initialised = false;
    if(!initialised){
        GameInit(platform);
        initialised = true;
    }
    {
        glViewport(0, 0, 1280, 720);
        glBindFramebuffer(GL_FRAMEBUFFER, game.fbo);
        glClearColor(0.0f / 255.0f, 0.0f / 255.0f, 0.0f / 255.0f, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }
    
    UpdateAsteroids();
    UpdateBullets();
    if(!game.has_invincibility){
        UpdatePlayer();
        UpdateAnimation(&game.player_anim);
    }else{
        game.invincibility_time -= game.platform->dt;
        if(game.invincibility_time <= 0){
            game.has_invincibility = false;
            game.player.pos.x = game.platform->initial_window_width / 2 - 16;
            game.player.pos.y = game.platform->initial_window_height / 2 - 16;
            game.player.acceleration = V2(0, 0);
            game.player.velocity = V2(0, 0);
            game.invincibility_time = 1;
        }
    }
    UpdateAnimation(&game.asteroid_anim);
    
    DrawUI();
    CheckRestart();
    UpdateAudio();
    
    {
        glViewport(0, 0, game.platform->window_width, game.platform->window_height);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glDisable(GL_DEPTH_TEST);
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f); 
        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(game.fbo_shader.id);
        glBindVertexArray(game.fbo_vao);
        glBindTexture(GL_TEXTURE_2D, game.fbo_texture);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }
}