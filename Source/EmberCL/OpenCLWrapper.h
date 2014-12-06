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

	NamedBuffer(const cl::Buffer& buff, const string& name)
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

	NamedImage2D(const cl::Image2D& image, const string& name)
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

	NamedImage2DGL(const IMAGEGL2D& image, const string& name)
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
	bool Init(uint platform, uint device, bool shared = false);

	//Programs.
	bool AddProgram(const string& name, const string& program, const string& entryPoint, bool doublePrecision);
	void ClearPrograms();

	//Buffers.
	bool AddBuffer(const string& name, size_t size, cl_mem_flags flags = CL_MEM_READ_WRITE);
	bool AddAndWriteBuffer(const string& name, void* data, size_t size, cl_mem_flags flags = CL_MEM_READ_WRITE);
	bool WriteBuffer(const string& name, void* data, size_t size);
	bool WriteBuffer(uint bufferIndex, void* data, size_t size);
	bool ReadBuffer(const string& name, void* data, size_t size);
	bool ReadBuffer(uint bufferIndex, void* data, size_t size);
	int FindBufferIndex(const string& name);
	uint GetBufferSize(const string& name);
	uint GetBufferSize(uint bufferIndex);
	void ClearBuffers();

	//Images.
	bool AddAndWriteImage(const string& name, cl_mem_flags flags, const cl::ImageFormat& format, ::size_t width, ::size_t height, ::size_t row_pitch, void* data = NULL, bool shared = false, GLuint texName = 0);
	bool WriteImage2D(uint index, bool shared, ::size_t width, ::size_t height, ::size_t row_pitch, void* data);
	bool ReadImage(const string& name, ::size_t width, ::size_t height, ::size_t row_pitch, bool shared, void* data);
	bool ReadImage(uint imageIndex, ::size_t width, ::size_t height, ::size_t row_pitch, bool shared, void* data);
	int FindImageIndex(const string& name, bool shared);
	uint GetImageSize(const string& name, bool shared);
	uint GetImageSize(uint imageIndex, bool shared);
	bool CompareImageParams(cl::Image& image, cl_mem_flags flags, const cl::ImageFormat& format, ::size_t width, ::size_t height, ::size_t row_pitch);
	void ClearImages(bool shared);
	bool CreateImage2D(cl::Image2D& image2D, cl_mem_flags flags, cl::ImageFormat format, ::size_t width, ::size_t height, ::size_t row_pitch = 0, void* data = NULL);
	bool CreateImage2DGL(IMAGEGL2D& image2DGL, cl_mem_flags flags, GLenum target, GLint miplevel, GLuint texobj);
	bool EnqueueAcquireGLObjects(const string& name);
	bool EnqueueAcquireGLObjects(IMAGEGL2D& image);
	bool EnqueueReleaseGLObjects(const string& name);
	bool EnqueueReleaseGLObjects(IMAGEGL2D& image);
	bool EnqueueAcquireGLObjects(const VECTOR_CLASS<cl::Memory>* memObjects = NULL);
	bool EnqueueReleaseGLObjects(const VECTOR_CLASS<cl::Memory>* memObjects = NULL);
	bool CreateSampler(cl::Sampler& sampler, cl_bool normalizedCoords, cl_addressing_mode addressingMode, cl_filter_mode filterMode);

	//Arguments.
	bool SetBufferArg(uint kernelIndex, uint argIndex, const string& name);
	bool SetBufferArg(uint kernelIndex, uint argIndex, uint bufferIndex);
	bool SetImageArg(uint kernelIndex, uint argIndex, bool shared, const string& name);
	bool SetImageArg(uint kernelIndex, uint argIndex, bool shared, uint imageIndex);

	/// <summary>
	/// Set an argument in the specified kernel, at the specified argument index.
	/// Must keep this here in the .h because it's templated.
	/// </summary>
	/// <param name="kernelIndex">Index of the kernel whose argument will be set</param>
	/// <param name="argIndex">Index of the argument to set</param>
	/// <param name="arg">The argument value to set</param>
	/// <returns>True if success, else false</returns>
	template <typename T>
	bool SetArg(uint kernelIndex, uint argIndex, T arg)
	{
		if (m_Init && kernelIndex < m_Programs.size())
		{
			cl_int err = m_Programs[kernelIndex].m_Kernel.setArg(argIndex, arg);

			return CheckCL(err, "cl::Kernel::setArg()");
		}

		return false;
	}

	//Kernels.
	int FindKernelIndex(const string& name);
	bool RunKernel(uint kernelIndex, uint totalGridWidth, uint totalGridHeight, uint totalGridDepth, uint blockWidth, uint blockHeight, uint blockDepth);

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
	bool Ok() const;
	bool Shared() const;
	cl::Context Context() const;
	uint PlatformIndex() const;
	uint DeviceIndex() const;
	uint LocalMemSize() const;

	static void MakeEvenGridDims(uint blockW, uint blockH, uint& gridW, uint& gridH);

private:
	bool CreateContext(bool shared);
	bool CreateSPK(const string& name, const string& program, const string& entryPoint, Spk& spk, bool doublePrecision);
	bool CheckCL(cl_int err, const char* name);
	std::string ErrorToStringCL(cl_int err);

	bool m_Init;
	bool m_Shared;
	uint m_PlatformIndex;
	uint m_DeviceIndex;
	uint m_LocalMemSize;
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
