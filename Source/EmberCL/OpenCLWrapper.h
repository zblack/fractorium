#pragma once

#include "EmberCLPch.h"

/// <summary>
/// OpenCLWrapper, Spk, NamedBuffer, NamedImage2D, NamedImage2DGL classes.
/// </summary>

namespace EmberCLns
{
#if CL_VERSION_1_2
#define IMAGEGL2D cl::ImageGL
#else
#define IMAGEGL2D cl::Image2DGL
#endif

/// <summary>
/// Class to contain all of the things needed to store an OpenCL program.
/// The name of it, the source, the compiled program object and the kernel.
/// </summary>
class EMBERCL_API Spk
{
public:
	string m_Name;
	cl::Program::Sources m_Source;
	cl::Program m_Program;
	cl::Kernel m_Kernel;
};

/// <summary>
/// Class to hold an OpenCL buffer with a name to identify it by.
/// </summary>
class EMBERCL_API NamedBuffer
{
public:
	NamedBuffer()
	{
	}

	NamedBuffer(cl::Buffer& buff, string name)
	{
		m_Buffer = buff;
		m_Name = name;
	}

	cl::Buffer m_Buffer;
	string m_Name;
};

/// <summary>
/// Class to hold a 2D image with a name to identify it by.
/// </summary>
class EMBERCL_API NamedImage2D
{
public:
	NamedImage2D()
	{
	}

	NamedImage2D(cl::Image2D& image, string name)
	{
		m_Image = image;
		m_Name = name;
	}

	cl::Image2D m_Image;
	string m_Name;
};

/// <summary>
/// Class to hold a 2D image that is mapped to an OpenGL texture
/// and a name to identify it by.
/// </summary>
class EMBERCL_API NamedImage2DGL
{
public:
	NamedImage2DGL()
	{
	}

	NamedImage2DGL(IMAGEGL2D& image, string name)
	{
		m_Image = image;
		m_Name = name;
	}

	IMAGEGL2D m_Image;
	string m_Name;
};

/// <summary>
/// Running kernels in OpenCL can require quite a bit of setup, tear down and
/// general housekeeping. This class helps shield the user from such hassles.
/// It's main utility is in holding collections of programs, buffers and images
/// all identified by names. That way, a user can access them as needed without
/// having to pollute their code.
/// In addition, writing to an existing object by name determines if the object
/// can be overwritten, or if it needs to be deleted and replaced by the new one.
/// This class derives from EmberReport, so the caller is able
/// to retrieve a text dump of error information if any errors occur.
/// </summary>
class EMBERCL_API OpenCLWrapper : public EmberReport
{
public:
	OpenCLWrapper();
	bool CheckOpenCL();
	bool Init(unsigned int platform, unsigned int device, bool shared = false);

	//Programs.
	bool AddProgram(std::string name, std::string& program, std::string& entryPoint, bool doublePrecision);
	void ClearPrograms();

	//Buffers.
	bool AddBuffer(string name, size_t size, cl_mem_flags flags = CL_MEM_READ_WRITE);
	bool AddAndWriteBuffer(string name, void* data, size_t size);
	bool WriteBuffer(string name, void* data, size_t size);
	bool WriteBuffer(unsigned int bufferIndex, void* data, size_t size);
	bool ReadBuffer(string name, void* data, size_t size);
	bool ReadBuffer(unsigned int bufferIndex, void* data, size_t size);
	int FindBufferIndex(string name);
	unsigned int GetBufferSize(string name);
	unsigned int GetBufferSize(unsigned int bufferIndex);
	void ClearBuffers();

