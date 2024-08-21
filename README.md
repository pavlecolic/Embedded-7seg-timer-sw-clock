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
Displej koji se koristi dostupan je na ploči, te nam nisu potrebne dodatne hardverske komponente. Potrebno je modifikovati dts fajl kako bi ploča mogla prepoznati displej, a modifikacija se vrši u skladu sa adresama specifikovanim  u soc_system.html fajlu. Polazni device tree koji modifikujemo možete pogledati [ovdje](https://github.com/etf-unibl/urs-2024/blob/lab-07-11106/19-2024/lab-07/socfpga_cyclone5_de1_soc.dts). 
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
  
Sada je moguće upaliti/ugasiti određene segmente na displejima kroz programski kod, ili testirati rad segmenata izmjenom *leds* čvora u device tree dodavanjem sljedećeg podčvora:
```
hex0_a {
			label = "hex0_a";
			gpios = <&hex0 0 0>;	
			linux,default-trigger = "none";
	};
 ```
 Nakon ove izmjene možemo da testiramo displej upisom vrijednosti 0 i 1 u odgovarajući *brightness* fajl. Atribut gpios definiše port na kom se nalazi led dioda, kao i broj pina na datom portu.
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
Sada kada smo konfigurisali device tree, potrebno je da kopiramo i .rbf fajl na FAT32 particiju, ako i da izimijenimo boot-env.txt fajl u buildroot folderu kopiranjem sadržaja boot-env.txt fajla sa repozitorijuma. Takođe, potrebno je dodati patch fajl u buildroot/board/terasic/de1soc_cyclone5 folder i navesti putanju (board/terasic/de1soc_cyclone5/de1-soc-handoff.patch) za opciju:
	*Bootloaders->Custom U-Boot Patches*
kod konfiguracije buildroot-a.
U nastavku je prikazan izvorni kod aplikacije.
```
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <sys/time.h>
#include <time.h>
#include <linux/input.h>
#include <sys/mman.h>
#include <stdlib.h>

#define LW_BRIDGE_BASE 0xFF200000
#define LW_BRIDGE_SPAN 0x00200000
#define GPIO_HEX0_BASE 0x60
#define GPIO_HEX1_BASE 0x50
#define GPIO_HEX2_BASE 0x40
#define GPIO_HEX3_BASE 0x30
#define GPIO_HEX4_BASE 0x20
#define GPIO_HEX5_BASE 0x10


unsigned char seg7[10] = {~0x3F, ~0x06, ~0x5B, ~0x4F, ~0x66, ~0x6D, ~0x7D, ~0x07, ~0x7F, ~0x6F};

// Postavljanje 7-seg displeja
void set_7seg_display(volatile unsigned int *gpio_base, unsigned char value) {
    *gpio_base = value;
}

volatile unsigned int *gpio_hex0, *gpio_hex1, *gpio_hex2, *gpio_hex3, *gpio_hex4, *gpio_hex5;

unsigned int sw0_on = 0;
unsigned int sw1_on = 0;

int mode = 0; // 0 for off, 1 for stopwatch, 2 for current time, 3 for countdown timer
int hundredths = 0;
int seconds = 0;
int minutes = 0;
int hours = 0;
int countdown_active = 0;
int stopwatch_paused = 0;
int timer_paused = 1;

void reset_timer() {
    hundredths = 0;
    seconds = 0;
    minutes = 0;
    hours = 0;
}


void display_stopwatch(int m, int s, int hs) {
    set_7seg_display(gpio_hex0, seg7[hs % 10]); // hundredths of a second units
    set_7seg_display(gpio_hex1, seg7[hs / 10]); // hundredths of a second tens
    set_7seg_display(gpio_hex2, seg7[s % 10]); // seconds units
    set_7seg_display(gpio_hex3, seg7[s / 10]); // seconds tens
    set_7seg_display(gpio_hex4, seg7[m % 10]); // minutes units
    set_7seg_display(gpio_hex5, seg7[m / 10]); // minutes tens
}

void display_current_time(int h, int m, int s) {
    set_7seg_display(gpio_hex0, seg7[s % 10]); // seconds units
    set_7seg_display(gpio_hex1, seg7[s / 10]); // seconds tens
    set_7seg_display(gpio_hex2, seg7[m % 10]); // minutes units
    set_7seg_display(gpio_hex3, seg7[m / 10]); // minutes tens
    set_7seg_display(gpio_hex4, seg7[h % 10]); // hours units
    set_7seg_display(gpio_hex5, seg7[h / 10]); // hours tens
}

void display_countdown_timer(int h, int m, int s) {
    set_7seg_display(gpio_hex0, seg7[s % 10]); 
    set_7seg_display(gpio_hex1, seg7[s / 10]); 
    set_7seg_display(gpio_hex2, seg7[m % 10]);
    set_7seg_display(gpio_hex3, seg7[m / 10]);
    set_7seg_display(gpio_hex4, seg7[h % 10]); 
    set_7seg_display(gpio_hex5, seg7[h / 10]); 
}

void timer_handler(int signum) {
    if(mode == 0) {
        reset_timer();
        display_current_time(hours, minutes, seconds);
    }
    if (mode == 1 && !stopwatch_paused) {
        // Stopwatch
        hundredths += 1;

        if (hundredths >= 100) {
            hundredths = 0;
            seconds += 1;
        }

        if (seconds >= 60) {
            seconds = 0;
            minutes += 1;
        }

        if (minutes >= 60) {
            minutes = 0;
        }

        display_stopwatch(minutes, seconds, hundredths);
    } else if (mode == 2) {
        // Trenutno vrijeme
        time_t now;
        struct tm *now_tm;
        int cur_hours, cur_minutes, cur_seconds;

        now = time(NULL);
        now_tm = localtime(&now);

        cur_hours = now_tm->tm_hour;
        cur_minutes = now_tm->tm_min;
        cur_seconds = now_tm->tm_sec;

        display_current_time(cur_hours, cur_minutes, cur_seconds);
    } else if (mode == 3 && !timer_paused) {
        // TAJMER
        if (hundredths == 0) {
            if (seconds == 0) {
                if (minutes == 0) {
                    if (hours == 0) {
                        // Timer reached zero, stop countdown
                        timer_paused = 1;
                    } else {
                        hours -= 1;
                        minutes = 59;
                        seconds = 59;
                        hundredths = 99;
                    }
                } else {
                    minutes -= 1;
                    seconds = 59;
                    hundredths = 99;
                }
            } else {
                seconds -= 1;
                hundredths = 99;
            }
        } else {
            hundredths -= 1;
        }

        display_countdown_timer(hours, minutes, seconds);
    }
}

void reset_stopwatch(int on) {
    hundredths = 0;
    seconds = 0;
    minutes = 0;
    if(on)
        stopwatch_paused = 0;
    else
        stopwatch_paused = 1;
    display_stopwatch(minutes, seconds, hundredths);
}

void change_mode() {
    if(!(sw0_on || sw1_on)) {
        mode = 0;
    } else if(sw0_on && !sw1_on) {
        mode = 1;
        reset_stopwatch(1);
    } else if(!sw0_on && sw1_on) {
        mode = 2;
    } else if(sw0_on && sw1_on) {
        mode = 3;
    }
}



void handle_event(struct input_event *ev) {
    if(ev->type == EV_KEY && ev->value == 1) {
        if(ev->code == KEY_A) {
            sw0_on = 1;
            change_mode();
        } else if(ev->code == KEY_S) {
            sw1_on = 1;
            change_mode();
        } else if (mode == 1) {
            if(ev->code == KEY_W)
                stopwatch_paused = !stopwatch_paused;
            else if(ev->code == KEY_E)
                reset_stopwatch(0);
        
        } else if(mode == 3) {
            if(timer_paused) {
                if(ev->code == KEY_W) {
                    timer_paused = 0;
                }
                else if (ev->code == KEY_T) {
                    hours = (hours + 1) % 24;
                } else if (ev->code == KEY_R) {
                    minutes = (minutes + 1) % 60;
                } else if (ev->code == KEY_E) {
                    seconds = (seconds + 1) % 60;
                }
                display_current_time(hours, minutes, seconds);
            } else {
                if(ev->code == KEY_W) {
                    timer_paused = 1;
                }
            }
        }
    } else if(ev->value == 0) {
        if(ev->code == KEY_A) {
            sw0_on = 0;
            change_mode();
        } else if(ev->code == KEY_S) {
            sw1_on = 0;
            change_mode();
        } 
    }
}

int main() {
    int fd;
    void *lw_bridge_base;
    struct itimerval timer;
    struct input_event ev;
    int ev_fd;

    if ((fd = open("/dev/mem", (O_RDWR | O_SYNC))) == -1) {
        perror("open /dev/mem");
        printf("Can't access phy");
        return 1;
    }

    lw_bridge_base = mmap(NULL, LW_BRIDGE_SPAN, (PROT_READ | PROT_WRITE), MAP_SHARED, fd, LW_BRIDGE_BASE);
    if (lw_bridge_base == MAP_FAILED) {
        perror("mmap");
        printf("Can't map HPS to FPGA");

        close(fd);
        return 1;
    }

    // GPIO adrese
    gpio_hex0 = (unsigned int *)(lw_bridge_base + GPIO_HEX0_BASE);
    gpio_hex1 = (unsigned int *)(lw_bridge_base + GPIO_HEX1_BASE);
    gpio_hex2 = (unsigned int *)(lw_bridge_base + GPIO_HEX2_BASE);
    gpio_hex3 = (unsigned int *)(lw_bridge_base + GPIO_HEX3_BASE);
    gpio_hex4 = (unsigned int *)(lw_bridge_base + GPIO_HEX4_BASE);
    gpio_hex5 = (unsigned int *)(lw_bridge_base + GPIO_HEX5_BASE);

    // Event interafece za gpio-keys-polled
    if ((ev_fd = open("/dev/input/event0", O_RDONLY)) == -1) {
        perror("open /dev/input/event0");
        if (munmap(lw_bridge_base, LW_BRIDGE_SPAN) != 0) {
            printf("Can't access event0");
            perror("munmap");
        }
        close(fd);
        return 1;
    }

    // Timer handler
    signal(SIGALRM, timer_handler);

    timer.it_value.tv_sec = 0;
    timer.it_value.tv_usec = 10000;
    timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_usec = 10000;

    // Start the timer
    if (setitimer(ITIMER_REAL, &timer, NULL) == -1) {
        perror("setitimer");
        if (munmap(lw_bridge_base, LW_BRIDGE_SPAN) != 0) {
            perror("munmap");
            printf("Can't access timer");


        }
        close(fd);
        return 1;
    }

    // Main loop
    while (1) {
        if (read(ev_fd, &ev, sizeof(struct input_event)) != -1) {
            handle_event(&ev);
        }
        pause();  // Wait for signals
    }

    if (munmap(lw_bridge_base, LW_BRIDGE_SPAN) != 0) {
        perror("munmap");
    }
    close(fd);
    close(ev_fd);
    return 0;
}
```
Koristimo *itimerval* strukturu koja nam omogućava da generišemo periodične signale, te vršimo poziv funkcije timer_handle periodično. Navedena funkcija je zadužena za prikaz brojnih vrijednosti na displejima, u zavisnosti od režima rada. Funkcija handle_event se poziva kada je detektovan događaj (pritisak na neki od tastera), a služi za obradu generisanog događaja.
## Demonstracija rada
Na sljedećoj slici prikazani su tasteri od interesa na ploči, kao i 7segmentni displej. Tasteri su numerisani zdesna nalijevo kao: KEY0, KEY1, KEY2, KEY3, SW0 i SW1.

 ![gk8e7SMI](https://github.com/user-attachments/assets/6e7ae3b2-5ac2-4148-908d-f5840560da75).
 Stanja:
1. SW1=OFF && SW2=OFF -> OFF  
2. SW1=ON && SW2=OFF -> STOPWATCH  
	2.1. KEY0 pressed -> PAUSE/CONTINUE  
	2.2. KEY1 pressed -> RESET  
3. SW1=OFF && SW3=ON -> CLOCK  
4. SW1=ON && SW2=ON -> TIMER  
   4.1. KEY0 -> PAUSE/CONTINUE     
   4.2. KEY1 -> increment seconds  
   4.3. KEY2 -> increment minutes  
   4.4. KEY3 -> increment hours  
   (inkrementi rade samo kada je tajmer pauziran)
