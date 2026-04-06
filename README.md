<img width="1280" height="305" alt="İstinye_Üniversitesi_logo svg" src="https://github.com/user-attachments/assets/5f914303-57e1-4d37-99bd-c368da9900bc" />


# 🛡️ Advanced Anti-Debug Trap & Dynamic Analysis Evasion (Windows x64)

[![C++ Build](https://github.com/gizemkizilay/Anti-Debug-Kapani/actions/workflows/build.yml/badge.svg)](https://github.com/gizemkizilay/Anti-Debug-Kapani/actions/workflows/build.yml)
![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)
![C++](https://img.shields.io/badge/C++-17-blue.svg)

Bu proje, Tersine Mühendislik vize ödevi kapsamında geliştirilmiş; uygulamanın kendi çalışma zamanı (runtime) ortamını analiz ederek bir hata ayıklayıcı (debugger) tarafından izlenip izlenmediğini tespit eden otonom bir koruma mekanizmasıdır.

---

## 👨‍🏫 Akademik Bilgiler
* **Üniversite:** İstinye Üniversitesi
* **Ders:** Tersine Mühendislik (Reverse Engineering)
* **Danışman Eğitmen:** Keyvan Arasteh Abbasabad
* **Geliştirici:** Gizem Kızılay

---

## 📖 İçindekiler
1. [Proje Özeti ve Amacı](#1-proje-özeti-ve-amacı)
2. [Teknik Derinlik ve Analiz Heuristikleri](#2-teknik-derinlik-ve-analiz-heuristikleri)
3. [Geliştirme Yol Haritası](#3-geliştirme-yol-haritası)
4. [Somut Çıktılar](#4-somut-çıktılar)
5. [Kurulum ve Derleme](#5-kurulum-ve-derleme)
6. [Beklenen Derinlik ve Özdeğerlendirme](#6-beklenen-derinlik-ve-özdeğerlendirme)
7. [Yasal Uyarı](#7-yasal-uyarı)

---

## 1. Proje Özeti ve Amacı
Projenin temel amacı, siber güvenlik dünyasında "Evasion" (Sakınma) teknikleri üzerine uzmanlaşmış, analiz araçlarını (x64dbg, WinDbg vb.) hissettiği an kullanıcıya veya analiste hiçbir uyarı vermeden (silent exit) süreci sonlandıran bir güvenlik katmanı inşa etmektir.

## 2. Teknik Derinlik ve Analiz Heuristikleri
Proje, standart Windows API'lerinin ötesine geçerek, işletim sistemi ve işlemci mimarisi seviyesinde derinlemesine kontroller yapar:

### A. İşletim Sistemi Seviyesi Denetimler (Windows Kernel & libc)
* **PEB (Process Environment Block) Analizi:** `IsDebuggerPresent()` kontrolü ile Windows çekirdeğinin süreç için tuttuğu "BeingDebugged" bayrağı anlık olarak sorgulanır.
* **Sessiz İnfaz (libc):** `stdlib.h` kütüphanesi üzerinden sağlanan `exit(0)` çağrısı kullanılır. Bu sayede tersine mühendisin "exception handling" veya "message box" üzerinden iz sürmesi engellenmiş olur.

### B. Bellek ve Kod Bütünlüğü Denetimi (Capstone Engine)
* **Software Breakpoint Avı (0xCC / INT 3 Tespit):** Debugger'ların kod akışını durdurmak için enjekte ettiği `0xCC` byte'larını tespit eder.
* **Dinamik Disassembly:** `Capstone Engine` kullanılarak, uygulamanın kritik fonksiyonları çalışma anında de-compile edilir.
* **Opcode Doğrulama:** Bellekteki makine kodu taranarak orijinal opcode yapısında bir bozulma veya dışarıdan müdahale (inline patching) olup olmadığı doğrulanır.

## 3. Geliştirme Yol Haritası (Milestones)

### 📅 Aşama 1: Ortam Hazırlığı ve Entegrasyon
* C++ geliştirme ortamının yapılandırılması.
* `Capstone Engine` kütüphanesinin projeye dahil edilmesi ve CMake konfigürasyonlarının tamamlanması.

### 🔍 Aşama 2: Windows API Kontrolleri
* `windows.h` üzerinden temel anti-debug bayraklarının kodlanması ve PEB yapısının sorgulanması.

### 🧠 Aşama 3: Capstone ile Bellek Analizi
* Kritik fonksiyonların bellek adreslerinin Capstone motoruna beslenerek çalışma anında assembly çıktısının alınması.
* Yazılım kesmelerinin (breakpoint) taranması ve tespit algoritmasının optimizasyonu.

### 🛡️ Aşama 4: Optimizasyon ve Test
* Projenin **Release** modunda derlenerek "False-Positive" durumlarının elenmesi.
* x64dbg ve benzeri araçlarla stres testlerinin yapılması.

## 4. Somut Çıktılar
* **Güvenli Binary (.exe):** Anti-debug mekanizmalarını içeren optimize edilmiş Windows uygulaması.
* **Kaynak Kodlar:** Modüler ve yorum satırlarıyla zenginleştirilmiş C++ kodları.
* **CMake Yapısı:** Projenin her ortamda kolayca derlenebilmesini sağlayan konfigürasyon dosyaları.

## 5. Kurulum ve Derleme
Projeyi bağımlılıklarıyla beraber derlemek için aşağıdaki adımları izleyin:
```bash
git clone https://github.com/gizemkizilay/Anti-Debug-Kapani.git
cd Anti-Debug-Kapani
mkdir build && cd build
cmake ..
cmake --build . --config Release
```

## 6. Beklenen Derinlik ve Özdeğerlendirme
* **1. Neden Bu Mimari Seçildi?:** Geleneksel anti-debug yöntemleri (örneğin sadece `IsDebuggerPresent` kullanımı) modern tersine mühendisler tarafından kolayca yamalanabilir (patch). Bu nedenle sadece API seviyesinde kalmayıp, işletim sistemi çekirdeği (PEB) ve işlemci/bellek seviyesinde (INT3 taraması, Opcode doğrulama) çok katmanlı ve hibrit bir mimari tercih edilmiştir.
* **2. Ana Güvenlik Sonuçları:** Sistem, zararlı analiz araçlarına karşı yüksek bir başarıyla "silent exit" (sessiz kapanış) sergiler. Analiste hiçbir hata mesajı vermeyerek "Exception Handling" üzerinden iz sürmeyi imkansız kılar ve kodun çalışma anındaki (runtime) bütünlüğünü garanti altına alır.
* **3. Üretim (Production) Kullanımı ve Genişletilebilirlik:** Bu prototip, üretim aşamasında ticari yazılımların DRM (Dijital Haklar Yönetimi) sistemlerine, lisanslama modüllerine veya oyunların Anti-Cheat (Hile Koruma) motorlarına doğrudan entegre edilebilir. Gelecekte projeye kod karmaşıklaştırma (Obfuscation), Anti-Dump teknikleri ve Ring0 (Kernel Mode) sürücü seviyesi kontroller eklenerek sistem çok daha ileri bir seviyeye taşınabilir.
