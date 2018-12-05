# ProgramacionParalelaSO
Se aplica filtro blur y otros a una imagen usando GPU
![alt text](https://github.com/Alg0r1thmic/ProgramacionParalelaSO/blob/master/Captura%20de%20pantalla%20de%202018-12-04%2019-15-04.png)

## instale opencl 

Stand-Alone Intel® SDK for OpenCL™ Applications 2017 R2
```shell
https://software.intel.com/en-us/intel-opencl/download

```

## Descomprima el archivo 
```shell
gzip -d intel_sdk_for_opencl_2017_7.0.0.2568_x64.gz
```

## vaya a la carpeta descomprimida
```shell
cd intel_sdk_for_opencl_2017_7.0.0.2568_x64
```
## instale por GUI(recomendado)
```shell
sudo chmod +x install_GUI.sh
```
```shell
sudo ./install_GUI.sh
```
## siga las instrucciones que se muestran en la gui al terminar ya tendria instalado opencl


# Si ud tiene Qt puede abrir el  .pro y ejecutarlo  de lo contrario puede ir a la carpeta sinGui y compilar de la siguiente manera
```shell
g++ main.cpp -o m -lOpenCL

```
## por ultimo llame el ejecutable en este caso m  y puede verificar que en esa carpeta esta la imagen con el filtro aplicado
```shell
./m
```
