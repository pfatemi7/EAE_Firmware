#pragma once

#include <cstdint>
#include <string>
#include <queue>
#include <mutex>
#include <memory>

/**
 * CAN Frame structure for simulation
 */
struct CanFrame {
    uint32_t id;
    std::string payload;
    
    CanFrame(uint32_t frame_id, const std::string& data) 
        : id(frame_id), payload(data) {}
};

/**
 * CAN Bus Simulator with loopback functionality
 * Thread-safe queue-based implementation
 */
class CanBusSim {
private:
    std::queue<CanFrame> tx_queue;
    std::queue<CanFrame> rx_queue;
    mutable std::mutex tx_mutex;
    mutable std::mutex rx_mutex;
    
public:
    /**
     * Send a frame to the bus
     * @param frame The CAN frame to send
     */
    void send(const CanFrame& frame);
    
    /**
     * Receive a frame from the bus
     * @return Pointer to frame if available, nullptr otherwise
     */
    std::unique_ptr<CanFrame> recv();
    
    /**
     * Process loopback - move all TX frames to RX queue
     */
    void loopback();
    
    /**
     * Check if RX queue has frames
     * @return true if frames are available
     */
    bool has_frames() const;
    
    /**
     * Get number of frames in TX queue
     * @return TX queue size
     */
    size_t tx_queue_size() const;
    
    /**
     * Get number of frames in RX queue
     * @return RX queue size
     */
    size_t rx_queue_size() const;
};
