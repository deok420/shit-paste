
/*
*	to draw local path call somewhere in visuals get_local_data( ).draw( );
*	to predict throwed nades u need nade entity
*	u can create a std::unordered_map with all predicted nades ( make sure u predict every tick )
*
*	pseudo:
*	static auto predicted_nades = std::unordered_map< unsigned long, data_t >( );
*	static auto last_server_tick = interfaces::m_client_state->m_server_tick;
*
*	if ( interfaces::m_client_state->m_server_tick != last_server_tick ) {
*		predicted_nades.clear( );
*
*		last_server_tick = interfaces::m_client_state->m_server_tick;
*	}
*
*	------------ in entity loop ------------
*
*	const auto handle = entity->get_handle( );
*
*	if ( predicted_nades.find( handle ) == predicted_nades.end( ) ) {
*		predicted_nades[ handle ] = data_t(
*			entity->get_thrower( ), * index *, entity->get_origin( ), entity->get_velocity( ),
*			* entity + 0x29F4 ( float ) *, time_to_ticks( entity->get_sim_time( ) - * entity + 0x29F4 ( float ) * )
*		);
*	}
*
*	if ( !predicted_nades.at( handle ).draw( ) ) {
*		predicted_nades.erase( handle );
*	}
*/