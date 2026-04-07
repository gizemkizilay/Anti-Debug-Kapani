# Conventional Commits Rehberi — Anti-Debug Kapanı

Bu belgede projeye eklenen her modül için Conventional Commits formatında
hazır `git commit` komutları ve gerekçe açıklamaları listelenmiştir.

---

## Format

```
<type>(<scope>): <kısa açıklama>

<neden yapıldığını açıklayan 1-2 cümle>
```

| Alan | Açıklama |
|------|----------|
| `type` | `feat`, `fix`, `refactor`, `test`, `ci`, `docs`, `chore` |
| `scope` | Değişikliğin ilgili olduğu modül veya klasör |
| `kısa açıklama` | İmperatif kipte, küçük harfle, nokta koymadan |

---

## 1 — TimingDetector Modülü

```
feat(detectors): add RDTSC and QPC based timing anomaly detector

Debugger varlığı CPU döngü sayısı ve duvar-saati gecikmesini anormal
ölçüde artırdığından, iki bağımsız ölçüm yöntemiyle (RDTSC delta ve
QueryPerformanceCounter) zamanlama anomalisi tespit edilebilmektedir.
```

**Kopyala-yapıştır:**
```bash
git commit -m "feat(detectors): add RDTSC and QPC based timing anomaly detector

Debugger varligi CPU dongü sayisini ve duvar-saati gecikmesini anormal
olcude artirdigindan, RDTSC delta ve QueryPerformanceCounter ile iki
bagimsiz yontemle zamanlama anomalisi tespit edilmektedir."
```

---

## 2 — HandleDetector Modülü

```
feat(detectors): add invalid-handle and debug-port detector

Gecersiz bir handle degerine (0xDEAD) yapilan CloseHandle cagrisi
STATUS_INVALID_HANDLE uretir; debugger bu exception'i yutar ve
GetLastError beklenen degeri dondurmez. NtQueryInformationProcess
araciligiyla debug port sorgusu ikinci bir dogrulama katmani saglar.
```

**Kopyala-yapıştır:**
```bash
git commit -m "feat(detectors): add invalid-handle and debug-port detector

Gecersiz handle (0xDEAD) ile CloseHandle cagrisi STATUS_INVALID_HANDLE
uretir; debugger bu exception'i yutar. NtQueryInformationProcess ile
debug port sorgusu ikinci bir dogrulama katmani saglar."
```

---

## 3 — HeapFlagDetector Modülü

```
feat(detectors): add PEB NtGlobalFlag and heap-header flag checker

Windows, bir sureci debugger altinda baslattiginda PEB->NtGlobalFlag
alanina 0x20 (HEAP_TAIL_CHECKING_ENABLED) ve 0x40
(HEAP_FREE_CHECKING_ENABLED) bitlerini atar. Bu bitlerin varligini
hem NtGlobalFlag hem de heap header Flags/ForceFlags alanlari uzerinden
kontrol ederek debugger barindiran ortamlar tespit edilmektedir.
```

**Kopyala-yapıştır:**
```bash
git commit -m "feat(detectors): add PEB NtGlobalFlag and heap-header flag checker

Windows debugger altinda PEB->NtGlobalFlag alanina 0x20 ve 0x40
bitlerini atar. NtGlobalFlag ve heap header Flags/ForceFlags alanlari
kontrol edilerek debugger barindiran ortamlar tespit edilir."
```

---

## 4 — MemoryEraser Modülü

```
feat(evasion): add XOR string obfuscation, secure wipe and PE header erasure

Statik analiz ve bellek dumplarini zorlastirmak icin uc onlem alinir:
XORSTR makrosu ile string literalleri derleme zamaninda XOR ile
sifrelenip runtime'da cozulur; ZeroSensitiveMemory SecureZeroMemory
kullanarak hassas verileri silinmez bicimde siler; ErasePEHeader
yuklü PE basligini bellekten silerek adli analizi engeller.
```

**Kopyala-yapıştır:**
```bash
git commit -m "feat(evasion): add XOR string obfuscation, secure wipe and PE header erasure

XORSTR makrosu string literalleri derleme zamaninda sifreler.
ZeroSensitiveMemory hassas verileri SecureZeroMemory ile siler.
ErasePEHeader bellekteki PE basligini silerek adli analizi engeller."
```

---

## 5 — Logger Utility

```
feat(utils): add conditional logger with debug-only file output

Gunluk kayitlarinin production binary'lerde iz birakmamasi icin
Logger sincap DEBUG preprocessor sembolune baglidir: DEBUG aktifken
olaylar zaman damgasiyla debug_log.txt dosyasina yazilir; Release
modda LOG_EVENT makrosu tamamen bos bir ifadeye derlenip sifir
ek yuk birakmaz.
```

