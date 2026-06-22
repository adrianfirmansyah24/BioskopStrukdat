#ifndef APP_H
#define APP_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <stack>
#include <queue>

using namespace std;

// ======================================================================
// FILM  (disimpan dalam vector / array dinamis)
// ======================================================================

struct Film
{
    int id;
    string judul;
    string genre;
    string jadwal;
    string studio;
    int harga;
};

class FilmManager
{
private:
    vector<Film> daftarFilm;

public:
    // Tambah film baru ke akhir vector
    void tambahFilmData(int id, string judul, string genre, string jadwal, string studio, int harga)
    {
        Film film;
        film.id     = id;
        film.judul  = judul;
        film.genre  = genre;
        film.jadwal = jadwal;
        film.studio = studio;
        film.harga  = harga;

        daftarFilm.push_back(film);
    }

    // Hapus film berdasarkan ID — O(n)
    bool hapusFilm(int id)
    {
        for (size_t i = 0; i < daftarFilm.size(); i++)
        {
            if (daftarFilm[i].id == id)
            {
                daftarFilm.erase(daftarFilm.begin() + i);
                return true;
            }
        }
        return false;
    }

    // Cari film berdasarkan ID — O(n), linear search di vector
    Film* cariFilm(int id)
    {
        for (size_t i = 0; i < daftarFilm.size(); i++)
        {
            if (daftarFilm[i].id == id)
            {
                return &daftarFilm[i];
            }
        }
        return nullptr;
    }

    void tampilkanFilm()
    {
        if (daftarFilm.empty())
        {
            cout << "\nBelum ada film tersedia.\n";
            return;
        }

        cout << "\n========== DAFTAR FILM ==========\n";
        for (const Film& film : daftarFilm)
        {
            cout << "ID      : " << film.id << endl;
            cout << "Judul   : " << film.judul << endl;
            cout << "Genre   : " << film.genre << endl;
            cout << "Jadwal  : " << film.jadwal << endl;
            cout << "Studio  : " << film.studio << endl;
            cout << "Harga   : Rp" << film.harga << endl;
            cout << "---------------------------------\n";
        }
    }

    vector<Film>& getDaftarFilm()
    {
        return daftarFilm;
    }
};

// ======================================================================
// BST FILM  (Binary Search Tree, pencarian film berdasarkan ID — O(log n))
// ======================================================================

struct BSTNode
{
    Film data;
    BSTNode* left;
    BSTNode* right;

    BSTNode(Film film)
    {
        data  = film;
        left  = nullptr;
        right = nullptr;
    }
};

class BSTFilm
{
private:
    BSTNode* root;

    BSTNode* insert(BSTNode* node, Film film)
    {
        if (node == nullptr)
        {
            return new BSTNode(film);
        }

        if (film.id < node->data.id)
        {
            node->left = insert(node->left, film);
        }
        else if (film.id > node->data.id)
        {
            node->right = insert(node->right, film);
        }

        return node;
    }

    BSTNode* search(BSTNode* node, int id)
    {
        if (node == nullptr || node->data.id == id)
        {
            return node;
        }

        if (id < node->data.id)
        {
            return search(node->left, id);
        }

        return search(node->right, id);
    }

    void inorder(BSTNode* node)
    {
        if (node == nullptr) return;

        inorder(node->left);

        cout << "\nID      : " << node->data.id << endl;
        cout << "Judul   : " << node->data.judul << endl;
        cout << "Genre   : " << node->data.genre << endl;
        cout << "Jadwal  : " << node->data.jadwal << endl;
        cout << "Studio  : " << node->data.studio << endl;
        cout << "Harga   : Rp" << node->data.harga << endl;
        cout << "-----------------------------" << endl;

        inorder(node->right);
    }

public:
    BSTFilm()
    {
        root = nullptr;
    }

    void insertFilm(Film film)
    {
        root = insert(root, film);
    }

    Film* cariFilm(int id)
    {
        BSTNode* hasil = search(root, id);
        if (hasil == nullptr) return nullptr;
        return &(hasil->data);
    }

    void tampilkanFilmBST()
    {
        if (root == nullptr)
        {
            cout << "\nBelum ada film di BST.\n";
            return;
        }

        cout << "\n===== DAFTAR FILM (BST - terurut by ID) =====\n";
        inorder(root);
    }
};

// ======================================================================
// BOOKING  (Linked List, urutan booking sesuai waktu pemesanan)
// ======================================================================

struct Booking
{
    string idTiket;
    string nama;
    string film;
    string jadwal;
    string studio;
    string kursi;
    int harga;

    Booking* next;
};

class BookingList
{
private:
    Booking* head;

public:
    BookingList()
    {
        head = nullptr;
    }

