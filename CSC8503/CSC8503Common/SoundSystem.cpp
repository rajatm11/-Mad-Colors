#include "SoundSystem.h"
#include "Debug.h"

using namespace NCL::CSC8503;

SoundSystem* SoundSystem::instance = NULL;

SoundSystem::SoundSystem(unsigned int channels) {
	listener		= NULL;
	masterVolume	= 1.0f;

	string msg = "Creating SoundSystem!";
	Debug::PrintToConsole(msg);

	device = alcOpenDevice(NULL);	//Open the 'best' device

	if(!device) {
		Debug::PrintToConsole("Failed to create SoundSystem! (No valid device!)");
	}

	Debug::PrintToConsole("SoundSystem initialised!!");

	context = alcCreateContext(device,NULL);
	alcMakeContextCurrent(context);

	alDistanceModel(AL_LINEAR_DISTANCE_CLAMPED);

	for(unsigned int i = 0; i < channels; ++i) {
		ALuint source;

		alGenSources(1,&source);
		ALenum error = alGetError();

		if(error == AL_NO_ERROR)	{
			sources.push_back(new OALSource(source));
		}
		else{
			break;
		}
	}

	Debug::PrintToConsole("SoundSystem has " + std::to_string(sources.size()) + " channels available!");
}

SoundSystem::~SoundSystem(void)	{
	for(vector<SoundEmitter*>::iterator i = temporaryEmitters.begin(); i != temporaryEmitters.end(); ++i) {
		delete (*i);
	}

	for(vector<OALSource*>::iterator i = sources.begin(); i != sources.end(); ++i) {
		alDeleteSources(1, &(*i)->source);
		delete (*i);
	}

	alcMakeContextCurrent(NULL);
	alcDestroyContext(context);
	alcCloseDevice(device);
}

void SoundSystem::Update(float msec) {
	UpdateListener();
	UpdateTemporaryEmitters(msec);

	//Update values for every node, whether in range or not
	for(vector<SoundEmitter*>::iterator i = emitters.begin(); i != emitters.end(); ++i) {
		if (*i) (*i)->UpdateSoundState(msec);
	}

	CullNodes();	//First off, remove nodes that are too far away

	if(emitters.size() > sources.size()) {
		std::sort(emitters.begin(), emitters.end(), SoundEmitter::CompareNodesByPriority);	//Then sort by priority

		DetachSources(emitters.begin() + (sources.size()+1), emitters.end());		//Detach sources from nodes that won't be covered this frame
		AttachSources(emitters.begin(), emitters.begin() + (sources.size()));	//And attach sources to nodes that WILL be covered this frame
	}
	else {
		AttachSources(emitters.begin(), emitters.end());//And attach sources to nodes that WILL be covered this frame
	}

	emitters.clear();	//We're done for the frame! empty the emitters list
}

void SoundSystem::CullNodes() {
	for(vector<SoundEmitter*>::iterator i = emitters.begin(); i != emitters.end();) {

		float length;

		if((*i)->GetIsGlobal()) {
			length = 0.0f;
		}
		else{
			length = (listener->GetTransform().GetPosition() - 
				(*i)->GetTransform().GetPosition()).Length();
		}
		
		if(length > (*i)->GetRadius() || !(*i)->GetSound() || (*i)->GetTimeLeft() < 0) {
			(*i)->DetachSource();	//Important!
			i = emitters.erase(i);
		}
		else{
			++i;
		}
	}
}

void SoundSystem::DetachSources(vector<SoundEmitter*>::iterator from, vector<SoundEmitter*>::iterator to) {
	for(vector<SoundEmitter*>::iterator i = from; i != to; ++i) {
		(*i)->DetachSource();
	}
}

void SoundSystem::AttachSources(vector<SoundEmitter*>::iterator from, vector<SoundEmitter*>::iterator to) {
	for(vector<SoundEmitter*>::iterator i = from; i != to; ++i) {
		if(!(*i)->GetSource()) {	//Don't attach a new source if we already have one!
			(*i)->AttachSource(GetSource());
		}
	}
}

OALSource* SoundSystem::GetSource() {
	for(vector<OALSource*>::iterator i = sources.begin(); i != sources.end(); ++i) {
		OALSource*s = *i;
		if(!s->inUse) {
			return s;
		}
	}
	return NULL;
}

void SoundSystem::SetMasterVolume(float value)	{
	value = max(0.0f, value);
	value = min(1.0f, value);
	masterVolume = value;
	alListenerf(AL_GAIN,masterVolume);
}

void SoundSystem::UpdateListener() {
	if(listener)	{
		Matrix4 worldMat = listener->GetTransform().GetMatrix();
		Vector3 worldPos = listener->GetTransform().GetPosition();

		Vector3 dirup[2];
		//forward
		dirup[0].x = -worldMat.array[2];
		dirup[0].y = -worldMat.array[6];
		dirup[0].z = -worldMat.array[10];
		//Up
		dirup[1].x = worldMat.array[1];
		dirup[1].y = worldMat.array[5];
		dirup[1].z = worldMat.array[9];

		alListenerfv(AL_POSITION,(float*)&worldPos);
		alListenerfv(AL_ORIENTATION,(float*)&dirup);
	}
}

void SoundSystem::PlayTriggerSound(Sound* s, Vector3 position, float radius, float pitch) {
	SoundEmitter* n = new SoundEmitter();
	n->SetLooping(false);
	n->GetTransform().SetPosition(position);
	n->SetSound(s);
	n->SetPitch(pitch);
	n->SetRadius(radius);
	temporaryEmitters.push_back(n);
}

void SoundSystem::PlayTriggerSound(Sound* s, SoundPriority p, float pitch) {
	SoundEmitter* n = new SoundEmitter();
	n->SetLooping(false);
	n->SetSound(s);
	n->SetIsGlobal(true);
	n->SetPitch(pitch);
	n->SetPriority(p);
	temporaryEmitters.push_back(n);
}

void SoundSystem::UpdateTemporaryEmitters(float msec) {
	for(vector<SoundEmitter*>::iterator i = temporaryEmitters.begin(); i != temporaryEmitters.end(); ) {

		if((*i)->GetTimeLeft() < 0.0f && !(*i)->GetLooping()) {
			delete (*i);
			i = temporaryEmitters.erase(i);	
		}
		else{
			emitters.push_back(*i);
			++i;
		}
	}
}