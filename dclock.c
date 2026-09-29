#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#include<math.h>
#include<SDL2/SDL.h>
#include<SDL2/SDL_ttf.h>
#include<SDL2/SDL_timer.h>

#define FONT_PT_MIN  1
#define FONT_PT_MAX  512

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

static TTF_Font *open_font_fit(const Some_text *text)
{
    if (!text || !text->font_path)
        return NULL;

    /* Explicit size requested: nothing to fit. */
    if (text->font_size >= 0)
        return TTF_OpenFont(text->font_path, text->font_size);

    if (!text->text || text->text[0] == '\0')
        return NULL;
    if (text->rect.w <= 0 || text->rect.h <= 0)
        return NULL;

    int lo = FONT_PT_MIN;
    int hi = FONT_PT_MAX;
    int target_w = text->rect.w;
    int target_h = text->rect.h;

    TTF_Font *best = NULL;

    while (lo <= hi) {
        int pt = lo + (hi - lo) / 2;

        TTF_Font *f = TTF_OpenFont(text->font_path, pt);
        if (!f) {                       /* size unusable, try smaller */
            hi = pt - 1;
            continue;
        }

        int w = 0, h = 0;
        if (TTF_SizeUTF8(f, text->text, &w, &h) != 0) {
            TTF_CloseFont(f);
            break;                      /* keep whatever best we had */
        }

        if (w <= target_w && h <= target_h) {
            if (best) TTF_CloseFont(best);
            best = f;                   /* fits: remember & try bigger */
            lo   = pt + 1;
        } else {
            TTF_CloseFont(f);
            hi = pt - 1;                /* too big: try smaller */
        }
    }

    return best;
}

int main(int argc, char **argv)
{
	if(TTF_Init() !=0)
	{
		SDL_Log("SDL_Init failed: %s", SDL_GetError());
		return cleanup(1);
	}
	is_TTF_Init = 1;
	
	SDL_Window *pclock_window = SDL_CreateWindow("Desktop Window", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 200, 100, SDL_WINDOW_ALWAYS_ON_TOP /*| SDL_WINDOW_BORDERLESS*/);
	
	SDL_Surface *pclock_surface = SDL_GetWindowSurface(pclock_window);
	SDL_Rect app_surface_rect = {0, 0, 200, 100};
	Uint32 app_surface_color = 0x282A36;
	
	time_t now = time(NULL);
	struct tm *t = localtime(&now);
	
	//toolbar
	SDL_Rect toolbar_rect = {0, 0, 200, 15};
	Uint32 toolbar_rect_color = 0x181A26;
	
	
	//main clock
	char *current_time = malloc(9 * sizeof(char)); // 8 chars and 1 NULL terminator
snprintf(current_time, 9, "%02d:%02d.%02d\0", t->tm_hour, t->tm_min, t->tm_sec);
	
	SDL_Rect main_clock_rect = {30, 20, 140, 40};
	Uint32 main_clock_rect_color = 0x502A36;
	char *main_clock_font = text_font;
	SDL_Color main_clock_text_color = {248, 248, 242, 255};
	int main_clock_text_size = -1;
	
	Some_text main_clock = (Some_text){current_time, main_clock_text_color, main_clock_font, main_clock_text_size, main_clock_rect, main_clock_rect_color};
	
	
	
	//timer clock
	char *timer_time = malloc(32);
	snprintf(timer_time, 32, "00:00.00.000"); // this dumb init is for font size 
	
	SDL_Rect timer_clock_rect = {5, 65, 195, 40};
	Uint32 timer_clock_rect_color = 0x282A52;
	char *timer_clock_font = text_font;
	SDL_Color timer_clock_text_color = {248, 248, 242, 255};
	int timer_clock_text_size = -1;
	
	Some_text timer_clock = (Some_text){timer_time, timer_clock_text_color, timer_clock_font, timer_clock_text_size, timer_clock_rect, timer_clock_rect_color};
	
	int timer_hour, timer_minute, timer_second, timer_fraction;
	
	
	
	TTF_Font *pmain_font = open_font_fit(&main_clock);
	if(!pmain_font)
	{
		SDL_Log("Failed to load font: %s", TTF_GetError());
		return cleanup(1);
	}
	TTF_Font *ptimer_font = open_font_fit(&timer_clock);
	if(!ptimer_font)
	{
		SDL_Log("Failed to load font: %s", TTF_GetError());
		return cleanup(1);
	}
	
	
	Uint64 timer_start, timer_end;
	double elapsed_time;
	timer_start = SDL_GetTicks64(); timer_end = SDL_GetTicks64();
	
	
	int app_running = 1;
	SDL_Event event;
	while(app_running)
	{
		now = time(NULL);
		t = localtime(&now);
		snprintf(current_time, 9, "%02d:%02d.%02d\0", t->tm_hour, t->tm_min, t->tm_sec);
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
		
		
		while(SDL_PollEvent(&event) != 0)
		{
			if(event.type == SDL_QUIT)
				app_running = 0;
		}

		
		elapsed_time = ((timer_end - timer_start) / 1000.0);
		Uint64 elapsed_ms = timer_end - timer_start;
		
		int total_seconds = (int) elapsed_time;
		timer_hour = (total_seconds / 3600) % 24;
		timer_minute = (total_seconds / 60) % 60;
		timer_second = total_seconds % 60;
		timer_fraction = elapsed_ms % 1000;
		
		snprintf(timer_clock.text, 32, "%d:%d.%d.%03d", timer_hour, timer_minute, timer_second, timer_fraction, (int)(timer_fraction * 1000));
		
		
		SDL_BlitSurface(pmain_time_surface, NULL, pclock_surface, &main_clock.rect);
		SDL_BlitSurface(ptimer_surface, NULL, pclock_surface, &timer_clock.rect);
		SDL_FreeSurface(pmain_time_surface);
		SDL_FreeSurface(ptimer_surface);
		
		SDL_Delay(100);
		timer_end = SDL_GetTicks64();
		
		SDL_UpdateWindowSurface(pclock_window);
	}
	
	TTF_CloseFont(pmain_font);
	TTF_CloseFont(ptimer_font);
	free(current_time);
	free(timer_time);
	SDL_DestroyWindow(pclock_window);
	
	return cleanup(0);
}