// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include <ShlObj_core.h>
#include "dots.h"
#include <unordered_map>
#include "menu.h"
#include "../ImGui/code_editor.h"
#include "../constchars.h"
#include "../cheats/misc/logs.h"
#include "../test/background.h"
#include "../test/jojoh.h"
#include "../ImGui/MenuControls.h"
#include "../test/maw.h"
#include "..\cheats\misc\misc.h"

#include "custom/c_gui.hpp"


#define ALPHA (ImGuiColorEditFlags_AlphaPreview | ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_AlphaBar| ImGuiColorEditFlags_InputRGB | ImGuiColorEditFlags_Float)
#define NOALPHA (ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_InputRGB | ImGuiColorEditFlags_Float)

std::vector <std::string> files;
std::vector <std::string> scripts;
std::string editing_script;

auto selected_script = 0;
auto loaded_editing_script = false;

static auto menu_setupped = false;
static auto should_update = true;

IDirect3DTexture9* all_skins[36];

LPDIRECT3DTEXTURE9 bg = nullptr;
LPDIRECT3DTEXTURE9 jo = nullptr;
LPDIRECT3DTEXTURE9 ma = nullptr;

bool LabelClick(const char* label, bool* v, const char* unique_id)
{
	ImGuiWindow* window = ImGui::GetCurrentWindow();
	if (window->SkipItems)
		return false;

	// The concatoff/on thingies were for my weapon config system so if we're going to make that, we still need this aids.
	char Buf[64];
	_snprintf(Buf, 62, crypt_str("%s"), label);

	char getid[128];
	sprintf_s(getid, 128, crypt_str("%s%s"), label, unique_id);


	ImGuiContext& g = *GImGui;
	const ImGuiStyle& style = g.Style;
	const ImGuiID id = window->GetID(getid);
	const ImVec2 label_size = ImGui::CalcTextSize(label, NULL, true);

	const ImRect check_bb(window->DC.CursorPos, ImVec2(label_size.y + style.FramePadding.y * 2 + window->DC.CursorPos.x, window->DC.CursorPos.y + label_size.y + style.FramePadding.y * 2));
	ImGui::ItemSize(check_bb, style.FramePadding.y);

	ImRect total_bb = check_bb;

	if (label_size.x > 0)
	{
		ImGui::SameLine(0, style.ItemInnerSpacing.x);
		const ImRect text_bb(ImVec2(window->DC.CursorPos.x, window->DC.CursorPos.y + style.FramePadding.y), ImVec2(window->DC.CursorPos.x + label_size.x, window->DC.CursorPos.y + style.FramePadding.y + label_size.y));

		ImGui::ItemSize(ImVec2(text_bb.GetWidth(), check_bb.GetHeight()), style.FramePadding.y);
		total_bb = ImRect(ImMin(check_bb.Min, text_bb.Min), ImMax(check_bb.Max, text_bb.Max));
	}

	if (!ImGui::ItemAdd(total_bb, id))
		return false;

	bool hovered, held;
	bool pressed = ImGui::ButtonBehavior(total_bb, id, &hovered, &held);
	if (pressed)
		*v = !(*v);

	if (*v)
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(126 / 255.f, 131 / 255.f, 219 / 255.f, 1.f));
	if (label_size.x > 0.0f)
		ImGui::RenderText(ImVec2(check_bb.GetTL().x + 12, check_bb.GetTL().y), Buf);
	if (*v)
		ImGui::PopStyleColor();

	return pressed;

}

void draw_keybind(const char* label, key_bind* key_bind, const char* unique_id)
{
	// reset bind if we re pressing esc
	if (key_bind->key == KEY_ESCAPE)
		key_bind->key = KEY_NONE;

	auto clicked = false;
	auto text = (std::string)m_inputsys()->ButtonCodeToString(key_bind->key);

	if (key_bind->key <= KEY_NONE || key_bind->key >= KEY_MAX)
		text = crypt_str("None");

	// if we clicked on keybind
	if (hooks::input_shouldListen && hooks::input_receivedKeyval == &key_bind->key)
	{
		clicked = true;
		text = crypt_str("...");
	}

	auto textsize = ImGui::CalcTextSize(text.c_str()).x + 8 * c_menu::get().dpi_scale;
	auto labelsize = ImGui::CalcTextSize(label);

	ImGui::Text(label);
	ImGui::SameLine();

	ImGui::SetCursorPosX(ImGui::GetWindowSize().x - (ImGui::GetWindowSize().x - ImGui::CalcItemWidth()) - max(-25 * c_menu::get().dpi_scale, textsize));
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 3 * c_menu::get().dpi_scale);

	if (ImGui::KeybindButton(text.c_str(), unique_id, ImVec2(max(50 * c_menu::get().dpi_scale, textsize), 23 * c_menu::get().dpi_scale), clicked))
		clicked = true;


	if (clicked)
	{
		hooks::input_shouldListen = true;
		hooks::input_receivedKeyval = &key_bind->key;
	}

	static auto hold = false, toggle = false, always = false;

	switch (key_bind->mode)
	{
	case HOLD:
		hold = true;
		toggle = false;
		always = false;
		break;
	case TOGGLE:
		toggle = true;
		hold = false;
		always = false;
		break;
	case ALWAYS:
		always = true;
		hold = false;
		toggle = false;
		break;
	}

	if (ImGui::BeginPopup(unique_id))
	{
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 6 * c_menu::get().dpi_scale);
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ((ImGui::GetCurrentWindow()->Size.x / 2) - (ImGui::CalcTextSize(crypt_str("Hold")).x / 2)));
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 11);

		if (LabelClick(crypt_str("Hold"), &hold, unique_id))
		{
			if (hold)
			{
				toggle = false;
				key_bind->mode = HOLD;
			}
			else if (toggle)
			{
				hold = false;
				key_bind->mode = TOGGLE;
			}
			else if (always)
			{
				always = false;
				key_bind->mode = ALWAYS;
			}
			else
			{
				toggle = false;
				key_bind->mode = HOLD;
			}

			ImGui::CloseCurrentPopup();
		}

		ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 9 * c_menu::get().dpi_scale);
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ((ImGui::GetCurrentWindow()->Size.x / 2) - (ImGui::CalcTextSize(crypt_str("Toggle")).x / 2)));
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 11);


		if (LabelClick(crypt_str("Toggle"), &toggle, unique_id))
		{
			if (toggle)
			{
				hold = false;
				key_bind->mode = TOGGLE;
			}
			else if (hold)
			{
				toggle = false;
				key_bind->mode = HOLD;
			}
			else if (always)
			{
				always = false;
				key_bind->mode = ALWAYS;
			}
			else
			{
				hold = false;
				key_bind->mode = TOGGLE;
			}

			ImGui::CloseCurrentPopup();
		}

		ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 12 * c_menu::get().dpi_scale);
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ((ImGui::GetCurrentWindow()->Size.x / 2) - (ImGui::CalcTextSize(crypt_str("Always")).x / 2)));
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 11);

		if (LabelClick(crypt_str("Always"), &always, unique_id))
		{
			if (always)
			{
				always = false;
				key_bind->mode = TOGGLE;
			}
			else if (hold)
			{
				toggle = false;
				key_bind->mode = HOLD;
			}
			else if (toggle)
			{
				hold = false;
				key_bind->mode = TOGGLE;
			}
			else
			{
				always = false;
				key_bind->mode = TOGGLE;
			}

			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}
}

__forceinline void padding(float x, float y)
{
	ImGui::SetCursorPosX(ImGui::GetCursorPosX() + x * c_menu::get().dpi_scale);
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + y * c_menu::get().dpi_scale);
}

void draw_multicombo(std::string name, std::vector<int>& variable, const char* labels[], int count, std::string& preview)
{

	auto hashname = crypt_str("##") + name; // we dont want to render name of combo

	for (auto i = 0, j = 0; i < count; i++)
	{
		if (variable[i])
		{
			if (j)
				preview += crypt_str(", ") + (std::string)labels[i];
			else
				preview = labels[i];

			j++;
		}
	}

	if (ImGui::BeginCombo(hashname.c_str(), preview.c_str())) // draw start
	{
		ImGui::Spacing();
		ImGui::BeginGroup();
		{
			for (auto i = 0; i < count; i++)
				ImGui::Selectable(labels[i], (bool*)&variable[i], ImGuiSelectableFlags_DontClosePopups);
		}
		ImGui::EndGroup();
		ImGui::Spacing();

		ImGui::EndCombo();
	}

	preview = crypt_str("None"); // reset preview to use later
}