    // Tambah booking baru ke akhir linked list — O(n)
    void tambahBooking(
        string idTiket,
        string nama,
        string film,
        string jadwal,
        string studio,
        string kursi,
        int harga
    )
    {
        Booking* baru = new Booking();
        baru->idTiket = idTiket;
        baru->nama    = nama;
        baru->film    = film;
        baru->jadwal  = jadwal;
        baru->studio  = studio;
        baru->kursi   = kursi;
        baru->harga   = harga;
        baru->next    = nullptr;

        if (head == nullptr)
        {
            head = baru;
        }
        else
        {
            Booking* temp = head;
            while (temp->next != nullptr)
            {
                temp = temp->next;
            }
            temp->next = baru;
        }
    }

    void tampilkanBooking()
    {
        if (head == nullptr)
        {
            cout << "\nBelum ada booking.\n";
            return;
        }

        Booking* temp = head;
        cout << "\n========== DAFTAR BOOKING ==========\n";

        while (temp != nullptr)
        {
            cout << "ID Tiket : " << temp->idTiket << endl;
            cout << "Nama     : " << temp->nama    << endl;
            cout << "Film     : " << temp->film    << endl;
            cout << "Jadwal   : " << temp->jadwal  << endl;
            cout << "Studio   : " << temp->studio  << endl;
            cout << "Kursi    : " << temp->kursi   << endl;
            cout << "Harga    : Rp" << temp->harga << endl;
            cout << "----------------------------------\n";

            temp = temp->next;
        }
    }

    // Cari booking berdasarkan ID tiket — O(n)
    Booking* cariBooking(string idTiket)
    {
        Booking* temp = head;

        while (temp != nullptr)
        {
            if (temp->idTiket == idTiket)
            {
                return temp;
            }
            temp = temp->next;
        }

        return nullptr;
    }

    // Hapus booking berdasarkan ID tiket — O(n)
    bool hapusBooking(string idTiket)
    {
        if (head == nullptr) return false;

        if (head->idTiket == idTiket)
        {
            Booking* hapus = head;
            head = head->next;
            delete hapus;
            return true;
        }

        Booking* current = head;

        while (current->next != nullptr)
        {
            if (current->next->idTiket == idTiket)
            {
                Booking* hapus = current->next;
                current->next  = hapus->next;
                delete hapus;
                return true;
            }
            current = current->next;
        }

        return false;
    }

    // Hapus booking dan kembalikan datanya — dipakai UndoStack
    Booking* hapusDanKembalikan(string idTiket)
    {
        if (head == nullptr) return nullptr;

        if (head->idTiket == idTiket)
        {
            Booking* hapus = head;
            head        = head->next;
            hapus->next = nullptr;
            return hapus;
        }

        Booking* current = head;

        while (current->next != nullptr)
        {
            if (current->next->idTiket == idTiket)
            {
                Booking* hapus = current->next;
                current->next  = hapus->next;
                hapus->next    = nullptr;
                return hapus;
            }
            current = current->next;
        }

        return nullptr;
    }

    Booking* getHead()
    {
        return head;
    }

    bool kosong()
    {
        return head == nullptr;
    }
};

// ======================================================================
// UNDO STACK  (Stack, LIFO — pembatalan booking paling baru di-undo dulu)
// ======================================================================

class UndoStack
{
private:
    stack<Booking> undoStack;

public:
    void pushBooking(Booking booking)
    {
        undoStack.push(booking);
    }

    Booking popBooking()
    {
        if (undoStack.empty())
        {
            throw runtime_error("Stack kosong!");
        }

        Booking data = undoStack.top();
        undoStack.pop();
        return data;
    }

    bool kosong()
    {
        return undoStack.empty();
    }

    void tampilkanStack()
    {
        if (undoStack.empty())
        {
            cout << "\nTidak ada riwayat pembatalan.\n";
            return;
        }

        stack<Booking> temp = undoStack;
        cout << "\n===== RIWAYAT PEMBATALAN =====\n";

        while (!temp.empty())
        {
            Booking data = temp.top();
            cout << "ID Tiket : " << data.idTiket << endl;
            cout << "Nama     : " << data.nama    << endl;
            cout << "Film     : " << data.film    << endl;
            cout << "---------------------------\n";
            temp.pop();
        }
    }
};

// ======================================================================
// WAITING LIST  (Queue, FIFO — antrean dipanggil sesuai urutan datang)
// ======================================================================

class WaitingList
{
private:
    queue<string> daftarTunggu;

public:
    void tambahAntrian(string nama)
    {
        daftarTunggu.push(nama);
    }

    // Mengembalikan nama yang dipanggil, atau string kosong jika antrean habis
    string panggilAntrian()
    {
        if (daftarTunggu.empty())
        {
            return "";
        }

        string nama = daftarTunggu.front();
        daftarTunggu.pop();
        return nama;
    }

