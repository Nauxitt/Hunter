#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <string.h>

#include "sprites.h"
#include "scorestate.h"
#include "stateengine.h"
#include "hunter.h"
#include "score.h"
#include "draw.h"

ScoreState * makeScoreState(ScoreState * state, MatchContext * match){
	if(state == NULL)
		state = (ScoreState *) calloc(sizeof(ScoreState), 1);
	state->match = match;
	EventHandler(state)->type = "ScoreState";
	EventHandler(state)->onDraw = scoreStateOnDraw;
	EventHandler(state)->onKeyUp = scoreStateOnKeyUp;

	for(int n=0; n < PLAYERS_LENGTH; n++)
		state->places[n] = n;

	// sort scores, highest to lowest

	for(int left=0; left < PLAYERS_LENGTH; left++){
		// find a higher score
		for(int right=left+1; right < PLAYERS_LENGTH; right++){
			int left_place = state->places[left];
			int right_place = state->places[right];

			// swap lower and higher score
			if(totalScore(match->scores[left_place]) < totalScore(match->scores[right])){
				state->places[left] = right_place;
				state->places[right] = left_place;
			}
		}
	}

	return state;
}


void scoreStateOnDraw(EventHandler * h){
	ScoreState * state = (ScoreState *) h;
	MatchContext * match = state->match;

	for(int h = 0; h < PLAYERS_LENGTH; h++){
		int y = 64 + h * 100;

		// Render hunter name
		Hunter * hunter = match->characters[state->places[h]];

		char msg[20];
		switch(h){
			case 0: strcpy((char*) &msg, "1ST - "); break;
			case 1: strcpy((char*) &msg, "2ND - "); break;
			case 2: strcpy((char*) &msg, "3RD - "); break;
			case 3: strcpy((char*) &msg, "4TH - "); break;
		}

		strcpy(&msg[6], (char*) &hunter->name);
		drawString((char*) &msg, 32, y);

		// Render hunter's total score
		int number = totalScore(match->scores[state->places[h]]);
		for(
			int place = 0;
			(place == 0) || (number > 0);
			number /= 10, place++
		){
			drawBigNumber(
					game.w-32 - place*textures.statbox.w,
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
