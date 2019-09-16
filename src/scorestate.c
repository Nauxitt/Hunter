#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include "sprites.h"
#include "scorestate.h"
#include "stateengine.h"
#include "hunter.h"
#include "score.h"

ScoreState * makeScoreState(ScoreState * state, MatchContext * match){
	if(state == NULL)
		state = (ScoreState *) calloc(sizeof(ScoreState), 1);
	state->match = match;
	EventHandler(state)->type = "ScoreState";
	EventHandler(state)->onDraw = scoreStateOnDraw;
	EventHandler(state)->onKeyUp = scoreStateOnKeyUp;
	return state;
}

void scoreStateOnDraw(EventHandler * h){
	ScoreState * state = (ScoreState *) h;
	MatchContext * match = state->match;

	SDL_Color white = {255, 255, 255};
	
	for(int h = 0; h < PLAYERS_LENGTH; h++){
		int y = 64 + h * 100;

		// Render hunter name
		Hunter * hunter = match->characters[h];
		SDL_Surface * nameMessage = TTF_RenderText_Solid(
				game.font, &hunter->name, white
			);
		SDL_Texture * nameTexture = SDL_CreateTextureFromSurface(
				game.renderer, nameMessage
			);
		SDL_CreateTextureFromSurface(game.renderer, nameMessage);
		SDL_Rect dest = {32, y, 0, 0};
		SDL_QueryTexture(
				nameTexture,
				NULL, NULL,
				&dest.w, &dest.h
			);
		SDL_RenderCopy(game.renderer, nameTexture, NULL, &dest);

		// Render hunter's total score
		int number = totalScore(match->scores[h]);
		for(
			int place = 0;
			(place == 0) || (number > 0);
			number /= 10, place++
		){
			drawBigNumber(
					200 - place * textures.statbox.w,
					y,
					number % 10
				);
		}
	}
}

void scoreStateOnKeyUp(EventHandler * h, SDL_Event * e){
	switch(e->key.keysym.scancode){
		case SDL_SCANCODE_RETURN:
		case SDL_SCANCODE_SPACE:
		case SDL_SCANCODE_ESCAPE:
			gamePopState();
			break;
		default:
			break;
	}
}
