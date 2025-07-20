#include "interface.h"
#include <iostream>
#include <unistd.h>

int main(int argc, char* argv[]) {
	auto interface = new Interface<int, int>("/my_mem", 10, 0);

	std::cout << interface->insert(10, 10) << "\n";

	sleep(1);

	std::optional<int> value;
	std::cout << interface->get(10, &value) << "\n";
	if (value)
		std::cout << value.value() << "\n";
}


