#include <iostream>
#include <chrono>
#include <thread>
#include <mutex>
#include <queue>
#include <iomanip>

std::mutex g_lock;
std::queue<int> data;

void write(int &wr_per_second, bool &iswork) {
    while (iswork) {
        const std::lock_guard<std::mutex> lock(g_lock);
        data.push(wr_per_second); // произвольное сообщение
        wr_per_second++;
    }
}

void read(int &rd_per_second, bool &iswork, int max) {
    while (iswork) {
        while (rd_per_second < max && iswork) {
            if (!data.empty()) {
                const std::lock_guard<std::mutex> lock(g_lock);
                data.pop();
                rd_per_second++;
            }
        }
    }

}

double elapsedseconds(std::chrono::time_point<std::chrono::system_clock> starttime) {
    std::chrono::time_point<std::chrono::system_clock> endtime;
    endtime = std::chrono::system_clock::now();
    return double(std::chrono::duration_cast<std::chrono::milliseconds>(endtime - starttime).count()) / 1000;
}

int main() {
    int wr_per_second = 0,
            rd_per_second = 0,
            tmp_wr, tmp_rd;
    int max_msg = 200000; // макс кол-во считанных сообщений/сек
    bool iswork = true; // флаг завершения работы потока
    int worktime = 25;

    auto starttime = std::chrono::system_clock::now();

    std::thread thr_write(write, std::ref(wr_per_second), std::ref(iswork));
    std::thread thr_read(read, std::ref(rd_per_second), std::ref(iswork), max_msg);
    tmp_wr = wr_per_second;
    tmp_rd = rd_per_second;

    while (elapsedseconds(starttime) < worktime) {
        auto curtime = std::chrono::time_point<std::chrono::system_clock>(std::chrono::system_clock::now() - starttime);
        std::time_t t = std::chrono::system_clock::to_time_t(curtime);
        std::cout << std::put_time(std::localtime(&t), "%M:%S") << " "; // таймер с миллисекундами нереально отследить

        std::cout << "read msg = " << tmp_rd << " delta = " << tmp_wr - tmp_rd << std::endl;
        tmp_wr = wr_per_second - tmp_wr; // расчет кол-ва записанных/считанных сообщений за прошедшую секунду
        tmp_rd = rd_per_second;
        rd_per_second = 0;
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    iswork = false;
    thr_write.join();
    thr_read.join();

    return 0;
}
