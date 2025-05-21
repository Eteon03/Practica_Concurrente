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
#include <iostream>

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
        //Traduce SDL_Scancode (estado del teclado por frame) a enum Key_Code
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
            default: return UNDEFINED;
            }
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

    //Funcion ejecutada cada frame para procesar la entrada
    void Input_Stage::compute(float)
    {
        SDL_Event event;

        // Mantenemos el Quit en el hilo principal
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_EVENT_QUIT)
            {
                scene.stop();
            }
        }

        //Thread Pool compartido para procesamiento de entrada
        static ThreadPool pool;

        //Mutex para proteger el estado del teclado (no critico aqui)
        static std::mutex keyboard_mutex;

        //Se obtiene el estado actual del teclado
        int num_keys = 0;
        const bool* keyboard_state = SDL_GetKeyboardState(&num_keys);

        //Copiamos el estado actual a un vector para evitar acceso fuera del hilo principal
        std::vector<bool> current_state(keyboard_state, keyboard_state + num_keys);

        //Enviamos la comparacion de teclas a un hilo del thread pool
        pool.submit([this, current_state = std::move(current_state)]()
            {
                //Estado anterior, unico por hilo
                static std::vector<bool> previous_state(current_state.size(), false);

                //Acceso sincronizado
                std::lock_guard<std::mutex> lock(keyboard_mutex);

                for (size_t i = 0; i < current_state.size(); ++i)
                {
                    bool was_down = previous_state[i];
                    bool is_down = current_state[i];

                    //Se detectan cambios de estado
                    if (was_down != is_down)
                    {
                        //Convertimos el scancode a Key_Code
                        Key_Code key = internal::key_code_from_scancode(static_cast<int>(i));
                        
                        //Comprobacion de tecla pulsada
                        std::cout << "KEY PRESSED: " << key << std::endl;
                        if (key != UNDEFINED)
                        {
                            //Se determina si esta presionada o liberada
                            Key_Event::State state = is_down ? Key_Event::PRESSED : Key_Event::RELEASED;

                            //Se añade el evento a la cola de entrada
                            scene.get_input_event_queue().push(key_events.push(key, state));
                        }
                    }
                }
                //Se guarda el estado actual como anterior para el siguiente frame
                previous_state = current_state;
            });
    }


    void Input_Stage::cleanup()
    {
        scene.get_input_event_queue().clear();

        key_events.clear();
    }

}
