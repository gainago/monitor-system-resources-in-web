"use strict";
// --- Глобальные ссылки на DOM ---
const cpuSpan = document.getElementById('cpu');
const ramSpan = document.getElementById('ram');
const loadSpan = document.getElementById('load');
const thead = document.querySelector('#processes thead');
const tbody = document.querySelector('#processes tbody');
let columns = [];
// --- Форматирование размеров (КБ -> читаемый вид) ---
function formatKb(kb) {
    if (kb >= 1024 * 1024)
        return (kb / (1024 * 1024)).toFixed(1) + 'G';
    if (kb >= 1024)
        return (kb / 1024).toFixed(1) + 'M';
    return kb + 'K';
}
// --- Рендер заголовка таблицы (вызывается один раз при получении columns) ---
function renderHeader(cols) {
    const tr = document.createElement('tr');
    cols.forEach(col => {
        const th = document.createElement('th');
        th.textContent = col.label;
        tr.appendChild(th);
    });
    thead.innerHTML = '';
    thead.appendChild(tr);
}
// --- Рендер одной строки ---
function renderRow(row) {
    const tr = document.createElement('tr');
    columns.forEach(col => {
        const td = document.createElement('td');
        const key = col.key;
        let val = row[key];
        if (key === 'virt_kb' || key === 'res_kb' || key === 'shr_kb') {
            td.textContent = formatKb(val);
        }
        else {
            td.textContent = String(val);
        }
        tr.appendChild(td);
    });
    tbody.appendChild(tr);
}
// --- Обновление UI ---
function update(data) {
    cpuSpan.textContent = data.system.cpu_percent.toFixed(1);
    ramSpan.textContent = data.system.ram_percent.toFixed(1);
    loadSpan.textContent = data.system.load_avg[0].toFixed(2);
    if (columns.length === 0 && data.table.columns.length > 0) {
        columns = data.table.columns;
        renderHeader(columns);
    }
    tbody.innerHTML = '';
    data.table.rows.forEach(row => renderRow(row));
}
// --- Подключение к SSE ---
const evtSource = new EventSource('/api/stream');
evtSource.onmessage = (event) => {
    try {
        const msg = JSON.parse(event.data);
        update(msg);
    }
    catch (e) {
        console.error('JSON parse error', e);
    }
};
evtSource.onerror = (err) => {
    console.error('SSE error', err);
};
//# sourceMappingURL=main.js.map