[section code]
readChar__0:
  ; SyncReceive: MakeRef(TYPE_CHAR=0x07, F_GLOBAL=0x0008, addr=0xD008)
  ; Reading from 0xD008 blocks until 1 byte received; st stores it on heap and returns a global ref
  usp 8
  mv fp[8], 0x070008000000D008
  st rr, fp[8]
  ret

writeChar__1:
  ; r0 holds reference to char symbol to put to stdout
  usp 16
  
  mv fp[8], 0x060008000000D100    ; reference to stdout outbox buffer
  cp fp[8], r0                    ; copy char value from r0 to buffer
  
	; OutboxLength = 1
  mv fp[8], 0x060008000000D01C
  ldi r0, 1
  cp fp[8], r0
  
  ; OutboxOffset = 0
  mv fp[8], 0x060008000000D018
  ldi r0, 0
  cp fp[8], r0
  
  ; Sending = 1
  mv fp[8], 0x060008000000D03C
  ldi r0, 1
  cp fp[8], r0

	; SyncSend = 1
  mv fp[8], 0x060008000000D000
  ldi r0, 1
  cp fp[8], r0
  
  ret

readOp__0:
	usp 8
.entry0:
	jmp .entry1
.entry1:
	call readChar__0
	mv fp[8], rr
	jmp .whileCond2
.whileCond2:
	eq r0, fp[8], 0b0000011100000000000000010000000000000000000000000000000000100000
	eq r1, fp[8], 0b0000011100000000000000010000000000000000000000000000000000001101
	lor r2, r0, r1
	eq r0, fp[8], 0b0000011100000000000000010000000000000000000000000000000000001010
	lor r1, r2, r0
	br r1, .whileBody3, .whileFallthrough4
.whileBody3:
	jmp .entry5
.entry5:
	call readChar__0
	mv fp[8], rr
	jmp .whileCond2
.whileFallthrough4:
	jmp .ret6
.ret6:
	mv rr, fp[8]
	ret

readInt__0:
	usp 16
.entry7:
	jmp .entry8
.entry8:
	mv fp[8], 0b0000010000000000000000010000000000000000000000000000000000000000
	call readChar__0
	mv fp[16], rr
	jmp .whileCond9
.whileCond9:
	eq r0, fp[16], 0b0000011100000000000000010000000000000000000000000000000000100000
	eq r1, fp[16], 0b0000011100000000000000010000000000000000000000000000000000001101
	lor r2, r0, r1
	eq r0, fp[16], 0b0000011100000000000000010000000000000000000000000000000000001010
	lor r1, r2, r0
	br r1, .whileBody10, .whileFallthrough11
.whileBody10:
	jmp .entry12
.entry12:
	call readChar__0
	mv fp[16], rr
	jmp .whileCond9
.whileFallthrough11:
	jmp .whileCond13
.whileCond13:
	neq r0, fp[16], 0b0000011100000000000000010000000000000000000000000000000000001101
	neq r1, fp[16], 0b0000011100000000000000010000000000000000000000000000000000001010
	land r2, r0, r1
	br r2, .whileBody14, .whileFallthrough15
.whileBody14:
	jmp .entry16
.entry16:
	mul r0, fp[8], 0b0000010000000000000000010000000000000000000000000000000000001010
	sub r1, fp[16], 0b0000010000000000000000010000000000000000000000000000000000110000
	add r2, r0, r1
	mv fp[8], r2
	call readChar__0
	mv fp[16], rr
	jmp .whileCond13
.whileFallthrough15:
	jmp .ret17
.ret17:
	mv rr, fp[8]
	ret

printUInt__1:
	usp 24
.entry18:
	jmp .entry19
.entry19:
	mv fp[8], 0b0000010000000000000000010000000000000000000000000000000000000001
	mv fp[16], r0
	jmp .whileCond20
.whileCond20:
	gte r1, fp[16], 0b0000010000000000000000010000000000000000000000000000000000001010
	br r1, .whileBody21, .whileFallthrough22
.whileBody21:
	jmp .entry23
.entry23:
	mul r1, fp[8], 0b0000010000000000000000010000000000000000000000000000000000001010
	mv fp[8], r1
	div r1, fp[16], 0b0000010000000000000000010000000000000000000000000000000000001010
	mv fp[16], r1
	jmp .whileCond20
.whileFallthrough22:
	jmp .whileCond24
.whileCond24:
	gt r1, fp[8], 0b0000010000000000000000010000000000000000000000000000000000000000
	br r1, .whileBody25, .whileFallthrough26
.whileBody25:
	jmp .entry27
.entry27:
	div r1, r0, fp[8]
	add r2, r1, 0b0000010000000000000000010000000000000000000000000000000000110000
	mv fp[24], r2
	push r0
	mv r0, fp[24]
	call writeChar__1
	pop r0
	mod r1, r0, fp[8]
	mv r0, r1
	div r1, fp[8], 0b0000010000000000000000010000000000000000000000000000000000001010
	mv fp[8], r1
	jmp .whileCond24
.whileFallthrough26:
	jmp .ret28
.ret28:
	mv rr, rr
	ret

printInt__1:
	usp 0
.entry29:
	jmp .entry30
.entry30:
	call __pic_disable_queueing__0
	call __pic_disable_interrupts__0
	jmp .ifCond31
.ifCond31:
	lt r1, r0, 0b0000010000000000000000010000000000000000000000000000000000000000
	br r1, .ifTrue32, .ifFallthrough33
.ifTrue32:
	jmp .entry34
.entry34:
	push r0
	st r0, 0b0000011100000000000000010000000000000000000000000000000000101101
	call writeChar__1
	pop r0
	bnot r1, r0
	add r2, r1, 0b0000010000000000000000010000000000000000000000000000000000000001
	mv r0, r2
	jmp .ifFallthrough33
.ifFallthrough33:
	push r0
	mv r0, r0
	call printUInt__1
	pop r0
	push r0
	st r0, 0b0000011100000000000000010000000000000000000000000000000000001101
	call writeChar__1
	pop r0
	push r0
	st r0, 0b0000011100000000000000010000000000000000000000000000000000001010
	call writeChar__1
	pop r0
	jmp .ret35
.ret35:
	mv rr, rr
	call __pic_enable_queueing__0
	call __pic_enable_interrupts__0
	ret