    void tampilkanAntrian()
    {
        if (daftarTunggu.empty())
        {
            cout << "\nWaiting list kosong.\n";
            return;
        }

        queue<string> temp = daftarTunggu;
        cout << "\n===== WAITING LIST =====\n";

        int nomor = 1;
        while (!temp.empty())
        {
            cout << nomor << ". " << temp.front() << endl;
            temp.pop();
            nomor++;
        }
    }

    vector<string> getSemuaAntrian()
    {
        vector<string> hasil;
        queue<string> temp = daftarTunggu;

        while (!temp.empty())
        {
            hasil.push_back(temp.front());
            temp.pop();
        }

        return hasil;
    }

    bool kosong()
    {
        return daftarTunggu.empty();
    }
};

// ======================================================================
// TIKET  (cetak struk & tiket ke file .txt)
// ======================================================================

class Tiket
{
public:
    static void cetakStruk(
        string idTiket,
        string nama,
        string film,
        string kursi,
        int harga
    )
    {
        string namaFile = "struk_" + idTiket + ".txt";
        ofstream file(namaFile);

        file << "=================================\n";
        file << "       CINEMATIX RECEIPT\n";
        file << "=================================\n\n";

        file << "ID Tiket : " << idTiket << endl;
        file << "Nama     : " << nama << endl;
        file << "Film     : " << film << endl;
        file << "Kursi    : " << kursi << endl;
        file << "Harga    : Rp" << harga << endl;
        file << "\nStatus   : LUNAS\n";

        file.close();
    }

    static void cetakTiket(
        string idTiket,
        string nama,
        string film,
        string jadwal,
        string studio,
        string kursi
    )
    {
        string namaFile = "tiket_" + idTiket + ".txt";
        ofstream file(namaFile);

        file << "=================================\n";
        file << "          CINEMATIX\n";
        file << "=================================\n\n";

        file << "ID Tiket : " << idTiket << endl;
        file << "Nama     : " << nama << endl;
        file << "Film     : " << film << endl;
        file << "Jadwal   : " << jadwal << endl;
        file << "Studio   : " << studio << endl;
        file << "Kursi    : " << kursi << endl;

        file.close();
    }
};

// ======================================================================
// STORAGE  (persistensi data ke file .txt, dipanggil saat server start/stop)
// ======================================================================

const string DELIM = "|~|";

inline vector<string> splitLine(const string& line, const string& delim)
{
    vector<string> hasil;
    size_t found;
    string sisa = line;

    while ((found = sisa.find(delim)) != string::npos)
    {
        hasil.push_back(sisa.substr(0, found));
        sisa = sisa.substr(found + delim.length());
    }
    hasil.push_back(sisa);

    return hasil;
}

inline void simpanFilmKeFile(const string& path, vector<Film>& daftarFilm)
{
    ofstream file(path, ios::trunc);

    for (Film& f : daftarFilm)
    {
        file << f.id << DELIM
             << f.judul << DELIM
             << f.genre << DELIM
             << f.jadwal << DELIM
             << f.studio << DELIM
             << f.harga << "\n";
    }

    file.close();
}

inline void muatFilmDariFile(const string& path, FilmManager& filmManager, BSTFilm& bstFilm)
{
    ifstream file(path);
    if (!file.is_open()) return;

    string line;

    while (getline(file, line))
    {
        if (line.empty()) continue;

        vector<string> p = splitLine(line, DELIM);
        if (p.size() < 6) continue;

        int id      = stoi(p[0]);
        string judul  = p[1];
        string genre  = p[2];
        string jadwal = p[3];
        string studio = p[4];
        int harga     = stoi(p[5]);

        filmManager.tambahFilmData(id, judul, genre, jadwal, studio, harga);

        Film f = { id, judul, genre, jadwal, studio, harga };
        bstFilm.insertFilm(f);
    }

    file.close();
}

inline void simpanBookingKeFile(const string& path, BookingList& bookingList)
{
    ofstream file(path, ios::trunc);

    Booking* temp = bookingList.getHead();

    while (temp != nullptr)
    {
        file << temp->idTiket << DELIM
             << temp->nama << DELIM
             << temp->film << DELIM
             << temp->jadwal << DELIM
             << temp->studio << DELIM
             << temp->kursi << DELIM
             << temp->harga << "\n";

        temp = temp->next;
    }

    file.close();
}

inline void muatBookingDariFile(const string& path, BookingList& bookingList)
{
    ifstream file(path);
    if (!file.is_open()) return;

    string line;

    while (getline(file, line))
    {
        if (line.empty()) continue;

        vector<string> p = splitLine(line, DELIM);
        if (p.size() < 7) continue;

        bookingList.tambahBooking(
            p[0],        // idTiket
            p[1],        // nama
            p[2],        // film
            p[3],        // jadwal
            p[4],        // studio
            p[5],        // kursi
            stoi(p[6])   // harga
        );
    }

    file.close();
}

#endif