std::string get_wep(int id, int custom_index = -1, bool knife = true)
{
	if (custom_index > -1)
	{
		if (knife)
		{
			switch (custom_index)
			{
			case 0: return crypt_str("weapon_knife");
			case 1: return crypt_str("weapon_bayonet");
			case 2: return crypt_str("weapon_knife_css");
			case 3: return crypt_str("weapon_knife_skeleton");
			case 4: return crypt_str("weapon_knife_outdoor");
			case 5: return crypt_str("weapon_knife_cord");
			case 6: return crypt_str("weapon_knife_canis");
			case 7: return crypt_str("weapon_knife_flip");
			case 8: return crypt_str("weapon_knife_gut");
			case 9: return crypt_str("weapon_knife_karambit");
			case 10: return crypt_str("weapon_knife_m9_bayonet");
			case 11: return crypt_str("weapon_knife_tactical");
			case 12: return crypt_str("weapon_knife_falchion");
			case 13: return crypt_str("weapon_knife_survival_bowie");
			case 14: return crypt_str("weapon_knife_butterfly");
			case 15: return crypt_str("weapon_knife_push");
			case 16: return crypt_str("weapon_knife_ursus");
			case 17: return crypt_str("weapon_knife_gypsy_jackknife");
			case 18: return crypt_str("weapon_knife_stiletto");
			case 19: return crypt_str("weapon_knife_widowmaker");
			}
		}
		else
		{
			switch (custom_index)
			{
			case 0: return crypt_str("ct_gloves"); //-V1037
			case 1: return crypt_str("studded_bloodhound_gloves");
			case 2: return crypt_str("t_gloves");
			case 3: return crypt_str("ct_gloves");
			case 4: return crypt_str("sporty_gloves");
			case 5: return crypt_str("slick_gloves");
			case 6: return crypt_str("leather_handwraps");
			case 7: return crypt_str("motorcycle_gloves");
			case 8: return crypt_str("specialist_gloves");
			case 9: return crypt_str("studded_hydra_gloves");
			}
		}
	}
	else
	{
		switch (id)
		{
		case 0: return crypt_str("knife");
		case 1: return crypt_str("gloves");
		case 2: return crypt_str("weapon_ak47");
		case 3: return crypt_str("weapon_aug");
		case 4: return crypt_str("weapon_awp");
		case 5: return crypt_str("weapon_cz75a");
		case 6: return crypt_str("weapon_deagle");
		case 7: return crypt_str("weapon_elite");
		case 8: return crypt_str("weapon_famas");
		case 9: return crypt_str("weapon_fiveseven");
		case 10: return crypt_str("weapon_g3sg1");
		case 11: return crypt_str("weapon_galilar");
		case 12: return crypt_str("weapon_glock");
		case 13: return crypt_str("weapon_m249");
		case 14: return crypt_str("weapon_m4a1_silencer");
		case 15: return crypt_str("weapon_m4a1");
		case 16: return crypt_str("weapon_mac10");
		case 17: return crypt_str("weapon_mag7");
		case 18: return crypt_str("weapon_mp5sd");
		case 19: return crypt_str("weapon_mp7");
		case 20: return crypt_str("weapon_mp9");
		case 21: return crypt_str("weapon_negev");
		case 22: return crypt_str("weapon_nova");
		case 23: return crypt_str("weapon_hkp2000");
		case 24: return crypt_str("weapon_p250");
		case 25: return crypt_str("weapon_p90");
		case 26: return crypt_str("weapon_bizon");
		case 27: return crypt_str("weapon_revolver");
		case 28: return crypt_str("weapon_sawedoff");
		case 29: return crypt_str("weapon_scar20");
		case 30: return crypt_str("weapon_ssg08");
		case 31: return crypt_str("weapon_sg556");
		case 32: return crypt_str("weapon_tec9");
		case 33: return crypt_str("weapon_ump45");
		case 34: return crypt_str("weapon_usp_silencer");
		case 35: return crypt_str("weapon_xm1014");
		default: return crypt_str("unknown");
		}
	}
}

IDirect3DTexture9* get_skin_preview(const char* weapon_name, const std::string& skin_name, IDirect3DDevice9* device)
{
	IDirect3DTexture9* skin_image = nullptr;
	std::string vpk_path;

	if (strcmp(weapon_name, crypt_str("unknown")) && strcmp(weapon_name, crypt_str("knife")) && strcmp(weapon_name, crypt_str("gloves"))) //-V526
	{
		if (skin_name.empty() || skin_name == crypt_str("default"))
			vpk_path = crypt_str("resource/flash/econ/weapons/base_weapons/") + std::string(weapon_name) + crypt_str(".png");
		else
			vpk_path = crypt_str("resource/flash/econ/default_generated/") + std::string(weapon_name) + crypt_str("_") + std::string(skin_name) + crypt_str("_light_large.png");
	}
	else
	{
		if (!strcmp(weapon_name, crypt_str("knife")))
			vpk_path = crypt_str("resource/flash/econ/weapons/base_weapons/weapon_knife.png");
		else if (!strcmp(weapon_name, crypt_str("gloves")))
			vpk_path = crypt_str("resource/flash/econ/weapons/base_weapons/ct_gloves.png");
		else if (!strcmp(weapon_name, crypt_str("unknown")))
			vpk_path = crypt_str("resource/flash/econ/weapons/base_weapons/weapon_snowball.png");

	}
	const auto handle = m_basefilesys()->Open(vpk_path.c_str(), crypt_str("r"), crypt_str("GAME"));
	if (handle)
	{
		int file_len = m_basefilesys()->Size(handle);
		char* image = new char[file_len]; //-V121

		m_basefilesys()->Read(image, file_len, handle);
		m_basefilesys()->Close(handle);

		D3DXCreateTextureFromFileInMemory(device, image, file_len, &skin_image);
		delete[] image;
	}

	if (!skin_image)
	{
		std::string vpk_path;

		if (strstr(weapon_name, crypt_str("bloodhound")) != NULL || strstr(weapon_name, crypt_str("hydra")) != NULL)
			vpk_path = crypt_str("resource/flash/econ/weapons/base_weapons/ct_gloves.png");
		else
			vpk_path = crypt_str("resource/flash/econ/weapons/base_weapons/") + std::string(weapon_name) + crypt_str(".png");

		const auto handle = m_basefilesys()->Open(vpk_path.c_str(), crypt_str("r"), crypt_str("GAME"));

		if (handle)
		{
			int file_len = m_basefilesys()->Size(handle);
			char* image = new char[file_len]; //-V121

			m_basefilesys()->Read(image, file_len, handle);
			m_basefilesys()->Close(handle);

			D3DXCreateTextureFromFileInMemory(device, image, file_len, &skin_image);
			delete[] image;
		}
	}

	return skin_image;
}

// setup some styles and colors, window size and bg alpha
// dpi setup
void c_menu::menu_setup(ImGuiStyle& style) //-V688
{
	ImGui::StyleColorsDark(); // colors setup
	ImGui::SetNextWindowBgAlpha(min(style.Alpha, 0.94f)); // window bg alpha setup

	styles.WindowPadding = style.WindowPadding;
	styles.WindowRounding = style.WindowRounding;
	styles.WindowMinSize = style.WindowMinSize;
	styles.ChildRounding = style.ChildRounding;
	styles.PopupRounding = style.PopupRounding;
	styles.FramePadding = style.FramePadding;
	styles.FrameRounding = style.FrameRounding;
	styles.ItemSpacing = style.ItemSpacing;
	styles.ItemInnerSpacing = style.ItemInnerSpacing;
	styles.TouchExtraPadding = style.TouchExtraPadding;
	styles.IndentSpacing = style.IndentSpacing;
	styles.ColumnsMinSpacing = style.ColumnsMinSpacing;
	styles.ScrollbarSize = style.ScrollbarSize;
	styles.ScrollbarRounding = style.ScrollbarRounding;
	styles.GrabMinSize = style.GrabMinSize;
	styles.GrabRounding = style.GrabRounding;
	styles.TabRounding = style.TabRounding;
	styles.TabMinWidthForUnselectedCloseButton = style.TabMinWidthForUnselectedCloseButton;
	styles.DisplayWindowPadding = style.DisplayWindowPadding;
	styles.DisplaySafeAreaPadding = style.DisplaySafeAreaPadding;
	styles.MouseCursorScale = style.MouseCursorScale;

	// setup skins preview
	for (auto i = 0; i < g_cfg.skins.skinChanger.size(); i++)
		if (!all_skins[i])
			all_skins[i] = get_skin_preview(get_wep(i, (i == 0 || i == 1) ? g_cfg.skins.skinChanger.at(i).definition_override_vector_index : -1, i == 0).c_str(), g_cfg.skins.skinChanger.at(i).skin_name, device); //-V810

	if (bg == nullptr)
		D3DXCreateTextureFromFileInMemoryEx(device, &background, sizeof(background),
			1340, 824,
			D3DX_DEFAULT, 0, D3DFMT_UNKNOWN, D3DPOOL_DEFAULT,
			D3DX_DEFAULT, D3DX_DEFAULT, NULL,
			(D3DXIMAGE_INFO*)NULL, (PALETTEENTRY*)NULL, &bg);
	if (jo == nullptr)
		D3DXCreateTextureFromFileInMemoryEx(device, &somepng, sizeof(somepng), 1440, 1080, D3DX_DEFAULT, 0, D3DFMT_UNKNOWN, D3DPOOL_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT, 0, NULL, NULL, &jo);
	if (ma == nullptr)
		D3DXCreateTextureFromFileInMemoryEx(device, &kit, sizeof(kit), 400, 50, D3DX_DEFAULT, 0, D3DFMT_UNKNOWN, D3DPOOL_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT, 0, NULL, NULL, &ma);

	menu_setupped = true; // we dont want to setup menu again
}

// resize current style sizes
void c_menu::dpi_resize(float scale_factor, ImGuiStyle& style) //-V688
{
	style.WindowPadding = (styles.WindowPadding * scale_factor);
	style.WindowRounding = (styles.WindowRounding * scale_factor);
	style.WindowMinSize = (styles.WindowMinSize * scale_factor);
	style.ChildRounding = (styles.ChildRounding * scale_factor);
	style.PopupRounding = (styles.PopupRounding * scale_factor);
	style.FramePadding = (styles.FramePadding * scale_factor);
	style.FrameRounding = (styles.FrameRounding * scale_factor);
	style.ItemSpacing = (styles.ItemSpacing * scale_factor);
	style.ItemInnerSpacing = (styles.ItemInnerSpacing * scale_factor);
	style.TouchExtraPadding = (styles.TouchExtraPadding * scale_factor);
	style.IndentSpacing = (styles.IndentSpacing * scale_factor);
	style.ColumnsMinSpacing = (styles.ColumnsMinSpacing * scale_factor);
	style.ScrollbarSize = (styles.ScrollbarSize * scale_factor);
	style.ScrollbarRounding = (styles.ScrollbarRounding * scale_factor);
	style.GrabMinSize = (styles.GrabMinSize * scale_factor);
	style.GrabRounding = (styles.GrabRounding * scale_factor);
	style.TabRounding = (styles.TabRounding * scale_factor);
	if (styles.TabMinWidthForUnselectedCloseButton != FLT_MAX) //-V550
		style.TabMinWidthForUnselectedCloseButton = (styles.TabMinWidthForUnselectedCloseButton * scale_factor);
	style.DisplayWindowPadding = (styles.DisplayWindowPadding * scale_factor);
	style.DisplaySafeAreaPadding = (styles.DisplaySafeAreaPadding * scale_factor);
	style.MouseCursorScale = (styles.MouseCursorScale * scale_factor);
}


std::string get_config_dir()
{
	std::string folder;
	static TCHAR path[MAX_PATH];

	if (SUCCEEDED(SHGetFolderPath(NULL, 0x001a, NULL, NULL, path)))
		folder = std::string(path) + crypt_str("\\ExodiumFrez\\Configs\\");

	CreateDirectory(folder.c_str(), NULL);
	return folder;
}

