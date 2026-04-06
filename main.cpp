#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <capstone/capstone.h>

// Hassas işlemlerin gerçekleştiği örnek kritik fonksiyonumuz
void CriticalFunction() {
    volatile int x = 0;
    for (int i = 0; i < 1000; i++) {
        x += i;
    }
    printf("\n[+] TEBRIKLER! Program guvenli bir sekilde, izlenmeden calisiyor.\n");
}

// Anti-Debug Kontrolleri
void AntiDebugCheck() {
    // ------------- ADIM 1: IsDebuggerPresent Kontrolü -------------
    // Windows API ile programı izleyen bir kullanıcı modu debugger'ı var mı bakıyoruz.
    if (IsDebuggerPresent()) {
        exit(0); // Sessizce, hiçbir log basmadan programı kapat.
    }

    // ------------- ADIM 2: Capstone ile Bellek Taraması (INT 3 Kontrolü) -------------
    // Program kendi belleğinde CriticalFunction adlı fonksiyonu tarayacaktır.
    // Debugger'lar yazılımsal breakpoint (Software Breakpoint) attıklarında, 
    // makine komutunun arasına 'INT 3' (0xCC) byte'ını yerleştirirler.

    csh handle;
    // Programınızın 32-bit (x86) veya 64-bit (x64) olmasına göre doğru Capstone modunu seçiyoruz:
#if defined(_WIN64)
    cs_mode mode = CS_MODE_64;
#else
    cs_mode mode = CS_MODE_32;
#endif

    // Capstone'u x86 mimarisi için başlatıyoruz.
    if (cs_open(CS_ARCH_X86, mode, &handle) != CS_ERR_OK) {
        return; 
    }

    cs_insn *insn;
    // "CriticalFunction" bellek adresinden itibaren ilk 100 byte disassembler'dan (tersine çevirici) geçirilir.
    // Bellekte bu 100 byte içindeki makine kodları çözümlenir.
    size_t count = cs_disasm(handle, (const uint8_t*)CriticalFunction, 100, (uint64_t)CriticalFunction, 0, &insn);
    
    if (count > 0) {
        // Ayrıştırılan tüm instruction'ları, komutları tek tek kontrol et
        for (size_t j = 0; j < count; j++) {
            // Eğer komut x86 mimarisine ait INT 3 komutu ise 
            // VEYA komutun ham byte'larından herhangi biri 0xCC (yazılımsal breakpoint) ise:
            if (insn[j].id == X86_INS_INT3 || (insn[j].size >= 1 && insn[j].bytes[0] == 0xCC)) {
                cs_free(insn, count);
                cs_close(&handle);
                // Breakpoint (0xCC) tespit edildi! KESİNLİKLE uyarı vermeden kapat.
                exit(0); 
            }
        }
        cs_free(insn, count); // Ayrıştırılmış komutların tutulduğu hafızayı temizle.
    } else {
        // Eğer Capstone disassembler analiz edemezse (hata vb.), 
        // son bir güvenlik (fallback) önlemi olarak düz byte taraması yap.
        const uint8_t* p = (const uint8_t*)CriticalFunction;
        for (int i = 0; i < 100; i++) {
            // Doğrudan ilk 100 byte'ı kontrol et, 0xCC var mı?
            if (p[i] == 0xCC) {
                cs_close(&handle);
                exit(0);
            }
        }
    }
    
    // İşimiz bitince Capstone nesnesini kapat
    cs_close(&handle);
}

int main() {
    // 1) Debugger denetimlerini ve hafıza taramalarını gerçekleştir
    AntiDebugCheck();
    
    // 2) Eğer bir güvenlik ihlali (debugger ya da breakpoint) yoksa fonksiyonu çalıştır
    CriticalFunction();
    
    return 0;
}
