;.MODEL FLAT, STDCALL
option prologue:none ; turn off default prologue creation
option epilogue:none ; turn off default epilogue creation

_text SEGMENT

option prologue:none ; turn off default prologue creation
option epilogue:none ; turn off default epilogue creation

; in vs2010, right click on Project / Build Customization, enable MASM
; add an .asm file to the project
; you should see a MASM category under Project / Properties
; you should see "compile" under right click on the .asm file.

getRSP PROC
	mov rax, rsp
	add rax, 8
	ret
getRSP ENDP

getRBP PROC
	mov rax, rbp
	ret
getRBP ENDP

; http://msdn.microsoft.com/en-us/library/ms235286.aspx
; The x64 Application Binary Interface (ABI) is a 4 register fast-call calling convention,
; with stack-backing for those registers. There is a strict one-to-one correspondence between
; arguments in a function, and the registers for those arguments. Any argument that doesn’t fit
; in 8 bytes, or is not 1, 2, 4, or 8 bytes, must be passed by reference.
; The arguments are passed in registers RCX, RDX, R8, and R9
setRSP PROC
	mov rsp, RCX
	add rsp, 2
	ret
setRSP ENDP

setRBP PROC
	mov rbp, RCX
	ret
setRBP ENDP

setRSPRBP PROC
	mov rsp, RCX
	mov rbp, RDX
	ret
setRSPRBP ENDP


; ending!
_text ENDS
END

