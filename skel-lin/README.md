# TEMA 1 SISTEME DE OPERARE

by Andrei Mărunțiș

## Detalii implementare

Tema consta in implementarea unei rutine care sa trateze segfault-uri care provind din programul rulat de loader.
**Nota:** Am implementat eu lucrul cu semnale, caci am rezolvat tema inainte de actualizarea scheletului
Am folosit mai multe variabile globale pentru a stoca date precum *file descriptor*-ul, vechea rutina de tratare a segfault-urilor, pagesize-ul sistemului pe care ma aflu.
Mentionez in mod special ca am pastrat si vechia rutina de tratare a segfault-urilor pentru a o apela in cazul in care primesc un segfault care nu se datoreaza memoriei nealocate a executabilului; astfel, impiedic ciclarea la infinit a programului pentru acces invalid la memorie.
Pentru a avea in vedere care pagini au fost deja alocate, stochez in pointer-ul data din structura segment un vector caracteristic care va avea valoarea 0 daca nu am alocat inca pagina sau 1 daca pagina e deja alocata. Acesti vectori sunt initializati inainte de rularea executabilului.
**Functionarea rutinei**: Caut segmentul si pagina care au generat segfault-ul. Daca ele exista in executabilul meu, le aloc memorie si scriu date in pagina alocata (daca este cazul). Daca paginile nu exista, apelez rutina veche ca sa ies din program. La final, modific permisiunile paginii nou alocate. Pentru mai multe detalii, comentariile din cod sunt foarte sigestive.

## Alte comentarii asupra temei

Tema ar putea fi imbunatatita in cateva moduri:
- verific valoarea returnata de apelurile de sistem pentru a evita situatii neprevazute; asta ar fi ajutat si la debug
- as putea sa evit anumite apeluri de sistem; spre exemplu, daca pagina alocata trebuie sa ramana umpluta cu 0, nu mai este nevoie sa mut file descriptor-ul in fisier (operatie care, oricum, nu are sens in aceasta situatie).
Lucrand la aceasta tema am invatat:
- sa lucrez cu semnale
- cum functioneaza pagefault-urile
- am exersat lucrul cu wrappere ale apelurilor de sistem
- structura fisierelor executabile