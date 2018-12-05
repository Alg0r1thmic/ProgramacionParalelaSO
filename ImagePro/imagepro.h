#ifndef IMAGEPRO_H
#define IMAGEPRO_H
#include <vector>
#include <stdio.h>
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <cstdio>
#include <time.h>
#ifdef __APPLE__
    #include "OpenCL/opencl.h"
#else
    #include "CL/cl.h"
#endif

using namespace std;

struct Image
{
    std::vector<char> pixel;
    int width, height;
};


class ImagePro
{
public:
    ImagePro(const char *);
    std::string GetPlatformName (cl_platform_id id);
    std::string GetDeviceName (cl_device_id id);
    void CheckError (cl_int error);
    std::string LoadKernel (const char* name);
    cl_program CreateProgram (const std::string& source, cl_context context);
    Image LoadImage (const char* path);
    void SaveImage (const Image& img, const char* path);
    Image RGBtoRGBA (const Image& input);
    Image RGBAtoRGB (const Image& input);
    void test();
private:
    const char *name;
};

#endif // IMAGEPRO_H
