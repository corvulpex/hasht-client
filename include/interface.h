#include <chrono>
#include <iostream>
#include <numeric>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <thread>
#include <unistd.h>
#include <mutex>
#include <optional>

typedef enum {
    INSERT,
    REMOVE,
    GET
} OP_Type;

typedef enum {
    EMPTY,
    INCOMING,
    FINISHED,
    ERROR
} OP_STATUS;

template <typename K, typename T>
struct Operation {
    std::mutex lock;
    OP_Type type;
    OP_STATUS status;
    K key;
    std::optional<T> value;
};

template <typename K, typename T>
struct shmem {
    Operation<K, T> op_ports[];
};


template <typename K, typename T>
class Interface {
	size_t port_count;
	Operation<K, T> *ports;


public:
	Interface(const char* mem_name, size_t port_count, size_t port_offset=0) {
		this->port_count = port_count;

		int shmem = shm_open(mem_name, O_RDWR, 0666);
		ftruncate(shmem, sizeof(Operation<K, T>) * (port_count + port_offset));
		ports = (Operation<K, T> *) mmap(0, sizeof(Operation<K, T>) * port_count, PROT_READ | PROT_WRITE, MAP_SHARED, shmem, sizeof(Operation<K, T>) * port_offset);
	}


	bool insert(K key, T value, size_t port = 0) {
		if (ports[port].status != EMPTY)
			return false;
	
		std::unique_lock<std::mutex> lock(ports[port].lock);
		if(ports[port].status != EMPTY)
			return false;
	
		ports[port].type = INSERT;
		ports[port].key = key;
		ports[port].value = value;
		ports[port].status = INCOMING;
		return true;
	}

	bool remove(K key, size_t port = 0) {
		if (ports[port].status != EMPTY)
			return false;

		std::unique_lock<std::mutex> lock(ports[port].lock);
		if(ports[port].status != EMPTY)
			return false;
	
		ports[port].type = REMOVE;
		ports[port].key = key;
		ports[port].status = INCOMING;
		return true;
	}

	bool get(K key, std::optional<T> *value, size_t port = 0) {
		if (ports[port].status != EMPTY)
			return false;

		std::unique_lock<std::mutex> lock(ports[port].lock);
		if(ports[port].status != EMPTY)
			return false;
	
		ports[port].type = GET;
		ports[port].key = key;
		ports[port].status = INCOMING;
	
		lock.unlock();

		while (ports[port].status != FINISHED)
			std::this_thread::sleep_for(std::chrono::milliseconds(50));

		lock.lock();
		if (ports[port].status != FINISHED)
			return false;

		*value = ports[port].value;
		ports[port].status = EMPTY;
		return true;
	}
};

