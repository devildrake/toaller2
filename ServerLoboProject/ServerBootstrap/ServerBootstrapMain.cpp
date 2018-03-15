#include <SFML\Network.hpp>
#include <iostream>
#include <Player.h>

class Direction {
	std::string ip, name;
	int port, id;
public:
	Direction(std::string ip, int port, std::string username, int id) {
		this->ip = ip;
		this->port = port;
		name = username;
		this->id = id;
	}

	std::string GetIP() {
		return ip;
	}
	std::string GetName() {
		return name;
	}
	int GetPort() {
		return port;
	}
	int GetID() {
		return id;
	}
};

int main() {
	sf::TcpListener listener;
	sf::Socket::Status status;
	int maxPlayers = 12;
	std::vector<Direction> aDirections;

	status = listener.listen(50000);
	if (status = sf::TcpListener::Done) {//SE HA PODIDO VINCULAR BIEN AL PUERTO
		for (int i = 0; i < maxPlayers; i++) {
			sf::TcpSocket* socket = new sf::TcpSocket();
			status = listener.accept(*socket);
			
			if (status == sf::Socket::Done){//SE HA CONECTADO A UN PEER
				sf::Packet incomingInfo;
				status = socket->receive(incomingInfo);
				if (status == sf::Socket::Done) { //RECEPCION DEL MENSAJE DE PRESENTACION DEL PEER
					std::string comando;
					incomingInfo >> comando;
					if (comando == "NAME_") { //COMANDO DE COMUNICACION "NAME_"
						//el peer manda su nombre de usuario y lo guardamos en la informacion de este peer
						std::string username;
						incomingInfo >> username;
						sf::Packet infoPeers;

						int size = aDirections.size();
						int playerID = i;

						infoPeers << "INFOS_" << Player::ROLE::_VILLAGER << playerID << size;
						for (int i = 0; i < size; i++) {
							infoPeers << aDirections[i].GetIP() << aDirections[i].GetPort() << playerID << username;
						}
						status = socket->send(infoPeers); //se envia la informacion de todos los peers que se habian conectado antes
						if (status != sf::Socket::Done) {
							std::cout << "Error al enviar el mensaje" << std::endl;
						}
						else {
							//se añade la informacion de este peer
							aDirections.push_back(Direction(socket->getRemoteAddress().toString(), socket->getRemotePort(),username,playerID));
						}
						socket->disconnect();
						delete socket;
					}
				}
			} 
			else {
				std::cout << "Error al conectar con el peer" << std::endl;
				delete socket;
			}
		}
	}
	else {
		std::cout << "No se ha podido vincular al puerto" << std::endl;
	}
}

