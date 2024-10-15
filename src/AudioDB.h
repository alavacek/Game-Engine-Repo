#ifndef AUDIODB_H
#define AUDIODB_H

#include <filesystem>
#include <unordered_map>

#include "AudioHelper.h"
#include "rapidjson/filereadstream.h"
#include "rapidjson/document.h"

class AudioDB
{
public:
	static void InitAudio();
	static void LoadIntroAudio(const rapidjson::Document& configDocument);
	static void EndIntroAudio();
	static void LoadGameplayAudio(const rapidjson::Document& configDocument);

	static void PlayBGM(const std::string& audioClipName);
	static void PlayChannel(int channel, const std::string& audioClipName, bool doesLoop);
	static void HaltChannel(int channel);

private:
	static bool hasIntroAudio;
	static std::unordered_map<std::string, Mix_Chunk*> cachedAudio;
};

#endif

