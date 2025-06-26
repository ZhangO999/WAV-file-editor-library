#include "SoundSegment_Simple.hpp"

namespace AudioEditor {

// ========== SimpleVector Implementation ==========

SimpleVector::SimpleVector() : data_(nullptr), size_(0), capacity_(0) {
}

SimpleVector::SimpleVector(size_t initial_size) : data_(nullptr), size_(0), capacity_(0) {
    resize(initial_size);
}

SimpleVector::SimpleVector(const SimpleVector& other) : data_(nullptr), size_(0), capacity_(0) {
    resize(other.size_);
    for (size_t i = 0; i < size_; ++i) {
        data_[i] = other.data_[i];
    }
}

SimpleVector& SimpleVector::operator=(const SimpleVector& other) {
    if (this != &other) {
        resize(other.size_);
        for (size_t i = 0; i < size_; ++i) {
            data_[i] = other.data_[i];
        }
    }
    return *this;
}

SimpleVector::~SimpleVector() {
    free(data_);
}

void SimpleVector::push_back(int16_t value) {
    if (size_ >= capacity_) {
        size_t new_capacity = capacity_ == 0 ? 1 : capacity_ * 2;
        int16_t* new_data = (int16_t*)realloc(data_, new_capacity * sizeof(int16_t));
        if (!new_data) {
            fprintf(stderr, "Error: Failed to allocate memory\n");
            exit(EXIT_FAILURE);
        }
        data_ = new_data;
        capacity_ = new_capacity;
    }
    data_[size_++] = value;
}

void SimpleVector::resize(size_t new_size) {
    if (new_size > capacity_) {
        int16_t* new_data = (int16_t*)realloc(data_, new_size * sizeof(int16_t));
        if (!new_data) {
            fprintf(stderr, "Error: Failed to allocate memory\n");
            exit(EXIT_FAILURE);
        }
        data_ = new_data;
        capacity_ = new_size;
    }
    
    // Initialize new elements to zero
    for (size_t i = size_; i < new_size; ++i) {
        data_[i] = 0;
    }
    
    size_ = new_size;
}

// ========== SimpleString Implementation ==========

SimpleString::SimpleString() : data_(nullptr), length_(0) {
}

SimpleString::SimpleString(const char* str) : data_(nullptr), length_(0) {
    if (str) {
        length_ = strlen(str);
        data_ = (char*)malloc(length_ + 1);
        if (!data_) {
            fprintf(stderr, "Error: Failed to allocate memory\n");
            exit(EXIT_FAILURE);
        }
        strcpy(data_, str);
    }
}

SimpleString::SimpleString(const SimpleString& other) : data_(nullptr), length_(0) {
    if (other.data_) {
        length_ = other.length_;
        data_ = (char*)malloc(length_ + 1);
        if (!data_) {
            fprintf(stderr, "Error: Failed to allocate memory\n");
            exit(EXIT_FAILURE);
        }
        strcpy(data_, other.data_);
    }
}

SimpleString& SimpleString::operator=(const SimpleString& other) {
    if (this != &other) {
        free(data_);
        data_ = nullptr;
        length_ = 0;
        
        if (other.data_) {
            length_ = other.length_;
            data_ = (char*)malloc(length_ + 1);
            if (!data_) {
                fprintf(stderr, "Error: Failed to allocate memory\n");
                exit(EXIT_FAILURE);
            }
            strcpy(data_, other.data_);
        }
    }
    return *this;
}

SimpleString::~SimpleString() {
    free(data_);
}

SimpleString& SimpleString::operator+=(const SimpleString& other) {
    if (other.data_ && other.length_ > 0) {
        size_t new_length = length_ + other.length_;
        char* new_data = (char*)realloc(data_, new_length + 1);
        if (!new_data) {
            fprintf(stderr, "Error: Failed to allocate memory\n");
            exit(EXIT_FAILURE);
        }
        data_ = new_data;
        strcpy(data_ + length_, other.data_);
        length_ = new_length;
    }
    return *this;
}

bool SimpleString::operator==(const SimpleString& other) const {
    if (length_ != other.length_) return false;
    if (!data_ && !other.data_) return true;
    if (!data_ || !other.data_) return false;
    return strcmp(data_, other.data_) == 0;
}

// ========== SegmentNode Implementation ==========

SegmentNode::SegmentNode() 
    : data(nullptr), offset(0), length(0), global_start(0), next(nullptr), is_buffer_owner(false) {
}

SegmentNode::SegmentNode(int16_t* data_ptr, size_t offset, size_t len)
    : data(data_ptr), offset(offset), length(len), global_start(0), next(nullptr), is_buffer_owner(false) {
}

SegmentNode::~SegmentNode() {
    if (is_buffer_owner) {
        free(data);
    }
}

// ========== WavIO Implementation ==========

SimpleVector WavIO::load(const char* filename) {
    SimpleVector samples;
    
    FILE* fp = fopen(filename, "rb");
    if (!fp) {
        perror("Error opening file");
        return samples;
    }
    
    // Skip WAV header
    fseek(fp, WAV_HEADER_SIZE, SEEK_SET);
    
    // Get file size
    fseek(fp, 0, SEEK_END);
    long file_size = ftell(fp);
    fseek(fp, WAV_HEADER_SIZE, SEEK_SET);
    
    long data_size = file_size - WAV_HEADER_SIZE;
    size_t num_samples = data_size / BYTES_PER_SAMPLE;
    
    samples.resize(num_samples);
    fread(samples.data(), BYTES_PER_SAMPLE, num_samples, fp);
    
    fclose(fp);
    return samples;
}

void WavIO::save(const char* filename, const SimpleVector& samples) {
    FILE* fp = fopen(filename, "wb");
    if (!fp) {
        perror("Error opening file");
        return;
    }
    
    uint32_t subchunk2_size = samples.size() * BYTES_PER_SAMPLE;
    uint32_t chunk_size = 36 + subchunk2_size;
    uint32_t byte_rate = SAMPLE_RATE * NUM_CHANNELS * BITS_PER_SAMPLE / 8;
    uint16_t block_align = NUM_CHANNELS * BITS_PER_SAMPLE / 8;
    
    // Write WAV header
    fwrite("RIFF", 1, 4, fp);
    fwrite(&chunk_size, 4, 1, fp);
    fwrite("WAVE", 1, 4, fp);
    fwrite("fmt ", 1, 4, fp);
    fwrite(&PCM_HEADER_SIZE, 4, 1, fp);
    fwrite(&PCM_FORMAT, 2, 1, fp);
    fwrite(&NUM_CHANNELS, 2, 1, fp);
    fwrite(&SAMPLE_RATE, 4, 1, fp);
    fwrite(&byte_rate, 4, 1, fp);
    fwrite(&block_align, 2, 1, fp);
    fwrite(&BITS_PER_SAMPLE, 2, 1, fp);
    fwrite("data", 1, 4, fp);
    fwrite(&subchunk2_size, 4, 1, fp);
    
    // Write audio data
    fwrite(samples.data(), BYTES_PER_SAMPLE, samples.size(), fp);
    
    fclose(fp);
}

// ========== SoundSegment Implementation ==========

SoundSegment::SoundSegment() : head(nullptr), total_length(0) {
}

SoundSegment::~SoundSegment() {
    SegmentNode* current = head;
    while (current) {
        SegmentNode* next = current->next;
        delete current;
        current = next;
    }
}

void SoundSegment::updateGlobalIndices() {
    size_t global_pos = 0;
    SegmentNode* current = head;
    
    while (current) {
        current->global_start = global_pos;
        global_pos += current->length;
        current = current->next;
    }
    
    total_length = global_pos;
}

SegmentNode* SoundSegment::findSegmentAt(size_t pos, size_t& local_offset) const {
    SegmentNode* current = head;
    
    while (current) {
        if (pos >= current->global_start && pos < current->global_start + current->length) {
            local_offset = pos - current->global_start;
            return current;
        }
        current = current->next;
    }
    
    return nullptr;
}

SegmentNode* SoundSegment::createSegment(int16_t* data, size_t offset, size_t len) {
    return new SegmentNode(data, offset, len);
}

size_t SoundSegment::length() const {
    return total_length;
}

void SoundSegment::read(SimpleVector& dest, size_t start_pos, size_t len) const {
    dest.resize(len);
    read(dest.data(), start_pos, len);
}

void SoundSegment::read(int16_t* dest, size_t start_pos, size_t len) const {
    if (!dest) return;

    size_t samples_copied = 0;
    SegmentNode* current = head;

    while (current && samples_copied < len) {
        if (start_pos >= current->global_start && 
            start_pos < current->global_start + current->length) {
            
            size_t local_index = start_pos - current->global_start;
            size_t samples_in_node = current->length - local_index;
            size_t samples_remaining = len - samples_copied;
            size_t samples_to_copy = (samples_remaining < samples_in_node) 
                                    ? samples_remaining 
                                    : samples_in_node;

            memcpy(dest + samples_copied,
                   current->data + current->offset + local_index,
                   samples_to_copy * BYTES_PER_SAMPLE);

            samples_copied += samples_to_copy;
            start_pos += samples_to_copy;
        }
        current = current->next;
    }
}

void SoundSegment::write(const SimpleVector& src, size_t pos) {
    write(src.data(), pos, src.size());
}

void SoundSegment::write(const int16_t* src, size_t pos, size_t len) {
    if (!src || len == 0) return;

    size_t end_pos = pos + len;

    // Extend track if necessary
    if (end_pos > total_length) {
        SegmentNode* last = head;

        if (!last) {
            // Create first node
            int16_t* data_buffer = (int16_t*)malloc(end_pos * BYTES_PER_SAMPLE);
            if (!data_buffer) {
                fprintf(stderr, "Error: Failed to allocate sample buffer\n");
                exit(EXIT_FAILURE);
            }
            memset(data_buffer, 0, end_pos * BYTES_PER_SAMPLE);
            
            last = new SegmentNode(data_buffer, 0, end_pos);
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
                int16_t* data_buffer = (int16_t*)malloc(new_node_len * BYTES_PER_SAMPLE);
                if (!data_buffer) {
                    fprintf(stderr, "Error: Failed to allocate sample buffer\n");
                    exit(EXIT_FAILURE);
                }
                memset(data_buffer, 0, new_node_len * BYTES_PER_SAMPLE);
                
                SegmentNode* new_node = new SegmentNode(data_buffer, 0, new_node_len);
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
    SegmentNode* current = head;

    while (current && remaining > 0) {
        if (global_index >= current->global_start && 
            global_index < current->global_start + current->length) {
            
            size_t local_offset = global_index - current->global_start;
            size_t available = current->length - local_offset;
            size_t to_write = (remaining < available) ? remaining : available;

            memcpy(current->data + current->offset + local_offset,
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

    // Find segments and perform deletion (simplified version)
    size_t remaining = len;
    size_t cur_pos = pos;
    
    while (remaining > 0) {
        size_t local_off;
        SegmentNode* current = findSegmentAt(cur_pos, local_off);
        if (!current) break;
        
        size_t available = current->length - local_off;
        size_t to_delete = (remaining < available) ? remaining : available;
        
        // Shift data left
        memmove(current->data + current->offset + local_off,
                current->data + current->offset + local_off + to_delete,
                (available - to_delete) * BYTES_PER_SAMPLE);
        
        current->length -= to_delete;
        remaining -= to_delete;
    }

    updateGlobalIndices();
    return true;
}

SimpleString SoundSegment::identify(const SoundSegment& ad) const {
    if (total_length == 0 || ad.total_length == 0 || total_length < ad.total_length) {
        return SimpleString("");
    }

    // Get all samples for cross-correlation
    SimpleVector target_samples = getAllSamples();
    SimpleVector ad_samples = ad.getAllSamples();

    // Calculate auto-correlation reference value
    double auto_ref = 0.0;
    for (size_t j = 0; j < ad_samples.size(); j++) {
        auto_ref += (double)ad_samples[j] * ad_samples[j];
    }
    double threshold = CORRELATION_THRESHOLD * auto_ref;

    // Find occurrences
    SimpleString result;
    size_t i = 0;
    while (i <= target_samples.size() - ad_samples.size()) {
        double corr = 0.0;
        for (size_t j = 0; j < ad_samples.size(); j++) {
            corr += (double)target_samples[i + j] * ad_samples[j];
        }
        
        if (corr >= threshold) {
            char buffer[64];
            snprintf(buffer, sizeof(buffer), "%zu,%zu", i, i + ad_samples.size() - 1);
            
            if (!result.empty()) {
                result += SimpleString("\n");
            }
            result += SimpleString(buffer);
            
            i += ad_samples.size();  // Skip ahead
        } else {
            i++;
        }
    }

    return result;
}

void SoundSegment::insert(const SoundSegment& src_track, size_t dest_pos, size_t src_pos, size_t len) {
    // Simplified implementation - copy data instead of sharing
    SimpleVector src_data = src_track.getAllSamples();
    if (src_pos >= src_data.size()) return;
    
    size_t actual_len = (src_pos + len > src_data.size()) ? src_data.size() - src_pos : len;
    
    // Create insertion data
    SimpleVector insert_data;
    for (size_t i = 0; i < actual_len; ++i) {
        insert_data.push_back(src_data[src_pos + i]);
    }
    
    // Get current destination data
    SimpleVector dest_data = getAllSamples();
    
    // Insert the data
    SimpleVector new_data;
    for (size_t i = 0; i < dest_pos && i < dest_data.size(); ++i) {
        new_data.push_back(dest_data[i]);
    }
    for (size_t i = 0; i < insert_data.size(); ++i) {
        new_data.push_back(insert_data[i]);
    }
    for (size_t i = dest_pos; i < dest_data.size(); ++i) {
        new_data.push_back(dest_data[i]);
    }
    
    // Replace current track with new data
    // Clear existing segments
    SegmentNode* current = head;
    while (current) {
        SegmentNode* next = current->next;
        delete current;
        current = next;
    }
    head = nullptr;
    total_length = 0;
    
    // Write new data
    write(new_data, 0);
}

void SoundSegment::loadFromWav(const char* filename) {
    SimpleVector samples = WavIO::load(filename);
    write(samples, 0);
}

void SoundSegment::saveToWav(const char* filename) const {
    SimpleVector samples = getAllSamples();
    WavIO::save(filename, samples);
}

void SoundSegment::printTrack() const {
    printf("Track (total_length=%zu):\n", total_length);
    SegmentNode* current = head;
    
    while (current) {
        printf("[ ");
        for (size_t i = 0; i < current->length && i < 10; ++i) {
            printf("%d ", current->data[current->offset + i]);
        }
        if (current->length > 10) {
            printf("... ");
        }
        printf("](start: %zu, len: %zu) ", current->global_start, current->length);
        current = current->next;
    }
    printf("\n");
}

SimpleVector SoundSegment::getAllSamples() const {
    SimpleVector result(total_length);
    read(result.data(), 0, total_length);
    return result;
}

SoundSegment* SoundSegment::create() {
    return new SoundSegment();
}

}  // namespace AudioEditor 