void load_config(std::string selected_config)
{
	if (cfg_manager->files.empty())
		return;

	cfg_manager->load(selected_config, false);
	c_lua::get().unload_all_scripts();

	for (auto& script : g_cfg.scripts.scripts)
		c_lua::get().load_script(c_lua::get().get_script_id(script));

	scripts = c_lua::get().scripts;

	if (selected_script >= scripts.size())
		selected_script = scripts.size() - 1; //-V103

	for (auto& current : scripts)
	{
		if (current.size() >= 5 && current.at(current.size() - 1) == 'c')
			current.erase(current.size() - 5, 5);
		else if (current.size() >= 4)
			current.erase(current.size() - 4, 4);
	}

	for (auto i = 0; i < g_cfg.skins.skinChanger.size(); ++i)
		all_skins[i] = nullptr;

	g_cfg.scripts.scripts.clear();

	cfg_manager->load(selected_config, true);
	cfg_manager->config_files();

	eventlogs::get().add(crypt_str("Loaded ") + selected_config + crypt_str(" config"), false);
}

void save_config(std::string selected_config)
{
	if (cfg_manager->files.empty())
		return;

	g_cfg.scripts.scripts.clear();

	for (auto i = 0; i < c_lua::get().scripts.size(); ++i)
	{
		auto script = c_lua::get().scripts.at(i);

		if (c_lua::get().loaded.at(i))
			g_cfg.scripts.scripts.emplace_back(script);
	}

	cfg_manager->save(selected_config);
	cfg_manager->config_files();

	eventlogs::get().add(crypt_str("Saved ") + selected_config + crypt_str(" config"), false);
}

void remove_config(std::string selected_config)
{
	if (cfg_manager->files.empty())
		return;

	eventlogs::get().add(crypt_str("Removed ") + selected_config + crypt_str(" config"), false);

	cfg_manager->remove(selected_config);
	cfg_manager->config_files();

	files = cfg_manager->files;

	if (g_cfg.selected_config >= files.size())
		g_cfg.selected_config = files.size() - 1; 

	for (auto& current : files)
		if (current.size() > 2)
			current.erase(current.size() - 3, 3);
}

void add_config(std::string name)
{
	if (name.empty())
		name = crypt_str("config");

	eventlogs::get().add(crypt_str("Added ") + name + crypt_str(" config"), false);

	if (name.find(crypt_str(".cfg")) == std::string::npos)
		name += crypt_str(".cfg");

	cfg_manager->save(name);
}


__forceinline void tab_start()
{
	ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX() + (20 * c_menu::get().dpi_scale), ImGui::GetCursorPosY() + (5 * c_menu::get().dpi_scale)));
}

__forceinline void tab_end()
{
	ImGui::PopStyleVar();
	ImGui::SetWindowFontScale(c_menu::get().dpi_scale);
}

void lua_edit(std::string window_name)
{
	std::string file_path;

	auto get_dir = [&]() -> void
	{
		static TCHAR path[MAX_PATH];

		if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, NULL, path)))
			file_path = std::string(path) + crypt_str("\\ExodiumFrez\\Scripts\\");

		CreateDirectory(file_path.c_str(), NULL);
		file_path += window_name + crypt_str(".lua");
	};

	get_dir();
	const char* child_name = (window_name + window_name).c_str();

	ImGui::SetNextWindowSize(ImVec2(700, 600), ImGuiCond_Once);
	ImGui::Begin(window_name.c_str(), nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
	ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarSize, 5.f);

	static TextEditor editor;

	if (!loaded_editing_script)
	{
		static auto lang = TextEditor::LanguageDefinition::Lua();

		editor.SetLanguageDefinition(lang);
		editor.SetReadOnly(false);

		std::ifstream t(file_path);

		if (t.good()) // does while exist?
		{
			std::string str((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());
			editor.SetText(str); // setup script content
		}

		loaded_editing_script = true;
	}

	// dpi scale for font
	// we dont need to resize it for full scale
	ImGui::SetWindowFontScale(1.f + ((c_menu::get().dpi_scale - 1.0) * 0.5f));

	// new size depending on dpi scale
	ImGui::SetWindowSize(ImVec2(ImFloor(800 * (1.f + ((c_menu::get().dpi_scale - 1.0) * 0.5f))), ImFloor(700 * (1.f + ((c_menu::get().dpi_scale - 1.0) * 0.5f)))));
	editor.Render(child_name, ImGui::GetWindowSize() - ImVec2(0, 66 * (1.f + ((c_menu::get().dpi_scale - 1.0) * 0.5f))));

	// seperate code with buttons
	ImGui::Separator();

	// set cursor pos to right edge of window
	ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetWindowSize().x - (16.f * (1.f + ((c_menu::get().dpi_scale - 1.0) * 0.5f))) - (250.f * (1.f + ((c_menu::get().dpi_scale - 1.0) * 0.5f))));
	ImGui::BeginGroup();

	ImGui::EndGroup();

	ImGui::PopStyleVar();
	ImGui::End();
}

#include "../ImGui/imgui.h"
#include "../ImGui/imgui_internal.h"
#include <map>

