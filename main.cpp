// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "includes.hpp"
#include "utils\ctx.hpp"
#include "utils\recv.h"
#include "utils\imports.h"
#include "nSkinz\SkinChanger.h"
#include "imagess.h"
#include "cheats/menu.h"

PVOID base_address = nullptr;

__forceinline void setup_render();
__forceinline void setup_netvars();
__forceinline void setup_skins();
__forceinline void setup_hooks();

static IDirect3DTexture9* bot_s = nullptr;

enum esp_info_position
{
	RIGHT,
	CENTER_DOWN,
	CENTER_UP
};

struct esp_info_s
{
	esp_info_s(const std::string name, const ImColor color, const int position = RIGHT)
	{
		f_name = name; f_color = color; f_position = position;
	}
	std::string f_name;
	int f_position;
	ImColor f_color;
};

void c_menu::EspPreview(IDirect3DDevice9* device)
{
	auto player = players_section;

	static std::vector<esp_info_s> info;
	static bool enabled = true;
	ImGui::SetNextWindowSize(ImVec2(280, 430));
	ImGui::Begin(" ", &enabled, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
	{
		auto cur_window = ImGui::GetCurrentWindow();
		ImVec2 w_pos = cur_window->Pos;

		if (g_cfg.player.type[player].box)
		{
			cur_window->DrawList->AddRect(ImVec2(w_pos.x + 40, w_pos.y + 60), ImVec2(w_pos.x + 200, w_pos.y + 360), ImColor(g_cfg.player.type[player].box_color[0], g_cfg.player.type[player].box_color[1], g_cfg.player.type[player].box_color[3]));
		}

		if (g_cfg.player.type[player].health)
		{
			cur_window->DrawList->AddRectFilled(ImVec2(w_pos.x + 34, w_pos.y + 60), ImVec2(w_pos.x + 37, w_pos.y + 360), ImColor(46, 201, 38, 255));
		}
		if (g_cfg.player.type[player].name)
			info.emplace_back(esp_info_s("name", ImColor(1.f, 1.f, 1.f), CENTER_UP));
		if (g_cfg.player.type[player].weapons)
			info.emplace_back(esp_info_s("Knife", ImColor(1.f, 1.f, 1.f, 1.f), CENTER_DOWN));

		for (auto render : info)
		{
			auto text_size = ImGui::CalcTextSize(render.f_name.c_str());
			auto pos = ImVec2(w_pos.x + 205, w_pos.y + 60);
			if (render.f_position == CENTER_DOWN)
			{
				pos = ImVec2(w_pos.x + (240 / 2) - text_size.x / 2, pos.y + 315 - text_size.y);
			}
			else if (render.f_position == CENTER_UP)
			{
				pos = ImVec2(w_pos.x + (240 / 2) - text_size.x / 2, pos.y - 5 - text_size.y);
			}
			cur_window->DrawList->AddText(pos, render.f_color, render.f_name.c_str());
		}

	}
	ImGui::Image(bot_s, ImVec2(280, 390));
	ImGui::End();
	info.clear();
}

DWORD WINAPI main(PVOID base)
{
	g_ctx.signatures =
	{
		crypt_str("A1 ? ? ? ? 50 8B 08 FF 51 0C"),
		crypt_str("B9 ?? ?? ?? ?? A1 ?? ?? ?? ?? FF 10 A1 ?? ?? ?? ?? B9"),
		crypt_str("0F 11 05 ?? ?? ?? ?? 83 C8 01"),
		crypt_str("8B 0D ?? ?? ?? ?? 8B 46 08 68"),
		crypt_str("B9 ? ? ? ? F3 0F 11 04 24 FF 50 10"),
		crypt_str("8B 3D ? ? ? ? 85 FF 0F 84 ? ? ? ? 81 C7"),
		crypt_str("A1 ? ? ? ? 8B 0D ? ? ? ? 6A 00 68 ? ? ? ? C6"),
		crypt_str("80 3D ? ? ? ? ? 53 56 57 0F 85"),
		crypt_str("55 8B EC 83 E4 F8 83 EC 18 56 57 8B F9 89 7C 24 0C"),
		crypt_str("80 3D ? ? ? ? ? 74 06 B8"),
		crypt_str("55 8B EC 83 E4 F0 B8 D8"),
		crypt_str("55 8B EC 83 E4 F8 81 EC ? ? ? ? 53 56 8B F1 57 89 74 24 1C"),
		crypt_str("55 8B EC 83 E4 F0 B8 ? ? ? ? E8 ? ? ? ? 56 8B 75 08 57 8B F9 85 F6"),
		crypt_str("55 8B EC 51 56 8B F1 80 BE ? ? ? ? ? 74 36"),
		crypt_str("56 8B F1 8B 8E ? ? ? ? 83 F9 FF 74 23"),
		crypt_str("55 8B EC 83 E4 F8 83 EC 70 56 57 8B F9 89 7C 24 14 83 7F 60"),
		crypt_str("55 8B EC A1 ? ? ? ? 83 EC 10 56 8B F1 B9"),
		crypt_str("57 8B F9 8B 07 8B 80 ? ? ? ? FF D0 84 C0 75 02"),
		crypt_str("55 8B EC 81 EC ? ? ? ? 53 8B D9 89 5D F8 80"),
		crypt_str("53 0F B7 1D ? ? ? ? 56"),
		crypt_str("8B 0D ? ? ? ? 8D 95 ? ? ? ? 6A 00 C6")
	};

	g_ctx.indexes =
	{
		5,
		33,
		339 + 1,
		218 + 1,
		219 + 1,
		34,
		157 + 1,
		75,
		460 + 1,
		482 + 1,
		452 + 1,
		483 + 1,
		284 + 1,
		223 + 1,
		246 + 1,
		27,
		17,
		123
	};

	// note: simv0l - create directories for configs, fonts, scripts etc.
	static TCHAR path[MAX_PATH];
	std::string main_folder;
	std::string configs_folder;
	std::string fonts_folder;
	std::string scripts_folder;
	std::string sounds_folder;
	SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, NULL, path);
	main_folder = std::string(path) + crypt_str("\\ExodiumFrez\\\\");
	configs_folder = std::string(path) + crypt_str("\\ExodiumFrez\\\\Configs\\");
	fonts_folder = std::string(path) + crypt_str("\\ExodiumFrez\\\\Fonts\\");
	scripts_folder = std::string(path) + crypt_str("\\ExodiumFrez\\\\Scripts\\");
	sounds_folder = std::string(path) + crypt_str("\\ExodiumFrez\\\\Sounds\\");
	CreateDirectory(main_folder.c_str(), NULL);
	CreateDirectory(configs_folder.c_str(), NULL);
	CreateDirectory(fonts_folder.c_str(), NULL);
	CreateDirectory(scripts_folder.c_str(), NULL);
	CreateDirectory(sounds_folder.c_str(), NULL);

	while (!IFH(GetModuleHandle)(crypt_str("serverbrowser.dll")))
		std::this_thread::sleep_for(std::chrono::milliseconds(100));

	base_address = base;

	setup_sounds();

	setup_skins();

	setup_netvars();

	if (bot_s == nullptr)
		D3DXCreateTextureFromFileInMemoryEx(m_device(), &bot, sizeof(bot), 280, 390, D3DX_DEFAULT, 0, D3DFMT_UNKNOWN, D3DPOOL_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT, 0, NULL, NULL, &bot_s);

	setup_render();

	cfg_manager->setup();

	c_lua::get().initialize();

	key_binds::get().initialize_key_binds();

	setup_hooks();
	Netvars::Netvars();

	return EXIT_SUCCESS;
}

