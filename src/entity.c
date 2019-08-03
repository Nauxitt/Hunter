#include "entity.h"

void entityOnDraw(EventHandler * h){
	Entity * entity = Entity(h);
	AnimationFrame * frame = entity->animation_frame;

	if(frame == NULL)
		frame = entity->animation_frame = entity->animation;

	// Handle animation timing
	entity->animation_finished = 0;

	if(frame){
		uint32_t time = SDL_GetTicks();
		
		// If the entity has no animation timing data, create
		// it and don't update to the next frame.
		if(entity->last_frame == 0){
			entity->last_frame = time;
		}

		// If it is time, update the frame, but only if there
		// is a new frame to update to.
		else if(time - entity->last_frame >= frame->duration){
			
			// next frame?
			if(entity->animation_frame->next)
				entity->animation_frame = entity->animation_frame->next;

			// animation end
			else {
				entity->animation_finished = 1;
			
				if (entity->animation_loop)
					frame = entity->animation_frame = entity->animation;
			}

			entity->last_frame = time;
		}
	}
	
	// Handle flips
	uint8_t flip_v=0, flip_h=0;
	flip_h = entity->flip_h;
	flip_v = entity->flip_v;

	if(frame){
		flip_h ^= frame->flip_h;
		flip_v ^= frame->flip_v;
	}
	
	SDL_RendererFlip flip = SDL_FLIP_NONE;
	if(flip_h) flip |= SDL_FLIP_HORIZONTAL;
	if(flip_v) flip |= SDL_FLIP_VERTICAL;

	// Handle source clipping
	SDL_Rect * src = NULL;
	if(frame)
		src = &frame->clip;

	// Rendering destination
	int src_w = 1;
	int src_h = 1;

	if(src){
		src_w = src->w;
		src_h = src->h;
	}
	else {
		SDL_QueryTexture(
				entity->texture,
				NULL, NULL,
				&src_w, &src_h
			);
	}

	SDL_Rect dest = {
			entity->x, entity->y,
			src_w*entity->scale_w, src_h*entity->scale_h
		};

	//    Frame center
	if(frame){
		dest.x -= frame->center_x * entity->scale_w;
		dest.y -= frame->center_y * entity->scale_h;
	}
	else {
		dest.x -= dest.w / 2;
		dest.y -= dest.h;
	}

	dest.x += entity->offset_x;
	dest.y += entity->offset_y;
	
	// Render
	SDL_RenderCopyEx(
			game.renderer, entity->texture,
			src, &dest,
			0, NULL,  // Rotation not currently supported
			flip
		);
}

void entitySetAnimation(Entity * entity, AnimationFrame * animation){
	if(entity->animation != animation){
		entity->animation = animation;
		entity->animation_frame = NULL;
	}
}

uint32_t animationGetDuration(AnimationFrame * animation){
	uint32_t duration = 0;

	for(AnimationFrame * frame = animation; frame != NULL; frame = frame->next)
		duration += frame->duration;

	return duration;
}
