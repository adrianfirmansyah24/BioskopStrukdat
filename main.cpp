/*
 * ============================================================================
 *  CINEMATIX SERVER
 * ============================================================================
 *  File ini adalah BACKEND (server) dari aplikasi Cinematix.
 *  Backend ini dibuat menggunakan library httplib (httplib.h) yang membuat
 *  program C++ kita bisa menerima request HTTP seperti web server biasa.
 *
 *  Semua STRUKTUR DATA yang dipakai ada di app.h, yaitu:
 *    1. VECTOR        -> menyimpan daftar Film         (FilmManager)
 *    2. BINARY SEARCH TREE (BST) -> mencari Film by ID  (BSTFilm)
 *    3. LINKED LIST   -> menyimpan daftar Booking        (BookingList)
 *    4. STACK (LIFO)  -> menyimpan riwayat pembatalan     (UndoStack)
 *    5. QUEUE (FIFO)  -> menyimpan antrean waiting list   (WaitingList)
 *
 *  File main.cpp ini TIDAK membuat struktur data baru. Tugasnya hanya
 *  "menjembatani" antara frontend (HTML/JS) dengan struktur data di app.h,
 *  lewat endpoint-endpoint HTTP (alamat seperti /api/films, /api/bookings).
 *
 *  FORMAT DATA YANG DIKIRIM KE FRONTEND:
 *  Supaya sederhana dan tidak perlu library JSON, server ini mengirim data
 *  sebagai TEKS BIASA yang dipisahkan oleh tanda "|" (pipe).
 *  Contoh 1 baris data Film:
 *      1|Avengers: Doomsday|Action|10:00 - 12:30|Studio 1|50000
 *  Setiap baris representasi 1 data, dan field dipisah karakter '|'.
 *  Ini sama persis dengan cara kita menyimpan data ke file .txt di app.h.
 * ============================================================================
 */

 #include "httplib.h"
 #include "app.h"
 
 #include <iostream>
 #include <sstream>
 
 using namespace std;
 using namespace httplib;
 
 // ============================================================================
 // GLOBAL STATE
 // ----------------------------------------------------------------------------
 // Objek-objek ini hidup selama server berjalan (di memory/RAM).
 // Setiap kali ada perubahan data, kita simpan ulang ke file .txt supaya
 // data tidak hilang kalau server dimatikan dan dijalankan ulang.
 // ============================================================================
 
 FilmManager   filmManager;     // VECTOR -> daftar film
 BSTFilm       bstFilm;         // BST    -> pencarian film cepat (O(log n))
 BookingList   bookingList;     // LINKED LIST -> daftar booking
 UndoStack     undoStack;       // STACK  -> riwayat pembatalan (untuk fitur undo)
 WaitingList   waitingList;     // QUEUE  -> antrean waiting list
 
 int idCounter = 1000;          // Counter sederhana untuk membuat ID tiket unik
 
 const string FILE_FILM    = "data_film.txt";
 const string FILE_BOOKING = "data_booking.txt";
 
 // Membuat ID tiket baru, contoh: TKT1001, TKT1002, dst.
 string generateID()
 {
     idCounter = idCounter + 1;
     return "TKT" + to_string(idCounter);
 }
 
 // ============================================================================
 // HELPER: PARSING REQUEST BODY (format "key=value" dipisah "&", seperti form)
 // ----------------------------------------------------------------------------
 // Saat frontend mengirim data lewat POST, datanya berbentuk teks seperti:
 //      nama=Budi&film=Sinners&kursi=A5&harga=55000
 // Fungsi ini membaca teks tersebut dan mengambil value dari key yang diminta.
 // Ini PENGGANTI JSON — supaya tidak perlu library tambahan, cukup pakai
 // string processing (find, substr) yang sudah dipelajari di dasar C++.
 // ============================================================================
 
 string getField(const string& body, const string& key)
 {
     string cariKey = key + "=";
     size_t pos = body.find(cariKey);
     if (pos == string::npos) return "";
 
     pos += cariKey.length();
     size_t akhir = body.find("&", pos);
 
     string value;
     if (akhir == string::npos)
     {
         value = body.substr(pos);
     }
     else
     {
         value = body.substr(pos, akhir - pos);
     }
 
     return value;
 }
 
 int getFieldInt(const string& body, const string& key)
 {
     string value = getField(body, key);
     if (value.empty()) return 0;
     return stoi(value);
 }
 
 // ============================================================================
 // SETUP DATA AWAL
 // ----------------------------------------------------------------------------
 // Dipanggil sekali saat server pertama kali dijalankan.
 // Membaca data lama dari file .txt (jika ada), atau mengisi data contoh
 // kalau file masih kosong / belum pernah dibuat.
 // ============================================================================
 
 void setupData()
 {
     // Muat data dari file .txt ke VECTOR (filmManager) dan BST (bstFilm)
     muatFilmDariFile(FILE_FILM, filmManager, bstFilm);
 
     // Muat data booking dari file .txt ke LINKED LIST (bookingList)
     muatBookingDariFile(FILE_BOOKING, bookingList);
 
     // Kalau belum ada film sama sekali, isi dengan data contoh
     if (filmManager.getDaftarFilm().empty())
     {
         filmManager.tambahFilmData(1, "Avengers: Doomsday", "Action",    "10:00 - 12:30", "Studio 1", 50000);
         filmManager.tambahFilmData(2, "Minecraft Movie",    "Adventure", "13:00 - 15:00", "Studio 2", 45000);
         filmManager.tambahFilmData(3, "Sinners",            "Horror",    "19:00 - 21:00", "Studio 3", 55000);
 
         // Setiap film yang masuk vector, dimasukkan juga ke BST
         // supaya nanti bisa dicari dengan cepat berdasarkan ID.
         for (Film& f : filmManager.getDaftarFilm())
         {
             bstFilm.insertFilm(f);
         }
 
         simpanFilmKeFile(FILE_FILM, filmManager.getDaftarFilm());
     }
 
     cout << "[INFO] Data berhasil dimuat dari file.\n";
     cout << "[INFO] Jumlah film tersedia: " << filmManager.getDaftarFilm().size() << "\n";
 }
 
 // ============================================================================
 // MAIN — titik masuk program, sekaligus tempat semua endpoint HTTP didaftarkan
 // ============================================================================
 
 int main()
 {
     Server svr;
 
     setupData();
 
     // Izinkan frontend mengakses server ini (CORS).
     // Diperlukan karena browser secara default membatasi request antar domain/port.
     svr.set_default_headers({
         { "Access-Control-Allow-Origin", "*" },
         { "Access-Control-Allow-Methods", "GET, POST, DELETE, OPTIONS" },
         { "Access-Control-Allow-Headers", "Content-Type" }
     });
 
     svr.Options(".*", [](const Request&, Response& res) {
         res.status = 200;
     });
 
     // Server ini juga menyajikan file frontend (index.html, style.css, script.js)
     // yang ada di folder "public", supaya semuanya jalan dari satu server saja.
     svr.set_mount_point("/", "./public");
 
 
     // ========================================================================
     // BAGIAN 1: ENDPOINT FILM
     // Struktur data yang dipakai di bagian ini: VECTOR dan BST
     // ========================================================================
 
     // [GET] /api/films
     // Mengambil semua film dari VECTOR (filmManager), lalu dikirim sebagai
     // teks baris-per-baris. Ini SAMA SEPERTI fungsi tampilkanFilm() di app.h,
     // hanya saja hasilnya dikirim lewat HTTP, bukan dicetak ke layar (cout).
     svr.Get("/api/films", [](const Request&, Response& res) {
         ostringstream teks;
 
         vector<Film>& daftar = filmManager.getDaftarFilm();
 
         for (Film& f : daftar)
         {
             teks << f.id << "|" << f.judul << "|" << f.genre << "|"
                  << f.jadwal << "|" << f.studio << "|" << f.harga << "\n";
         }
 
         res.set_content(teks.str(), "text/plain");
     });
 
     // [POST] /api/films
     // Menambahkan film baru. Film disimpan ke VECTOR (filmManager) sekaligus
     // dimasukkan ke BST (bstFilm) supaya nanti bisa dicari dengan cepat.
     svr.Post("/api/films", [](const Request& req, Response& res) {
         int id        = getFieldInt(req.body, "id");
         string judul  = getField(req.body, "judul");
         string genre  = getField(req.body, "genre");
         string jadwal = getField(req.body, "jadwal");
         string studio = getField(req.body, "studio");
         int harga     = getFieldInt(req.body, "harga");
 
         filmManager.tambahFilmData(id, judul, genre, jadwal, studio, harga);
 
         Film f = { id, judul, genre, jadwal, studio, harga };
         bstFilm.insertFilm(f);   // INSERT ke Binary Search Tree
 
         simpanFilmKeFile(FILE_FILM, filmManager.getDaftarFilm());
 
         res.set_content("ok", "text/plain");
     });
 
     // [DELETE] /api/films/{id}
     // Menghapus film dari VECTOR berdasarkan ID — operasi hapus pada array dinamis.
     svr.Delete(R"(/api/films/(\d+))", [](const Request& req, Response& res) {
         int id = stoi(req.matches[1]);
         bool sukses = filmManager.hapusFilm(id);
 
         simpanFilmKeFile(FILE_FILM, filmManager.getDaftarFilm());
 
         res.set_content(sukses ? "ok" : "not_found", "text/plain");
     });
 
     // [GET] /api/films/search/{id}
     // INI BAGIAN PENTING UNTUK PRESENTASI:
     // Pencarian film di sini TIDAK memakai vector, tapi memakai BST (Binary
     // Search Tree). BST membandingkan ID dengan akar (root), lalu bergerak
     // ke kiri (jika ID lebih kecil) atau ke kanan (jika ID lebih besar).
     // Karena itu, pencarian rata-rata hanya butuh O(log n) langkah,
     // jauh lebih cepat dibanding mencari satu-satu di vector (O(n)).
     svr.Get(R"(/api/films/search/(\d+))", [](const Request& req, Response& res) {
         int id = stoi(req.matches[1]);
         Film* hasil = bstFilm.cariFilm(id);   // pencarian lewat BST
 
         if (hasil == nullptr)
         {
             res.status = 404;
             res.set_content("not_found", "text/plain");
             return;
         }
 
         ostringstream teks;
         teks << hasil->id << "|" << hasil->judul << "|" << hasil->genre << "|"
              << hasil->jadwal << "|" << hasil->studio << "|" << hasil->harga;
 
         res.set_content(teks.str(), "text/plain");
     });
 
 
     // ========================================================================
     // BAGIAN 2: ENDPOINT BOOKING
     // Struktur data yang dipakai di bagian ini: LINKED LIST dan STACK
     // ========================================================================
 
     // [GET] /api/bookings
     // Mengambil semua data booking dengan cara TRAVERSAL LINKED LIST:
     // mulai dari head, lalu pindah ke node berikutnya (->next) satu per satu
     // sampai menemukan nullptr (akhir list).
     svr.Get("/api/bookings", [](const Request&, Response& res) {
         ostringstream teks;
 
         Booking* temp = bookingList.getHead();   // mulai dari head linked list
 
         while (temp != nullptr)
         {
             teks << temp->idTiket << "|" << temp->nama << "|" << temp->film << "|"
                  << temp->jadwal << "|" << temp->studio << "|" << temp->kursi << "|"
                  << temp->harga << "\n";
 
             temp = temp->next;   // pindah ke node selanjutnya
         }
 
         res.set_content(teks.str(), "text/plain");
     });
 
     // [POST] /api/bookings
     // Menambahkan booking baru ke akhir LINKED LIST.
     // Sebelum menambah, kita cek dulu (traversal) apakah kursi yang sama
     // pada film yang sama sudah pernah dibooking orang lain.
     svr.Post("/api/bookings", [](const Request& req, Response& res) {
         string nama   = getField(req.body, "nama");
         string film   = getField(req.body, "film");
         string jadwal = getField(req.body, "jadwal");
         string studio = getField(req.body, "studio");
         string kursi  = getField(req.body, "kursi");
         int harga     = getFieldInt(req.body, "harga");
 
         // Cek kursi bentrok: traversal linked list dari awal sampai akhir
         Booking* temp = bookingList.getHead();
         while (temp != nullptr)
         {
             if (temp->film == film && temp->kursi == kursi)
             {
                 res.status = 409;   // 409 = Conflict
                 res.set_content("seat_taken", "text/plain");
                 return;
             }
             temp = temp->next;
         }
 
         string idTiket = generateID();
 
         // INSERT ke akhir LINKED LIST
         bookingList.tambahBooking(idTiket, nama, film, jadwal, studio, kursi, harga);
 
         // Cetak tiket & struk ke file .txt (fitur wajib: bukti transaksi)
         Tiket::cetakTiket(idTiket, nama, film, jadwal, studio, kursi);
         Tiket::cetakStruk(idTiket, nama, film, kursi, harga);
 
         simpanBookingKeFile(FILE_BOOKING, bookingList);
 
         res.set_content("ok|" + idTiket, "text/plain");
     });
 
     // [GET] /api/bookings/search/{idTiket}
     // Mencari 1 booking dengan TRAVERSAL LINKED LIST (mulai dari head,
     // bandingkan idTiket setiap node, sampai ketemu atau mencapai akhir list).
     svr.Get(R"(/api/bookings/search/([^/]+))", [](const Request& req, Response& res) {
         string idTiket = req.matches[1];
         Booking* hasil = bookingList.cariBooking(idTiket);
 
         if (hasil == nullptr)
         {
             res.status = 404;
             res.set_content("not_found", "text/plain");
             return;
         }
 
         ostringstream teks;
         teks << hasil->idTiket << "|" << hasil->nama << "|" << hasil->film << "|"
              << hasil->jadwal << "|" << hasil->studio << "|" << hasil->kursi << "|"
              << hasil->harga;
 
         res.set_content(teks.str(), "text/plain");
     });
 
     // [DELETE] /api/bookings/{idTiket}
     // INI BAGIAN PENTING UNTUK PRESENTASI (kombinasi LINKED LIST + STACK):
     // 1. Booking dicari & dihapus dari LINKED LIST (hapusDanKembalikan).
     // 2. Data booking yang terhapus tadi langsung di-PUSH ke STACK (undoStack).
     //    Stack bersifat LIFO (Last In First Out) — booking yang PALING BARU
     //    dibatalkan akan jadi yang PALING ATAS, dan paling pertama di-undo.
     svr.Delete(R"(/api/bookings/([^/]+))", [](const Request& req, Response& res) {
         string idTiket = req.matches[1];
 
         // Hapus dari LINKED LIST sekaligus ambil datanya
         Booking* data = bookingList.hapusDanKembalikan(idTiket);
 
         if (data == nullptr)
         {
             res.status = 404;
             res.set_content("not_found", "text/plain");
             return;
         }
 
         // PUSH ke STACK (riwayat pembatalan)
         undoStack.pushBooking(*data);
         delete data;   // bebaskan memory node yang sudah di-copy ke stack
 
         simpanBookingKeFile(FILE_BOOKING, bookingList);
 
         res.set_content("ok", "text/plain");
     });
 
     // [POST] /api/bookings/undo
     // Mengambil booking terakhir yang dibatalkan dengan POP dari STACK,
     // lalu memasukkannya kembali ke LINKED LIST.
     // Karena STACK adalah LIFO, undo akan selalu mengembalikan booking
     // yang PALING TERAKHIR dibatalkan terlebih dahulu.
     svr.Post("/api/bookings/undo", [](const Request&, Response& res) {
         if (undoStack.kosong())
         {
             res.status = 404;
             res.set_content("empty", "text/plain");
             return;
         }
 
         Booking data = undoStack.popBooking();   // POP dari STACK
 
         // INSERT kembali ke LINKED LIST
         bookingList.tambahBooking(
             data.idTiket, data.nama, data.film,
             data.jadwal, data.studio, data.kursi, data.harga
         );
 
         simpanBookingKeFile(FILE_BOOKING, bookingList);
 
         res.set_content("ok|" + data.idTiket, "text/plain");
     });
 
 
     // ========================================================================
     // BAGIAN 3: ENDPOINT WAITING LIST
     // Struktur data yang dipakai di bagian ini: QUEUE
     // ========================================================================
 
     // [GET] /api/waiting
     // Menampilkan semua antrean. QUEUE bersifat FIFO (First In First Out),
     // jadi urutan tampil di sini SAMA dengan urutan orang mendaftar.
     svr.Get("/api/waiting", [](const Request&, Response& res) {
         vector<string> semua = waitingList.getSemuaAntrian();
 
         ostringstream teks;
         for (size_t i = 0; i < semua.size(); i++)
         {
             teks << semua[i] << "\n";
         }
 
         res.set_content(teks.str(), "text/plain");
     });
 
     // [POST] /api/waiting
     // Menambahkan nama baru ke QUEUE — operasi ENQUEUE (push ke belakang antrean).
     svr.Post("/api/waiting", [](const Request& req, Response& res) {
         string nama = getField(req.body, "nama");
         waitingList.tambahAntrian(nama);   // ENQUEUE
         res.set_content("ok", "text/plain");
     });
 
     // [POST] /api/waiting/call
     // Memanggil orang yang paling depan di antrean — operasi DEQUEUE
     // (mengambil & menghapus elemen paling depan dari QUEUE).
     svr.Post("/api/waiting/call", [](const Request&, Response& res) {
         string nama = waitingList.panggilAntrian();   // DEQUEUE
 
         if (nama.empty())
         {
             res.status = 404;
             res.set_content("empty", "text/plain");
             return;
         }
 
         res.set_content("ok|" + nama, "text/plain");
     });
 
 
     // ========================================================================
     // JALANKAN SERVER
     // ========================================================================
 
     cout << "\n========================================\n";
     cout << "   CINEMATIX SERVER BERJALAN\n";
     cout << "   Buka browser ke: http://localhost:8080\n";
     cout << "========================================\n\n";
 
     svr.listen("0.0.0.0", 8080);
 
     return 0;
 }