# Advanced Anti-Debug Trap & Dynamic Analysis Evasion Engine (Windows x64)

Bu proje, siber güvenlik ve tersine mühendislik süreçlerini zorlaştırmak amacıyla geliştirilmiş, işletim sistemi ve işlemci mimarisi seviyesinde kontroller yapan otonom bir koruma mekanizmasıdır. Uygulama, çalışma zamanı (runtime) ortamını analiz ederek bir hata ayıklayıcı (debugger) tarafından izlendiğini tespit ettiği an, analiste hiçbir uyarı vermeden (silent exit) süreci sonlandırır.

## 🛠 Teknik Mimari ve Analiz Katmanları

Proje, standart Windows API'lerinin ötesine geçerek hibrit bir denetim mekanizması sunar:

### 1. İşletim Sistemi Seviyesi Heuristikler (Kernel32 & libc)
* **Peb-Based Detection:** `IsDebuggerPresent()` API çağrısı ile süreç blokları (Process Environment Block) üzerinden temel hata ayıklayıcı sorgulaması yapılır.
* **Silent Termination:** Tespit anında `libc` (C Standart Kütüphanesi) üzerinden `exit(0)` çağrılarak program hata vermeden sonlandırılır. Bu yöntem, tersine mühendisin "exception handling" veya "message box" üzerinden iz sürmesini engeller.

### 2. Bellek ve Kod Bütünlüğü Denetimi (Capstone Engine)
Bu katman, projenin en gelişmiş savunma hattıdır ve statik analiz yöntemlerini dinamik belleğe taşır:
* **Software Breakpoint Avı (INT 3 Detection):** Debugger'ların kod akışını durdurmak için enjekte ettiği `0xCC` (x86/64 için INT 3) byte'larını tespit eder.
* **Dinamik Disassembly:** `Capstone` disassembler motoru kullanılarak, uygulamanın kritik fonksiyonları çalışma anında de-compile edilir. 
* **Opcode Doğrulama:** Bellekteki makine kodu taranarak orijinal opcode yapısında bir bozulma veya dışarıdan müdahale (inline hooking/patching) olup olmadığı doğrulanır.

### 3. Anti-Analysis & Evasion Stratejisi
* **False-Positive Önleme:** Yazılımın "Release" konfigürasyonunda derlenmesi zorunlu kılınarak, derleyicinin (MSVC) eklediği sahte padding byte'larının (0xCC) hatalı tespit yapması engellenmiştir.
* **Stealth Mode:** Tespit mekanizması sessizce çalışır; analistin nerede yakalandığını anlamasını zorlaştırmak için hiçbir log veya hata çıktısı üretilmez.

## 🚀 Kurulum ve Derleme (Build Process)

Proje, modern C++ standartları ve CMake mimarisi ile inşa edilmiştir:

1. **Bağımlılıklar:** Capstone Engine, Windows SDK, C Standard Library (libc).
2. **Derleme Komutu:** ```bash
   cmake -DCMAKE_BUILD_TYPE=Release .
   cmake --build .
