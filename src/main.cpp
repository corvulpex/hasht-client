#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <unistd.h>
#include "input.h"
#include "interface.h"


std::optional<int> my_string_to_int(std::string s) {
	int i;
	try {
		int i = std::stoi(s);
	}
	catch (std::invalid_argument const &ex) {
		return {};
	}
	return i;
}


int main(int argc, char* argv[]) {
	auto interface = std::make_unique<HashtInterface<int, int>>("/my_mem", 30);

	auto input = new InputHandler<int, int>(std::move(interface), my_string_to_int, my_string_to_int, std::to_string);

	input->run();
}

