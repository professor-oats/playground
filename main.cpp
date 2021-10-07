#include "rtweekend.h"

#include "color.h"
#include "hittable_list.h"
#include "sphere.h"
#include "camera.h"
#include "material.h"
#include "moving_sphere.h"
#include "aarect.h"
#include "box.h"
#include "constant_medium.h"
#include "bvh.h"

#include <iostream>

color ray_color(const ray& r, const color& background, const hittable& world, int depth) {
    hit_record rec;

    // If we've exceeded the ray bounce limit, no more light is gathered.
    if (depth <= 0) {
	return color(0,0,0);
    }

    // If the ray hits nothing, return the background color.
    if (!world.hit(r, 0.001, infinity, rec)) {
	return background;
    }

    ray scattered;
    color attenuation;
    color emitted = rec.mat_ptr->emitted(rec.u, rec.v, rec.p);

    if (!rec.mat_ptr->scatter(r, rec, attenuation, scattered)){
	return emitted;
    }

    return emitted + attenuation * ray_color(scattered, background, world, depth-1);
} 

hittable_list flaming_sphere() {
     hittable_list objects;

     auto sphere_material = make_shared<lambertian>((color(0.5, 0.3, 0.7)));
     objects.add(make_shared<sphere>(point3(4,2,4), 2, sphere_material));
     
     auto boundary_ground = make_shared<sphere>(point3(0,-1000,0), 1000, make_shared<dielectric>(1.5));
     objects.add(boundary_ground);
     objects.add(make_shared<constant_medium>(boundary_ground, 0.002, color(0.2, 0.4, 0.9)));

     //light 1

     auto light1 = make_shared<diffuse_light>(color(7,7,7));
     auto round_light1 = make_shared<sphere>(point3(-600, 800, 0), 500, light1);
     objects.add(round_light1);


     hittable_list sparkles;	// Making hittable_list which then will be put with shared bvh_node pointer. Must make a type of grid to spawn them in.

     //Simple declarations
     double offset;
     double pos;
     bool front;

     int amount_sparkles = 50;  //Size 0.05 deviation 0.01?
     for (int i = 0; i < amount_sparkles; i++) {
	//offset = random_double(0,8)   	// K start point is determined of object radius that sparks surrounds


	int cs_x = random_int(0, 7);    // Color switch x, y, z
	int cs_y = random_int(0, 7);
	int cs_z = random_int(0, 7);

	pos = random_double(0, 8);    // Position and offset are decided from object radius that sparks surrounds | Random from negative works quite badly. So I need a fix.

	// std::cerr << offset << '\n';

	double ypos = random_double(0, 5);   
	double spark_dim1 = pos + 0.10;
	double spark_dim2 = ypos + 0.24;

	if (front > 0) {
	    offset = 6.1 + random_double();
	    front = 0;
	}
	else {
	    offset = 1 + random_double();
	    front = 1;
	}


	objects.add(make_shared<xy_rect>(pos, spark_dim1, ypos, spark_dim2, offset, make_shared<diffuse_light>(color(cs_x, cs_y, cs_z)))); 
     }
	
     return objects;
}


int main() {


    // World  - Setting initialised values in world and image for some of the variables. Before this they were consts but now they must be changed for each setting in the switches

    hittable_list world;
    point3 lookfrom;
    point3 lookat;
    auto vfov = 20.0; 	    //vfov acting as a zoom specified in degrees. It changes the height of the viewport and the width of the viewport depends on height.

    auto aperture = 0.0;    // Depth of field or defocus blur
    color background(0,0,0);

    // Image

    auto aspect_ratio = 16.0/9.0;    // 16.0 / 9.0;
    int image_width = 600;
    int samples_per_pixel = 20;
    int max_depth = 30;



    switch (0) {
	default:
	case 1:
	     world = flaming_sphere();	// Det här får fan duga
	     aspect_ratio = 16.0/12.0;
	     background = color(0,0,0);
	     lookfrom = point3(8,4,25);	// 28,4,27 seems cool
	     lookat = point3(4, 2, 4);  // 0, 2, 0
	     vfov = 20.0;
	     break;
    }

    // lookfrom 8,4,20 and lookat 4,2,4 will most likely be one prospect for flame picture. It's great zoom.	

    // Aspected height
    int image_height = static_cast<int>(image_width / aspect_ratio);

    // Camera

    vec3 vup(0,1,0);
    auto dist_to_focus = 10.0;

    camera cam(lookfrom, lookat, vup, vfov, aspect_ratio, aperture, dist_to_focus, 0.0, 1.0);


    // Render

    std::cout << "P3\n" << image_width << ' ' << image_height << "\n255\n";

    for (int j = image_height - 1; j >= 0; --j) {
	std::cerr << "\rScanlines remaining: " << j << ' ' << std::flush;
	for (int i = 0; i < image_width; ++i) {
	    color pixel_color(0,0,0);
	    for (int s = 0; s < samples_per_pixel; ++s) {
	        auto u = (i + random_double()) / (image_width-1);
	        auto v = (j + random_double()) / (image_height-1);
	        ray r(cam.get_ray(u, v));
	        pixel_color += ray_color(r, background, world, max_depth);
	    }
	write_color(std::cout, pixel_color, samples_per_pixel);
        }
    }
    std::cerr << "\nDone.\n";
}

