#include <iostream>
#include <string>
#include <SFML\Network.hpp>
#include <SFML\Graphics.hpp>
#include <Player.h>
#include <queue>
#include <thread>
#define NUMPLAYERS 6
#define LADO_CASILLA 25


int ComprovarBotonPulsado(int _x, int _y, std::vector<sf::Vector2f>botones,std::vector<PlayerServer>*aPlayers) {
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

	if (idPulsado != -1) {
		return aPlayers->at(idPulsado).id;
	}
	else {
		return -1;
	}
}

void SendPacketToAllPeers(sf::Packet aPacket,std::vector<PlayerServer>*aPlayers) {
	sf::Socket::Status status;
	for (int i = 0; i < aPlayers->size(); i++) {
		if (aPlayers->at(i).socket != nullptr) {
			status = aPlayers->at(i).socket->send(aPacket);

			if (status == sf::Socket::Done) {
				std::cout << "PACKET ENVIADO\n";
			}
			else if(status==sf::Socket::Disconnected){
				aPlayers->at(i).socket->disconnect();
				aPlayers->at(i).socket = nullptr;
				aPlayers->at(i).alive = false;
				std::cout << "DESCONEXION DE UN PEER\n";
			}
			else {
				std::cout << "ERROR DE EMISION DE PACKET\n";
			}

		}
	}

}

void ResetVotes(std::vector<PlayerServer>*aPlayers){
	for (int i = 0; i < aPlayers->size(); i++) {
		aPlayers->at(i).voted = false;
		aPlayers->at(i).currentVotes = 0;
	}
}

void socketSelectorMethod(std::vector<PlayerServer>*aPlayers, std::queue<sf::Packet>* aEventos) {
	sf::SocketSelector ss;
	sf::TcpSocket::Status status;
	for (int i = 0; i < aPlayers->size(); i++) {
		if(aPlayers->at(i).socket!=nullptr)
		ss.add(*aPlayers->at(i).socket);
	}
	while (ss.wait()) {
		for (int i = 0; i < aPlayers->size(); i++) {
			if (aPlayers->at(i).socket != nullptr) {
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
						aPlayers->at(i).socket = nullptr;
						aPlayers->at(i).alive = false;
						//aPlayers->erase(aPlayers->begin() + i);
					}
					else if (status == sf::TcpSocket::Status::Error) {
						std::cout << "Receiving ERROR\n";
					}

				}
			}
		}
	}
}

int CheckWin(std::vector<PlayerServer>*aPlayers) {
	int alivePlayers=0;
	int aliveWolves=0;
	for (int i = 0; i < aPlayers->size();i++) {
		if (aPlayers->at(i).alive) {
			alivePlayers++;
			if (aPlayers->at(i).role == Player::ROLE::_WOLF) {
				aliveWolves++;
			}
		}
	}

	if (alivePlayers == aliveWolves) {
		return 2;
	}
	else if(aliveWolves==0) {
		return 1;
	}
	return 0;
}

void KillMostVotedPlayer(std::vector<PlayerServer>*aPlayers,Player::Turn* turnToChange,std::vector<std::string>*aMsjs) {
	bool draw = false;
	int mostVotedPlayerIndex = 0;
	for (int i = 0; i < aPlayers->size(); i++) {
		if (aPlayers->at(i).currentVotes>aPlayers->at(mostVotedPlayerIndex).currentVotes) {
			mostVotedPlayerIndex = i;
			draw = false;
		}
		else if (i != mostVotedPlayerIndex) {
			if (aPlayers->at(i).currentVotes == aPlayers->at(mostVotedPlayerIndex).currentVotes) {
				draw = true;
			}
		}
	}

	if (!draw) {
		aPlayers->at(mostVotedPlayerIndex).alive = false;
	}


	aMsjs->clear();

	aMsjs->push_back("Ha muerto  " + aPlayers->at(mostVotedPlayerIndex).userName);

	ResetVotes(aPlayers);


	switch (*turnToChange) {
		case Player::Turn::_DAY:
			aMsjs->push_back("Anochece y los lobos buscan su nueva presa");

			*turnToChange = Player::Turn::_WOLVES;
			break;
		case Player::Turn::_WOLVES:
			aMsjs->push_back("Se hace de dia");

			*turnToChange = Player::Turn::_DAY;
			break;
	}


}

