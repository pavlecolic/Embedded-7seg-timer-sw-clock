# Informacije o projektu
Ovaj repozitorijum sadrži rješenje projektnog zadatka iz predmeta Ugrađeni računarski sistemi.
## Tekst projektnog zadatka
Realizovati aplikaciju kojom se realizuje funkcionalnost mjerenja vremena, štoperice i tajmera na DE1-SoC ploči. Informacije o vremenu se prikazuju na 7-segmentnim displejima, a način rada se kontroliše pomoću tastera dostupnih na ploči.
## Preduslovi za izradu projekta
Za izradu projektnog zadatka potrebne su sljedeće stavke:
- [Altera DE1-SoC](https://www.terasic.com.tw/cgi-bin/page/archive.pl?Language=English&No=836) razvojna ploča sa pripadajućim napajanjem i kablom za povezivanje razvojnog računara sa UART interfejsom na ploči
- SD kartica (minimalno 4GB)
- pravilno konfigurisano Buildroot okruženje
- pripremljen toolchain za kroskompajliranje softvera sa razvojne na ciljnu platformu
Obzirom da se projektni zadatak nadovezuje na [laboratorijske vježbe](https://github.com/etf-unibl/urs-2024), sve stavke koje prethode izradi projekta detaljno su opisane u tom repozitorijumu i neće biti obrađene ovdje. Dakle, finalna konfiguracija sa laboratorijskih vježbi se uzima kao polazna u izradi projektnog zadatka. 
## Struktura repozitorijuma
Ovdje je ukratko opisana struktura foldera u repozitorijumu, a u nastavku će svrha svakog fajla biti pojašnjena detaljnije.
- u *code* folderu nalazi se fajl sa izvornim kodom
- u *rbf* folderu nalzi se fajl koji je potrebno kopirati na FAT32 particiju na kartici
- u folderu *buildroot* nalaze se fajlovi relevantni za Buildroot okruženje, kao što su konfiguracioni fajlovi, device tree fajl, patch fajl.
- u folderu *scripts* nalaze se korisne skripte koje nam olakšavaju podešavanje okruženja 
## Tesitranje rada 7-segmentnog displeja
Displej koji se koristi dostupan je na ploči, te nam nisu potrebne dodatne hardverske komponente. Potrebno je modifikovati dts fajl kako bi ploča mogla prepoznati displej, a modifikacija se vrši u skladu sa adresama specifikovanim  u soc_system.html fajlu. Polazni device tree koji modifikujemo možete pogledati [ovdje](https://github.com/etf-unibl/urs-2024/blob/lab-07-11106/19-2024/lab-07/socfpga_cyclone5_de1_soc.dts). 
Dodajemo [GPIO binding](https://github.com/altera-opensource/linux-socfpga/blob/socfpga-6.1.20-lts/Documentation/devicetree/bindings/gpio/gpio-altera.txt) za svaki 7-segmentni displej u dts fajl, ukupno šest čvorova. Primjer kontrolera za hex0 displej:
```
hex0: gpio@ff200060 {
		compatible = "altr,pio-1.0";
		reg = <0xff200060 0x10>;
		altr,ngpio = <7>;
		#gpio-cells = <2>;
		gpio-controller;
	};
 ```
Širina od 7 bita nam omogućava da kontorlišemo svaki segment. Naravno, prije testiranja potrebno je uvjeriti se da je u konfiguraciji kernela omogućena podrška za GPIO tastere (Device Drivers→Input device support→Keyboards→GPIO Buttons). Sada je moguće upaliti/ugasiti određene segmente na displejima kroz programski kod, ili testirati rad segmenata izmjenom *leds* čvora u device tree
```
hex0_a {
			label = "hex0_a";
			gpios = <&hex0 0 0>;	
			linux,default-trigger = "none";
	};
 ```
 , nakon čega možemo da testiramo displej upisom odgovarajućih vrijednosti u *brightness* fajl. Atribut gpios definiše port na kom se nalazi led dioda (npr. hex0), kao i broj pina na datom portu.
 ## Rad sa dugmićima i svičevima
 Rad sa dugmićima i svičevima ćemo omogućiti pomoću polling mehanizma. Za razliku od interrupt-driven pristupa, kod pollinga se periodično provjerava stanje uređaja. Potrebno je omogućiti opciju Device Drivers→Input device support→Keyboards→Polled GPIO Buttons. Atribut poll-interval predstavlja interval provjere stanja uređaja u milisekundama. Omogućićemo četiri dugmića, a za promjenu režima rada dovoljna su nam dva sviča (4 moguća stanja).
 ‚‚‚
 	gpio-keys-polled {
		compatible = "gpio-keys-polled";
		poll-interval = <10>;

		key0 {
			label = "key0";
			gpios = <&buttons 0 0>;	
			linux,code = <17>;	
		};
		
		key1 {
			label = "key1";
			gpios = <&buttons 1 0>;	
			linux,code = <18>;	
		};
		
		key2 {
			label = "key2";
			gpios = <&buttons 2 0>;
			linux,code = <19>;
		};
		
		key3 {
			label = "key3";
			gpios = <&buttons 3 0>;	
			linux,code = <20>;
		};

		sw0 {
			label = "sw0";
			gpios = <&switches 0 0>;	
			linux,code = <30>;
		};

		sw1 {
			label = "sw1";
			gpios = <&switches 1 0>;	
			linux,code = <31>;
		};
	};
 ‚‚‚
Kompajliranjem dts fajla komandom *make dtbs* dobićemo socfpga_cyclone5_de1_soc.dtb fajl koji je potrebno prebaciti na FAT32 particiju na kartici.
 
