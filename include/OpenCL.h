#pragma once

#ifdef linux
#include <CL/cl.h>
#include <CL/opencl.h>

#elif defined _WIN32
#define CL_USE_DEPRECATED_OPENCL_1_2_APIS
#include <CL/cl_gl.h>
#include <CL/cl.h>
#include <CL/opencl.h>

// Note: windows.h must be included before Gl/GL.h
#include <windows.h>
#include <GL/GL.h>

#elif defined TARGET_OS_MAC
#include <OpenCL/opencl.h>

#endif

#include <string>
#include <SFML/Graphics.hpp>
#include <unordered_map>
#include <iostream>

struct device {
	
	cl_device_id id;
	cl_device_type type;
	cl_uint clock_frequency;
	char version[128];
	cl_platform_id platform;
	cl_uint comp_units;
	char extensions[1024];
	char name[256];
	cl_bool is_little_endian = false;
	bool cl_gl_sharing = false;

};


class OpenCL {
	
public:

	OpenCL(sf::Vector2i resolution);
	~OpenCL();

	bool init();

	void run_kernel(std::string kernel_name);

	void draw(sf::RenderWindow *window);

private:

	int error = 0;

	// Sprite and texture that is shared between CL and GL
	sf::Sprite viewport_sprite;
	sf::Texture viewport_texture;
	sf::Vector2i viewport_resolution;

	// The device which we have selected according to certain criteria
	cl_platform_id platform_id;
	cl_device_id device_id;

	// The GL shared context and its subsiquently generated command queue
	cl_context context;
	cl_command_queue command_queue;

	// Maps which contain a mapping from "name" to the host side CL memory object
	std::unordered_map<std::string, cl_kernel> kernel_map;
	std::unordered_map<std::string, cl_mem> buffer_map;

	// Query the hardware on this machine and select the best device and the platform on which it resides
	void aquire_hardware();

	// After aquiring hardware, create a shared context using platform specific CL commands
	void create_shared_context();

	// Command queues must be created with a valid context
	void create_command_queue();
	
	// Compile the kernel and store it in the kernel map with the name as the key
	bool compile_kernel(std::string kernel_path, std::string kernel_name);

	// Buffer operations
	// All of these functions create and store a buffer in a map with the key representing their name

	// Create an image buffer from an SF texture. Access Type is the read/write specifier required by OpenCL
	int create_image_buffer(std::string buffer_name, cl_uint size, sf::Texture* texture, cl_int access_type);

	// Create a buffer with CL_MEM_READ_ONLY and CL_MEM_COPY_HOST_PTR
	int create_buffer(std::string buffer_name, cl_uint size, void* data);

	// Create a buffer with user defined data access flags
	int create_buffer(std::string buffer_name, cl_uint size, void* data, cl_mem_flags flags);

	// Store a cl_mem object in the buffer map <string:name, cl_mem:buffer>
	int store_buffer(cl_mem buffer, std::string buffer_name);

	// Using CL release the memory object and remove the KVP associated with the buffer name
	int release_buffer(std::string buffer_name);

	void assign_kernel_args();
	int set_kernel_arg(std::string kernel_name, int index, std::string buffer_name);

	bool vr_assert(int error_code, std::string function_name);

};