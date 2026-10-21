#pragma once

#include "Resources.hpp"

class Sounds
{
private:
    Music music1;
    Music music2;
    Music music3;
    int current_track;

    SoundBuffer damagebuffer;
    Sound damagesound;
    SoundBuffer coinbuffer;
    Sound coinsound;
    SoundBuffer healbuffer;
    Sound healsound;
    SoundBuffer gameoverbuffer;
    Sound gameoversound;

public:
    Sounds() : current_track(1), damagesound(damagebuffer), coinsound(coinbuffer), healsound(healbuffer), gameoversound(gameoverbuffer)
    {
        fs::path path1 = asset_path("retro-waves-of-love_92802.ogg");
        require_loaded(music1.openFromFile(path1), path1);
        fs::path path2 = asset_path("retro-dreamscape_92772.ogg");
        require_loaded(music2.openFromFile(path2), path2);
        fs::path path3 = asset_path("pixelated-adventures_92797.ogg");
        require_loaded(music3.openFromFile(path3), path3);

        fs::path damagepath = asset_path("hitHurt.ogg");
        require_loaded(damagebuffer.loadFromFile(damagepath), damagepath);
        damagesound.setBuffer(damagebuffer);
        fs::path coinpath = asset_path("pickupCoin.ogg");
        require_loaded(coinbuffer.loadFromFile(coinpath), coinpath);
        coinsound.setBuffer(coinbuffer);
        fs::path healpath = asset_path("powerUp.ogg");
        require_loaded(healbuffer.loadFromFile(healpath), healpath);
        healsound.setBuffer(healbuffer);
        fs::path gameoverpath = asset_path("explosion.ogg");
        require_loaded(gameoverbuffer.loadFromFile(gameoverpath), gameoverpath);
        gameoversound.setBuffer(gameoverbuffer);

        damagesound.setVolume(5);
        coinsound.setVolume(10);
        healsound.setVolume(5);
        gameoversound.setVolume(5);
        music1.setVolume(10);
        music2.setVolume(10);
        music3.setVolume(10);
        music1.setLooping(true);
        music2.setLooping(true);
        music3.setLooping(true);
    }

    void play1()
    {
        music2.stop();
        music3.stop();
        music1.play();
        current_track = 1;
    }

    void stop_music()
    {
        music1.stop();
        music2.stop();
        music3.stop();
    }

    void restart_music()
    {
        stop_music();
        play1();
    }

    void play2()
    {
        music1.stop();
        music3.stop();
        music2.play();
        current_track = 2;
    }

    void play3()
    {
        music1.stop();
        music2.stop();
        music3.play();
        current_track = 3;
    }

    void switch_tracks()
    {
        if (current_track == 1)
        {
            play2();
        }
        else if (current_track == 2)
        {
            play3();
        }
        else
        {
            play1();
        }
    }

    void pause()
    {
        music1.pause();
        music2.pause();
        music3.pause();
        damagesound.pause();
        coinsound.pause();
        healsound.pause();
        gameoversound.pause();
    }

    void resume()
    {
        if (current_track == 1)
            music1.play();
        else if (current_track == 2)
            music2.play();
        else
            music3.play();
        if (damagesound.getStatus() == SoundSource::Status::Paused)
            damagesound.play();
        if (coinsound.getStatus() == SoundSource::Status::Paused)
            coinsound.play();
        if (healsound.getStatus() == SoundSource::Status::Paused)
            healsound.play();
    }

    void stop_effects()
    {
        damagesound.stop();
        coinsound.stop();
        healsound.stop();
        gameoversound.stop();
    }

    void playDamageSound()
    {
        damagesound.play();
    }

    void playCoinSound()
    {
        coinsound.play();
    }

    void playHealSound()
    {
        healsound.play();
    }

    void playGameoverSound()
    {
        gameoversound.play();
    }
};
