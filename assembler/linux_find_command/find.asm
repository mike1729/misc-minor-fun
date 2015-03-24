BITS        64
SECTION		.data
  ELOOP	      equ  -40  
  ENOENT      equ  -2   
  ENOTDIR     equ  -20  
  buf_size    equ  512
  O_RDONLY    equ  0
  O_NOFOLLOW  equ  131072
  msg:	      db   "find: `"
  len	      equ  $ - msg
  msgENOENT:  db   "': No such file or directory", 10
  lenENOENT   equ  $ - msgENOENT
  msgENOTDIR: db   "': Not a directory", 10
  lenENOTDIR  equ  $ - msgENOTDIR
  
SECTION 	.bss
path:		resb 4096

SECTION		.text
;-------------------------------------------------------------------------------
;				_start
;-------------------------------------------------------------------------------	
GLOBAL      	_start
_start:
  mov  rcx, [rsp]
  cmp  rcx, 2
  jne  done
  
  mov  rax, 2
  mov  rdi, [rsp+16]
  mov  rsi, O_NOFOLLOW
  syscall 		;open file or dir
  
  cmp   rax, ELOOP 	;file is a symlink
  je    done
  cmp   rax, ENOENT	;file does not exist
  je    case.ENOENT
  cmp   rax, ENOTDIR	;file is not a directory
  je    case.ENOTDIR
  
  mov   rdi, rax
  mov   rax, 3 
  syscall 		;close fd
  mov   rdi, [rsp+16]
  mov   rsi, 0
  call  process_path  	;do magic
  jmp   done
  
case.ENOENT:
  mov   r8, msgENOENT
  mov   r9, lenENOENT
  call  write_message
  jmp   done
case.ENOTDIR:
  mov   r8, msgENOTDIR
  mov   r9, lenENOTDIR
  call  write_message
  jmp   done
done:
  mov   eax, 60
  mov   rdi, 2
  mov   rsi, 0
  syscall 
;-------------------------------------------------------------------------------
;				process_path
;-------------------------------------------------------------------------------	
process_path:     	
  sub    rsp, 528
  push   path		;remember path end
  push   rsi		;pointer to string file/dir name
  push   rdi		;offset 
  
;count size of file/dir name
  mov    rax, 0
  mov    rcx, -1
  mov    rdx, rdi
  repnz  scasb
  add    rcx, 2
  neg    rcx  		;size of file/dir name
  
;copy name to path and update offset
  mov	 rdi, path
  add    rdi, [rsp+8]	;move pointer to the offset
  add    [rsp+8], rcx	;update offset
  mov    rsi, [rsp]
  rep    movsb  	;copy from rsi to rdi (update path with file/dir name)
  mov    rcx, [rsp+8]
  add    [rsp+16], rcx  ;update path end
  
;print path
  mov    [rdi], byte 10
  mov	 [rdi+1], byte 0
  mov    rax, 1
  mov    rdi, 1
  mov	 rsi, path
  mov    rdx, [rsp+8]
  add    rdx, 1
  syscall		;print path
  
  
;open file
  mov    rax, 2
  mov    rdi, [rsp+16]
  mov    [rdi], byte 0
  mov    rdi, path
  mov    rsi, O_NOFOLLOW
  syscall 		;open file or dir
  
;append '/' to the end of path
  mov    rcx, [rsp+16]
  cmp    [rcx-1], byte '/'
  je     after_append
  mov    [rcx], byte '/'
  inc    qword [rsp+8]
after_append:

;remember file descriptor
  mov    [rsp+16], rax
  
do_while:
;syscall getdents
  mov    rdi, [rsp+16]  ;file descriptor
  mov    rax, 78    	;syscall getdents number
  lea    rsi, [rsp+40]
  mov    rdx, buf_size
  syscall		;call getdents

;process dir entries
  mov    [rsp+24], rax	;num_read
  mov    qword [rsp+32], 0;bpos
  inner_for:
  mov	 rax,  [rsp+24]
  cmp    [rsp+32], rax
  jge 	 inner_for_end
  	mov  rax, [rsp+32]
	lea  rdi, [rsp + 40 + rax] 	
	mov  r8w, [rdi + 16]		;read d_reclen
	add  [rsp+32], r8w		;update bpos with d_reclen
	
	add  rdi,  18			;points to name of file or directory
	cmp  [rdi], byte '.'
	jne  proc_path
	cmp  [rdi+1], byte 0		;dirname == "."
	je   inner_for
	cmp  [rdi+1], byte '.'
	jne  proc_path
	cmp  [rdi+2], byte 0		;dirname == ".."
	je   inner_for
	
	proc_path:
	mov  rsi, [rsp+8]
	call process_path
		  	
  	jmp inner_for
  inner_for_end:
  cmp    qword [rsp+24],  0
  jg	 do_while
  
; close file descriptor
  mov    rax, 3
  mov    rdi, [rsp+16]
  syscall		;close file descriptor
    
  add    rsp, 552
  ret  
  
;-------------------------------------------------------------------------------
;				write_message
;-------------------------------------------------------------------------------	
write_message:
  mov   rax, 1
  mov   rdi, 1
  mov   rsi, msg
  mov   rdx, len
  syscall 		;write msg
  mov   al, 0
  mov   rcx, -1
  mov   rdi, [rsp+24]
  repnz scasb
  mov   rdx, -2
  sub   rdx, rcx
  mov   rax, 1
  mov   rdi, 1
  mov   rsi, [rsp+24]
  syscall		;write path
  mov   rax, 1
  mov   rdi, 1
  mov   rsi, r8
  mov   rdx, r9
  syscall 		;write msg
  ret
