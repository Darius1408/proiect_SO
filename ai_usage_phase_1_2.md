# Utilizare AI - Proiect OS Faza 1 si 2

**Instrument folosit:** ChatGPT / Gemini
**Data:** 10 Mai 2026

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