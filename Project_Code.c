/*
 * ================================================================
 *  IngatKita — Stimulasi Kognitif Lansia
 * ================================================================
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>

#define MAX_PASIEN    20
#define MAX_SESI      30
#define MAX_SOAL      60
#define SOAL_PER_SESI  5
#define ALZHEIMER      0
#define VASKULAR       1

/* ── Struct ── */
typedef struct {
    char kata[50];
    char pertanyaan[200];
    char hint1[120];
    char hint2[60];
    int  tipe;
} Soal;

typedef struct {
    int  skor, total, nomor;
    char tanggal[20];
    char catatan[200];
} Sesi;

typedef struct {
    char nama[60];
    int  usia, tipe;
    Sesi riwayat[MAX_SESI];
    int  jumlah_sesi;
} Pasien;

/* ── Data Global ── */
Pasien pasien[MAX_PASIEN];
int    jml_pasien = 0;
Soal   bank_soal[MAX_SOAL];
int    jml_soal   = 0;

/* ── Prototipe ── */
void inisialisasi_soal(void);
void menu_utama(void);
void mode_terapis(void);
void mode_pasien(Pasien *p);
void daftarkan_pasien(void);
void tampilkan_pasien(void);
int  pilih_pasien(void);
void jalankan_sesi(Pasien *p);
void cetak_laporan(Pasien *p, Sesi *s);
void cetak_ringkasan(Pasien *p);
void tampilkan_riwayat(Pasien *p);
void tambah_soal_baru(void);
void tambah_soal(const char *kata, const char *pertanyaan, const char *h1, const char *h2, int tipe);
void get_tanggal(char *buf);
int  cek_jawaban(const char *input, const char *kunci);
void garis(int n);
void bersihkan(void);

/* ================================================================
 *  MAIN
 * ================================================================ */
int main(void) {
    srand((unsigned)time(NULL));
    inisialisasi_soal();
    menu_utama();
    return 0;
}

/* ================================================================
 *  MENU & NAVIGASI
 * ================================================================ */
void menu_utama(void) {
    int pilihan;
    printf("\n==============================\n");
    printf("  IngatKita - Kognitif Lansia\n");
    printf("==============================\n");
    do {
        printf("\n  1. Mode Terapis\n");
        printf("  2. Mulai Sesi Pasien\n");
        printf("  0. Keluar\n");
        printf("  Pilihan: ");
        scanf("%d", &pilihan); bersihkan();

        if (pilihan == 1) mode_terapis();
        else if (pilihan == 2) {
            int idx = pilih_pasien();
            if (idx >= 0) mode_pasien(&pasien[idx]);
        }
    } while (pilihan != 0);

    printf("\n  Terima kasih telah menggunakan IngatKita.\n\n");
}

void mode_terapis(void) {
    int p;
    do {
        printf("\n--- MODE TERAPIS ---\n");
        printf("  1. Daftarkan pasien baru\n");
        printf("  2. Lihat daftar pasien\n");
        printf("  3. Lihat riwayat pasien\n");
        printf("  4. Tambah soal baru\n");
        printf("  0. Kembali\n");
        printf("  Pilihan: ");
        scanf("%d", &p); bersihkan();

        if      (p == 1) daftarkan_pasien();
        else if (p == 2) { tampilkan_pasien(); printf("  [Enter]"); getchar(); }
        else if (p == 3) {
            int idx = pilih_pasien();
            if (idx >= 0) tampilkan_riwayat(&pasien[idx]);
        }
        else if (p == 4) tambah_soal_baru();
    } while (p != 0);
}

/* ================================================================
 *  MANAJEMEN PASIEN
 * ================================================================ */
void daftarkan_pasien(void) {
    if (jml_pasien >= MAX_PASIEN) { printf("  Data penuh.\n"); return; }

    Pasien *p = &pasien[jml_pasien];
    printf("\n  Nama  : "); fgets(p->nama, sizeof(p->nama), stdin);
    p->nama[strcspn(p->nama, "\n")] = '\0';
    printf("  Usia  : "); scanf("%d", &p->usia); bersihkan();
    printf("  Tipe  : 1=Alzheimer  2=Vaskular\n  Pilih : ");
    int t; scanf("%d", &t); bersihkan();
    p->tipe = (t == 2) ? VASKULAR : ALZHEIMER;
    p->jumlah_sesi = 0;

    jml_pasien++;
    printf("  Pasien \"%s\" berhasil didaftarkan.\n", p->nama);
}

