# Utilizare AI - Proiect OS Faza 1, 2 și 3

**Instrument folosit:** ChatGPT / Gemini
**Data ultimei actualizări:** 18 Mai 2026

## Faza 1

### 1. Generarea funcției parse_condition
**Prompt:** "Am o structură de înregistrare în C pentru un proiect. Te rog să îmi generezi o funcție `int parse_condition(const char *input, char *field, char *op, char *value);` care primește un string de forma `field:operator:value` și îl împarte în cele 3 componente."
**Ce a generat AI-ul:** AI-ul a generat o funcție bazată pe `sscanf` cu regex-ul simplificat `%[^:]` pentru a sparge stringul la caracterul două puncte (`:`).
**Ce am schimbat și de ce:** Am păstrat implementarea aproape intactă, dar m-am asigurat că returnează 1 dacă a găsit exact 3 argumente (succes) și 0 în caz de eșec, pentru a o putea folosi ușor în instrucțiunile `if` din codul meu.

### 2. Generarea funcției match_condition
**Prompt:** "Structura mea este: `typedef struct { ... } Report;` Generează o funcție `int match_condition(...)` care returnează 1 dacă înregistrarea îndeplinește condiția și 0 altfel. Suportă operatorii ==, !=, <, <=, >, >= pentru fields severity și strings (inspector, category)."
**Ce a generat AI-ul:** O funcție cu multe if/else-uri care folosește `strcmp` pentru a identifica câmpul, `atoi` pentru a converti valoarea, și din nou `strcmp` pentru a evalua operatorii.
**Ce am schimbat și de ce:** Codul generat acoperea prea multe cazuri inutile, așa că l-am simplificat pentru a se potrivi strict cu câmpurile testate.
**Ce am învățat:** Am învățat cum se pot scrie parsere simple de argumente din linia de comandă folosind formatarea din `sscanf`.

---

## Faza 2

### 3. Trecerea de la signal() la sigaction()
* **Prompt:** „Cum pot intercepta semnalele SIGINT și SIGUSR1 într-un program C care rulează continuu, având în vedere că cerința interzice explicit folosirea funcției vechi `signal()`?”
* **Ce a generat AI-ul:** Un exemplu de utilizare a structurii `sigaction`, explicând rolul câmpurilor `sa_handler`, `sa_flags` și funcția `sigemptyset()`.
* **Ce am învățat și aplicat:** Am înțeles că `sigaction` este standardul POSIX modern. De asemenea, am învățat să folosesc `SA_RESTART` pentru ca apelurile de sistem întrerupte de semnale să repornească automat.

### 4. Comunicarea Inter-Procese (IPC) și funcția kill()
* **Prompt:** „Am două programe C diferite. Unul stă în așteptare cu `pause()`. Cum fac ca primul program să îl 'trezească' și să îi trimită o notificare (SIGUSR1) fix după ce am scris ceva într-un fișier?”
* **Ce a generat AI-ul:** Explicații despre funcția `kill(pid, semnal)` și logica de a salva PID-ul monitorului într-un fișier temporar (`.monitor_pid`).
* **Ce am învățat și aplicat:** Am descoperit că numele funcției `kill()` este înșelător – ea poate trimite orice semnal, nu doar să omoare procese. Am integrat logica folosind `open` și `read` pentru a prelua PID-ul și a trimite notificarea.

### 5. Explicarea și validarea codului pentru Daemon (monitor_reports)
* **Prompt-ul meu:** „e ok tot: `#include <stdio.h> ... volatile sig_atomic_t keep_running = 1; ...` [am atașat tot codul pentru monitor_reports]. explică-mi fisierul simplu cu comentarii pe cod ca să înțeleg.”
* **Ce a răspuns AI-ul:** Mi-a confirmat că programul este "Async-Signal-Safe" (pentru că folosește `write` în loc de `printf` în handlere) și mi-a rescris codul adăugând comentarii logice pentru fiecare bloc, explicând analogia cu un „paznic care se pune la somn” folosind `pause()`.
* **Ce am învățat:** Am înțeles exact de ce `while(keep_running) { pause(); }` este mult mai eficient decât o verificare continuă (0% CPU) și de ce variabila de control trebuie să fie de tipul `volatile sig_atomic_t` pentru a nu se bloca/corupe atunci când este modificată asincron de o întrerupere (semnal).

### 6. Identificarea funcționalităților lipsă din specificații (Dangling Symlinks & lstat)
* **Prompt-ul meu:** „proiectul meu are implementate toate specificațiile fazelor 1 si 2? ți-am atașat si documentul cu specificațiile ca să verifici” (împreună cu fișierul `city_manager.c`).
* **Ce a răspuns AI-ul:** A analizat codul comparativ cu documentul PDF și a descoperit că uitasem implementarea comenzii `--view` și cerința de a folosi `lstat()` pentru a detecta "dangling links" (link-uri simbolice stricate). Mi-a generat funcția `check_dangling_symlinks()`.
* **Ce am învățat și integrat:** Am adăugat `#include <dirent.h>` și am pus funcția de verificare la începutul lui `main()`. Cel mai important, am înțeles diferența crucială din Linux dintre `stat()` (care urmărește destinația link-ului) și `lstat()` (care citește proprietățile link-ului în sine), lucru esențial pentru a nu primi crash-uri atunci când fișierul original `.dat` a fost șters de comanda `remove_district`.

