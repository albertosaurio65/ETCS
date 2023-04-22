/*
 * European Train Control System
 * Copyright (C) 2019  Iván Izquierdo
 * Copyright (C) 2019-2020  César Benito <cesarbema2009@hotmail.com>
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "volume.h"
#include "../tcp/server.h"
#include <iostream>
#ifdef __ANDROID__
void set_volume(int percent)
{

}
int get_volume()
{
    return 50;
}
#elif defined(_WIN32)
//#include <Windows.h>
void set_volume(int percent)
{
}
int get_volume()
{
    /*DWORD volume;
    // Get the volume for the default playback device
    if (waveOutGetVolume(NULL, &volume) != MMSYSERR_NOERROR) {
        std::cerr << "Error getting system volume." << std::endl;
        return 1;
    }
    // Extract the left and right channel volumes
    WORD leftVolume = LOWORD(volume);
    WORD rightVolume = HIWORD(volume);
    return (leftVolume+rightVolume)/2*100/65536;*/
    return 50;
}
#elif defined(__unix__)
#include <cstdlib>
#include <alsa/asoundlib.h>
void set_volume(int percent)
{
    std::system(("amixer set Master "+std::to_string(percent)+"%").c_str());
}
int get_volume()
{
    long minVolume, maxVolume, currentVolume;
    snd_mixer_t *mixer;
    snd_mixer_elem_t *elem;
    snd_mixer_selem_id_t *sid;
    const char *card = "default"; // Use the default sound card
    const char *selem_name = "Master"; // Use the Master channel

    // Open the mixer
    if (snd_mixer_open(&mixer, 0) < 0) {
        std::cerr << "Error opening mixer." << std::endl;
        return -1;
    }
    // Attach the mixer to the default sound card
    if (snd_mixer_attach(mixer, card) < 0) {
        std::cerr << "Error attaching mixer to sound card." << std::endl;
        snd_mixer_close(mixer);
        return -1;
    }
    snd_mixer_selem_register(mixer, nullptr, nullptr);
    // Load the mixer elements
    if (snd_mixer_load(mixer) < 0) {
        std::cerr << "Error loading mixer elements." << std::endl;
        snd_mixer_close(mixer);
        return -1;
    }
    // Create a mixer element ID for the Master channel
    snd_mixer_selem_id_alloca(&sid);
    snd_mixer_selem_id_set_index(sid, 0); // Use the first device
    snd_mixer_selem_id_set_name(sid, selem_name); // Use the Master channel
    // Find the Master channel element
    elem = snd_mixer_find_selem(mixer, sid);
    if (!elem) {
        std::cerr << "Error finding mixer element." << std::endl;
        snd_mixer_close(mixer);
        return -1;
    }
    // Get the minimum and maximum volume values
    snd_mixer_selem_get_playback_volume_range(elem, &minVolume, &maxVolume);
    // Get the current volume value
    snd_mixer_selem_get_playback_volume(elem, SND_MIXER_SCHN_MONO, &currentVolume);
    // Close the mixer
    snd_mixer_close(mixer);
    // Compute the average volume
    int averageVolume = (int)((currentVolume - minVolume) * 100.0 / (maxVolume - minVolume) + 0.5);
    return (averageVolume/10)*10;
}
#endif
volume_window::volume_window() : input_window("Volume", 1, false)
{
    int vol_orig = get_volume();
    inputs[0] = new input_data("", false);
    inputs[0]->data = std::to_string(vol_orig);
    inputs[0]->setAccepted(true);
    std::vector<Button*> keys;
    for (int i=0; i<12; i++) {
        keys.push_back(nullptr);
    }
    keys[0] = new TextButton(get_text("-"), 102, 50);
    keys[2] = new TextButton(get_text("+"), 102, 50);
    keys[0]->setPressedAction([this]
    {
        int vol = stoi(inputs[0]->data);
        if (vol >= 30) {
            vol -= 10;
            set_volume(vol);
            inputs[0]->setData(std::to_string(vol));
        }
    });
    keys[2]->setPressedAction([this]
    {
        int vol = stoi(inputs[0]->data);
        if (vol <= 90) {
            vol += 10;
            set_volume(vol);
            inputs[0]->setData(std::to_string(vol));
        }
    });
    keys[0]->upType = false;
    keys[2]->upType = false;
    inputs[0]->keys = keys;
    exit_button.setPressedAction([this, vol_orig]
    {
        set_volume(vol_orig);
        write_command("json",R"({"DriverSelection":"CloseWindow"})");
    });
    create();
}
void volume_window::setLayout()
{
    input_window::setLayout();
}
void volume_window::sendInformation()
{
    set_volume(stoi(inputs[0]->data_accepted));
    input_window::sendInformation();
}