void c_menu::draw(bool is_open)
{
	static auto w = 0, h = 0, current_h = 0;
	m_engine()->GetScreenSize(w, current_h);

	if (h != current_h)
	{
		if (h)
			update_scripts = true;

		h = current_h;
		update_dpi = true;
	}

	// animation related code
	static float m_alpha = 0.0002f;
	m_alpha = math::clamp(m_alpha + (9.f * ImGui::GetIO().DeltaTime * (is_open ? 1.f : -1.f)), 0.0001f, 1.f);

	// set alpha in class to use later in widgets
	public_alpha = m_alpha;

	if (m_alpha <= 0.0001f)
		return;

	// set new alpha
	ImGui::PushStyleVar(ImGuiStyleVar_Alpha, m_alpha);

	// setup colors and some styles
	if (!menu_setupped)
		menu_setup(ImGui::GetStyle());

	ImGui::PushStyleColor(ImGuiCol_ScrollbarGrab, ImVec4(ImGui::GetStyle().Colors[ImGuiCol_ScrollbarGrab].x, ImGui::GetStyle().Colors[ImGuiCol_ScrollbarGrab].y, ImGui::GetStyle().Colors[ImGuiCol_ScrollbarGrab].z, m_alpha));

	// default menu size
	const int x = 670, y = 412;

	// last active tab to switch effect & reverse alpha & preview alpha
	// IMPORTANT: DO TAB SWITCHING BY LAST_TAB!!!!!
	static int last_tab = active_tab;
	static bool preview_reverse = false;
	static int cfg_subtab = 0;
	static int cfg_id = 0;

	if (update_dpi)
	{
		int w, h;
		m_engine()->GetScreenSize(w, h);

		dpi_scale = math::clamp(h / 1080.0f, 1.f, 4.0f);

		// font and window size setting
		ImGui::SetWindowFontScale(dpi_scale);
		ImGui::SetWindowSize(ImVec2(ImFloor(x), ImFloor(y)));

		// styles resizing
		dpi_resize(dpi_scale, ImGui::GetStyle());

		// setup new window sizes
		width = ImFloor(x * dpi_scale);
		height = ImFloor(y * dpi_scale);

		// end of dpi updating
		ImGui::GetStyle().PopupRounding = 0;
		ImGui::GetStyle().Colors[ImGuiCol_PopupBg] = ImColor(29, 29, 29, 220);
		update_dpi = false;
	}	

	c_gui gui;
	// start menu render
	ImGui::Begin(crypt_str("###MM"), nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground);
	{
		auto p = ImGui::GetWindowPos();
		auto draw = ImGui::GetWindowDrawList();
		ImGui::SetWindowSize(ImVec2(540, 530));
		static int tabs = 0;

		/*window drawing*/
	    // background
		draw->AddRectFilled(p, p + ImVec2(540, 530), ImColor(29, 29, 29, 220));
		// subtabs
		draw->AddRectFilled(p, p + ImVec2(230, 530), ImColor(28, 28, 30, 245));
		// tabs
		draw->AddRectFilled(p, p + ImVec2(60, 530), ImColor(44, 43, 48, 245));
		// Logo
		draw->AddText(Logo, 30.f, p + ImVec2(15, 15), ImColor(255, 255, 255, 255), "T");
		/*==============*/


		/*window elements*/
		{
			ImGui::SetCursorPos(ImVec2(6, 80));
			ImGui::BeginGroup();
			if (gui.tab("0", tabs == 0, ImVec2(50, 50)))
				tabs = 0;
			if (gui.tab("1", tabs == 1, ImVec2(50, 50)))
				tabs = 1;
			if (gui.tab("2", tabs == 2, ImVec2(50, 50)))
				tabs = 2;
			if (gui.tab("3", tabs == 3, ImVec2(50, 50)))
				tabs = 3;
			ImGui::EndGroup();

			switch (tabs) {
			case 0: {
				static auto subtab = -1;

				ImGui::SetCursorPos(ImVec2(64, 20));
				ImGui::BeginGroup();
				{
					if (ImGui::CollapsingHeader("Main", true)) {
						ImGui::NewLine();

						if (gui.subtab("ragebot", subtab == 0, ImVec2(160, 35)))
							subtab = 0;
						if (gui.subtab("exploits", subtab == 1, ImVec2(160, 35)))
							subtab = 1;
					}

					ImGui::NewLine();

					if (ImGui::CollapsingHeader("Weapons")) {
						ImGui::NewLine();

						if (gui.subtab("main", subtab == 2, ImVec2(160, 35)))
							subtab = 2;
						if (gui.subtab("settings", subtab == 3, ImVec2(160, 35)))
							subtab = 3;
					}

					ImGui::NewLine();

					if (ImGui::CollapsingHeader("Antiaims")) {
						ImGui::NewLine();

						if (gui.subtab("General", subtab == 4, ImVec2(160, 35)))
							subtab = 4;
						if (gui.subtab("FakeLags", subtab == 5, ImVec2(160, 35)))
							subtab = 5;
						if (gui.subtab("Other", subtab == 6, ImVec2(160, 35)))
							subtab = 6;
					}
				}
				ImGui::EndGroup();

				ImGui::SetCursorPos(ImVec2(240, 55));	
				ImGui::BeginGroup();
				{ 
					switch (subtab) {
					case 0: {
						ImGui::PushItemWidth(240);
						static auto checkbox = false;
						static auto item = 0, slider = 0, slider2 = 0;
						draw->AddText(Title, 20.f, p + ImVec2(240, 25), ImColor(255, 255, 255, 255), "Ragebot");

						ImGui::Checkbox(crypt_str("Enable"), &g_cfg.ragebot.enable);

						if (g_cfg.ragebot.enable)
							g_cfg.legitbot.enabled = false;

						ImGui::SliderInt("aimbot radius", &g_cfg.ragebot.field_of_view, 0, 180);
						ImGui::Checkbox("Automatic fire", &g_cfg.ragebot.autoshoot);
						ImGui::Checkbox("Automatic wall penetration", &g_cfg.ragebot.autowall);
						ImGui::Checkbox("Aimbot hitboxes", &g_cfg.player.lag_hitbox);
						ImGui::SameLine();
						ImGui::ColorEdit("##lagcompcolor", &g_cfg.player.lag_hitbox_color, ALPHA, ImGuiColorEditFlags_NoInputs);
						ImGui::Checkbox("Silent aimbot", &g_cfg.ragebot.silent_aim);
						ImGui::Checkbox("Auto Zeus", &g_cfg.ragebot.zeus_bot);
						ImGui::Checkbox("Auto Knife", &g_cfg.ragebot.knife_bot);

						ImGui::PopItemWidth();
						break;
					}
					case 1: {
						ImGui::PushItemWidth(240);
						draw->AddText(Title, 20.f, p + ImVec2(240, 25), ImColor(255, 255, 255, 255), "Exploits");

						ImGui::Checkbox("Doubletap", &g_cfg.ragebot.double_tap);

						if (g_cfg.ragebot.double_tap)
						{
							ImGui::SameLine();
							draw_keybind(crypt_str(""), &g_cfg.ragebot.double_tap_key, crypt_str("##HOTKEY_DT"));
							ImGui::Spacing();
						}

						ImGui::Checkbox("Hideshots", &g_cfg.antiaim.hide_shots);

						if (g_cfg.antiaim.hide_shots)
						{
							ImGui::SameLine();
							draw_keybind(crypt_str(""), &g_cfg.antiaim.hide_shots_key, crypt_str("##HOTKEY_HS"));
							ImGui::Spacing();
						}

						ImGui::PopItemWidth();
						break;
					}
					case 2: {
						ImGui::PushItemWidth(240);
						draw->AddText(Title, 20.f, p + ImVec2(240, 25), ImColor(255, 255, 255, 255), "Main");
						const char* rage_weapons[8] = { crypt_str("Revolver / Deagle"), crypt_str("Pistols"), crypt_str("SMGs"), crypt_str("Rifles"), crypt_str("Auto"), crypt_str("Scout"), crypt_str("AWP"), crypt_str("Heavy") };

						ImGui::Text("Current Weapon");
						ImGui::Combo("##Current weapon", &hooks::rage_weapon, "Revolver / Deagle\0Pistols\0SMGs\0Rifles\0Auto\0Scout\0AWP\0Heavy");
						ImGui::Spacing();
						ImGui::Text("Target selection");
						ImGui::Combo("##Target selection", &g_cfg.ragebot.weapon[hooks::rage_weapon].selection_type, "Cycle\0Near crosshair\0Lowest distance\0Lowest health\0Highest damage");
						ImGui::Spacing();
						ImGui::Text("Hitboxes");
						draw_multicombo(crypt_str("Hitboxes"), g_cfg.ragebot.weapon[hooks::rage_weapon].hitboxes, hitboxes, ARRAYSIZE(hitboxes), preview); // TODO
						ImGui::Checkbox("Automatic scope", &g_cfg.ragebot.weapon[hooks::rage_weapon].autoscope);

						ImGui::Checkbox("Automatic stop", &g_cfg.ragebot.weapon[hooks::rage_weapon].autostop);
						if (g_cfg.ragebot.weapon[hooks::rage_weapon].autostop) // TODO
						{
							ImGui::Spacing();
							ImGui::Text("Autostop Modifiers");
							draw_multicombo(crypt_str("Autostop Modifiers"), g_cfg.ragebot.weapon[hooks::rage_weapon].autostop_modifiers, autostop_modifiers, ARRAYSIZE(autostop_modifiers), preview);
						}

						ImGui::Checkbox("Prefer safe points", &g_cfg.ragebot.weapon[hooks::rage_weapon].prefer_safe_points);
						ImGui::Checkbox("Prefer body aim", &g_cfg.ragebot.weapon[hooks::rage_weapon].prefer_body_aim);

						draw_keybind(crypt_str("Force safe points"), &g_cfg.ragebot.safe_point_key, crypt_str("##HOKEY_FORCE_SAFE_POINTS"));//todo
						draw_keybind(crypt_str("Force body aim"), &g_cfg.ragebot.body_aim_key, crypt_str("##HOKEY_FORCE_BODY_AIM"));//todo


						ImGui::PopItemWidth();
						break;
					}
					case 3: {
						ImGui::PushItemWidth(240);
						draw->AddText(Title, 20.f, p + ImVec2(240, 25), ImColor(255, 255, 255, 255), "Settings");

						ImGui::Checkbox("Hitchance", &g_cfg.ragebot.weapon[hooks::rage_weapon].hitchance);
						ImGui::Spacing();
						if (g_cfg.ragebot.weapon[hooks::rage_weapon].hitchance)
							ImGui::SliderInt("Hitchance amount", &g_cfg.ragebot.weapon[hooks::rage_weapon].hitchance_amount, 0, 99);

						ImGui::Spacing();

						ImGui::SliderInt("Minimum visible damage", &g_cfg.ragebot.weapon[hooks::rage_weapon].minimum_visible_damage, 0, 120);

						ImGui::Spacing();

						if (g_cfg.ragebot.autowall)
							ImGui::SliderInt("Minimum wall damage", &g_cfg.ragebot.weapon[hooks::rage_weapon].minimum_damage, 0, 120);

						ImGui::Spacing();

						ImGui::Checkbox("Static point scale", &g_cfg.ragebot.weapon[hooks::rage_weapon].static_point_scale);

						ImGui::Spacing();

						if (g_cfg.ragebot.weapon[hooks::rage_weapon].static_point_scale)
						{
							ImGui::SliderFloat("Head scale", &g_cfg.ragebot.weapon[hooks::rage_weapon].head_scale, 0.2f, 1.0f, g_cfg.ragebot.weapon[hooks::rage_weapon].head_scale ? crypt_str("%.2f") : crypt_str("None"));

							ImGui::Spacing();

							ImGui::SliderFloat("Body scale", &g_cfg.ragebot.weapon[hooks::rage_weapon].body_scale, 0.2f, 1.0f, g_cfg.ragebot.weapon[hooks::rage_weapon].body_scale ? crypt_str("%.2f") : crypt_str("None"));
						}

						ImGui::Spacing();

						ImGui::Checkbox("Enable Max Misses", &g_cfg.ragebot.weapon[hooks::rage_weapon].max_misses);

						ImGui::Spacing();

						if (g_cfg.ragebot.weapon[hooks::rage_weapon].max_misses)
							ImGui::SliderInt("Max Misses", &g_cfg.ragebot.weapon[hooks::rage_weapon].max_misses_amount, 0, 6);

						draw_keybind(crypt_str("Damage override"), &g_cfg.ragebot.weapon[hooks::rage_weapon].damage_override_key, crypt_str("##HOTKEY__DAMAGE_OVERRIDE"));//todo

						if (g_cfg.ragebot.weapon[hooks::rage_weapon].damage_override_key.key > KEY_NONE && g_cfg.ragebot.weapon[hooks::rage_weapon].damage_override_key.key < KEY_MAX)//todo
							ImGui::SliderInt("Minimum override damage", &g_cfg.ragebot.weapon[hooks::rage_weapon].minimum_override_damage, 1, 120, true); //todo


						ImGui::PopItemWidth();
						break;
					}

					case 4: {
						ImGui::PushItemWidth(240);
						draw->AddText(Title, 20.f, p + ImVec2(240, 25), ImColor(255, 255, 255, 255), "Main AntiAims");

						ImGui::Checkbox("Enable", &g_cfg.antiaim.enable);
						ImGui::Spacing();
						ImGui::Text("Pitch");
						ImGui::Combo("##Pitch", &g_cfg.antiaim.pitch, "None\0Down\0Lowest distance\0Up");
						ImGui::Spacing();
						ImGui::Text("Angle");
						ImGui::Combo("##Angle", &g_cfg.antiaim.base_angle, "Local view\0Freestand\0At targets");
						ImGui::Spacing();
						ImGui::Text("Desync");
						ImGui::Combo("##Desync", &g_cfg.antiaim.desync, "None\0Static\0Jitter");
						ImGui::Spacing();
						ImGui::Text("Yaw");
						ImGui::Combo("##Yaw", &g_cfg.antiaim.yaw, "Static\0Jitter\0Spin");
						ImGui::Spacing();
						ImGui::Text("Direction");
						ImGui::Combo("##direction", &g_cfg.antiaim.desync_dir, "Manual\0Freestand\0Spin");
						ImGui::Spacing();
						ImGui::Text("Lower Body Yaw");
						ImGui::Combo("##Lower Body Yaw", &g_cfg.antiaim.lby_type, "Normal\0Opposite\0Sway");


						if (g_cfg.antiaim.yaw)
						{
							ImGui::SliderInt(g_cfg.antiaim.yaw == 1 ? ("Jitter range") : ("Spin range"), &g_cfg.antiaim.range, 0, 180);

							ImGui::Spacing();
							if (g_cfg.antiaim.yaw == 2)
								ImGui::SliderInt("Spin speed", &g_cfg.antiaim.speed, 0, 15);

							ImGui::Spacing();
						}

						if (g_cfg.antiaim.desync_dir != 1)
							draw_keybind(crypt_str("Invert desync"), &g_cfg.antiaim.flip_desync, crypt_str("##HOTKEY_INVERT_DESYNC"));//todo

						draw_keybind(crypt_str("Manual back"), &g_cfg.antiaim.manual_back, crypt_str("##HOTKEY_INVERT_BACK"));//todo

						draw_keybind(crypt_str("Manual left"), &g_cfg.antiaim.manual_left, crypt_str("##HOTKEY_INVERT_LEFT"));//todo

						draw_keybind(crypt_str("Manual right"), &g_cfg.antiaim.manual_right, crypt_str("##HOTKEY_INVERT_RIGHT")); //todo

						if (g_cfg.antiaim.manual_back.key > KEY_NONE && g_cfg.antiaim.manual_back.key < KEY_MAX || g_cfg.antiaim.manual_left.key > KEY_NONE && g_cfg.antiaim.manual_left.key < KEY_MAX || g_cfg.antiaim.manual_right.key > KEY_NONE && g_cfg.antiaim.manual_right.key < KEY_MAX)
						{
							ImGui::Checkbox("Manuals indicator", &g_cfg.antiaim.flip_indicator);
							ImGui::SameLine();
							ImGui::ColorEdit("##invc", &g_cfg.antiaim.flip_indicator_color, ALPHA); //todo
						}

						ImGui::PopItemWidth();
						break;
					}

					case 5: {
						ImGui::PushItemWidth(240);
						draw->AddText(Title, 20.f, p + ImVec2(240, 25), ImColor(255, 255, 255, 255), "FakeLags");

						ImGui::Spacing();
						ImGui::Text("Fake lag type");
						ImGui::Combo("##Fake lag type", &g_cfg.antiaim.fakelag_type, "Static\0Random\0Dynamic\0Fluctuate");
						ImGui::Spacing();
						ImGui::SliderInt("Limit", &g_cfg.antiaim.fakelag_limit, 0, 15);
						ImGui::Spacing();
						ImGui::Text("Fake lag triggers");
						draw_multicombo(crypt_str("Fake lag triggers"), g_cfg.antiaim.fakelag_enablers, lagstrigger, ARRAYSIZE(lagstrigger), preview);

						auto enabled_fakelag_triggers = false;

						for (auto i = 0; i < ARRAYSIZE(lagstrigger); i++)
							if (g_cfg.antiaim.fakelag_enablers[i])
								enabled_fakelag_triggers = true;

						if (enabled_fakelag_triggers)
							ImGui::Spacing();
						ImGui::SliderInt("Triggers limit", &g_cfg.antiaim.triggers_fakelag_limit, 0, 15);
						ImGui::Spacing();

						ImGui::PopItemWidth();
						break;
					}
					case 6: {
						ImGui::PushItemWidth(240);
						draw->AddText(Title, 20.f, p + ImVec2(240, 25), ImColor(255, 255, 255, 255), "Other");

						ImGui::Checkbox("Pitch zero on land", &g_cfg.antiaim.pitch0land);
						ImGui::Spacing();
						ImGui::Checkbox("Static Legs In Air", &g_cfg.antiaim.static_legs_in_air);
						ImGui::Spacing();
						ImGui::SliderInt("Desync range", &g_cfg.antiaim.desync_range, 1, 60);
						ImGui::Spacing();
						ImGui::SliderInt("Body lean", &g_cfg.antiaim.body_lean, 0, 100);
						ImGui::Spacing();
						ImGui::SliderInt("Inverted body lean", &g_cfg.antiaim.inverted_body_lean, 0, 100);

						ImGui::PopItemWidth();
						break;
					}
					}
				}
				ImGui::EndGroup();

				break;
			}
			case 1: {
				static auto subtab = -1;

				ImGui::SetCursorPos(ImVec2(64, 20));
				ImGui::BeginGroup();
				{

					if (ImGui::CollapsingHeader("Players")) {
						ImGui::NewLine();

						if (gui.subtab("Player", subtab == 0, ImVec2(160, 35)))
							subtab = 0;
						if (gui.subtab("Chams", subtab == 1, ImVec2(160, 35)))
							subtab = 1;
						if (gui.subtab("Grenades", subtab == 2, ImVec2(160, 35)))
							subtab = 2;
						if (gui.subtab("Weapons", subtab == 3, ImVec2(160, 35)))
							subtab = 3;
					}
					ImGui::NewLine();
					if (ImGui::CollapsingHeader("Visuals")) {
						ImGui::NewLine();

						if (gui.subtab("Another", subtab == 4, ImVec2(160, 35)))
							subtab = 4;
						if (gui.subtab("World", subtab == 5, ImVec2(160, 35)))
							subtab = 5;
						if (gui.subtab("Indicators", subtab == 6, ImVec2(160, 35)))
							subtab = 6;
						if (gui.subtab("Viewmodel", subtab == 7, ImVec2(160, 35)))
							subtab = 7;
					}
				}
				ImGui::EndGroup();

				ImGui::SetCursorPos(ImVec2(240, 55));
				ImGui::BeginGroup();
				{
					switch (subtab) {

					case 0: {
						ImGui::PushItemWidth(240);
						static auto checkbox = false;
						auto player = players_section;
						static auto item = 0, slider = 0, slider2 = 0;
						draw->AddText(Title, 20.f, p + ImVec2(240, 25), ImColor(255, 255, 255, 255), "Player");

						ImGui::Checkbox("Enable Visuals", &g_cfg.player.enable);

						const char* players_sections[] = { crypt_str("Enemy"), crypt_str("Team"), crypt_str("Local") };

						ImGui::Spacing();
						ImGui::Text("Player Type");
						ImGui::Combo("##Player Type", &players_section, "Enemy\0Team\0Local");

						if (player == ENEMY)
						{
							ImGui::Checkbox("Offscreen arrows", &g_cfg.player.arrows);
							ImGui::SameLine();
							ImGui::ColorEdit("##arrowscolor", &g_cfg.player.arrows_color, ALPHA);

							if (g_cfg.player.arrows)
							{
								ImGui::SliderInt("Arrows distance", &g_cfg.player.distance, 1, 100);
								ImGui::SliderInt("Arrows size", &g_cfg.player.size, 1, 100);
							}
						}

						ImGui::Checkbox("Bounding box", &g_cfg.player.type[player].box);
						ImGui::SameLine();
						ImGui::ColorEdit("##boxcolor", &g_cfg.player.type[player].box_color, ALPHA);

						ImGui::Checkbox("Name", &g_cfg.player.type[player].name);
						ImGui::SameLine();
						ImGui::ColorEdit("##namecolor", &g_cfg.player.type[player].name_color, ALPHA);

						ImGui::Checkbox("Health bar", &g_cfg.player.type[player].health);
						ImGui::Checkbox("Health color", &g_cfg.player.type[player].custom_health_color);
						ImGui::SameLine();
						ImGui::ColorEdit("##healthcolor", &g_cfg.player.type[player].health_color, ALPHA);

						for (auto i = 0, j = 0; i < ARRAYSIZE(flags); i++)
						{
							if (g_cfg.player.type[player].flags[i])
							{
								if (j)
									preview += crypt_str(", ") + (std::string)flags[i];
								else
									preview = flags[i];

								j++;
							}
						}

						ImGui::Spacing();
						ImGui::Text("Flags");
						draw_multicombo(crypt_str("Flags"), g_cfg.player.type[player].flags, flags, ARRAYSIZE(flags), preview);
						ImGui::Checkbox("weapons", &g_cfg.player.type[player].weapons);
						ImGui::Spacing();
						ImGui::Text("Weapon");
						if (g_cfg.player.type[player].weapons)
							draw_multicombo(crypt_str("Weapon"), g_cfg.player.type[player].weapon, weaponplayer, ARRAYSIZE(weaponplayer), preview);


						if (g_cfg.player.type[player].weapon[WEAPON_ICON] || g_cfg.player.type[player].weapon[WEAPON_TEXT])
						{
							ImGui::Text(crypt_str("Color "));
							ImGui::SameLine();
							ImGui::ColorEdit("##weapcolor", &g_cfg.player.type[player].weapon_color, ALPHA);
						}

						ImGui::Checkbox("Skeleton", &g_cfg.player.type[player].skeleton);
						ImGui::SameLine();
						ImGui::ColorEdit("##skeletoncolor", &g_cfg.player.type[player].skeleton_color, ALPHA);

						ImGui::Checkbox("Ammo bar", &g_cfg.player.type[player].ammo);
						ImGui::SameLine();
						ImGui::ColorEdit("##ammocolor", &g_cfg.player.type[player].ammobar_color, ALPHA);

						ImGui::Checkbox("Footsteps", &g_cfg.player.type[player].footsteps);
						ImGui::SameLine();
						ImGui::ColorEdit("##footstepscolor", &g_cfg.player.type[player].footsteps_color, ALPHA);

						if (g_cfg.player.type[player].footsteps)
						{
							ImGui::Spacing();
							ImGui::SliderInt("Thickness", &g_cfg.player.type[player].thickness, 1, 10);
							ImGui::Spacing();
							ImGui::SliderInt("Radius", &g_cfg.player.type[player].radius, 50, 500);
							ImGui::Spacing();
						}

						//ImGui::Spacing();
						//ImGui::Text("Weapon");
						//draw_combo(crypt_str("Player model T"), g_cfg.player.player_model_t, player_model_t, ARRAYSIZE(player_model_t));
						//padding(0, 3);
						//draw_combo(crypt_str("Player model CT"), g_cfg.player.player_model_ct, player_model_ct, ARRAYSIZE(player_model_ct));


						ImGui::PopItemWidth();
						break;
					}

					case 1: {
						ImGui::PushItemWidth(240);
						static auto checkbox = false;
						static auto item = 0, slider = 0, slider2 = 0;
						draw->AddText(Title, 20.f, p + ImVec2(240, 25), ImColor(255, 255, 255, 255), "Chams");

						auto player = players_section;

						if (player == LOCAL)
						ImGui::Spacing();
						ImGui::Text("Type");
						ImGui::Combo("##Type", &g_cfg.player.local_chams_type, "Real\0Desync");

						if (player != LOCAL || !g_cfg.player.local_chams_type)
					    ImGui::Spacing();
						ImGui::Text("Chams");
							draw_multicombo(crypt_str("Chams"), g_cfg.player.type[player].chams, g_cfg.player.type[player].chams[PLAYER_CHAMS_VISIBLE] ? chamsvisact : chamsvis, g_cfg.player.type[player].chams[PLAYER_CHAMS_VISIBLE] ? ARRAYSIZE(chamsvisact) : ARRAYSIZE(chamsvis), preview);

						if (g_cfg.player.type[player].chams[PLAYER_CHAMS_VISIBLE] || player == LOCAL && g_cfg.player.local_chams_type) //-V648
						{
							if (player == LOCAL && g_cfg.player.local_chams_type)
							{
								ImGui::Checkbox(crypt_str("Enable desync chams"), &g_cfg.player.fake_chams_enable);
								ImGui::Checkbox(crypt_str("Visualize lag"), &g_cfg.player.visualize_lag);
								ImGui::Checkbox(crypt_str("Layered"), &g_cfg.player.layered);

								ImGui::Spacing();
								ImGui::Text("Fake chams material");
								ImGui::Combo("##Fake chams material", &g_cfg.player.fake_chams_type, "Regular\0Mettalic\0Flat\0Pulse\0Crystal\0Glass\0Circuit\0Golden\0Glow\0");

								ImGui::Text(crypt_str("Color "));
								ImGui::SameLine();
								ImGui::ColorEdit(crypt_str("##fakechamscolor"), &g_cfg.player.fake_chams_color, ALPHA);

								if (g_cfg.player.fake_chams_type != 6)
								{
									ImGui::Checkbox(crypt_str("Double material "), &g_cfg.player.fake_double_material);
									ImGui::SameLine();
									ImGui::ColorEdit(crypt_str("##doublematerialcolor"), &g_cfg.player.fake_double_material_color, ALPHA);
								}

								ImGui::Checkbox(crypt_str("Animated material"), &g_cfg.player.fake_animated_material);
								ImGui::SameLine();
								ImGui::ColorEdit(crypt_str("##animcolormat"), &g_cfg.player.fake_animated_material_color, ALPHA);
							}
							else
							{
								ImGui::Spacing();
								ImGui::Text("Player chams material");
								ImGui::Combo("##Player chams material", &g_cfg.player.type[player].chams_type, "Regular\0Mettalic\0Flat\0Pulse\0Crystal\0Glass\0Circuit\0Golden\0Glow\0");

								if (g_cfg.player.type[player].chams[PLAYER_CHAMS_VISIBLE])
								{
									ImGui::Text(crypt_str("Visible "));
									ImGui::SameLine();
									ImGui::ColorEdit(crypt_str("##chamsvisible"), &g_cfg.player.type[player].chams_color, ALPHA);
								}

								if (g_cfg.player.type[player].chams[PLAYER_CHAMS_VISIBLE] && g_cfg.player.type[player].chams[PLAYER_CHAMS_INVISIBLE])
								{
									ImGui::Text(crypt_str("Invisible "));
									ImGui::SameLine();
									ImGui::ColorEdit(crypt_str("##chamsinvisible"), &g_cfg.player.type[player].xqz_color, ALPHA);
								}

								if (g_cfg.player.type[player].chams_type != 6)
								{
									ImGui::Checkbox(crypt_str("Double material "), &g_cfg.player.type[player].double_material);
									ImGui::SameLine();
									ImGui::ColorEdit(crypt_str("##doublematerialcolor"), &g_cfg.player.type[player].double_material_color, ALPHA);
								}

								ImGui::Checkbox(crypt_str("Animated material"), &g_cfg.player.type[player].animated_material);
								ImGui::SameLine();
								ImGui::ColorEdit(crypt_str("##animcolormat"), &g_cfg.player.type[player].animated_material_color, ALPHA);

								if (player == ENEMY)
								{
									ImGui::Checkbox(crypt_str("Backtrack chams"), &g_cfg.player.backtrack_chams);

									if (g_cfg.player.backtrack_chams)
									{
										ImGui::Spacing();
										ImGui::Text("Backtrack chams material");
										ImGui::Combo("##Backtrack chams material", &g_cfg.player.type[player].glow_type, "Regular\0Mettalic\0Flat\0Pulse\0Crystal\0Glass\0Circuit\0Golden\0Glow\0");

										ImGui::Text(crypt_str("Color "));
										ImGui::SameLine();
										ImGui::ColorEdit(crypt_str("##backtrackcolor"), &g_cfg.player.backtrack_chams_color, ALPHA);

									}
								}

							}
						}

						if (player == ENEMY || player == TEAM)
						{
							ImGui::Checkbox(crypt_str("Ragdoll chams"), &g_cfg.player.type[player].ragdoll_chams);

							if (g_cfg.player.type[player].ragdoll_chams)
							{
								ImGui::Spacing();
								ImGui::Text("ragdoll chams material");
								ImGui::Combo("##ragdoll chams material", &g_cfg.player.type[player].ragdoll_chams_material, "Regular\0Mettalic\0Flat\0Pulse\0Crystal\0Glass\0Circuit\0Golden\0Glow\0");

								ImGui::Text(crypt_str("Color "));
								ImGui::SameLine();
								ImGui::ColorEdit(crypt_str("##ragdollcolor"), &g_cfg.player.type[player].ragdoll_chams_color, ALPHA);
							}
						}
						else if (!g_cfg.player.local_chams_type)
						{
							ImGui::Checkbox(crypt_str("Transparency in scope"), &g_cfg.player.transparency_in_scope);

							if (g_cfg.player.transparency_in_scope)
								ImGui::Spacing();
							ImGui::SliderFloat(crypt_str("Alpha"), &g_cfg.player.transparency_in_scope_amount, 0.0f, 1.0f);
							ImGui::Spacing();

						}

						ImGui::Spacing();

						ImGui::Checkbox(crypt_str("Glow"), &g_cfg.player.type[player].glow);

						if (g_cfg.player.type[player].glow)
						{
							ImGui::Spacing();
							ImGui::Text("Glow type");
							ImGui::Combo("##Glow type", &g_cfg.player.type[player].glow_type, "Standard\0Pulse\0Inner");
							ImGui::Text(crypt_str("Color "));
							ImGui::SameLine();
							ImGui::ColorEdit(crypt_str("##glowcolor"), &g_cfg.player.type[player].glow_color, ALPHA);
						}


						ImGui::PopItemWidth();
						break;
					}

					case 2: {
						ImGui::PushItemWidth(240);
						static auto checkbox = false;
						static auto item = 0, slider = 0, slider2 = 0;
						draw->AddText(Title, 20.f, p + ImVec2(240, 25), ImColor(255, 255, 255, 255), "Grenades");

						ImGui::Checkbox("Grenade prediction", &g_cfg.esp.grenade_prediction);
						ImGui::SameLine();
						ImGui::ColorEdit("##grenpredcolor", &g_cfg.esp.grenade_prediction_color, ALPHA);

						if (g_cfg.esp.grenade_prediction)
						{
							ImGui::Checkbox("On click", &g_cfg.esp.on_click);
							ImGui::Text(crypt_str("Tracer color "));
							ImGui::SameLine();
							ImGui::ColorEdit("##tracergrenpredcolor", &g_cfg.esp.grenade_prediction_tracer_color, ALPHA);
						}

						ImGui::Checkbox("Grenade projectiles", &g_cfg.esp.projectiles);


						if (g_cfg.esp.projectiles)
						{
							ImGui::Spacing();
							ImGui::Text("Grenade esp");
							draw_multicombo(crypt_str("Grenade ESP"), g_cfg.esp.grenade_esp, proj_combo, ARRAYSIZE(proj_combo), preview);
						}

						if (g_cfg.esp.grenade_esp[GRENADE_ICON] || g_cfg.esp.grenade_esp[GRENADE_TEXT])
						{
							ImGui::Text(crypt_str("Color "));
							ImGui::SameLine();
							ImGui::ColorEdit("##projectcolor", &g_cfg.esp.projectiles_color, ALPHA);
						}

						if (g_cfg.esp.grenade_esp[GRENADE_BOX])
						{
							ImGui::Text(crypt_str("Box color "));
							ImGui::SameLine();
							ImGui::ColorEdit("##grenade_box_color", &g_cfg.esp.grenade_box_color, ALPHA);
						}

						if (g_cfg.esp.grenade_esp[GRENADE_GLOW])
						{
							ImGui::Text(crypt_str("Glow color "));
							ImGui::SameLine();
							ImGui::ColorEdit("##grenade_glow_color", &g_cfg.esp.grenade_glow_color, ALPHA);
						}

						ImGui::Checkbox("Grenade Warning", &g_cfg.esp.grenadetracer1);//g_cfg.esp.molotov_timer

						if (g_cfg.esp.grenadetracer1)
						{
							ImGui::Spacing();
							ImGui::Text("Grenade warning type");
							ImGui::Combo("##Grenade warning type", &g_cfg.esp.grenadetracer, "None\0Beam\0Line");
							ImGui::SameLine();
							ImGui::ColorEdit("##GrenadeTrailscolor", &g_cfg.esp.grenadetrace, ALPHA);

							ImGui::Checkbox("Advanced mode", &g_cfg.esp.advance_mode);
							ImGui::Spacing();
							ImGui::SliderFloat("Distance limit", &g_cfg.esp.dist, 0, 2000);
						}

						ImGui::Checkbox("Molotov Timer", &g_cfg.esp.molotov_timer);

						ImGui::Checkbox("Smoke Timer", &g_cfg.esp.smoke_timer);

						ImGui::PopItemWidth();
						break;
					}

					case 3: {
						ImGui::PushItemWidth(240);
						static auto checkbox = false;
						static auto item = 0, slider = 0, slider2 = 0;
						draw->AddText(Title, 20.f, p + ImVec2(240, 25), ImColor(255, 255, 255, 255), "Weapons");

						ImGui::Text("Weapon esp");
						draw_multicombo(crypt_str("Weapon esp"), g_cfg.esp.weapon, weaponesp, ARRAYSIZE(weaponesp), preview);

						if (g_cfg.esp.weapon[WEAPON_ICON] || g_cfg.esp.weapon[WEAPON_TEXT] || g_cfg.esp.weapon[WEAPON_DISTANCE])
						{
							ImGui::Text(crypt_str("Color "));
							ImGui::SameLine();
							ImGui::ColorEdit("##weaponcolor", &g_cfg.esp.weapon_color, ALPHA);
						}

						if (g_cfg.esp.weapon[WEAPON_BOX])
						{
							ImGui::Text(crypt_str("Box color "));
							ImGui::SameLine();
							ImGui::ColorEdit("##weaponboxcolor", &g_cfg.esp.box_color, ALPHA);
						}

						if (g_cfg.esp.weapon[WEAPON_GLOW])
						{
							ImGui::Text(crypt_str("Glow color "));
							ImGui::SameLine();
							ImGui::ColorEdit("##weaponglowcolor", &g_cfg.esp.weapon_glow_color, ALPHA);
						}

						if (g_cfg.esp.weapon[WEAPON_AMMO])
						{
							ImGui::Text(crypt_str("Ammo bar color "));
							ImGui::SameLine();
							ImGui::ColorEdit("##weaponammocolor", &g_cfg.esp.weapon_ammo_color, ALPHA);
						}

						ImGui::PopItemWidth();
						break;
					}

					case 4: {
						ImGui::PushItemWidth(240);
						static auto checkbox = false;
						static auto item = 0, slider = 0, slider2 = 0;
						draw->AddText(Title, 20.f, p + ImVec2(240, 25), ImColor(255, 255, 255, 255), "Another");

						ImGui::Text("Removals");
						draw_multicombo(crypt_str("Removals"), g_cfg.esp.removals, removals, ARRAYSIZE(removals), preview);

						if (g_cfg.esp.removals[REMOVALS_ZOOM])
							ImGui::Checkbox("Fix zoom sensitivity", &g_cfg.esp.fix_zoom_sensivity);

						ImGui::Checkbox("Client bullet impacts", &g_cfg.esp.client_bullet_impacts);
						ImGui::SameLine();
						ImGui::ColorEdit("##clientbulletimpacts", &g_cfg.esp.client_bullet_impacts_color, ALPHA);

						ImGui::Checkbox("Server bullet impacts", &g_cfg.esp.server_bullet_impacts);
						ImGui::SameLine();
						ImGui::ColorEdit("##serverbulletimpacts", &g_cfg.esp.server_bullet_impacts_color, ALPHA);

						ImGui::Checkbox("Local bullet tracers", &g_cfg.esp.bullet_tracer);
						ImGui::SameLine();
						ImGui::ColorEdit("##bulltracecolor", &g_cfg.esp.bullet_tracer_color, ALPHA);

						ImGui::Checkbox("Enemy bullet tracers", &g_cfg.esp.enemy_bullet_tracer);
						ImGui::SameLine();
						ImGui::ColorEdit("##enemybulltracecolor", &g_cfg.esp.enemy_bullet_tracer_color, ALPHA);

						ImGui::Spacing();
						ImGui::Text("Hitmarker");
						draw_multicombo(crypt_str("Hitmarker"), g_cfg.esp.hitmarker, hitmarkers, ARRAYSIZE(hitmarkers), preview);
						ImGui::Checkbox("Damage marker", &g_cfg.esp.damage_marker);
						ImGui::SameLine(200);
						ImGui::ColorEdit("##dmgindcol", &g_cfg.esp.damage_marker_color, ALPHA);

						ImGui::Checkbox("Taser range", &g_cfg.esp.taser_range);
						ImGui::Checkbox("Penetration dot", &g_cfg.esp.penetration_reticle);

						ImGui::PopItemWidth();
						break;
					}
					case 5: {
						ImGui::PushItemWidth(240);
						static auto checkbox = false;
						static auto item = 0, slider = 0, slider2 = 0;
						draw->AddText(Title, 20.f, p + ImVec2(240, 25), ImColor(255, 255, 255, 255), "World");

						ImGui::Checkbox("NightMode", &g_cfg.esp.nightmode);

						ImGui::Checkbox("Full bright", &g_cfg.esp.bright);

						ImGui::Spacing();
						ImGui::Text("Target selection");
						//ImGui::Combo("##Skybox", &g_cfg.esp.skybox, "Cycle\0Near crosshair\0Lowest distance\0Lowest health\0Highest damage");
						ImGui::Combo("##Skybox", &g_cfg.esp.skybox, "None\0Tibet\0Baggage\0Italy\0Aztec\0Vertigo\0Daylight\0Daylight 2\0Clouds\0Clouds 2\0Gray\0test\0Clear\0Canals\0Cobblestone\0Assault\0Clouds dark\0Night\0Night 2\0Night flat\0Dusty\0Rainy\0Custom");

						ImGui::Text(crypt_str("Color "));
						ImGui::SameLine();
						ImGui::ColorEdit("##skyboxcolor", &g_cfg.esp.skybox_color, NOALPHA);

						if (g_cfg.esp.skybox == 21)
						{
							static char sky_custom[64] = "\0";

							if (!g_cfg.esp.custom_skybox.empty())
								strcpy_s(sky_custom, sizeof(sky_custom), g_cfg.esp.custom_skybox.c_str());

							ImGui::Text(crypt_str("Name"));
							ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.f);

							if (ImGui::InputText(crypt_str("##customsky"), sky_custom, sizeof(sky_custom)))
								g_cfg.esp.custom_skybox = sky_custom;

							ImGui::PopStyleVar();
						}

						if (g_cfg.esp.nightmode)
						{
							ImGui::Text("World color ");
							ImGui::SameLine();
							ImGui::ColorEdit("##worldcolor", &g_cfg.esp.world_color, ALPHA);

							ImGui::Text("Props color ");
							ImGui::SameLine();
							ImGui::ColorEdit("##propscolor", &g_cfg.esp.props_color, ALPHA);
						}

						ImGui::Checkbox("World modulation", &g_cfg.esp.world_modulation);

						if (g_cfg.esp.world_modulation)
						{
							ImGui::Spacing();
							ImGui::SliderFloat("Bloom", &g_cfg.esp.bloom, 0.0f, 750.0f);
							ImGui::Spacing();
							ImGui::SliderFloat("Exposure", &g_cfg.esp.exposure, 0.0f, 2000.0f);
							ImGui::Spacing();
							ImGui::SliderFloat("Ambient", &g_cfg.esp.ambient, 0.0f, 1500.0f);
						}

						ImGui::Checkbox("Fog modulation", &g_cfg.esp.fog);

						if (g_cfg.esp.fog)
						{
							ImGui::Spacing();
							ImGui::SliderInt("Distance", &g_cfg.esp.fog_distance, 0, 2500);
							ImGui::Spacing();
							ImGui::SliderInt("Density", &g_cfg.esp.fog_density, 0, 100);

							ImGui::Text("Color ");
							ImGui::SameLine();
							ImGui::ColorEdit("##fogcolor", &g_cfg.esp.fog_color, NOALPHA);
						}

						ImGui::PopItemWidth();
						break;
					}
					case 6: {
						ImGui::PushItemWidth(240);
						static auto checkbox = false;
						static auto item = 0, slider = 0, slider2 = 0;
						draw->AddText(Title, 20.f, p + ImVec2(240, 25), ImColor(255, 255, 255, 255), "Indicators");

						ImGui::Text("Main Indicators");
						draw_multicombo(crypt_str("Main Indicators"), g_cfg.esp.indicators, indicators, ARRAYSIZE(indicators), preview);
						padding(0, 3);

						ImGui::Checkbox("Watermark", &g_cfg.menu.watermark);
						ImGui::Text(crypt_str("Watermark Color"));
						ImGui::SameLine();
						ImGui::ColorEdit("##watermark_color", &g_cfg.misc.watermark_color, ALPHA);
						ImGui::Checkbox("Spectators List", &g_cfg.misc.spectators_list);
						ImGui::Text(crypt_str("Spectator list Color"));
						ImGui::SameLine();
						ImGui::ColorEdit("##spectator_list_color", &g_cfg.esp.spectatorss, ALPHA);
						ImGui::Checkbox("Keybinds", &g_cfg.misc.keybinds);
						ImGui::Text(crypt_str("Keybinds Color"));
						ImGui::SameLine();
						ImGui::ColorEdit("##keybinds_color", &g_cfg.esp.keybinds, ALPHA);
						ImGui::Checkbox("Information Bar", &g_cfg.misc.InfoBar);
						ImGui::Checkbox("Stats indicators", &g_cfg.misc.evoinds);
						ImGui::Checkbox("Centered Indicators", &g_cfg.misc.centerinds);

						if (g_cfg.misc.centerinds) {
							ImGui::Text(crypt_str("Color active "));
							ImGui::SameLine();
							ImGui::ColorEdit("##centerinds_active_color", &g_cfg.misc.centerinds_active, ALPHA);
							ImGui::Text(crypt_str("Color inactive"));
							ImGui::SameLine();
							ImGui::ColorEdit("##centerinds_inactive_color", &g_cfg.misc.centerinds_inactive, ALPHA);
						}

						ImGui::Checkbox("Fake Line", &g_cfg.misc.fakeline);

						if (g_cfg.misc.fakeline) {
							ImGui::Text(crypt_str("Fake line Color"));
							ImGui::SameLine();
							ImGui::ColorEdit("##fakeline_color", &g_cfg.misc.fakeline_color, ALPHA);
						}

						if (g_cfg.esp.removals[REMOVALS_ZOOM])
							ImGui::Checkbox("Fade Scope", &g_cfg.esp.fade_scope);

						if (g_cfg.esp.fade_scope) {

							ImGui::Text(crypt_str("Fade Scope Color 1 "));
							ImGui::SameLine();
							ImGui::ColorEdit("##fade1motherfucker", &g_cfg.esp.fade1, ALPHA);
							ImGui::Text(crypt_str("Fade Scope Color 2 "));
							ImGui::SameLine();
							ImGui::ColorEdit("##fadeshitaye228", &g_cfg.esp.fade2, ALPHA);

							ImGui::SliderFloat("size", &g_cfg.esp.size_sniper, 0, 400);
							ImGui::SliderFloat("gap", &g_cfg.esp.gap_sniper, 0, 150);
						}

						ImGui::Checkbox("better hitmarker", &g_cfg.esp.hitmarker2);

						ImGui::Checkbox("Circle Desync Indicator", &g_cfg.esp.circlefake);
					    ImGui::Checkbox("Bomb Timer", &g_cfg.esp.bomb_timer);


						ImGui::PopItemWidth();
						break;
					}

					case 7: {
						ImGui::PushItemWidth(240);
						static auto checkbox = false;
						static auto item = 0, slider = 0, slider2 = 0;
							draw->AddText(Title, 20.f, p + ImVec2(240, 25), ImColor(255, 255, 255, 255), "Viewmodel");

							draw_keybind(crypt_str("Thirdperson"), &g_cfg.misc.thirdperson_toggle, crypt_str("##TPKEY__HOTKEY"));

							ImGui::Checkbox("Thirdperson when spectating", &g_cfg.misc.thirdperson_when_spectating);
							ImGui::Checkbox("Field of view while zoomed", &g_cfg.esp.fov_while_zoomed);

							ImGui::Spacing();
							if (g_cfg.misc.thirdperson_toggle.key > KEY_NONE && g_cfg.misc.thirdperson_toggle.key < KEY_MAX)
								ImGui::SliderInt("Thirdperson distance", &g_cfg.misc.thirdperson_distance, 50, 300);
							ImGui::Spacing();
							ImGui::Checkbox("Rare animations", &g_cfg.skins.rare_animations);
							ImGui::Spacing();
							ImGui::SliderInt("Viewmodel field of view", &g_cfg.esp.viewmodel_fov, 0, 180);
							ImGui::Spacing();
							ImGui::SliderInt("Field of view", &g_cfg.esp.fov, 0, 180);
							ImGui::Spacing();
							ImGui::SliderInt("Viewmodel X", &g_cfg.esp.viewmodel_x, -50, 50);
							ImGui::Spacing();
							ImGui::SliderInt("Viewmodel Y", &g_cfg.esp.viewmodel_y, -50, 50);
							ImGui::Spacing();
							ImGui::SliderInt("Viewmodel Z", &g_cfg.esp.viewmodel_z, -50, 50);
							ImGui::Spacing();
							ImGui::SliderInt("Viewmodel roll", &g_cfg.esp.viewmodel_roll, -180, 180);

							ImGui::Checkbox("Gravity ragdolls", &g_cfg.misc.ragdolls);
							ImGui::Checkbox("Preserve killfeed", &g_cfg.esp.preserve_killfeed);

							ImGui::Checkbox("Aspect Ratio", &g_cfg.misc.aspect_ratio);

							if (g_cfg.misc.aspect_ratio)
							{
								padding(0, -5);
								ImGui::Spacing();
								ImGui::SliderFloat("Amount", &g_cfg.misc.aspect_ratio_amount, 0.5f, 2.0f);
								ImGui::Spacing();
							}

						ImGui::PopItemWidth();
						break;
					}
				}
				ImGui::EndGroup();
				break;
					}
				}
			case 2: {
				static auto subtab = -1;

				ImGui::SetCursorPos(ImVec2(64, 20));
				ImGui::BeginGroup();
				{

					if (ImGui::CollapsingHeader("Misc")) {
						ImGui::NewLine();

						if (gui.subtab("Other", subtab == 0, ImVec2(160, 35)))
							subtab = 0;
						if (gui.subtab("Movement", subtab == 1, ImVec2(160, 35)))
							subtab = 1;
					}
				}
				ImGui::EndGroup();

				ImGui::SetCursorPos(ImVec2(240, 55));
				ImGui::BeginGroup();
				{
					switch (subtab) {

					case 0: {
						ImGui::PushItemWidth(240);
						static auto checkbox = false;
						auto player = players_section;
						static auto item = 0, slider = 0, slider2 = 0;
						draw->AddText(Title, 20.f, p + ImVec2(240, 25), ImColor(255, 255, 255, 255), "Other");

						ImGui::Checkbox("Anti-untrusted", &g_cfg.misc.anti_untrusted);
						ImGui::Checkbox("Rank reveal", &g_cfg.misc.rank_reveal);
						ImGui::Checkbox("Unlock inventory access", &g_cfg.misc.inventory_access);

						ImGui::Checkbox("Enable buybot", &g_cfg.misc.buybot_enable);

						if (g_cfg.misc.buybot_enable)
						{

							ImGui::Spacing();
							ImGui::Text("Snipers");
							ImGui::Combo("##Snipers", &g_cfg.misc.buybot1, "None\0Auto\0AWP\0SSG 08");
							padding(0, 3);

							ImGui::Text("Pistols");
							ImGui::Combo("##Pistols", &g_cfg.misc.buybot2, "None\0Dual Berettas\0Deagle/Revolver");
							padding(0, 3);

							ImGui::Text("Others");
							draw_multicombo(crypt_str("Others"), g_cfg.misc.buybot3, grenades, ARRAYSIZE(grenades), preview);
						}

						ImGui::Spacing();
						ImGui::Text("Logs");
						draw_multicombo(crypt_str("Logs"), g_cfg.misc.events_to_log, events, ARRAYSIZE(events), preview);
						padding(0, 3);

						ImGui::Text("Logs output");
						draw_multicombo(crypt_str("Logs output"), g_cfg.misc.log_output, events_output, ARRAYSIZE(events_output), preview);

						const char* sounds[] =
						{
							"None",
							"Mettalic",
							"Cod",
							"Bubble",
							"Stapler",
							"Bell",
							"Flick"
						};

						ImGui::Text("Hitsound");
						ImGui::Combo("##Hitsound", &g_cfg.esp.hitsound, "None\0Mettalic\0Cod\0Bubble\0Stapler\0Bell\0Flick");

						if (g_cfg.misc.events_to_log[EVENTLOG_HIT] || g_cfg.misc.events_to_log[EVENTLOG_ITEM_PURCHASES] || g_cfg.misc.events_to_log[EVENTLOG_BOMB])
						{
							ImGui::Text(crypt_str("Color "));
							ImGui::SameLine();
							ImGui::ColorEdit("##logcolor", &g_cfg.misc.log_color, ALPHA);
						}

						ImGui::Checkbox("Show CS:GO logs", &g_cfg.misc.show_default_log);

						ImGui::PopItemWidth();
						break;
					}

					case 1: {
						ImGui::PushItemWidth(240);
						static auto checkbox = false;
						static auto item = 0, slider = 0, slider2 = 0;
						draw->AddText(Title, 20.f, p + ImVec2(240, 25), ImColor(255, 255, 255, 255), "Movement");

						ImGui::Checkbox("Automatic jump", &g_cfg.misc.bunnyhop);
						ImGui::Checkbox("Automatic strafes", &g_cfg.misc.airstrafe);

						if (g_cfg.misc.airstrafe)
						{
							ImGui::SliderInt("Air strafe smoothing", &g_cfg.misc.strafe_smoothing, 0, 100, false);
						}

						ImGui::Checkbox("Fast stop", &g_cfg.misc.fast_stop);
						ImGui::Checkbox("Slide walk", &g_cfg.misc.slidewalk);
						ImGui::Checkbox("No duck cooldown", &g_cfg.misc.noduck);

						draw_keybind(crypt_str("Auto peek"), &g_cfg.misc.automatic_peek, crypt_str("##AUTOPEEK__HOTKEY"));

						if (&g_cfg.misc.automatic_peek)
						{

							ImGui::Text(crypt_str("Auto peek color selection "));
							ImGui::SameLine();
							ImGui::ColorEdit("##autopeek color", &g_cfg.misc.automatic_peek2, ALPHA);
						}

						draw_keybind(crypt_str("Slow walk"), &g_cfg.misc.slowwalk_key, crypt_str("##SLOWWALK__HOTKEY"));
						ImGui::Spacing();
						ImGui::Text("Slow walk type");
						ImGui::Combo("##Slow walk type", &g_cfg.misc.slowwalk_type, "Accuracy\0Custom");

						if (g_cfg.misc.slowwalk_type)
						{
							ImGui::SliderInt("Speed", &g_cfg.misc.slowwalk_speed, 1, 180, false, crypt_str("%du/s"));
						}

						if (g_cfg.misc.noduck)
							draw_keybind(crypt_str("Fake duck"), &g_cfg.misc.fakeduck_key, crypt_str("##FAKEDUCK__HOTKEY"));

						draw_keybind(crypt_str("Edge jump"), &g_cfg.misc.edge_jump, crypt_str("##EDGEJUMP__HOTKEY"));

						ImGui::PopItemWidth();
						break;
					}
				}
					ImGui::EndGroup();
					break;
				}
			}
			case 3:
				static bool is_sure_check = false;
				static float started_think = 0;
				static std::string selected_name = "";

				ImGui::SetCursorPos(ImVec2(64, 20));
				ImGui::BeginGroup();
				{
					if (ImGui::CollapsingHeader("Configs", true)) {
						ImGui::NewLine();

						cfg_manager->config_files();
						files = cfg_manager->files;

						for (auto file : files) {
							bool is_selected = selected_name == file;

							if (gui.subtab(file.c_str(), is_selected, ImVec2(160, 35))) {
								selected_name = is_selected ? "" : file;

								is_sure_check = false;
								started_think = 0;
							}
						}
					}

					ImGui::PushItemWidth(240);

					ImGui::SetCursorPos(ImVec2(240, 55));
					ImGui::BeginGroup();

					if (!selected_name.empty()) {
						draw->AddText(Title, 20.f, p + ImVec2(240, 25), ImColor(255, 255, 255, 255), selected_name.c_str());

						if (ImGui::Button("Load", ImVec2(240, 25)))
							load_config(selected_name);

						if (ImGui::Button("Save", ImVec2(240, 25)))
							save_config(selected_name);

						if (ImGui::Button((is_sure_check ? "Are you sure about deleting cfg? " : "Remove"), ImVec2(240, 25))) {
							if (is_sure_check) {
								remove_config(selected_name);
								is_sure_check = false;
							}
							else {
								if (started_think == 0)
									started_think = GetTickCount();
								else if (started_think > 400) {
									is_sure_check = true;

									started_think = 0;
								}
							}
						}
					}
					else {
						static char config_name[64] = "\0";

						ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.f);
						ImGui::InputText(crypt_str("##configname"), config_name, sizeof(config_name));
						ImGui::PopStyleVar();

						if (ImGui::Button("Create config", ImVec2(240, 25)))
							add_config(config_name);
					}

					ImGui::PopItemWidth();
					ImGui::EndGroup();

					break;
				}
			}
		}
	}
	ImGui::End();

	ImGui::PopStyleColor();
	ImGui::PopStyleVar();
}