---

## Faza 3

### 7. Multiplexarea ierarhică de procese și conducte de date (Pipe)
* **Prompt:** "Cum configurez o ierarhie în C unde programul principal `city_hub` deschide un pipe, face fork pentru un proces copil `hub_mon`, iar acesta face la rândul lui fork pentru un nepot care execută `monitor`? Vreau ca procesul copil să citească prin pipe tot ce scrie nepotul."
* **Ce a generat AI-ul:** Explicații teoretice despre apelul `pipe(pipe_fd)` și un șablon de cod bazat pe clonarea dublă (`fork()`), arătând cum descriptorii de citire `pipe_fd[0]` și scriere `pipe_fd[1]` trebuie distribuiți între procese.
* **Ce am învățat și aplicat:** Am înțeles cum se coordonează o structură arborescentă de procese (Părinte -> Copil -> Nepot). Cel mai important aspect aplicat a fost regula de aur a pipe-urilor Unix: închiderea timpurie a capetelor neutilizate în procesele care nu le folosesc. Am înțeles că dacă procesul părinte sau copil nu închid capătul de scriere, funcția `read()` blocantă nu va detecta niciodată sfârșitul fluxului (EOF) și va bloca programul la infinit (deadlock).

### 8. Redirecționarea descriptorilor standard folosind dup2()
* **Prompt:** "Cum folosesc funcția `dup2()` pentru a redirecționa ieșirea standard a unui proces copil rulat prin `execlp` în interiorul conductei (`pipe`), astfel încât părintele să poată captura datele text emise de executabilul extern `scorer`?"
* **Ce a generat AI-ul:** Un exemplu de cod în care, chiar înainte de apelul de tip `exec`, procesul copil execută comanda `dup2(pipe_fd[1], STDOUT_FILENO)` și apoi închide descriptorii redundanți ai conductei.
* **Ce am învățat și aplicat:** Am implementat cu succes această tehnică în comanda `calculate_scores`. Am înțeles că prin `dup2()`, „priza” standard de ieșire (descriptorul `1`) este mutată direct pe canalul de scriere al pipe-ului. Astfel, când `scorer` rulează și folosește un `printf` clasic, el nu știe că ecranul a fost decuplat, iar textul este deturnat direct către `city_hub`.

### 9. Depanarea unui bug critic de precedență a operatorilor (Debugging)
* **Prompt:** "Codul meu pentru citirea din pipe compilează fără nicio eroare sau warning, dar la execuție se blochează sau afișează date complet corupte în interiorul buclei: `while((bytes_read = read(pipe_fd[0], buffer, sizeof(buffer) - 1) > 0))`. De ce?"
* **Ce a răspuns AI-ul:** A identificat o eroare clasică și extrem de subtilă de limbaj C legată de prioritatea operatorilor. Operatorul relațional `>` are o precedență mai mare decât operatorul de atribuire `=`. Prin urmare, codul meu efectua mai întâi compararea `read(...) > 0` (rezultând valoarea booleană 1) și apoi asigura acea valoare de `1` variabilei `bytes_read`.
* **Ce am schimbat și de ce:** Am corectat manual linia de cod prin izolarea corectă cu paranteze rotunde: `while((bytes_read = read(pipe_fd[0], buffer, sizeof(buffer) - 1)) > 0)`. Această corecție a permis salvarea dimensiunii reale a octeților citiți în variabila `bytes_read`, rezolvând bug-ul de afișare.

### 10. Protocolul de mesaje cu tag-uri pentru fluxuri continue (Byte Streams)
* **Prompt:** "Deoarece un pipe POSIX transmite datele sub formă de flux continuu de octeți fără delimitări clare de structură, cum pot face ca procesul cititor să distingă între tipurile de mesaje scrise la stdout de către monitor?"
* **Ce a generat AI-ul:** Ideea de a implementa un protocol simplu de mesaje bazat pe prefixe fixe text (Tag-uri): `MSG:INFO|`, `MSG:NOTIFY|` și `MSG:ERR|`, urmate de caracterul separator `\n`.
* **Ce am învățat:** Am înțeles diferența majoră dintre citirea înregistrărilor binare de dimensiuni fixe din Faza 1 (unde știam exact că citim blocuri de dimensiunea `sizeof(Report)`) și parsarea unui flux de text asincron cu lungimi variabile. Totodată, am însușit importanța utilizării tipului de date POSIX `ssize_t` (signed size) în loc de `size_t` (unsigned) pentru funcțiile de I/O, deoarece avem nevoie ca variabila să poată stoca valoarea negativă `-1` în cazul în care o conductă se rupe sau apare o eroare de sistem.