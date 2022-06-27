#include "GameUI.h"
#include "../../Common/Win32Window.h"
#include "../../Common/Window.h"
#include "../../Common/imgui.h"
#include "../../Common/imgui_impl_win32.h"
#include "../../Common/imgui_impl_opengl3.h"
#include "../../Common/Assets.h"
#include "../CSC8503Common/Debug.h"


#include "Game.h"
#include "NetworkedGame.h"

NCL::CSC8503::GameUI* NCL::CSC8503::GameUI::p_self = nullptr;
NCL::Win32Code::ExInputResult ImguiProcessInput(void* data);

NCL::CSC8503::GameUI::GameUI(Game* g)
{
	frameCommand = Command::NO_CHANGE;
	game = g;
	p_self = this;

	titleColourDuration = 1.5;
	titleColourTimer = 1.5;
	prevTitleColour = Vector4((float)rand() / RAND_MAX, (float)rand() / RAND_MAX, (float)rand() / RAND_MAX, 1);
	targetTitleColour = Vector4((float)rand() / RAND_MAX, (float)rand() / RAND_MAX, (float)rand() / RAND_MAX, 1);

	Win32Code::Win32Window* win32_w = dynamic_cast<Win32Code::Win32Window*>(Window::GetWindow());
	if (!win32_w) return;
	win32_w->SetExtraMsgFunc(ImguiProcessInput);

	//IsValid = true;
	//IsValid = false;

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;

	//io.ConfigFlags = ImGuiConfigFlags_NoMouseCursorChange;
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	//io.Fonts->AddFontFromFileTTF((Assets::FONTSDIR + "Roboto - Medium.ttf").c_str(), 16);
	defaultFont = io.Fonts->AddFontFromFileTTF("../../Assets/Fonts/VioletWasteland-Bgw5.ttf", 30);
	headerFont = io.Fonts->AddFontFromFileTTF("../../Assets/Fonts/VioletWasteland-Bgw5.ttf", 50);
	titleFont = io.Fonts->AddFontFromFileTTF("../../Assets/Fonts/VioletWasteland-Bgw5.ttf", 100);
	debugFont = io.Fonts->AddFontFromFileTTF("../../Assets/Fonts/Roboto-Medium.ttf", 16);
	//io.Fonts->AddFontDefault();

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsClassic();
	//ImGui::StyleColorsLight();

	// Setup Platform/Renderer backends
	//ImGui_ImplWin32_Init(Win32)
	ImGui_ImplWin32_Init(win32_w->GetHandle());
	ImGui_ImplOpenGL3_Init("#version 400");

}

