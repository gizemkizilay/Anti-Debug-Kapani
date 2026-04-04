Advanced Anti-Debug Trap & Dynamic Analysis Evasion (Windows x64)
1. Proje Özeti ve Amacı
Bu projenin amacı, siber güvenlik dünyasında "Evasion" (Sakınma) teknikleri üzerine uzmanlaşmış, uygulamanın kendi çalışma zamanı (runtime) ortamını analiz ederek bir hata ayıklayıcı (debugger) tarafından izlenip izlenmediğini tespit eden otonom bir koruma mekanizması geliştirmektir. Tespit anında uygulama, analizciye hiçbir uyarı vermeden (silent exit) kendi sürecini sonlandırarak tersine mühendislik faaliyetini akamete uğratır.

2. Teknik Derinlik ve Analiz Heuristikleri
Proje, standart Windows API'lerinin ötesine geçerek, işletim sistemi ve işlemci mimarisi seviyesinde derinlemesine kontroller yapar:

A. İşletim Sistemi Seviyesi Denetimler (Windows Kernel & libc)
PEB (Process Environment Block) Analizi: IsDebuggerPresent() mantığı kullanılarak, Windows çekirdeğinin sürece dair tuttuğu "BeingDebugged" bayrağı kontrol edilir.

Sessiz İnfaz (libc): stdlib.h üzerinden sağlanan exit(0) çağrısı ile uygulama sonlandırılır. Bu, tersine mühendisin hata yakalama (exception handling) mekanizmalarını kullanarak analiz yapmasını imkansız hale getirir.

B. Bellek ve Kod Bütünlüğü Denetimi (Capstone Engine)
Software Breakpoint Avı (INT 3 Tespit): Debugger'lar, kodun akışını durdurmak için makine kodunun arasına 0xCC (x86/x64 için INT 3) byte'ı enjekte eder. Uygulama, kendi .text (çalıştırılabilir kod) bölümünü Capstone motoru ile disassemble ederek araya sıkıştırılmış bu yabancı byte'ları tarar.

Dinamik Opcode Doğrulama: Capstone Engine kullanılarak kritik fonksiyonlar çalışma anında de-compile edilir. Orijinal opcode yapısındaki herhangi bir bozulma (inline hooking veya patching) anında tespit edilir.

3. Somut Çıktılar (Deliverables)
Güvenli Binary (.exe): Capstone ve Anti-Debug mekanizmalarını içeren, optimize edilmiş Windows x64 uygulaması.

Otomatik Test Senaryosu: Uygulamanın normal şartlarda (Release) sorunsuz çalıştığını, ancak bir debugger (x64dbg) altına alındığında saniyesinde kapandığını gösteren canlı test ortamı.

Gelişmiş Tehdit Modeli: Tersine mühendisin bu kontrolleri "Patching" yöntemiyle atlatmaya çalışmasına karşı geliştirilen "Silent Exit" direnç stratejisi.

4. Geliştirme Yol Haritası (Milestones)
Aşama 1: Ortam Hazırlığı ve Kütüphane Entegrasyonu
Visual Studio ve CMake derleme ortamının yapılandırılması.

Capstone Engine kütüphanesinin (libcapstone) projeye dahil edilmesi ve linklenmesi.

Aşama 2: Windows API ve PEB Kontrolleri
windows.h kütüphanesi üzerinden temel anti-debug bayraklarının kodlanması.

IsDebuggerPresent mantığının çekirdek fonksiyona entegre edilmesi.

Aşama 3: Capstone ile Bellek Taraması
Kritik fonksiyonların bellek adreslerinin tespit edilmesi.

Capstone motorunun x64 modunda başlatılarak bellekteki opcode'ların anlık analizi.

0xCC byte'ı tarama algoritmasının optimize edilmesi.

Aşama 4: Optimizasyon ve Sessiz Kapanış
Release modunda derleme yapılarak "False-Positive" (hatalı tespit) durumlarının elenmesi.

Tersine mühendislik araçlarıyla (x64dbg) stres testlerinin yapılması ve projenin teslim haline getirilmesi.