BOOL WINAPI DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved)
{
	if (dwReason == DLL_PROCESS_ATTACH)
	{
		IFH(DisableThreadLibraryCalls)(hModule);

		auto current_process = IFH(GetCurrentProcess)();
		auto priority_class = IFH(GetPriorityClass)(current_process);

		//if (priority_class != HIGH_PRIORITY_CLASS && priority_class != REALTIME_PRIORITY_CLASS)
			//IFH(SetPriorityClass)(current_process, HIGH_PRIORITY_CLASS);

		CreateThread(nullptr, 0, main, hModule, 0, nullptr); //-V718 //-V513

	}

	return TRUE;
}

__forceinline void setup_render()
{
	static auto create_font = [](const char* name, int size, int weight, DWORD flags) -> vgui::HFont
	{
		g_ctx.last_font_name = name;

		auto font = m_surface()->FontCreate();
		m_surface()->SetFontGlyphSet(font, name, size, weight, 0, 0, flags);

		return font;
	};

	fonts[LOGS] = create_font(crypt_str("Lucida Console"), 9, FW_MEDIUM, FONTFLAG_DROPSHADOW);
	fonts[ESP] = create_font(crypt_str("Smallest Pixel-7"), 11, FW_MEDIUM, FONTFLAG_OUTLINE);
	fonts[NAME] = create_font(crypt_str("Verdana"), 12, FW_MEDIUM, FONTFLAG_ANTIALIAS | FONTFLAG_DROPSHADOW);
	fonts[SUBTABWEAPONS] = create_font(crypt_str("undefeated"), 13, FW_MEDIUM, FONTFLAG_ANTIALIAS | FONTFLAG_DROPSHADOW);
	fonts[KNIFES] = create_font(crypt_str("icomoon"), 13, FW_MEDIUM, FONTFLAG_ANTIALIAS | FONTFLAG_DROPSHADOW);
	fonts[GRENADES] = create_font(crypt_str("undefeated"), 20, FW_MEDIUM, FONTFLAG_ANTIALIAS | FONTFLAG_DROPSHADOW);
	fonts[INDICATORFONT] = create_font(crypt_str("Calibri"), 35, 100, FONTFLAG_ANTIALIAS | FONTFLAG_DROPSHADOW);
	fonts[DAMAGE_MARKER] = create_font(crypt_str("Verdana"), 16, FW_HEAVY, FONTFLAG_OUTLINE);
	fonts[WARNING] = create_font(crypt_str("Impact"), 32, FW_HEAVY, BOLD_FONTTYPE);
	fonts[WEAPONICONS] = create_font(crypt_str("Counter-Strike"), 25, FW_MEDIUM, FONTFLAG_ANTIALIAS | FONTFLAG_DROPSHADOW);
	fonts[light_font] = create_font(crypt_str("Lightning Bolt"), 20, FONTFLAG_ANTIALIAS, FONTFLAG_DROPSHADOW);
	fonts[INDICATORS] = create_font(crypt_str("Calibri"), 15, FW_HEAVY, FONTFLAG_ANTIALIAS);
	fonts[IND2] = create_font(crypt_str("Verdana"), 27, FW_MEDIUM, FONTFLAG_ANTIALIAS | FONTFLAG_DROPSHADOW);
	fonts[ESP1] = create_font(crypt_str("Smallest Pixel-7"), 11, FW_MEDIUM, FONTFLAG_ANTIALIAS);
	fonts[NAME1] = create_font(crypt_str("Verdana"), 25, FW_MEDIUM, FONTFLAG_ANTIALIAS | FONTFLAG_DROPSHADOW);
	fonts[NAME2] = create_font(crypt_str("Verdana"), 10, FW_MEDIUM, FONTFLAG_ANTIALIAS | FONTFLAG_DROPSHADOW);
	fonts[ESP2] = create_font(crypt_str("Smallest Pixel-7"), 9, FW_MEDIUM, FONTFLAG_OUTLINE);
	fonts[VELOCITY] = create_font(crypt_str("Verdana"), 35, FW_HEAVY, FONTFLAG_ANTIALIAS | FONTFLAG_DROPSHADOW);
	fonts[ESP2] = create_font(crypt_str("Smallest Pixel-7"), 11, FW_MEDIUM, FONTFLAG_ANTIALIAS);
	fonts[HELPER] = create_font(crypt_str("Calibri"), 20, 100, FONTFLAG_ANTIALIAS | FONTFLAG_DROPSHADOW);
	fonts[WEAPONICONS] = create_font(crypt_str("Counter-Strike"), 25, FW_MEDIUM, FONTFLAG_ANTIALIAS | FONTFLAG_DROPSHADOW);
	fonts[INDICATORFONT1] = create_font(crypt_str("Calibri"), 28, 700, FONTFLAG_ANTIALIAS | FONTFLAG_DROPSHADOW);
	fonts[ESP3] = create_font(crypt_str("Tahoma"), 9, FW_MEDIUM, FONTFLAG_ANTIALIAS | FONTFLAG_DROPSHADOW);
	fonts[NAME3] = create_font(crypt_str("Verdana"), 12, FW_NORMAL, FONTFLAG_ANTIALIAS | FONTFLAG_DROPSHADOW);

	g_ctx.last_font_name.clear();
}

