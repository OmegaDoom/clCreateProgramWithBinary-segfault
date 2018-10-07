#define CL_HPP_TARGET_OPENCL_VERSION 200
#define CL_HPP_ENABLE_EXCEPTIONS
#define CL_TARGET_OPENCL_VERSION 220

#include <iostream>
#include <iterator>
#include <memory>
#include <vector>
#include <fstream>
#include <CL/cl2.hpp>

std::unique_ptr<cl::Device> get_device(cl_device_type device_type, int major_version, int minor_version)
{
	std::vector<cl::Platform> platforms;
	cl::Platform::get(&platforms);
	
	for(auto& platform : platforms)
	{
		std::vector<cl::Device> devices;
		platform.getDevices(device_type, &devices);
		
		for (auto& device : devices)
		{
			std::string version;
			device.getInfo(CL_DEVICE_VERSION, &version);
			size_t major_ver_pos = version.find(' ') + 1;
			size_t minor_ver_pos = version.find('.', major_ver_pos) + 1;
			size_t ver_end_pos = version.find(' ', minor_ver_pos);
			int major_ver = std::stoi(std::string(version.cbegin() + major_ver_pos, version.cbegin() + minor_ver_pos - 1));
			int minor_ver = std::stoi(std::string(version.cbegin() + minor_ver_pos, version.cbegin() + ver_end_pos));
			if ((major_ver > major_version) || (major_ver == major_version && minor_ver >= minor_version))
				return std::unique_ptr<cl::Device>(new cl::Device(device));
		}
	}
	return nullptr;
}

std::string get_source(const char* file_name)
{
	std::ifstream ifs(file_name);
	std::string source((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
	return std::move(source);
}

int work()
{
	auto device = get_device(CL_DEVICE_TYPE_CPU, 2, 0);
	if (!device)
	{
		std::cout << "device failed" << std::endl;
		return 1;
	}

	cl::Context context(*device);
	cl::CommandQueue queue = cl::CommandQueue(context, *device);

    cl_command_queue_properties properties = CL_QUEUE_ON_DEVICE | CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE;
	cl::CommandQueue device_queue = cl::CommandQueue(context, *device, properties);

	size_t array_size = 1000;
	size_t data_size = array_size * sizeof(int);

	int *input = new int[array_size];
	int *output = new int[array_size];

	for (int i = 0; i < array_size; i++)
	{
        input[i] = i;
	}

	cl::Buffer input_buffer = cl::Buffer(context, CL_MEM_READ_ONLY, data_size);
	cl::Buffer output_buffer = cl::Buffer(context, CL_MEM_WRITE_ONLY, data_size);

	queue.enqueueWriteBuffer(input_buffer, CL_TRUE, 0, data_size, input);

	std::string source = get_source("kernel.cl");
	if (source.empty())
	{
		std::cout << "source is empty" << std::endl;
		return 1;
	}
	std::vector<std::string> sources(1, source);
	std::vector<cl::Device> devices(1, *device);
    cl::Program::Binaries binaries;
    const char* build_options = "-cl-std=CL2.0";

	try
	{
        auto program = cl::Program(context, sources);
		program.build(devices, build_options);
        program.getInfo(CL_PROGRAM_BINARIES, &binaries);
	}
	catch(const cl::Error& error)
	{
		//cl_build_status status = program.getBuildInfo<CL_PROGRAM_BUILD_STATUS>(*device);
		//// Get the build log
		//std::string name = device->getInfo<CL_DEVICE_NAME>();
		//std::string build_log = program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(*device);
		//std::cout << build_log << std::endl;
		//throw;
	}

	cl::Program program = cl::Program(context, devices, binaries);
    program.build(devices, build_options);

	cl::Kernel kernel(program, "krnl");

	cl::NDRange global(array_size);

	kernel.setArg(0, device_queue);
	kernel.setArg(1, input_buffer);
	kernel.setArg(2, output_buffer);
	queue.enqueueNDRangeKernel(kernel, cl::NullRange, global, cl::NullRange);
	queue.enqueueReadBuffer(output_buffer, CL_TRUE, 0, data_size, output);
    std::copy(output, &output[array_size], std::ostream_iterator<int>(std::cout, " "));
	return 0;
}

int main (int argc, char* argv[])
{
	try
	{
		return work();
	}
	catch(const cl::Error& error)
	{
		std::cout << error.what() << "(" << error.err() << ")" << std::endl;
	}
}