void tampilkan_pasien(void) {
    const char *tipe_str[] = {"Alzheimer", "Vaskular"};
    printf("\n--- DAFTAR PASIEN ---\n");
    if (jml_pasien == 0) { printf("  Belum ada pasien.\n"); return; }
    for (int i = 0; i < jml_pasien; i++) {
        printf("  %d. %-20s %d thn [%s] %d sesi\n",
            i+1, pasien[i].nama, pasien[i].usia,
            tipe_str[pasien[i].tipe], pasien[i].jumlah_sesi);
    }
}

int pilih_pasien(void) {
    if (jml_pasien == 0) {
        printf("  Belum ada pasien. Daftarkan lewat Mode Terapis.\n");
        return -1;
    }
    tampilkan_pasien();
    printf("  Pilih nomor pasien: ");
    int no; scanf("%d", &no); bersihkan();
    if (no >= 1 && no <= jml_pasien) return no - 1;
    printf("  Nomor tidak valid.\n");
    return -1;
}

/* ================================================================
 *  SESI PASIEN 
 * ================================================================ */
void mode_pasien(Pasien *p) {
    printf("\n==============================\n");
    printf("  Selamat datang, %s!\n", p->nama);
    printf("  Sesi ke-%d\n", p->jumlah_sesi + 1);
    printf("==============================\n");
    printf("  Terapis: tekan Enter untuk mulai...");
    getchar();
    jalankan_sesi(p);
}

void jalankan_sesi(Pasien *p) {
    
    int pool[MAX_SOAL], n = 0;
    for (int i = 0; i < jml_soal; i++)
        if (bank_soal[i].tipe == p->tipe) pool[n++] = i;

    if (n == 0) { printf("  Tidak ada soal untuk tipe ini.\n"); return; }

    
    for (int i = n-1; i > 0; i--) {
        int j = rand() % (i+1);
        int tmp = pool[i]; pool[i] = pool[j]; pool[j] = tmp;
    }

    int total = (n < SOAL_PER_SESI) ? n : SOAL_PER_SESI;

    Sesi s;
    memset(&s, 0, sizeof(Sesi));
    s.total  = total;
    s.nomor  = p->jumlah_sesi + 1;
    get_tanggal(s.tanggal);

    printf("\n--- SESI %d | %s | %d soal ---\n\n", s.nomor, s.tanggal, total);

    for (int i = 0; i < total; i++) {
        Soal *q = &bank_soal[pool[i]];
        char buf[80];
        int benar = 0;

        
        printf("  Soal %d/%d\n", i+1, total);
        garis(35);
        printf("  %s\n\n", q->pertanyaan);

        /* Percobaan 1 */
        printf("  Jawaban: "); fgets(buf, sizeof(buf), stdin);
        if (cek_jawaban(buf, q->kata)) {
            printf("  Luar biasa! BENAR!\n\n"); benar = 1;
        } else {
            /* Hint 1 */
            printf("  Petunjuk 1: %s\n", q->hint1);
            printf("  Coba lagi : "); fgets(buf, sizeof(buf), stdin);
            if (cek_jawaban(buf, q->kata)) {
                printf("  Bagus! BENAR!\n\n"); benar = 1;
            } else {
                /* Hint 2 */
                printf("  Petunjuk 2: %s\n", q->hint2);
                printf("  Coba lagi : "); fgets(buf, sizeof(buf), stdin);
                if (cek_jawaban(buf, q->kata)) {
                    printf("  Hebat! BENAR dengan bantuan!\n\n"); benar = 1;
                } else {
                    printf("  Jawaban: %s — Tidak apa-apa, terus semangat!\n\n", q->kata);
                }
            }
        }
        if (benar) s.skor++;
    }

    /* Input catatan terapis */
    printf("  Catatan terapis (Enter untuk skip):\n  > ");
    fgets(s.catatan, sizeof(s.catatan), stdin);
    s.catatan[strcspn(s.catatan, "\n")] = '\0';

    /* Simpan sesi ke riwayat pasien lewat pointer */
    if (p->jumlah_sesi < MAX_SESI)
        p->riwayat[p->jumlah_sesi++] = s;

    cetak_laporan(p, &s);
    cetak_ringkasan(p);

    printf("  [Enter untuk lanjut...]"); getchar();
}

/* ================================================================
 *  LAPORAN
 * ================================================================ */
