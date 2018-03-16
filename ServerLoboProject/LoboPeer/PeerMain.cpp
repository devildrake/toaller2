#include <iostream>
#include <string>
#include <SFML\Network.hpp>
#include <SFML\Graphics.hpp>
#include <Player.h>
#include <queue>
#include <thread>
#define NUMPLAYERS 6
#define LADO_CASILLA 25


int ComprovarBotonPulsado(int _x, int _y, std::vector<sf::Vector2f>botones) {
	int x = _x;
	int y = _y;

	int idPulsado = -1;

	for (int i = 0; i < botones.size(); i++) {
		if (x > botones[i].x&&x<botones[i].x + LADO_CASILLA&&y>botones[i].y&&y < botones[i].y + LADO_CASILLA) {
			idPulsado = i;
			break;
		}
	}
	/*
	float xCasilla = _x / LADO_CASILLA;
	float yCasilla = _y / LADO_CASILLA;
	sf::Vector2f casilla(xCasilla, yCasilla);
	*/
	std::cout << "Pulsado el boton" << idPulsado << "\n";

	return idPulsado;
}

void socketSelectorMethod(std::vector<PlayerServer>*aPlayers, std::queue<sf::Packet>* aEventos) {
	sf::SocketSelector ss;
	sf::TcpSocket::Status status;
	for (int i = 0; i < aPlayers->size(); i++) {
		ss.add(*aPlayers->at(i).socket);
	}
	while (ss.wait()) {
		for (int i = 0; i < aPlayers->size(); i++) {
			if (ss.isReady(*aPlayers->at(i).socket)) {
				sf::Packet local;
				status = aPlayers->at(i).socket->receive(local);
				if (status == sf::TcpSocket::Status::Done) {
					aEventos->push(local);
				}
				else if (status == sf::TcpSocket::Status::Disconnected) {
					std::cout << "Receiving DISCONNECT\n";
					ss.remove(*aPlayers->at(i).socket);
					aPlayers->at(i).socket->disconnect();
					aPlayers->erase(aPlayers->begin() + i);
				}
				else if (status == sf::TcpSocket::Status::Error) {
					std::cout << "Receiving ERROR\n";
				}

			}
		}
	}
}

