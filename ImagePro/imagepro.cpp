#include "imagepro.h"


ImagePro::ImagePro(const char *n)
    :name(n)
{

}

string ImagePro::GetPlatformName(cl_platform_id id)
{
    size_t size = 0;
    clGetPlatformInfo (id, CL_PLATFORM_NAME, 0, nullptr, &size);

    std::string result;
    result.resize (size);
    clGetPlatformInfo (id, CL_PLATFORM_NAME, size,
        const_cast<char*> (result.data ()), nullptr);

    return result;
}

string ImagePro::GetDeviceName(cl_device_id id)
{
    size_t size = 0;
    clGetDeviceInfo (id, CL_DEVICE_NAME, 0, nullptr, &size);

    std::string result;
    result.resize (size);
    clGetDeviceInfo (id, CL_DEVICE_NAME, size,
        const_cast<char*> (result.data ()), nullptr);

    return result;
}

void ImagePro::CheckError(cl_int error)
{
    if (error != CL_SUCCESS) {
        std::cerr << "OpenCL call failed with error " << error << std::endl;
        std::exit (1);
    }
}

string ImagePro::LoadKernel(const char *name)
{
    std::ifstream in (name);
    std::string result (
        (std::istreambuf_iterator<char> (in)),
        std::istreambuf_iterator<char> ());
    return result;
}

cl_program ImagePro::CreateProgram(const string &source, cl_context context)
{
    size_t lengths [1] = { source.size () };
    const char* sources [1] = { source.data () };

    cl_int error = 0;
    cl_program program = clCreateProgramWithSource (context, 1, sources, lengths, &error);
    CheckError (error);

    return program;
}

Image ImagePro::LoadImage(const char *path)
{
    std::ifstream in (path, std::ios::binary);

    std::string s;
    in >> s;

    if (s != "P6") {
        exit (1);
    }

    for (;;) {
        getline (in, s);

        if (s.empty ()) {
            continue;
        }

        if (s [0] != '#') {
            break;
        }
    }

    std::stringstream str (s);
    int width, height, maxColor;
    str >> width >> height;
    in >> maxColor;

    if (maxColor != 255) {
        exit (1);
    }

    {
        std::string tmp;
        getline(in, tmp);
    }

    std::vector<char> data (width * height * 3);
    in.read (reinterpret_cast<char*> (data.data ()), data.size ());

    const Image img = { data, width, height };
    return img;
}

void ImagePro::SaveImage(const Image &img, const char *path)
{
    std::ofstream out (path, std::ios::binary);

    out << "P6\n";
    out << img.width << " " << img.height << "\n";
    out << "255\n";
    out.write (img.pixel.data (), img.pixel.size ());
}

Image ImagePro::RGBtoRGBA(const Image &input)
{
    Image result;
    result.width = input.width;
    result.height = input.height;

    for (std::size_t i = 0; i < input.pixel.size (); i += 3) {
        result.pixel.push_back (input.pixel [i + 0]);
        result.pixel.push_back (input.pixel [i + 1]);
        result.pixel.push_back (input.pixel [i + 2]);
        result.pixel.push_back (0);
    }

    return result;
}

Image ImagePro::RGBAtoRGB(const Image &input)
{
    Image result;
    result.width = input.width;
    result.height = input.height;

    for (std::size_t i = 0; i < input.pixel.size (); i += 4) {
        result.pixel.push_back (input.pixel [i + 0]);
        result.pixel.push_back (input.pixel [i + 1]);
        result.pixel.push_back (input.pixel [i + 2]);
    }

    return result;
}

