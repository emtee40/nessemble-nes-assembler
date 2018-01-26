.inesprg 1
.ineschr 1
.inesmir 0
.inesmap 0

;;;;;;;;;;;;;;;;

.prg 0

RESET:
    SEI
    CLD
    LDX #$40
    STX $4017
    LDX #$FF
    TXS
    INX
    STX $2000
    STX $2001
    STX $4010

;;;;;;;;

vblankwait1:
    LDA $2002
    BPL vblankwait1

;;;;;;;;

    LDA #$00
clrmem:
    STA <$00, X
    STA $0100, X
    STA $0200, X
    STA $0300, X
    STA $0400, X
    STA $0500, X
    STA $0600, X
    STA $0700, X
    INX
    BNE clrmem

;;;;;;;;

vblankwait2:
    LDA $2002
    BPL vblankwait2

;;;;;;;;

    JSR unknown_C063

;;;;;;;;

unknown_C03D:
    LDA #$00
    STA $2003
    LDA #$02
    STA $4014
    JSR unknown_C074

;;;;;;;;

unknown_C04A:
    LDA #$00
    STA $4015
    LDA #$00
    STA <$00
    LDA #$08
    STA <$01
    JSR unknown_C109
    JSR unknown_C16C

;;;;;;;;

unknown_C05D:
    LDA $2002
    JMP LoadPalettes

;;;;;;;;

unknown_C063:
    LDY #$40
    LDX #$00
    LDA #$F8
    STA $0200, X
    INX
    INX
    INX
    INX
    DEY
    BNE unknown_C063
    RTS

;;;;;;;;

unknown_C074:
    LDA $2002
    LDA #$20
    STA $2006
    LDA #$00
    STA $2006
    LDA #$00
    LDY #$10
    LDX #$00
:
    STA $2007
    DEX
    BNE :-
    DEY
    BNE :-
    RTS

;;;;;;;;

NMI:
    RTI

;;;;;;;;

LoadPalettes:
    LDA #$00
    STA $2000
    STA $2001
    LDA #$3F
    STA $2006
    LDA #$00
    STA $2006
    STA $0325
:
    LDA $0325
    CMP #$04
    BCS unknown_C0BD
    LDY $0325
    LDA palette, Y
    STA $2007
    INC $0325
    JMP :-

;;;;;;;;

unknown_C0BD:
    LDA #$21
    STA $2006
    LDA #$CA
    STA $2006
    LDA #$00
    STA $0325
:
    LDA $0325
    CMP #$0D
    BCS unknown_C0E2
    LDY $0325
    LDA $C136, Y
    STA $2007
    INC $0325
    BCS :-

;;;;;;;;

unknown_C0E2:
    LDA #$00
    STA $2006
    STA $2006
    STA $2005
    STA $2005
    LDA #$90
    STA $2000
    LDA #$1E
    STA $2001

;;;;;;;;

Forever:
    JMP Forever

;;;;;;;;

unknown_C0FD:
    LDA #$00
    BEQ :+
    LDA #$47
    LDX #$C1
    JMP $0300
:
    RTS

;;;;;;;;

unknown_C109:
    LDA #$47
    STA <$08
    LDA #$C1
    STA <$09
    LDA #$00
    STA <$0A
    LDA #$03
    STA <$0B
    LDX #$DA
    LDA #$FF
    STA <$10
    LDY #$00
:
    INX
    BEQ :+
:
    LDA [$08], Y
    STA [$0A], Y
    INY
    BNE :--
    INC <$09
    INC <$0B
    BNE :--
:
    INC <$10
    BNE :--
    RTS

;;;;;;;;

text:
    .ascii "Hello World!"
    .db $00

palette:
    .db $1F, $00, $10, $20

data:
    .db $8D, $0E, $03, $8E, $0F, $03, $8D, $15
    .db $03, $8E, $16, $03, $88, $B9, $FF, $FF
    .db $8D, $1F, $03, $88, $B9, $FF, $FF, $8D
    .db $1E, $03, $8C, $21, $03, $20, $FF, $FF

unknown_C16C:
    LDY #$00
    BEQ :+
    LDA #$92
    LDX #$C0
    JMP $0300
:
    RTS

;;;;;;;;

.org $FFFA

    .dw NMI
    .dw RESET
    .dw NMI

;;;;;;;;;;;;;;;;

.chr 0

    .incpng "lesson1.png"
