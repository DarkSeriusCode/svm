# SVM (Simple virtual machine)
SVM is a 16-bit virtual machine based on my own architecture. This project was created for my
educational purposes. The goal of the project is to develop my own architecture and create the VM
without any third-party dependencies. Currently, most features are not yet implemented.

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
Let's run this program (named `main.asm`):
```asm
_main:
    movi r0, 34
    movi r1, 35
    add r0, r1
```

```bash
sasm -o main main.asm
svm main
```
Because the project is unfinished, the execution result will be saved in `vm.dump`
`r0: 0x0045` means that register `r0` contains 0x0045 (69 in decimal).

For more information about the assembler see `docs/assembler.md`
