#include "mainwindow.h"
#include "imagepro.h"
#include <QApplication>
//#include <QCoreApplication>
#include <CL/cl.hpp>
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <time.h>
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

////////////////////////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[])
{

    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    //ImagePro image("test.ppm");
    //image.test();
    return a.exec();

}

///////////////////////////////////////////////////////////////////////////////////////////////////

