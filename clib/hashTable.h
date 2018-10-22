//
// Created by george on 20/7/2018.
//

#ifndef EFFICIENTOPERATORS_HASHTABLE_H
#define EFFICIENTOPERATORS_HASHTABLE_H

//
// Created by george on 14/10/18.
//

#ifndef C_SABER_HASHTABLE_H
#define C_SABER_HASHTABLE_H

#include <cstdlib>
#include <cstring>

#define KEY_SIZE 12
#define VALUE_SIZE 4
#define MAP_SIZE 1024


template <class Key, class Value>
struct ht_node {
    char status;
    long timestamp;
    Key key;
    Value value;
    int counter;
    //char padding[3];
};


template <class Key, class Value>
class hashtable {
private:
    int size;
    struct ht_node<Key, Value> *table;

private:

    inline int round (int value) {
        if (!(value && (!(value&(value-1))))) {
            value--;
            value |= value >> 1;
            value |= value >> 2;
            value |= value >> 4;
            value |= value >> 8;
            value |= value >> 16;
            value++;
        }
        return value;
    }

    inline int hashFunction (const int x) { return (x & (this->size - 1)); }
    inline int hash (const int *key) {
        return hashFunction(*key);
    }
    inline int hash (const long *key) {
        return hashFunction((int) *key);
    }
    inline int hash (const float *key) {
        return hashFunction((int) *key);
    }
    inline int hash (const void *key) {
        /*unsigned */int hashval = 0;
        int i = KEY_SIZE-1;

        unsigned char * diref = (unsigned char *) key;
        /* Convert key to an integer */
        while (i >= 0) { // little-endian
            hashval = hashval << 8;
            hashval += diref[i];
            i--;
        }
        //printf ("%d \n", hashval);
        return hashFunction(hashval);
    }

    inline bool isEqual (const int *v1, const int *v2) { return *v1 == *v2; }
    inline bool isEqual (const long *v1, const long *v2) { return *v1 == *v2; }
    inline bool isEqual (const float *v1, const float *v2) { return *v1 == *v2; }
    inline bool isEqual (const void *v1, const void *v2) { return memcmp(v1, v2, KEY_SIZE) == 0; }

public:

    hashtable () : hashtable(MAP_SIZE) {}
    hashtable (int size) {
        // rounding up to next power of 2
        size = round(size);
        this->size = size;
        this->table = (ht_node<Key, Value> *)  malloc( sizeof(ht_node<Key, Value>) * size );
        //printf("%d \n", sizeof(ht_node<Key, Value>));
        for (int i = 0; i < size; i++)
            memset(&this->table[i], 0, sizeof(ht_node<Key, Value>));
    }

    hashtable (ht_node<Key, Value> * table) : hashtable(table, MAP_SIZE) {}

    hashtable (ht_node<Key, Value> * table, int size) {
        this->table = table;
        this->size = size;
    }

    int getSize () { return this->size; }

    ht_node<Key, Value> * getTable () { return this->table; }

    void insert (const Key * key, const Value value, const long timestamp) {
        int hashIndex = hash(key);
        //find next free space -> use two for loops
        int numOfTrials = 0;
        while (this->table[hashIndex].status
               && isEqual(&this->table[hashIndex].key, key) && numOfTrials < this->size) {
            hashIndex++;
            hashIndex %= this->size;
            numOfTrials++;
        }

        if (numOfTrials >= this->size) {
            printf ("error: the hashtable is full \n");
            exit(1);
        }

        this->table[hashIndex].status = 1;
        this->table[hashIndex].timestamp = timestamp;
        memcpy(&this->table[hashIndex].key, key, KEY_SIZE); //strcpy(table[hashIndex].key, key);
        this->table[hashIndex].value = value;
        this->table[hashIndex].counter = 1;
    }

    Value insert_and_modify (const Key * key, const Value value, const long timestamp) {
        int ind = hash(key), i = ind;
        char tempStatus;
        for (; i < this->size; i++) {
            tempStatus = this->table[i].status;
            if (tempStatus && isEqual(&this->table[i].key, key)) { //update
                this->table[i].value += value;
                this->table[i].counter++;
                return this->table[i].value;
            }
            if (!tempStatus) { // first insert
                this->table[i].status = 1;
                this->table[i].timestamp = timestamp;
                memcpy(&this->table[i].key, key, KEY_SIZE); //strcpy(table[hashIndex].key, key);
                this->table[i].value = value;
                this->table[i].counter = 1;
                return value;
            }
        }
        for (i = 0; i < ind; i++) {
            tempStatus = this->table[i].status;
            if (tempStatus && isEqual(&this->table[i].key, key)) {
                this->table[i].value += value;
                this->table[i].counter++;
                return this->table[i].value;
            }
            if (!tempStatus) {
                this->table[i].status = 1;
                this->table[i].timestamp = timestamp;
                memcpy(&this->table[i].key, key, KEY_SIZE); //strcpy(table[hashIndex].key, key);
                this->table[i].value= value;
                this->table[i].counter++;
                return value;
            }
        }

        printf ("error: the hashtable is full \n");
        exit(1);
    }

