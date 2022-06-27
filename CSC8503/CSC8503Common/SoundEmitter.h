#pragma once

#include "GameObject.h"
#include "Sound.h"
#include "SoundSystem.h"

#define NUM_STREAM_BUFFERS 3

namespace NCL {
	namespace CSC8503 {

		enum SoundPriority {
			SOUNDPRIORTY_LOW,
			SOUNDPRIORITY_MEDIUM,
			SOUNDPRIORITY_HIGH,
			SOUNDPRIORITY_ALWAYS
		};	

		struct OALSource;

		class SoundEmitter : public GameObject {
		public:
			SoundEmitter(void);
			SoundEmitter(Sound* s);
			~SoundEmitter(void);

			void			Reset();

			void			SetSound(Sound* s);
			Sound*			GetSound() { return sound; }

			void			SetPriority(SoundPriority p) { priority = p; }
			SoundPriority	GetPriority() { return priority; }

			void			SetVolume(float value) { volume = min(1.0f, max(0.0f, value)); }
			float			GetVolume() { return volume; }

			void			SetLooping(bool state) { isLooping = state; }
			bool			GetLooping() { return isLooping; }

			void			SetRadius(float value) { radius = max(0.0f, value); }
			float			GetRadius() { return radius; }

			float			GetPitch() { return pitch; }
			void			SetPitch(float value) { pitch = value; }

			bool			GetIsGlobal() { return isGlobal; }
			void			SetIsGlobal(bool value) { isGlobal = value; }

			double			GetTimeLeft() { return timeLeft; }

			OALSource* GetSource() { return oalSource; }

			void			UpdateSoundState(float msec);

			static bool		CompareNodesByPriority(SoundEmitter* a, SoundEmitter* b);

			void			AttachSource(OALSource* s);
			void			DetachSource();

			void			Update(float dt) override;

		protected:
			Sound* sound;
			OALSource* oalSource;
			SoundPriority	priority;
			float			volume;
			float			radius;
			float			pitch;
			bool			isLooping;
			bool			isGlobal;
			double			timeLeft;

			double			streamPos;						
			ALuint			streamBuffers[NUM_STREAM_BUFFERS];
		};
	}
}