__forceinline void setup_netvars()
{
	netvars::get().tables.clear();
	auto client = m_client()->GetAllClasses();

	if (!client)
		return;

	while (client)
	{
		auto recvTable = client->m_pRecvTable;

		if (recvTable)
			netvars::get().tables.emplace(std::string(client->m_pNetworkName), recvTable);

		client = client->m_pNext;
	}
}


using mother_fucker_valve = Vector * (__thiscall*)(void*);
inline mother_fucker_valve _yes_fuck;
#include "cheats/lagcompensation/local_animations.h"
Vector* __fastcall eye_angles(void* ecx, void* edx)
{
	static int* WantedReturnAddress1 = (int*)util::FindSignature(("client.dll"), "8B CE F3 0F 10 00 8B 06 F3 0F 11 45 ? FF 90 ? ? ? ? F3 0F 10 55 ?"); //Update Animations X/Y
	static int* WantedReturnAddress2 = (int*)util::FindSignature(("client.dll"), "F3 0F 10 55 ? 51 8B 8E ? ? ? ?");                                    //Update Animations X/Y
	static int* WantedReturnAddress3 = (int*)util::FindSignature(("client.dll"), "8B 55 0C 8B C8 E8 ? ? ? ? 83 C4 08 5E 8B E5");                       //Retarded valve fix

	static auto oGetEyeAngles = _yes_fuck;

	if (_ReturnAddress() != WantedReturnAddress1 && _ReturnAddress() != WantedReturnAddress2 && _ReturnAddress() != WantedReturnAddress3)
		return oGetEyeAngles(ecx);

	if (!ecx || ((player_t*)ecx)->EntIndex() != m_engine()->GetLocalPlayer())
		return oGetEyeAngles(ecx);

	return &local_animations::get().local_data.real_angles;
}



