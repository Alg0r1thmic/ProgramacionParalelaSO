#include "mainwindow.h"//
#include <QApplication>
//#include <QCoreApplication>
#include <CL/cl.hpp>
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>

using namespace std;
/*
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();
}
*/



#ifdef __APPLE__
    #include "OpenCL/opencl.h"
#else
    #include "CL/cl.h"
#endif

struct Image
{
    std::vector<char> pixel;
    int width, height;
};

Image load_image (const char* path)
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

void save_image (const Image& img, const char* path)
{
    std::ofstream out (path, std::ios::binary);

    out << "P6\n";
    out << img.width << " " << img.height << "\n";
    out << "255\n";
    out.write (img.pixel.data (), img.pixel.size ());
}

Image RGB_to_RGBA (const Image& input)
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

Image RGBA_to_RGB (const Image& input)
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

void check_error (cl_int error)
{
    if (error != CL_SUCCESS) {
        std::cerr << "Hubo un error en OpenCL " << error << std::endl;
        std::exit (1);
    }
}

std::string load_kernel (const char* name)
{
    std::ifstream in (name);
    std::string result (
        (std::istreambuf_iterator<char> (in)),
        std::istreambuf_iterator<char> ());
    return result;
}

cl_program create_program (const std::string& source,
    cl_context context)
{
    size_t lengths [1] = { source.size () };
    const char* sources [1] = { source.data () };

    cl_int error = 0;
    cl_program program = clCreateProgramWithSource (context, 1, sources, lengths, &error);
    check_error (error);

    return program;
}


int main(int argc, char *argv[])
{

    QApplication a(argc, argv);
    MainWindow w;
    w.show();


    cl_uint platformIdCount = 0;
    clGetPlatformIDs (0, nullptr, &platformIdCount);

    cout<< "sad" << endl;
    std::vector<cl_platform_id> platformIds (platformIdCount);
    clGetPlatformIDs (platformIdCount, platformIds.data (), nullptr);

    cl_uint deviceIdCount = 0;
    clGetDeviceIDs (platformIds [0], CL_DEVICE_TYPE_ALL, 0, nullptr,
        &deviceIdCount);

    if (deviceIdCount == 0) {
        std::cerr << "No hay dispositivos" << std::endl;
        return 1;
    } else {
        std::cout << "Hay " << deviceIdCount << " dispositivox" << std::endl;
    }

    std::vector<cl_device_id> deviceIds (deviceIdCount);
    clGetDeviceIDs (platformIds [0], CL_DEVICE_TYPE_ALL, deviceIdCount,
        deviceIds.data (), nullptr);

    const cl_context_properties contextProperties [] =
    {
        CL_CONTEXT_PLATFORM, reinterpret_cast<cl_context_properties> (platformIds [0]),
        0, 0
    };

    cl_int error = CL_SUCCESS;
    cl_context context = clCreateContext (contextProperties, deviceIdCount,
        deviceIds.data (), nullptr, nullptr, &error);
    check_error (error);

    std::cout << "ok" << std::endl;

    float filter [] = {
        1, 2, 1,
        2, 4, 2,
        1, 2, 1
    };

    /*Filtro deescurecer
    float filter [] = {
        1, 0, 1,
        0, 1, 1,
        1, 0, 1
    };
    */

    // normalizacion
    for (int i = 0; i < 9; ++i) {
        filter [i] /= 16.0f;
    }

    cl_program program = create_program (load_kernel ("kernels/image.cl"),
        context);

    check_error (clBuildProgram (program, deviceIdCount, deviceIds.data (),
        "-D FILTER_SIZE=1", nullptr, nullptr));

    cl_kernel kernel = clCreateKernel (program, "Filter", &error);
    check_error (error);

    // OpenCL soporta RGB
    //const *imagename=w.getFilename().c_str();
    const auto image = RGB_to_RGBA (load_image ("test.ppm"));

    static const cl_image_format format = { CL_RGBA, CL_UNORM_INT8 };
    cl_mem inputImage = clCreateImage2D (context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, &format,
        image.width, image.height, 0,
        // This is a bug in the spec
        const_cast<char*> (image.pixel.data ()),
        &error);
    check_error (error);

    cl_mem outputImage = clCreateImage2D (context, CL_MEM_WRITE_ONLY, &format,
        image.width, image.height, 0,
        nullptr, &error);
    check_error (error);

    cl_mem filterWeightsBuffer = clCreateBuffer (context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
        sizeof (float) * 9, filter, &error);
    check_error (error);

    clSetKernelArg (kernel, 0, sizeof (cl_mem), &inputImage);
    clSetKernelArg (kernel, 1, sizeof (cl_mem), &filterWeightsBuffer);
    clSetKernelArg (kernel, 2, sizeof (cl_mem), &outputImage);

    cl_command_queue queue = clCreateCommandQueue (context, deviceIds [0],
        0, &error);
    check_error (error);
    std::size_t offset [3] = { 0 };
    std::size_t size [3] = { image.width, image.height, 1 };
    check_error (clEnqueueNDRangeKernel (queue, kernel, 2, offset, size, nullptr,
        0, nullptr, nullptr));

    Image result = image;
    std::fill (result.pixel.begin (), result.pixel.end (), 0);

    std::size_t origin [3] = { 0 };
    std::size_t region [3] = { result.width, result.height, 1 };
    clEnqueueReadImage (queue, outputImage, CL_TRUE,
        origin, region, 0, 0,
        result.pixel.data (), 0, nullptr, nullptr);

    save_image (RGBA_to_RGB (result), "output.ppm");

    clReleaseMemObject (outputImage);
    clReleaseMemObject (filterWeightsBuffer);
    clReleaseMemObject (inputImage);

    clReleaseCommandQueue (queue);

    clReleaseKernel (kernel);
    clReleaseProgram (program);

    clReleaseContext (context);

    return a.exec();

}
