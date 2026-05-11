# Instructiuni de Compilare si Rulare - Proiect OS Faza 2
Student: Spiridon Darius Cristian

## 1. COMPILARE
Pentru a compila ambele programe, puteti folosi Makefile-ul inclus ruland comanda:
    make

Alternativ, compilarea manuala cu gcc se face astfel:
    gcc -Wall -o p city_manager.c
    gcc -Wall -o monitor monitor_reports.c

## 2. RULARE
Pasul 1: Porniti monitorul in background (sau intr-un terminal separat) pentru a intercepta semnalele:
    ./monitor

Pasul 2: Folositi programul principal (city_manager) dintr-un alt terminal. 
Exemple de comenzi functionale:

- Pentru a adauga un raport (si a notifica automat monitorul):
    ./p --role inspector --user Alex --add centru
    
- Pentru a lista rapoartele dintr-un district:
    ./p --role inspector --user Alex --list centru
    
- Pentru a vedea un raport specific:
    ./p --role inspector --user Alex --view centru <id_raport>
    
- Pentru a sterge un district (manager only):
    ./p --role manager --user Alex --remove_district centru

Pasul 3: Pentru a opri monitorul curat, apasati Ctrl+C in terminalul in care ruleaza. Acesta va sterge automat fisierul ascuns .monitor_pid.