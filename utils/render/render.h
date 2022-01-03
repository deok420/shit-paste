#pragma once

#include "../../includes.hpp"
#include "..\..\ImGui\imgui.h"
#include "..\..\sdk\math\Vector2D.hpp"
enum e_font_flags {
	FONT_NONE,
	FONT_CENTERED_X = 1 << 0,
	FONT_CENTERED_Y = 1 << 1,
	FONT_CENTERED = FONT_CENTERED_X | FONT_CENTERED_Y,
	FONT_DROP_SHADOW = 1 << 2,
	FONT_OUTLINE = 1 << 4
}; 
#include <d3d9.h>

template <typename T>
struct bit_flag_t {
	bit_flag_t() = default;
	bit_flag_t(const T& value) { m_value = value; }

	__forceinline bool has(const T& value) const { return m_value & value; }

	__forceinline void add(const T& value) { m_value |= value; }

	__forceinline void remove(const T& value) { m_value &= ~value; }

	__forceinline void clear() { m_value = {}; }

	__forceinline bool empty() const { return m_value == std::numeric_limits<T>::quiet_NaN(); }

	__forceinline operator T() const { return m_value; }

	__forceinline bit_flag_t<T>& operator=(const bit_flag_t<T>& value) {
		m_value = value.m_value;

		return *this;
	}

	__forceinline T& operator=(const T& value) {
		m_value = value;

		return m_value;
	}

	T m_value = {};
};
enum render_objects {
	R_NONE,
	R_CIRCLE,
	R_CIRCLE_FILLED,
	R_RECT,
	R_RECT_FILLED,
	R_LINE,
	R_GRADIENT,

	R_POLYGON,
	R_POLYGON_FILLED,
	R_ARC,

	R_TEXT,
	R_TEXT_OUTLINE,
	R_TEXT_SHADOW,
};

class rendering {
public:
	int type = R_NONE;

	ImVec2 pos;
	ImVec2 size;

	float radius;
	int rounding;
	int points;
	float min_angle, max_angle, thickness;

	std::vector<Vector2D> poly_points;

	Color color1;
	Color color2;
	Color color3;
	Color color4;

	ImFont* font;
	int font_size;
	std::string text;
};
namespace render_new {
	крч кинь мне сурс ексодиумада 
	inline std::vector<rendering> old_proccesing;
	inline std::vector<rendering> proccesing;

	inline ImU32 GetU32(Color _color)
	{
		return ((_color[3] & 0xff) << 24) + ((_color[2] & 0xff) << 16) + ((_color[1] & 0xff) << 8)
			+ (_color[0] & 0xff);
	}

	void begin();
	void procces_render();

	Vector2D get_text_size(std::string_view txt, ImFont* font);

	void text(std::string_view txt, int font_size, Vector2D pos, const Color& clr, ImFont* font, bit_flag_t<uint8_t> flags = FONT_NONE);

	void line(const Vector2D& from, const Vector2D& to, const Color& clr);

	void rect(const Vector2D& pos, const Vector2D& size, const Color& clr, float rounding = 0.f);

	void rect_filled(const Vector2D& pos, const Vector2D& size, const Color& clr, float rounding = 0.f);

	void rect_filed_multi_clr(int x, int y, int x1, int y2, const Color& clr_upr_left, const Color& clr_upr_right, const Color& clr_bot_left, const Color& clr_bot_right);

	void polygon(std::vector<Vector2D> points, const Color& clr);

	void circle(int x, int y, int points, int radius, Color color);

	void circle_filled(int x, int y, int points, int radius, Color color);

	void polygon_filled(std::vector<Vector2D> points, const Color& clr);

	void arc(float x, float y, float radius, float min_angle, float max_angle, Color col, float thickness);

	extern ImDrawList* m_draw_list;
}