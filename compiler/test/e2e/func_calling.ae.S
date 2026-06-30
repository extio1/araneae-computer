[section code]
start:
	reset
	call main
	hlt

g:
.entry0:
	br .entry1
.entry1:
	br .ret2
.ret2:
	add r1, r0, 0b0000010100000000000000010000000000000000000000000000000100000000
	mv rr, r1
	ret

f:
.entry3:
	br .entry4
.entry4:
	mv fp[24], 0b0000010100000000000000010000000000000000000000000000000000000000
	br .ifCond5
.ifCond5:
	eq r2, r0, r1
	br r2, .ifTrue6, .ifFallthrough7
.ifTrue6:
	br .entry8
.ifFallthrough7:
	br .ret9
.ret9:
	push r0
	push r1
	push r2
	mv r0, fp[24]
	call g
	pop r2
	pop r1
	pop r0
	mv rr, rr
	ret
.entry8:
	add r3, fp[24], 0b0000010100000000000000010000000000000000000000000000000000001010
	mv fp[24], r3
	br .ifFallthrough7

main:
.entry10:
	br .entry11
.entry11:
	mv fp[24], 0b0000010100000000000000010000000000000000000000000000000000001010
	neg r0, 0b0000010100000000000000010000000000000000000000000000000000001010
	mv fp[32], r0
	push r0
	mv r0, fp[24]
	mv r1, fp[32]
	call f
	pop r0
	mv fp[40], rr
	br .ret12
.ret12:
	mv rr, fp[40]
	ret

