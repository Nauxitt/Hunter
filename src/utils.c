void printRect(SDL_Rect * rect){
	if(rect == NULL)
		printf("{NULL PTR RECT}");
	else
		printf("{%d, %d, %d, %d}", rect->x, rect->y, rect->w, rect->h);
}
