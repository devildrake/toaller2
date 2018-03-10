#include <Player.h>
#include <SFML\Network.hpp>


void main() {
	int numPlayers = 6;
	sf::TcpListener listener;
	std::vector<std::string> aMensajes;
	sf::SocketSelector ss;
	sf::Socket::Status status;
	std::vector<PlayerServer*>aPlayers;
	bool exit = false;
	status = listener.listen(50000);

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
		////////////////////////GAMESTART///////////////////////
		////////////////////////GAMESTART///////////////////////
		////////////////////////GAMESTART///////////////////////
		////////////////////////GAMESTART///////////////////////

		for (int i = 0; i < numPlayers; i++) {
			sf::Packet infoPacket;
			infoPacket << "PLAYERS_";
			infoPacket << i;

			for (int j = 0; j < numPlayers; j++) {
				infoPacket << aPlayers[i]->GetUserName();
				infoPacket << aPlayers[i]->id;
				aPlayers[i]->socket->send(infoPacket);
			}
		}
		
		for (int i = 0; i < numPlayers; i++) {
			sf::Packet infoPacket;
			infoPacket << "START_";
			aPlayers[i]->socket->send(infoPacket);
		}




		////////////////////////GAMESTART///////////////////////
		////////////////////////GAMESTART///////////////////////
		////////////////////////GAMESTART///////////////////////
		////////////////////////GAMESTART///////////////////////





	}
}