void cetak_laporan(Pasien *p, Sesi *s) {
    int pct = (s->total > 0) ? (s->skor * 100 / s->total) : 0;
    const char *tipe_str[] = {"Alzheimer", "Vaskular"};

    printf("\n====== LAPORAN SESI ======\n");
    printf("  Pasien  : %s (%d thn)\n", p->nama, p->usia);
    printf("  Tipe    : %s\n", tipe_str[p->tipe]);
    printf("  Tanggal : %s | Sesi ke-%d\n", s->tanggal, s->nomor);
    printf("  Skor    : %d/%d (%d%%)\n", s->skor, s->total, pct);

    /* Bar progres */
    printf("  Progres : [");
    for (int i = 0; i < 10; i++) printf("%c", i < pct/10 ? '#' : '-');
    printf("]\n");

    /* Perbandingan sesi  */
    if (p->jumlah_sesi > 1) {
        Sesi *prev = &p->riwayat[p->jumlah_sesi - 2];
        int prev_pct = (prev->total > 0) ? (prev->skor * 100 / prev->total) : 0;
        printf("  Sesi lalu: %d%% | Sekarang: %d%% ", prev_pct, pct);
        if      (pct > prev_pct) printf("(Meningkat!)\n");
        else if (pct == prev_pct) printf("(Stabil)\n");
        else                     printf("(Perlu perhatian)\n");
    }

    if (strlen(s->catatan) > 0)
        printf("  Catatan : %s\n", s->catatan);

    /* Afirmasi dan motivasi */
    int p_pct = (s->total > 0) ? (s->skor * 100 / s->total) : 0;
    if      (p_pct >= 80) printf("\n  Luar biasa, %s! Kamu hebat hari ini!\n", p->nama);
    else if (p_pct >= 50) printf("\n  Bagus, %s! Terus latih ingatanmu!\n", p->nama);
    else                  printf("\n  Terima kasih sudah berusaha, %s. Besok pasti lebih baik!\n", p->nama);

    printf("==========================\n");
}

void cetak_ringkasan(Pasien *p) {
    if (p->jumlah_sesi == 0) return;

    int total_skor = 0, total_soal = 0;
    int best_pct = 0, worst_pct = 100, best_i = 0, worst_i = 0;

    for (int i = 0; i < p->jumlah_sesi; i++) {
        Sesi *s = &p->riwayat[i];
        int pct = (s->total > 0) ? (s->skor * 100 / s->total) : 0;
        total_skor += s->skor;
        total_soal += s->total;
        if (pct >= best_pct)  { best_pct  = pct;  best_i  = i; }
        if (pct <= worst_pct) { worst_pct = pct;  worst_i = i; }
    }

    int rata = (total_soal > 0) ? (total_skor * 100 / total_soal) : 0;

    printf("\n====== RINGKASAN KESELURUHAN ======\n");
    printf("  Pasien      : %s\n", p->nama);
    printf("  Total sesi  : %d | Rata-rata: %d%%\n", p->jumlah_sesi, rata);
    printf("  Terbaik     : Sesi %d (%d%%) — %s\n",
           best_i+1, best_pct, p->riwayat[best_i].tanggal);
    printf("  Terlemah    : Sesi %d (%d%%) — %s\n",
           worst_i+1, worst_pct, p->riwayat[worst_i].tanggal);

    /* Perkembangan */
    int separuh = p->jumlah_sesi / 2;
    if (separuh >= 1) {
        int sum_awal = 0, sum_akhir = 0;
        for (int i = 0; i < separuh; i++) {
            Sesi *s = &p->riwayat[i];
            sum_awal += (s->total > 0) ? (s->skor * 100 / s->total) : 0;
        }
        for (int i = separuh; i < p->jumlah_sesi; i++) {
            Sesi *s = &p->riwayat[i];
            sum_akhir += (s->total > 0) ? (s->skor * 100 / s->total) : 0;
        }
        int avg_awal  = sum_awal  / separuh;
        int avg_akhir = sum_akhir / (p->jumlah_sesi - separuh);
        printf("  Tren        : ");
        if      (avg_akhir > avg_awal + 5) printf("Meningkat\n");
        else if (avg_akhir < avg_awal - 5) printf("Menurun — evaluasi pendekatan terapi\n");
        else                               printf("Stabil\n");
    }
    printf("===================================\n");
}

