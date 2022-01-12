WAVFile ReadWAV(char *filename){
    //TODO: more robust file checking
    WAVFile wav_file = {};
    WAVHeader *header = 0;
    File file = game.platform->OSReadFile(filename);
    header = (WAVHeader *)file.data;
    wav_file.header  = *header;
    i16 *samples = (i16 *)file.data;
    samples += 20;
    wav_file.samples = samples; 
    wav_file.sample_count = header->sub_chunk_2_size / 2; //each sample is 16bits so just divide by 2
    return wav_file;
}

Audio LoadAudioFromWAV(char *filename){
    Audio sound = {};
    WAVFile file = ReadWAV(filename);
    sound.samples = file.samples;
    sound.sample_count = file.sample_count;
    sound.current_position = 0;
    sound.looping = false;
    sound.playing = false;
    return sound;
}

void InitAudio(){
    game.shoot_sound = LoadAudioFromWAV("../data/shoot.wav");
}

void AudioPlayFromStart(Audio *audio){
    game.shoot_sound.current_position = 0;
    game.shoot_sound.playing = true;
}

void UpdateAudio(){
    if(game.shoot_sound.playing){
        i16 *current_sample = game.platform->samples;
        i16 *sample_value = game.shoot_sound.samples + game.shoot_sound.current_position;
        for(int i = 0; i < game.platform->samples_to_write; i++){
            *current_sample++ = *sample_value++; //left
            *current_sample++ = *sample_value++; //right
            game.shoot_sound.current_position += 2;
            if(game.shoot_sound.current_position > game.shoot_sound.sample_count){
                if(game.shoot_sound.looping){
                    game.shoot_sound.current_position = 0;
                    sample_value = game.shoot_sound.samples;
                }else{
                    game.shoot_sound.current_position = 0;
                    game.shoot_sound.playing = false;
                    break;
                }
            }
        }
    }
}