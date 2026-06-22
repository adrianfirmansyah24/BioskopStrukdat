// ============================================================================
// SCRIPT.JS — FRONTEND CINEMATIX
// ----------------------------------------------------------------------------
// File ini mengambil data dari BACKEND (main.cpp) lewat fetch().
// Data yang diterima dari server BUKAN JSON, tapi TEKS BIASA yang dipisah
// tanda "|" untuk antar-field, dan baris baru (\n) untuk antar-data.
// Contoh data film yang diterima dari server:
//      1|Avengers: Doomsday|Action|10:00 - 12:30|Studio 1|50000
//      2|Minecraft Movie|Adventure|13:00 - 15:00|Studio 2|45000
// Kita tinggal pakai .split("\n") untuk pisah per baris,
// lalu .split("|") untuk pisah per field.
// ============================================================================

const API_BASE = ""; // kosong karena frontend disajikan oleh server yang sama (same-origin)

// ===================== STATE =====================

let films         = [];
let bookings      = [];
let waitingQueue  = [];
let selectedSeats = [];   // array, bisa pilih banyak kursi
let currentFilm   = null;

// Layout kursi per studio (statis di frontend, server hanya menyimpan kursi mana yang terisi)
const studioLayouts = {
    "Studio 1": { rows: ["A","B","C","D","E","F","G","H"], cols: 10 },
    "Studio 2": { rows: ["A","B","C","D","E","F"],         cols: 8  },
    "Studio 3": { rows: ["A","B","C","D","E","F","G"],     cols: 12 },
};

// ===================== HELPERS =====================

function formatRupiah(n) { return "Rp" + n.toLocaleString("id-ID"); }

function showMsg(elId, text, type) {
    const el = document.getElementById(elId);
    el.textContent = text;
    el.className = "msg " + type;
    setTimeout(() => { el.textContent = ""; el.className = "msg"; }, 3500);
}

function bookedSeatsForFilm(filmJudul) {
    return bookings
        .filter(b => b.film === filmJudul)
        .map(b => ({ kursi: b.kursi, nama: b.nama }));
}

// Mengubah satu baris teks "id|judul|genre|jadwal|studio|harga" menjadi objek Film
function parseFilmLine(line) {
    const p = line.split("|");
    return {
        id:     parseInt(p[0]),
        judul:  p[1],
        genre:  p[2],
        jadwal: p[3],
        studio: p[4],
        harga:  parseInt(p[5]),
    };
}

// Mengubah satu baris teks booking menjadi objek Booking
function parseBookingLine(line) {
    const p = line.split("|");
    return {
        idTiket: p[0],
        nama:    p[1],
        film:    p[2],
        jadwal:  p[3],
        studio:  p[4],
        kursi:   p[5],
        harga:   parseInt(p[6]),
    };
}

// Mengubah teks balasan server "ok|TKT1001" menjadi { status, value }
function parseStatusLine(text) {
    const p = text.split("|");
    return { status: p[0], value: p[1] || "" };
}

// Membuat body request dengan format key=value&key2=value2 (seperti form HTML biasa)
function buildFormBody(fields) {
    return Object.keys(fields)
        .map(key => `${key}=${encodeURIComponent(fields[key])}`)
        .join("&");
}

// ===================== FETCH HELPERS (pengganti axios/JSON) =====================

async function apiGetText(path) {
    const res = await fetch(API_BASE + path);
    return res.text();
}

async function apiPostForm(path, fields) {
    const res = await fetch(API_BASE + path, {
        method: "POST",
        headers: { "Content-Type": "application/x-www-form-urlencoded" },
        body: buildFormBody(fields || {}),
    });
    const text = await res.text();
    return { ok: res.ok, status: res.status, text };
}

async function apiDelete(path) {
    const res = await fetch(API_BASE + path, { method: "DELETE" });
    const text = await res.text();
    return { ok: res.ok, status: res.status, text };
}

// ===================== LOAD DATA DARI SERVER =====================

async function loadFilms() {
    const teks = await apiGetText("/api/films");
    films = teks
        .split("\n")
        .filter(line => line.trim() !== "")
        .map(parseFilmLine);
}