int main() {
	Player::Turn currentTurn = Player::Turn::_DAY;
	Player::ROLE myRole;
	int myPort;
	int myID;
	std::string username;
	std::cout << "Introduce el nombre de usuario" << std::endl;
	std::cin >> username;

	std::vector<PlayerServer>aPlayers;

	PlayerServer* mySelf = new PlayerServer();

	mySelf->alive = true;
	mySelf->socket = nullptr;
	mySelf->userName = username;

	sf::TcpSocket* socket = new sf::TcpSocket();
	sf::IpAddress ip = sf::IpAddress::getLocalAddress();
	sf::Socket::Status status;

	status = socket->connect(ip, 50000);
	if (status != sf::Socket::Done) {
		std::cout << "Error al conectar con el servidor" << std::endl;
		delete socket;
	}
	else {
		sf::Packet namePacket;
		namePacket << "NAME_";
		namePacket << username;

		status = socket->send(namePacket);

		if (!status == sf::Socket::Done) {
			std::cout << "Error de envio de nombre\n";
		}


		myPort = socket->getLocalPort();
		sf::Packet incomingInfo;
		status = socket->receive(incomingInfo);
		if(status == sf::Socket::Done){
			socket->disconnect();
			delete socket;
			std::string command;
			incomingInfo >> command;
			if (command == "INFOS_") {
				int roleId;
				int previousPeers;
				incomingInfo >> roleId;
				myRole = (Player::ROLE)roleId;

				incomingInfo >> myID;
				mySelf->id = myID;
				aPlayers.push_back(*mySelf);
				incomingInfo >> previousPeers;

				for (int i = 0; i < previousPeers; i++) {
					PlayerServer player;
					std::string ip;
					int port;


					sf::TcpSocket* socket = new sf::TcpSocket();
					incomingInfo >> ip;
					incomingInfo >> port;
					incomingInfo >> player.id;
					incomingInfo >> player.userName;

					status = socket->connect(ip, port);

					if (status == sf::TcpSocket::Socket::Status::Done) {
						player.socket = socket;
						aPlayers.push_back(player);
						sf::Packet infoPSend;
						Print("Conexion establecida con otro peer");

						infoPSend << "INFOP_" << myID << username << myRole;
						status = socket->send(infoPSend);
						if (status != sf::Socket::Done) {
							std::cout << "No se ha podido mandar la informacion de conexion a otro peer" << std::endl;
						}
						else {
							std::cout << "Informacion al nuevo peer enviada correctamente" << std::endl;
						}
					}
					else if (status == sf::TcpSocket::Socket::Status::Disconnected)  {
						std::cout << "Intento de conexion fallido (PEER DESCONECTADO)\n";
						delete socket;
					}
					else if (status == sf::TcpSocket::Socket::Status::Error) {
						std::cout << "Intento de conexion a otro peer fallido (ERROR RANDOM)\n";
						delete socket;
					}
				}
				sf::TcpListener listener;
				status = listener.listen(myPort);
				if(status == sf::Socket::Done){
					for (int i = aPlayers.size(); i < NUMPLAYERS; i++) {
						sf::TcpSocket* newSocket = new sf::TcpSocket();
						status = listener.accept(*newSocket);
						if (status == sf::TcpSocket::Status::Done) {
							std::cout << "NUEVA CONEXION ENTRANTE" << std::endl;
							sf::Packet infoPacket;
							status = newSocket->receive(infoPacket);

							if (status == sf::Socket::Done) {
								std::cout << "Informacion del nuevo peer recibida correctamente" << std::endl;
								infoPacket >> command;
								if (command == "INFOP_") {
									std::cout << "Receiving new peer info\n";
									PlayerServer newPlayer;
									newPlayer.socket = newSocket;
									infoPacket >> newPlayer.userName;
									infoPacket >> newPlayer.id;
									infoPacket >> roleId;
									newPlayer.role = (Player::ROLE)roleId;
									aPlayers.push_back(newPlayer);
								}
							}
							else if (status == sf::Socket::Disconnected) {
								std::cout << "Error de recepcion de peer nuevo (DESCONEXION)\n";
								newSocket->disconnect();
								delete newSocket;
							}
							else if (status == sf::Socket::Error) {
								std::cout << "Error de recepcion de peer nuevo\n";
								newSocket->disconnect();
								delete newSocket;
							}

						}
						else {
							std::cout << "No se ha podido aceptar la nueva conexion" << std::endl;
							delete newSocket;
						}
					}
				}
				else { //el peer no se ha podido vincular al puerto para aceptar nuevas conexiones
					std::cout << "El peer no se ha podido vincular al puerto" << std::endl;
				}
				listener.close();
				////////////////////CONEXION ESTABLECIDA///////////////////////////////
				////////////////////CONEXION ESTABLECIDA///////////////////////////////
				////////////////////CONEXION ESTABLECIDA///////////////////////////////
				////////////////////CONEXION ESTABLECIDA///////////////////////////////
				////////////////////CONEXION ESTABLECIDA///////////////////////////////
				////////////////////CONEXION ESTABLECIDA///////////////////////////////
				int whoToVote;

				std::string mensaje;
				std::vector<std::string> aMensajes;
				std::vector<sf::Vector2f> botones;
				sf::Vector2i screenDimensions(800, 600);

				sf::RenderWindow window;
				window.create(sf::VideoMode(screenDimensions.x, screenDimensions.y), "Chat");

				sf::Font font;
				if (!font.loadFromFile("Barber_Street.ttf"))
				{
					std::cout << "Can't load the font file" << std::endl;
				}

				mensaje = " >";

				sf::Text chattingText(mensaje, font, 20);
				chattingText.setFillColor(sf::Color(0, 160, 0));
				chattingText.setStyle(sf::Text::Bold);


				sf::Text text(mensaje, font, 14);
				text.setFillColor(sf::Color(0, 160, 0));
				text.setStyle(sf::Text::Bold);
				text.setPosition(0, 560);

				sf::RectangleShape separator(sf::Vector2f(800, 5));
				separator.setFillColor(sf::Color(200, 200, 200, 255));
				separator.setPosition(0, 550);

				sf::RectangleShape separator2(sf::Vector2f(5, 600));
				separator2.setFillColor(sf::Color(200, 200, 200, 255));
				separator2.setPosition(550, 0);


				std::queue<sf::Packet>aEvents;

				std::thread threadRecepcion(&socketSelectorMethod, &aPlayers, &aEvents);
				bool end;
				
				//LOOP DEL CHAT
				while (window.isOpen()) {
					if (aEvents.size() > 0) {
						sf::Packet packet;
						packet = aEvents.front();
						packet >> command;

						if (command == "MSG_") {
							std::string mensaje;



						}
						else if (command == "VOTE_") {

						}
						else if (command == "REVEAL_") {

						}
						else if (command == "REVIVE") {

						}




						aEvents.pop();
					}

					sf::Event evento;
					//Gestión de eventos
					while (window.pollEvent(evento)) {
						switch (evento.type) {
						case sf::Event::Closed:
							window.close();
							break;
						case sf::Event::MouseButtonPressed:
							if (evento.mouseButton.button == sf::Mouse::Left&&mySelf->alive) {
								int x = evento.mouseButton.x;
								int y = evento.mouseButton.y;

								whoToVote = ComprovarBotonPulsado(x, y, botones);

								if (whoToVote >= 0) {

									sf::Packet votePacket;

									switch (currentTurn) {
									case Player::Turn::_DAY:
										//if (mySelf.alive) {
										votePacket << "VOTE_";
										votePacket << whoToVote;
										votePacket << mySelf->id;
										status = socket->send(votePacket);

										if (status != sf::TcpSocket::Status::Done) {
											std::cout << "Error de emision\n";
										}
										//}
										break;
									case Player::Turn::_WOLVES:
										if (mySelf->role == Player::ROLE::_WOLF) {

											std::cout << "As a wolf, I summon my right to vote\n";

											//if (mySelf.alive) {
											votePacket << "VOTE_";
											votePacket << whoToVote;
											votePacket << mySelf->id;
											status = socket->send(votePacket);


											if (status != sf::TcpSocket::Status::Done) {
												std::cout << "Error de emision\n";
											}

											std::cout << "MY ID is " << mySelf->id << " and I am voting for the player with ID " << whoToVote << "\n";
											//}
										}
										break;
									case Player::Turn::_WITCHTURN:
										if (mySelf->role == Player::ROLE::_WITCH) {
											//if (mySelf.alive) {
											votePacket << "VOTE_";
											votePacket << mySelf->id;
											votePacket << whoToVote;
											status = socket->send(votePacket);


											if (status != sf::TcpSocket::Status::Done) {
												std::cout << "Error de emision\n";
											}

											//}
										}
										break;
									}
								}
							}
							break;
						case sf::Event::KeyPressed:
							if (evento.key.code == sf::Keyboard::Escape) {
								//Close(aSock, finish);
								socket->disconnect();
								window.close();
							}
							else if (evento.key.code == sf::Keyboard::Return) {
								std::string mensajeDefinitivo = username;
								mensajeDefinitivo += "-" + mensaje;
								sf::Packet packet;

								packet << "MSJ_";
								packet << mySelf->id;
								packet << mensajeDefinitivo;
								if (mensaje == " >/Disconnect") {
									//Close(aSock, finish);
									socket->disconnect();
									window.close();
								}
								//Envio del mensaje

								std::cout << "Trying to send " << mensajeDefinitivo << std::endl;

								status = socket->send(packet);
								if (status != sf::Socket::Done) {
									std::cout << "No se pudo mandar el mensaje\n";
								}
								else {
									std::cout << "Sending " << mensajeDefinitivo << std::endl;
								}

								//pushToAMsj(&aMensajes, mensajeDefinitivo);
								mensaje = " >";
							}
							break;
						case sf::Event::TextEntered:
							if (evento.text.unicode >= 32 && evento.text.unicode <= 126) {
								mensaje += (char)evento.text.unicode;
							}
							else if (evento.text.unicode == 8 && mensaje.length() > 0) {
								mensaje.erase(mensaje.length() - 1, mensaje.length());
							}
							break;
						}
					}

					//std::cout << mySelf.role << "\n
					window.draw(separator);
					window.draw(separator2);





				}

				threadRecepcion.join();


			}

		}
	}
}