int main() {
	Player::Turn currentTurn = Player::Turn::_DAY;
	Player::ROLE myRole;
	int myPort;
	int myID;
	bool thereWasAVote = false;
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
				mySelf->role = myRole;
				aPlayers.push_back(*mySelf);
				incomingInfo >> previousPeers;

				for (int i = 0; i < previousPeers; i++) {
					PlayerServer player;
					std::string ip;
					int port;


					sf::TcpSocket* socket = new sf::TcpSocket();
					std::string aName;
					incomingInfo >> ip;
					incomingInfo >> port;
					incomingInfo >> player.id;
					incomingInfo >> aName;//player.userName;
					int roleId;
					incomingInfo >> roleId;

					player.role = (Player::ROLE)roleId;

/////////////////////////ROl
					player.userName = aName;

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

				/*
				 * TENDRIAMOS QUE HACER UN THREAD PARA LOS RECIEVE, PORQUE SI BLOQUEAMOS EN EL RECEIVE Y EN ESE MOMENTO SE INTENTA CONECTAR
				 * OTRO PEER ESTE NO VA A PODER HACERLO.
				 */
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
								infoPacket >> command;
								if (command == "INFOP_") {
									std::cout << "Receiving new peer info\n";
									PlayerServer newPlayer;
									newPlayer.socket = newSocket;
									infoPacket >> newPlayer.id;
									infoPacket >> newPlayer.userName;
									infoPacket >> roleId;
									newPlayer.role = (Player::ROLE)roleId;
									aPlayers.push_back(newPlayer);

									//std::cout << "Informacion del nuevo peer recibida correctamente\n nombre de usuario "<<newPlayer.userName<<"\n" << std::endl;
									std::cout << "Informacion del nuevo peer recibida correctamente\n ROL--> "<<newPlayer.role<<"\n" << std::endl;



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

				for (size_t i = 0; i < NUMPLAYERS; i++) {
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
				bool end=false;
				
				//LOOP DEL CHAT
				while (window.isOpen()) {

					if (thereWasAVote) {
						bool allVoted = true;


						switch (currentTurn) {
						case Player::_DAY:
							for (int i = 0; i < aPlayers.size(); i++) {
								if (aPlayers[i].alive) {
									if (!aPlayers[i].voted) {
										allVoted = false;

										aMensajes.push_back(aPlayers[i].GetUserName() + " no ha votado ");


									}
								}
							}
							break;
						case Player::_WOLVES:
							for (int i = 0; i < aPlayers.size(); i++) {
								if (aPlayers[i].alive) {
									if (aPlayers[i].role == Player::ROLE::_WOLF) {
										if (!aPlayers[i].voted) {
											allVoted = false;
										}
									}
								}
							}
							break;
						}

						if (allVoted) {
							KillMostVotedPlayer(&aPlayers,&currentTurn,&aMensajes);
							int whoWon = CheckWin(&aPlayers);

							if (whoWon == 2) {
								aMensajes.clear();
								aMensajes.push_back("Han ganado los lobos");
							}
							else if (whoWon == 1) {
								aMensajes.clear();
								aMensajes.push_back("Han ganado los pueblos");
							}


							mySelf->voted = false;
							//aPlayers[mySelf->id].voted = true;



						}


						thereWasAVote = false;
					}


					if (aEvents.size() > 0) {
						sf::Packet packet;
						packet = aEvents.front();
						packet >> command;

						if (command == "MSG_") {
							std::string mensaje;
							packet >> mensaje;
							switch (currentTurn) {
							case Player::Turn::_DAY:
								aMensajes.push_back(mensaje);
								break;
							case Player::Turn::_WOLVES:
								if (mySelf->role == Player::ROLE::_WOLF) {
									aMensajes.push_back(mensaje);
								}
								break;
							}

						}
						else if (command == "VOTE_") {
							int votingPlayerIndex;
							int votedPlayerIndex;

							int votedPlayer;
							int votingPlayer;

							packet >> votedPlayer;
							packet >> votingPlayer;

							for (int i = 0; i < aPlayers.size(); i++) {
								if (aPlayers[i].id == votedPlayer) {
									votedPlayerIndex = i;
								}
								else if (aPlayers[i].id == votingPlayer) {
									votingPlayerIndex = i;
								}
							}


							
							if (currentTurn == Player::Turn::_DAY) {
								if (!aPlayers[votingPlayerIndex].voted) {
									if (aPlayers[votedPlayerIndex].alive) {
										aMensajes.push_back("Recibido voto de " + aPlayers[votingPlayerIndex].userName);
										aPlayers[votingPlayerIndex].voted = true;
										aMensajes.push_back("Voted = " + aPlayers[votingPlayerIndex].voted);

										aPlayers[votedPlayerIndex].currentVotes++;
										thereWasAVote = true;
									}
								}
							}
							else {
								if (aPlayers[votingPlayerIndex].role == Player::ROLE::_WOLF) {
									if (!aPlayers[votingPlayerIndex].voted) {
										if (aPlayers[votedPlayerIndex].alive) {
											aPlayers[votingPlayerIndex].voted = true;
											aPlayers[votedPlayerIndex].currentVotes++;
											thereWasAVote = true;
										}
									}
								}
							}

/*
							switch (currentTurn) {
							case Player::Turn::_DAY:
								if (!aPlayers[votingPlayerIndex].voted) {
									if (aPlayers[votedPlayerIndex].alive) {
										aPlayers[votingPlayerIndex].voted = true;
										aPlayers[votedPlayerIndex].currentVotes++;
										thereWasAVote = true;
									}
								}
								break;
							case Player::Turn::_WOLVES:
								if (aPlayers[votingPlayerIndex].role == Player::ROLE::_WOLF) {
									if (!aPlayers[votingPlayerIndex].voted) {
										if (aPlayers[votedPlayerIndex].alive) {
											aPlayers[votingPlayerIndex].voted = true;
											aPlayers[votedPlayerIndex].currentVotes++;
											thereWasAVote = true;
										}
									}
								}
								break;
							}



							std::cout << "Player with id " << votingPlayer << " wanted to vote for player with id " << votedPlayer << "\n";*/


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

								whoToVote = ComprovarBotonPulsado(x, y, botones,&aPlayers);

								if (whoToVote >= 0) {

									sf::Packet votePacket;
									PlayerServer whichPlayer;

									for (int i = 0; i < aPlayers.size(); i++) {
										if (aPlayers[i].id == whoToVote) {
											whichPlayer = aPlayers[i];
										}
									}

									switch (currentTurn) {
									case Player::Turn::_DAY:
										//if (mySelf.alive) 
										if (!mySelf->voted) {
											if (whichPlayer.alive) {
												votePacket << "VOTE_";
												votePacket << whoToVote;
												votePacket << mySelf->id;

												if(whichPlayer.alive){

													SendPacketToAllPeers(votePacket, &aPlayers);
													aMensajes.push_back("Has votado por " + whichPlayer.GetUserName());
													mySelf->voted = true;
													thereWasAVote = true;

													for (int i = 0; i < aPlayers.size(); i++){
														int myIndex;
														if (aPlayers[i].id == mySelf->id) {
															myIndex = i;
														}
														aPlayers[myIndex].voted = true;
													}

												}



											}
											else {
												aMensajes.push_back("No puedes votar a alguien ya muerto");
											}
										}
										else {
											aMensajes.push_back("Ya has votado este turno");
										}

										//status = socket->send(votePacket);

										//if (status != sf::TcpSocket::Status::Done) {
										//	std::cout << "Error de emision\n";
										//}
										//}
										break;
									case Player::Turn::_WOLVES:
										if (mySelf->role == Player::ROLE::_WOLF) {

											std::cout << "As a wolf, I summon my right to vote\n";

											//if (mySelf.alive) {
											votePacket << "VOTE_";
											votePacket << whoToVote;
											votePacket << mySelf->id;


											mySelf->voted = true;
											thereWasAVote = true;

											for (int i = 0; i < aPlayers.size(); i++) {
												int myIndex;
												if (aPlayers[i].id == mySelf->id) {
													myIndex = i;
												}
												aPlayers[myIndex].voted = true;
											}


											//status = socket->send(votePacket);

											SendPacketToAllPeers(votePacket, &aPlayers);

											//if (status != sf::TcpSocket::Status::Done) {
											//	std::cout << "Error de emision\n";
											//}

											std::cout << "MY ID is " << mySelf->id << " and I am voting for the player with ID " << whoToVote << "\n";
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
								if (mySelf->alive) {

									std::string mensajeDefinitivo = mySelf->userName;
									mensajeDefinitivo += "-" + mensaje;
									sf::Packet packet;

									packet << "MSG_";
									//packet << mySelf->id;
									packet << mensajeDefinitivo;
									if (mensaje == " >/Disconnect") {
										//Close(aSock, finish);
										socket->disconnect();
										window.close();
									}

									switch (currentTurn) {
									case Player::Turn::_DAY:
										//Envio del mensaje
										SendPacketToAllPeers(packet, &aPlayers);
										aMensajes.push_back(mensajeDefinitivo);
										break;
									case Player::Turn::_WOLVES:
										//Envio del mensaje
										SendPacketToAllPeers(packet, &aPlayers);
										aMensajes.push_back(mensajeDefinitivo);
										break;
									}

									//pushToAMsj(&aMensajes, mensajeDefinitivo);
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
						std::string role;


						if (!aPlayers[i].alive) {
							role = aPlayers.at(i).GetRoleAsString()[0];
						}
						else if(aPlayers[i].id!=mySelf->id){
							role = "?";
						}
						else {
							role = aPlayers.at(i).GetRoleAsString()[0];
						}

						if (mySelf->role == Player::ROLE::_WOLF) {
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


						if (aPlayers[j].id == mySelf->id) {
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
						window.close();
					}
					


				}

				threadRecepcion.join();


			}

		}
	}
}