void ImagePro::test()
{
    clock_t total;
     clock_t t;
    t = clock();

    cl_uint platformIdCount = 0;
    clGetPlatformIDs (0, nullptr, &platformIdCount);

    if (platformIdCount == 0) {
        std::cerr << "No OpenCL platform found" << std::endl;
    } else {
        std::cout << "Found " << platformIdCount << " platform(s)" << std::endl;
    }

    std::vector<cl_platform_id> platformIds (platformIdCount);
    clGetPlatformIDs (platformIdCount, platformIds.data (), nullptr);

    for (cl_uint i = 0; i < platformIdCount; ++i) {
        std::cout << "\t (" << (i+1) << ") : " << GetPlatformName (platformIds [i]) << std::endl;
    }

    cl_uint deviceIdCount = 0;
    clGetDeviceIDs (platformIds [0], CL_DEVICE_TYPE_ALL, 0, nullptr,
        &deviceIdCount);

    if (deviceIdCount == 0) {
        std::cerr << "No OpenCL devices found" << std::endl;
    } else {
        std::cout << "Found " << deviceIdCount << " device(s)" << std::endl;
    }

    std::vector<cl_device_id> deviceIds (deviceIdCount);

    // MODIFICAR EL ÍNDICE DE platformIds PARA SELECCIONAR LA TARJETA DE VIDEO
    clGetDeviceIDs (platformIds [0], CL_DEVICE_TYPE_ALL, deviceIdCount,
        deviceIds.data (), nullptr);

    for (cl_uint i = 0; i < deviceIdCount; ++i) {
        std::cout << "\t (" << (i+1) << ") : " << GetDeviceName (deviceIds [i]) << std::endl;
    }

    // MODIFICAR EL ÍNDICE DE platformIds PARA SELECCIONAR LA TARJETA DE VIDEO
    const cl_context_properties contextProperties [] =
    {
        CL_CONTEXT_PLATFORM, reinterpret_cast<cl_context_properties> (platformIds [0]),
        0, 0
    };

    cl_int error = CL_SUCCESS;
    cl_context context = clCreateContext (contextProperties, deviceIdCount,
        deviceIds.data (), nullptr, nullptr, &error);
    CheckError (error);

    std::cout << "Context created" << std::endl;

    float filter [] = {
        1, 2, 1,
        2, 4, 2,
        1, 2, 1
    };

    for (int i = 0; i < 9; ++i) {
        filter [i] /= 16.0f;
    }

    cl_program program = CreateProgram (LoadKernel ("kernels/image.cl"),
        context);

    CheckError (clBuildProgram (program, deviceIdCount, deviceIds.data (),
        "-D FILTER_SIZE=1", nullptr, nullptr));cerr << "ok\n";

    cl_kernel kernel = clCreateKernel (program, "Filter", &error);
    CheckError (error);
    const auto image = RGBtoRGBA (LoadImage (this->name));

    static const cl_image_format format = { CL_RGBA, CL_UNORM_INT8 };
    cl_mem inputImage = clCreateImage2D (context,
                                        CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                                        &format,
                                        image.width, image.height, 0,
                                        const_cast<char*> (image.pixel.data ()),
                                        &error
                                        );
    CheckError (error);



    cl_mem outputImage = clCreateImage2D (context, CL_MEM_WRITE_ONLY, &format,
        image.width, image.height, 0,
        nullptr, &error);
    CheckError (error);

    cl_mem filterWeightsBuffer = clCreateBuffer (context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
        sizeof (float) * 9, filter, &error);
    CheckError (error);

    clSetKernelArg (kernel, 0, sizeof (cl_mem), &inputImage);
    clSetKernelArg (kernel, 1, sizeof (cl_mem), &filterWeightsBuffer);
    clSetKernelArg (kernel, 2, sizeof (cl_mem), &outputImage);

    cl_command_queue queue = clCreateCommandQueue (context, deviceIds [0],
        0, &error);
    CheckError (error);

    std::size_t offset [3] = { 0 };
    std::size_t global_size [3] = { image.width, image.height, 1 };//HILOS
    std::size_t group_work_size [3] = { 16, 16, 1 };//BLOQUES (cantidad de hilos que estarán en un bloque)
    CheckError (clEnqueueNDRangeKernel (queue, kernel, 2, offset, global_size, group_work_size,
        0, nullptr, nullptr));

    Image result = image;
    std::fill (result.pixel.begin (), result.pixel.end (), 0);

    std::size_t origin [3] = { 0 };
    std::size_t region [3] = { result.width, result.height, 1 };
    clEnqueueReadImage (queue, outputImage, CL_TRUE,
        origin, region, 0, 0,
        result.pixel.data (), 0, nullptr, nullptr);

    SaveImage (RGBAtoRGB (result), "output.ppm");




    clReleaseMemObject (outputImage);
    clReleaseMemObject (filterWeightsBuffer);
    clReleaseMemObject (inputImage);

    clReleaseCommandQueue (queue);

    clReleaseKernel (kernel);
    clReleaseProgram (program);

    clReleaseContext (context);
        t = clock() - t;
    printf ("%f segundos\n",t,((float)t)/CLOCKS_PER_SEC);

}