__forceinline void setup_skins()
{
	auto items = std::ifstream(crypt_str("csgo/scripts/items/items_game_cdn.txt"));
	auto gameItems = std::string(std::istreambuf_iterator <char> { items }, std::istreambuf_iterator <char> { });

	if (!items.is_open())
		return;

	items.close();
	memory.initialize();

	for (auto i = 0; i <= memory.itemSchema()->paintKits.lastElement; i++)
	{
		auto paintKit = memory.itemSchema()->paintKits.memory[i].value;

		if (paintKit->id == 9001)
			continue;

		auto itemName = m_localize()->FindSafe(paintKit->itemName.buffer + 1);
		auto itemNameLength = WideCharToMultiByte(CP_UTF8, 0, itemName, -1, nullptr, 0, nullptr, nullptr);

		if (std::string name(itemNameLength, 0); WideCharToMultiByte(CP_UTF8, 0, itemName, -1, &name[0], itemNameLength, nullptr, nullptr))
		{
			if (paintKit->id < 10000)
			{
				if (auto pos = gameItems.find('_' + std::string{ paintKit->name.buffer } +'='); pos != std::string::npos && gameItems.substr(pos + paintKit->name.length).find('_' + std::string{ paintKit->name.buffer } +'=') == std::string::npos)
				{
					if (auto weaponName = gameItems.rfind(crypt_str("weapon_"), pos); weaponName != std::string::npos)
					{
						name.back() = ' ';
						name += '(' + gameItems.substr(weaponName + 7, pos - weaponName - 7) + ')';
					}
				}
				SkinChanger::skinKits.emplace_back(paintKit->id, std::move(name), paintKit->name.buffer);
			}
			else
			{
				std::string_view gloveName{ paintKit->name.buffer };
				name.back() = ' ';
				name += '(' + std::string{ gloveName.substr(0, gloveName.find('_')) } +')';
				SkinChanger::gloveKits.emplace_back(paintKit->id, std::move(name), paintKit->name.buffer);
			}
		}
	}

	std::sort(SkinChanger::skinKits.begin(), SkinChanger::skinKits.end());
	std::sort(SkinChanger::gloveKits.begin(), SkinChanger::gloveKits.end());
}

