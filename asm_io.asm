;bibliothèque pour assembleur
;ne gère pas un flot d'entrée supérieur à 1000 caractères
;asm_io.inc
%define taille_mot 4 ; 4 en 32 bits, 8 en 64 bits

%define taille_entree_buff 1000 ; tampon qui contiendra les caractères
                                ; lus depuis l'entrée standard

%macro ajout_tampon 3
    add %2,%1
    cmp %2,fin_buff-1
    jne suite_%3
    mov %2,entree_buff
    suite_%3:
%endmacro

segment .data
    int_format:      db  "%d", 0 ;format nombre printf
    caractere: db 0,0
    debut_flot: dd 0
    fin_flot: dd 0
segment .bss
    entier: resd 1  ;place pour un entier lue
    str_buff: resd 40 ;place pour une chaine
    entree_buff: resd taille_entree_buff ;tampon
    fin_buff:

segment .text

;*****************************************
global read_int
read_int:
    push ebx
    push ecx
    push edx
    mov ecx,entree_buff
    mov edx,ecx
    add edx,[fin_flot]
    add ecx,[debut_flot]
; sauter caractère blanc tampon jusqu'a non blanc ou vide
loop:
    cmp ecx,edx
    jne  suite
    call tampon
    mov edx,entree_buff
    add edx,[fin_flot]
    jmp loop
suite:
    cmp byte [ecx],10 ; caratère saut de ligne
    je  suivant
    cmp byte [ecx],' '
    je  suivant
    cmp byte [ecx],9 ; caractère tabulation
    je  suivant
    jmp lecture_nombre
suivant:
    ajout_tampon 1,ecx,1_read_int
    jmp loop
; si non blanc saut, si vide remplir tampon remcommencer
lecture_nombre:
;si tiret 1 dans ebx sinon 0
    mov bl,0
    cmp byte [ecx],'-'
    jne suite2
    mov bl,1
    ajout_tampon 1,ecx,2_read_int
suite2:
;lire nombre suivant
    push ecx
    push entier
    call convertir_str_int; #à changer ,le nombre peut continuer sur le début du tampon
    add esp,2*taille_mot
;deplacer flot
    ajout_tampon eax,ecx,3_read_int
    sub ecx,entree_buff
    mov [debut_flot],ecx
    mov eax,[entier]
;inverser ou non
    cmp bl,0
    je suite3
    neg eax
suite3:
;retour
    pop edx
    pop ecx
    pop ebx
    ret

;*****************************************
tampon:
;Convention C
;Demande une saisie et vide l'entrée standard
;dans le tampon.
;le début du flot est à l'adresse *debut_flot+entree_tampon
;la fin du flot à l'adresse fin_flot (égale à debut flot si flot est vide)
    push eax
    push ebx
    push ecx
    push edx
    mov ecx,entree_buff
    add ecx,[fin_flot]
loop_tampon:
    mov eax,3
    mov edx,1
    mov ebx,0
    int 0x80
    mov dl,[ecx]
    ajout_tampon 1,ecx,1_tampon
    cmp dl,10 ;10 : saut de ligne ASCII
    jne loop_tampon
    sub ecx,entree_buff
    mov [fin_flot],ecx
    pop edx
    pop ecx
    pop ebx
fin_tampon:
    pop eax
    ret

;*****************************************
sprint_int:
;convention C. Écrit en base 10 le nombre.(positif négatif ou nul)
;renvoie le nombre de caractère écrit
;arg1 nombre_a ecrire, arg2 place où écrire.
    push ebp
    mov ebp,esp
    push ebx
    mov eax,0
    mov ebx,[ebp+3*taille_mot]
    cmp dword [ebp+2*taille_mot],0
    jl negatif_sprint_int
    jg positif_sprint_int
;zero
    mov eax,1
    mov byte [ebx],'0'
    inc ebx
    jmp fin_sprint_int
negatif_sprint_int:
    neg dword [ebp+2*taille_mot]
    mov byte [ebx],'-'
    inc ebx
positif_sprint_int:
    push ebx
    push dword [ebp+2*taille_mot]
    call esprint_int
    add esp,2*taille_mot
    add ebx,eax
fin_sprint_int:
    mov byte [ebx],0
    pop ebx
    pop ebp
    ret


;*****************************************
esprint_int:
;convention C.
;Écrit en base 10 le nombre positif strictement à écrire.
;renvoie le nombre de caractère écrit.
;arg1 nombre_a ecrire, arg2 place où écrire.
    push ebp
    mov ebp,esp
    mov eax,0
    cmp dword [ebp+2*taille_mot],0
    je fin_esprint_int
; si arg1 neq 0
;sauvegarde registre
    push ebx
    push edx
    mov edx,0
    mov ebx,10
    mov eax,[ebp+2*taille_mot]
    div ebx
    push dword [ebp+3*taille_mot]
    push eax
    call esprint_int
    add esp,2*taille_mot
    add edx,'0'
    mov ebx,[ebp+3*taille_mot]
    add ebx,eax
    mov [ebx],dl
    pop edx
    pop ebx
    inc eax
fin_esprint_int:
    pop ebp
    ret


;*****************************************
convertir_str_int:
;convention C.
;Convertie la chaine en entier positif ou nul.
;Retourne l'entier positif.
;Retourne le nombre de caractère lue en cas d'erreur
;arg1 endroit où écrire (4 byte), arg2 endroit où lire
    push ebp
    mov ebp,esp
    push ebx ; adresse où lire
    push ecx ; nouveau caractère
    mov eax,0 ;eax nombre à ajouter
    mov ebx,[ebp+3*taille_mot]
loop_convertir_str_int:
    cmp byte [ebx],'0'
    jl fin_convertir_str_int
    cmp byte [ebx],'9'
    jg fin_convertir_str_int
    mov edx,0
    mov ecx,10
    mul ecx
    mov ecx,0
    mov cl,[ebx]
    sub cl,'0'
    add eax,ecx
    inc ebx
    jmp loop_convertir_str_int
fin_convertir_str_int:
    mov ecx,[ebp+2*taille_mot]
    mov [ecx],eax
    mov eax,ebx
    sub eax,[ebp+3*taille_mot]
    pop ecx
    pop ebx
    pop ebp
    ret


;*****************************************
global print_string
print_string:
    push ebx
    push ecx
    push edx
    push eax
    call taille_chaine
    mov edx,eax
    mov ecx,[esp]
    mov eax,4
    mov ebx,1
    int 0x80
    pop eax
    pop edx
    pop ecx
    pop ebx
    ret

;*****************************************
print_char:
;affiche un caractère contenue dans al
    mov [caractere],al
    push eax
    mov eax,caractere
    call print_string
    pop eax
    ret


;*****************************************
global print_espace
print_espace:
    push eax
    mov eax,32
    call print_char
    pop eax
    ret

;*****************************************
global print_nl
print_nl:
    push eax
    mov eax,10
    call print_char
    pop eax
    ret

;*****************************************
taille_chaine:
;convention C. Retourne la taille de la chaine.
;argument : adresse de la chaine
;le caractère 0 (fin de chaine) ne compte pas
    push ebp
    mov ebp,esp
    mov eax,[ebp+taille_mot*2]
    dec eax
loop_tc:
    inc eax
    cmp byte [eax],0
    jne loop_tc
    sub eax,[ebp+taille_mot*2]
    pop ebp
    ret

;*****************************************
global print_int
print_int:
    push eax
    push str_buff
    push eax
    call sprint_int
    add esp,2*taille_mot
    mov eax,str_buff
    call print_string
    pop eax
    ret