async function loadBookings() {
    const teks = await apiGetText("/api/bookings");
    bookings = teks
        .split("\n")
        .filter(line => line.trim() !== "")
        .map(parseBookingLine);
}

async function loadWaiting() {
    const teks = await apiGetText("/api/waiting");
    waitingQueue = teks
        .split("\n")
        .filter(line => line.trim() !== "");
}

// ===================== FILM CARDS =====================

function renderFilmGrid() {
    const grid   = document.getElementById("film-grid");
    const select = document.getElementById("select-film");

    grid.innerHTML   = "";
    select.innerHTML = '<option value="">-- Pilih Film --</option>';

    films.forEach(film => {
        const card = document.createElement("div");
        card.className = "film-card";
        card.innerHTML = `
            <p class="genre-tag">${film.genre}</p>
            <h3>${film.judul}</h3>
            <div class="film-meta">
                <span>🕐 ${film.jadwal}</span>
                <span>🎬 ${film.studio}</span>
            </div>
            <p class="film-price">${formatRupiah(film.harga)}</p>
        `;
        card.addEventListener("click", () => {
            document.getElementById("select-film").value = film.id;
            onFilmSelected(film.id);
            document.getElementById("booking").scrollIntoView({ behavior: "smooth" });
        });
        grid.appendChild(card);

        const opt = document.createElement("option");
        opt.value       = film.id;
        opt.textContent = `${film.judul} — ${formatRupiah(film.harga)}`;
        select.appendChild(opt);
    });
}

// ===================== FILM SELECTED =====================

function onFilmSelected(filmId) {
    currentFilm   = films.find(f => f.id == filmId) || null;
    selectedSeats = [];

    updateFilmPreview(currentFilm);
    updateSeatMap();

    const seatSection = document.getElementById("seatmap-section");
    const namaSection = document.getElementById("nama-section");
    const btnBooking  = document.getElementById("btn-booking");

    if (currentFilm) {
        seatSection.style.display = "block";
        namaSection.style.display = "block";
        btnBooking.style.display  = "inline-block";
    } else {
        seatSection.style.display = "none";
        namaSection.style.display = "none";
        btnBooking.style.display  = "none";
    }
}

document.getElementById("select-film").addEventListener("change", function () {
    onFilmSelected(this.value);
});

// ===================== FILM PREVIEW =====================

function updateFilmPreview(film) {
    const preview = document.getElementById("film-preview");

    if (!film) {
        preview.innerHTML = '<p class="preview-placeholder">Pilih film untuk melihat detail & denah kursi.</p>';
        return;
    }

    const layout = studioLayouts[film.studio];
    const total  = layout ? layout.rows.length * layout.cols : "-";
    const terisi = bookedSeatsForFilm(film.judul).length;
    const sisa   = typeof total === "number" ? total - terisi : "-";

    preview.innerHTML = `
        <div class="preview-detail">
            <div><strong>Film</strong>        <span>${film.judul}</span></div>
            <div><strong>Genre</strong>       <span>${film.genre}</span></div>
            <div><strong>Jadwal</strong>      <span>${film.jadwal}</span></div>
            <div><strong>Studio</strong>      <span>${film.studio}</span></div>
            <div><strong>Harga/Kursi</strong> <span style="color:var(--accent);font-weight:600">${formatRupiah(film.harga)}</span></div>
            <div><strong>Kursi Sisa</strong>  <span style="color:var(--green)">${sisa} / ${total}</span></div>
        </div>
    `;
}

// ===================== SEAT MAP =====================

