# 🛡️ Açık Kaynak Sistem Mimarisi ve Güvenlik Zafiyeti Analiz Raporu

**Proje Adı:** Ollama (Lokal Büyük Dil Modeli / LLM Altyapısı)  

**Analist:** Gizem Kızılay

**Analiz Kapsamı:** Tersine Mühendislik, CI/CD Süreçleri, Konteyner Güvenliği ve Tehdit Modelleme (Threat Modeling)  

---

## 1. Yürütme Özeti (Executive Summary)
Bu rapor, popüler açık kaynaklı yapay zeka altyapısı olan "Ollama" projesinin kaynak kod, dağıtım ve çalışma zamanı (runtime) mimarilerini siber güvenlik perspektifiyle incelemektedir. Yapılan statik ve mimari analizler sonucunda; yazılımın kurulum aşamasında, konteyner izolasyonunda ve kimlik doğrulama mekanizmalarında kritik seviyede mimari zafiyetler (Design Flaws) tespit edilmiştir.

---

## 🏗️ 2. Başlangıç Vektörü ve Kurulum Analizi (`install.sh`)

Uygulamanın işletim sistemine entegrasyonu, reverse engineering (tersine mühendislik) prensipleriyle incelendiğinde aşağıdaki bulgular elde edilmiştir:

* **Sistem Parmak İzi (Fingerprinting):** Betik dosyası, öncelikle `uname` sistem çağrılarını kullanarak işletim sistemi ve işlemci mimarisi (ARM/AMD64) tespiti yapmaktadır.
* **Kalıcılık Sağlama (Persistence):** Dosya, `/usr/local/bin` dizinine kendini kopyalamanın yanı sıra, sistem her yeniden başlatıldığında otonom çalışabilmek adına `/etc/systemd/system/` dizininde bir `ollama.service` daemon'u (arka plan servisi) oluşturmaktadır.
* **🚨 Kritik Güvenlik Zafiyeti (Tedarik Zinciri / Supply Chain Risk):** Kurulum prosedürü, güvenlik camiasında "Anti-Pattern" olarak bilinen `curl -fsSL [URL] | sh` (Körleme Çalıştırma) metodolojisiyle çalışmaktadır. İndirilen çalıştırılabilir (executable) dosyalar için herhangi bir **Kriptografik İmza Doğrulaması (GPG)** veya **SHA-256 Checksum** kontrolü yapılmamaktadır. Bu durum, DNS Zehirlenmesi (DNS Spoofing) veya Ortadaki Adam (Man-in-the-Middle) saldırılarıyla orijinal dosyanın zararlı bir payload (virüs/truva atı) ile değiştirilmesine doğrudan olanak tanır.

---

## 🔄 3. DevSecOps ve CI/CD Pipeline İncelemesi (`.github/workflows`)

Projenin Sürekli Entegrasyon ve Sürekli Dağıtım (CI/CD) mimarisi incelendiğinde oldukça karmaşık bir yapı göze çarpmaktadır:

* **Çapraz Derleme Mimarisi (Cross-Platform Matrix):** GitHub Actions üzerinde çalışan YAML dosyaları, projenin Linux, macOS ve Windows ortamları için izole ortamlarda eşzamanlı olarak derlenmesini sağlamaktadır.
* **Donanım Hızlandırma Testleri:** Entegrasyon süreçleri, CUDA (NVIDIA) ve ROCm (AMD) kütüphanelerinin derleme anında (build-time) doğru bağlanıp bağlanmadığını test eden özel iş akışlarına sahiptir.
* **Webhook Mimarisi ve Tetikleme:** Projedeki CI/CD akışının temel motoru Webhook'lardır. Geliştirici tarafından ana depoya (repository) kod push edildiğinde, GitHub sistemi hedef sunuculara anında asenkron bir HTTP POST isteği (Webhook) gönderir. Bu olay güdümlü (event-driven) mimari; kodun derlenmesi, test edilmesi ve yeni sürümün (release) oluşturulması süreçlerini manuel müdahale olmaksızın milisaniyeler içinde otonom olarak başlatır.

