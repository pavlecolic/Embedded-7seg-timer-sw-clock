# Informacije o projektu
Ovaj repozitorijum sadrži rješenje projektnog zadatka iz predmeta Ugrađeni računarski sistemi.
## Tekst projektnog zadatka
Realizovati aplikaciju kojom se realizuje funkcionalnost mjerenja vremena, štoperice i tajmera na DE1-SoC ploči. Informacije o vremenu se prikazuju na 7-segmentnim displejima, a način rada se kontroliše pomoću tastera dostupnih na ploči.
## Preduslovi za izradu projekta
Za izradu projektnog zadatka potrebne su sljedeće stavke:
- [Altera DE1-SoC](https://www.terasic.com.tw/cgi-bin/page/archive.pl?Language=English&No=836) razvojna ploča sa pripadajućim napajanjem i kablom za povezivanje razvojnog računara sa UART interfejsom na ploči
- SD kartica (minimalno 4GB)
- pravilno konfigurisano Buildroot okruženje
- pripremljen toolchain za kroskompajliranje softvera sa razvojne na ciljnu platformu.

Obzirom da se projektni zadatak nadovezuje na [laboratorijske vježbe](https://github.com/etf-unibl/urs-2024), sve stavke koje prethode izradi projekta detaljno su opisane u tom repozitorijumu i neće detaljno biti obrađene ovdje. Dakle, finalna konfiguracija sa laboratorijskih vježbi se uzima kao polazna u izradi projektnog zadatka. 
## Struktura repozitorijuma
Ovdje je ukratko opisana struktura foldera u repozitorijumu, a u nastavku će svrha svakog fajla biti pojašnjena detaljnije.
- u *code* folderu nalazi se fajl sa izvornim kodom
- u *rbf* folderu nalzi se fajl koji je potrebno kopirati na FAT32 particiju na kartici
- u folderu *buildroot* nalaze se fajlovi relevantni za Buildroot okruženje, kao što su konfiguracioni fajlovi, device tree fajl, patch fajl.
- u folderu *scripts* nalaze se korisne skripte koje nam olakšavaju podešavanje okruženja 
## Tesitranje rada 7-segmentnog displeja
Prije svega, potrebno je da kopiramo i .rbf fajl na FAT32 particiju, ako i da izimijenimo boot-env.txt fajl u buildroot folderu kopiranjem sadržaja boot-env.txt fajla sa repozitorijuma. Takođe, potrebno je dodati patch fajl u buildroot/board/terasic/de1soc_cyclone5 folder i navesti putanju (board/terasic/de1soc_cyclone5/de1-soc-handoff.patch) za opciju:
	*Bootloaders->Custom U-Boot Patches*
kod konfiguracije buildroot-a.  
Displej koji se koristi dostupan je na ploči, te nam nisu potrebne dodatne hardverske komponente. Potrebno je modifikovati dts fajl kako bi ploča mogla prepoznati displej, a modifikacija se vrši u skladu sa adresama specifikovanim  u *soc_system.html* fajlu. Polazni device tree koji modifikujemo možete pogledati [ovdje](https://github.com/etf-unibl/urs-2024/blob/lab-07-11106/19-2024/lab-07/socfpga_cyclone5_de1_soc.dts). 
Putanja fajla od interesa je: linux-socfpga/arch/arm/boot/dts/socfpga_cyclone5_de1_soc.dts  
Ukoliko fajl ne postoji potrebno ga je kreirati.
Dodajemo [GPIO binding](https://github.com/altera-opensource/linux-socfpga/blob/socfpga-6.1.20-lts/Documentation/devicetree/bindings/gpio/gpio-altera.txt) za svaki 7-segmentni displej u dts fajl. Kako imamo dostupno šest 7segmentnih displeja, imaćemo ukupno šest čvorova. Primjer kontrolera za jedan displej:
```
hex0: gpio@ff200060 {
		compatible = "altr,pio-1.0";
		reg = <0xff200060 0x10>;
		altr,ngpio = <7>;
		#gpio-cells = <2>;
		gpio-controller;
	};
 ```
Širina od 7 bita za svaki displej nam omogućava da kontorlišemo segmente. Prije testiranja potrebno je uvjeriti se da je u konfiguraciji kernela omogućena podrška za GPIO tastere:  
  
&nbsp;&nbsp;&nbsp;&nbsp;*Device Drivers→Input device support→Keyboards→GPIO Buttons*  

Pozicionidamo se u linux-socfpga folder i pokrenemo sljedeću komandu za kompajliranje *device tree-a*:
```
make dtbs
```
Ova komanda će da kreira .dtb fajl istog naziva koji je potrebno kopirati na FAT32 particiju.
Sada je moguće upaliti/ugasiti određene segmente na displejima kroz programski kod, ili testirati rad segmenata izmjenom *leds* čvora u device tree dodavanjem sljedećeg podčvora:
```
hex0_a {
			label = "hex0_a";
			gpios = <&hex0 0 0>;	
			linux,default-trigger = "none";
	};
 ```

 Nakon ove izmjene i ponovnog kompajliranja možemo da testiramo displej upisom vrijednosti 0 i 1 u odgovarajući *brightness* fajl. Atribut gpios definiše port na kom se nalazi led dioda, kao i broj pina na datom portu.
 ## Rad sa dugmićima i svičevima
 Rad sa dugmićima i svičevima ćemo omogućiti pomoću polling mehanizma. Za razliku od interrupt-driven pristupa, kod pollinga se periodično provjerava stanje uređaja. Potrebno je omogućiti opciju:  
   
&nbsp;&nbsp;&nbsp;&nbsp;*Device Drivers→Input device support→Keyboards→Polled GPIO Buttons*  
  
Atribut *poll-interval* predstavlja interval provjere stanja uređaja u milisekundama. Imamo četiri dugmića, a za promjenu režima rada dovoljna su nam dva sviča za četiri moguća stanja (off, stopwatch, clock, timer).
 ```
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
 ```
Svakom tasteru dodijeljen je određeni [kod](https://github.com/torvalds/linux/blob/master/include/uapi/linux/input-event-codes.h).  Kompajliranjem dts fajla komandom *make dtbs* dobićemo socfpga_cyclone5_de1_soc.dtb fajl koji je potrebno prebaciti na FAT32 particiju na kartici. Komandom evtest možemo da vidimo kreirani event fajl, te da testiramo rad naših tastera uz informacije o vremenu, stanju i kodu. Detaljnije informacije možete pronaći na [ovom linku](https://www.kernel.org/doc/Documentation/input/input.txt) u sekciji 5 Event Interface. Strukturu *input_event* ćemo koristiti pri implementaciji aplikacije za dobijanje informacija o pritisnutom tasteru.
```
# Testiranje pomoću evtest alata
# Pritisak na taster KEY0 (dodijeljen mu je code 17, koji odgovara slovu W)
Event: time 1706154740.024961, type 1 (EV_KEY), code 17 (KEY_W), value 0
Event: time 1706154740.024961, -------------- SYN_REPORT ------------
Event: time 1706154740.224967, type 1 (EV_KEY), code 17 (KEY_W), value 1
Event: time 1706154740.224967, -------------- SYN_REPORT ------------
```
## Realizacija aplikacije
Sada kada smo konfigurisali device tree možemo da realizujemo našu aplikaciju.
Koristimo *itimerval* strukturu koja nam omogućava da generišemo periodične signale, te vršimo poziv funkcije timer_handle periodično. Navedena funkcija je zadužena za prikaz brojnih vrijednosti na displejima, u zavisnosti od režima rada i trenutnih vrijednosti. Funkcija handle_event se poziva kada je detektovan događaj (pritisak na neki od tastera), a služi za obradu generisanog događaja.
Funkciji prosljeđujemo input_event strukturu koja nam daje potrebne informacije o događaju.
```
struct input_event {
	struct timeval time;
	unsigned short type;
	unsigned short code;
	unsigned int value;
};
```
Poređenjem vrijednosti *code* atributa iz strukture sa vrijednostima specifikovanim u devie tree fajlu zaključujemo koji taster je pritisnut te vršimo promjenu stanja u skladu s tim.  
Kompletan izvorni kod sa komentarima nalazi se u folderu code u repozitorijumu.
Izvorni kod potrebno je kroskompajlirati za ciljanu platformu korišćenjem ranije generisanog toolcahina-a, sljedećom komandom:
```
arm-linux-gcc 7seg.c -o 7seg
```
> **Note:** Prije izvršavanja komande potrebno je izvršiti skriptu set-environment.sh (folder scripts) za otvoreni terminal.
> ```
> source .set-environment.sh

Nakon toga, možemo kopirati izvršni fajl na particiju 2 koja sadrži *root* fajl sistem. 
## Demonstracija rada
Povezaćemo ploču sa razvojnim računarom i izvršiti komandu:
```
sudo picocom -b 115200 /dev/ttyUSB0
```
Na sljedećoj slici prikazani su tasteri od interesa na ploči, kao i 7segmentni displej. Tasteri su numerisani zdesna nalijevo kao: KEY0, KEY1, KEY2, KEY3, SW0 i SW1.
Možemo da se pozicioniramo u direktorijum gdje smo smjestili izvršni fajl i pokrenuti našu aplikaciju.
 ![gk8e7SMI](https://github.com/user-attachments/assets/6e7ae3b2-5ac2-4148-908d-f5840560da75)  
Stanja:
1. SW0=OFF && SW1=OFF -> OFF  
2. SW0=ON && SW1=OFF -> STOPWATCH  
	2.1. KEY0 pressed -> PAUSE/CONTINUE  
	2.2. KEY1 pressed -> RESET  
3. SW0=OFF && SW1=ON -> CLOCK  
4. SW0=ON && SW1=ON -> TIMER  
   4.1. KEY0 -> PAUSE/CONTINUE     
   4.2. KEY1 -> increment seconds  
   4.3. KEY2 -> increment minutes  
   4.4. KEY3 -> increment hours  
   (inkrementi rade samo kada je tajmer pauziran)
