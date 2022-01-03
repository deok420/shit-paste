#pragma once
#include "../../utils/singleton.h"
#include "../../includes.hpp"

using namespace std;

class c_hud : public singleton<c_hud> {
private:
    /// Elements accent color mode.
    enum e_color_mode {
        STATIC = 1, /// Just accent color
        RAINBOW,        /// Dynamic color
        GRADIENT    /// Gradient dynamic color
    };

    /// Watermark options to draw.
    enum e_watermark_options {
        NONE = 0, /// Default argument, only logo will be drawn.
        USER = (1 << 0), /// User name
        INCOMING = (1 << 1), /// Incoming latency (only in-game)
        OUTGOING = (1 << 2), /// Outgoing latency (only in-game)
        TIME = (1 << 3)  /// Current Windows time
    };

    /// Color stuff.

    /// <summary> Returns color, based on current style. </summary>
    void update_accent_color();

    struct style_t {
        e_color_mode    m_col_mode = RAINBOW; /// Elements accent color mode.
        Color                m_col_fallback = Color(210, 150, 20, 200); /// Change your accent color here.
        Color                m_col_accent[2]; /// Final color, will be used in elements rendering.

        /// Watermark stuff.
        Vector                m_watermark_paddings = Vector(5, 3);
        Vector                m_watermark_margins = Vector(10, 10);

        /// Default elements stuff.
        size_t            m_outline_size = 4; /// Main outline size (in pixels).
        size_t            m_line_height = render::text(fonts[ESP], "A").y;

        size_t            m_pad_base = 5;
        size_t            m_pad_header_top = 6;
        size_t            m_pad_header_sides = 6;
        size_t            m_pad_header_bottom = 7;

        Color                m_col_base = Color(0, 0, 0);
        Color                m_col_base_outer_outline = Color(0, 0, 0, 220);
        Color                m_col_base_inner_outline = Color(255, 255, 255, 4);

        Color                m_col_outline = Color(0, 0, 0, 200);
        Color                m_col_outer_outline = Color(0, 0, 0, 220);
        Color                m_col_inner_outline = Color(255, 255, 255, 8);
    } m_style;

    /// Elements rendering.

    /// <summary> Usage: draw_watermark(USER | OUTGOING | TIME) for user name, outgoing latency and time rendering. </summary>
    void draw_watermark(uint8_t options = NONE);

    void draw_frame(const Vector& pos, const std::wstring& name, const std::vector<std::wstring>& first_column, const std::vector<std::wstring>& second_column = {});
    void draw_binds(const Vector& pos);
    void draw_spectators(const Vector& pos);

public:
    /// <summary> Entry function. Should be called in d3dx9 or paint hook. </summary>
    void render();
};