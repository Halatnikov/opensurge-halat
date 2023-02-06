/*
 * Open Surge Engine
 * menu.h - menu for mobile devices
 * Copyright (C) 2008-2023  Alexandre Martins <alemartf@gmail.com>
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

#include <stdbool.h>
#include "menu.h"
#include "util/touch.h"
#include "subscenes/info.h"
#include "subscenes/screenshot.h"
#include "../../core/scene.h"
#include "../../core/storyboard.h"
#include "../../core/timer.h"
#include "../../core/util.h"
#include "../../core/video.h"
#include "../../core/image.h"
#include "../../core/input.h"
#include "../../core/logfile.h"
#include "../../entities/actor.h"
#include "../../scenes/level.h"

/* buttons */
typedef enum mobilemenu_button_t mobilemenu_button_t;
enum mobilemenu_button_t
{
    BUTTON_NONE = -1,

    BUTTON_BACK,
    BUTTON_SCREENSHOT,
    BUTTON_DEBUG,
    BUTTON_INFO,

    BUTTON_COUNT
};

typedef enum mobilemenu_buttonstate_t mobilemenu_buttonstate_t;
enum mobilemenu_buttonstate_t
{
    UNPRESSED,
    PRESSED
};

static const char* SPRITE_NAME[] = {
    [BUTTON_BACK] = "Mobile Menu - Back",
    [BUTTON_SCREENSHOT] = "Mobile Menu - Screenshot",
    [BUTTON_DEBUG] = "Mobile Menu - Debug",
    [BUTTON_INFO] = "Mobile Menu - Info"
};

static const int ANIMATION_NUMBER[] = {
    [UNPRESSED] = 0,
    [PRESSED] = 1
};

static struct {
    actor_t* actor;
    mobilemenu_buttonstate_t state;
} button[BUTTON_COUNT];

static const v2d_t INITIAL_BUTTON_POSITION = (v2d_t){ .x = 0, .y = 0 };
static v2d_t next_button_position(v2d_t button_position, const image_t* button_image);
static mobilemenu_button_t button_at(v2d_t position);
static void animate_button(mobilemenu_button_t b);

/* menu state */
typedef enum mobilemenustate_t mobilemenustate_t;
enum mobilemenustate_t
{
    APPEARING,
    WAITING,
    DISAPPEARING,

    TRIGGERED_BACK,
    TRIGGERED_SCREENSHOT,
    TRIGGERED_DEBUG,
    TRIGGERED_INFO
};

static const mobilemenustate_t TRIGGERED_STATE[] = {
    [BUTTON_BACK] = TRIGGERED_BACK,
    [BUTTON_SCREENSHOT] = TRIGGERED_SCREENSHOT,
    [BUTTON_DEBUG] = TRIGGERED_DEBUG,
    [BUTTON_INFO] = TRIGGERED_INFO
};

static mobilemenustate_t state = APPEARING;

static void update_appearing();
static void update_waiting();
static void update_disappearing();
static void update_triggered_back();
static void update_triggered_screenshot();
static void update_triggered_debug();
static void update_triggered_info();
static void (*update[])() = {
    [APPEARING] = update_appearing,
    [WAITING] = update_waiting,
    [DISAPPEARING] = update_disappearing,

    [TRIGGERED_BACK] = update_triggered_back,
    [TRIGGERED_SCREENSHOT] = update_triggered_screenshot,
    [TRIGGERED_DEBUG] = update_triggered_debug,
    [TRIGGERED_INFO] = update_triggered_info
};



/* touch/mouse input */
#define LOG(...)        logfile_message("Mobile Menu - " __VA_ARGS__)
static const inputbutton_t BACK_BUTTON = IB_FIRE4;
static input_t* input = NULL;
static input_t* mouse_input = NULL;
static void on_touch_start(v2d_t touch_start);
static void on_touch_end(v2d_t touch_start, v2d_t touch_end);
static void on_touch_move(v2d_t touch_start, v2d_t touch_current);


/* private stuff */
static const float FADE_TIME = 0.25f; /* in seconds */
static const image_t* screenshot = NULL;
static image_t* background = NULL;


/*
 * mobilemenu_init()
 * Initializes the mobile menu
 */
void mobilemenu_init(void *game_screenshot)
{
    LOG("Entered the mobile menu");

    state = APPEARING;
    screenshot = (const image_t*)game_screenshot;
    background = image_clone(video_get_backbuffer());
    mouse_input = input_create_mouse();
    input = input_create_user(NULL);

    v2d_t button_position = INITIAL_BUTTON_POSITION;
    for(mobilemenu_button_t b = 0; b < BUTTON_COUNT; b++) {
        button[b].state = UNPRESSED;

        button[b].actor = actor_create();
        button[b].actor->alpha = 0.0f;
        button[b].actor->position = button_position;
        animate_button(b);

        button_position = next_button_position(
            button_position,
            actor_image(button[b].actor)
        );
    }
}



/*
 * mobilemenu_update()
 * Updates the mobile menu
 */
void mobilemenu_update()
{
    update[state]();
    /*if(changed_scene()) return;*/
}



/*
 * mobilemenu_render()
 * Renders the mobile menu
 */
