/*
 * Copyright © 2025+ ÁRgB (angel.rodriguez@udit.es)
 *
 * Distributed under the Boost Software License, version 1.0
 * See ./LICENSE or www.boost.org/LICENSE_1_0.txt
 */

#include <engine/Control.hpp>
#include <engine/Key_Event.hpp>
#include <engine/Path_Tracing.hpp>
#include <engine/Starter.hpp>
#include <engine/Scene.hpp>
#include <engine/Window.hpp>
#include <engine/Entity.hpp>
#include <engine/Thread_Pool.hpp>
#include <mutex>

#include "Camera_Controller.hpp"

using namespace std;
using namespace udit;
using namespace udit::engine;

namespace
{

    void load_camera (Scene & scene, std::mutex& mtx)
    {
        std::lock_guard<std::mutex> lock(mtx); //Protege el acceso concurrente a la escena

        auto & entity = scene.create_entity ();

        scene.create_component< Transform > (entity);
        scene.create_component< Path_Tracing::Camera > (entity, Path_Tracing::Camera::Sensor_Type::APS_C, 16.f / 1000.f);

        std::shared_ptr< Controller > camera_controller = std::make_shared< Camera_Controller > (scene, entity.id);

        scene.create_component< Control::Component > (entity, camera_controller);
    }

    void load_ground (Scene & scene, std::mutex& mtx)
    {
        std::lock_guard<std::mutex> lock(mtx); //Protege el acceso concurrente a la escena
        auto & entity = scene.create_entity ();

        scene.create_component< Transform > (entity);

        auto model_component = scene.create_component< Path_Tracing::Model > (entity);

        model_component->add_plane (Vector3{0, .25f, 0}, Vector3{0, -1, 0}, model_component->add_diffuse_material (Path_Tracing::Color(.4f, .4f, .5f)));
    }

    void load_shape (Scene & scene, std::mutex& mtx)
    {
        std::lock_guard<std::mutex> lock(mtx); //Evita condiciones de carrera

        auto & entity = scene.create_entity ();

        scene.create_component< Transform > (entity);

        auto model_component = scene.create_component< Path_Tracing::Model > (entity);

        model_component->add_sphere (Vector3{.0f, 0.f, -1.0f}, .25f, model_component->add_diffuse_material  (Path_Tracing::Color(.8f, .8f, .8f)));
        model_component->add_sphere (Vector3{.5f, 0.f, -1.1f}, .15f, model_component->add_metallic_material (Path_Tracing::Color(.4f, .5f, .6f), 0.1f));
    }

    //Funcion principal de carga. Ejecuta 3 tareas concurrentemente en el thread pool
    void load (Scene & scene)
    {
        //Codigo previo:
        //load_camera (scene);
        //load_ground (scene);
        //load_shape  (scene);

        //Instancia del thread pool
        ThreadPool pool;

        //Se comparte el mutex para sincronizar el acceso a la escena
        std::mutex scene_mutex;

        //Envio de tareas al thread pool para ejecutarlas en paralelo
        auto future_camera = pool.submit(load_camera, std::ref(scene), std::ref(scene_mutex));
        auto future_ground = pool.submit(load_ground, std::ref(scene), std::ref(scene_mutex));
        auto future_shape = pool.submit(load_shape, std::ref(scene), std::ref(scene_mutex));

        //Se espera a que cada una de las tareas termine antes de continuar
        future_camera.get();
        future_ground.get();
        future_shape.get();

    }

    void engine_application ()
    {
        Window window("Ray Tracing Engine", 1024, 600);

        Scene scene(window);

        load (scene);

        scene.run ();
    }

}

int main (int , char * [])
{
    engine::starter.run (engine_application);

    return 0;
}
