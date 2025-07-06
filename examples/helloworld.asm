#use "build/console.so" 1

_main:
    out 1, msg, msg_len
ret

msg:
    .ascii "Hello world"
    .byte 10 ; For a new line

msg_len: .sizeof msg
