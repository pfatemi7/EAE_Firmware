#include "can.h"
#include <iostream>

void CanBusSim::send(const CanFrame& frame) {
    std::lock_guard<std::mutex> lock(tx_mutex);
    tx_queue.push(frame);
}

std::unique_ptr<CanFrame> CanBusSim::recv() {
    std::lock_guard<std::mutex> lock(rx_mutex);
    
    if (rx_queue.empty()) {
        return nullptr;
    }
    
    auto frame = std::make_unique<CanFrame>(rx_queue.front());
    rx_queue.pop();
    return frame;
}

void CanBusSim::loopback() {
    std::lock_guard<std::mutex> tx_lock(tx_mutex);
    std::lock_guard<std::mutex> rx_lock(rx_mutex);
    
    // Move all TX frames to RX queue
    while (!tx_queue.empty()) {
        rx_queue.push(tx_queue.front());
        tx_queue.pop();
    }
}

bool CanBusSim::has_frames() const {
    std::lock_guard<std::mutex> lock(rx_mutex);
    return !rx_queue.empty();
}

size_t CanBusSim::tx_queue_size() const {
    std::lock_guard<std::mutex> lock(tx_mutex);
    return tx_queue.size();
}

size_t CanBusSim::rx_queue_size() const {
    std::lock_guard<std::mutex> lock(rx_mutex);
    return rx_queue.size();
}
