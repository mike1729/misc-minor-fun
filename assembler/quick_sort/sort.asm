BITS        64
SECTION     .text
GLOBAL      sort
sort:	
  cmp  rsi, 1
  jle  done
  	push rdi
  	push rsi
  	mov  rax, rdi		
  	lea  rcx, [rdi+8]
  	lea  rdx, [rsi-1]
  	sar  rdx, 1
  	lea  rdx, [rdi+8*rdx]
  	mov  rsi, [rax]
  	mov  r8, [rdx]
  	mov  [rdx], rsi
  	mov  [rax], r8
  	mov  rsi, [rsp]
  	lea  rdx, [rdi+8*rsi-8]
  	mov  r9, [rdi]
  	while:
  		cmp rcx, rdx
  		jg  while_end
  		cmp [rcx], r9
  		jge elseif		;if([rcx]<r9) then do
  			mov  rsi, [rax]
  			mov  r8, [rcx]
  			mov  [rcx], rsi
  			mov  [rax], r8
  			mov  rsi, [rsp]
  			lea  rax, [rax+8]
  			lea  rcx, [rcx+8]
  			jmp  while
  		elseif:			;else if ([rcx]>r9) then do
  		je else
  			mov  rsi, [rdx]
  			mov  r8, [rcx]
  			mov  [rcx], rsi
  			mov  [rdx], r8
  			mov  rsi, [rsp]
  			lea  rdx, [rdx-8]
  			jmp  while
		else:  			;else [rcx]=r9 	 then do	
			lea  rcx, [rcx+8]	
  			jmp  while
  	while_end:
  	push rdx
  	mov  rsi, rax
  	sub  rsi, rdi
  	sar  rsi, 3
  	call sort
  	mov  rdx, [rsp]
  	lea  rdx, [rdx+8]
  	sub  rdx, rdi
  	sar  rdx, 3
  	mov  rsi, [rsp+8]
  	sub  rsi, rdx
  	pop  rdi
  	lea  rdi, [rdi+8]
	call sort  
 	pop  rsi
 	pop  rdi
  done:
  ret
