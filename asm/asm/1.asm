INCLUDELIB MSVCRT
.data?
mydata db 5000 dup(?)
db 500 dup(?)
db 500 dup(?)
org 75ffff00h
adata db ?

newseg SEGMENT
org 0ffff00h
nextdata db ?
newseg ends

.code
main PROC
	lea rax, mydata
	lea rbx, nextdata

	ret
main ENDP
END