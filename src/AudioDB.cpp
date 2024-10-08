#include "AudioDB.h"

bool AudioDB::hasIntroAudio;

void AudioDB::InitAudio()
{
	int frequency = 44100;  // CD quality sound
	Uint16 format = MIX_DEFAULT_FORMAT;  // Let SDL_mixer choose the format (commonly AUDIO_S16SYS)
	int channels = 2;  // Stereo sound
	int chunksize = 2048;  // Buffer size

	if (AudioHelper::Mix_OpenAudio498(frequency, format, channels, chunksize) < 0) {
		exit(0);
	}
}

void AudioDB::LoadIntroAudio(const rapidjson::Document& configDocument)
{
	if (configDocument.HasMember("intro_bgm") && configDocument["intro_bgm"].IsString())
	{
		hasIntroAudio = true;

		AudioDB::PlayBGM(configDocument["intro_bgm"].GetString());
	}
	else
	{
		hasIntroAudio = false;
	}
}

void AudioDB::EndIntroAudio()
{
	if (hasIntroAudio)
	{
		AudioHelper::Mix_HaltChannel498(0);
	}
}

void AudioDB::LoadGameplayAudio(const rapidjson::Document& configDocument)
{
	if (configDocument.HasMember("gameplay_audio") && configDocument["gameplay_audio"].IsString())
	{
		AudioDB::PlayBGM(configDocument["gameplay_audio"].GetString());
	}
}

void AudioDB::PlayBGM(const std::string& audioPath)
{
	// halt current audio playing, then play one specified by audioPath
	AudioHelper::Mix_HaltChannel498(0);

	std::string bgmAudioPath = "resources/audio/";
	if (std::filesystem::exists(bgmAudioPath + audioPath + ".wav"))
	{
		bgmAudioPath += audioPath + ".wav";
	}
	else if (std::filesystem::exists(bgmAudioPath + audioPath + ".ogg"))
	{
		bgmAudioPath += audioPath + ".ogg";
	}
	else
	{
		// Better message to say no background music was specified
		std::cout << "error: failed to play audio clip " + audioPath;
		exit(0);
	}

	Mix_Chunk* bgm = AudioHelper::Mix_LoadWAV498(bgmAudioPath.c_str());
	if (!bgm) {
		std::cout << "error: failed to play audio clip " + audioPath;
		exit(0);
	}

	AudioHelper::Mix_PlayChannel498(0, bgm, -1);
}