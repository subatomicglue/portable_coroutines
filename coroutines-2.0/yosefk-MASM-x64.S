global getRSP
section .text
getRSP:
	mov rax, rsp
	add rax, 8
	ret

global getRBP
section .text
getRBP:
	mov rax, rbp
	ret

; http://msdn.microsoft.com/en-us/library/ms235286.aspx
; The x64 Application Binary Interface (ABI) is a 4 register fast-call calling convention,
; with stack-backing for those registers. There is a strict one-to-one correspondence between
; arguments in a function, and the registers for those arguments. Any argument that doesn’t fit
; in 8 bytes, or is not 1, 2, 4, or 8 bytes, must be passed by reference.
; The arguments are passed in registers RCX, RDX, R8, and R9
global setRSP
section .text
setRSP:
	mov rsp, RCX
	ret

global setRBP
section .text
setRBP:
	mov rbp, RCX
	ret