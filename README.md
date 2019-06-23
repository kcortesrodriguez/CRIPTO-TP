
# Shared Seceret in Images with Steganography  
  
Project for Cryptography and Security course at Instituto Tecnológico de Buenos Aires.  
  
## Getting Started  
These instructions will install the system in your local machine.  
  
### Prerequisites  
* cmake
* clang
* makefile
  
### Compiling and Installing  
Build the project at root using the makefile provided
```
$> make
```

An executable named "CRIPTO_TP" will result from the previous command at the root directory.  

## Usage  
  
### Options  
The program should receive as parameters:

 - -d or -r
 - -s "path_to_secret_image.bmp"
 - -m "path_to_watermark_image.bmp"
 - -k number
 - -n number
 - --dir directory

Meaning of each of the mandatory parameters:
- -d: indicates that a secret image will be distributed in other images.
- -R: indicates that a secret image will be recovered from other images.
- -s image: The image name corresponds to the name of a file of extension .bmp. In the case that the option (-d) has been chosen, this file must exist since it is the image to be hidden and must be a black and white image (8 bits per pixel). If the option (-r) was chosen, it file will be the output file, with the secret image revealed at the end of the program.
- -m image: The image name corresponds to the name of a file with the extension .bmp. In the case that the option (-d) has been chosen, this file is a black and white image that will serve as a "watermark" to verify the entire process. It must be the same size as the secret image. In the case that the option (-r) has been chosen, this file is a black and white image that contains the transformation of the "watermark" image.
- -k number: The number corresponds to the minimum number of shadows needed to recover the secret in a scheme (k, n).
- -n number: The number corresponds to the total number of shadows in which the secret will be distributed in a scheme (k, n).
- --dir directory The directory where the images are located in which the secret will be distributed (in case the option (-d) has been chosen), or where the images that contain the secret are hidden (in the case that option (-r) has been chosen. It must contain images of .bmp extension, 24 bits per pixel.
  
#### Examples  
Distribute the secret “Secreto.bmp” with watermark “Marca.bmp” with scheme (4,8) saving shares at “./pruebas/shares”:

```
./CRIPTO_TP -d -s ./pruebas/Secreto.bmp -m ./pruebas/Marca.bmp -k 4 -n 8 --dir ./pruebas/shares
```

Retrieve secret “secreto.bmp”, with watermark “RW.bmp” (at output) with scheme (4,8) looking for images at “output/lsb"

```
./CRIPTO_TP -r -s ./output/Secreto.bmp -m ./output/RW.bmp -k 4 -n 8 --dir ./output/lsb
```

If you wish you can run with a verbose option, including ```-v``` at the end.
  
## Authors  
* Juan Manuel Alonso  
* Ignacio Cifuentes  
* Kevin Cortés Rodriguez