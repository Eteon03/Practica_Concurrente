/*
 * Copyright © 2025+ ÁRgB (angel.rodriguez@udit.es)
 *
 * Distributed under the Boost Software License, version 1.0
 * See ./LICENSE or www.boost.org/LICENSE_1_0.txt
 */

#include <raytracer/Pinhole_Camera.hpp>

#include <iostream>
#include <execution>
using namespace std;

namespace udit::raytracer
{

    void Pinhole_Camera::calculate (Buffer< Ray > & primary_rays)
    {
        auto buffer_width  = primary_rays.get_width  ();
        auto buffer_height = primary_rays.get_height ();

        Vector2 half_sensor_resolution
        (
            0.5f * static_cast< float >(buffer_width ),
            0.5f * static_cast< float >(buffer_height)
        );

        Vector2 half_sensor_size
        (
            0.5f * get_sensor_width (),
            0.5f * get_sensor_width () * half_sensor_resolution.y / half_sensor_resolution.x
        );

        auto  & transform_matrix   = transform.get_matrix   ();
        Vector3 sensor_center      = transform.get_position ();
        Vector3 focal_point        = transform_matrix * Vector4(0, 0, -focal_length, 1);
        Vector3 right_direction    = transform_matrix * Vector4(half_sensor_size.x, 0, 0, 0);
        Vector3 up_direction       = transform_matrix * Vector4(0, half_sensor_size.y, 0, 0);
        //La imagen se creaba invertida, asi que he tenido que invertir los + y - de las direcciones
        Vector3 sensor_bottom_left = sensor_center + (right_direction - up_direction);

        Vector3 horizontal_step    = right_direction / half_sensor_resolution.x;
        //Tambien aqui
        horizontal_step = -horizontal_step;

        Vector3 vertical_step      = up_direction    / half_sensor_resolution.y;

        size_t total_pixels = buffer_width * buffer_height;
        std::vector<size_t> indices(total_pixels);
        std::iota(indices.begin(), indices.end(), 0);

        std::for_each(std::execution::par, indices.begin(), indices.end(), [&](size_t index)
            {
                int x = static_cast<int>(index % buffer_width);
                int y = static_cast<int>(index / buffer_width);

                //Y tambien aqui
                y = buffer_height - 1 - y;

                Vector3 pixel_position = sensor_bottom_left + horizontal_step * static_cast<float>(x) + vertical_step * static_cast<float>(y);
                primary_rays[index] = Ray{ pixel_position,focal_point - pixel_position };
            });
    }

}
