
    org $4000

main
    ldx #8
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
:15 dta 0
    dta 3       ; SKCTL

    run main
