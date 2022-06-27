#pragma once

#include <vector>
#include <algorithm>

#include "Sound.h"
#include "SoundEmitter.h"

#include "../../Plugins/OpenAL/include/alc.h"

namespace NCL {
	namespace CSC8503 {	

		using std::vector;

		class SoundEmitter;
		enum SoundPriority;

		struct OALSource {
			ALuint	source;
			bool	inUse;

			OALSource(ALuint src) {
				source = src;
				inUse = false;
			}
		};

		class SoundSystem {
		public:
			static void Initialise(unsigned int channels = 32) {
				instance = new SoundSystem(channels);
			}

			static void Destroy() {
				delete instance;
			}

			static SoundSystem* GetSoundSystem() {
				return instance;
			}

			void		SetListener(GameObject* l) { listener = l; }
			GameObject* GetListener() { return listener; }

			void		AddSoundEmitter(SoundEmitter* s) { emitters.push_back(s); }

			void		ClearSoundEmitters() { emitters.clear(); }

			void		Update(float msec);

			void		SetMasterVolume(float value);

			void		PlayTriggerSound(Sound* s, Vector3 position, float radius = 250, float pitch = 1);
			void		PlayTriggerSound(Sound* s, SoundPriority p, float pitch = 1);

		protected:
			SoundSystem(unsigned int channels = 128);
			~SoundSystem(void);

			void		UpdateListener();

			void		DetachSources(vector<SoundEmitter*>::iterator from, vector<SoundEmitter*>::iterator to);
			void		AttachSources(vector<SoundEmitter*>::iterator from, vector<SoundEmitter*>::iterator to);

			void		CullNodes();

			OALSource* GetSource();

			void		UpdateTemporaryEmitters(float msec);

			vector<OALSource*>	sources;

			vector<SoundEmitter*>	emitters;
			vector<SoundEmitter*>	temporaryEmitters;

			GameObject* listener;

			ALCcontext* context;
			ALCdevice* device;

			float				masterVolume;

			static SoundSystem* instance;
		};
	}
}