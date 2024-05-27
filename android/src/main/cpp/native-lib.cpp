#include <jni.h>
#include <fluidsynth.h>
#include <unistd.h>
#include <map>

fluid_settings_t* settings = new_fluid_settings();
std::map<int, fluid_synth_t*> synths = {};
std::map<int, fluid_audio_driver_t*> drivers = {};
std::map<int, int> soundfonts = {};
int nextSfId = 1;

// Set default volume
void set_default_volume(fluid_settings_t* settings, double volume) {
    fluid_settings_setnum(settings, "synth.gain", volume);
}

extern "C" JNIEXPORT int JNICALL
Java_com_melihhakanpektas_flutter_1midi_1pro_FlutterMidiProPlugin_loadSoundfont(JNIEnv* env, jclass clazz, jstring path, jint bank, jint program) {
    const char *nativePath = env->GetStringUTFChars(path, nullptr);
    // Create a new settings object for each synth to ensure individual control
    fluid_settings_t* local_settings = new_fluid_settings();
    set_default_volume(local_settings, 1.0); // Set the desired default volume

    synths[nextSfId] = new_fluid_synth(local_settings);
    drivers[nextSfId] = new_fluid_audio_driver(local_settings, synths[nextSfId]);
    int sfId = fluid_synth_sfload(synths[nextSfId], nativePath, 1);
    for (int i = 0; i < 16; i++) {
        fluid_synth_program_select(synths[nextSfId], i, sfId, bank, program);
    }
    env->ReleaseStringUTFChars(path, nativePath);
    soundfonts[nextSfId] = sfId;
    nextSfId++;
    return nextSfId - 1;
}

extern "C" JNIEXPORT void JNICALL
Java_com_melihhakanpektas_flutter_1midi_1pro_FlutterMidiProPlugin_selectInstrument(JNIEnv* env, jclass clazz, jint sfId, jint channel, jint bank, jint program) {
    fluid_synth_program_select(synths[sfId], channel, soundfonts[sfId], bank, program);
}

extern "C" JNIEXPORT void JNICALL
Java_com_melihhakanpektas_flutter_1midi_1pro_FlutterMidiProPlugin_playNote(JNIEnv* env, jclass clazz, jint channel, jint key, jint velocity, jint sfId) {
    fluid_synth_noteon(synths[sfId], channel, key, velocity);
}

extern "C" JNIEXPORT void JNICALL
Java_com_melihhakanpektas_flutter_1midi_1pro_FlutterMidiProPlugin_stopNote(JNIEnv* env, jclass clazz, jint channel, jint key, jint sfId) {
    fluid_synth_noteoff(synths[sfId], channel, key);
}

extern "C" JNIEXPORT void JNICALL
Java_com_melihhakanpektas_flutter_1midi_1pro_FlutterMidiProPlugin_unloadSoundfont(JNIEnv* env, jclass clazz, jint sfId) {
    delete_fluid_audio_driver(drivers[sfId]);
    delete_fluid_synth(synths[sfId]);
    synths.erase(sfId);
    drivers.erase(sfId);
    soundfonts.erase(sfId);
}

extern "C" JNIEXPORT void JNICALL
Java_com_melihhakanpektas_flutter_1midi_1pro_FlutterMidiProPlugin_dispose(JNIEnv* env, jclass clazz) {
    for (auto const& x : synths) {
        delete_fluid_audio_driver(drivers[x.first]);
        delete_fluid_synth(synths[x.first]);
    }
    synths.clear();
    drivers.clear();
    soundfonts.clear();
    delete_fluid_settings(settings);
}