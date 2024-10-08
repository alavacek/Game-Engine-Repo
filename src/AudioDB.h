#ifndef AUDIODB_H
#define AUDIODB_H

#include "AudioHelper.h"
#include <filesystem>
#include "rapidjson/filereadstream.h"
#include "rapidjson/document.h"

class AudioDB
{
public:
	static void InitAudio();
	static void LoadIntroAudio(const rapidjson::Document& configDocument);
	static void EndIntroAudio();
	static void LoadGameplayAudio(const rapidjson::Document& configDocument);

	static void PlayBGM(const std::string& audioPath);
private:
	static bool hasIntroAudio;
};

#endif

