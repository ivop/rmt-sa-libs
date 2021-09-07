
    org $4000

main
    ldx #8
loop
    lda shadow,x
    sta $d200,x
    lda shadow2,x
    sta $d210,x
    dex
    bpl loop

    lda shadow+0x0f
    sta $d20f
    lda shadow2+0x0f
    sta $d21f

    jmp main


    org $8200

shadow
:15 dta 0
    dta 3       ; SKCTL

shadow2
:15 dta 0
    dta 3       ; SKCTL

    run main
