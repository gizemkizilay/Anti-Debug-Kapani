# PROJE ÖNERİSİ: Gelişmiş Anti-Debug Kapanı ve Dinamik Analiz Savuşturma

## 1. Proje Özeti ve Amacı
Bu projenin amacı, tersine mühendislik süreçlerini zorlaştırmak amacıyla, uygulamanın kendi çalışma zamanı (runtime) ortamını analiz ederek bir hata ayıklayıcı (debugger) veya dinamik analiz aracı (örneğin GDB, strace) tarafından izlenip izlenmediğini tespit eden otonom bir koruma mekanizması geliştirmektir. Tespit anında uygulama, analizciye hiçbir uyarı vermeden (sessizce) kendi sürecini (process) sonlandıracaktır.

## 2. Teknik Derinlik ve Analiz Heuristikleri (Hocanın Beklediği Kısım)
Proje, standart Windows API'lerinin (`IsDebuggerPresent`) ötesine geçerek, işletim sistemi ve işlemci mimarisi seviyesinde derinlemesine kontroller yapacaktır. Kullanılacak temel heuristikler şunlardır:

* **İşletim Sistemi Seviyesi Heuristikler (`libc` ile):**
    * **Özel Ptrace Kontrolü:** Linux mimarisinde bir process sadece tek bir tracer'a sahip olabilir. Uygulama, `ptrace(PTRACE_TRACEME, 0, 1, 0)` çağrısı yaparak kendini trace etmeye çalışacaktır. Eğer başarısız olursa, uygulamanın halihazırda bir debugger tarafından izlendiği kanıtlanacaktır.
    * **Proses Durum Analizi:** `libc` dosya okuma fonksiyonları kullanılarak `/proc/self/status` dosyası ayrıştırılacak (parsing) ve `TracerPid` değerinin `0`'dan büyük olup olmadığı kontrol edilecektir.
* **Bellek ve Kod Bütünlüğü Heuristikleri (`capstone` ile):**
    * **Software Breakpoint Avı (INT 3 Tespit):** Debugger'lar genellikle uygulamanın duraklaması için makine kodunun arasına `0xCC` (x86/x64 için INT 3) byte'ı enjekte eder. Uygulama, kendi `.text` (çalıştırılabilir kod) bölümünü bellek üzerinden okuyacak ve araya sıkıştırılmış `0xCC` byte'larını tarayacaktır.
    * **Dinamik Disassembly ve Opcode Doğrulama:** `Capstone` motoru kullanılarak, uygulamanın kritik fonksiyonları çalışma anında de-compile edilecek ve orijinal opcode yapısında bir bozulma/değişiklik (inline hooking vb.) olup olmadığı doğrulanacaktır.
* **Zamanlama (Execution Timing) Heuristiği (Ekstra Güvenlik Katmanı):**
    * İşlemci seviyesinde `RDTSC` (Read Time-Stamp Counter) komutu kullanılarak kritik fonksiyonların çalışma süreleri ölçülecektir. Bir debugger uygulamanın çalışmasını yavaşlatacağı için, iki `RDTSC` çağrısı arasındaki aşırı zaman farkı debugger varlığına işaret edecektir.

## 3. Somut Çıktılar (Deliverables)
1.  **Güvenli Çalıştırılabilir Dosya (Binary):** Belirtilen anti-debug mekanizmalarını içeren, C/C++ ile yazılmış derlenmiş program.
2.  **Otomatik Test Betiği (Bash/Python):** Uygulamanın normal çalıştırıldığında sorunsuz sonlandığını, ancak `gdb` veya `ltrace/strace` altında çalıştırıldığında kendini anında sessizce kapattığını gösteren test senaryosu.
3.  **Tehdit Modeli ve Veri Akış Diyagramı:** Bir tersine mühendisin bu kontrolleri nasıl atlatmaya çalışacağını (örneğin NOP sledding veya byte patching) ve uygulamanın buna nasıl direneceğini gösteren mimari şema.

## 4. Geliştirme Yol Haritası (Kısa Yol Haritası)

* **Aşama 1: Çekirdek Çatı ve Bağımlılıkların Kurulumu (1. Hafta)**
    * C/C++ geliştirme ortamının hazırlanması.
    * `Capstone` kütüphanesinin (libcapstone) projeye entegrasyonu ve Makefile/CMake konfigürasyonlarının yapılması.
* **Aşama 2: OS ve Çevre Tabanlı Kontrollerin Kodlanması (2. Hafta)**
    * `libc` üzerinden `ptrace` mantığının entegre edilmesi.
    * `/proc/self/status` dosyasının okunup `TracerPid` değerinin regex/string operasyonları ile parse edilmesi.
* **Aşama 3: Capstone ile Bellek Analizi ve Breakpoint Tespiti (3. Hafta)**
    * Uygulamanın kendi bellek adres alanına (memory map) erişim sağlaması.
    * Kritik fonksiyonların bellek adreslerinin `Capstone`'a beslenerek çalışma anında assembly çıktısının alınması.
    * Zararlı byte (`0xCC`) taraması ve zamanlama (`RDTSC`) kontrollerinin eklenmesi.
* **Aşama 4: Sessiz Kapanış Mekanizması ve Testler (4. Hafta)**
    * Tespit anında uygulamanın çökmeden ve hata vermeden `_exit(0)` ile sonlandırılması.
    * GDB, x64dbg ve radare2 gibi tersine mühendislik araçlarıyla stres testlerinin yapılması.
    * Hocaya sunulacak raporun ve test betiklerinin son haline getirilmesi.

