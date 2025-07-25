#ifndef INTERFACE_H
#define INTERFACE_H

#include "shared_mem.h"

#include <cerrno>
#include <chrono>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <thread>
#include <unistd.h>
#include <mutex>
#include <optional>


template <typename K, typename T>
class HashtInterface {
	const char* mem_name;
	size_t port_count;
	SH_MEM<K, T> *sh_mem;


public:
	HashtInterface(const char* mem_name, size_t port_count) {
		this->mem_name = mem_name;
		this->port_count = port_count;

		int shmem = shm_open(mem_name, O_RDWR, 0666);
		if (!shmem) {
			std::cerr << "Opening shared memory failed: " << strerror(errno) << "\n";
			throw std::runtime_error("");
		}
		if (ftruncate(shmem, shared_mem_size<K, T>(port_count))) {
			std::cerr << "Ftruncate failed: " << strerror(errno) << "\n";
			if (shm_unlink(mem_name) == -1) {
				std::cerr << "Could not unlink shared memory correctly: " << strerror(errno) << "\n";
			};
			throw std::runtime_error("");
		};
		
		sh_mem = (SH_MEM<K, T> *) mmap(0, shared_mem_size<K, T>(port_count), PROT_READ | PROT_WRITE, MAP_SHARED, shmem, 0); 

		if (sh_mem == MAP_FAILED) {
			std::cerr << "Mmap failed: " << strerror(errno) << "\n";
			if (shm_unlink(mem_name) == -1) {
				std::cerr << "Could not unlink shared memory correctly: " << strerror(errno) << "\n";
			};
			throw std::runtime_error("");
		}
	}

	~HashtInterface() {
		if(shm_unlink(mem_name) == -1) {
			std::cerr << "Could not unlink shared memory correctly: " << strerror(errno) << "\n";
		}
		if (munmap(sh_mem, shared_mem_size<K, T>(port_count)) == -1) {
			std::cerr << "Could not unmap shared memory correctly: " << strerror(errno) << "\n";
		}

	}

	std::optional<size_t> queue_operation(Operation<K, T> op) {
		if ((sh_mem->op_head + 1) % port_count == sh_mem->ret_tail)
			return {};

		std::unique_lock<std::mutex> lock(sh_mem->head_lock);
		if ((sh_mem->op_head + 1) % port_count == sh_mem->ret_tail)
			return {};

		size_t port = sh_mem->op_head;
		sh_mem->op_ports[port] = op;
		sh_mem->op_head = (sh_mem->op_head + 1) % port_count;
		return port;
	}

	bool insert(K key, T value) {
		Operation<K, T> op = {
			.type = INSERT,
			.status	= INCOMING,
			.key = key,
			.value = value,
		};
		return queue_operation(op).has_value();
	}

	bool remove(K key) {
		Operation<K, T> op = {
			.type = REMOVE,
			.status	= INCOMING,
			.key = key
		};
		return queue_operation(op).has_value();
	}

	bool get(K key, std::optional<T> *value) {
		Operation<K, T> op = {
			.type = GET,
			.status	= INCOMING,
			.key = key
		};

		std::optional<size_t> port = queue_operation(op);
		if (!port)
	  		return false;

		while (sh_mem->op_ports[port.value()].status != FINISHED) {
			std::this_thread::sleep_for(std::chrono::milliseconds(50));
		}

		std::optional<T> val = sh_mem->op_ports[port.value()].value;
		sh_mem->op_ports[port.value()].status = EMPTY;
		*value = val;
		return true;
	}
};


#endif