**Kopyala-yapıştır:**
```bash
git commit -m "feat(utils): add conditional logger with debug-only file output

DEBUG aktifken olaylar zaman damgasiyla debug_log.txt dosyasina
yazilir. Release modda LOG_EVENT makrosu bos ifadeye derlenerek
production binary'de sifir ek yuk birakilir."
```

---

## 6 — tests/test_detectors.cpp

```
test(detectors): add self-test suite for all detector and evasion modules

Debugger bulunmayan normal bir Release ortaminda her detector'un
false dondurmesi ve her yardimci fonksiyonun dogru davranmasi
check edilir. 7 test fonksiyonu main() tarafindan sirasyla cagrilir
ve sonuclar stdout'a yazilir; cikis kodu 0 (basari) / 1 (hata).
```

**Kopyala-yapıştır:**
```bash
git commit -m "test(detectors): add self-test suite for all detector and evasion modules

Normal Release ortaminda her detector false dondurmeli, her yardimci
fonksiyon dogru davranmalidir. 7 test fonksiyonu main() icerisinde
calistirilip cikis kodu 0/1 ile sonuclara gore rapor edilir."
```

---

## 7 — CMakeLists.txt Güncellemesi

```
build(cmake): add target_sources, per-config compiler flags and CTest integration

Kaynak dosyalari target_sources() ile eklenerek derleme grafigi
netlestirildi. MSVC icin /W4 /WX- /std:c++17 ve konfigurasyon
bazli /DDEBUG+/Od (Debug) / /O2+/DNDEBUG (Release) bayraklari;
GCC icin esdeger -Wall -Wextra ve optimizasyon secenekleri tanimi.
test_detectors hedefi ve CTest kaydI (add_test) eklendi.
```

**Kopyala-yapıştır:**
```bash
git commit -m "build(cmake): add target_sources, per-config compiler flags and CTest integration

target_sources() ile kaynak dosyalari eklendi, derleme grafigi
netlestirildi. MSVC/GCC icin Debug/Release bayraklari tanimlandi.
test_detectors executable ve CTest kaydedici add_test eklendi."
```

---

## 8 — GitHub Actions Workflow Güncellemesi

```
ci(workflow): add matrix build, ctest step and release artifact upload

Debug ve Release icin paralel matrix build tanimlandi; her job
ctest --output-on-failure ile testleri calistirir. Release jobunda
actions/upload-artifact@v4 ile AntiDebugTrap.exe Artifacts
sekmesine yuklenir. develop branchi tetikleyicilere eklendi ve
actions/checkout v3'ten v4'e yukseltildi.
```

**Kopyala-yapıştır:**
```bash
git commit -m "ci(workflow): add matrix build, ctest step and release artifact upload

Debug ve Release icin paralel matrix build eklendi. Her job ctest
ile testleri calistirir. Release jobunda artifact upload ile
AntiDebugTrap.exe Artifacts sekmesine yuklenir. develop branchi
tetikleyicilere eklendi, checkout v3->v4 guncellendi."
```

---

## Özet Tablosu

| # | Dosya/Modül | Type | Scope |
|---|-------------|------|-------|
| 1 | `TimingDetector.h/.cpp` | `feat` | `detectors` |
| 2 | `HandleDetector.h/.cpp` | `feat` | `detectors` |
| 3 | `HeapFlagDetector.h/.cpp` | `feat` | `detectors` |
| 4 | `MemoryEraser.h/.cpp` | `feat` | `evasion` |
| 5 | `Logger.h/.cpp` | `feat` | `utils` |
| 6 | `tests/test_detectors.cpp` | `test` | `detectors` |
| 7 | `CMakeLists.txt` | `build` | `cmake` |
| 8 | `.github/workflows/build.yml` | `ci` | `workflow` |

---

## Type Referans Kartı

| Type | Ne zaman kullanılır |
|------|---------------------|
| `feat` | Yeni özellik / modül ekleme |
| `fix` | Hata düzeltme |
| `refactor` | Davranış değiştirmeden kod yeniden yazma |
| `test` | Test ekleme veya güncelleme |
| `ci` | CI/CD pipeline değişiklikleri |
| `docs` | Yalnızca dokümantasyon değişikliği |
| `chore` | Bağımlılık güncellemesi, .gitignore vb. |
| `build` | Derleme sistemi (CMake, Makefile vb.) |
