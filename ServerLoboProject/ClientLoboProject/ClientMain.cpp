#include <Player.h>
#include <SFML\Network.hpp>
#include <SFML\Graphics.hpp>
#include <thread>
#include <chrono>
#include <queue>
#define LADO_CASILLA 25
#define NUM_PLAYERS 6

int ComprovarBotonPulsado(int _x, int _y,std::vector<sf::Vector2f>botones){
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
	std::cout << "Pulsado el boton" << idPulsado<<"\n";

	return idPulsado;
}

void eventMethod(sf::TcpSocket* maSocket,std::queue<sf::Packet>* aEventos) {
	Print("Lanzado thread de recepcion");	
	bool disconnected=false;
	while (!disconnected) {
		sf::TcpSocket::Status status;
		sf::Packet local;
		Print("Preparado para recibir");
		status = maSocket->receive(local);
		if (status == sf::TcpSocket::Status::Done) {
			aEventos->push(local);
			Print("Recibiendo un evento");
		}
		else if (status == sf::TcpSocket::Status::Disconnected) {
			std::cout << "Receiving DISCONNECT\n";
			disconnected = true;
			maSocket->disconnect();
		}
		else if (status == sf::TcpSocket::Status::Error) {
			std::cout << "Receiving ERROR\n";
		}
	}
}

