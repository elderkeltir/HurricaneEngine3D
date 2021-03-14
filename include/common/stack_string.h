//
//  stack_string.h
//
//  Created by Oleksandr Kysil on 28.02.2021.
//

#ifndef MAX_CHARACTER
#define MAX_CHARACTER 256
#endif

#ifndef stack_string_h
#define stack_string_h

class stack_string {
public:
    const char* c_str() {
        return m_buffer;
    }
    
    /*
        Constructors
     */
    
    stack_string() {}
    
    // Create
    stack_string(char* string) {
        strcpy(m_buffer, string);
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
    
    size_t find_index(const char* stackString) {
        // Searched string shouldn't be empty
        if (strlen(stackString) == 0) {
            return -1;
        }
        
        const size_t current_size_array = strlen(m_buffer);
        const size_t substring_size_array = strlen(stackString);
        
        for (size_t i=0; i<current_size_array; i++) {
            for (size_t j=0; j<substring_size_array; j++) {
                if (m_buffer[i] == stackString[j]) {
                    return i;
                }
            }
        }
        
        return -1;
    }
    
    void append(const char* cstring) {
        const size_t current_size_array = strlen(m_buffer);
        const size_t appended_size_array = strlen(cstring);
        
        if (appended_size_array + current_size_array + 1 > MAX_CHARACTER) {
            assert(false);
            return;
        }
        
        strcpy(m_buffer+current_size_array, cstring);
    }
    
    void insert(const size_t pos, const char * cstring) {
        const size_t current_size_string = strlen(m_buffer);
        const size_t appended_size_array = strlen(cstring);
        
        char temp[MAX_CHARACTER];
        
        if (current_size_string + appended_size_array + 1 > MAX_CHARACTER) {
            assert(false);
            return;
        }

        strcpy(temp, m_buffer+pos);
        strcpy(m_buffer+pos, cstring);
        strcpy(m_buffer+pos+appended_size_array, temp);
    }
    
    void replace(const size_t from, const size_t length, const char* cstring) {
        strcpy(m_buffer+from, cstring);
    }
    
    void erase(const size_t pos = 0, const size_t length = MAX_CHARACTER) {
        if (pos == 0 && length == MAX_CHARACTER) {
            // Delete whole buffer
            m_buffer[pos] = '\0';
        } else {
            char pref[pos];
            char suff[MAX_CHARACTER];
            
            // Clean the middle of buffer
            for (size_t i = pos; i<=length; i++) {
                m_buffer[i] = '\0';
            }
            
            strcpy(pref, m_buffer);
            strcpy(suff, m_buffer+pos+length);
            
            // Cleanup buffer
            m_buffer[0] = '\0';
            
            strcpy(m_buffer, pref);
            strcpy(m_buffer+pos,suff);
        }
    }
    
    bool empty() {
        if (sizeof(m_buffer) == 0) {
            return true;
        }
        return  false;
    }
    
    size_t lenght() {
        return sizeof(m_buffer);
    }
    
private:
    char m_buffer[MAX_CHARACTER];
};

#endif /* stack_string_h */
