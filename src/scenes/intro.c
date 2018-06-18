/*
 * Open Surge Engine
 * intro.c - introduction scene
 * Copyright (C) 2008-2011, 2013  Alexandre Martins <alemartf(at)gmail(dot)com>
 * http://opensurge2d.org
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "intro.h"
#include "quest.h"
#include "../core/v2d.h"
#include "../core/timer.h"
#include "../core/scene.h"
#include "../core/storyboard.h"
#include "../core/fadefx.h"
#include "../core/video.h"
#include "../core/audio.h"
#include "../core/input.h"
#include "../core/osspec.h"
#include "../core/font.h"
#include "../entities/background.h"


/* private data */
#define INTRO_TIMEOUT       3.0f
static float elapsed_time;
static int must_fadein;
static image_t* bg;
static input_t* in;

#define COLOR_1            "ffe13e"
#define COLOR_2            "cdc3b6"
#define COLOR_3            "eabd85"
#define BACKGROUND         "0a0a0a"
static const char *text = 
 "<color=" COLOR_1 ">"
 GAME_TITLE " " GAME_VERSION_STRING
 "</color>\n"
 "<color=" COLOR_2 ">"
 GAME_WEBSITE
 "</color>\n"
 "\n"
 "This program is free software; you can redistribute it and/or modify "
 "it under the terms of the GNU General Public License as published by "
 "the Free Software Foundation; either version 3 of the License, or "
 "(at your option) any later version.\n"
 "\n"
 "This program is distributed in the hope that it will be useful, "
 "but WITHOUT ANY WARRANTY; without even the implied warranty of "
 "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the "
 "GNU General Public License for more details.\n"
 "\n"
 /*"You should have received a copy of the GNU General Public License "
 "along with this program.  If not, see http://www.gnu.org/licenses/"*/
;

static image_t* create_background();



/* public functions */

/*
 * intro_init()
 * Initializes the introduction scene
 */
void intro_init(void *foo)
{
    elapsed_time = 0.0f;
    must_fadein = TRUE;
    music_stop();
    bg = create_background();
    in = input_create_user(NULL);
}


/*
 * intro_release()
 * Releases the introduction scene
 */
void intro_release()
{
    input_destroy(in);
    image_destroy(bg);
}


/*
 * intro_update()
 * Updates the introduction scene
 */
void intro_update()
{
    elapsed_time += timer_get_delta();
    if(!fadefx_is_fading() && !must_fadein && (input_button_pressed(in, IB_FIRE3) || input_button_pressed(in, IB_FIRE4)))
        elapsed_time += INTRO_TIMEOUT;

    if(must_fadein) {
        fadefx_in(image_rgb(0,0,0), 1.0f);
        must_fadein = FALSE;
    }
    else if(elapsed_time >= INTRO_TIMEOUT) {
        if(fadefx_over()) {
            scenestack_pop();
            return;
        }
        fadefx_out(image_rgb(0,0,0), 1.0f);
    }

    if(music_is_playing())
        music_stop();
}



/*
 * intro_render()
 * Renders the introduction scene
 */
void intro_render()
{
    image_blit(bg, video_get_backbuffer(), 0, 0, (VIDEO_SCREEN_W - image_width(bg))/2, (VIDEO_SCREEN_H - image_height(bg))/2, image_width(bg), image_height(bg));
}





/* creates the background */
image_t* create_background()
{
    image_t* img = image_create(VIDEO_SCREEN_W, VIDEO_SCREEN_H);
    font_t *fnt = font_create("small");
    v2d_t camera = v2d_new(VIDEO_SCREEN_W/2, VIDEO_SCREEN_H/2);

    image_clear(video_get_backbuffer(), image_hex2rgb(BACKGROUND));
    font_set_width(fnt, VIDEO_SCREEN_W - 5);
    font_set_text(fnt, "%s", text);
    font_set_position(fnt, v2d_new(5, 5));
    font_render(fnt, camera);
    image_blit(video_get_backbuffer(), img, 0, 0, 0, 0, image_width(img), image_height(img));

    font_destroy(fnt);
    return img;
}
