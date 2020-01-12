.entry LOOP
.extern L3
LOOP:	inc1 r1
	mov2 $$,r3
	bne1 L3
	mov1 $$,r3