---

## 🐳 4. Konteyner İzolasyonu ve Docker Mimarisi

Dağıtım mimarisinin yapıtaşı olan `Dockerfile` ve konteynerizasyon süreci incelendiğinde ciddi izolasyon ihlalleri tespit edilmiştir:

* **İmaj Katmanları (Layering):** Konteyner, yapay zeka modellerinin ihtiyaç duyduğu devasa grafik sürücüsü bağımlılıkları nedeniyle oldukça kalın (bloated) bir temel imaj (base image) üzerine inşa edilmiştir. Bu durum, saldırı yüzeyini (attack surface) genişletmektedir.
* **🚨 Kritik Güvenlik Riski (Ayrıcalık Yükseltme / Privilege Escalation):**
  Docker imajında "Least Privilege" (En Az Yetki) prensibi ihlal edilmiştir. İmaj içinde kısıtlı yetkilere sahip spesifik bir `USER` tanımlanmadığı için uygulama konteyner içerisinde varsayılan olarak **Root (Kök Kullanıcı)** yetkileriyle çalışmaktadır.
* **Ağ ve Port Zafiyeti:** Konteyner, varsayılan olarak `11434` TCP portunu dinlemektedir. Bir saldırganın konteyner içinde bulacağı olası bir zafiyet (Container Escape), doğrudan host (ana) makinenin Root yetkileriyle ele geçirilmesiyle sonuçlanabilir.

---

## 🔍 5. Tehdit Modelleme ve Kaynak Kod Analizi (Threat Modeling)

Uygulamanın Go (Golang) ile yazılmış ana kaynak kodları (`main.go` ve `server/routes.go`) Statik Kod Analizi (SAST) perspektifiyle değerlendirilmiştir:

* **Giriş Noktası (Entrypoint) ve API Yönlendirmeleri:** REST API mimarisini yöneten `routes.go` dosyası incelendiğinde, sistemin HTTP isteklerini doğrudan iç çekirdeğe (model işleme motoruna) ilettiği görülmektedir.
* **🚨 Kritik Güvenlik Riski (Broken Authentication / Sıfır Kimlik Doğrulama):**
  Ollama mimarisinde "Zero Trust" (Sıfır Güven) felsefesi tamamen göz ardı edilmiştir. Sistemin API uçlarında (Endpoints) hiçbir **JWT (JSON Web Token), OAuth, Header tabanlı API Key veya Basic Auth** mekanizması *bulunmamaktadır*.
* **Hacker Senaryosu (Attack Vector):** Savunmasız bırakılan bu yapıya dış ağlardan (Public IP) erişim sağlayan yetkisiz bir tehdit aktörü; 
  1. `/api/pull` endpoint'ine göndereceği basit bir cURL isteği ile sunucuya terabaytlarca büyüklükte rastgele model yükletebilir (Disk Exhaustion).
  2. `/api/delete` endpoint'ini kullanarak sistemdeki tüm kurumsal AI modellerini tek tuşla silebilir (Data Destruction).
  3. Sürekli model çalışma istekleri göndererek CPU/GPU kaynaklarını %100 oranında kilitleyebilir ve sistemi tamamen erişilemez (Denial of Service - DoS) hale getirebilir.

---
**Sonuç ve Değerlendirme:**
Ollama projesi; kullanım kolaylığı ve lokal yapay zeka çalıştırma konusunda devrimsel bir teknoloji sunsa da, "Security by Design" (Tasarım Aşamasında Güvenlik) ilkelerinden yoksundur. Kurumsal ağlarda kullanılmadan önce sistemin önüne kesinlikle bir ters vekil (Reverse Proxy) kimlik doğrulama katmanı (örn. Nginx/Traefik) ve sıkı firewall kuralları eklenmelidir.
