#include "circlestrafer.h"

static auto trace_fraction_worldprops( Vector start, Vector end ) -> float {
	Ray_t ray;
	CGameTrace tr;
	CTraceFilter filter;

	filter.pSkip = g_ctx.local();

	ray.Init( start, end );
	m_trace()->TraceRay( ray, MASK_SOLID, &filter, &tr );

	return tr.fraction;
};

void circlestrafer::start( ) {
	if (g_ctx.local()->get_move_type( ) == MOVETYPE_NOCLIP || g_ctx.local()->get_move_type( ) == MOVETYPE_LADDER ) {
		return;
	}

	strafe_angle = 0;
	active = true;

	Vector angles; m_engine( )->GetViewAngles( angles );
	angles.x = 0;

	Vector
		forward = angles.Direction( ),
		right = forward.Cross( Vector( 0, 0, 1 ) ),
		left = Vector( -right.x, -right.y, right.z );

	float left_path = trace_fraction_worldprops( g_ctx.local()->m_vecOrigin( ) + Vector( 0, 0, 10 ), g_ctx.local()->m_vecOrigin( ) + left * 250.0f + Vector( 0, 0, 10 ) );
	float right_path = trace_fraction_worldprops(g_ctx.local()->m_vecOrigin( ) + Vector( 0, 0, 10 ), g_ctx.local()->m_vecOrigin( ) + right * 250.0f + Vector( 0, 0, 10 ) );

	if ( left_path > right_path )
		right_movement = -1;
	else
		right_movement = 1;
}

void circlestrafer::strafe( ) {
	Vector velocity = g_ctx.local()->m_vecVelocity( );
	velocity.z = 0;

	float speed = velocity.Length( );
	if ( speed < 45 ) speed = 45;
	if ( speed > 800 ) speed = 800;

	float final_path = trace_fraction_worldprops(g_ctx.local()->m_vecOrigin( ) + Vector( 0, 0, 10 ), g_ctx.local()->m_vecOrigin( ) + Vector( 0, 0, 10 ) + velocity / 3.0f );
	float delta_angle = right_movement * fmax( ( 275.0f / speed ) * ( 2.0f / final_path ) * ( 128.0f / ( 1.7f / m_globals( )->m_intervalpertick ) ) * 4.20f, 2.0f );
	strafe_angle += delta_angle;

	if ( fabs( strafe_angle ) >= 360.0f ) {
		strafe_angle = 0.0f;
		active = false;
		right_movement = 0;
	} else {
		g_ctx.get_command()->m_forwardmove = cos( ( strafe_angle + 90 * right_movement ) * ( DirectX::XM_PI / 180.0f ) ) * 450.0f;
		g_ctx.get_command( )->m_sidemove = sin( ( strafe_angle + 90 * right_movement ) * ( DirectX::XM_PI / 180.0f ) ) * 450.0f;
	}
}










































