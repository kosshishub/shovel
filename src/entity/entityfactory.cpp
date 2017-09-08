#include <cstdio>
#include <entity/entity.hpp>
#include <log/log.hpp>
using namespace ECS;

namespace EntityFactory {

	bool createPlayer(  ){
		
		Logger::log("ENTITIYFACTORY :: Creating player...");

		int count = entityCount();
		for (int i = 0; i < count; ++i) {

			Entity*e = getEntityByLocation( i );

			if( hasComponents( e, COMPONENT_NONE ) ) {

				setComponents( e,
					  COMPONENT_LOCATION
					| COMPONENT_VELOCITY
					| COMPONENT_AIRDRAG
					| COMPONENT_CAMERA
					| COMPONENT_LOCALCONTROL
				);

				e->speed = 1.0f;
				e->acceleration = 1.0f;
				e->drag = 0.9f;
				e->location[0] = 32;
				e->location[1] = 128;
				e->location[2] = 32;


				e->velocity[0] = 0;
				e->velocity[1] = 0;
				e->velocity[2] = 0;


				e->yaw = 0;
				e->pitch = 90;

				return true;
			}
		}
		Logger::error("ENTITIYFACTORY :: ERROR :: OUT OF ENTITIES!?");
		return false;
	}


}