NCL::CSC8503::GameUI::~GameUI()
{
	if (!isValid) return;

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

int NCL::CSC8503::GameUI::GetNumPlayers() const
{
	int numPlayers = 0;
	for (int i = 0; i < 4; ++i) {
		numPlayers += players[i] ? 1 : 0;
	}
	return numPlayers;
}

void NCL::CSC8503::GameUI::ResetPlayers() {
	for (int i = 0; i < 4; ++i) {
		players[i] = nullptr;
	}
}

void NCL::CSC8503::GameUI::UpdateUI(float dt)
{
	frameCommand = Command::NO_CHANGE;
	if (!isValid) return;

	titleColourTimer += dt;
	titleColour = Vector4::Lerp(prevTitleColour, targetTitleColour, titleColourTimer / titleColourDuration);
	if (titleColourTimer >= titleColourDuration) {
		prevTitleColour = titleColour;
		targetTitleColour = Vector4((float)rand() / RAND_MAX, (float)rand() / RAND_MAX, (float)rand() / RAND_MAX, 1);
		titleColourTimer = 0;
	}

	if (game->GetState() == Game::State::MAIN_MENU) {
		if (showMultiplayerMenu) {
			DrawMultiplayerConnectMenu();
		}
		else if (showHowToPlay) {
			DrawHowToPlay();
		}
		else {
			DrawMenu();
		}
	}
	else if (game->GetState() == Game::State::WAITING) {
		DrawMultiplayerLobbyScreen();
	}
	else if (game->GetState() == Game::State::LOADING) {
		DrawLoadingScreen();
	}
	else if (game->GetState() == Game::State::PAUSED) {
		game->gameOver ? DrawWinOrLose() :DrawPauseMenu();
	}
	else if (game->GetState() == Game::State::PLAYING) {
		if (Debug::IsActive()) {
			DrawDebug();
		}
		else {
			DrawPlayingUI();
		}
	}
	else {
		DrawNothing();
	}
}

void NCL::CSC8503::GameUI::DrawUI() const
{
	if (!isValid) return;

	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void NCL::CSC8503::GameUI::DrawMenu() {
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	//ImGui::ShowDemoWindow();

	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings;
	const ImGuiViewport* main_viewport = ImGui::GetMainViewport();

	//Draw background begin
	ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x, main_viewport->WorkPos.y), ImGuiCond_Always);
	ImGui::SetNextWindowSize(ImVec2(main_viewport->Size.x, main_viewport->Size.y), ImGuiCond_Always);
	if (!ImGui::Begin("Pause Background", NULL, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoSavedSettings))
	{
		ImGui::End();
		return;
	}
	//Draw background end

	ImGui::PushFont(titleFont);
	ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(titleColour.x * 255, titleColour.y * 255, titleColour.z * 255, 255));
	ImGui::SetCursorPos(ImVec2(main_viewport->GetCenter().x - ImGui::CalcTextSize("Mad Colours").x * 0.5, main_viewport->GetCenter().y - 300));
	ImGui::Text("Mad Colours");
	ImGui::PopFont();
	ImGui::PopStyleColor();

	//Draw menu begin
	ImGui::SetNextWindowPos(ImVec2(main_viewport->GetCenter().x - 150, main_viewport->GetCenter().y + 50), ImGuiCond_Always);
	ImGui::SetNextWindowSize(ImVec2(300, 100), ImGuiCond_Always);
	ImGui::BeginChild("Start Menu", ImVec2(300, 230), true, ImGuiWindowFlags_NoSavedSettings);
	float contentWidth = ImGui::GetWindowContentRegionWidth();
	float contentheight = ImGui::GetWindowHeight();

	//set "Start Game button"
	if (ImGui::Button("Singleplayer", ImVec2(contentWidth, 50))) {
		frameCommand = Command::START_SINGLEPLAYER;
		SoundSystem::GetSoundSystem()->PlayTriggerSound(Sound::GetSound("menuclick.wav"), SoundPriority::SOUNDPRIORITY_HIGH);
	}

	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.070, 0.6, 0, 1));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.117, 0.980, 0, 1));
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.258, 1, 0.313, 1));
	if (ImGui::Button("Multiplayer", ImVec2(contentWidth, 50))) {
		frameCommand = Command::START_MULTIPLAYER;
		showMultiplayerMenu = true;
		SoundSystem::GetSoundSystem()->PlayTriggerSound(Sound::GetSound("menuclick.wav"), SoundPriority::SOUNDPRIORITY_HIGH);
	}
	ImGui::PopStyleColor(3);

	//set "setting button" color
	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.80, 0.4, 0.00, 1));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1, 0.50, 0.00, 1));
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(1, 0.65, 0.0, 1));
	if (ImGui::Button("How to Play", ImVec2(contentWidth, 50))) {
		showHowToPlay = true;
		SoundSystem::GetSoundSystem()->PlayTriggerSound(Sound::GetSound("menuclick.wav"), SoundPriority::SOUNDPRIORITY_HIGH);
	}
	ImGui::PopStyleColor(3);

	//set "Exit Game" color
	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.3, 0.05, 0.05, 1));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.6, 0.1, 0.1, 1));
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.9, 0.1, 0.1, 1));
	//ImGui::Button("Exit Game", ImVec2(contentWidth, 50));
	if (ImGui::Button("Exit Game", ImVec2(contentWidth, 50))) {
		frameCommand = Command::EXIT_GAME;
		SoundSystem::GetSoundSystem()->PlayTriggerSound(Sound::GetSound("menuclick.wav"), SoundPriority::SOUNDPRIORITY_HIGH);
	}
	ImGui::PopStyleColor(3);

	ImGui::EndChild();
	//draw menu end


	ImGui::End();
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void NCL::CSC8503::GameUI::DrawLoadingScreen() {
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings;
	const ImGuiViewport* main_viewport = ImGui::GetMainViewport();

	//Draw background begin
	ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x, main_viewport->WorkPos.y), ImGuiCond_Always);
	ImGui::SetNextWindowSize(ImVec2(main_viewport->Size.x, main_viewport->Size.y), ImGuiCond_Always);
	if (!ImGui::Begin("Pause Background", NULL, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoSavedSettings))
	{
		ImGui::End();
		return;
	}

	ImGui::PushFont(titleFont);
	ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 119, 225, 255));
	ImGui::SetCursorPos(ImVec2(main_viewport->GetCenter().x - ImGui::CalcTextSize("Mad Colours").x * 0.5, main_viewport->GetCenter().y - 300));
	ImGui::Text("Mad Colours");
	ImGui::PopFont();
	ImGui::PopStyleColor();

	ImGui::PushFont(headerFont);
	string loadingStr = "Loading Game";
	ImGui::SetCursorPos(ImVec2(main_viewport->GetCenter().x - ImGui::CalcTextSize(loadingStr.c_str()).x * 0.5, main_viewport->GetCenter().y));
	ImGui::Text(loadingStr.c_str());
	ImGui::PopFont();

	ImGui::SetCursorPos(ImVec2(main_viewport->GetCenter().x - 200, main_viewport->GetCenter().y + 75));
	ImGui::ProgressBar(amountLoaded / 100.0f, ImVec2(400, 50));

	ImGui::End();
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void NCL::CSC8503::GameUI::DrawPauseMenu() {
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	//ImGui::ShowDemoWindow();

	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings;
	const ImGuiViewport* main_viewport = ImGui::GetMainViewport();

	//Draw background begin
	ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x, main_viewport->WorkPos.y), ImGuiCond_Always);
	ImGui::SetNextWindowSize(ImVec2(main_viewport->Size.x, main_viewport->Size.y), ImGuiCond_Always);
	if (!ImGui::Begin("Pause Background", NULL, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoSavedSettings))
	{
		ImGui::End();
		return;
	}
	//Draw background end

	ImGui::PushFont(titleFont);
	ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(titleColour.x * 255, titleColour.y * 255, titleColour.z * 255, 255));
	ImGui::SetCursorPos(ImVec2(main_viewport->GetCenter().x - ImGui::CalcTextSize("Paused").x * 0.5, main_viewport->GetCenter().y - 200));
	ImGui::Text("Paused");
	ImGui::PopFont();
	ImGui::PopStyleColor();

	//Draw menu begin
	ImGui::SetNextWindowPos(ImVec2(main_viewport->GetCenter().x - 150, main_viewport->GetCenter().y + 50), ImGuiCond_Always);
	ImGui::SetNextWindowSize(ImVec2(300, 100), ImGuiCond_Always);
	ImGui::BeginChild("Start Menu", ImVec2(300, 120), true, ImGuiWindowFlags_NoSavedSettings);
	float contentWidth = ImGui::GetWindowContentRegionWidth();
	float contentheight = ImGui::GetWindowHeight();

	//set "Continue Game" button
	if (ImGui::Button("Continue", ImVec2(contentWidth, 50))) {
		frameCommand = Command::CONTINUE;
		SoundSystem::GetSoundSystem()->PlayTriggerSound(Sound::GetSound("menuclick.wav"), SoundPriority::SOUNDPRIORITY_HIGH);
	}

	//set "Exit" color
	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.3, 0.05, 0.05, 1));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.6, 0.1, 0.1, 1));
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.9, 0.1, 0.1, 1));
	//ImGui::Button("Exit Game", ImVec2(contentWidth, 50));
	if (ImGui::Button("Exit", ImVec2(contentWidth, 50))) {
		frameCommand = Command::EXIT_TO_MENU;
		SoundSystem::GetSoundSystem()->PlayTriggerSound(Sound::GetSound("menuclick.wav"), SoundPriority::SOUNDPRIORITY_HIGH);
	}
	ImGui::PopStyleColor(3);

	ImGui::EndChild();


	ImGui::End();
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void NCL::CSC8503::GameUI::DrawMultiplayerConnectMenu() {
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings;
	const ImGuiViewport* main_viewport = ImGui::GetMainViewport();

	//Draw background begin
	ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x, main_viewport->WorkPos.y), ImGuiCond_Always);
	ImGui::SetNextWindowSize(ImVec2(main_viewport->Size.x, main_viewport->Size.y), ImGuiCond_Always);
	if (!ImGui::Begin("Pause Background", NULL, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoSavedSettings))
	{
		ImGui::End();
		return;
	}

	ImGui::PushFont(titleFont);
	ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(titleColour.x * 255, titleColour.y * 255, titleColour.z * 255, 255));
	ImGui::SetCursorPos(ImVec2(main_viewport->GetCenter().x - ImGui::CalcTextSize("Mad Colours").x * 0.5, main_viewport->GetCenter().y - 300));
	ImGui::Text("Mad Colours");
	ImGui::PopFont();
	ImGui::PopStyleColor();

	//Draw menu begin
	ImGui::SetNextWindowPos(ImVec2(main_viewport->GetCenter().x - 150, main_viewport->GetCenter().y - 50), ImGuiCond_Always);
	ImGui::SetNextWindowSize(ImVec2(300, 100), ImGuiCond_Always);
	ImGui::BeginChild("Start Menu", ImVec2(300, 480), true, ImGuiWindowFlags_NoSavedSettings);
	float contentWidth = ImGui::GetWindowContentRegionWidth();
	float contentheight = ImGui::GetWindowHeight();

	if (ImGui::Button("Host as Server", ImVec2(contentWidth, 50))) {
		frameCommand = Command::HOST_SERVER;
		showMultiplayerMenu = false;
		SoundSystem::GetSoundSystem()->PlayTriggerSound(Sound::GetSound("menuclick.wav"), SoundPriority::SOUNDPRIORITY_HIGH);
	}

	ImGui::Separator();

	ImGui::Text("Enter Host IP: "); 
	ImGui::InputText(" ", connectIPStr, IM_ARRAYSIZE(connectIPStr));

	ImGui::Separator();

	int buttonWidth = 90;

	if (ImGui::Button("1", ImVec2(buttonWidth, 50))) {
		EnterIPValue('1');
	}
	ImGui::SameLine();
	if (ImGui::Button("2", ImVec2(buttonWidth, 50))) {
		EnterIPValue('2');
	}
	ImGui::SameLine();
	if (ImGui::Button("3", ImVec2(buttonWidth, 50))) {
		EnterIPValue('3');
	}

	if (ImGui::Button("4", ImVec2(buttonWidth, 50))) {
		EnterIPValue('4');
	}
	ImGui::SameLine();
	if (ImGui::Button("5", ImVec2(buttonWidth, 50))) {
		EnterIPValue('5');
	}
	ImGui::SameLine();
	if (ImGui::Button("6", ImVec2(buttonWidth, 50))) {
		EnterIPValue('6');
	}

	if (ImGui::Button("7", ImVec2(buttonWidth, 50))) {
		EnterIPValue('7');
	}
	ImGui::SameLine();
	if (ImGui::Button("8", ImVec2(buttonWidth, 50))) {
		EnterIPValue('8');
	}
	ImGui::SameLine();
	if (ImGui::Button("9", ImVec2(buttonWidth, 50))) {
		EnterIPValue('9');
	}

	if (ImGui::Button(".", ImVec2(buttonWidth, 50))) {
		EnterIPValue('.');
	}
	ImGui::SameLine();
	if (ImGui::Button("0", ImVec2(buttonWidth, 50))) {
		EnterIPValue('0');
	}
	ImGui::SameLine();
	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.3, 0.05, 0.05, 1));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.6, 0.1, 0.1, 1));
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.9, 0.1, 0.1, 1));
	if (ImGui::Button("Del", ImVec2(buttonWidth, 50)) && ipStrIndex > 0) {
		ipStrIndex--;
		connectIPStr[ipStrIndex] = ' ';
		SoundSystem::GetSoundSystem()->PlayTriggerSound(Sound::GetSound("menuclick.wav"), SoundPriority::SOUNDPRIORITY_HIGH);
	}
	ImGui::PopStyleColor(3);

	ImGui::Separator();

	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.070, 0.6, 0, 1));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.117, 0.980, 0, 1));
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.258, 1, 0.313, 1));
	if (ImGui::Button("Connect", ImVec2(contentWidth, 50))) {
		if (((NetworkedGame*)game)->ConnectClient(string(connectIPStr))) {
			frameCommand = Command::CONNECT_TO_SERVER;
			showMultiplayerMenu = false;
		}
		SoundSystem::GetSoundSystem()->PlayTriggerSound(Sound::GetSound("menuclick.wav"), SoundPriority::SOUNDPRIORITY_HIGH);
	}
	ImGui::PopStyleColor(3);

	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.3, 0.05, 0.05, 1));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.6, 0.1, 0.1, 1));
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.9, 0.1, 0.1, 1));
	if (ImGui::Button("Cancel", ImVec2(contentWidth, 50))) {
		ipStrIndex = 0;
		connectIPStr[0] = '1'; ipStrIndex++;
		connectIPStr[1] = '2'; ipStrIndex++;
		connectIPStr[2] = '7'; ipStrIndex++;
		connectIPStr[3] = '.'; ipStrIndex++;
		connectIPStr[4] = '0'; ipStrIndex++;
		connectIPStr[5] = '.'; ipStrIndex++;
		connectIPStr[6] = '0'; ipStrIndex++;
		connectIPStr[7] = '.'; ipStrIndex++;
		connectIPStr[8] = '1'; ipStrIndex++;
		showMultiplayerMenu = false;
		SoundSystem::GetSoundSystem()->PlayTriggerSound(Sound::GetSound("menuclick.wav"), SoundPriority::SOUNDPRIORITY_HIGH);
	}
	ImGui::PopStyleColor(3);

	ImGui::EndChild();

	ImGui::End();
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void NCL::CSC8503::GameUI::DrawMultiplayerLobbyScreen() {
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings;
	const ImGuiViewport* main_viewport = ImGui::GetMainViewport();

	//Draw background begin
	ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x, main_viewport->WorkPos.y), ImGuiCond_Always);
	ImGui::SetNextWindowSize(ImVec2(main_viewport->Size.x, main_viewport->Size.y), ImGuiCond_Always);
	if (!ImGui::Begin("Pause Background", NULL, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoSavedSettings))
	{
		ImGui::End();
		return;
	}

	ImGui::PushFont(titleFont);
	ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(titleColour.x * 255, titleColour.y * 255, titleColour.z * 255, 255));
	ImGui::SetCursorPos(ImVec2(main_viewport->GetCenter().x - ImGui::CalcTextSize("Mad Colours").x * 0.5, main_viewport->GetCenter().y - 300));
	ImGui::Text("Mad Colours");
	ImGui::PopFont();
	ImGui::PopStyleColor();

	bool isServer = ((NetworkedGame*)game)->thisServer;

	string serverStr;
	if (isServer) {
		if (GetNumPlayers() < 2) {
			serverStr = "Waiting for more players...";
		}
		else {
			serverStr = "Press the 'Start Game' button when ready";
		}
	}
	else {
		serverStr = "Waiting for server to begin game";
	}
	string playersStr = "Players connected: " + std::to_string(GetNumPlayers()) + "/4";
	ImGui::SetCursorPos(ImVec2(main_viewport->GetCenter().x - ImGui::CalcTextSize(serverStr.c_str()).x * 0.5, main_viewport->GetCenter().y + 50));
	ImGui::Text(serverStr.c_str());
	ImGui::SetCursorPos(ImVec2(main_viewport->GetCenter().x - ImGui::CalcTextSize(playersStr.c_str()).x * 0.5, main_viewport->GetCenter().y + 80));
	ImGui::Text(playersStr.c_str());


	if (GetNumPlayers() > 1 && isServer) {
		//Draw menu begin
		ImGui::SetNextWindowPos(ImVec2(main_viewport->GetCenter().x - 150, main_viewport->GetCenter().y + 150), ImGuiCond_Always);
		ImGui::SetNextWindowSize(ImVec2(300, 100), ImGuiCond_Always);
		ImGui::BeginChild("Start Menu", ImVec2(300, 70), true, ImGuiWindowFlags_NoSavedSettings);
		float contentWidth = ImGui::GetWindowContentRegionWidth();
		float contentheight = ImGui::GetWindowHeight();

		if (ImGui::Button("Start Game", ImVec2(280, 50))) {
			frameCommand = Command::START_MULTIPLAYER;
		}

		ImGui::EndChild();
	}
	
	ImGui::End();
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void NCL::CSC8503::GameUI::DrawHowToPlay() {
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings;
	const ImGuiViewport* main_viewport = ImGui::GetMainViewport();

	//Draw background begin
	ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x, main_viewport->WorkPos.y), ImGuiCond_Always);
	ImGui::SetNextWindowSize(ImVec2(main_viewport->Size.x, main_viewport->Size.y), ImGuiCond_Always);
	if (!ImGui::Begin("Pause Background", NULL, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoSavedSettings))
	{
		ImGui::End();
		return;
	}

	ImGui::PushFont(titleFont);
	ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(titleColour.x * 255, titleColour.y * 255, titleColour.z * 255, 255));
	ImGui::SetCursorPos(ImVec2(main_viewport->GetCenter().x - ImGui::CalcTextSize("Mad Colours").x * 0.5, main_viewport->GetCenter().y - 300));
	ImGui::Text("Mad Colours");
	ImGui::PopFont();
	ImGui::PopStyleColor();

	//Draw menu begin
	ImGui::SetNextWindowPos(ImVec2(main_viewport->GetCenter().x - 400, main_viewport->GetCenter().y - 150), ImGuiCond_Always);
	ImGui::SetNextWindowSize(ImVec2(800, 100), ImGuiCond_Always);
	ImGui::BeginChild("Start Menu", ImVec2(800, 650), true, ImGuiWindowFlags_NoSavedSettings);
	float contentWidth = ImGui::GetWindowContentRegionWidth();
	float contentheight = ImGui::GetWindowHeight();

	ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 140, 0, 255));
	ImGui::PushFont(headerFont);
	auto windowWidth = ImGui::GetWindowSize().x;
	auto textWidth = ImGui::CalcTextSize("How To Play").x;
	ImGui::SetCursorPos(ImVec2((windowWidth - textWidth) * 0.5f, ImGui::GetCursorPosY() + 10));
	ImGui::Text("How To Play");
	ImGui::PopStyleColor();
	ImGui::PopFont();
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 10);
	ImGui::Separator();

	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 20);
	ImGui::PushFont(debugFont);
	DrawTextCenter("-Mad colours is a game primarily focused on offense and defense.");
	DrawTextCenter("-Each player begins with a wall comprised of uncoloured blocks, which can either be coloured or de-coloured.", 10);
	DrawTextCenter("-The objective of the game is to ensure that you have the most blocks painted out of each player by the time", 10);
	DrawTextCenter("the match is finished (90 seconds), by colouring your own wall to gain points yourself, and de-colouring opponent walls to");
	DrawTextCenter("opponent walls to reduce their points (shown in the bottom right).");
	DrawTextCenter("-Each player starts with 12 paint balls, which can be fired as coloured paint (left mouse button) or", 10);
	DrawTextCenter("de-colour paint (right mouse button). Other players can be shot at, and will need to respawn if their health");
	DrawTextCenter("falls to zero. This can be useful when defending your own wall, or attacking another. Once you have ran out of paint");
	DrawTextCenter("balls, you must collect paint from a refill point. Refill points are shown as spinning paint guns, and are either");
	DrawTextCenter("green meaning available, or red meaning unavailable (when recently collected by another player).");
	ImGui::PopFont();

	ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 140, 0, 255));
	ImGui::PushFont(headerFont);
	textWidth = ImGui::CalcTextSize("Controls").x;
	ImGui::SetCursorPos(ImVec2((windowWidth - textWidth) * 0.5f, ImGui::GetCursorPosY() + 10));
	ImGui::Text("Controls");
	ImGui::PopStyleColor();
	ImGui::PopFont();
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 10);
	ImGui::Separator();

	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 20);
	ImGui::PushFont(debugFont);
	ImGui::SetCursorPosX((ImGui::GetWindowWidth() * 0.5f) - 150);
	ImGui::Text("[MOUSE] - Look around");
	ImGui::SetCursorPosX((ImGui::GetWindowWidth() * 0.5f) - 150);
	ImGui::Text("[LEFT MOUSE BUTTON] - Shoot colour paint ball");
	ImGui::SetCursorPosX((ImGui::GetWindowWidth() * 0.5f) - 150);
	ImGui::Text("[RIGHT MOUSE BUTTON] - Shoot de-colour paint ball");
	ImGui::SetCursorPosX((ImGui::GetWindowWidth() * 0.5f) - 150);
	ImGui::Text("[WASD] - Move");
	ImGui::SetCursorPosX((ImGui::GetWindowWidth() * 0.5f) - 150);
	ImGui::Text("[SPACE] - Jump");
	ImGui::SetCursorPosX((ImGui::GetWindowWidth() * 0.5f) - 150);
	ImGui::Text("[ESC] - Pause (only in singleplayer)");
	ImGui::PopFont();

	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.3, 0.05, 0.05, 1));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.6, 0.1, 0.1, 1));
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.9, 0.1, 0.1, 1));
	ImGui::SetCursorPos(ImVec2((ImGui::GetWindowWidth() - 300) * 0.5f, ImGui::GetWindowHeight() - 70));
	if (ImGui::Button("Close", ImVec2(300, 50))) {
		showHowToPlay = false;
		SoundSystem::GetSoundSystem()->PlayTriggerSound(Sound::GetSound("menuclick.wav"), SoundPriority::SOUNDPRIORITY_HIGH);
	}
	ImGui::PopStyleColor(3);

	ImGui::EndChild();

	ImGui::End();
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void NCL::CSC8503::GameUI::DrawDebug() {
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoSavedSettings;
	const ImGuiViewport* main_viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x, main_viewport->WorkPos.y), ImGuiCond_Always);
	ImGui::SetNextWindowSize(ImVec2(500, 500), ImGuiCond_Always);

	ImGui::PushFont(debugFont);
	ImGui::Begin("DebugMode", NULL, window_flags);

	if (!ImGui::CollapsingHeader("Performance & Stats")) {
		ImGui::Text("FPS: %5.3lf", Debug::GetFPS());
		ImGui::Text("Memory Usage (MB): %4.5lf", Debug::GetMemoryUsage());
		string text = "Total GameObjects: " + (to_string(Debug::GetNumTotalObjects()));
		ImGui::Text(text.c_str());
		text = "Num Broadphase Collisions: " + (to_string(Debug::GetNumBroadphaseCollisions()));
		ImGui::Text(text.c_str());
		text = "Num Narrowphase Collisions: " + (to_string(Debug::GetNumNarrowphaseCollisions()));
		ImGui::Text(text.c_str());
	}
	if (!ImGui::CollapsingHeader("Toggles")) {
		
		static bool fullscreen = Debug::GetFullScreen();
		if (ImGui::Checkbox("Fullscreen", &fullscreen)) {
			Window::GetWindow()->SetFullScreen(fullscreen);
			Debug::SetFullScreen(fullscreen);
		}

		static bool freeCam = Debug::GetFreeCam();
		if (ImGui::Checkbox("Free Cam Enabled", &freeCam)) {
			Debug::SetFreeCam(freeCam);
		}

		static bool gravityEnabled = Debug::GetApplyGravity();
		if (ImGui::Checkbox("Gravity Enabled", &gravityEnabled)) {
			Debug::SetApplyGravity(gravityEnabled);
		}

		static bool aiActive = Debug::GetAIActive();
		if (ImGui::Checkbox("AI Active", &aiActive)) {
			Debug::SetAIActive(aiActive);
		}

		static bool collisionVolShow = Debug::GetShowCollisionVolumes();
		if (ImGui::Checkbox("Show Collision Volumes", &collisionVolShow)) {
			Debug::SetShowCollisionVolumes(collisionVolShow);
		}

		static bool collisionMeshShow = Debug::GetShowCollisionMeshes();
		if (ImGui::Checkbox("Show Collision Meshes", &collisionMeshShow)) {
			Debug::SetShowCollisionMeshes(collisionMeshShow);
		}
	}
	if (!ImGui::CollapsingHeader("Selected Object Debugging")) {
		GameObject* selectedObject = Debug::GetSelectedObject();
		if (selectedObject) {
			string text = "Name: ";
			ImGui::Text(text.c_str()); ImGui::SameLine(); ImGui::Text((selectedObject->GetName() == "" ? "No Name" : selectedObject->GetName()).c_str());
			Vector3 objPos = selectedObject->GetTransform().GetPosition();
			text = "Position: (" + to_string(objPos.x) + ", " + to_string(objPos.y) + ", " + to_string(objPos.z) + ")";
			ImGui::Text(text.c_str());
			Quaternion objRot = selectedObject->GetTransform().GetOrientation();
			text = "Orientation: (" + to_string(objRot.x) + ", " + to_string(objRot.y) + ", " + to_string(objRot.z) + ", " + to_string(objRot.w) + ")";
			ImGui::Text(text.c_str());
			Vector3 objScale = selectedObject->GetTransform().GetScale();
			text = "Scale: (" + to_string(objScale.x) + ", " + to_string(objScale.y) + ", " + to_string(objScale.z) + ")";
			ImGui::Text(text.c_str());
			ImGui::Separator();
			ImGui::Text("AI Infomation");
			Opponent* o = dynamic_cast<Opponent*>(selectedObject);
			if (o) {
				text = "Current State: " + o->GetActiveStateName();
				ImGui::Text(text.c_str());
				text = "Previous State: " + o->GetPrevStateName();
				ImGui::Text(text.c_str());
			}
		}
		else {
			ImGui::Text("No Object Selected");
		}
	}

	ImGui::PopFont();
	ImGui::End();
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void NCL::CSC8503::GameUI::DrawPlayingUI() {
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings;
	const ImGuiViewport* main_viewport = ImGui::GetMainViewport();

	//Draw background begin
	ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x, main_viewport->WorkPos.y), ImGuiCond_Always);
	ImGui::SetNextWindowSize(ImVec2(main_viewport->Size.x, main_viewport->Size.y), ImGuiCond_Always);
	if (!ImGui::Begin("Pause Background", NULL, ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoSavedSettings))
	{
		ImGui::End();
		return;
	}

	ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 119, 255, 255));
	ImGui::PushFont(headerFont);
	string timeRemaningString = "Time Remaining: " + std::to_string((int)game->gameTimer);
	ImGui::SetCursorPos(ImVec2(main_viewport->GetCenter().x - ImGui::CalcTextSize(timeRemaningString.c_str()).x * 0.5, 15));
	ImGui::Text(timeRemaningString.c_str());
	ImGui::PopStyleColor();
	ImGui::PopFont();

	string paintBallString = "Paint Balls: " + std::to_string(players[0] ? players[0]->GetAmmo() : 0);
	ImGui::SetCursorPos(ImVec2(25, main_viewport->WorkSize.y - 50));
	ImGui::Text(paintBallString.c_str());

	string healthString = "Health: " + std::to_string(players[0] ? players[0]->GetHealth() : 100);
	ImGui::SetCursorPos(ImVec2(25, main_viewport->WorkSize.y - 100));
	ImGui::Text(healthString.c_str());

	string playerWallString = "Your Blocks Remaining: " + std::to_string(players[0] ? players[0]->GetNumBlocksRemaining() : 0);
	ImGui::SetCursorPos(ImVec2(main_viewport->WorkSize.x - ImGui::CalcTextSize(playerWallString.c_str()).x - 50, main_viewport->WorkSize.y - 200));
	ImGui::Text(playerWallString.c_str());

	for (int i = 1; i < 4; ++i) {
		if (players[i]) {
			string opponentWallString = "P" + std::to_string(players[i]->GetID() + 1) + " Blocks Remaining: " + std::to_string(players[i] ? players[i]->GetNumBlocksRemaining() : 0);
			ImGui::SetCursorPos(ImVec2(main_viewport->WorkSize.x - ImGui::CalcTextSize(opponentWallString.c_str()).x - 50, main_viewport->WorkSize.y - 200 + (i * 50)));
			ImGui::Text(opponentWallString.c_str());
		}
	}

	if (players[0] && players[0]->IsRespawning()) {
		ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 18, 0, 255));
		ImGui::PushFont(headerFont);
		string timeRemaningString = "Respawning...";
		ImGui::SetCursorPos(ImVec2(main_viewport->GetCenter().x - ImGui::CalcTextSize(timeRemaningString.c_str()).x * 0.5, main_viewport->GetCenter().y));
		ImGui::Text(timeRemaningString.c_str());
		ImGui::PopStyleColor();
		ImGui::PopFont();
	}

	ImGui::End();
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void NCL::CSC8503::GameUI::DrawWinOrLose() {
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	//here
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings;
	const ImGuiViewport* main_viewport = ImGui::GetMainViewport();

	//Draw background begin
	ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x, main_viewport->WorkPos.y), ImGuiCond_Always);
	ImGui::SetNextWindowSize(ImVec2(main_viewport->Size.x, main_viewport->Size.y), ImGuiCond_Always);
	if (!ImGui::Begin("Pause Background", NULL, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoSavedSettings))
	{
		ImGui::End();
		return;
	}

	ImGui::PushFont(titleFont);
	ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(titleColour.x * 255, titleColour.y * 255, titleColour.z * 255, 255));
	ImGui::SetCursorPos(ImVec2(main_viewport->GetCenter().x - ImGui::CalcTextSize("Game Over").x * 0.5, main_viewport->GetCenter().y - 200));
	ImGui::Text("Game Over");
	ImGui::PopFont();
	ImGui::PopStyleColor();

	string gameOverString;
	if (game->winners.size() == 1) {
		if (game->winners[0]->GetID() == players[0]->GetID()) {
			gameOverString = "You win!";
		}
		else {
			gameOverString = "Player " + std::to_string(game->winners[0]->GetID() + 1) + " wins!";
		}
	}
	else if (game->winners.size() == GetNumPlayers()) {
		gameOverString = "Tie between all players";
	}
	else {
		gameOverString = "Tie between ";
		for (int i = 0; i < game->winners.size(); ++i) {
			gameOverString += "player " + to_string((game->winners[i]->GetID() + 1));
			if (i != game->winners.size() - 1) {
				gameOverString += " and ";
			}
		}
	}

	ImGui::SetCursorPos(ImVec2(main_viewport->GetCenter().x - ImGui::CalcTextSize(gameOverString.c_str()).x * 0.5, main_viewport->GetCenter().y - 50));
	ImGui::Text(gameOverString.c_str());

	//Draw background end
	//Draw WinOrLose begin
	ImGui::SetNextWindowPos(ImVec2(main_viewport->GetCenter().x - 150, main_viewport->GetCenter().y + 50), ImGuiCond_Always);
	ImGui::SetNextWindowSize(ImVec2(300, 100), ImGuiCond_Always);
	ImGui::BeginChild("WinOrLose Screen", ImVec2(300, 68), true, ImGuiWindowFlags_NoSavedSettings);
	float contentWidth = ImGui::GetWindowContentRegionWidth();
	float contentheight = ImGui::GetWindowHeight();

	//set "Back to Menu button"
	if (ImGui::Button("Back to Menu", ImVec2(contentWidth, 50))) {//use system color blue
		frameCommand = Command::EXIT_TO_MENU;
		SoundSystem::GetSoundSystem()->PlayTriggerSound(Sound::GetSound("menuclick.wav"), SoundPriority::SOUNDPRIORITY_HIGH);
	}

	ImGui::EndChild();
	//Draw WinOrLose begin

	ImGui::End();
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void NCL::CSC8503::GameUI::DrawNothing() {
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void NCL::CSC8503::GameUI::DrawSmallPauseBt() {
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar;//ImGuiWindowFlags_MenuBar
	const ImGuiViewport* main_viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x, main_viewport->WorkPos.y), ImGuiCond_Always);
	ImGui::SetNextWindowSize(ImVec2(46, 1), ImGuiCond_Always);

	ImGui::Begin(" ", NULL, window_flags);
	ImGui::SetWindowFontScale(0.6);//font size

	if (PauseGame == true) {
		if (ImGui::Button("Play", ImVec2(30, 15))) {
			Debug::PrintToConsole("wwwPlay");
			PauseGame = false;
		}
	}
	else {
		if (ImGui::Button("Pause", ImVec2(30, 15))) {
			Debug::PrintToConsole("wwwPause");
			PauseGame = true;
		}
	}

	ImGui::End();
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void NCL::CSC8503::GameUI::EnterIPValue(char val) {
	if (ipStrIndex < 15) {
		connectIPStr[ipStrIndex] = val;
		ipStrIndex++;
	}
	SoundSystem::GetSoundSystem()->PlayTriggerSound(Sound::GetSound("menuclick.wav"), SoundPriority::SOUNDPRIORITY_HIGH);
}

