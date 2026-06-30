[section code]
start:
	reset
	call main
	hlt

main:
  ldi r0, 123
	call printInt__1
	ret

