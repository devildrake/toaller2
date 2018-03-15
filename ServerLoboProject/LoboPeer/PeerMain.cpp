#include <iostream>
#include <string>
#include <SFML\Network.hpp>
#include <Player.h>

int main() {
	Player::ROLE myRole;
	int myID;
	std::string username;
	std::cout << "Introduce el nombre de usuario" << std::endl;
	std::cin >> username;

	sf::TcpSocket* socket = new sf::TcpSocket();
	sf::IpAddress ip = sf::IpAddress::getLocalAddress();
	sf::Socket::Status status;

	status = socket->connect(ip, 50000);
	if (status != sf::Socket::Done) {
		std::cout << "Error al conectar con el servidor" << std::endl;
	}
	else {
		sf::Packet incomingInfo;
		status = socket->receive(incomingInfo);
		if(status == sf::Socket::Done){
			std::string command;
			incomingInfo >> command;
			if (command == "INFOS_") {
				incomingInfo >> myRole;
			}
		}
	}
}