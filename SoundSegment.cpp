#include "SoundSegment.hpp"
#include <iostream>
#include <fstream>
#include <string.h>
#include <algorithm>
#include <sstream>
#include <stdexcept>
#include <cmath>

namespace AudioEditor {

// ========== SegmentNode Implementation ==========

SegmentNode::SegmentNode() 
    : offset(0), length(0), global_start(0), is_buffer_owner(false) {
}

SegmentNode::SegmentNode(std::shared_ptr<std::vector<int16_t>> data_ptr, size_t offset, size_t len)
    : data(data_ptr), offset(offset), length(len), global_start(0), is_buffer_owner(false) {
}

void SegmentNode::addChild(std::shared_ptr<SegmentNode> child) {
    if (!child) return;
    children.push_back(std::weak_ptr<SegmentNode>(child));
}

void SegmentNode::addParent(std::shared_ptr<SegmentNode> parent) {
    if (!parent) return;
    parents.push_back(std::weak_ptr<SegmentNode>(parent));
}

bool SegmentNode::hasActiveChildren() const {
    for (const auto& weak_child : children) {
        if (!weak_child.expired()) {
            return true;
        }
    }
    return false;
}

// ========== WavIO Implementation ==========

std::vector<int16_t> WavIO::load(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Cannot open file: " + filename);
    }

    // Skip WAV header
    file.seekg(WAV_HEADER_SIZE);
    
    // Get file size
    file.seekg(0, std::ios::end);
    size_t file_size = file.tellg();
    file.seekg(WAV_HEADER_SIZE);
    
    size_t data_size = file_size - WAV_HEADER_SIZE;
    size_t num_samples = data_size / BYTES_PER_SAMPLE;
    
    std::vector<int16_t> samples(num_samples);
    file.read(reinterpret_cast<char*>(samples.data()), data_size);
    
    if (!file) {
        throw std::runtime_error("Error reading audio data from: " + filename);
    }
    
    return samples;
}

void WavIO::save(const std::string& filename, const std::vector<int16_t>& samples) {
    std::ofstream file(filename, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Cannot create file: " + filename);
    }

    uint32_t subchunk2_size = samples.size() * BYTES_PER_SAMPLE;
    uint32_t chunk_size = 36 + subchunk2_size;
    uint32_t byte_rate = SAMPLE_RATE * NUM_CHANNELS * BITS_PER_SAMPLE / 8;
    uint16_t block_align = NUM_CHANNELS * BITS_PER_SAMPLE / 8;

    // Write WAV header
    file.write("RIFF", 4);
    file.write(reinterpret_cast<const char*>(&chunk_size), 4);
    file.write("WAVE", 4);
    file.write("fmt ", 4);
    file.write(reinterpret_cast<const char*>(&PCM_HEADER_SIZE), 4);
    file.write(reinterpret_cast<const char*>(&PCM_FORMAT), 2);
    file.write(reinterpret_cast<const char*>(&NUM_CHANNELS), 2);
    file.write(reinterpret_cast<const char*>(&SAMPLE_RATE), 4);
    file.write(reinterpret_cast<const char*>(&byte_rate), 4);
    file.write(reinterpret_cast<const char*>(&block_align), 2);
    file.write(reinterpret_cast<const char*>(&BITS_PER_SAMPLE), 2);
    file.write("data", 4);
    file.write(reinterpret_cast<const char*>(&subchunk2_size), 4);

    // Write audio data
    file.write(reinterpret_cast<const char*>(samples.data()), subchunk2_size);
    
    if (!file) {
        throw std::runtime_error("Error writing to file: " + filename);
    }
}

// ========== SoundSegment Implementation ==========

SoundSegment::SoundSegment() : total_length(0) {
}

SoundSegment::SoundSegment(SoundSegment&& other) noexcept 
    : head(std::move(other.head)), total_length(other.total_length) {
    other.total_length = 0;
}

SoundSegment& SoundSegment::operator=(SoundSegment&& other) noexcept {
    if (this != &other) {
        head = std::move(other.head);
        total_length = other.total_length;
        other.total_length = 0;
    }
    return *this;
}

void SoundSegment::updateGlobalIndices() {
    size_t global_pos = 0;
    auto current = head;
    
    while (current) {
        current->global_start = global_pos;
        global_pos += current->length;
        current = current->next;
    }
    
    total_length = global_pos;
}

std::shared_ptr<SegmentNode> SoundSegment::findSegmentAt(size_t pos, size_t& local_offset) const {
    auto current = head;
    
    while (current) {
        if (pos >= current->global_start && pos < current->global_start + current->length) {
            local_offset = pos - current->global_start;
            return current;
        }
        current = current->next;
    }
    
    return nullptr;
}

std::shared_ptr<SegmentNode> SoundSegment::splitNode(std::shared_ptr<SegmentNode> node, size_t local_offset) {
    if (!node || local_offset >= node->length) {
        return nullptr;
    }

    auto right = std::make_shared<SegmentNode>(node->data, node->offset + local_offset, 
                                               node->length - local_offset);
    right->global_start = node->global_start + local_offset;
    right->next = node->next;
    node->next = right;
    node->length = local_offset;

    return right;
}