void tampilkan_riwayat(Pasien *p) {
    printf("\n--- RIWAYAT: %s ---\n", p->nama);
    if (p->jumlah_sesi == 0) { printf("  Belum ada sesi.\n"); return; }

    printf("  No  Tanggal       Skor    Catatan\n");
    garis(50);
    for (int i = 0; i < p->jumlah_sesi; i++) {
        Sesi *s = &p->riwayat[i];
        int pct = (s->total > 0) ? (s->skor * 100 / s->total) : 0;
        printf("  %-3d %-12s  %d/%d(%d%%)  %s\n",
               i+1, s->tanggal, s->skor, s->total, pct,
               strlen(s->catatan) ? s->catatan : "-");
    }

    /* Grafik ASCII */
    printf("\n  Grafik (%%)\n");
    for (int baris = 10; baris >= 1; baris--) {
        printf("  %3d%% |", baris*10);
        for (int i = 0; i < p->jumlah_sesi && i < 15; i++) {
            int pct = (p->riwayat[i].total > 0) ?
                      (p->riwayat[i].skor * 100 / p->riwayat[i].total) : 0;
            printf("%s", pct >= baris*10 ? " ## " : "    ");
        }
        printf("\n");
    }
    printf("       +");
    for (int i = 0; i < p->jumlah_sesi && i < 15; i++) printf("----");
    printf("\n        ");
    for (int i = 0; i < p->jumlah_sesi && i < 15; i++) printf("S%-3d", i+1);
    printf("\n");

    cetak_ringkasan(p);
    printf("  [Enter]"); getchar();
}

/* ================================================================
 *  TAMBAH SOAL (terapis)
 * ================================================================ */
void tambah_soal_baru(void) {
    if (jml_soal >= MAX_SOAL) { printf("  Bank soal penuh.\n"); return; }

    Soal *s = &bank_soal[jml_soal];
    printf("\n--- TAMBAH SOAL BARU ---\n");
    printf("  Kata jawaban  : "); fgets(s->kata,       sizeof(s->kata),       stdin); s->kata[strcspn(s->kata,"\n")]='\0';
    printf("  Pertanyaan    : "); fgets(s->pertanyaan, sizeof(s->pertanyaan), stdin); s->pertanyaan[strcspn(s->pertanyaan,"\n")]='\0';
    printf("  Petunjuk 1    : "); fgets(s->hint1,      sizeof(s->hint1),      stdin); s->hint1[strcspn(s->hint1,"\n")]='\0';
    printf("  Petunjuk 2    : "); fgets(s->hint2,      sizeof(s->hint2),      stdin); s->hint2[strcspn(s->hint2,"\n")]='\0';
    printf("  Tipe (1=Alzheimer, 2=Vaskular): ");
    int t; scanf("%d",&t); bersihkan();
    s->tipe = (t == 2) ? VASKULAR : ALZHEIMER;
    jml_soal++;
    printf("  Soal \"%s\" ditambahkan.\n", s->kata);
}

/* ================================================================
 *  INISIALISASI SOAL DEFAULT
 * ================================================================ */
