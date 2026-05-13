#include "MetricsCollector.h"
#include "ProcReader.h"
#include "MetricsSorter.h"
#include "FrontendTableBuilder.h"
#include "httplib.h"
#include <iostream>
#include <memory>

// Вспомогательная функция для чтения файла в строку
static std::string readFile(const std::string& path) {
    std::ifstream f(path, std::ios::binary);
    if (!f) return "";
    std::stringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

int startServer() {
    using namespace sysmon;

    // ---------- Сборщик метрик ----------
    auto reader = std::make_unique<ProcReader>();
    unsigned int cores = reader->getCpuCount();
    std::cout << "Detected " << cores << " CPU cores\n";
    MetricsCollector collector(std::move(reader), cores);

    // ---------- HTTP-сервер ----------
    httplib::Server svr;

    svr.Get("/", [](const httplib::Request&, httplib::Response& res) {
        std::string content = readFile("../frontend/public/index.html");
        if (content.empty()) {
            res.status = 404;
            res.set_content("index.html not found", "text/plain");
        } else {
            res.set_content(content, "text/html");
        }
    });

    svr.Get("/main.js", [](const httplib::Request&, httplib::Response& res) {
        std::string content = readFile("../frontend/public/main.js");
        if (content.empty()) {
            res.status = 404;
            res.set_content("main.js not found", "text/plain");
        } else {
            res.set_content(content, "application/javascript");
        }
    });

    svr.Get("/style.css", [](const httplib::Request&, httplib::Response& res) {
        std::string content = readFile("../frontend/public/style.css");
        if (content.empty()) {
            res.status = 404;
            res.set_content("style.css not found", "text/plain");
        } else {
            res.set_content(content, "text/css");
        }
    });

    // SSE-поток. 
    svr.Get("/api/stream", [&](const httplib::Request&, httplib::Response& res) {
        res.set_content_provider(
            "text/event-stream", // Специальная строка, получил которую библиотека начинает работать по протоколу SSE
            [&](size_t, httplib::DataSink& sink) {
                // Первый вызов нужен, чтобы заполнить кэш CPU-дельты, но не отдаём его
                collector.collect();
                while (sink.is_writable()) {
                    FullSystemSnapshot snapshot = collector.collect(); // ~1 сек

                    // Сортируем процессы по CPU% убыванию
                    auto sorted = MetricsSorter::sort(snapshot.processes,
                                                      SortField::CPU_PERCENT,
                                                      SortOrder::DESC);
                    snapshot.processes = sorted;

                    std::string json = FrontendTableBuilder::build(snapshot);
                    std::string event = "data: " + json + "\n\n";
                    sink.write(event.data(), event.size());
                }
                return true;
            },
            [](bool) {} // не вызываем функцию при закрытии соединения
        );
    });

    std::cout << "Server listening on http://0.0.0.0:8085\n";
    svr.listen("0.0.0.0", 8085);
    std::cout << "Server stop listening\n";

    return 0;
}