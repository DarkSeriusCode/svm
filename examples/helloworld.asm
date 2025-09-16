#use "build/console.so" 1

_main:
    ld r1, msg_len ;; Load a value at msg_len to the register because io/out CAN NOT accept addresses as a lenght
    out 1, msg, r1
ret

msg:
    .ascii "Hello world"
    .byte 10 ; For a new line

msg_len: .sizeof msg
