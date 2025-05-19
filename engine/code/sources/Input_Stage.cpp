/*
 * Copyright © 2025+ ÁRgB (angel.rodriguez@udit.es)
 *
 * Distributed under the Boost Software License, version 1.0
 * See ./LICENSE or www.boost.org/LICENSE_1_0.txt
 */

#include <engine/Input_Stage.hpp>
#include <engine/Scene.hpp>
#include <engine/Thread_Pool.hpp>
#include <mutex>
#include <SDL3/SDL.h>

namespace udit::engine
{

    namespace internal
    {

        static Key_Code key_code_from_sdl_key_code(SDL_Keycode sdl_key_code)
        {
            switch (sdl_key_code)
            {
            case SDLK_A:     return KEY_A;
            case SDLK_B:     return KEY_B;
            case SDLK_C:     return KEY_C;
            case SDLK_D:     return KEY_D;
            case SDLK_E:     return KEY_E;
            case SDLK_F:     return KEY_F;
            case SDLK_G:     return KEY_G;
            case SDLK_H:     return KEY_H;
            case SDLK_I:     return KEY_I;
            case SDLK_J:     return KEY_J;
            case SDLK_K:     return KEY_K;
            case SDLK_L:     return KEY_L;
            case SDLK_M:     return KEY_M;
            case SDLK_N:     return KEY_N;
            case SDLK_O:     return KEY_O;
            case SDLK_P:     return KEY_P;
            case SDLK_Q:     return KEY_Q;
            case SDLK_R:     return KEY_R;
            case SDLK_S:     return KEY_S;
            case SDLK_T:     return KEY_T;
            case SDLK_U:     return KEY_U;
            case SDLK_V:     return KEY_V;
            case SDLK_W:     return KEY_W;
            case SDLK_X:     return KEY_X;
            case SDLK_Y:     return KEY_Y;
            case SDLK_Z:     return KEY_Z;
            case SDLK_0:     return KEY_0;
            case SDLK_1:     return KEY_1;
            case SDLK_2:     return KEY_2;
            case SDLK_3:     return KEY_3;
            case SDLK_4:     return KEY_4;
            case SDLK_5:     return KEY_5;
            case SDLK_6:     return KEY_6;
            case SDLK_7:     return KEY_7;
            case SDLK_8:     return KEY_8;
            case SDLK_9:     return KEY_9;
            case SDLK_LEFT:  return KEY_LEFT;
            case SDLK_RIGHT: return KEY_RIGHT;
            case SDLK_UP:    return KEY_UP;
            case SDLK_DOWN:  return KEY_DOWN;
            }

            return UNDEFINED;
        }

        static Key_Code key_code_from_scancode(int scancode)
        {
            switch (scancode)
            {
            case SDL_SCANCODE_W: return KEY_W;
            case SDL_SCANCODE_A: return KEY_A;
            case SDL_SCANCODE_S: return KEY_S;
            case SDL_SCANCODE_D: return KEY_D;
            case SDL_SCANCODE_UP: return KEY_UP;
            case SDL_SCANCODE_DOWN: return KEY_DOWN;
            case SDL_SCANCODE_LEFT: return KEY_LEFT;
            case SDL_SCANCODE_RIGHT: return KEY_RIGHT;
            }

            return UNDEFINED;
        }

    }

    template<>
    Stage::Unique_Ptr Stage::create< Input_Stage >(Scene& scene)
    {
        return std::make_unique< Input_Stage >(scene);
    }

    template<>
    template<>
    Id Registrar< Stage >::record< Input_Stage >()
    {
        return registry().add("Input_Stage", Stage::create< Input_Stage >);
    }

    template<>
    Id Stage::setup< Input_Stage >()
    {
        static Id id = INVALID_ID;

        if (not_valid(id))
        {
            id = Stage::record< Input_Stage >();
        }

        return id;
    }

    template<>
    Id Stage::id_of< Input_Stage >()
    {
        return Stage::setup< Input_Stage >();
    }

    void Input_Stage::compute(float)
    {
        SDL_Event event;

        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_EVENT_QUIT)
            {
                scene.stop();
            }
        }

        static ThreadPool pool;
        static std::mutex queue_mutex;

        static std::vector<bool> previous_state;
        int num_keys = 0;
        const bool* current_state = SDL_GetKeyboardState(&num_keys);

        // Inicializamos el estado anterior la primera vez
        if (previous_state.size()!=static_cast<size_t>(num_keys))
        {
            previous_state.resize(num_keys, false);
        }

        // Copiamos el estado actual a un buffer local para pasarlo al thread pool
        std::vector<bool> current_state_copy(current_state, current_state + num_keys);

        pool.submit([this, current_state_copy, num_keys]()
            {
                static std::mutex internal_mutex;
                static std::vector<bool> local_previous_state;

                std::lock_guard<std::mutex> lock(internal_mutex);

                if (local_previous_state.size()!=current_state_copy.size())
                {
                    local_previous_state.resize(current_state_copy.size(), false);
                }

                for (size_t i = 0; i < current_state_copy.size(); i++)
                {
                    const bool was_down = local_previous_state[i];
                    const bool is_down = current_state_copy[i];

                    if (!was_down && is_down)
                    {
                        // PRESSED
                        scene.get_input_event_queue().push(
                            key_events.push(static_cast<Key_Code>(i), Key_Event::PRESSED)
                        );
                    }
                    else if (was_down && !is_down)
                    {
                        // RELEASED
                        scene.get_input_event_queue().push(
                            key_events.push(static_cast<Key_Code>(i), Key_Event::RELEASED)
                        );
                    }
                }

                local_previous_state = current_state_copy;
            });
    }

    void Input_Stage::cleanup()
    {
        scene.get_input_event_queue().clear();

        key_events.clear();
    }

}
