#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QMenu>
#include <QDebug>
#include <QPixmap>
#include <iostream>
#include "imagepro.h"
using namespace std;
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_clicked()
{
    QString  filename=QFileDialog::getOpenFileName(this,tr("open file"),"/home/raul/","All files (*.*)");
    this->fileName=filename.toStdString();
    QPixmap pix(filename);
    int w=ui->label->width();
    int h=ui->label->height();
    ui->label->setPixmap(pix.scaled(w,h,Qt::KeepAspectRatio));
    cout<< this->fileName <<endl;
    const char * c = this->fileName.c_str();
    ImagePro image(c);
    image.test();
}

void MainWindow::on_pushButton_2_clicked()
{
    QString name="/home/raul/ProgramacionParalelaSO/build-ImagePro-Desktop_Qt_5_11_2_GCC_64bit-Debug/output.ppm";
    QPixmap pix(name);
    int w=ui->label_2->width();
    int h=ui->label_2->height();
    ui->label_2->setPixmap(pix.scaled(w,h,Qt::KeepAspectRatio));
}

string MainWindow::getFilename()
{
    return this->fileName;
}
