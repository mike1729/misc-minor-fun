BITS        64
SECTION     .text
GLOBAL      expr_eval:function

EXTERN      malloc

expr_eval:
  push  rbp
  mov   rbp, rsp
  sub   rsp, 48			;1,2
  mov   [rbp - 3*8], rcx	;3 rcx -- float const * data
  mov   [rbp - 4*8], rdx	;4 rdx -- cols
  mov   [rbp - 5*8], rsi	;5 rsi -- rows
  mov   [rbp - 6*8], rdi	;6 rdi -- struct expr * expr
  lea   rdi,    [4*rsi]
  call  malloc  WRT ..plt
  
  mov	[rbp - 1*8],    rax	; results table
  mov	qword [rbp - 2*8], 0	; loop counter

for_loop:
  mov	rax,  [rbp - 2*8]	; counter
  cmp	rax,  [rbp - 5*8]	; compare with rows
  jae   done
  	mov   rdi, [rbp - 1*8]		;results[0]
  	lea   rdi, [rdi + 4 * rax]	;results[i]
	mov   rsi, [rbp - 6*8]		;expr
	mov   rcx, [rbp - 4*8]		;cols
	mul   rcx
	mov   rdx, [rbp - 3*8]		;data[0]
	lea   rdx, [rdx + 4*rax]        ;data[i]
	call  expr_eval_one 		;WRT ..plt  	
    	inc   qword [rbp - 2*8]
    	jmp   for_loop
  done:
  mov  rax, [rbp - 1*8]
  add  rsp, 48
  pop  rbp
  ret

expr_eval_one:
  push rbp
  mov  rbp, rsp
  sub  rsp, 16
  mov  [rbp - 1*8], rdi
  mov  [rbp - 2*8], rsi
  xor  rcx, rcx
  mov  ecx, [rsi]
  lea  rax, [rel base]
  add  rax, [rax + 8*rcx]
  xor  rcx, rcx
  jmp  rax
  base:	
	dq   expr_const - base
  	dq   expr_val - base
    	dq   expr_plus - base
    	dq   expr_minus - base
    	dq   expr_sqrt - base
    	dq   expr_sin - base
    	dq   expr_cos - base
    	dq   expr_add - base
    	dq   expr_sub - base
    	dq   expr_mul - base
    	dq   expr_div - base
    	dq   expr_min - base
    	dq   expr_max - base
  expr_const: 
        mov  ecx, [rsi+8]
	mov  [rdi], ecx
	jmp  fin	
  expr_val:
	mov  cl, [rsi + 8]
	mov  ecx, [rdx + 4*rcx]
        mov  [rdi], ecx
	jmp  fin	
  expr_plus:
	mov  rsi, [rsi + 8]
	call expr_eval_one
	jmp  fin	
  expr_minus:
	mov  rsi, [rsi + 8]
	call expr_eval_one
	fld  dword [rdi]
	fchs 
	fstp dword [rdi]
	jmp  fin	
  expr_sqrt:
	mov  rsi, [rsi + 8]
	call expr_eval_one
	fld  dword [rdi]
	fsqrt
	fstp dword [rdi]
	jmp  fin	
  expr_sin:
	mov  rsi, [rsi + 8]
	call expr_eval_one
	fld  dword [rdi]
	fsin
	fstp dword [rdi]
	jmp  fin	
  expr_cos:
	mov  rsi, [rsi + 8]
	call expr_eval_one
	fld  dword [rdi]
	fcos
	fstp dword [rdi]
	jmp  fin	
  expr_add:
	mov  rsi, [rsi + 8]
	call expr_eval_one
	fld  dword [rdi]
	mov  rsi, [rbp - 2*8]
	mov  rsi, [rsi + 16]
	call expr_eval_one
	fadd dword [rdi]
	fstp dword [rdi]
	jmp  fin	
  expr_sub:
	mov  rsi, [rsi + 8]
	call expr_eval_one
	fld  dword [rdi]
	mov  rsi, [rbp - 2*8]
	mov  rsi, [rsi + 16]
	call expr_eval_one
	fsub dword [rdi]
	fstp dword [rdi]
	jmp  fin	
  expr_mul:
	mov  rsi, [rsi + 8]
	call expr_eval_one
	fld  dword [rdi]
	mov  rsi, [rbp - 2*8]
	mov  rsi, [rsi + 16]
	call expr_eval_one
	fmul dword [rdi]
	fstp dword [rdi]
	jmp  fin	
  expr_div:
	mov  rsi, [rsi + 8]
	call expr_eval_one
	fld  dword [rdi]
	mov  rsi, [rbp - 2*8]
	mov  rsi, [rsi + 16]
	call expr_eval_one
	fdiv dword [rdi]
	fstp dword [rdi]
	jmp  fin	
  expr_min:
	mov  rsi, [rsi + 8]
	call expr_eval_one
	fld  dword [rdi]
	mov  rsi, [rbp - 2*8]
	mov  rsi, [rsi + 16]
	call expr_eval_one
	fld  dword [rdi]
	fcomi
	fstp dword [rdi]
	fstp dword [rbp - 2*8]
	jbe  fin
	xor  rax, rax
	mov  eax, [rbp - 2*8]
	mov  [rdi], eax
	jmp  fin
  expr_max:
	mov  rsi, [rsi + 8]
	call expr_eval_one
	fld  dword [rdi]
	mov  rsi, [rbp - 2*8]
	mov  rsi, [rsi + 16]
	call expr_eval_one
	fld  dword [rdi]
	fcomi
	fstp dword [rdi]
	fstp dword [rbp - 2*8]
	jae  fin
	xor  rax, rax
	mov  eax, [rbp - 2*8]
	mov  [rdi], eax
fin:
  add rsp, 16
  pop rbp
  ret