__forceinline void setup_hooks()
{
	static auto getforeignfallbackfontname = (DWORD)(util::FindSignature(crypt_str("vguimatsurface.dll"), g_ctx.signatures.at(9).c_str()));
	hooks::original_getforeignfallbackfontname = (DWORD)DetourFunction((PBYTE)getforeignfallbackfontname, (PBYTE)hooks::hooked_getforeignfallbackfontname);

	static auto setupbones = (DWORD)(util::FindSignature(crypt_str("client.dll"), g_ctx.signatures.at(10).c_str()));
	hooks::original_setupbones = (DWORD)DetourFunction((PBYTE)setupbones, (PBYTE)hooks::hooked_setupbones);

	static auto doextrabonesprocessing = (DWORD)(util::FindSignature(crypt_str("client.dll"), g_ctx.signatures.at(11).c_str()));
	hooks::original_doextrabonesprocessing = (DWORD)DetourFunction((PBYTE)doextrabonesprocessing, (PBYTE)hooks::hooked_doextrabonesprocessing);

	static auto standardblendingrules = (DWORD)(util::FindSignature(crypt_str("client.dll"), g_ctx.signatures.at(12).c_str()));
	hooks::original_standardblendingrules = (DWORD)DetourFunction((PBYTE)standardblendingrules, (PBYTE)hooks::hooked_standardblendingrules); //-V206

	static auto updateclientsideanimation = (DWORD)(util::FindSignature(crypt_str("client.dll"), g_ctx.signatures.at(13).c_str()));
	hooks::original_updateclientsideanimation = (DWORD)DetourFunction((PBYTE)updateclientsideanimation, (PBYTE)hooks::hooked_updateclientsideanimation);

	//static auto physicssimulate = (DWORD)(util::FindSignature(crypt_str("client.dll"), g_ctx.signatures.at(14).c_str()));
	//hooks::original_physicssimulate = (DWORD)DetourFunction((PBYTE)physicssimulate, (PBYTE)hooks::hooked_physicssimulate);

	static auto modifyeyeposition = (DWORD)(util::FindSignature(crypt_str("client.dll"), g_ctx.signatures.at(15).c_str()));
	hooks::original_modifyeyeposition = (DWORD)DetourFunction((PBYTE)modifyeyeposition, (PBYTE)hooks::hooked_modifyeyeposition);

	static auto calcviewmodelbob = (DWORD)(util::FindSignature(crypt_str("client.dll"), g_ctx.signatures.at(16).c_str()));
	hooks::original_calcviewmodelbob = (DWORD)DetourFunction((PBYTE)calcviewmodelbob, (PBYTE)hooks::hooked_calcviewmodelbob);

	static auto shouldskipanimframe = (DWORD)(util::FindSignature(crypt_str("client.dll"), g_ctx.signatures.at(17).c_str()));
	DetourFunction((PBYTE)shouldskipanimframe, (PBYTE)hooks::hooked_shouldskipanimframe);

	static auto checkfilecrcswithserver = (DWORD)(util::FindSignature(crypt_str("engine.dll"), g_ctx.signatures.at(18).c_str()));
	DetourFunction((PBYTE)checkfilecrcswithserver, (PBYTE)hooks::hooked_checkfilecrcswithserver);

	static auto processinterpolatedlist = (DWORD)(util::FindSignature(crypt_str("client.dll"), g_ctx.signatures.at(19).c_str()));
	hooks::original_processinterpolatedlist = (DWORD)DetourFunction((byte*)processinterpolatedlist, (byte*)hooks::processinterpolatedlist);

	hooks::client_hook = new vmthook(reinterpret_cast<DWORD**>(m_client()));
	hooks::client_hook->hook_function(reinterpret_cast<uintptr_t>(hooks::hooked_fsn), 37);
	hooks::client_hook->hook_function(reinterpret_cast<uintptr_t>(hooks::hooked_writeusercmddeltatobuffer), 24);

	static auto CViewRender_PerformScreenOverlay = util::FindSignature(crypt_str("client.dll"), "55 8b ec 51 a1 ? ? ? ? 53 56 8b d9");
	hooks::original_performscreenoverlay = (DWORD)DetourFunction((PBYTE)CViewRender_PerformScreenOverlay, (PBYTE)hooks::hkPerformScreenOverlay);

	hooks::clientstate_hook = new vmthook(reinterpret_cast<DWORD**>((CClientState*)(uint32_t(m_clientstate()) + 0x8)));
	hooks::clientstate_hook->hook_function(reinterpret_cast<uintptr_t>(hooks::hooked_packetstart), 5);
	hooks::clientstate_hook->hook_function(reinterpret_cast<uintptr_t>(hooks::hooked_packetend), 6);

	hooks::panel_hook = new vmthook(reinterpret_cast<DWORD**>(m_panel()));
	hooks::panel_hook->hook_function(reinterpret_cast<uintptr_t>(hooks::hooked_painttraverse), 41);

	hooks::clientmode_hook = new vmthook(reinterpret_cast<DWORD**>(m_clientmode()));
	hooks::clientmode_hook->hook_function(reinterpret_cast<uintptr_t>(hooks::hooked_createmove), 24);
	hooks::clientmode_hook->hook_function(reinterpret_cast<uintptr_t>(hooks::hooked_postscreeneffects), 44);
	hooks::clientmode_hook->hook_function(reinterpret_cast<uintptr_t>(hooks::hooked_overrideview), 18);
	hooks::clientmode_hook->hook_function(reinterpret_cast<uintptr_t>(hooks::hooked_drawfog), 17);

	hooks::inputinternal_hook = new vmthook(reinterpret_cast<DWORD**>(m_inputinternal()));
	hooks::inputinternal_hook->hook_function(reinterpret_cast<uintptr_t>(hooks::hooked_setkeycodestate), 91);
	hooks::inputinternal_hook->hook_function(reinterpret_cast<uintptr_t>(hooks::hooked_setmousecodestate), 92);

	hooks::engine_hook = new vmthook(reinterpret_cast<DWORD**>(m_engine()));
	hooks::engine_hook->hook_function(reinterpret_cast<uintptr_t>(hooks::hooked_isconnected), 27);
	hooks::engine_hook->hook_function(reinterpret_cast<uintptr_t>(hooks::hooked_getscreenaspectratio), 101);
	hooks::engine_hook->hook_function(reinterpret_cast<uintptr_t>(hooks::hooked_ishltv), 93);

	hooks::renderview_hook = new vmthook(reinterpret_cast<DWORD**>(m_renderview()));
	hooks::renderview_hook->hook_function(reinterpret_cast<uintptr_t>(hooks::hooked_sceneend), 9);

	hooks::materialsys_hook = new vmthook(reinterpret_cast<DWORD**>(m_materialsystem()));
	hooks::materialsys_hook->hook_function(reinterpret_cast<uintptr_t>(hooks::hooked_beginframe), 42);
	hooks::materialsys_hook->hook_function(reinterpret_cast<uintptr_t>(hooks::hooked_getmaterial), 84);

	hooks::modelrender_hook = new vmthook(reinterpret_cast<DWORD**>(m_modelrender()));
	hooks::modelrender_hook->hook_function(reinterpret_cast<uintptr_t>(hooks::hooked_dme), 21);

	hooks::surface_hook = new vmthook(reinterpret_cast<DWORD**>(m_surface()));
	hooks::surface_hook->hook_function(reinterpret_cast<uintptr_t>(hooks::hooked_lockcursor), 67);

	hooks::bspquery_hook = new vmthook(reinterpret_cast<DWORD**>(m_engine()->GetBSPTreeQuery()));
	hooks::bspquery_hook->hook_function(reinterpret_cast<uintptr_t>(hooks::hooked_listleavesinbox), 6);

	hooks::prediction_hook = new vmthook(reinterpret_cast<DWORD**>(m_prediction()));
	hooks::prediction_hook->hook_function(reinterpret_cast<uintptr_t>(hooks::hooked_runcommand), 19);

	hooks::trace_hook = new vmthook(reinterpret_cast<DWORD**>(m_trace()));
	hooks::trace_hook->hook_function(reinterpret_cast<uintptr_t>(hooks::hooked_clip_ray_collideable), 4);
	hooks::trace_hook->hook_function(reinterpret_cast<uintptr_t>(hooks::hooked_trace_ray), 5);



	hooks::filesystem_hook = new vmthook(reinterpret_cast<DWORD**>(util::FindSignature(crypt_str("engine.dll"), g_ctx.signatures.at(20).c_str()) + 0x2));
	hooks::filesystem_hook->hook_function(reinterpret_cast<uintptr_t>(hooks::hooked_loosefileallowed), 128);

	while (!(INIT::Window = IFH(FindWindow)(crypt_str("Valve001"), nullptr)))
		std::this_thread::sleep_for(std::chrono::milliseconds(100));

	INIT::OldWindow = (WNDPROC)IFH(SetWindowLongPtr)(INIT::Window, GWL_WNDPROC, (LONG_PTR)hooks::Hooked_WndProc);

	hooks::directx_hook = new vmthook(reinterpret_cast<DWORD**>(m_device()));
	hooks::directx_hook->hook_function(reinterpret_cast<uintptr_t>(hooks::Hooked_EndScene_Reset), 16);
	hooks::directx_hook->hook_function(reinterpret_cast<uintptr_t>(hooks::hooked_present), 17);
	hooks::directx_hook->hook_function(reinterpret_cast<uintptr_t>(hooks::Hooked_EndScene), 42);

	hooks::hooked_events.RegisterSelf();
}

