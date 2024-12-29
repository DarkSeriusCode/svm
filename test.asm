section code
_main:
    load r0 num
    mov r1, 333
    add r0, r1
    sub r2, r3
    mul r4, r5
    div r6, r7
    store r0, num
end

;; Num is declared here
section data
iq dw -69
num db 34
; name dw "Mio Akiyama I love u"
end
