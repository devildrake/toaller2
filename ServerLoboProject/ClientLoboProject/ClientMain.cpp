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

void eventMethod(sf::TcpSocket* maSocket,std::queue<sf::Packet>* aEventos,bool* end) {
	Print("Lanzado thread de recepcion");	
	while (!*end) {
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

	std::cout << "Enter your username: \n";
	//std::cin >> myUserName;
	myUserName = ((char)((rand()%57)+65));
	bool end = false;
	int whoToVote;

	std::string mensaje;
	std::vector<std::string> aMensajes;
	std::vector<sf::Vector2f> botones;
	sf::TcpSocket* socket = new sf::TcpSocket();
	sf::Socket::Status status;
	std::vector<Player>aPlayers;
	Player mySelf;

	for (size_t i = 0; i < NUM_PLAYERS; i++) {
		sf::Vector2f boton;
		boton.x = 555;
		if (i > 0) {
			boton.y = LADO_CASILLA * i+5*i;
		}
		else {
			boton.y = LADO_CASILLA * i;
		}
		botones.push_back(boton);
		//botones[i].setPosition(sf::Vector2f(555, 20 * i));
		//chattingText.setString(chatting);
		//window.draw(chattingText);
	}

	status = socket->connect(sf::IpAddress::getLocalAddress(),50000);
	
	if (status == sf::TcpSocket::Done) {
		std::thread eventThread(&eventMethod, socket, &aEvents, &end);

		//Lanzamos el thread con el socket selector para recibir los mensajes.
		//std::thread receiveThread(&receiveMethod, socket, &aMensajes, &mySelf, &aPlayers, &end,&currentTurn,&mySelf.role,&mySelf.id);
		//std::thread eventThread(&EmptyMethod,socket);
		

		Print("CONNECTED\n");



		sf::Packet nameSendPacket;

		nameSendPacket << "NAME_";
		nameSendPacket << myUserName;

		mySelf.userName = myUserName;
		socket->send(nameSendPacket);

		//AHORA NO LE LLEGA NUNCA NINGUN EVENTO POR LO VISTO
		


		////////////TODOS CONECTADOS//////////////////
		////////////TODOS CONECTADOS//////////////////
		////////////TODOS CONECTADOS//////////////////
		////////////TODOS CONECTADOS//////////////////
		////////////TODOS CONECTADOS//////////////////


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


		//LOOP DEL CHAT
		while (window.isOpen()) {

			////////////////////////////////GESTION DE PACKETS///////////////////////
			////////////////////////////////GESTION DE PACKETS///////////////////////
			////////////////////////////////GESTION DE PACKETS///////////////////////
			////////////////////////////////GESTION DE PACKETS///////////////////////
			////////////////////////////////GESTION DE PACKETS///////////////////////
			////////////////////////////////GESTION DE PACKETS///////////////////////
			////////////////////////////////GESTION DE PACKETS///////////////////////
			////////////////////////////////GESTION DE PACKETS///////////////////////
			////////////////////////////////GESTION DE PACKETS///////////////////////
			////////////////////////////////GESTION DE PACKETS///////////////////////
			if (aEvents.size() > 0) {


				sf::Packet receptionPacket;
				std::string code;

				//sf::TcpSocket::Status status;
				//receptionPacket = aEvents.front();

				receptionPacket = aEvents.front();

				receptionPacket >> code;

				if (code == "PLAYERS_") {
					int roleId;
					receptionPacket >> mySelf.id;

					receptionPacket >> roleId;

					for (int i = 0; i < 6; i++) {
						std::string userName;
						int id;

						receptionPacket >> userName;
						receptionPacket >> id;


						Player player(userName, id);

						aPlayers.push_back(player);


						mySelf = aPlayers[id];
						mySelf.role = (Player::ROLE)roleId;

						//delete aNewPlayer;
						std::cout << "Receiving player with userName " << player.GetUserName() << " and id " << player.id << "\n";
					}
					aMensajes.push_back("Welcome to the world of Castronegro's wolf");
					aMensajes.push_back("The night will start shortly, stay safe");

					std::cout << "YOUR ROLE IS " << mySelf.GetRoleAsString() << "\n";
					std::cout << "ROLE: " << mySelf.role << std::endl;

				}
				else if (code == "MSJ_") {
					std::string mensaje;
					receptionPacket >> mensaje;
					aMensajes.push_back(mensaje);
				}
				else if (code == "DEATH_") {
					aMensajes.clear();

					int deadPlayerId;
					int deadPlayerRole;
					switch (currentTurn) {
					case Player::Turn::_DAY:
						receptionPacket >> deadPlayerId;
						receptionPacket >> deadPlayerRole;
						aPlayers[deadPlayerId].role = (Player::ROLE)deadPlayerRole;

						if (deadPlayerId >= 0 && deadPlayerId <= 11) {
							if (mySelf.id == deadPlayerId) {
								aMensajes.push_back("Has muerto");

								mySelf.alive = false;
							}
							else {
								aMensajes.push_back("Ha muerto " + aPlayers[deadPlayerId].userName + ", su rol era " + aPlayers[deadPlayerId].GetRoleAsString());

								for (int i = 0; i < NUM_PLAYERS; i++) {
									if (aPlayers[i].id == deadPlayerId) {
										aPlayers[i].alive = false;
										aPlayers[i].wasAlive = false;
									}
								}
							}
						}
						currentTurn = Player::Turn::_WOLVES;
						aMensajes.push_back("Se hace de noche en Castronegro");
						if (mySelf.role == Player::ROLE::_WOLF) {
							aMensajes.push_back("Debes ponerte de acuerdo con tus hermanos lobos");
						}

						break;
					case Player::Turn::_WOLVES:
						if (mySelf.role == Player::ROLE::_WITCH) {
							if (mySelf.id == deadPlayerId) {
								mySelf.alive = false;
							}
							else {
								for (int i = 0; i < NUM_PLAYERS; i++) {
									if (aPlayers[i].id == deadPlayerId) {
										aPlayers[i].alive = false;
									}
								}
							}
						}
						currentTurn = Player::Turn::_WITCHTURN;
						aMensajes.push_back("Es el turno de la bruja");
						if (mySelf.role == Player::ROLE::_WITCH) {
							aMensajes.push_back("Debes votar aquien quieres matar");
							aMensajes.push_back("y votar a quien quieras revivir");

						}
						break;
					case Player::Turn::_WITCHTURN:
						receptionPacket >> deadPlayerId;

						if (mySelf.id == deadPlayerId) {
							mySelf.alive = false;
						}
						else {
							for (int i = 0; i < NUM_PLAYERS; i++) {
								if (aPlayers.at(i).id == deadPlayerId) {
									aPlayers.at(i).alive = false;
									aPlayers.at(i).wasAlive = false;
								}
							}
						}
						currentTurn = Player::Turn::_DAY;
						aMensajes.push_back("Se hace de dia en Castronegro");

						break;
					}

				}
				else if (code == "GAME_OVER_") {
					int whoWins;

					receptionPacket >> whoWins;

					if (whoWins == 0) {
						aMensajes.push_back("Todos los lobos han muerto, ganan los pueblos!");
					}
					else {
						aMensajes.push_back("Han muerto todos los pueblerinos, ganan los lobos!");
					}



					end = true;
				}

				aEvents.pop();

			}

			////////////////////////////////GESTION DE PACKETS///////////////////////
			////////////////////////////////GESTION DE PACKETS///////////////////////
			////////////////////////////////GESTION DE PACKETS///////////////////////
			////////////////////////////////GESTION DE PACKETS///////////////////////
			////////////////////////////////GESTION DE PACKETS///////////////////////


			sf::Event evento;
			//Gestión de eventos
			while (window.pollEvent(evento)) {
				switch (evento.type) {
				case sf::Event::Closed:
					window.close();
					break;
				case sf::Event::MouseButtonPressed:
					if (evento.mouseButton.button == sf::Mouse::Left&&mySelf.alive) {
						int x = evento.mouseButton.x;
						int y = evento.mouseButton.y;

						whoToVote = ComprovarBotonPulsado(x, y, botones);

						if (whoToVote >= 0) {

							sf::Packet votePacket;

							switch (currentTurn) {
							case Player::Turn::_DAY:
								if (mySelf.alive) {
									votePacket << "VOTE_";
									votePacket << whoToVote;
									votePacket << mySelf.id;
									socket->send(votePacket);
								}
								break;
							case Player::Turn::_WOLVES:
								if (mySelf.role == Player::ROLE::_WOLF) {

									std::cout << "As a wolf, I summon my right to vote\n";

									if (mySelf.alive) {
										votePacket << "VOTE_";
										votePacket << whoToVote;
										votePacket << mySelf.id;
										socket->send(votePacket);

										std::cout << "MY ID is " << mySelf.id << " and I am voting for the player with ID " << whoToVote << "\n";
									}
								}
								break;
							case Player::Turn::_WITCHTURN:
								if (mySelf.role == Player::ROLE::_WITCH) {
									if (mySelf.alive) {
										votePacket << "VOTE_";
										votePacket << whoToVote;
										socket->send(votePacket);
									}
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
						std::string mensajeDefinitivo = myUserName;
						mensajeDefinitivo += "-" + mensaje;
						sf::Packet packet;

						packet << "MSJ_";
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
			for (size_t i = 0; i < aMensajes.size(); i++) {
				std::string chatting = aMensajes[i];
				chattingText.setPosition(sf::Vector2f(0, 20 * i));
				chattingText.setString(chatting);
				window.draw(chattingText);
			}

			for (size_t i = 0; i < aPlayers.size(); i++) {
				//			std::string chatting = i + "-> " + aUserNames[i];
				std::string chatting = std::to_string(i) + "-> " + aPlayers.at(i).userName + " (" +aPlayers.at(i).GetRoleAsString()[0] + ")";
				chattingText.setPosition(sf::Vector2f(590, 20 * i +10*i));
				chattingText.setString(chatting);
				window.draw(chattingText);
			}

				for (int j = 0; j<aPlayers.size(); j++){
					sf::RectangleShape rectBlanco(sf::Vector2f(LADO_CASILLA, LADO_CASILLA));


					if (j == mySelf.id) {
						if (!mySelf.alive) {
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
				window.close();
			}
		}

		if (end) {
			system("pause");
		}
		eventThread.join();

		end = true;

		////////////TODOS CONECTADOS//////////////////
		////////////TODOS CONECTADOS//////////////////
		////////////TODOS CONECTADOS//////////////////
		////////////TODOS CONECTADOS//////////////////
		////////////TODOS CONECTADOS//////////////////

		//receiveThread.join();
	}
	else {
		std::cout << "ERROR\n";
	}
}


//void EmptyMethod(sf::TcpSocket*socket) {
//
//}
//void receiveMethod(sf::TcpSocket*socket, std::vector<std::string>* aMsjs, Player* mySelf, std::vector<Player>*aPlayers, bool* end, Player::Turn* currentTurn, Player::ROLE* maRole, int* myId) {
//	while (!*end) {
//		sf::Packet receptionPacket;
//		std::string code;
//
//		sf::TcpSocket::Status status;
//		status = socket->receive(receptionPacket);
//
//		if (status == sf::TcpSocket::Done) {
//			receptionPacket >> code;
//
//			if (code == "PLAYERS_") {
//				int roleId;
//				receptionPacket >> mySelf->id;
//
//				receptionPacket >> roleId;
//
//				for (int i = 0; i < 6; i++) {
//					std::string userName;
//					int id;
//
//					receptionPacket >> userName;
//					receptionPacket >> id;
//
//
//					Player player(userName,id);
//
//					aPlayers->push_back(player);
//
//
//
//
//					//delete aNewPlayer;
//					std::cout<<"Receiving player with userName " << player.GetUserName() << " and id " << player.id << "\n";
//				}
//				aMsjs->push_back("Welcome to the world of Castronegro's wolf");
//				aMsjs->push_back("The night will start shortly, stay safe");
//
//				mySelf = &aPlayers->at(mySelf->id);
//				mySelf->role = (Player::ROLE)roleId;
//
//				switch (roleId) {
//				case 0:
//					mySelf->role = Player::ROLE::_VILLAGER;
//					break;
//				case 1:
//					mySelf->role = Player::ROLE::_WOLF;
//					break;
//				case 2:
//					mySelf->role = Player::ROLE::_WITCH;
//					break;
//				case 3:
//					mySelf->role = Player::ROLE::_CHILD;
//					break;
//				}
//
//				std::cout << "YOUR ROLE IS " << mySelf->GetRoleAsString() << "\n";
//				std::cout << "ROLE: " << mySelf->role << std::endl;
//
//				*maRole = mySelf->role;
//				*myId = mySelf->id;
//
//
//			}
//			else if (code == "MSJ_") {
//				std::string mensaje;
//				receptionPacket >> mensaje;
//				aMsjs->push_back(mensaje);
//			}
//			else if (code == "DEATH_") {
//				aMsjs->clear();
//
//				int deadPlayerId;
//				int deadPlayerRole;
//				switch (*currentTurn) {
//				case Player::Turn::_DAY:
//					receptionPacket >> deadPlayerId;
//					receptionPacket >> deadPlayerRole;
//					aPlayers->at(deadPlayerId).role = (Player::ROLE)deadPlayerRole;
//
//					if (deadPlayerId >= 0 && deadPlayerId <= 11) {
//						if (mySelf->id == deadPlayerId) {
//							aMsjs->push_back("Has muerto");
//
//							mySelf->alive = false;
//						}
//						else {
//							aMsjs->push_back("Ha muerto " + aPlayers->at(deadPlayerId).userName + ", su rol era " + aPlayers->at(deadPlayerId).GetRoleAsString());
//
//							for (int i = 0; i < NUM_PLAYERS; i++) {
//								if (aPlayers->at(i).id == deadPlayerId) {
//									aPlayers->at(i).alive = false;
//									aPlayers->at(i).wasAlive = false;
//								}
//							}
//						}
//					}
//					*currentTurn = Player::Turn::_WOLVES;
//					aMsjs->push_back("Se hace de noche en Castronegro");
//					if (mySelf->role == Player::ROLE::_WOLF) {
//						aMsjs->push_back("Debes ponerte de acuerdo con tus hermanos lobos");
//					}
//
//					break;
//				case Player::Turn::_WOLVES:
//					if (mySelf->role == Player::ROLE::_WITCH) {
//						if (mySelf->id == deadPlayerId) {
//							mySelf->alive = false;
//						}
//						else {
//							for (int i = 0; i < NUM_PLAYERS; i++) {
//								if (aPlayers->at(i).id == deadPlayerId) {
//									aPlayers->at(i).alive = false;
//								}
//							}
//						}
//					}
//					*currentTurn = Player::Turn::_WITCHTURN;
//					aMsjs->push_back("Es el turno de la bruja");
//					if (mySelf->role == Player::ROLE::_WITCH) {
//						aMsjs->push_back("Debes votar aquien quieres matar");
//						aMsjs->push_back("y votar a quien quieras revivir");
//
//					}
//					break;
//				case Player::Turn::_WITCHTURN:
//					receptionPacket >> deadPlayerId;
//
//					if (mySelf->id == deadPlayerId) {
//						mySelf->alive = false;
//					}
//					else {
//						for (int i = 0; i < NUM_PLAYERS; i++) {
//							if (aPlayers->at(i).id == deadPlayerId) {
//								aPlayers->at(i).alive = false;
//								aPlayers->at(i).wasAlive = false;
//							}
//						}
//					}
//					*currentTurn = Player::Turn::_DAY;
//					aMsjs->push_back("Se hace de dia en Castronegro");
//
//					break;
//				}
//
//			}
//			else if (code == "GAME_OVER_") {
//				int whoWins;
//
//				receptionPacket >> whoWins;
//
//				if (whoWins == 0) {
//					aMsjs->push_back("Todos los lobos han muerto, ganan los pueblos!");
//				}
//				else {
//					aMsjs->push_back("Han muerto todos los pueblerinos, ganan los lobos!");
//				}
//				
//
//
//				*end = true;
//			}
//		}
//		else if(status==sf::TcpSocket::Status::Disconnected){
//			std::cout << "Desconexion del servidor\n";
//			socket->disconnect();
//			*end = true;
//		}
//		else if (status == sf::TcpSocket::Status::Error) {
//			std::cout << "ERROR CRITICO DE RECEPCIÓN\n";
//		}
//	}
//}