    int insert_and_increment_counter (const Key * key, const long timestamp) {
        int ind = hash(key), i = ind;
        char tempStatus;
        for (; i < this->size; i++) {
            tempStatus = this->table[i].status;
            if (tempStatus && isEqual(&this->table[i].key, key)) { //update
                this->table[i].counter++;
                return this->table[i].counter;
            }
            if (!tempStatus) { // first insert
                this->table[i].status = 1;
                this->table[i].timestamp = timestamp;
                memcpy(&this->table[i].key, key, KEY_SIZE); //strcpy(table[hashIndex].key, key);

                this->table[i].counter = 1;
                return 1;
            }
        }
        for (i = 0; i < ind; i++) {
            tempStatus = this->table[i].status;
            if (tempStatus && isEqual(&this->table[i].key, key)) {
                this->table[i].counter++;
                return this->table[i].counter;
            }
            if (!tempStatus) {
                this->table[i].status = 1;
                this->table[i].timestamp = timestamp;
                memcpy(&this->table[i].key, key, KEY_SIZE); //strcpy(table[hashIndex].key, key);
                this->table[i].counter++;
                return 1;
            }
        }

        printf ("error: the hashtable is full \n");
        exit(1);
    }

    int evict_and_decrement_counter (const Key * key) {
        int ind = hash(key), i = ind;
        char tempStatus;
        for (; i < this->size; i++) {
            tempStatus = this->table[i].status;
            if (tempStatus && isEqual(&this->table[i].key, key)) { //update
                this->table[i].counter--;
                return this->table[i].counter;
            }
            if (!tempStatus) {
                return 0;
            }
        }
        for (i = 0; i < ind; i++) {
            tempStatus = this->table[i].status;
            if (tempStatus && isEqual(&this->table[i].key, key)) {
                this->table[i].counter++;
                return this->table[i].counter;
            }
            if (!tempStatus) {
                return 0;
            }
        }
        return 0;
    }

    bool get_value (const Key * key, Value &result) {
        int ind = hash(this->size, key), i = ind;
        for (; i < this->size; i++) {
            if ((this->table[i].status) && isEqual(&this->table[i].key, key)) {
                result = this->table[i].value;
                return true;
            }
        }
        for (i = 0; i < ind; i++) {
            if ((this->table[i].status) && isEqual(&this->table[i].key, key)) {
                result = this->table[i].value;
                return true;
            }
        }
        return false;
    }

    bool get_counter (const Key * key, int &result) {
        int ind = hash(this->size, key), i = ind;
        for (; i < this->size; i++) {
            if ((this->table[i].status) && isEqual(&this->table[i].key, key)) {
                result = this->table[i].counter;
                return true;
            }
        }
        for (i = 0; i < ind; i++) {
            if ((this->table[i].status) && isEqual(&this->table[i].key, key)) {
                result = this->table[i].counter;
                return true;
            }
        }
        return false;
    }

    bool get_index (const Key * key, int &index) {
        int ind = hash(this->size, key), i = ind;
        for (; i < this->size; i++) {
            if ((this->table[i].status) && isEqual(&this->table[i].key, key)) {
                index = i;
                return true;
            }
            if (!this->table[i].status) {
                index = i;
                return false;
            }
        }
        for (i = 0; i < ind; i++) {
            if ((this->table[i].status) && isEqual(&this->table[i].key, key)) {
                index = i;
                return true;
            }
            if (!this->table[i].status) {
                index = i;
                return false;
            }
        }
        index = -1;
        return false;
    }

    bool get_index (ht_node<Key,Value> * table, const Key * key, int &index) {
        int ind = hash(key), i = ind;
        for (; i < size; i++) {
            if ((table[i].status) && isEqual(&table[i].key, key)) {
                index = i;
                return true;
            }
            if (!table[i].status) {
                index = i;
                return false;
            }
        }
        for (i = 0; i < ind; i++) {
            if ((table[i].status) && isEqual(&table[i].key, key)) {
                index = i;
                return true;
            }
            if (!table[i].status) {
                index = i;
                return false;
            }
        }
        index = -1;
        return false;
    }

    void deleteHashtable () {
        free(this->table);
    }

    /*~hashtable () {
        free(this->table);
    }*/
};



#endif //C_SABER_HASHTABLE_H



#endif //EFFICIENTOPERATORS_HASHTABLE_H