std::shared_ptr<SegmentNode> SoundSegment::createSegment(std::shared_ptr<std::vector<int16_t>> data, 
                                                         size_t offset, size_t len) {
    auto node = std::make_shared<SegmentNode>(data, offset, len);
    return node;
}

size_t SoundSegment::length() const {
    return total_length;
}

void SoundSegment::read(std::vector<int16_t>& dest, size_t start_pos, size_t len) const {
    dest.resize(len);
    read(dest.data(), start_pos, len);
}

void SoundSegment::read(int16_t* dest, size_t start_pos, size_t len) const {
    if (!dest) return;

    size_t samples_copied = 0;
    auto current = head;

    while (current && samples_copied < len) {
        if (start_pos >= current->global_start && 
            start_pos < current->global_start + current->length) {
            
            size_t local_index = start_pos - current->global_start;
            size_t samples_in_node = current->length - local_index;
            size_t samples_remaining = len - samples_copied;
            size_t samples_to_copy = std::min(samples_remaining, samples_in_node);

            std::memcpy(dest + samples_copied,
                       current->data->data() + current->offset + local_index,
                       samples_to_copy * BYTES_PER_SAMPLE);

            samples_copied += samples_to_copy;
            start_pos += samples_to_copy;
        }
        current = current->next;
    }
}

void SoundSegment::write(const std::vector<int16_t>& src, size_t pos) {
    write(src.data(), pos, src.size());
}

void SoundSegment::write(const int16_t* src, size_t pos, size_t len) {
    if (!src || len == 0) return;

    size_t end_pos = pos + len;

    // Extend track if necessary
    if (end_pos > total_length) {
        auto last = head;

        if (!last) {
            // Create first node
            auto data_buffer = std::make_shared<std::vector<int16_t>>(end_pos);
            last = std::make_shared<SegmentNode>(data_buffer, 0, end_pos);
            last->is_buffer_owner = true;
            head = last;
        } else {
            // Find last node
            while (last->next) {
                last = last->next;
            }

            size_t last_end = last->global_start + last->length;
            if (last_end < end_pos) {
                size_t new_node_len = end_pos - last_end;
                auto data_buffer = std::make_shared<std::vector<int16_t>>(new_node_len);
                auto new_node = std::make_shared<SegmentNode>(data_buffer, 0, new_node_len);
                new_node->global_start = last_end;
                new_node->is_buffer_owner = true;
                last->next = new_node;
            }
        }

        updateGlobalIndices();
    }

    // Write data to appropriate segments
    size_t remaining = len;
    size_t global_index = pos;
    size_t src_index = 0;
    auto current = head;

    while (current && remaining > 0) {
        if (global_index >= current->global_start && 
            global_index < current->global_start + current->length) {
            
            size_t local_offset = global_index - current->global_start;
            size_t available = current->length - local_offset;
            size_t to_write = std::min(remaining, available);

            std::memcpy(current->data->data() + current->offset + local_offset,
                       src + src_index,
                       to_write * BYTES_PER_SAMPLE);

            remaining -= to_write;
            src_index += to_write;
            global_index += to_write;
        }
        current = current->next;
    }
}

bool SoundSegment::deleteRange(size_t pos, size_t len) {
    if (pos + len > total_length) {
        return false;
    }

    // First pass: check if any segments in range have children
    size_t to_check = len;
    size_t cur_pos = pos;
    size_t local_off;
    auto current = findSegmentAt(pos, local_off);

    while (current && to_check > 0) {
        if (current->hasActiveChildren()) {
            return false;
        }

        size_t available = current->length - local_off;
        if (to_check > available) {
            to_check -= available;
            current = current->next;
            local_off = 0;
        } else {
            break;
        }
    }

    // Second pass: perform actual deletion
    size_t to_delete = len;
    current = findSegmentAt(pos, local_off);
    std::shared_ptr<SegmentNode> prev = nullptr;

    // Find previous node if not at head
    if (current != head) {
        prev = head;
        while (prev && prev->next != current) {
            prev = prev->next;
        }
    }

    while (to_delete > 0 && current) {
        size_t available = current->length - local_off;

        if (to_delete < available) {
            // Partial deletion within segment
            std::memmove(current->data->data() + current->offset + local_off,
                        current->data->data() + current->offset + local_off + to_delete,
                        (available - to_delete) * BYTES_PER_SAMPLE);
            current->length -= to_delete;
            to_delete = 0;
        } else {
            // Full deletion of segment or part of it
            to_delete -= available;
            current->length = local_off;

            if (current->length == 0) {
                // Remove empty segment from linked list
                auto temp = current;
                if (prev) {
                    prev->next = current->next;
                } else {
                    head = current->next;
                }
                current = current->next;
                local_off = 0;
                continue;
            }
        }

        if (to_delete > 0) {
            prev = current;
            current = current->next;
            local_off = 0;
        }
    }

    updateGlobalIndices();
    return true;
}

