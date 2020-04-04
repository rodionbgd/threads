#include <iostream>
#include <chrono>
#include <thread>
#include <mutex>
#include <queue>
#include <windows.h>
#include <iomanip>

std::mutex g_lock;
std::queue<int> data;

void write(int &wr_per_second) {
    while (true) {
        g_lock.lock();
        data.push(wr_per_second); // произвольное сообщение
        wr_per_second++;
        g_lock.unlock();
    }
}

void read(int &rd_per_second) {
    while (true) {
        if (!data.empty()) {
            g_lock.lock();
            data.pop();
            rd_per_second++;
            g_lock.unlock();
        }
    }

}

double elapsedseconds(std::chrono::time_point<std::chrono::system_clock> starttime) {
    std::chrono::time_point<std::chrono::system_clock> endtime;
    endtime = std::chrono::system_clock::now();
    return double(std::chrono::duration_cast<std::chrono::milliseconds>(endtime - starttime).count()) / 1000;
}

int main() {
    int wr_per_second = 0;
    int rd_per_second = 0;
    auto starttime = std::chrono::system_clock::now();
    std::thread thr_write(write, std::ref(wr_per_second));
    std::thread thr_read(read, std::ref(rd_per_second));
    int tmp_wr = wr_per_second;
    int tmp_rd = rd_per_second;
    while (elapsedseconds(starttime) < 120) {
        auto curtime = std::chrono::time_point<std::chrono::system_clock>(std::chrono::system_clock::now() - starttime);
        std::time_t t = std::chrono::system_clock::to_time_t(curtime);
        std::cout << std::put_time(std::localtime(&t), "%M:%S") << " "; // таймер с миллисекундами нереально отследить

        std::cout << tmp_wr - tmp_rd << std::endl;
        tmp_wr = wr_per_second - tmp_wr; // расчет кол-ва записанных/считанных сообщений за прошедшую секунду
        tmp_rd = rd_per_second - tmp_rd;
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    thr_write.join();
    thr_read.join();
    return 0;
}
