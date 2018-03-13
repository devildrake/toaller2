#include <Player.h>
#include <queue>
#include <SFML\Network.hpp>
#include <thread>


void socketSelectorMethod(std::vector<PlayerServer*>*aPlayers, std::queue<sf::Packet>* aEventos) {
	sf::SocketSelector ss;
	sf::TcpSocket::Status status;
	for (int i = 0; i < aPlayers->size(); i++) {
		ss.add(*aPlayers->at(i)->socket);
	}
	while (ss.wait()) {
		for (int i = 0; i < aPlayers->size(); i++) {
			if (ss.isReady(*aPlayers->at(i)->socket)) {
				sf::Packet local;
				status = aPlayers->at(i)->socket->receive(local);
				if (status == sf::TcpSocket::Status::Done) {
					aEventos->push(local);
				}else if(status == sf::TcpSocket::Status::Disconnected){
					std::cout << "Receiving DISCONNECT\n";
					ss.remove(*aPlayers->at(i)->socket);
					aPlayers->at(i)->socket->disconnect();
					aPlayers->erase(aPlayers->begin() + i);
				}
				else if (status == sf::TcpSocket::Status::Error) {
					std::cout << "Receiving ERROR\n";
				}

			}
		}
	}
}

void main() {
	int numPlayers = 6;
	sf::TcpListener listener;
	std::vector<std::string> aMensajes;
	sf::Socket::Status status;
	std::vector<PlayerServer*>aPlayers;
	bool exit = false;
	status = listener.listen(50000);
	std::queue<sf::Packet> aEventos;
	
	if (status != sf::TcpListener::Done) {
		std::cout << "Error al vincularse al puerto " + listener.getLocalPort() << std::endl;
	}
	else {
		for (int i = 0; i < numPlayers; i++) {
			sf::TcpSocket* socket = new sf::TcpSocket();
			status = listener.accept(*socket);
			if (status != sf::Socket::Done) {
				std::cout << "Error al aceptar la conexion\n";
			}
			else {
				sf::Packet namePacket;
				std::cout << "Conexion aceptada\n";
				status = socket->receive(namePacket);

				if (status == sf::Socket::Done) {
					std::string receiveCode;
					namePacket >> receiveCode;

					if (receiveCode == "NAME_") {
						std::cout << "Receiving name\n";
						std::string aUserName;
						namePacket >> aUserName;
						PlayerServer* aPlayer = new PlayerServer(socket, aUserName);
						aPlayer->id = i;
						std::cout << "Player with id " << aPlayer->id << " and named " << aPlayer->GetUserName()<<"\n";

						aPlayers.push_back(aPlayer);
						namePacket.clear();
					}


				}
				else if(status==sf::Socket::Disconnected) {
					std::cout << "Desconexión inesperada antes de recibir nombre\n";
					socket->disconnect();
					delete socket;
				}
				else if (status == sf::Socket::Error) {
					std::cout << "Error inesperado antes de recibir nombre\n";
					//socket->disconnect();
					//delete socket;
				}
			}
		}

		listener.close();
		////////////////////////GAMESTART///////////////////////
		////////////////////////GAMESTART///////////////////////
		////////////////////////GAMESTART///////////////////////
		////////////////////////GAMESTART///////////////////////

		std::thread receptionThread(&socketSelectorMethod, &aPlayers, &aEventos);

		for (int i = 0; i < numPlayers; i++) {
			sf::Packet infoPacket;
			infoPacket << "PLAYERS_";
			infoPacket << i;

			for (int j = 0; j < numPlayers; j++) {
				infoPacket << aPlayers[j]->GetUserName();
				aPlayers[j]->id = j;
				infoPacket << aPlayers[j]->id;
			}
			aPlayers[i]->socket->send(infoPacket);

		}
		
		for (int i = 0; i < numPlayers; i++) {
			sf::Packet infoPacket;
			infoPacket << "START_";
			aPlayers[i]->socket->send(infoPacket);
		}

		while (!exit) {
			if (aEventos.size() > 0) {
				sf::Packet receivedPacket = aEventos.front();
				sf::Packet sendPacket;
				std::string code;

				receivedPacket >> code;

				if (code == "MSJ_") {
					std::string mensaje;
					receivedPacket >> mensaje;
					sendPacket << "MSJ_";

					sendPacket<<mensaje;

					sf::TcpSocket::Status status;

					for (int i = 0; i < aPlayers.size(); i++) {
						status = aPlayers[i]->socket->send(sendPacket);

						if (status != sf::TcpSocket::Done) {
							std::cout << "NotDone\n";
						}
						else {
							std::cout << "Sent\n";
						}

					}
				}

				aEventos.pop();

			}



		}

		
		receptionThread.join();
		////////////////////////GAMESTART///////////////////////
		////////////////////////GAMESTART///////////////////////
		////////////////////////GAMESTART///////////////////////
		////////////////////////GAMESTART///////////////////////





	}
}