//
//  stack_string.h
//
//  Created by Oleksandr Kysil@HurricaneEngine3D on 28.02.2021.
//

#pragma once

template<size_t size>
class stack_string {
public:
    const char* c_str() {
        return m_buffer;
    }
    
    stack_string() {
        m_buffer[0] = '\0';
    }
    
    explicit stack_string(const char* cstring) {
        assert(cstring);
        if (strlen(cstring) <= size) {
            strcpy(m_buffer, cstring);
        } else {
            // Reached the buffer's size, clean buffer
            m_buffer[0] = '\0';
        }
    }
    
    stack_string(const stack_string &stackString) {
        stack_string(stackString.m_buffer);
    }
    
    stack_string(stack_string &&other) {
        std::swap(m_buffer, other.m_buffer);
    }
    
    ~stack_string() {
        m_buffer[0] = '\0';
    }
    
    const stack_string &operator=(const char* cstring) {
        assert(is_valid());
        if (cstring != this->m_buffer) {
            strcpy(m_buffer, cstring);
        }
        
        return *this;
    }
    
    const stack_string &operator=(stack_string const& other) {
        if (&other != this) {
            strcpy(m_buffer, other.m_buffer);
        }
        
        return *this;
    }
    
    const stack_string &operator=(stack_string&& that) {
        if (&that != this) {
            std::swap(*this, that);
        }
        
        return *this;
    }
    
    const stack_string &operator+(stack_string const& cstring) {
        return operator+(cstring.m_buffer);
    }
    
    const stack_string &operator+(const char* cstring) {
        assert(is_valid());
        const size_t current_size_array = strlen(m_buffer);
        const size_t appended_size_array = strlen(cstring);
        
        if (appended_size_array + current_size_array + 1 > size) {
            assert(false);
            return *this;
        }
        strcat(m_buffer, cstring);
        return *this;
    }
    
    size_t find_index(const char* cstring) const {
        assert(is_valid());
        if ((cstring) && (cstring[0] == '\0')) {
            return -1;
        }
        
        const char* first_occurence = strstr(m_buffer, cstring);
        
        if (first_occurence) {
            return first_occurence-m_buffer;
        }
        
        return -1;
    }
    
    size_t find_index(const stack_string &stackString) const {
        return find_index(stackString.m_buffer);
    }
    
    void append(const char* cstring) {
        assert(is_valid());
        const size_t current_size_array = strlen(m_buffer);
        const size_t appended_size_array = strlen(cstring);
        
        if (appended_size_array + current_size_array + 1 > size) {
            assert(false);
            return;
        }
        
        strcpy(m_buffer+current_size_array, cstring);
    }
    
    void append(const stack_string &stackString) {
        append(stackString.m_buffer);
    }
    
    void insert(const size_t pos, const char* cstring) {
        assert(is_valid());
        const size_t current_size_string = strlen(m_buffer);
        const size_t appended_size_array = strlen(cstring);
        
        char temp[size];
        
        if (current_size_string + appended_size_array + 1 > size) {
            assert(false);
            return;
        }

        strcpy(temp, m_buffer+pos);
        strcpy(m_buffer+pos, cstring);
        strcpy(m_buffer+pos+appended_size_array, temp);
    }
    
    void insert(const size_t pos, const stack_string &stackString) {
        insert(pos, stackString.m_buffer);
    }
    
    void replace(const size_t from, const char* cstring) {
        assert(is_valid());
        strcpy(m_buffer+from, cstring);
    }
    
    void replace(const size_t from, const stack_string &stackString) {
        strcpy(m_buffer+from, stackString.m_buffer);
    }
    
    void erase(const size_t pos, const size_t length) {
        if (pos == 0 && length == size) {
            // Delete whole buffer
            m_buffer[pos] = '\0';
        } else {
            char temp[size];
            
            strcpy(temp, m_buffer+pos+length);
            strcpy(m_buffer+pos, temp);
        }
    }
    
    bool is_valid() const {
        return m_buffer && strlen(m_buffer) < size;
    }
    
    bool empty() const {
        assert(is_valid());
        return m_buffer && m_buffer[0] == '\0';
    }
    
    size_t lenght() const {
        assert(is_valid());
        return strlen(m_buffer);
    }
    
private:
    char m_buffer[size];
};