std::string SoundSegment::identify(const SoundSegment& ad) const {
    if (total_length == 0 || ad.total_length == 0 || total_length < ad.total_length) {
        return "";
    }

    // Get all samples for simplicity (in production, could optimize this)
    auto target_samples = getAllSamples();
    auto ad_samples = ad.getAllSamples();

    // Calculate auto-correlation reference value
    double auto_ref = 0.0;
    for (size_t j = 0; j < ad_samples.size(); j++) {
        auto_ref += static_cast<double>(ad_samples[j]) * ad_samples[j];
    }
    double threshold = CORRELATION_THRESHOLD * auto_ref;

    std::vector<std::pair<size_t, size_t>> occurrences;
    size_t i = 0;
    while (i <= target_samples.size() - ad_samples.size()) {
        double corr = 0.0;
        for (size_t j = 0; j < ad_samples.size(); j++) {
            corr += static_cast<double>(target_samples[i + j]) * ad_samples[j];
        }
        
        if (corr >= threshold) {
            occurrences.emplace_back(i, i + ad_samples.size() - 1);
            i += ad_samples.size();  // Skip ahead to avoid overlapping matches
        } else {
            i++;
        }
    }

    // Format result string
    std::ostringstream result;
    for (size_t i = 0; i < occurrences.size(); ++i) {
        if (i > 0) result << "\n";
        result << occurrences[i].first << "," << occurrences[i].second;
    }

    return result.str();
}

void SoundSegment::insert(const SoundSegment& src_track, size_t dest_pos, size_t src_pos, size_t len) {
    if (len == 0) return;

    size_t remaining = len;
    size_t current_global = src_pos;

    std::shared_ptr<SegmentNode> insertion_head = nullptr;
    std::shared_ptr<SegmentNode> insertion_tail = nullptr;

    // Build the list of segments to insert
    while (remaining > 0) {
        size_t local_off;
        auto cur_node = src_track.findSegmentAt(current_global, local_off);
        if (!cur_node) break;

        // Calculate how much of this segment we need
        size_t take = std::min(cur_node->length - local_off, remaining);

        // Create a new segment that shares data with the source segment
        auto clone = createSegment(cur_node->data, cur_node->offset + local_off, take);
        clone->addParent(cur_node);
        
        // Note: We would need to modify the const constraint to add child relationships
        // For now, we'll create a copy instead of sharing (safer approach)
        auto data_copy = std::make_shared<std::vector<int16_t>>(take);
        std::memcpy(data_copy->data(), 
                   cur_node->data->data() + cur_node->offset + local_off,
                   take * BYTES_PER_SAMPLE);
        clone = createSegment(data_copy, 0, take);
        clone->is_buffer_owner = true;

        if (!insertion_head) {
            insertion_head = clone;
            insertion_tail = clone;
        } else {
            insertion_tail->next = clone;
            insertion_tail = clone;
        }

        remaining -= take;
        current_global += take;
    }

    // Find where to insert in destination track
    size_t dest_local;
    auto dest_node = findSegmentAt(dest_pos, dest_local);
    
    if (dest_node && dest_local > 0) {
        dest_node = splitNode(dest_node, dest_local);
    }

    // Insert the new segments into the destination track
    if (!dest_node) {
        // Insert at end of track
        if (!head) {
            head = insertion_head;
        } else {
            auto tail = head;
            while (tail->next) {
                tail = tail->next;
            }
            tail->next = insertion_head;
        }
    } else {
        // Inserting in middle of track
        if (head == dest_node) {
            insertion_tail->next = dest_node;
            head = insertion_head;
        } else {
            auto prev = head;
            while (prev && prev->next != dest_node) {
                prev = prev->next;
            }
            if (prev) {
                prev->next = insertion_head;
                insertion_tail->next = dest_node;
            }
        }
    }
    
    updateGlobalIndices();
}

void SoundSegment::loadFromWav(const std::string& filename) {
    auto samples = WavIO::load(filename);
    write(samples, 0);
}

void SoundSegment::saveToWav(const std::string& filename) const {
    auto samples = getAllSamples();
    WavIO::save(filename, samples);
}

void SoundSegment::printTrack() const {
    std::cout << "Track (total_length=" << total_length << "):\n";
    auto current = head;
    
    while (current) {
        std::cout << "[ ";
        for (size_t i = 0; i < current->length && i < 10; ++i) {  // Limit output for readability
            std::cout << (*current->data)[current->offset + i] << " ";
        }
        if (current->length > 10) {
            std::cout << "... ";
        }
        std::cout << "](start: " << current->global_start << ", len: " << current->length << ") ";
        current = current->next;
    }
    std::cout << "\n";
}

std::vector<int16_t> SoundSegment::getAllSamples() const {
    std::vector<int16_t> result(total_length);
    read(result.data(), 0, total_length);
    return result;
}

std::unique_ptr<SoundSegment> SoundSegment::create() {
    return std::make_unique<SoundSegment>();
}

}  // namespace AudioEditor 