void inisialisasi_soal(void) {
    /* Alzheimer */
    tambah_soal("sendok",
        "Alat makan berbentuk lonjong yang kita pakai setiap hari\n"
        "  untuk menyendok nasi atau sup ke mulut.",
        "Terbuat dari logam atau plastik, tangkainya panjang",
        "Huruf awal: S", ALZHEIMER);
    tambah_soal("bantal",
        "Benda lembut dan empuk yang setiap malam kita letakkan\n"
        "  di bawah kepala supaya tidur terasa nyaman.",
        "Biasanya ada sarungnya, diletakkan di atas kasur",
        "Huruf awal: B", ALZHEIMER);
    tambah_soal("payung",
        "Ketika hujan turun, kita membuka benda ini\n"
        "  supaya badan tidak basah kuyup.",
        "Bentuknya bulat di atas, ada gagang untuk dipegang",
        "Huruf awal: P", ALZHEIMER);
    tambah_soal("cermin",
        "Setiap pagi sebelum keluar rumah, kita berdiri di depan\n"
        "  benda ini untuk melihat wajah dan merapikan penampilan.",
        "Permukaannya licin dan mengkilap, bisa memantulkan bayangan",
        "Huruf awal: C", ALZHEIMER);
    tambah_soal("sabun",
        "Setiap kali mandi, kita menggosokkan benda wangi ini\n"
        "  ke seluruh tubuh agar badan bersih dan segar.",
        "Ada yang berbentuk batang padat, ada juga yang cair",
        "Huruf awal: S", ALZHEIMER);
    tambah_soal("ibu",
        "Orang perempuan yang melahirkan kita dan merawat kita\n"
        "  sejak kecil dengan penuh kasih sayang.",
        "Kita memanggilnya dengan penuh kasih sayang",
        "Huruf awal: I", ALZHEIMER);
    tambah_soal("nenek",
        "Ibu dari ayah atau ibu kita yang sudah berambut putih,\n"
        "  sering memasak makanan kesukaan cucu-cucunya.",
        "Biasanya lebih tua dari orang tua kita",
        "Huruf awal: N", ALZHEIMER);
    tambah_soal("sate",
        "Makanan favorit banyak orang: daging ditusuk dengan lidi\n"
        "  lalu dibakar di atas bara api.",
        "Disantap bersama bumbu kacang atau kecap manis",
        "Huruf awal: S", ALZHEIMER);

    /* Vaskular */
    tambah_soal("merah",
        "Warna yang ada di bagian atas bendera Indonesia,\n"
        "  dan juga warna tomat yang sudah matang.",
        "Warna yang melambangkan keberanian",
        "Huruf awal: M", VASKULAR);
    tambah_soal("tiga",
        "Angka yang ada di antara dua dan empat.\n"
        "  Segitiga memiliki sisi sebanyak angka ini.",
        "Satu... dua... ?",
        "Huruf awal: T", VASKULAR);
    tambah_soal("dapur",
        "Ruangan di rumah tempat kita memasak makanan.\n"
        "  Di sini ada kompor, panci, wajan, dan pisau.",
        "Dari sini aroma masakan yang sedap berasal",
        "Huruf awal: D", VASKULAR);
    tambah_soal("pagi",
        "Waktu dalam sehari setelah matahari terbit\n"
        "  dan sebelum siang. Saat kita biasanya sarapan.",
        "Setelah tidur malam, inilah waktu pertama kita bangun",
        "Huruf awal: P", VASKULAR);
    tambah_soal("kursi",
        "Perabot rumah tangga yang memiliki empat kaki\n"
        "  dan sandaran, fungsinya untuk duduk.",
        "Ada di ruang tamu, ruang makan, dan kantor",
        "Huruf awal: K", VASKULAR);
    tambah_soal("hijau",
        "Warna daun dan rumput yang segar di taman.\n"
        "  Warna ini identik dengan alam dan tumbuhan.",
        "Hasil campuran warna biru dan kuning",
        "Huruf awal: H", VASKULAR);
    tambah_soal("pintu",
        "Bagian rumah yang kita buka saat masuk\n"
        "  dan kita tutup saat keluar atau ingin privasi.",
        "Ada yang terbuat dari kayu, ada juga dari besi",
        "Huruf awal: P", VASKULAR);
    tambah_soal("meja",
        "Perabot dengan permukaan datar dan berkaki.\n"
        "  Tempat kita makan, belajar, atau bekerja.",
        "Kursi biasanya diletakkan di depan benda ini",
        "Huruf awal: M", VASKULAR);
}

/* ================================================================
 *  UTILITAS
 * ================================================================ */
int cek_jawaban(const char *input, const char *kunci) {
    char a[60], b[60];
    strncpy(a, input, 59); a[59]='\0';
    strncpy(b, kunci,  59); b[59]='\0';
    for (int i=0;a[i];i++) a[i]=(char)tolower((unsigned char)a[i]);
    for (int i=0;b[i];i++) b[i]=(char)tolower((unsigned char)b[i]);
    int la=strlen(a);
    while (la>0&&(a[la-1]=='\n'||a[la-1]==' ')) a[--la]='\0';
    return strcmp(a,b)==0;
}

void get_tanggal(char *buf) {
    time_t t = time(NULL);
    strftime(buf, 20, "%d-%m-%Y", localtime(&t));
}

void tambah_soal(const char *kata, const char *pertanyaan,
                 const char *h1, const char *h2, int tipe) {
    if (jml_soal >= MAX_SOAL) return;
    Soal *s = &bank_soal[jml_soal++];
    strncpy(s->kata,       kata,       49); s->kata[49]='\0';
    strncpy(s->pertanyaan, pertanyaan, 199); s->pertanyaan[199]='\0';
    strncpy(s->hint1,      h1,         119); s->hint1[119]='\0';
    strncpy(s->hint2,      h2,          59); s->hint2[59]='\0';
    s->tipe = tipe;
}

void garis(int n) { for(int i=0;i<n;i++) printf("-"); printf("\n"); }
void bersihkan(void) { int c; while((c=getchar())!='\n'&&c!=EOF); }