void NCL::CSC8503::GameUI::Demo() {
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	ImGui::ShowDemoWindow();

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
NCL::Win32Code::ExInputResult ImguiProcessInput(void* data)
{
	ImGuiIO& io = ImGui::GetIO();
	RAWINPUT* rawInput = (RAWINPUT*)data;

	if (rawInput->header.dwType == RIM_TYPEMOUSE)
	{
		static int btDowns[5] = { RI_MOUSE_BUTTON_1_DOWN,
								  RI_MOUSE_BUTTON_2_DOWN,
								  RI_MOUSE_BUTTON_3_DOWN,
								  RI_MOUSE_BUTTON_4_DOWN,
								  RI_MOUSE_BUTTON_5_DOWN };

		static int btUps[5] = { RI_MOUSE_BUTTON_1_UP,
								RI_MOUSE_BUTTON_2_UP,
								RI_MOUSE_BUTTON_3_UP,
								RI_MOUSE_BUTTON_4_UP,
								RI_MOUSE_BUTTON_5_UP };

		for (int i = 0; i < 5; ++i) {
			if (rawInput->data.mouse.usButtonFlags & btDowns[i]) {
				io.MouseDown[i] = true;
			}
			else if (rawInput->data.mouse.usButtonFlags & btUps[i]) {
				io.MouseDown[i] = false;
			}
		}
	}
	else if (rawInput->header.dwType == RIM_TYPEKEYBOARD)
	{
		USHORT key = rawInput->data.keyboard.VKey;
		bool down = !(rawInput->data.keyboard.Flags & RI_KEY_BREAK);

		if (key < 256)
			io.KeysDown[key] = down;
		if (key == VK_CONTROL)
			io.KeyCtrl = down;
		if (key == VK_SHIFT)
			io.KeyShift = down;
		if (key == VK_MENU)
			io.KeyAlt = down;

	}
	return { false, false };
}

void NCL::CSC8503::GameUI::DrawTextCenter(const string& text, float verticalGap) {
	auto windowWidth = ImGui::GetWindowSize().x;
	auto textWidth = ImGui::CalcTextSize(text.c_str()).x;
	ImGui::SetCursorPos(ImVec2((windowWidth - textWidth) * 0.5f, ImGui::GetCursorPosY() + verticalGap));
	ImGui::Text(text.c_str());
}