void mobilemenu_render()
{
    v2d_t camera = v2d_multiply(video_get_screen_size(), 0.5f);

    image_blit(background, 0, 0, 0, 0, image_width(background), image_height(background));

    for(mobilemenu_button_t b = 0; b < BUTTON_COUNT; b++)
        actor_render(button[b].actor, camera);
}



/*
 * mobilemenu_release()
 * Releases the mobile menu
 */
void mobilemenu_release()
{
    LOG("Left the mobile menu");

    for(mobilemenu_button_t b = 0; b < BUTTON_COUNT; b++)
        actor_destroy(button[b].actor);

    input_destroy(input);
    input_destroy(mouse_input);
    image_destroy(background);
}




/* private stuff */


/* button logic */

v2d_t next_button_position(v2d_t button_position, const image_t* button_image)
{
    v2d_t screen_size = video_get_screen_size();
    int button_width = image_width(button_image);
    int button_height = image_height(button_image);

    button_position.x += button_width;
    if(button_position.x + button_width > screen_size.x) {
        button_position.x = INITIAL_BUTTON_POSITION.x;
        button_position.y += button_height;
    }

    return button_position;
}

/* the button that is displayed at a particular point in the screen */
mobilemenu_button_t button_at(v2d_t position)
{
    for(mobilemenu_button_t b = 0; b < BUTTON_COUNT; b++) {
        v2d_t d = v2d_subtract(position, button[b].actor->position);
        int w = image_width(actor_image(button[b].actor));
        int h = image_height(actor_image(button[b].actor));

        if(0.0f <= d.x && d.x < w) {
            if(0.0f <= d.y && d.y < h)
                return b;
        }
    }

    return BUTTON_NONE;
}

void animate_button(mobilemenu_button_t b)
{
    actor_change_animation(
        button[b].actor,
        sprite_get_animation(SPRITE_NAME[b], ANIMATION_NUMBER[button[b].state])
    );
}



/* update scene */

/* appearing: fade in buttons */
void update_appearing()
{
    float dt = timer_get_delta();
    float alpha = button[BUTTON_BACK].actor->alpha;

    alpha = min(1.0f, alpha + dt / FADE_TIME);
    for(mobilemenu_button_t b = 0; b < BUTTON_COUNT; b++)
        button[b].actor->alpha = alpha;

    if(alpha >= 1.0f)
        state = WAITING;
}

/* disappearing: fade out buttons */
void update_disappearing()
{
    float dt = timer_get_delta();
    float alpha = button[BUTTON_BACK].actor->alpha;

    alpha = max(0.0f, alpha - dt / FADE_TIME);
    for(mobilemenu_button_t b = 0; b < BUTTON_COUNT; b++)
        button[b].actor->alpha = alpha;

    if(alpha <= 0.0f) {
        scenestack_pop();
        /*return;*/
    }
}

/* waiting: detect if any button is pressed */
void update_waiting()
{
    /* handle touch input */
    handle_touch_input(mouse_input, on_touch_start, on_touch_end, on_touch_move);

    /* animate buttons */
    for(mobilemenu_button_t b = 0; b < BUTTON_COUNT; b++)
        animate_button(b);

    /* check if the back button of the smartphone has been pressed */
    if(input_button_pressed(input, BACK_BUTTON))
        state = TRIGGERED_STATE[BUTTON_BACK]; /* behaves the same as pressing BACK on the screen */
}

/* triggered the back button */
void update_triggered_back()
{
    LOG("Chose option: BACK");

    state = DISAPPEARING;
}

/* triggered the screenshot button */
void update_triggered_screenshot()
{
    LOG("Chose option: SCREENSHOT");

    state = WAITING;

    mobile_subscene_t* subscene = mobile_subscene_screenshot(screenshot);
    scenestack_push(storyboard_get_scene(SCENE_MOBILEPOPUP), subscene);
}

/* triggered the debug button */
void update_triggered_debug()
{
    LOG("Chose option: DEBUG");

    state = DISAPPEARING;

    level_enter_debug_mode();
}

/* triggered the info button */
void update_triggered_info()
{
    LOG("Chose option: INFO");

    state = WAITING;

    mobile_subscene_t* subscene = mobile_subscene_info();
    scenestack_push(storyboard_get_scene(SCENE_MOBILEPOPUP), subscene);
}



/* input handling */
void on_touch_start(v2d_t touch_start)
{
    mobilemenu_button_t b = button_at(touch_start);

    if(b == BUTTON_NONE)
        return;

    button[b].state = PRESSED;
}

void on_touch_end(v2d_t touch_start, v2d_t touch_end)
{
    mobilemenu_button_t p = button_at(touch_start);
    mobilemenu_button_t q = button_at(touch_end);

    if(p == BUTTON_NONE || p != q)
        return;

    for(mobilemenu_button_t b = 0; b < BUTTON_COUNT; b++) {
        if(button[b].state == PRESSED)
            state = TRIGGERED_STATE[b];

        button[b].state = UNPRESSED;
    }
}

void on_touch_move(v2d_t touch_start, v2d_t touch_current)
{
    mobilemenu_button_t p = button_at(touch_start);
    mobilemenu_button_t q = button_at(touch_current);

    if(p == BUTTON_NONE || p == q)
        return;

    for(mobilemenu_button_t b = 0; b < BUTTON_COUNT; b++)
        button[b].state = UNPRESSED;
}