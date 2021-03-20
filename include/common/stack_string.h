//
//  stack_string.h
//
//  Created by Oleksandr Kysil@HurricaneEngine3D on 28.02.2021.
//

#pragma once

#ifndef MAX_CHARACTER
#define MAX_CHARACTER 256
#endif

class stack_string {
public:
    const char* c_str() {
        return m_buffer;
    }
    
    /*
         Constructors
    */
    
    stack_string() {
        m_buffer[0] = '\0';
    }
    
    // Create
    stack_string(char* cstring) {
        if (strlen(cstring) <= MAX_CHARACTER) {
            strcpy(m_buffer, cstring);
        }
    }
    
    // Copy object
    stack_string(const stack_string &stackString) {
        strcpy(m_buffer, stackString.m_buffer);
    }
    
    // Swap two objects
    stack_string(stack_string &&other) {
        std::swap(m_buffer, other.m_buffer);
    }
    
    // Destructor
    ~stack_string() {
        m_buffer[0] = '\0';
    }
    
    /*
         Operators
    */
    
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
        const size_t current_size_array = strlen(m_buffer);
        const size_t appended_size_array = strlen(cstring.m_buffer);
        
        if (appended_size_array + current_size_array + 1 > MAX_CHARACTER) {
            assert(false);
            return *this;
        }
        strcat(m_buffer, cstring.m_buffer);
        return *this;
    }
    
    /*
        Methods
     */
    
    size_t find_index(const stack_string &stackString) {
        if (strlen(stackString.m_buffer) == 0) {
            return -1;
        }
        
        const char* first_occurence = strstr(m_buffer, stackString.m_buffer);
        
        if (first_occurence != NULL) {
            return first_occurence-m_buffer;
        }
        
        return -1;
    }
    
    void append(const stack_string &stackString) {
        const size_t current_size_array = strlen(m_buffer);
        const size_t appended_size_array = strlen(stackString.m_buffer);
        
        if (appended_size_array + current_size_array + 1 > MAX_CHARACTER) {
            assert(false);
            return;
        }
        
        strcpy(m_buffer+current_size_array, stackString.m_buffer);
    }
    
    void insert(const size_t pos, const stack_string &stackString) {
        const size_t current_size_string = strlen(m_buffer);
        const size_t appended_size_array = strlen(stackString.m_buffer);
        
        char temp[MAX_CHARACTER];
        
        if (current_size_string + appended_size_array + 1 > MAX_CHARACTER) {
            assert(false);
            return;
        }

        strcpy(temp, m_buffer+pos);
        strcpy(m_buffer+pos, stackString.m_buffer);
        strcpy(m_buffer+pos+appended_size_array, temp);
    }
    
    void replace(const size_t from, const stack_string &stackString) {
        strcpy(m_buffer+from, stackString.m_buffer);
    }
    
    bool empty() {
        if (strlen(m_buffer) == 0) {
            return true;
        }
        return  false;
    }
    
    size_t lenght() {
        return strlen(m_buffer);
    }
    
private:
    char m_buffer[MAX_CHARACTER];
};