void main() {
	std::queue<sf::Packet>aEvents;
	srand(time(NULL));
	Player::Turn currentTurn;
	currentTurn = Player::Turn::_DAY;
	std::string myUserName = "";
	int myId;
	Player::ROLE myRole;

	std::cout << "Enter your username: \n";
	//std::cin >> myUserName;
	myUserName = ((char)((rand() % 57) + 65));
	bool end = false;
	int whoToVote;

	std::string mensaje;
	std::vector<std::string> aMensajes;
	std::vector<sf::Vector2f> botones;
	sf::TcpSocket* socket = new sf::TcpSocket();
	sf::Socket::Status status;
	std::vector<Player>aPlayers;

	for (size_t i = 0; i < NUM_PLAYERS; i++) {
		sf::Vector2f boton;
		boton.x = 555;
		if (i > 0) {
			boton.y = LADO_CASILLA * i + 5 * i;
		}
		else {
			boton.y = LADO_CASILLA * i;
		}
		botones.push_back(boton);
		//botones[i].setPosition(sf::Vector2f(555, 20 * i));
		//chattingText.setString(chatting);
		//window.draw(chattingText);
	}

	status = socket->connect(sf::IpAddress::getLocalAddress(), 50000);

	if (status == sf::Socket::Done) {
		sf::Packet namePacket;
		namePacket << myUserName;
		status = socket->send(namePacket);


		std::cout << "Conexion establecida\n";

		sf::Vector2i screenDimensions(800, 600);

		sf::RenderWindow window;
		window.create(sf::VideoMode(screenDimensions.x, screenDimensions.y), "Chat");

		sf::Font font;
		if (!font.loadFromFile("Barber_Street.ttf"))
		{
			std::cout << "Can't load the font file" << std::endl;
		}


		//void eventMethod(sf::TcpSocket* maSocket, std::queue<sf::Packet>* aEventos) {

		std::thread receptionThread(&eventMethod, socket, &aEvents);

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




		//LOOP DEL CHAT
		while (window.isOpen()) {
		
			if (aEvents.size() > 0) {
				sf::Packet packet = aEvents.front();
				std::string code;
				packet >> code;

				if (code == "PLAYERS_") {
					aMensajes.clear();
					packet >> myId;
					int playerSize;
					int roleId;
					packet >> playerSize;

					packet >> roleId;
					myRole = (Player::ROLE)roleId;

					for (int j = 0; j < playerSize; j++) {

						Player player;
						int roleId;

						packet >> player.userName;
						packet >> player.id;
						packet >> roleId;

						if (myRole == Player::ROLE::_WOLF&&(Player::ROLE)roleId==Player::ROLE::_WOLF) {
							player.role = (Player::ROLE)roleId;
						}
						//aMensajes.push_back("SERVIDOR: Entra " + player.userName);
						aPlayers.push_back(player);
					}
					aMensajes.push_back("Es de dia en Castronegro");
					aPlayers[myId].role = myRole;
				}
				else if (code == "MSJ_") {
					std::string mensaje;
					packet >> mensaje;
					aMensajes.push_back(mensaje);
				}
				else if (code == "DEAD_") {
					for (int i = 0; i < aPlayers.size(); i++) {
						packet >> aPlayers[i].alive;

						if (aPlayers[i].alive == false) {
							int roleId;
							packet >> roleId;
							aPlayers[i].role = (Player::ROLE)roleId;
						}

					}
				}
				else if (code == "ENDTURN_") {
					aMensajes.clear();
					int turnId;
					packet >> turnId;
					currentTurn = (Player::Turn)turnId;
					std::string unMensaje;
					packet >> unMensaje;
					aMensajes.push_back(unMensaje);
				}
				else if (code == "GAMEOVER_") {
					packet >> mensaje;
					aMensajes.clear();
					aMensajes.push_back(mensaje);
					end = true;
				}
				else if (code == "DISCONNECT_") {
					aMensajes.clear();
					int discId;
					int roleId;
					packet >> discId;
					packet >> roleId;


					aMensajes.push_back(aPlayers[discId].GetUserName() + " se ha desconectado");
					aPlayers[discId].role = (Player::ROLE)roleId;
					aPlayers[discId].alive = false;
					aMensajes.push_back("los votos se reinician");

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
					if (evento.mouseButton.button == sf::Mouse::Left) {
						int x = evento.mouseButton.x;
						int y = evento.mouseButton.y;

						whoToVote = ComprovarBotonPulsado(x, y, botones);
						if (whoToVote != -1) {
							sf::Packet votePacket;
							votePacket << "VOTE_";
							votePacket << myId;
							votePacket << whoToVote;

							status = socket->send(votePacket);

							if (status == sf::Socket::Done) {
								std::cout << "Voto enviado\n";
							}
							else if (status == sf::Socket::Disconnected) {
								aMensajes.push_back("Desconexion del servidor\n");
							}
							else if (status == sf::Socket::Error) {
								aMensajes.push_back("Error enviando el voto\n");
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
						std::string mensajeDefinitivo = aPlayers[myId].userName;
							mensajeDefinitivo += "-" + mensaje;
							sf::Packet packet;

							packet << "MSJ_";
							//packet << mySelf->id;
							packet << mensajeDefinitivo;
							packet << myId;

							status == socket->send(packet);

							if (status == sf::Socket::Disconnected) {
								std::cout << "Server desconectado\n";
								end = true;
								socket->disconnect();
								delete socket;

							}
							else if (status == sf::Socket::Error) {
								std::cout << "Error de envío de mensaje\n";
							}
						
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

			window.draw(separator);
			window.draw(separator2);
			for (size_t i = 0; i < aMensajes.size(); i++) {
				std::string chatting = aMensajes[i];
				chattingText.setPosition(sf::Vector2f(0, 20 * i));
				chattingText.setString(chatting);
				window.draw(chattingText);
			}

			for (size_t i = 0; i < aPlayers.size(); i++) {
				//			std::string chatting = i + "-> " + aUserNames[i];
				std::string role;

				if (!aPlayers[i].alive) {
					role = aPlayers.at(i).GetRoleAsString();
				}
				else if (aPlayers[i].id != myId) {
					role = "?";
				}
				else {
					role = aPlayers.at(i).GetRoleAsString();
				}

				if (myRole == Player::ROLE::_WOLF) {
					if (aPlayers[i].role == Player::ROLE::_WOLF) {
						role = "W";
					}
				}




				std::string chatting = std::to_string(aPlayers[i].id) + "-> " + aPlayers[i].userName + " (" + role + ")";
				chattingText.setPosition(sf::Vector2f(590, 20 * i + 10 * i));
				chattingText.setString(chatting);
				window.draw(chattingText);
			}

			for (int j = 0; j<aPlayers.size(); j++) {
				sf::RectangleShape rectBlanco(sf::Vector2f(LADO_CASILLA, LADO_CASILLA));


				if (aPlayers[j].id == myId) {
					if (!aPlayers[j].alive) {
						rectBlanco.setFillColor(sf::Color::Magenta);
					}
					else {
						rectBlanco.setFillColor(sf::Color::Blue);
					}
				}
				else {
					if (!aPlayers[j].alive)
						rectBlanco.setFillColor(sf::Color::Red);
					else
						rectBlanco.setFillColor(sf::Color::White);
				}

				rectBlanco.setPosition(sf::Vector2f(botones[j].x, botones[j].y/**LADO_CASILLA*/));
				window.draw(rectBlanco);
			}

			std::string mensaje_ = mensaje + "_";
			text.setString(mensaje_);
			window.draw(text);

			window.display();
			window.clear();

			if (end) {
				std::this_thread::sleep_for(std::chrono::seconds(2));
				window.close();
			}


		
		}

		receptionThread.join();

		socket->disconnect();
		delete socket;
	}
	else if (status == sf::Socket::Disconnected) {
		std::cout << "Sevidor desconectado\n";

	}
	else if (status == sf::Socket::Error) {
		std::cout << "Error de conexion\n";

	}






}