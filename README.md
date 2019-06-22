
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
todo  
  
### Options  
todo  
  
#### Examples  
Distribute the secret “Secreto.bmp” with watermark “Marca.bmp” with scheme (4,8) saving shares at “./pruebas/shares”:

```
./CRIPTO_TP -d -s ./pruebas/Secreto.bmp -m ./pruebas/Marca.bmp -k 4 -n 8 --dir ./pruebas/shares
```

Retrieve secret “secreto.bmp”, with watermark “Rw.bmp” (at output) with scheme (4,8) looking for images at “output/lsb"

```
./CRIPTO_TP -r -s ./output/Secreto.bmp -m ./output/Rw.bmp -k 4 -n 8 --dir ./output/lsb
```

If you wish you can run with a verbose option, including ```-v``` at the end.
  
## Authors  
* Juan Manuel Alonso  
* Ignacio Cifuentes  
* Kevin Cortés Rodriguez