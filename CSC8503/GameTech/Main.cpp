#include "../../Common/Window.h"

#include "../CSC8503Common/StateMachine.h"
#include "../CSC8503Common/StateTransition.h"
#include "../CSC8503Common/State.h"

#include "../CSC8503Common/BehaviourAction.h"
#include "../CSC8503Common/BehaviourSequence.h"
#include "../CSC8503Common/BehaviourSelector.h"

#include "../CSC8503Common/GameServer.h"
#include "../CSC8503Common/GameClient.h"

#include "../CSC8503Common/NavigationGrid.h"

#include "../CSC8503Common/PushdownState.h"
#include "../CSC8503Common/PushdownMachine.h"

#include "../CSC8503Common/SoundSystem.h"

#include "TutorialGame.h"
#include "Game.h"
#include "NetworkedGame.h"

using namespace NCL;
using namespace CSC8503;

#include <chrono>
#include <thread>

/*

The main function should look pretty familar to you!
We make a window, and then go into a while loop that repeatedly
runs our 'game' until we press escape. Instead of making a 'renderer'
and updating it, we instead make a whole game, and repeatedly update that,
instead.

This time, we've added some extra functionality to the window class - we can
hide or show the

*/

int main() {
	Window*w = Window::CreateGameWindow("CSC8508 - Team 1", 1920, 1080);
	SoundSystem::Initialise();
	
	//Testing Network Integration with new functions added
	//TestNetworking();
	//TestBehaviourTree();
	
	if (!w->HasInitialised()) {
		return -1;
	}	
	srand(time(0));
	w->SetWindowPosition(0, 0);
	w->ShowOSPointer(true);
	w->LockMouseToWindow(true);
	w->SetFullScreen(true);
	
	NetworkedGame* g = new NetworkedGame();
	w->GetTimer()->GetTimeDeltaSeconds(); //Clear the timer so we don't get a large first dt!
	while (g->IsPlaying() && w->UpdateWindow() && !Window::GetKeyboard()->KeyDown(KeyboardKeys::DELETEKEY)) {
		float dt = w->GetTimer()->GetTimeDeltaSeconds();
		if (dt > 0.1f) {
			Debug::PrintToConsole("Skipping large time delta");
			continue; //must have hit a breakpoint or something to have a 1 second frame time!
		}
		if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::PRIOR)) {
			w->ShowConsole(true);
		}
		if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::NEXT)) {
			w->ShowConsole(false);
		}
	
		if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::T)) {
			w->SetWindowPosition(0, 0);
		}
	
		g->UpdateGame(dt);
	
		w->SetTitle("Gametech frame time:" + std::to_string(1000.0f * dt));
	
		//DisplayPathfinding();
	
	}
	Sound::DeleteSounds();
	SoundSystem::Destroy();
	
	Window::DestroyGameWindow();
}