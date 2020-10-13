init:
	mov	bx, 0xb800
	mov	es, bx
	mov	bx, 0

	mov	byte [es:bx +  0], 'K'
	mov	byte [es:bx +  2], 'e'
	mov	byte [es:bx +  4], 'r'
	mov	byte [es:bx +  6], 'n'
	mov	byte [es:bx +  8], 'e'
	mov	byte [es:bx + 10], 'l'
	mov	byte [es:bx + 12], '!'

	cli
	hlt
	jmp	init