	//Images.
	bool AddAndWriteImage(string name, cl_mem_flags flags, const cl::ImageFormat& format, ::size_t width, ::size_t height, ::size_t row_pitch, void* data = NULL, bool shared = false, GLuint texName = 0);
	bool WriteImage2D(unsigned int index, bool shared, ::size_t width, ::size_t height, ::size_t row_pitch, void* data);
	bool ReadImage(string name, ::size_t width, ::size_t height, ::size_t row_pitch, bool shared, void* data);
	bool ReadImage(unsigned int imageIndex, ::size_t width, ::size_t height, ::size_t row_pitch, bool shared, void* data);
	int FindImageIndex(string name, bool shared);
	unsigned int GetImageSize(string name, bool shared);
	unsigned int GetImageSize(unsigned int imageIndex, bool shared);
	bool CompareImageParams(cl::Image& image, cl_mem_flags flags, const cl::ImageFormat& format, ::size_t width, ::size_t height, ::size_t row_pitch);
	void ClearImages(bool shared);
	bool CreateImage2D(cl::Image2D& image2D, cl_mem_flags flags, cl::ImageFormat format, ::size_t width, ::size_t height, ::size_t row_pitch = 0, void* data = NULL);
	bool CreateImage2DGL(IMAGEGL2D& image2DGL, cl_mem_flags flags, GLenum target, GLint miplevel, GLuint texobj);
	bool EnqueueAcquireGLObjects(string name);
	bool EnqueueAcquireGLObjects(IMAGEGL2D& image);
	bool EnqueueReleaseGLObjects(string name);
	bool EnqueueReleaseGLObjects(IMAGEGL2D& image);
	bool EnqueueAcquireGLObjects(const VECTOR_CLASS<cl::Memory>* memObjects = NULL);
	bool EnqueueReleaseGLObjects(const VECTOR_CLASS<cl::Memory>* memObjects = NULL);
	bool CreateSampler(cl::Sampler& sampler, cl_bool normalizedCoords, cl_addressing_mode addressingMode, cl_filter_mode filterMode);
	
	//Arguments.
	bool SetBufferArg(unsigned int kernelIndex, unsigned int argIndex, string name);
	bool SetBufferArg(unsigned int kernelIndex, unsigned int argIndex, unsigned int bufferIndex);
	bool SetImageArg(unsigned int kernelIndex, unsigned int argIndex, bool shared, string name);
	bool SetImageArg(unsigned int kernelIndex, unsigned int argIndex, bool shared, unsigned int imageIndex);
	
	/// <summary>
	/// Set an argument in the specified kernel, at the specified argument index.
	/// Must keep this here in the .h because it's templated.
	/// </summary>
	/// <param name="kernelIndex">Index of the kernel whose argument will be set</param>
	/// <param name="argIndex">Index of the argument to set</param>
	/// <param name="arg">The argument value to set</param>
	/// <returns>True if success, else false</returns>
	template <typename T>
	bool SetArg(unsigned int kernelIndex, unsigned int argIndex, T arg)
	{
		if (m_Init && kernelIndex < m_Programs.size())
		{
			cl_int err = m_Programs[kernelIndex].m_Kernel.setArg(argIndex, arg);

			return CheckCL(err, "cl::Kernel::setArg()");
		}

		return false;
	}

	//Kernels.
	int FindKernelIndex(string name);
	bool RunKernel(unsigned int kernelIndex, unsigned int totalGridWidth, unsigned int totalGridHeight, unsigned int totalGridDepth, unsigned int blockWidth, unsigned int blockHeight, unsigned int blockDepth);

	//Info.
	template<typename T>
	T GetInfo(size_t platform, size_t device, cl_device_info name);
	string PlatformName(size_t platform);
	vector<string> PlatformNames();
	string DeviceName(size_t platform, size_t device);
	vector<string> DeviceNames(size_t platform);
	string DeviceAndPlatformNames();
	string DumpInfo();

	//Accessors.
	bool Ok();
	bool Shared();
	cl::Context Context();
	unsigned int PlatformIndex();
	unsigned int DeviceIndex();
	unsigned int LocalMemSize();

	static void MakeEvenGridDims(unsigned int blockW, unsigned int blockH, unsigned int& gridW, unsigned int& gridH);

private:
	bool CreateContext(bool shared);
	bool CreateSPK(std::string& name, std::string& program, std::string& entryPoint, Spk& spk, bool doublePrecision);
	bool CheckCL(cl_int err, const char* name);
	std::string ErrorToStringCL(cl_int err);

	bool m_Init;
	bool m_Shared;
	unsigned int m_PlatformIndex;
	unsigned int m_DeviceIndex;
	unsigned int m_LocalMemSize;
	cl::Platform m_Platform;
	cl::Context m_Context;
	cl::Device m_Device;
	cl::CommandQueue m_Queue;
	std::vector<cl::Platform> m_Platforms;
	std::vector<std::vector<cl::Device>> m_Devices;
	std::vector<cl::Device> m_DeviceVec;
	std::vector<Spk> m_Programs;
	std::vector<NamedBuffer> m_Buffers;
	std::vector<NamedImage2D> m_Images;
	std::vector<NamedImage2DGL> m_GLImages;
};
}