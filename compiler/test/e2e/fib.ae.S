[section code]
start:
	reset
	call main
	hlt

fib:
	usp 0
.entry0:
	br .entry1
.entry1:
	br .ifCond2
.ifCond2:
	lt r0, r1, r0
	br r0, .ifTrue3, .ifFallthrough4
.ifTrue3:
	br .entry5
.ifFallthrough4:
	br .ret7
.ret7:
	mv rr, r3
	ret
.entry5:
	br .ret6
.ret6:
	add r0, r1, 0b0000010100000000000000010000000000000000000000000000000000000001
	add r1, r2, rr
	push r0
	push r1
	mv r0, r0
	mv r1, fp[8]
	mv r2, fp[16]
	mv r3, r2
	call fib
	pop r1
	pop r0
	mv rr, rr
	ret

main:
	usp 8
.entry8:
	br .entry9
.entry9:
	st r0, 0b0000010100000000000000010000000000000000000000000000000000110010
	st r1, 0b0000010100000000000000010000000000000000000000000000000000000000
	st r2, 0b0000010100000000000000010000000000000000000000000000000000000001
	st r3, 0b0000010100000000000000010000000000000000000000000000000000000000
	call fib
	mv fp[8], rr
	br .ret10
.ret10:
	mv rr, fp[8]
	ret

