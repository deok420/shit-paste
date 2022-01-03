#include "..\..\includes.hpp"

//enum e_color_mode {
//	STATIC = 1, /// Just accent color
//	RAINBOW,        /// Dynamic color
//	GRADIENT    /// Gradient dynamic color
//};
//
///// Watermark options to draw.
//enum e_watermark_options {
//	NONE = 0, /// Default argument, only logo will be drawn.
//	USER = (1 << 0), /// User name
//	INCOMING = (1 << 1), /// Incoming latency (only in-game)
//	OUTGOING = (1 << 2), /// Outgoing latency (only in-game)
//	TIME = (1 << 3)  /// Current Windows time
//};
//
//struct style_t {
//	e_color_mode    m_col_mode = RAINBOW; /// Elements accent color mode.
//	Color                m_col_fallback = Color(210, 150, 20, 200); /// Change your accent color here.
//	Color                m_col_accent[2]; /// Final color, will be used in elements rendering.
//
//	/// Watermark stuff.
//	Vector                m_watermark_paddings = Vector(5, 3);
//	Vector2D                m_watermark_margins = Vector(10, 10);
//
//	/// Default elements stuff.
//	size_t            m_outline_size = 4; /// Main outline size (in pixels).
//	//size_t            m_line_height = render::measure_text(font::tahoma_8, "A").y;
//
//	size_t            m_pad_base = 5;
//	size_t            m_pad_header_top = 6;
//	size_t            m_pad_header_sides = 6;
//	size_t            m_pad_header_bottom = 7;
//
//	Color                m_col_base = Color(0, 0, 0);
//	Color                m_col_base_outer_outline = Color(0, 0, 0, 220);
//	Color                m_col_base_inner_outline = Color(255, 255, 255, 4);
//
//	Color                m_col_outline = Color(0, 0, 0, 200);
//	Color                m_col_outer_outline = Color(0, 0, 0, 220);
//	Color                m_col_inner_outline = Color(255, 255, 255, 8);
//} m_style;

class misc : public singleton <misc> 
{
public:
	void watermark();
	void draw_watermark(uint8_t options);
	void zeus_range();
	void NoDuck(CUserCmd* cmd);
	void AutoCrouch(CUserCmd* cmd);
	void SlideWalk(CUserCmd* cmd);
	void automatic_peek(CUserCmd* cmd);
	void ViewModel();
	void FullBright();
	void PovArrows(player_t* e, Color color);
	void NightmodeFix();
	void desync_arrows();
	void aimbot_hitboxes();
	//void aimbot_hitboxes();
	void ragdolls();
	void rank_reveal();
	void fast_stop(CUserCmd* m_pcmd);
	void spectators_list();	
	void double_tap_deffensive(CUserCmd* m_pcmd);
	bool double_tap(CUserCmd* m_pcmd);
	void hide_shots(CUserCmd* m_pcmd, bool should_work);

	void lagcompexploit(CUserCmd* m_pcmd);

	void FastFeautres(CUserCmd* pCmd);

	void JumpThrow(CUserCmd* Cmd);

	bool recharging_double_tap = false;

	bool double_tap_enabled = false;
	bool double_tap_key = false;

	bool hide_shots_enabled = false;
	bool hide_shots_key = false;
};