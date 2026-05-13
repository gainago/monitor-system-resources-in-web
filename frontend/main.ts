// --- Типы данных от сервера ---
interface SystemData {
    cpu_percent: number;
    ram_used_kb: number;
    ram_total_kb: number;
    ram_percent: number;
    load_avg: number[];
    uptime_seconds: number;
    timestamp_ms: number;
}

interface TableColumn {
    key: string;
    label: string;
}

interface ProcessRow {
    pid: number;
    user: string;
    priority: number;
    nice: number;
    virt_kb: number;
    res_kb: number;
    shr_kb: number;
    state: string;
    cpu_percent: number;
    mem_percent: number;
    time_ticks: string;   // уже отформатировано MM:SS.hh
    command: string;
}

interface ServerMessage {
    system: SystemData;
    table: {
        columns: TableColumn[];
        rows: ProcessRow[];
    };
}

// --- Глобальные ссылки на DOM ---
const cpuSpan = document.getElementById('cpu') as HTMLSpanElement;
const ramSpan = document.getElementById('ram') as HTMLSpanElement;
const loadSpan = document.getElementById('load') as HTMLSpanElement;
const thead = document.querySelector('#processes thead') as HTMLTableSectionElement;
const tbody = document.querySelector('#processes tbody') as HTMLTableSectionElement;

let columns: TableColumn[] = [];

// --- Форматирование размеров (КБ -> читаемый вид) ---
function formatKb(kb: number): string {
    if (kb >= 1024 * 1024) return (kb / (1024 * 1024)).toFixed(1) + 'G';
    if (kb >= 1024) return (kb / 1024).toFixed(1) + 'M';
    return kb + 'K';
}

// --- Рендер заголовка таблицы (вызывается один раз при получении columns) ---
function renderHeader(cols: TableColumn[]) {
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
function renderRow(row: ProcessRow) {
    const tr = document.createElement('tr');
    columns.forEach(col => {
        const td = document.createElement('td');
        const key = col.key;
        let val: any = (row as any)[key];
        if (key === 'virt_kb' || key === 'res_kb' || key === 'shr_kb') {
            td.textContent = formatKb(val as number);
        } else {
            td.textContent = String(val);
        }
        tr.appendChild(td);
    });
    tbody.appendChild(tr);
}

// --- Обновление UI ---
function update(data: ServerMessage) {
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
evtSource.onmessage = (event: MessageEvent) => {
    try {
        const msg: ServerMessage = JSON.parse(event.data);
        update(msg);
    } catch (e) {
        console.error('JSON parse error', e);
    }
};
evtSource.onerror = (err) => {
    console.error('SSE error', err);
};