function updateSeatMap() {
    const mapEl  = document.getElementById("seat-map");
    const infoEl = document.getElementById("selected-seat-info");
    mapEl.innerHTML = "";

    if (!currentFilm) {
        infoEl.innerHTML = "Belum ada kursi dipilih.";
        return;
    }

    const layout = studioLayouts[currentFilm.studio];
    if (!layout) {
        infoEl.innerHTML = "Belum ada kursi dipilih.";
        return;
    }

    const booked = bookedSeatsForFilm(currentFilm.judul);

    layout.rows.forEach(row => {
        const rowDiv = document.createElement("div");
        rowDiv.className = "seat-row";

        const label = document.createElement("span");
        label.className   = "row-label";
        label.textContent = row;
        rowDiv.appendChild(label);

        for (let c = 1; c <= layout.cols; c++) {
            if (c === Math.floor(layout.cols / 2) + 1) {
                const gap = document.createElement("div");
                gap.style.width = "18px";
                rowDiv.appendChild(gap);
            }

            const seatId     = row + c;
            const seatEl     = document.createElement("div");
            seatEl.className = "seat";
            seatEl.textContent = c;

            const bookedEntry = booked.find(b => b.kursi === seatId);

            if (bookedEntry) {
                seatEl.classList.add("booked");
                seatEl.setAttribute("data-bookedby", bookedEntry.nama);
                seatEl.title = "Dipesan oleh: " + bookedEntry.nama;
            } else if (selectedSeats.includes(seatId)) {
                seatEl.classList.add("selected");
                seatEl.addEventListener("click", () => toggleSeat(seatId));
            } else {
                seatEl.classList.add("available");
                seatEl.addEventListener("click", () => toggleSeat(seatId));
            }

            rowDiv.appendChild(seatEl);
        }

        mapEl.appendChild(rowDiv);
    });

    if (selectedSeats.length === 0) {
        infoEl.innerHTML = "Belum ada kursi dipilih.";
    } else {
        const totalHarga = currentFilm.harga * selectedSeats.length;
        infoEl.innerHTML = `
            Kursi dipilih: <strong>${selectedSeats.join(", ")}</strong>
            &nbsp;·&nbsp; ${selectedSeats.length} kursi
            &nbsp;·&nbsp; Total: <strong>${formatRupiah(totalHarga)}</strong>
        `;
    }
}

function toggleSeat(seatId) {
    const idx = selectedSeats.indexOf(seatId);
    if (idx === -1) {
        selectedSeats.push(seatId);
    } else {
        selectedSeats.splice(idx, 1);
    }
    updateSeatMap();
}

// ===================== BOOKING LIST (UI) =====================

function renderTicketList() {
    const list = document.getElementById("ticket-list");

    if (bookings.length === 0) {
        list.innerHTML = '<p class="empty-state">Belum ada booking. Pesan tiket dulu!</p>';
        return;
    }

    list.innerHTML = "";
    bookings.forEach(b => {
        const card = document.createElement("div");
        card.className = "ticket-card";
        card.id = "ticket-" + b.idTiket;
        card.innerHTML = `
            <div class="ticket-left">
                <span class="ticket-id">${b.idTiket}</span>
                <span class="ticket-film">${b.film}</span>
                <span class="ticket-meta">👤 ${b.nama} &nbsp;·&nbsp; 💺 Kursi ${b.kursi} &nbsp;·&nbsp; 🕐 ${b.jadwal} &nbsp;·&nbsp; 🎬 ${b.studio}</span>
            </div>
            <div class="ticket-right">
                <span class="ticket-price">${formatRupiah(b.harga)}</span>
                <button class="ticket-view" data-id="${b.idTiket}">Lihat Karcis</button>
                <button class="ticket-cancel" data-id="${b.idTiket}">Batalkan</button>
            </div>
        `;
        list.appendChild(card);
    });

    document.querySelectorAll(".ticket-view").forEach(btn => {
        btn.addEventListener("click", function () { showTicketModal(this.dataset.id); });
    });

    document.querySelectorAll(".ticket-cancel").forEach(btn => {
        btn.addEventListener("click", function () { cancelBooking(this.dataset.id); });
    });
}

// ===================== KARCIS / TIKET MODAL =====================

