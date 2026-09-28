#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#include<SDL2/SDL.h>
#include<SDL2/SDL_ttf.h>
#include<SDL2/SDL_timer.h>

int is_TTF_Init = 0;
int is_window_init = 0;
//char *text_font = "DS-DIGI.TTF";
char *text_font = "digital-7.mono.ttf";

typedef struct some_text
{
	char *text;
	SDL_Color text_color;
	char *font_path;
	int font_size;
	SDL_Rect rect;
	Uint32 rect_color;
}Some_text;

int cleanup(int return_value)
{
	if(is_TTF_Init)
	{
		TTF_Quit();
	}
	
	return(return_value);
}

static TTF_Font* open_font_px(int based_on_what, const char* path, Some_text text) // based_on_what = 1 is rect-height; based_on_what = 2 is some-width.
{
	if(text.font_size >= 0) // makes this funcion work if and only if a negative number is given.
	{
		TTF_Font *f = TTF_OpenFont(text_font, text.font_size);
		return f;
	}
	
	if(based_on_what == 1)
	{
		int target_h = text.rect.h;
		
		int pt = target_h;
		TTF_Font* f = TTF_OpenFont(path, pt);
		if (!f) return NULL;
	
		for (int i = 0; i < 8; ++i) {
			int h = TTF_FontHeight(f);
			if (h == target_h) return f;
			
			int next_pt = (pt * target_h) / h;
			if (next_pt < 1) next_pt = 1;
			if (next_pt == pt) return f;
			
			pt = next_pt;
			TTF_CloseFont(f);
			f = TTF_OpenFont(path, pt);
			if (!f) return NULL;
		}
		return f;
	}
	
	else if (based_on_what == 2)
	{
		int target_w = text.rect.w;
		const char* str = text.text;
		
		int pt = target_w;
		TTF_Font* f = TTF_OpenFont(path, pt);
		if (!f) return NULL;
		
		for (int i = 0; i < 8; ++i) {
			int w = 0, h = 0;
			if (TTF_SizeUTF8(f, str, &w, &h) != 0) {
				TTF_CloseFont(f);
				return NULL;
			}
			
			if (w == target_w) return f;
			if (w == 0) {
				TTF_CloseFont(f);
				return NULL;
			}
			
			int next_pt = (pt * target_w) / w;
			if (next_pt < 1) next_pt = 1;
			if (next_pt == pt) return f;
			
			pt = next_pt;
			TTF_CloseFont(f);
			f = TTF_OpenFont(path, pt);
			if (!f) return NULL;
		}
		return f;
	}

	else // dumbass fallback to save ass, hell yeah!
	{
		TTF_Font* f = TTF_OpenFont(path, 13);
		return f;
	}
}

int main(int argc, char **argv)
{
	TTF_Init(); is_TTF_Init = 1;
	
	SDL_Window *pclock_window = SDL_CreateWindow("Desktop Window", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 200, 100, 0);
	
	SDL_Surface *pclock_surface = SDL_GetWindowSurface(pclock_window);
	SDL_Rect app_surface_rect = {0, 0, 200, 100};
	Uint32 app_surface_color = 0x282A36;
	
	time_t now = time(NULL);
	struct tm *t = localtime(&now);
	
	//toolbar
	SDL_Rect toolbar_rect = {0, 0, 200, 15};
	Uint32 toolbar_rect_color = 0x181A26;
	
	
	//main clock
	char *current_time = malloc(8 * sizeof(char));
	sprintf(current_time, "%02d:%02d.%02d", t->tm_hour, t->tm_min, t->tm_sec);
	
	SDL_Rect main_clock_rect = {5, 20, 80, 20};
	Uint32 main_clock_rect_color = 0x502A36;
	char *main_clock_font = text_font;
	SDL_Color main_clock_text_color = {248, 248, 242, 255};
	int main_clock_text_size = -1;
	
	Some_text main_clock = (Some_text){current_time, main_clock_text_color, main_clock_font, main_clock_text_size, main_clock_rect, main_clock_rect_color};
	
	
	
	//timer clock
	char *timer_time = malloc(8 * sizeof(char));
	sprintf(timer_time, "HELLO!");
	
	SDL_Rect timer_clock_rect = {70, 60, 80, 20};
	Uint32 timer_clock_rect_color = 0x282A52;
	char *timer_clock_font = text_font;
	SDL_Color timer_clock_text_color = {248, 248, 242, 255};
	int timer_clock_text_size = -1;
	
	Some_text timer_clock = (Some_text){timer_time, timer_clock_text_color, timer_clock_font, timer_clock_text_size, timer_clock_rect, timer_clock_rect_color};
	
	
	
	TTF_Font *pmain_font = open_font_px(1, text_font, main_clock);
	if(!pmain_font)
	{
		SDL_Log("Failed to load font: %s", TTF_GetError());
		return cleanup(1);
	}
	TTF_Font *ptimer_font = open_font_px(1, text_font, timer_clock);
	if(!ptimer_font)
	{
		SDL_Log("Failed to load font: %s", TTF_GetError());
		return cleanup(1);
	}
	
	
	int app_running = 1;
	SDL_Event event;
	while(app_running)
	{
		now = time(NULL);
		t = localtime(&now);
		sprintf(current_time, "%02d:%02d.%02d", t->tm_hour, t->tm_min, t->tm_sec);
		main_clock.text = current_time;
		
		SDL_FillRect(pclock_surface, &app_surface_rect, app_surface_color);
		SDL_FillRect(pclock_surface, &toolbar_rect, toolbar_rect_color);
		SDL_FillRect(pclock_surface, &main_clock.rect, main_clock.rect_color);
		SDL_FillRect(pclock_surface, &timer_clock.rect, timer_clock.rect_color);
		
		SDL_Surface *pmain_time_surface = TTF_RenderText_Solid(pmain_font, main_clock.text, main_clock.text_color);
		if(!pmain_time_surface)
		{
			SDL_Log("Failed to render text: %s", TTF_GetError());
			cleanup(1);
		}
		SDL_Surface *ptimer_surface = TTF_RenderText_Solid(ptimer_font, timer_clock.text, timer_clock.text_color);
		if(!ptimer_surface)
		{
			SDL_Log("Failed to render text: %s", TTF_GetError());
			cleanup(1);
		}
		
		
		SDL_BlitSurface(pmain_time_surface, NULL, pclock_surface, &main_clock.rect);
		SDL_BlitSurface(ptimer_surface, NULL, pclock_surface, &timer_clock.rect);
		SDL_FreeSurface(pmain_time_surface);
		SDL_FreeSurface(ptimer_surface);
		
		while(SDL_PollEvent(&event) != 0)
		{
			if(event.type == SDL_QUIT)
				app_running = 0;
		}
		
		SDL_Delay(100);
		SDL_UpdateWindowSurface(pclock_window);
	}
	
	SDL_DestroyWindow(pclock_window);
	
	return cleanup(0);
}