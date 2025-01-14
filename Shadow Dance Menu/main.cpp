#include "includes.h"
#include "CoreHack.h"
#include "imgui/droidSans.h"
#include "imgui/vbeFont.h"


extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

Present oPresent;
HWND window = NULL;
WNDPROC oWndProc;
ID3D11Device* pDevice = NULL;
ID3D11DeviceContext* pContext = NULL;
ID3D11RenderTargetView* mainRenderTargetView;
FILE* f;
HMODULE hModule;
ImFont* mainFont;
ImFont* vbeFont;
HANDLE CurProcHandle; //29-Jan-23
int CurProcId; //29-Jan-23

void InitImGui()
{
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
	vbeFont = io.Fonts->AddFontFromMemoryCompressedTTF(vbeFont_compressed_data, vbeFont_compressed_size, 50);
	mainFont = io.Fonts->AddFontFromMemoryCompressedTTF(droidSans_compressed_data, droidSans_compressed_size, 15);
	ImGui_ImplWin32_Init(window);
	ImGui_ImplDX11_Init(pDevice, pContext);
}

LRESULT __stdcall WndProc(const HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {

	if (true && ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
		return true;

	return CallWindowProc(oWndProc, hWnd, uMsg, wParam, lParam);
}

// ImGui Menu Vars
bool bMenuInit = false, bMenuExit = false, bShowMenu = true;
//bool bSvCheats = false; //28-Jan-23
bool bVBE = false, bVBEParticle = false, bDrawRange = false, bParticleHack = false, bNoFog = false;
const char* weatherList[] = { "Default", "Winter", " Rain", "MoonBeam", "Pestilence", "Harvest", "Sirocco", "Spring", "Ash", "Aurora" };
int camDistance = 1200, rangeVal = 1200;
int item_current = 0;

HRESULT __stdcall hkPresent(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags)
{

	if (bMenuExit == true)
	{
		pDevice->Release();
		pContext->Release();
		pSwapChain->Release();
		oWndProc = (WNDPROC)SetWindowLongPtr(window, GWLP_WNDPROC, (LONG_PTR)(oWndProc));
		oPresent(pSwapChain, SyncInterval, Flags);
		return 0;
	}

	if (!bMenuInit)
	{
		if (SUCCEEDED(pSwapChain->GetDevice(__uuidof(ID3D11Device), (void**)& pDevice)))
		{
			pDevice->GetImmediateContext(&pContext);
			DXGI_SWAP_CHAIN_DESC sd;
			pSwapChain->GetDesc(&sd);
			window = sd.OutputWindow;
			ID3D11Texture2D* pBackBuffer;
			pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)& pBackBuffer);
			pDevice->CreateRenderTargetView(pBackBuffer, NULL, &mainRenderTargetView);
			pBackBuffer->Release();
			oWndProc = (WNDPROC)SetWindowLongPtr(window, GWLP_WNDPROC, (LONG_PTR)WndProc);
			InitImGui();
			IM_ASSERT(ImGui::GetCurrentContext() != NULL && "Missing dear imgui context. Refer to examples app!");
			bMenuInit = true;
		}

		else
			return oPresent(pSwapChain, SyncInterval, Flags);
	}

	if (GetAsyncKeyState(VK_INSERT) & 1)
	{
		bShowMenu = !bShowMenu;
	}

	int tempBVBE = -1, tempBDrawRange = -1, tempBParticleHack = -1, tempBNoFog = -1, tempBSvCheats = -1;
	int tempcamDistance = -1;
	int weather_item = -1;
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	if (bShowMenu)
	{
		ImGui::PushFont(mainFont);
		ImGui::Begin("Shadow Dance Panda");
		//ImGui::Checkbox("Enable", &bSvCheats); //28-Jan-23
		ImGui::Text("Visuals");
		ImGui::Checkbox("Overlay Text.", &bVBE);
		ImGui::Checkbox("Draw Blink Dagger Circle Range.", &bDrawRange);
		ImGui::Text("CameraDistance");
		ImGui::SliderInt("##slider", &camDistance, 0, 3000, "%d");
		ImGui::SameLine();
		if (ImGui::Button("Reset", ImVec2(70, 20))) {
			camDistance = 1200;
		}

		ImGui::Dummy(ImVec2(1, 10));
		ImGui::Text("Weather");
		ImGui::ListBox("##list", &item_current, weatherList, IM_ARRAYSIZE(weatherList), 4);

		ImGui::Dummy(ImVec2(1,20));
		ImGui::Text("Hacks");
		ImGui::Checkbox("No Map Fog.", &bNoFog);
		ImGui::Checkbox("Particle Map Hack.", &bParticleHack);
		ImGui::Dummy(ImVec2(1, 20));
		//ImGui::Text("@SK68-ph");
		ImGui::End();
		ImGui::PopFont();
	}

	if (bVBE)
	{
		ImGui::PushFont(vbeFont);
		if (!bShowMenu)
		{
			ImGui::Begin("VBE", NULL, ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoMove);
		}
		else
		{
			ImGui::Begin("VBE", NULL,  ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar);
		}
	
		int VBE = getVBE();

		if (VBE == 0) // Visible by enemy
		{
			ImGui::TextColored(ImVec4(255, 0, 0, 255), "Visible");
		}
		if (VBE == 1) // not Visible by enemy
		{
			ImGui::TextColored(ImVec4(0, 255, 0, 255), "Not Visible");
		}
		else if (VBE == -1) 
		{
			//bVBE = false;
		}
		ImGui::Dummy(ImVec2(1, 15));
		ImGui::End();
		ImGui::PopFont();
	}
	//if (tempBSvCheats != bSvCheats)
	//{
	//	SetSvCheats(!bSvCheats);
	//}
	if (weather_item != item_current)
	{
		SetWeather(item_current);
	}
	if (tempBDrawRange != bDrawRange)
	{
		if (bDrawRange)
			rangeVal = 1200;
		else
			rangeVal = 0;
		SetDrawRange(rangeVal);
	}
	if (tempBParticleHack != bParticleHack)
	{
		SetParticleHack(!bParticleHack);
		
	}
	if (tempBNoFog != bNoFog)
	{
		SetNoFog(!bNoFog);
	}
	if (tempcamDistance != camDistance)
	{
		SetCamDistance(camDistance);

	}
	
	GetHeroValue();
	if (GetAsyncKeyState(VK_NUMPAD1) & 1) //29-Jan-23
	{
		PrintHero1(); 
		
	}

	tempBVBE = bVBE, tempBDrawRange = bDrawRange, tempBParticleHack = bParticleHack, tempBNoFog = bNoFog;
	tempcamDistance = camDistance;
	weather_item = item_current;

	ImGui::Render();
	pContext->OMSetRenderTargets(1, &mainRenderTargetView, NULL);
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
	return oPresent(pSwapChain, SyncInterval, Flags);
}


