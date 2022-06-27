#pragma once

#include <vector>
#include <string>
#include "Agent.h"

#include <iostream>
#include <iomanip>
#include <windows.h>
#include <psapi.h>
#pragma comment(lib,"psapi.lib")


struct ExInputResult;
class ImFont;

namespace NCL {
	namespace CSC8503 {
		class Game;

		class GameUI {
		public:
			GameUI(Game* g);
			~GameUI();

			enum class Command {
				START_SINGLEPLAYER,
				START_MULTIPLAYER,
				HOST_SERVER,
				CONNECT_TO_SERVER,
				OPEN_HOW_TO_PLAY,
				EXIT_GAME,
				CONTINUE,
				EXIT_TO_MENU,
				NO_CHANGE
			};

			Command GetFrameCommand() const { return frameCommand; }
			
			void SetPlayer(int playerID, Agent* a) { players[playerID] = a; }
			int GetNumPlayers() const;
			void ResetPlayers();

			void SetAmountLoaded(int val) { amountLoaded = val; }

			char* GetConnectIP() {
				return connectIPStr;
			}

			void UpdateUI(float dt);
			void DrawUI() const;

			bool IsValid() const { return isValid; }
			void SetValid(bool v) { isValid = v; }

			static GameUI* GetGameUI() { return p_self; }

		protected:
			void DrawMenu();
			void DrawLoadingScreen();
			void DrawPauseMenu();
			void DrawMultiplayerConnectMenu();
			void DrawMultiplayerLobbyScreen();
			void DrawHowToPlay();
			void DrawDebug();
			void DrawPlayingUI();
			void Demo();
			void DrawWinOrLose();
			void DrawNothing();
			void DrawSmallPauseBt();

			Game* game;
			static GameUI* p_self;

			void DrawTextCenter(const string& text, float verticalGap = 0);

			Command frameCommand;

			ImFont* debugFont;
			ImFont* defaultFont;
			ImFont* headerFont;
			ImFont* titleFont;

			Agent* players[4];

			Vector4 titleColour;
			Vector4 targetTitleColour;
			Vector4 prevTitleColour;
			float titleColourTimer;
			float titleColourDuration;

			int amountLoaded = 0;

			bool showMultiplayerMenu;
			bool showHowToPlay;

			void EnterIPValue(char val);
			char connectIPStr[16] = "127.0.0.1";
			int ipStrIndex = 9;

			bool isValid = false;
			bool PauseGame = false;

		};
	}
}