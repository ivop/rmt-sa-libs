
    org $4000

main
    ldx #9
loop
    lda shadow,x
    sta $d200,x
    dex
    bpl loop

    lda shadow+0x0f
    sta $d20f

    jmp main


    org $8200
shadow
:16 dta 0
