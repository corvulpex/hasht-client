#ifndef INTERFACE_H
#define INTERFACE_H

#include "shared_mem.h"

#include <chrono>
#include <iostream>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <thread>
#include <unistd.h>
#include <mutex>
#include <optional>


template <typename K, typename T>
class HashtInterface {
	size_t port_count;
	SH_MEM<K, T> *sh_mem;


public:
	HashtInterface(const char* mem_name, size_t port_count) {
		this->port_count = port_count;

		int shmem = shm_open(mem_name, O_RDWR, 0666);
		ftruncate(shmem, sizeof(Operation<K, T>) * port_count + sizeof(size_t) * 3 + sizeof(std::mutex) * 2);
		sh_mem = (SH_MEM<K, T> *) mmap(0, sizeof(Operation<K, T>) * port_count + sizeof(size_t) * 3 + sizeof(std::mutex) * 2, PROT_READ | PROT_WRITE, MAP_SHARED, shmem, 0); 
	}

	std::optional<size_t> queue_operation(Operation<K, T> op) {
		if ((sh_mem->op_head + 1) % port_count == sh_mem->ret_tail)
			return {};

		std::unique_lock<std::mutex> lock(sh_mem->head_lock);
		if ((sh_mem->op_head + 1) % port_count == sh_mem->ret_tail)
			return {};

		size_t port = sh_mem->op_head;
		sh_mem->op_ports[port] = op;
		sh_mem->op_head++;
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
		std::optional<size_t> port;
		port = queue_operation(op);
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
