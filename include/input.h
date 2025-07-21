#ifndef INPUT_H
#define INPUT_H

#include "interface.h"

#include <iostream>
#include <memory>
#include <optional>
#include <sstream>
#include <string>

template <typename K, typename T>
class InputHandler {
	std::unique_ptr<HashtInterface<K, T>> interface;
	std::optional<K> (*string_to_key)(std::string);
	std::optional<T> (*string_to_value)(std::string);
	std::string (*value_to_string)(T);

public:
	
	InputHandler(std::unique_ptr<HashtInterface<K, T>> interface, std::optional<K> (*string_to_key_func)(std::string), std::optional<T> (*string_to_value_func)(std::string), std::string (*value_to_string_func)(T)) {
		this->interface = std::move(interface);
		string_to_key = string_to_key_func;
		string_to_value = string_to_value_func;
		value_to_string = value_to_string_func;
	};

	bool parse_input(char *command, K *key, T *value) {
		std::string input_line;
		std::getline(std::cin, input_line);
		
		std::istringstream iss(input_line);
		std::string cmd, param1, param2;

		iss >> cmd >> param1 >> param2;

		if (cmd.length() > 1) {
			std::cout << "Command can only be one letter\n";
			return false;
		}

		std::optional<K> kp = string_to_key(param1);
		if (cmd[0] != 'q' && !kp) {
			std::cout << "Could not parse key correctly\n";
			return false;
		}
		std::optional<T> vp = string_to_value(param2);
		if(cmd[0] == 'i' && !vp) {
			std::cout << "Could not parse value correctly\n";
			return false;
		}

		*command = cmd[0];
		if (kp)
			*key = kp.value();
		if(vp)
			*value = vp.value();
		return true;
	}

	void run() {
		while (true) {
			char c;
			K key;
			T value;

			if (!parse_input(&c, &key, &value))
				continue;
			
			switch (c) {
				case 'q':
					return;

				case 'i':
					if (! interface->insert(key, value))
						std::cout << "Action failed\n";
					break;

				case 'r':
					if (! interface->remove(key))
						std::cout << "Action failed\n";
					break;

				case 'g':
					std::optional<T> ret_val;
					if (!interface->get(key, &ret_val)) {
						std::cout << "Action failed\n";
						break;
					};
					if (ret_val) {
						std::cout << "Value: " << value_to_string(ret_val.value()) << "\n";
					} 
					else {
						std::cout << "No value found\n";
					}
					break;
			}

		}
	}

};

#endif