function buildTicketHTML(b) {
    return `
        <button class="ticket-modal-close" id="ticket-modal-close">&times;</button>
        <div class="karcis">
            <div class="karcis-header">
                <span class="karcis-logo">CINEMATIX</span>
                <span class="karcis-tag">E-TICKET</span>
            </div>

            <div class="karcis-film">
                <h3>${b.film}</h3>
                <p>${b.studio} &nbsp;·&nbsp; ${b.jadwal}</p>
            </div>

            <div class="karcis-divider">
                <span class="karcis-notch karcis-notch-left"></span>
                <span class="karcis-dashed"></span>
                <span class="karcis-notch karcis-notch-right"></span>
            </div>

            <div class="karcis-body">
                <div class="karcis-row">
                    <div class="karcis-field">
                        <label>Nama</label>
                        <span>${b.nama}</span>
                    </div>
                    <div class="karcis-field">
                        <label>Kursi</label>
                        <span class="karcis-seat">${b.kursi}</span>
                    </div>
                </div>
                <div class="karcis-row">
                    <div class="karcis-field">
                        <label>ID Tiket</label>
                        <span>${b.idTiket}</span>
                    </div>
                    <div class="karcis-field">
                        <label>Harga</label>
                        <span>${formatRupiah(b.harga)}</span>
                    </div>
                </div>
            </div>

            <div class="karcis-barcode">
                ${"▌▌▍▌▌▍▌▍▌▌▍▌▌▌▍▌▍▌▌▍▌▌▌▍▌▌▍▌▌▍▌".split("").map(c => `<span>${c}</span>`).join("")}
            </div>
            <p class="karcis-footer">Tunjukkan karcis ini di pintu masuk studio</p>
        </div>
    `;
}

function showTicketModal(idTiket) {
    const b = bookings.find(x => x.idTiket === idTiket);
    if (!b) return;

    const overlay = document.getElementById("ticket-modal-overlay");
    const content = document.getElementById("ticket-modal-content");

    content.innerHTML = buildTicketHTML(b);
    overlay.style.display = "flex";

    document.getElementById("ticket-modal-close").addEventListener("click", hideTicketModal);
    overlay.addEventListener("click", (e) => {
        if (e.target === overlay) hideTicketModal();
    });
}

function hideTicketModal() {
    document.getElementById("ticket-modal-overlay").style.display = "none";
}

// ===================== BOOKING ACTION =====================

document.getElementById("btn-booking").addEventListener("click", async () => {
    const nama = document.getElementById("input-nama").value.trim();

    if (!currentFilm)               { showMsg("booking-msg", "Pilih film dulu!", "error"); return; }
    if (selectedSeats.length === 0) { showMsg("booking-msg", "Pilih minimal 1 kursi!", "error"); return; }
    if (!nama)                      { showMsg("booking-msg", "Nama tidak boleh kosong!", "error"); return; }

    const btn = document.getElementById("btn-booking");
    btn.disabled = true;
    btn.textContent = "Memproses...";

    // PENTING: ambil data booking TERBARU dari server sebelum mengecek kursi.
    // Ini mencegah kursi yang sudah dipesan orang lain (sejak halaman terakhir
    // dimuat) ikut terpilih lagi.
    await loadBookings();

    const sudahTerisi = bookedSeatsForFilm(currentFilm.judul).map(b => b.kursi);
    const kursiValid  = selectedSeats.filter(k => !sudahTerisi.includes(k));
    const kursiBentrok = selectedSeats.filter(k => sudahTerisi.includes(k));

    let berhasil = 0;
    const idTiketBaru = [];

    // Kirim booking SATU PER SATU secara berurutan (bukan paralel), supaya
    // tidak ada 2 request masuk bersamaan dan lolos pengecekan kursi bentrok
    // di server pada saat yang sama (race condition).
    for (const kursi of kursiValid) {
        const { ok, text } = await apiPostForm("/api/bookings", {
            nama,
            film:   currentFilm.judul,
            jadwal: currentFilm.jadwal,
            studio: currentFilm.studio,
            kursi,
            harga:  currentFilm.harga,
        });

        const hasil = parseStatusLine(text);
        if (ok && hasil.status === "ok") {
            berhasil++;
            idTiketBaru.push(hasil.value);
        }
    }

    btn.disabled = false;
    btn.textContent = "Pesan Sekarang";

    // Refresh ulang state booking supaya seat map langsung menampilkan
    // kursi yang baru saja terisi sebagai warna "booked".
    await loadBookings();

    selectedSeats = [];
    document.getElementById("input-nama").value = "";
    updateSeatMap();
    updateFilmPreview(currentFilm);
    renderTicketList();

    if (kursiBentrok.length > 0) {
        showMsg(
            "booking-msg",
            `⚠️ Kursi ${kursiBentrok.join(", ")} sudah terisi duluan. ${berhasil} tiket lainnya berhasil dipesan.`,
            berhasil > 0 ? "success" : "error"
        );
    } else if (berhasil > 0) {
        const totalHarga = currentFilm.harga * berhasil;
        showMsg("booking-msg", `✅ ${berhasil} tiket berhasil dipesan! Total: ${formatRupiah(totalHarga)}`, "success");
    } else {
        showMsg("booking-msg", "Booking gagal. Coba refresh halaman.", "error");
    }

    if (idTiketBaru.length > 0) {
        // Tampilkan karcis untuk tiket pertama yang berhasil dipesan
        showTicketModal(idTiketBaru[0]);
    }
});

