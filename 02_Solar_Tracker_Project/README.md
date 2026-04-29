<!-- Proje Günlüğü -->
# Milestone 1: Requirement Analysis (Scope & Peripherals)

1. **Project Scope (Proje Kapsamı)**  
    Bu proje, güneş panellerinin verimliliğini artırmak amacıyla çift eksenli (azimut ve yükseklik) bir takip sistemi tasarlamayı kapsar. Sistem:
    a. 4 adet LDR sensörü aracılığıyla ışık yoğunluğunu takip edecektir.
    b. STM32F410RB mikrodenetleyicisi üzerinde FreeRTOS işletim sistemi kullanarak gerçek zamanlı       çalışacaktır.
    c. Panelden elde edilen anlık güç verilerini (voltaj/akım) INA219 üzerinden okuyacaktır.
    d. Verileri ESP8266 üzerinden bulut tabanlı bir veritabanına (ThingSpeak) aktaracaktır.

2. **Objectives (Hedefler)**    
    Maksimum Verimlilik: Güneş panelini her zaman ışığa dik açıyla (90°) tutarak enerji kazanımını maksimize etmek.

    Gürültü Azaltma: LDR verilerindeki çevresel gürültüyü Predictive Filtering (Öngörülü Filtreleme) ile minimize ederek servoların gereksiz titremesini (jitter) önlemek.

    Güç İzleme: Sistemin kendi tükettiği enerji ile panelden üretilen enerjiyi karşılaştırmak için hassas ölçüm yapmak.

    Modülerlik: Yazılımı, donanım bağımlılığını minimize eden bir katmanlı mimari (HAL/LL) ile geliştirmek.

3. **Constraints (Kısıtlamalar & Sınırlar)**  
    Donanım Sınırı: STM32 Nucleo board'un sunduğu akım limitleri dahilinde kalınmalıdır. Servolar yüksek akım çektiği için harici besleme kullanılacaktır (Common Ground unutulmadan).

    Gerçek Zamanlılık: Motor kontrol döngüsü, sistemin stabilitesi için en yüksek önceliğe sahip olmalı ve gecikme <20ms olmalıdır.

    Enerji Bütçesi: Sistemin kontrol ve haberleşme için harcadığı enerji, güneş takibi sayesinde kazanılan ek enerjiden düşük olmalıdır.

    Bağlantı: WiFi üzerinden veri gönderimi sırasında oluşabilecek ağ kopmalarında sistemin kilitlenmemesi (non-blocking) sağlanmalıdır.

4. **STM32 Peripheral Selection (Donanım Konfigürasyonu)**  

Hangi pinleri kullanacağını netleştirmen lazım.
   - ADC: LDR okumaları için (ADC1 IN0, IN1, IN4, IN8).
   - I2C: INA219 güç sensörü için (I2C1 SDA/SCL).
   - PWM: 2 adet servo motor için (TIM2 / TIM3 PWM).
   - UART: ESP8266 veri aktarımı için (USART1 TX/RX).
   - UART: Debug için (USART2 VCP).
   - GPIO: Status LED / Heartbeat için.

| Peripheral | Purpose | Connection Detail |
| --- | --- | --- |
| ADC1 | LDR Sensing | 4 Channels (IN0, IN1, IN4, IN8) - 12-bit Resolution |
| I2C1 | Power Monitor | INA219 Communication (SDA/SCL) |
| TIM2 / TIM3 | Servo Control | 2x PWM Outputs (50Hz Signal) |
| USART1 | WiFi Communication | ESP8266 AT Commands (TX/RX) |
| USART2 | Debugging | PC Terminal (VCP üzerinden log takibi) |
| GPIO | Status LEDs | System Heartbeat (LD2 - PA5 hariç, senin düzeltmene göre!) |

# Milestone 2: System Design (UML & Task Prioritization)
**Görev:** Diyagramları .png olarak kaydet ve PlantUML klasörüne at. RTOS Önceliklendirmesini yap.
1. Control Task (High): Motorların hızlı tepki vermesi için.
2. Sensing Task (Medium): Veri okuma.
3. Telemetry Task (Low): Buluta veri gönderme (İnternet yavaştır, sistemi bekletmesin).

# Milestone 3: Prototype Development (Drivers)
**Görev:** STM32CubeIDE ile bir proje oluşturup .ioc dosyasında bu çevre birimlerini (I2C, ADC, PWM) aktif etmelisin.