DWORD WINAPI MainThread(HMODULE hModule)
{
	//AllocConsole();
	//FILE* f;
	//freopen_s(&f, "CONOUT$", "w", stdout);
	AllocConsole(); //29-Jan-23
	FILE* f;
	freopen_s(&f, "CONOUT$", "w", stdout);
	{
		CurProcId = GetCurrentProcessId();
		CurProcHandle = OpenProcess(PROCESS_ALL_ACCESS, TRUE, CurProcId);
	}
	InitHack();
	bool init_hook = false;
	do
	{
		if (kiero::init(kiero::RenderType::D3D11) == kiero::Status::Success)
		{
			kiero::bind(8, (void**)& oPresent, hkPresent);
			init_hook = true;
		}
	} while (!init_hook);
	MessageBeep(MB_OK);
	while (bMenuExit == false)
	{
		if (GetAsyncKeyState(VK_END) & 1)
		{
			bMenuExit = true;
			Sleep(1000);
			ResetConvars();
			RemoveVmtHooks();
			MessageBeep(MB_OK);
			kiero::shutdown();
			if (f) fclose(f); FreeConsole();
			
			FreeLibraryAndExitThread(hModule, 0);
		}
	}
	return 1;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved)
{
	if (dwReason == DLL_PROCESS_ATTACH)
	{
		DisableThreadLibraryCalls(hModule);
		CloseHandle(CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)MainThread, hModule, 0, nullptr));
	}
	return TRUE;
}
