#include "EmberCLPch.h"
#include "OpenCLWrapper.h"

namespace EmberCLns
{
/// <summary>
/// Constructor that sets everything to an uninitialized state.
/// No OpenCL setup is done here, the caller must explicitly do it.
/// </summary>
OpenCLWrapper::OpenCLWrapper()
{
	m_Init = false;
	m_Shared = false;
	m_PlatformIndex = 0;
	m_DeviceIndex = 0;
	m_LocalMemSize = 0;
	cl::Platform::get(&m_Platforms);
	m_Devices.resize(m_Platforms.size());

	for (size_t i = 0; i < m_Platforms.size(); i++)
		m_Platforms[i].getDevices(CL_DEVICE_TYPE_ALL, &m_Devices[i]);
}

/// <summary>
/// Determine if OpenCL is available on the system.
/// </summary>
/// <returns>True if any OpenCL platform and at least 1 device within that platform exists on the system, else false.</returns>
bool OpenCLWrapper::CheckOpenCL()
{
	for (size_t i = 0; i < m_Platforms.size(); i++)
		for (size_t j = 0; j < m_Devices[i].size(); j++)
			return true;

	return false;
}

/// <summary>
/// Initialize the specified platform and device.
/// This can be shared with OpenGL.
/// </summary>
/// <param name="platform">The index platform of the platform to use</param>
/// <param name="device">The index device of the device to use</param>
/// <param name="shared">True if shared with OpenGL, else false.</param>
/// <returns>True if success, else false.</returns>
bool OpenCLWrapper::Init(unsigned int platform, unsigned int device, bool shared)
{
	cl_int err;

	m_Init = false;
	m_ErrorReport.clear();
	
	if (m_Platforms.size() > 0)
	{
		if (platform < m_Platforms.size() && platform < m_Devices.size())
		{
			m_PlatformIndex = platform;//Platform is ok, now do context.
		
			if (CreateContext(shared))
			{
				//Context is ok, now do device.
				if (device < m_Devices[m_PlatformIndex].size())
				{
					//At least one GPU device is present, so create a command queue.
					m_Queue = cl::CommandQueue(m_Context, m_Devices[m_PlatformIndex][device], 0, &err);

					if (CheckCL(err, "cl::CommandQueue()"))
					{
						m_DeviceIndex = device;
						m_Platform = m_Platforms[m_PlatformIndex];
						m_Device = m_Devices[m_PlatformIndex][device];
						m_DeviceVec.clear();
						m_DeviceVec.push_back(m_Device);
						m_LocalMemSize = (unsigned int)GetInfo<cl_ulong>(m_PlatformIndex, m_DeviceIndex, CL_DEVICE_LOCAL_MEM_SIZE);
						m_Shared = shared;
						m_Init = true;//Command queue is ok, it's now ok to begin building and running programs.
					}
				}
			}
		}
	}

	return m_Init;
}

/// <summary>
/// Compile and add the program, using the specified entry point.
/// If a program with the same name already exists then it will be replaced.
/// </summary>
/// <param name="name">The name of the program</param>
/// <param name="program">The program source</param>
/// <param name="entryPoint">The name of the entry point kernel function in the program</param>
/// <returns>True if success, else false.</returns>
bool OpenCLWrapper::AddProgram(std::string name, std::string& program, std::string& entryPoint, bool doublePrecision)
{
	Spk spk;
		
	if (CreateSPK(name, program, entryPoint, spk, doublePrecision))
	{
		for (size_t i = 0; i < m_Programs.size(); i++)
		{
			if (name == m_Programs[i].m_Name)
			{
				m_Programs[i] = spk;
				return true;
			}
		}

		//Nothing was found, so add.
		m_Programs.push_back(spk);
		return true;
	}
	
	return false;
}

/// <summary>
/// Clear the programs.
/// </summary>
void OpenCLWrapper::ClearPrograms()
{
	m_Programs.clear();
}

/// <summary>
/// Add a buffer with the specified size and name.
/// Three possible actions to take:
///		Buffer didn't exist, so create and add.
///		Buffer existed, but was a different size. Replace.
///		Buffer existed with the same size, do nothing.
/// </summary>
/// <param name="name">The name of the buffer</param>
/// <param name="size">The size in bytes of the buffer</param>
/// <param name="flags">The buffer flags. Default: CL_MEM_READ_WRITE.</param>
/// <returns>True if success, else false.</returns>
bool OpenCLWrapper::AddBuffer(string name, size_t size, cl_mem_flags flags)
{
	cl_int err;
	
	if (m_Init)
	{
		int bufferIndex = FindBufferIndex(name);

		if (bufferIndex == -1)//If the buffer didn't exist, create and add.
		{
			cl::Buffer buff(m_Context, flags, size, NULL, &err);

			if (!CheckCL(err, "cl::Buffer()"))
				return false;

			NamedBuffer nb(buff, name);

			m_Buffers.push_back(nb);
		}
		else if (GetBufferSize(bufferIndex) != size)//If it did exist, only create and add if the sizes were different.
		{
			m_Buffers[bufferIndex] = NamedBuffer(cl::Buffer(m_Context, flags, 0, NULL, &err), "emptybuffer");

			cl::Buffer buff(m_Context, flags, size, NULL, &err);

			if (!CheckCL(err, "cl::Buffer()"))
				return false;

			NamedBuffer nb(buff, name);

			m_Buffers[bufferIndex] = nb;
		}
		//If the buffer existed and the sizes were the same, take no action.

		return true;
	}

	return false;
}

/// <summary>
/// Add and/or write a buffer of data with the specified name to the list of buffers.
/// Three possible actions to take:
///		Buffer didn't exist, so create and add.
///		Buffer existed, but was a different size. Replace.
///		Buffer existed with the same size, copy data.
/// </summary>
/// <param name="name">The name of the buffer</param>
/// <param name="data">A pointer to the buffer</param>
/// <param name="size">The size in bytes of the buffer</param>
/// <returns>True if success, else false.</returns>
bool OpenCLWrapper::AddAndWriteBuffer(string name, void* data, size_t size)
{
	cl_int err;
	bool b = false;

	if (m_Init)
	{
		int bufferIndex = FindBufferIndex(name);

		//Easy case: totally new buffer, so just create and add.
		if (bufferIndex == -1)
		{
			cl::Buffer buff(m_Context, CL_MEM_READ_WRITE, size, NULL, &err);

			if (!CheckCL(err, "cl::Buffer()"))
				return b;

			NamedBuffer nb(buff, name);

			m_Buffers.push_back(nb);
			b = WriteBuffer((unsigned int)m_Buffers.size() - 1, data, size);
		}
		else//Harder case: the buffer already exists. Replace or overwrite?
		{
			if (GetBufferSize(bufferIndex) == size)//Size was equal, so just copy data without creating a new buffer.
			{
				b = WriteBuffer(bufferIndex, data, size);
			}
			else//Size was not equal, so create entirely new buffer, replace, and copy data.
			{
				cl::Buffer buff(m_Context, CL_MEM_READ_WRITE, size, NULL, &err);

				if (!CheckCL(err, "cl::Buffer()"))
					return b;

				NamedBuffer nb(buff, name);

				m_Buffers[bufferIndex] = nb;
				b = WriteBuffer(bufferIndex, data, size);
			}
		}
	}

	return b;
}

/// <summary>
/// Write data to an existing buffer with the specified name.
/// </summary>
/// <param name="name">The name of the buffer</param>
/// <param name="data">A pointer to the buffer</param>
/// <param name="size">The size in bytes of the buffer</param>
/// <returns>True if success, else false.</returns>
bool OpenCLWrapper::WriteBuffer(string name, void* data, size_t size)
{
	int bufferIndex = FindBufferIndex(name);

	return bufferIndex != -1 ? WriteBuffer(bufferIndex, data, size) : false; 
}

/// <summary>
/// Write data to an existing buffer at the specified index.
/// </summary>
/// <param name="bufferIndex">The index of the buffer</param>
/// <param name="data">A pointer to the buffer</param>
/// <param name="size">The size in bytes of the buffer</param>
/// <returns>True if success, else false.</returns>
bool OpenCLWrapper::WriteBuffer(unsigned int bufferIndex, void* data, size_t size)
{
	if (m_Init && (bufferIndex < m_Buffers.size()) && (GetBufferSize(bufferIndex) == size))
	{
		cl::Event e;
		cl_int err = m_Queue.enqueueWriteBuffer(m_Buffers[bufferIndex].m_Buffer, CL_TRUE, 0, size, data, NULL, &e);

		e.wait();
		m_Queue.finish();

		if (CheckCL(err, "cl::CommandQueue::enqueueWriteBuffer()"))
			return true;
	}

	return false;
}

/// <summary>
/// Read data from an existing buffer with the specified name.
/// </summary>
/// <param name="name">The name of the buffer</param>
/// <param name="data">A pointer to a buffer to copy the data to</param>
/// <param name="size">The size in bytes of the buffer</param>
/// <returns>True if success, else false.</returns>
bool OpenCLWrapper::ReadBuffer(string name, void* data, size_t size)
{
	int bufferIndex = FindBufferIndex(name);

	return bufferIndex != -1 ? ReadBuffer(bufferIndex, data, size) : false; 
}

/// <summary>
/// Read data from an existing buffer at the specified index.
/// </summary>
/// <param name="bufferIndex">The index of the buffer</param>
/// <param name="data">A pointer to a buffer to copy the data to</param>
/// <param name="size">The size in bytes of the buffer</param>
/// <returns>True if success, else false.</returns>
bool OpenCLWrapper::ReadBuffer(unsigned int bufferIndex, void* data, size_t size)
{
	if (m_Init && (bufferIndex < m_Buffers.size()) && (GetBufferSize(bufferIndex) == size))
	{
		cl::Event e;
		cl_int err = m_Queue.enqueueReadBuffer(m_Buffers[bufferIndex].m_Buffer, CL_TRUE, 0, size, data, NULL, &e);

		e.wait();
		m_Queue.finish();

		if (CheckCL(err, "cl::CommandQueue::enqueueReadBuffer()"))
			return true;
	}

	return false;
}

/// <summary>
/// Find the index of the buffer with the specified name.
/// </summary>
/// <param name="name">The name of the buffer to search for</param>
/// <returns>The index if found, else -1.</returns>
int OpenCLWrapper::FindBufferIndex(string name)
{
	for (unsigned int i = 0; i < m_Buffers.size(); i++)
		if (m_Buffers[i].m_Name == name)
			return (int)i;

	return -1;
}

/// <summary>
/// Get the size of the buffer with the specified name.
/// </summary>
/// <param name="name">The name of the buffer to search for</param>
/// <returns>The size of the buffer if found, else 0.</returns>
unsigned int OpenCLWrapper::GetBufferSize(string name)
{
	unsigned int bufferIndex = FindBufferIndex(name);

	return bufferIndex != -1 ? GetBufferSize(bufferIndex) : 0; 
}

/// <summary>
/// Get the size of the buffer at the specified index.
/// </summary>
/// <param name="name">The index of the buffer to get the size of</param>
/// <returns>The size of the buffer if found, else 0.</returns>
unsigned int OpenCLWrapper::GetBufferSize(unsigned int bufferIndex)
{
	if (m_Init && bufferIndex < m_Buffers.size())
		return (unsigned int)m_Buffers[bufferIndex].m_Buffer.getInfo<CL_MEM_SIZE>();

	return 0;
}

/// <summary>
/// Clear all buffers.
/// </summary>
void OpenCLWrapper::ClearBuffers()
{
	m_Buffers.clear();
}

/// <summary>
/// Add and/or write a new 2D image.
/// Three possible actions to take:
///		Image didn't exist, so create and add.
///		Image existed, but was a different size. Replace.
///		Image existed with the same size, copy data.
/// </summary>
/// <param name="name">The name of the image to add/replace</param>
/// <param name="flags">The memory flags</param>
/// <param name="format">The image format</param>
/// <param name="width">The width in pixels of the image</param>
/// <param name="height">The height in pixels of the image</param>
/// <param name="row_pitch">The row pitch (usually zero)</param>
/// <param name="data">The image data. Default: NULL.</param>
/// <param name="shared">True if shared with an OpenGL texture, else false. Default: false.</param>
/// <param name="texName">The texture ID of the shared OpenGL texture if shared. Default: 0.</param>
/// <returns>True if success, else false.</returns>
bool OpenCLWrapper::AddAndWriteImage(string name, cl_mem_flags flags, const cl::ImageFormat& format, ::size_t width, ::size_t height, ::size_t row_pitch, void* data, bool shared, GLuint texName)
{
	cl_int err;

	if (m_Init)
	{
		int imageIndex = FindImageIndex(name, shared);

		if (imageIndex == -1)//If the image didn't exist, create and add.
		{
			if (shared)
			{
				//::wglMakeCurrent(wglGetCurrentDC(), wglGetCurrentContext());
				IMAGEGL2D imageGL(m_Context, flags, GL_TEXTURE_2D, 0, texName, &err);
				NamedImage2DGL namedImageGL(imageGL, name);

				if (CheckCL(err, "cl::ImageGL()"))
				{
					m_GLImages.push_back(namedImageGL);

					if (data)
						return WriteImage2D((unsigned int)m_GLImages.size() - 1, true, width, height, row_pitch, data);//OpenGL images/textures require a separate write.
					else
						return true;
				}
			}
			else
			{
				NamedImage2D namedImage(cl::Image2D(m_Context, flags, format, width, height, row_pitch, data, &err), name);

				if (CheckCL(err, "cl::Image2D()"))
				{
					m_Images.push_back(namedImage);
					return true;
				}
			}
		}
		else//It did exist, so create new if sizes are different. Write if data is not NULL.
		{
			if (shared)
			{
				IMAGEGL2D imageGL = m_GLImages[imageIndex].m_Image;

				if (!CompareImageParams(imageGL, flags, format, width, height, row_pitch))
				{
					NamedImage2DGL namedImageGL(IMAGEGL2D(m_Context, flags, GL_TEXTURE_2D, 0, texName, &err), name);//Sizes are different, so create new.

					if (CheckCL(err, "cl::ImageGL()"))
					{
						m_GLImages[imageIndex] = namedImageGL;
					}
					else
						return false;
				}

				//Write data to new image since OpenGL images/textures require a separate write, must match new size.
				if (data)
					return WriteImage2D(imageIndex, true, width, height, row_pitch, data);
				else
					return true;
			}
			else
			{
				NamedImage2D namedImage = m_Images[imageIndex];

				if (!CompareImageParams(namedImage.m_Image, flags, format, width, height, row_pitch))
				{
					NamedImage2D namedImage(cl::Image2D(m_Context, flags, format, width, height, row_pitch, data, &err), name);

					if (CheckCL(err, "cl::Image2D()"))
					{
						m_Images[imageIndex] = namedImage;
						return true;
					}
				}
				else if (data)
					return WriteImage2D(imageIndex, false, width, height, row_pitch, data);
				else//Strange case: images were same dimensions but no data was passed in, so do nothing.
					return true;
			}
		}
	}

	return false;
}

/// <summary>
/// Write data to an existing 2D image at the specified index.
/// </summary>
/// <param name="index">The index of the image</param>
/// <param name="shared">True if shared with an OpenGL texture, else false.</param>
/// <param name="width">The width in pixels of the image</param>
/// <param name="height">The height in pixels of the image</param>
/// <param name="row_pitch">The row pitch (usually zero)</param>
/// <param name="data">The image data</param>
/// <returns>True if success, else false.</returns>
bool OpenCLWrapper::WriteImage2D(unsigned int index, bool shared, ::size_t width, ::size_t height, ::size_t row_pitch, void* data)
{
	if (m_Init)
	{
		cl_int err;
		cl::Event e;
		cl::size_t<3> origin, region;
		
		origin[0] = 0;
		origin[1] = 0;
		origin[2] = 0;

		region[0] = width;
		region[1] = height;
		region[2] = 1;

		if (shared && index < m_GLImages.size())
		{
			IMAGEGL2D imageGL = m_GLImages[index].m_Image;

			if (EnqueueAcquireGLObjects(imageGL))
			{
				err = m_Queue.enqueueWriteImage(imageGL, CL_TRUE, origin, region, row_pitch, 0, data, NULL, &e);
				e.wait();
				m_Queue.finish();

				bool b = EnqueueReleaseGLObjects(imageGL);
				return CheckCL(err, "cl::enqueueWriteImage()") && b;
			}
		}
		else if (!shared && index < m_Images.size())
		{
			err = m_Queue.enqueueWriteImage(m_Images[index].m_Image, CL_TRUE, origin, region, row_pitch, 0, data, NULL, &e);
			e.wait();
			m_Queue.finish();
			return CheckCL(err, "cl::enqueueWriteImage()");
		}
	}

	return false;
}

/// <summary>
/// Read data from an existing 2D image with the specified name.
/// </summary>
/// <param name="name">The name of the image</param>
/// <param name="width">The width in pixels of the image</param>
/// <param name="height">The height in pixels of the image</param>
/// <param name="row_pitch">The row pitch (usually zero)</param>
/// <param name="shared">True if shared with an OpenGL texture, else false.</param>
/// <param name="data">A pointer to a buffer to copy the data to</param>
/// <returns>True if success, else false.</returns>
bool OpenCLWrapper::ReadImage(string name, ::size_t width, ::size_t height, ::size_t row_pitch, bool shared, void* data)
{
	if (m_Init)
	{
		int imageIndex = FindImageIndex(name, shared);

		if (imageIndex != -1)
			return ReadImage(imageIndex, width, height, row_pitch, shared, data);
	}

	return false;
}

/// <summary>
/// Read data from an existing 2D image at the specified index.
/// </summary>
/// <param name="name">The name of the image</param>
/// <param name="width">The width in pixels of the image</param>
/// <param name="height">The height in pixels of the image</param>
/// <param name="row_pitch">The row pitch (usually zero)</param>
/// <param name="shared">True if shared with an OpenGL texture, else false.</param>
/// <param name="data">A pointer to a buffer to copy the data to</param>
/// <returns>True if success, else false.</returns>
bool OpenCLWrapper::ReadImage(unsigned int imageIndex, ::size_t width, ::size_t height, ::size_t row_pitch, bool shared, void* data)
{
	if (m_Init)
	{
		cl_int err;
		cl::Event e;
		cl::size_t<3> origin, region;
		
		origin[0] = 0;
		origin[1] = 0;
		origin[2] = 0;

		region[0] = width;
		region[1] = height;
		region[2] = 1;

		if (shared && imageIndex < m_GLImages.size())
		{
			IMAGEGL2D imageGL = m_GLImages[imageIndex].m_Image;

			if (EnqueueAcquireGLObjects(imageGL))
			{
				err = m_Queue.enqueueReadImage(m_GLImages[imageIndex].m_Image, true, origin, region, row_pitch, 0, data);
				bool b = EnqueueReleaseGLObjects(m_GLImages[imageIndex].m_Image);
				return CheckCL(err, "cl::enqueueReadImage()") && b;
			}
		}
		else if (!shared && imageIndex < m_Images.size())
		{
			err = m_Queue.enqueueReadImage(m_Images[imageIndex].m_Image, true, origin, region, row_pitch, 0, data);
			return CheckCL(err, "cl::enqueueReadImage()");
		}
	}

	return false;
}

/// <summary>
/// Find the index of the 2D image with the specified name.
/// </summary>
/// <param name="name">The name of the image to search for</param>
/// <param name="shared">True if shared with an OpenGL texture, else false.</param>
/// <returns>The index if found, else -1.</returns>
int OpenCLWrapper::FindImageIndex(string name, bool shared)
{
	if (shared)
	{
		for (unsigned int i = 0; i < m_GLImages.size(); i++)
			if (m_GLImages[i].m_Name == name)
				return i;
	}
	else
	{
		for (unsigned int i = 0; i < m_Images.size(); i++)
			if (m_Images[i].m_Name == name)
				return i;
	}

	return -1;
}

/// <summary>
/// Get the size of the 2D image with the specified name.
/// </summary>
/// <param name="name">The name of the image to search for</param>
/// <param name="shared">True if shared with an OpenGL texture, else false.</param>
/// <returns>The size of the 2D image if found, else 0.</returns>
unsigned int OpenCLWrapper::GetImageSize(string name, bool shared)
{
	int imageIndex = FindImageIndex(name, shared);
	return GetImageSize(imageIndex, shared);
}

/// <summary>
/// Get the size of the 2D image at the specified index.
/// </summary>
/// <param name="imageIndex">Index of the image to search for</param>
/// <param name="shared">True if shared with an OpenGL texture, else false.</param>
/// <returns>The size of the 2D image if found, else 0.</returns>
unsigned int OpenCLWrapper::GetImageSize(unsigned int imageIndex, bool shared)
{
	size_t size = 0;

	if (m_Init)
	{
		if (shared && imageIndex < m_GLImages.size())
		{
			vector<cl::Memory> images;
			images.push_back(m_GLImages[imageIndex].m_Image);
			IMAGEGL2D image = m_GLImages[imageIndex].m_Image;
			
			if (EnqueueAcquireGLObjects(&images))
				size = image.getImageInfo<CL_IMAGE_WIDTH>() * image.getImageInfo<CL_IMAGE_HEIGHT>() * image.getImageInfo<CL_IMAGE_ELEMENT_SIZE>();//Should pitch be checked here?

			EnqueueReleaseGLObjects(&images);
		}
		else if (!shared && imageIndex < m_Images.size())
		{
			cl::Image2D image = m_Images[imageIndex].m_Image;
			size = image.getImageInfo<CL_IMAGE_WIDTH>() * image.getImageInfo<CL_IMAGE_HEIGHT>() * image.getImageInfo<CL_IMAGE_ELEMENT_SIZE>();//Should pitch be checked here?
		}
	}

	return (unsigned int)size;
}

/// <summary>
/// Compare the passed in image with the specified parameters.
/// </summary>
/// <param name="image">The image to compare</param>
/// <param name="flags">The memory flags to compare (ommitted)</param>
/// <param name="format">The format to compare</param>
/// <param name="width">The width to compare</param>
/// <param name="height">The height to compare</param>
/// <param name="row_pitch">The row_pitch to compare (omitted)</param>
/// <returns>True if all parameters matched, else false.</returns>
bool OpenCLWrapper::CompareImageParams(cl::Image& image, cl_mem_flags flags, const cl::ImageFormat& format, ::size_t width, ::size_t height, ::size_t row_pitch)
{
	cl_image_format tempFormat = image.getImageInfo<CL_IMAGE_FORMAT>();

	return (/*image.getImageInfo<CL_MEM_FLAGS>()       == flags  &&*/
			tempFormat.image_channel_data_type       == format.image_channel_data_type &&
			tempFormat.image_channel_order           == format.image_channel_order &&
			image.getImageInfo<CL_IMAGE_WIDTH>()     == width  &&
			image.getImageInfo<CL_IMAGE_HEIGHT>()    == height/* && 
			image.getImageInfo<CL_IMAGE_ROW_PITCH>() == row_pitch*/);//Pitch will be (width * bytes per pixel) + padding.
}

/// <summary>
/// Clear all images.
/// </summary>
/// <param name="shared">True to clear shared images, else clear regular images.</param>
void OpenCLWrapper::ClearImages(bool shared)
{
	if (shared)
		m_GLImages.clear();
	else
		m_Images.clear();
}

/// <summary>
/// Create a 2D image and store in the image passed in.
/// </summary>
/// <param name="image2D">The 2D image to store the newly created image in</param>
/// <param name="flags">The memory flags to use</param>
/// <param name="format">The format to use</param>
/// <param name="width">The width in pixels of the image</param>
/// <param name="height">The height in pixels of the image</param>
/// <param name="row_pitch">The row pitch (usually zero)</param>
/// <param name="data">The image data. Default: NULL.</param>
/// <returns>True if success, else false.</returns>
bool OpenCLWrapper::CreateImage2D(cl::Image2D& image2D, cl_mem_flags flags, cl::ImageFormat format, ::size_t width, ::size_t height, ::size_t row_pitch, void* data)
{
	if (m_Init)
	{
		cl_int err;

		image2D = cl::Image2D(m_Context,
					flags,
					format,
					width,
					height,
					row_pitch,
					data,
					&err);

		return CheckCL(err, "cl::Image2D()");
	}

	return false;
}

/// <summary>
/// Create a 2D image shared with an OpenGL texture and store in the image passed in.
/// </summary>
/// <param name="image2DGL">The 2D image to store the newly created image in</param>
/// <param name="flags">The memory flags to use</param>
/// <param name="target">The target</param>
/// <param name="miplevel">The mip map level</param>
/// <param name="texobj">The texture ID of the shared OpenGL texture</param>
/// <returns>True if success, else false.</returns>
bool OpenCLWrapper::CreateImage2DGL(IMAGEGL2D& image2DGL, cl_mem_flags flags, GLenum target, GLint miplevel, GLuint texobj)
{
	if (m_Init)
	{
		cl_int err;

		image2DGL = IMAGEGL2D(m_Context,
					flags,
					target,
					miplevel,
					texobj,
					&err);

		return CheckCL(err, "cl::ImageGL()");
	}

	return false;
}

/// <summary>
/// Acquire the shared 2D image with the specified name.
/// </summary>
/// <param name="name">The name of the image to acquire</param>
/// <returns>True if success, else false.</returns>
bool OpenCLWrapper::EnqueueAcquireGLObjects(string name)
{
	int index = FindImageIndex(name, true);

	if (index != -1)
		return EnqueueAcquireGLObjects(m_GLImages[index].m_Image);

	return false;
}

/// <summary>
/// Acquire the shared 2D image.
/// </summary>
/// <param name="image">The image to acquire</param>
/// <returns>True if success, else false.</returns>
bool OpenCLWrapper::EnqueueAcquireGLObjects(IMAGEGL2D& image)
{
	if (m_Init && m_Shared)
	{
		vector<cl::Memory> images;

		images.push_back(image);
		cl_int err = m_Queue.enqueueAcquireGLObjects(&images);
		m_Queue.finish();
		return CheckCL(err, "cl::CommandQueue::enqueueAcquireGLObjects()");
	}

	return false;
}

/// <summary>
/// Reelease the shared 2D image with the specified name.
/// </summary>
/// <param name="name">The name of the image to release</param>
/// <returns>True if success, else false.</returns>
bool OpenCLWrapper::EnqueueReleaseGLObjects(string name)
{
	int index = FindImageIndex(name, true);

	if (index != -1)
		return EnqueueReleaseGLObjects(m_GLImages[index].m_Image);

	return false;
}

/// <summary>
/// Release the shared 2D image.
/// </summary>
/// <param name="image">The image to release</param>
/// <returns>True if success, else false.</returns>
bool OpenCLWrapper::EnqueueReleaseGLObjects(IMAGEGL2D& image)
{
	if (m_Init && m_Shared)
	{
		vector<cl::Memory> images;

		images.push_back(image);
		cl_int err = m_Queue.enqueueReleaseGLObjects(&images);
		m_Queue.finish();
		return CheckCL(err, "cl::CommandQueue::enqueueReleaseGLObjects()");
	}

	return false;
}

/// <summary>
/// Acquire a vector of shared OpenGL memory objects.
/// </summary>
/// <param name="memObjects">The memory objects to acquire</param>
/// <returns>True if success, else false.</returns>
bool OpenCLWrapper::EnqueueAcquireGLObjects(const VECTOR_CLASS<cl::Memory>* memObjects)
{
	if (m_Init && m_Shared)
	{
		cl_int err = m_Queue.enqueueAcquireGLObjects(memObjects);

		m_Queue.finish();
		return CheckCL(err, "cl::CommandQueue::enqueueAcquireGLObjects()");
	}

	return false;
}

/// <summary>
/// Release a vector of shared OpenGL memory objects.
/// </summary>
/// <param name="memObjects">The memory objects to release</param>
/// <returns>True if success, else false.</returns>
bool OpenCLWrapper::EnqueueReleaseGLObjects(const VECTOR_CLASS<cl::Memory>* memObjects)
{
	if (m_Init && m_Shared)
	{
		cl_int err = m_Queue.enqueueReleaseGLObjects(memObjects);

		m_Queue.finish();
		return CheckCL(err, "cl::CommandQueue::enqueueReleaseGLObjects()");
	}

	return false;
}

/// <summary>
/// Create a texture sampler.
/// </summary>
/// <param name="sampler">The sampler to store the newly created sampler in</param>
/// <param name="normalizedCoords">True to use normalized coordinates, else don't.</param>
/// <param name="addressingMode">The addressing mode to use</param>
/// <param name="filterMode">The filter mode to use</param>
/// <returns>True if success, else false.</returns>
bool OpenCLWrapper::CreateSampler(cl::Sampler& sampler, cl_bool normalizedCoords, cl_addressing_mode addressingMode, cl_filter_mode filterMode)
{
	cl_int err;
		
	sampler = cl::Sampler(m_Context,
				normalizedCoords,
				addressingMode,
				filterMode,
				&err);

	return CheckCL(err, "cl::Sampler()");
}

/// <summary>
/// Set the argument at the specified index for the kernel at the specified index to be
/// the buffer with the specified name.
/// </summary>
/// <param name="kernelIndex">Index of the kernel</param>
/// <param name="argIndex">Index of the argument</param>
/// <param name="name">The name of the buffer</param>
/// <returns>True if success, else false.</returns>
bool OpenCLWrapper::SetBufferArg(unsigned int kernelIndex, unsigned int argIndex, string name)
{
	int bufferIndex = OpenCLWrapper::FindBufferIndex(name);

	return bufferIndex != -1 ? SetBufferArg(kernelIndex, argIndex, bufferIndex) : false;
}

/// <summary>
/// Set the argument at the specified index for the kernel at the specified index to be
/// the buffer at the specified index.
/// </summary>
/// <param name="kernelIndex">Index of the kernel</param>
/// <param name="argIndex">Index of the argument</param>
/// <param name="bufferIndex">Index of the buffer</param>
/// <returns>True if success, else false.</returns>
bool OpenCLWrapper::SetBufferArg(unsigned int kernelIndex, unsigned int argIndex, unsigned int bufferIndex)
{
	if (m_Init && bufferIndex < m_Buffers.size())
		return SetArg<cl::Buffer>(kernelIndex, argIndex, m_Buffers[bufferIndex].m_Buffer);

	return false;
}

/// <summary>
/// Set the argument at the specified index for the kernel at the specified index to be
/// the 2D image with the specified name.
/// </summary>
/// <param name="kernelIndex">Index of the kernel</param>
/// <param name="argIndex">Index of the argument</param>
/// <param name="shared">True if shared with an OpenGL texture, else false</param>
/// <param name="name">The name of the 2D image</param>
/// <returns>True if success, else false.</returns>
bool OpenCLWrapper::SetImageArg(unsigned int kernelIndex, unsigned int argIndex, bool shared, string name)
{
	if (m_Init)
	{
		int imageIndex = FindImageIndex(name, shared);
		return SetImageArg(kernelIndex, argIndex, shared, imageIndex);
	}

	return false;
}

/// <summary>
/// Set the argument at the specified index for the kernel at the specified index to be
/// the 2D image at the specified index.
/// </summary>
/// <param name="kernelIndex">Index of the kernel</param>
/// <param name="argIndex">Index of the argument</param>
/// <param name="shared">True if shared with an OpenGL texture, else false</param>
/// <param name="imageIndex">Index of the 2D image</param>
/// <returns>True if success, else false.</returns>
bool OpenCLWrapper::SetImageArg(unsigned int kernelIndex, unsigned int argIndex, bool shared, unsigned int imageIndex)
{
	cl_int err;

	if (m_Init)
	{
		if (shared && imageIndex < m_GLImages.size())
		{
			err = m_Programs[kernelIndex].m_Kernel.setArg(argIndex, m_GLImages[imageIndex].m_Image);
			return CheckCL(err, "cl::Kernel::setArg()");
		}
		else if (!shared && imageIndex < m_Images.size())
		{
			err = m_Programs[kernelIndex].m_Kernel.setArg(argIndex, m_Images[imageIndex].m_Image);
			return CheckCL(err, "cl::Kernel::setArg()");
		}
	}

	return false;
}

/// <summary>
/// Find the index of the kernel with the specified name.
/// </summary>
/// <param name="name">The name of the kernel to search for</param>
/// <returns>The index if found, else -1.</returns>
int OpenCLWrapper::FindKernelIndex(string name)
{
	for (unsigned int i = 0; i < m_Programs.size(); i++)
		if (m_Programs[i].m_Name == name)
			return (int)i;

	return -1;
}

/// <summary>
/// Run the kernel at the specified index, using the specified grid and block dimensions.
/// </summary>
/// <param name="kernelIndex">Index of the kernel to run</param>
/// <param name="totalGridWidth">Total width of the grid</param>
/// <param name="totalGridHeight">Total height of the grid</param>
/// <param name="totalGridDepth">The total depth grid</param>
/// <param name="blockWidth">Width of each block</param>
/// <param name="blockHeight">Height of each block</param>
/// <param name="blockDepth">Depth of each block</param>
/// <returns>True if success, else false.</returns>
bool OpenCLWrapper::RunKernel(unsigned int kernelIndex, unsigned int totalGridWidth, unsigned int totalGridHeight, unsigned int totalGridDepth,
	unsigned int blockWidth, unsigned int blockHeight, unsigned int blockDepth)
{
	if (m_Init && kernelIndex < m_Programs.size())
	{
		cl::Event e;
		cl_int err = m_Queue.enqueueNDRangeKernel(m_Programs[kernelIndex].m_Kernel,
			cl::NullRange,
			cl::NDRange(totalGridWidth, totalGridHeight, totalGridDepth),
			cl::NDRange(blockWidth, blockHeight, blockDepth),
			NULL,
			&e);

		e.wait();
		m_Queue.finish();
		return CheckCL(err, "cl::CommandQueue::enqueueNDRangeKernel()");
	}

	return false;
}

/// <summary>
/// Get device information for the specified field.
/// Template argument expected to be cl_ulong, cl_uint or cl_int;
/// </summary>
/// <param name="name">The device field/feature to query</param>
/// <returns>The value of the field</returns>
template<typename T>
T OpenCLWrapper::GetInfo(size_t platform, size_t device, cl_device_info name)
{
	T val;

	if (platform < m_Devices.size() && device < m_Devices[platform].size())
		m_Devices[platform][device].getInfo(name, &val);

	return val;
}

/// <summary>
/// Get the platform name at the specified index.
/// </summary>
/// <param name="i">The platform index to get the name of</param>
/// <returns>The platform name if found, else empty string</returns>
string OpenCLWrapper::PlatformName(size_t platform)
{
	if (platform < m_Platforms.size())
		return m_Platforms[platform].getInfo<CL_PLATFORM_VENDOR>() + " " + m_Platforms[platform].getInfo<CL_PLATFORM_NAME>() + " " + m_Platforms[platform].getInfo<CL_PLATFORM_VERSION>();
	else
		return "";
}

/// <summary>
/// Get all available platform names on the system as a vector of strings.
/// </summary>
/// <returns>All available platform names on the system as a vector of strings</returns>
vector<string> OpenCLWrapper::PlatformNames()
{
	vector<string> platforms;

	platforms.reserve(m_Platforms.size());

	for (unsigned int i = 0; i < m_Platforms.size(); i++)
		platforms.push_back(PlatformName(i));

	return platforms;
}

/// <summary>
/// Get the device name at the specified index on the platform
/// at the specified index.
/// </summary>
/// <param name="platform">The platform index of the device</param>
/// <param name="device">The device index</param>
/// <returns>The name of the device if found, else empty string</returns>
string OpenCLWrapper::DeviceName(size_t platform, size_t device)
{
	string s;

	if (platform < m_Platforms.size() && platform < m_Devices.size())
		if (device < m_Devices[platform].size())
			s = m_Devices[platform][device].getInfo<CL_DEVICE_VENDOR>() + " " + m_Devices[platform][device].getInfo<CL_DEVICE_NAME>();// + " " + m_Devices[platform][device].getInfo<CL_DEVICE_VERSION>();

	return s;
}

/// <summary>
/// Get all available device names on the platform at the specified index as a vector of strings.
/// </summary>
/// <param name="platform">The platform index of the devices to query</param>
/// <returns>All available device names on the platform at the specified index as a vector of strings</returns>
vector<string> OpenCLWrapper::DeviceNames(size_t platform)
{
	unsigned int i = 0;
	string s;
	vector<string> devices;

	do
	{
		s = DeviceName(platform, i);

		if (s != "")
			devices.push_back(s);

		i++;
	} while (s != "");
	
	return devices;
}

/// <summary>
/// Get all availabe device and platform names as one contiguous string.
/// </summary>
/// <returns>A string with all available device and platform names</returns>
string OpenCLWrapper::DeviceAndPlatformNames()
{
	ostringstream os;
	vector<string> deviceNames;

	for (size_t platform = 0; platform < m_Platforms.size(); platform++)
	{
		os << PlatformName(platform) << endl;

		deviceNames = DeviceNames(platform);

		for (size_t device = 0; device < m_Devices[platform].size(); device++)
			os << "\t" << deviceNames[device] << endl;
	}

	return os.str();
}

/// <summary>
/// Get all information about the currently used device.
/// </summary>
/// <returns>A string with all information about the currently used device</returns>
string OpenCLWrapper::DumpInfo()
{
	ostringstream os;
	vector<size_t> sizes;

	os.imbue(std::locale(""));

	for (size_t platform = 0; platform < m_Platforms.size(); platform++)
	{
		os << "Platform " << platform << ": " << PlatformName(platform) << endl;

		for (size_t device = 0; device < m_Devices[platform].size(); device++)
		{
			os << "Device " << device << ": " << DeviceName(platform, device) << endl;
			os << "CL_DEVICE_OPENCL_C_VERSION: "		  << GetInfo<string>  (platform, device, CL_DEVICE_OPENCL_C_VERSION)		  << endl;
			os << "CL_DEVICE_LOCAL_MEM_SIZE: "			  << GetInfo<cl_ulong>(platform, device, CL_DEVICE_LOCAL_MEM_SIZE)			  << endl;
			os << "CL_DEVICE_LOCAL_MEM_TYPE: "			  << GetInfo<cl_uint> (platform, device, CL_DEVICE_LOCAL_MEM_TYPE)			  << endl;
			os << "CL_DEVICE_MAX_COMPUTE_UNITS: "		  << GetInfo<cl_uint> (platform, device, CL_DEVICE_MAX_COMPUTE_UNITS)		  << endl;
			os << "CL_DEVICE_MAX_READ_IMAGE_ARGS: "		  << GetInfo<cl_uint> (platform, device, CL_DEVICE_MAX_READ_IMAGE_ARGS)		  << endl;
			os << "CL_DEVICE_MAX_WRITE_IMAGE_ARGS: "	  << GetInfo<cl_uint> (platform, device, CL_DEVICE_MAX_WRITE_IMAGE_ARGS)	  << endl;
			os << "CL_DEVICE_MAX_MEM_ALLOC_SIZE: "		  << GetInfo<cl_ulong>(platform, device, CL_DEVICE_MAX_MEM_ALLOC_SIZE)		  << endl;
													
			os << "CL_DEVICE_GLOBAL_MEM_CACHE_TYPE: "	  << GetInfo<cl_uint> (platform, device, CL_DEVICE_GLOBAL_MEM_CACHE_TYPE)	  << endl;
			os << "CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE: " << GetInfo<cl_uint> (platform, device, CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE) << endl;
			os << "CL_DEVICE_GLOBAL_MEM_CACHE_SIZE: "	  << GetInfo<cl_ulong>(platform, device, CL_DEVICE_GLOBAL_MEM_CACHE_SIZE)	  << endl;
			os << "CL_DEVICE_GLOBAL_MEM_SIZE: "			  << GetInfo<cl_ulong>(platform, device, CL_DEVICE_GLOBAL_MEM_SIZE)			  << endl;
			os << "CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE: "  << GetInfo<cl_ulong>(platform, device, CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE)  << endl;
													
			os << "CL_DEVICE_MAX_CONSTANT_ARGS: "		  << GetInfo<cl_uint> (platform, device, CL_DEVICE_MAX_CONSTANT_ARGS)		  << endl;
			os << "CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS: "  << GetInfo<cl_uint> (platform, device, CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS)  << endl;
			os << "CL_DEVICE_MAX_WORK_GROUP_SIZE: "		  << GetInfo<::size_t>(platform, device, CL_DEVICE_MAX_WORK_GROUP_SIZE)		  << endl;

			sizes = GetInfo<vector< ::size_t>>(platform, device, CL_DEVICE_MAX_WORK_ITEM_SIZES);
			os << "CL_DEVICE_MAX_WORK_ITEM_SIZES: "		  << sizes[0] << ", " << sizes[1] << ", " << sizes[2] << endl << endl;

			if (device != m_Devices[platform].size() - 1 && platform != m_Platforms.size() - 1)
				os << endl;
		}

		os << endl;
	}

	return os.str();
}

/// <summary>
/// OpenCL properties, getters only.
/// </summary>
bool OpenCLWrapper::Ok() { return m_Init; }
bool OpenCLWrapper::Shared() { return m_Shared; }
cl::Context OpenCLWrapper::Context() { return m_Context; }
unsigned int OpenCLWrapper::PlatformIndex() { return m_PlatformIndex; }
unsigned int OpenCLWrapper::DeviceIndex() { return m_DeviceIndex; }
unsigned int OpenCLWrapper::LocalMemSize() { return m_LocalMemSize; }

/// <summary>
/// Makes the even grid dims.
/// </summary>
/// <param name="blockW">The block w.</param>
/// <param name="blockH">The block h.</param>
/// <param name="gridW">The grid w.</param>
/// <param name="gridH">The grid h.</param>
void OpenCLWrapper::MakeEvenGridDims(unsigned int blockW, unsigned int blockH, unsigned int& gridW, unsigned int& gridH)
{
	if (gridW % blockW != 0)
		gridW += (blockW - (gridW % blockW));

	if (gridH % blockH != 0)
		gridH += (blockH - (gridH % blockH));
}

/// <summary>
/// Create a context that is optionall shared with OpenGL.
/// </summary>
/// <param name="shared">True if shared with OpenGL, else not shared.</param>
/// <returns>True if success, else false.</returns>
bool OpenCLWrapper::CreateContext(bool shared)
{
	cl_int err;

	if (shared)
	{
		//Define OS-specific context properties and create the OpenCL context.
		#if defined (__APPLE__) || defined(MACOSX)
			CGLContextObj kCGLContext = CGLGetCurrentContext();
			CGLShareGroupObj kCGLShareGroup = CGLGetShareGroup(kCGLContext);
			cl_context_properties props[] =
			{
				CL_CONTEXT_PROPERTY_USE_CGL_SHAREGROUP_APPLE, (cl_context_properties)kCGLShareGroup,
				0
			};

			m_Context = cl::Context(CL_DEVICE_TYPE_GPU, props, NULL, NULL, &err);//May need to tinker with this on Mac.
		#else
			#if defined WIN32
				cl_context_properties props[] =
				{
					CL_GL_CONTEXT_KHR, (cl_context_properties)wglGetCurrentContext(),
					CL_WGL_HDC_KHR, (cl_context_properties)wglGetCurrentDC(),
					CL_CONTEXT_PLATFORM, (cl_context_properties)(m_Platforms[m_PlatformIndex])(),
					0
				};
				
				m_Context = cl::Context(CL_DEVICE_TYPE_GPU, props, NULL, NULL, &err);
			#else
				cl_context_properties props[] =
				{
					CL_GL_CONTEXT_KHR, (cl_context_properties)glXGetCurrentContext(),
					CL_GLX_DISPLAY_KHR, (cl_context_properties)glXGetCurrentDisplay(),
					CL_CONTEXT_PLATFORM, (cl_context_properties)(m_Platforms[m_Platform])(),
					0
				};

				m_Context = cl::Context(CL_DEVICE_TYPE_GPU, props, NULL, NULL, &err);
			#endif
		#endif
	}
	else
	{
		cl_context_properties props[3] =
		{
			CL_CONTEXT_PLATFORM,
			(cl_context_properties)(m_Platforms[m_PlatformIndex])(),
			0
		};

		m_Context = cl::Context(CL_DEVICE_TYPE_ALL, props, NULL, NULL, &err);
	}

	return CheckCL(err, "cl::Context()");
}

/// <summary>
/// Create an Spk object created by compiling the program arguments passed in.
/// </summary>
/// <param name="name">The name of the program</param>
/// <param name="program">The source of the program</param>
/// <param name="entryPoint">The name of the entry point kernel function in the program</param>
/// <param name="spk">The Spk object to store the resulting compiled program in</param>
/// <returns>True if success, else false.</returns>
bool OpenCLWrapper::CreateSPK(std::string& name, std::string& program, std::string& entryPoint, Spk& spk, bool doublePrecision)
{
	if (m_Init)
	{
		cl_int err;

		spk.m_Name = name;
		spk.m_Source = cl::Program::Sources(1, std::make_pair(program.c_str(), program.length() + 1));
		spk.m_Program = cl::Program(m_Context, spk.m_Source);

		if (doublePrecision)
			err = spk.m_Program.build(m_DeviceVec, "-cl-mad-enable");//Tinker with other options later.
		else
			err = spk.m_Program.build(m_DeviceVec, "-cl-mad-enable -cl-no-signed-zeros -cl-single-precision-constant");
			//err = spk.m_Program.build(m_DeviceVec, "-cl-mad-enable -cl-no-signed-zeros -cl-fast-relaxed-math -cl-single-precision-constant");//This can cause some rounding.
			//err = spk.m_Program.build(m_DeviceVec, "-cl-mad-enable -cl-single-precision-constant");
			
		if (CheckCL(err, "cl::Program::build()"))
		{
			//Building of program is ok, now create kernel with the specified entry point.
			spk.m_Kernel = cl::Kernel(spk.m_Program, entryPoint.c_str(), &err);
				
			if (CheckCL(err, "cl::Kernel()"))
				return true;//Everything is ok.
		}
	}

	return false;
}

/// <summary>
/// Check an OpenCL return value for errors.
/// </summary>
/// <param name="err">The error code to inspect</param>
/// <param name="name">A description of where the value was gotten from</param>
/// <returns>True if success, else false.</returns>
bool OpenCLWrapper::CheckCL(cl_int err, const char* name)
{
	if (err != CL_SUCCESS)
	{
		ostringstream ss;
		ss << "ERROR: " << ErrorToStringCL(err) << " in " << name << "." << std::endl;
		m_ErrorReport.push_back(ss.str());
	}

	return err == CL_SUCCESS;
}

/// <summary>
/// Translate an OpenCL error code into a human readable string.
/// </summary>
/// <param name="err">The error code to translate</param>
/// <returns>A human readable description of the error passed in</returns>
std::string OpenCLWrapper::ErrorToStringCL(cl_int err)
{
	switch (err)
	{
		case CL_SUCCESS:								   return "Success";
		case CL_DEVICE_NOT_FOUND:						   return "Device not found";
		case CL_DEVICE_NOT_AVAILABLE:					   return "Device not available";
		case CL_COMPILER_NOT_AVAILABLE:					   return "Compiler not available";
		case CL_MEM_OBJECT_ALLOCATION_FAILURE:			   return "Memory object allocation failure";
		case CL_OUT_OF_RESOURCES:						   return "Out of resources";
		case CL_OUT_OF_HOST_MEMORY:						   return "Out of host memory";
		case CL_PROFILING_INFO_NOT_AVAILABLE:			   return "Profiling information not available";
		case CL_MEM_COPY_OVERLAP:						   return "Memory copy overlap";
		case CL_IMAGE_FORMAT_MISMATCH:					   return "Image format mismatch";
		case CL_IMAGE_FORMAT_NOT_SUPPORTED:				   return "Image format not supported";
		case CL_BUILD_PROGRAM_FAILURE:					   return "Program build failure";
		case CL_MAP_FAILURE:							   return "Map failure";
		case CL_MISALIGNED_SUB_BUFFER_OFFSET:			   return "Misaligned sub buffer offset";
		case CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST: return "Exec status error for events in wait list";
		case CL_INVALID_VALUE:							   return "Invalid value";
		case CL_INVALID_DEVICE_TYPE:					   return "Invalid device type";
		case CL_INVALID_PLATFORM:						   return "Invalid platform";
		case CL_INVALID_DEVICE:							   return "Invalid device";
		case CL_INVALID_CONTEXT:						   return "Invalid context";
		case CL_INVALID_QUEUE_PROPERTIES:				   return "Invalid queue properties";
		case CL_INVALID_COMMAND_QUEUE:					   return "Invalid command queue";
		case CL_INVALID_HOST_PTR:						   return "Invalid host pointer";
		case CL_INVALID_MEM_OBJECT:						   return "Invalid memory object";
		case CL_INVALID_IMAGE_FORMAT_DESCRIPTOR:		   return "Invalid image format descriptor";
		case CL_INVALID_IMAGE_SIZE:						   return "Invalid image size";
		case CL_INVALID_SAMPLER:						   return "Invalid sampler";
		case CL_INVALID_BINARY:							   return "Invalid binary";
		case CL_INVALID_BUILD_OPTIONS:					   return "Invalid build options";
		case CL_INVALID_PROGRAM:						   return "Invalid program";
		case CL_INVALID_PROGRAM_EXECUTABLE:				   return "Invalid program executable";
		case CL_INVALID_KERNEL_NAME:					   return "Invalid kernel name";
		case CL_INVALID_KERNEL_DEFINITION:				   return "Invalid kernel definition";
		case CL_INVALID_KERNEL:							   return "Invalid kernel";
		case CL_INVALID_ARG_INDEX:						   return "Invalid argument index";
		case CL_INVALID_ARG_VALUE:						   return "Invalid argument value";
		case CL_INVALID_ARG_SIZE:						   return "Invalid argument size";
		case CL_INVALID_KERNEL_ARGS:					   return "Invalid kernel arguments";
		case CL_INVALID_WORK_DIMENSION:					   return "Invalid work dimension";
		case CL_INVALID_WORK_GROUP_SIZE:				   return "Invalid work group size";
		case CL_INVALID_WORK_ITEM_SIZE:					   return "Invalid work item size";
		case CL_INVALID_GLOBAL_OFFSET:					   return "Invalid global offset";
		case CL_INVALID_EVENT_WAIT_LIST:				   return "Invalid event wait list";
		case CL_INVALID_EVENT:							   return "Invalid event";
		case CL_INVALID_OPERATION:						   return "Invalid operation";
		case CL_INVALID_GL_OBJECT:						   return "Invalid OpenGL object";
		case CL_INVALID_BUFFER_SIZE:					   return "Invalid buffer size";
		case CL_INVALID_MIP_LEVEL:						   return "Invalid mip-map level";
		case CL_INVALID_GLOBAL_WORK_SIZE:				   return "Invalid global work size";
		case CL_INVALID_PROPERTY:						   return "Invalid property";
		default:
		{
			ostringstream ss;
			ss << "<Unknown error code> " << err;
			return ss.str();
		}
	}
}
}