// ===================== CANCEL & UNDO =====================

async function cancelBooking(idTiket) {
    const { ok } = await apiDelete("/api/bookings/" + idTiket);
    if (!ok) return;

    await loadBookings();
    renderTicketList();

    if (currentFilm) {
        updateSeatMap();
        updateFilmPreview(currentFilm);
    }

    showUndoBanner(idTiket);
}

function showUndoBanner(idTiket) {
    const old = document.getElementById("undo-banner");
    if (old) old.remove();

    const banner = document.createElement("div");
    banner.id = "undo-banner";
    banner.style.cssText = `
        position: fixed; bottom: 28px; left: 50%; transform: translateX(-50%);
        background: #1e1e1e; border: 1px solid var(--accent, #e8c84a);
        color: #f0ede6; padding: 14px 24px; border-radius: 8px;
        font-size: 13px; display: flex; align-items: center; gap: 16px;
        box-shadow: 0 8px 32px rgba(0,0,0,0.5); z-index: 999; white-space: nowrap;
    `;
    banner.innerHTML = `
        <span>Booking <strong style="color:#e8c84a">${idTiket}</strong> dibatalkan.</span>
        <button id="btn-undo" style="
            background: #e8c84a; color: #0d0d0d; border: none;
            padding: 6px 16px; border-radius: 4px; font-weight:600;
            cursor: pointer; font-size: 12px; letter-spacing: 1px; text-transform: uppercase;
        ">UNDO</button>
    `;
    document.body.appendChild(banner);

    document.getElementById("btn-undo").addEventListener("click", async () => {
        await undoLastCancel();
        banner.remove();
    });

    setTimeout(() => { if (banner.parentNode) banner.remove(); }, 5000);
}

async function undoLastCancel() {
    const { ok } = await apiPostForm("/api/bookings/undo");
    if (!ok) return;

    await loadBookings();
    renderTicketList();

    if (currentFilm) {
        updateSeatMap();
        updateFilmPreview(currentFilm);
    }
}

// ===================== WAITING LIST =====================

function renderWaitingList() {
    const ol = document.getElementById("waiting-list-display");
    ol.innerHTML = "";
    waitingQueue.forEach(nama => {
        const li = document.createElement("li");
        li.textContent = nama;
        ol.appendChild(li);
    });
}

document.getElementById("btn-add-waiting").addEventListener("click", async () => {
    const nama = document.getElementById("input-waiting").value.trim();
    if (!nama) { showMsg("waiting-msg", "Nama tidak boleh kosong!", "error"); return; }

    await apiPostForm("/api/waiting", { nama });
    await loadWaiting();

    document.getElementById("input-waiting").value = "";
    renderWaitingList();
    showMsg("waiting-msg", `${nama} berhasil masuk waiting list.`, "success");
});

document.getElementById("btn-call-waiting").addEventListener("click", async () => {
    const { ok, text } = await apiPostForm("/api/waiting/call");

    if (!ok) {
        showMsg("waiting-msg", "Waiting list kosong!", "error");
        return;
    }

    const hasil = parseStatusLine(text);

    await loadWaiting();
    renderWaitingList();
    showMsg("waiting-msg", `🎉 ${hasil.value} dipanggil! Silakan booking sekarang.`, "success");
});

// ===================== INIT =====================

async function init() {
    try {
        await Promise.all([loadFilms(), loadBookings(), loadWaiting()]);
        renderFilmGrid();
        renderTicketList();
        renderWaitingList();
    } catch (err) {
        console.error(err);
        document.getElementById("film-grid").innerHTML =
            '<p class="empty-state">Gagal terhubung ke server. Pastikan main.cpp sudah dijalankan di terminal.</p>';
    }
}

init();