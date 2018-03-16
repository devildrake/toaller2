#include <SFML\Network.hpp>
#include <iostream>
#include <Player.h>
#define NUMPLAYERS 6

class Direction {
	std::string ip, name;
	int port, id;
	Player::ROLE role;// = Player::ROLE::_VILLAGER;

public:
	Direction(std::string ip, int port, std::string username, int id) {
		this->ip = ip;
		this->port = port;
		name = username;
		this->id = id;
		role = Player::ROLE::_VILLAGER;
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
	Player::ROLE GetRole() {
		return role;
	}

	void SetRole(Player::ROLE aRole) {
		role = aRole;
	}

};

int main() {
	sf::TcpListener listener;
	sf::Socket::Status status;
	std::vector<Direction> aDirections;
	int wolves = 0;
	status = listener.listen(50000);
	if (status == sf::TcpListener::Done) {//SE HA PODIDO VINCULAR BIEN AL PUERTO
		Print("Esperando conexiones...");
		for (int i = 0; i < NUMPLAYERS; i++) {
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
						std::cout << "Sending info to peer (" << size << " peers)" << std::endl;
						
						infoPeers << "INFOS_";

						Direction direction(socket->getRemoteAddress().toString(), socket->getRemotePort(), username, playerID);


						if (wolves < 3) {
							infoPeers << (int)Player::ROLE::_WOLF;
							direction.SetRole(Player::ROLE::_WOLF);

							wolves++;

						}
						else {
							infoPeers << (int)Player::ROLE::_VILLAGER;
							direction.SetRole(Player::ROLE::_VILLAGER);

						}

						infoPeers << playerID << size;

						for (int j = 0; j< size; j++) {
							infoPeers << aDirections[j].GetIP();
							infoPeers << aDirections[j].GetPort();
							infoPeers << aDirections[j].GetID();
							infoPeers << aDirections[j].GetName();
							infoPeers << (int)aDirections[j].GetRole();
							std::cout << "Name going inside packet--->" << aDirections[j].GetName() << std::endl;

						}
						status = socket->send(infoPeers); //se envia la informacion de todos los peers que se habian conectado antes
						if (status != sf::Socket::Done) {
							std::cout << "Error al enviar el mensaje" << std::endl;
						}
						else {
							//se añade la informacion de este peer
							aDirections.push_back(direction);

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
		Print("Cerrando listener");
		listener.close();
	}


	else {
		std::cout << "No se ha podido vincular al puerto" << std::endl;
		system("pause");
		listener.close();
	}
}

