[section code]
start:
	reset
	call main
	hlt

readOp:
	usp 8
.entry0:
	br .entry1
.entry1:
	call readChar
	mv fp[8], rr
	call readChar
	call readChar
	br .ret2
.ret2:
	mv rr, fp[8]
	ret

readInt:
	usp 16
.entry3:
	br .entry4
.entry4:
	mv fp[8], 0b0000010100000000000000010000000000000000000000000000000000000000
	mv fp[16], 0b0000010100000000000000010000000000000000000000000000000000000000
	call readChar
	mv fp[8], rr
	br .doWhileBody5
.doWhileBody5:
	br .entry8
.entry8:
	sub r0, fp[8], 0b0000010100000000000000010000000000000000000000000000000000110000
	add r1, 0b0000010100000000000000010000000000000000000000000000000000001010, r0
	mul r0, fp[16], r1
	mv fp[16], r0
	call readChar
	mv fp[8], rr
	br .doWhileCond6
.doWhileFallthrough7:
	call readChar
	br .ret9
.ret9:
	mv rr, fp[16]
	ret
.doWhileCond6:
	neq r0, fp[8], 0b0000011100000000000000010000000000000000000000000000000000001101
	br r0, .doWhileBody5, .doWhileFallthrough7

printUInt:
	usp 24
.entry10:
	br .entry11
.entry11:
	mv fp[8], 0b0000010100000000000000010000000000000000000000000000000000000001
	mv fp[16], r0
	br .whileCond12
.whileCond12:
	gte r1, fp[16], 0b0000010100000000000000010000000000000000000000000000000000001010
	br r1, .whileBody13, .whileFallthrough14
.whileBody13:
	br .entry15
.entry15:
	mul r1, fp[8], 0b0000010100000000000000010000000000000000000000000000000000001010
	mv fp[8], r1
	div r1, fp[16], 0b0000010100000000000000010000000000000000000000000000000000001010
	mv fp[16], r1
	br .whileCond12
.whileFallthrough14:
	br .whileCond16
.whileCond16:
	gt r1, fp[8], 0b0000010100000000000000010000000000000000000000000000000000000000
	br r1, .whileBody17, .whileFallthrough18
.whileBody17:
	br .entry19
.entry19:
	add r1, fp[8], 0b0000010100000000000000010000000000000000000000000000000000110000
	div r2, r0, r1
	mv fp[24], r2
	push r0
	mv r0, fp[24]
	call writeChar
	pop r0
	mod r1, r0, fp[8]
	mv r0, r1
	div r1, fp[8], 0b0000010100000000000000010000000000000000000000000000000000001010
	mv fp[8], r1
	br .whileCond16
.whileFallthrough18:
	push r0
	st r0, 0b0000011100000000000000010000000000000000000000000000000000001101
	call writeChar
	pop r0
	push r0
	st r0, 0b0000011100000000000000010000000000000000000000000000000000001010
	call writeChar
	pop r0
	br .ret20
.ret20:
	mv rr, rr
	ret

printInt:
	usp 0
.entry21:
	br .entry22
.entry22:
	br .ifCond23
.ifCond23:
	bshr r1, r0, 0b0000010100000000000000010000000000000000000000000000000000111111
	neq r2, r1, 0b0000010100000000000000010000000000000000000000000000000000000000
	br r2, .ifTrue24, .ifFallthrough25
.ifTrue24:
	br .entry26
.ifFallthrough25:
	push r0
	mv r0, r0
	call printUInt
	pop r0
	br .ret27
.ret27:
	mv rr, rr
	ret
.entry26:
	push r0
	st r0, 0b0000011100000000000000010000000000000000000000000000000000101101
	call writeChar
	pop r0
	bnot r1, r0
	add r2, r1, 0b0000010100000000000000010000000000000000000000000000000000000001
	mv r0, r2
	br .ifFallthrough25

main:
	usp 32
.entry28:
	br .entry29
.entry29:
	mv fp[8], 0b0000010100000000000000010000000000000000000000000000000000000000
	mv fp[16], 0b0000010100000000000000010000000000000000000000000000000000000000
	br .whileCond30
.whileCond30:
	br 0b0000000100000000000000010000000000000000000000000000000000000001, .whileBody31, .whileFallthrough32
.whileBody31:
	br .entry33
.entry33:
	call readOp
	mv fp[24], rr
	br .ifCond34
.whileFallthrough32:
	br .ret54
.ret54:
	mv rr, fp[16]
	ret
.ifCond34:
	eq r0, fp[24], 0b0000011100000000000000010000000000000000000000000000000001110001
	br r0, .ifTrue35, .ifFallthrough36
.ifTrue35:
	br .entry37
.ifFallthrough36:
	call readInt
	mv fp[32], rr
	br .ifCond38
.ifCond38:
	eq r0, fp[24], 0b0000011100000000000000010000000000000000000000000000000000101011
	br r0, .ifTrue39, .ifFallthrough40
.ifTrue39:
	br .entry41
.ifFallthrough40:
	br .ifCond42
.ifCond42:
	eq r0, fp[24], 0b0000011100000000000000010000000000000000000000000000000000101101
	br r0, .ifTrue43, .ifFallthrough44
.ifTrue43:
	br .entry45
.ifFallthrough44:
	br .ifCond46
.ifCond46:
	eq r0, fp[24], 0b0000011100000000000000010000000000000000000000000000000000101010
	br r0, .ifTrue47, .ifFallthrough48
.ifTrue47:
	br .entry49
.ifFallthrough48:
	br .ifCond50
.ifCond50:
	eq r0, fp[24], 0b0000011100000000000000010000000000000000000000000000000000101111
	br r0, .ifTrue51, .ifFallthrough52
.ifTrue51:
	br .entry53
.ifFallthrough52:
	mv r0, fp[16]
	call printInt
	br .whileCond30
.entry53:
	div r0, fp[16], fp[32]
	mv fp[16], r0
	br .ifFallthrough52
.entry49:
	mul r0, fp[16], fp[32]
	mv fp[16], r0
	br .ifFallthrough48
.entry45:
	sub r0, fp[16], fp[32]
	mv fp[16], r0
	br .ifFallthrough44
.entry41:
	add r0, fp[16], fp[32]
	mv fp[16], r0
	br .ifFallthrough40
.entry37:
	br .whileFallthrough32

