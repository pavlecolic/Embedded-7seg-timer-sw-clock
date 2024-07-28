# Informacije o projektu
Ovaj repozitorijum sadrži rješenje projektnog zadatka iz predmeta Ugrađeni računarski sistemi.
## Tekst projektnog zadatka
Realizovati aplikaciju kojom se realizuje funkcionalnost mjerenja vremena, štoperice i tajmera na DE1-SoC ploči. Informacije o vremenu se prikazuju na 7-segmentnim displejima, a način rada se kontroliše pomoću tastera dostupnih na ploči.
## Preduslovi za izradu projekta
Za izradu projektnog zadatka potrebne su sljedeće stavke:
- [Altera DE1-SoC](https://www.terasic.com.tw/cgi-bin/page/archive.pl?Language=English&No=836) razvojna ploča sa pripadajućim napajanjem i kablom za povezivanje razvojnog računara sa UART interfejsom na ploči.
- SD kartica (minimalno 4GB)
- pravilno konfigurisano Buildroot okruženje
- pripremljen toolchain za kroskompajliranje softvera sa razvojne na ciljnu platformu
Imajući u vidu da se projektni zadatak nadovezuje na [laboratorijske vježbe](https://github.com/etf-unibl/urs-2024), sve stavke koje prethode izradi projekta detaljno su opisane u tom repozitorijumu i neće biti obrađene ovdje. Dakle, finalna konfiguracija sa laboratorijskih vježbi se uzima kao polazna u izradi projektnog zadatka. 
## Struktura repozitorijuma
Ovdje je ukratko opisana struktura foldera u repozitorijumu, a u nastavku će svrha svakog fajla biti pojašnjena detaljnije.
- u *code* folderu nalazi se fajl sa izvornim kodom
- u *rbf* folderu nalzi se fajl koji je potrebno kopirati na FAT32 particiju na kartici
- u folderu *buildroot* nalaze se fajlovi relevantni za Buildroot okruženje, kao što su konfiguracioni fajlovi, device tree fajl, patch fajl.
- u folderu *scripts* nalaze se korisne skripte kojen am olakšavaju podešavanje okruženja
## 
## Tesitranje rada 7-segmentnog displeja
Displej koji se koristi dostupan je na ploči, te nam nisu potrebne dodatne hardverske komponente. Potrebno je modifikovati dts fajl kako bi ploča mogla prepoznati displej kao dostupan uređaj.
## 
