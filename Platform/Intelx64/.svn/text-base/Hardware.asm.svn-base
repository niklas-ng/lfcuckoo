	.586p
_TEXT	segment dword public use32 'CODE'
_TEXT	ends
_DATA	segment dword public use32 'DATA'
_DATA	ends
_TEXT	segment dword public use32 'CODE'
	assume	cs:_TEXT,ds:_DATA,es:_DATA
	align 4

; __cdecl versions 32-bit primitives
_atomicswap     proc    near
	mov             ecx,dword ptr ss:[esp+04H]
	mov		eax,dword ptr ss:[esp+08H]
	lock    xchg	eax,[ecx+0H]
	ret
_atomicswap     endp

_testandset	proc	near
	mov             eax,dword ptr ss:[esp+04H]
	lock	bts dword ptr [eax],0H
	setnc           al
	movzx           eax,al
	ret
_testandset	endp

_compareandswap proc near
	mov             ecx,dword ptr ss:[esp+04H]
	mov             eax,dword ptr ss:[esp+08H]
	mov             edx,dword ptr ss:[esp+0CH]
	lock    cmpxchg	dword ptr [ecx],edx
	sete		al
	movzx		eax,al
	ret
_compareandswap endp

_compareandswap_o    proc    near
	mov             ecx,dword ptr ss:[esp+04H]
	mov             eax,dword ptr ss:[esp+08H]
	mov             edx,dword ptr ss:[esp+0CH]
	lock	cmpxchg	dword ptr [ecx],edx
	ret
_compareandswap_o    endp

_fetchandadd	proc	near
	mov             ecx,dword ptr ss:[esp+04H]
	mov             eax,dword ptr ss:[esp+08H]
	lock	add dword ptr [ecx],eax
	ret
_fetchandadd	endp

; __fastcall versions 32-bit primitives
@atomicswap@8     proc    near
	mov		eax,edx
	lock    xchg	eax,[ecx+0H]
	ret
@atomicswap@8     endp
@testandset@4	proc	near
	lock	bts dword ptr [ecx],0H
	setnc           al
	movzx           eax,al
	ret
@testandset@4	endp
@compareandswap@12 proc near
	mov		eax,edx
	mov		edx,dword ptr ss:[esp+04H]
	lock    cmpxchg	dword ptr [ecx],edx
	sete	al
	movzx	eax,al
	ret		4
@compareandswap@12 endp
@compareandswap_o@12    proc    near
	mov		eax,edx
	mov		edx,dword ptr ss:[esp+04H]
	lock    cmpxchg	dword ptr [ecx],edx
    ret		4
@compareandswap_o@12    endp
@fetchandadd@8	proc	near
	lock	add dword ptr [ecx],edx
	ret
@fetchandadd@8	endp


; __cdecl version 64-bit primitives
atomicswap64    proc    near
	push		ebx
	push		esi

        mov             esi,dword ptr ss:[esp+0CH]
        mov             ebx,dword ptr ss:[esp+010H]
        mov             ecx,dword ptr ss:[esp+014H]

        mov             eax,dword ptr [esi+00H]
        mov             edx,dword ptr [esi+04H]
as64L2:	
        lock    cmpxchg8b   qword ptr [esi+00H]
        jne             as64L2
	pop		esi
	pop		ebx
        ret
atomicswap64    endp

compareandswap64    proc    near
	push		ebx
	push		esi
        mov             esi,dword ptr ss:[esp+0CH]
        mov             eax,dword ptr ss:[esp+010H]
        mov             edx,dword ptr ss:[esp+014H]
        mov             ebx,dword ptr ss:[esp+018H]
        mov             ecx,dword ptr ss:[esp+01CH]
        lock    cmpxchg8b   qword ptr [esi+00H]
	sete		al
	movzx		eax,al
	pop		esi
	pop		ebx
        ret
compareandswap64    endp

compareandswap64_o  proc    near
	push		ebx
	push		esi
        mov             esi,dword ptr ss:[esp+0CH]
        mov             eax,dword ptr ss:[esp+010H]
        mov             edx,dword ptr ss:[esp+014H]
        mov             ebx,dword ptr ss:[esp+018H]
        mov             ecx,dword ptr ss:[esp+01CH]
        lock    cmpxchg8b   qword ptr [esi+00H]
	pop		esi
	pop		ebx
        ret
compareandswap64_o  endp

testandset64    proc    near
	push		ebx
	push		esi
        mov             esi,dword ptr ss:[esp+0CH]
        mov            eax,0
        mov            edx,0
	mov		ebx,1
	mov		ecx,0
        lock    cmpxchg8b   qword ptr [esi+00H]
	sete		al
	movzx		eax,al
	pop		esi
	pop		ebx
	ret
testandset64    endp

fetchandadd64   proc    near
	push		ebx
	push		esi
        mov             esi,dword ptr ss:[esp+0CH]

        mov             eax,dword ptr [esi+00H]
        mov             edx,dword ptr [esi+04H]
fa64L5:	
        mov             ebx,dword ptr ss:[esp+010H]
        mov             ecx,dword ptr ss:[esp+014H]
	add		ebx,eax
	add		ecx,edx
        lock    cmpxchg8b   qword ptr [esi+00H]
        jne             fa64L5
	pop		esi
	pop 	        ebx
        ret
fetchandadd64   endp


_TEXT	ends
_DATA	segment dword public use32 'DATA'
_DATA	ends
_s@	equ	s@
	public  _atomicswap
	public	_testandset
	public	_compareandswap
	public  _compareandswap_o
	public  _fetchandadd
	public  @atomicswap@8
	public	@testandset@4
	public	@compareandswap@12
	public  @compareandswap_o@12
	public  @fetchandadd@8
	end

