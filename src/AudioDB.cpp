#include "AudioDB.h"

std::unordered_map<std::string, Mix_Chunk*> AudioDB::cachedAudio;

void AudioDB::InitAudio()
{
	int frequency = 44100;  // CD quality sound
	Uint16 format = MIX_DEFAULT_FORMAT;  // Let SDL_mixer choose the format (commonly AUDIO_S16SYS)
	int channels = 2;  // Stereo sound
	int chunksize = 2048;  // Buffer size

	if (AudioHelper::Mix_OpenAudio498(frequency, format, channels, chunksize) < 0) 
	{
		exit(0);
	}

	AudioHelper::Mix_AllocateChannels498(50);
}


void AudioDB::LoadGameplayAudio(const rapidjson::Document& configDocument)
{
	if (configDocument.HasMember("gameplay_audio") && configDocument["gameplay_audio"].IsString())
	{
		AudioDB::PlayBGM(configDocument["gameplay_audio"].GetString());
	}
}

void AudioDB::PlayBGM(const std::string& audioClipName)
{
	PlayChannel(0, audioClipName, true);

}

void AudioDB::PlayChannel(int channel, const std::string& audioClipName, bool doesLoop)
{
	// halt current audio playing, then play one specified by audioPath
	AudioHelper::Mix_HaltChannel498(channel);

	Mix_Chunk* audio;

	if (cachedAudio.find(audioClipName) != cachedAudio.end())
	{
		audio = cachedAudio[audioClipName];
	}
	else
	{
		std::string audioPath = "resources/audio/";

		if (std::filesystem::exists(audioPath + audioClipName + ".wav"))
		{
			audioPath += audioClipName + ".wav";
		}
		else if (std::filesystem::exists(audioPath + audioClipName + ".ogg"))
		{
			audioPath += audioClipName + ".ogg";
		}
		else
		{
			// Better message to say no background music was specified
			std::cout << "error: failed to play audio clip " + audioClipName;
			exit(0);
		}

		audio = AudioHelper::Mix_LoadWAV498(audioPath.c_str());
		 
		if (!audio) {
			std::cout << "error: failed to play audio clip " + audioClipName;
			exit(0);
		}

		cachedAudio[audioClipName] = audio;
	}

	if (doesLoop)
	{
		AudioHelper::Mix_PlayChannel498(channel, audio, -1);
	}
	else
	{
		AudioHelper::Mix_PlayChannel498(channel, audio, 0);
	}

}

void AudioDB::HaltChannel(int channel)
{
	AudioHelper::Mix_HaltChannel498(channel);
}

void AudioDB::SetVolume(int channel, float volume)
{
	AudioHelper::Mix_Volume498(channel, static_cast<int>(volume));
}