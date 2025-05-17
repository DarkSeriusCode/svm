# SVM (Simple Virtual Machine)

> [!WARNING]
> This project is unfinished, so many features may not work properly!

SVM is a 16-bit virtual machine. This project is developed only for the recreational purposes and is
not supposed to be used in serious real-world applications. The goal of the project is to develop an 
assembler and the VM from scratch. Do not treat this repo as something serious.

## Building
All you need is `make` and a C compiler.
```bash
git clone https://github.com/DarkSeriusCode/svm
cd svm
make
./build/sasm --help
./build/svm --help
```

`sasm` is an assembler for SVM.
`svm` is the virtual machine itself.

## Example
Let's write a simple "Hello World" program. (main.asm)
```asm
_main:
    out 1, message, 12
ret

message:
    .ascii "Hello World"
    .byte 10
```

```bash
sasm -o main main.asm
svm main -d ./build/dev/console.so:1
```
Here we compile the program and run it on th VM with the device `./build/dev/console.so` connected to port 1.
You can find additional examples in `examples` folder to learn how to use this repo
