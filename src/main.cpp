
#include <iostream>
#include <SFML/Graphics.hpp>
#include <random>
#include <chrono>
#include "util.hpp"
#include <thread>
#include "OpenCL.h"

float elap_time() {
	static std::chrono::time_point<std::chrono::system_clock> start;
	static bool started = false;

	if (!started) {
		start = std::chrono::system_clock::now();
		started = true;
	}

	std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();
	std::chrono::duration<double> elapsed_time = now - start;
	return static_cast<float>(elapsed_time.count());
}

const int WINDOW_X = 1920;
const int WINDOW_Y = 1080;

enum Mouse_State {PRESSED, DEPRESSED};

int main() {

	sf::RenderWindow window(sf::VideoMode(WINDOW_X, WINDOW_Y), "quick-sfml-template");
	window.setFramerateLimit(60);

	float physic_step = 0.166f;
	float physic_time = 0.0f;
	double frame_time = 0.0, elapsed_time = 0.0, delta_time = 0.0, accumulator_time = 0.0, current_time = 0.0;
	
	OpenCL cl;

	sf::Vector4f range(-1.0f, 1.0f, -1.0f, 1.0f);
	sf::Vector2i image_resolution(WINDOW_X, WINDOW_Y);

	if (!cl.init())
		return -1;
	
	while (!cl.compile_kernel("../kernels/mandlebrot.cl", "mandlebrot")) {
		std::cin.get();
	}

	cl.create_image_buffer("viewport_image", image_resolution, sf::Vector2f(0, 0), CL_MEM_WRITE_ONLY);
	cl.create_buffer("image_res", sizeof(sf::Vector2i), &image_resolution);
	cl.create_buffer("range", sizeof(sf::Vector4f), (void*)&range, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR);
	
	cl.set_kernel_arg("mandlebrot", 0, "image_res");
	cl.set_kernel_arg("mandlebrot", 1, "viewport_image");
	cl.set_kernel_arg("mandlebrot", 2, "range");

	while (window.isOpen())
	{
		sf::Event event; // Handle input
		while (window.pollEvent(event)) {
			if (event.type == sf::Event::Closed) {
				window.close();
			}
			if (event.type == sf::Event::KeyPressed) {
				if (event.key.code == sf::Keyboard::Down) {
					range.z += 0.001f;
					range.w += 0.001f;
				}
				if (event.key.code == sf::Keyboard::Up) {
					range.z -= 0.001f;
					range.w -= 0.001f;
				}
				if (event.key.code == sf::Keyboard::Right) {
					range.x += 0.001f;
					range.y += 0.001f;
				}
				if (event.key.code == sf::Keyboard::Left) {
					range.x -= 0.001f;
					range.y -= 0.001f;
				}
				if (event.key.code == sf::Keyboard::Equal) {
					range.x *= 1.02f;
					range.y *= 1.02f;
					range.z *= 1.02f;
					range.w *= 1.02f;
				}
				if (event.key.code == sf::Keyboard::Dash) {
					range.x *= 0.98f;
					range.y *= 0.98f;
					range.z *= 0.98f;
					range.w *= 0.98f;
				}
			}
		}

		elapsed_time = elap_time(); // Handle time
		delta_time = elapsed_time - current_time;
		current_time = elapsed_time;
		if (delta_time > 0.02f)
			delta_time = 0.02f;
		accumulator_time += delta_time;

		while (accumulator_time >= physic_step) { // While the frame has sim time, update 
			accumulator_time -= physic_step;
			physic_time += physic_step;
		}

		window.clear(sf::Color::White);
	
		cl.run_kernel("mandlebrot", image_resolution);
		cl.draw(&window);

		window.display();

	}
	return 0;

}
