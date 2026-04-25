# Utilizare AI - Proiect OS Faza 1

**Instrument folosit:** ChatGPT / Gemini
**Data:** 25 Aprilie 2026

## 1. Generarea funcției parse_condition
**Prompt:** "Am o structură de înregistrare în C pentru un proiect. Te rog să îmi generezi o funcție `int parse_condition(const char *input, char *field, char *op, char *value);` care primește un string de forma `field:operator:value` și îl împarte în cele 3 componente."

**Ce a generat AI-ul:** AI-ul a generat o funcție bazată pe `sscanf` cu regex-ul simplificat `%[^:]` pentru a sparge stringul la caracterul două puncte (`:`).

**Ce am schimbat și de ce:** Am păstrat implementarea aproape intactă, dar m-am asigurat că returnează 1 dacă a găsit exact 3 argumente (succes) și 0 în caz de eșec, pentru a o putea folosi ușor în instrucțiunile `if` din codul meu.

## 2. Generarea funcției match_condition
**Prompt:**
"Structura mea este: `typedef struct { int id; char inspector[32]; float latitude; float longitude; char category[32]; int severity; time_t timestamp; char description[256]; } Report;`
Generează o funcție `int match_condition(Report *r, const char *field, const char *op, const char *value);` care returnează 1 dacă înregistrarea îndeplinește condiția și 0 altfel. Suportă operatorii ==, !=, <, <=, >, >= pentru fields severity și strings (inspector, category)."

**Ce a generat AI-ul:**
O funcție cu multe if/else-uri care folosește `strcmp` pentru a identifica câmpul, `atoi` pentru a converti valoarea pentru severity, și din nou `strcmp` pentru a evalua operatorii.

**Ce am schimbat și de ce:**
Codul generat acoperea prea multe cazuri inutile, așa că l-am simplificat pentru a se potrivi strict cu câmpurile testate (severity, category, inspector). M-am asigurat că se face conversia corectă `atoi(value)` doar când se compară câmpul de tip int (`severity`), lăsând restul comparațiilor ca stringuri.

**Ce am învățat:**
Am învățat cum se pot scrie parsere simple de argumente din linia de comandă folosind formatarea din `sscanf`, fără a fi nevoie de tokenizare complexă cu `strtok`.