#include <Player.h>
#include <queue>
#include <SFML\Network.hpp>
#include <thread>
#include <time.h>


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

void EndTurn(PlayerServer::Turn* turnoACambiar,std::vector<PlayerServer*>* aPlayers) {
	std::vector<PlayerServer*> deadPlayers;
	PlayerServer* mostVotedPlayer = aPlayers->at(0);


	switch (*turnoACambiar) {
	case PlayerServer::Turn::_DAY:
		bool draw;
		for (int i = 0; i < aPlayers->size(); i++) {
			if (aPlayers->at(i)->currentVotes > mostVotedPlayer->currentVotes) {
				mostVotedPlayer = aPlayers->at(i);
				draw = false;
			}
			else if (aPlayers->at(i)->currentVotes == mostVotedPlayer->currentVotes&&mostVotedPlayer->id!=i) {
				draw = true;
			}
		}

		if(!draw){
			sf::Packet deathNotificationPacket;
			deathNotificationPacket << "DEATH_";
			deathNotificationPacket << mostVotedPlayer->id;
			deathNotificationPacket << (int)aPlayers->at(mostVotedPlayer->id)->role;

			for (int j = 0; j < aPlayers->size(); j++) {
				aPlayers->at(j)->socket->send(deathNotificationPacket);
			}
		}


		*turnoACambiar = PlayerServer::Turn::_WOLVES;
		break;
	case PlayerServer::Turn::_WOLVES:

		for (int i = 0; i < aPlayers->size(); i++) {
			if (aPlayers->at(i)->wasAlive&&!aPlayers->at(i)->alive) {
				deadPlayers.push_back(aPlayers->at(i));
				//aPlayers->at(i)->wasAlive = false;
			}
			aPlayers->at(i)->voted = false;
		}

		for (int i = 0; i < deadPlayers.size(); i++) {
			sf::Packet deathNotificationPacket;

			deathNotificationPacket << "DEATH_";
			deathNotificationPacket << deadPlayers.at(i)->id;
			deathNotificationPacket << (int)deadPlayers.at(i)->role;
			for (int j = 0; j < aPlayers->size(); j++) {
				aPlayers->at(i)->socket->send(deathNotificationPacket);
			}
		}
		*turnoACambiar = PlayerServer::Turn::_WITCHTURN;

		break;
	case PlayerServer::Turn::_WITCHTURN:

		for (int i = 0; i < aPlayers->size(); i++) {
			if (aPlayers->at(i)->wasAlive && !aPlayers->at(i)->alive) {
				deadPlayers.push_back(aPlayers->at(i));
				aPlayers->at(i)->wasAlive = false;
			}
			aPlayers->at(i)->voted = false;
		}

		for (int i = 0; i < deadPlayers.size(); i++) {
			sf::Packet deathNotificationPacket;

			deathNotificationPacket << "DEATH_";
			deathNotificationPacket << deadPlayers.at(i)->id;
			deathNotificationPacket << (int)deadPlayers.at(i)->role;
			for (int j = 0; j < aPlayers->size(); j++) {
				aPlayers->at(j)->socket->send(deathNotificationPacket);
			}
		}

		*turnoACambiar = PlayerServer::Turn::_DAY;
		break;
	}

	//for (int i = 0; i < aPlayers->size(); i++) {
	//	sf::Packet packet;
	//	packet << "END_TURN_";
	//	aPlayers->at(i)->socket->send(packet);

	//}
	
}

void main() {
	srand(time(NULL));
	std::vector<PlayerServer*> witch;
	std::vector<PlayerServer*> wolves;

	PlayerServer::Turn currentTurn = PlayerServer::Turn::_DAY;
	int numPlayers = 6;
	sf::TcpListener listener;
	std::vector<std::string> aMensajes;
	sf::Socket::Status status;
	std::vector<PlayerServer*>aPlayers;
	bool exit = false;
	status = listener.listen(50000);
	std::queue<sf::Packet> aEventos;
	bool witchPotions[2] = { false,false };
	bool witchVotes[2] = { false,false };

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
						aPlayer->role = Player::ROLE::_VILLAGER;
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




				if (wolves.size() == 0) {

					while (wolves.size() < 3) {
						int randomNumber = rand() % numPlayers;
						if (aPlayers[randomNumber]->role==Player::ROLE::_VILLAGER) {
							aPlayers[randomNumber]->role = Player::ROLE::_WOLF;
							wolves.push_back(aPlayers[randomNumber]);
						}
					}

					while (witch.size() == 0) {
						int randomNumber = rand() % numPlayers;
						if (aPlayers[randomNumber]->role == Player::ROLE::_VILLAGER) {
							aPlayers[randomNumber]->role = Player::ROLE::_WITCH;
							witch.push_back(aPlayers[randomNumber]);
						}
					}



					for (int i = 0; i < numPlayers; i++) {


						sf::Packet infoPacket;
						infoPacket << "PLAYERS_";
						infoPacket << i;

						infoPacket << (int)aPlayers[i]->role;

						for (int j = 0; j < numPlayers; j++) {
							infoPacket << aPlayers[j]->GetUserName();
							aPlayers[j]->id = j;
							infoPacket << aPlayers[j]->id;
						}
						aPlayers[i]->socket->send(infoPacket);
						
						std::cout << "Sending info to player " << aPlayers[i]->id<<"\n";
					}

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

							sendPacket << mensaje;

							sf::TcpSocket::Status status;

							switch (currentTurn) {
							case Player::Turn::_DAY:
								for (int i = 0; i < aPlayers.size(); i++) {
									status = aPlayers[i]->socket->send(sendPacket);

									if (status != sf::TcpSocket::Done) {
										std::cout << "NotDone\n";
									}
									else {
										std::cout << "Sent\n";
									}

								}
								break;
							case Player::Turn::_WOLVES:
								for (int i = 0; i < wolves.size(); i++) {
									status = wolves[i]->socket->send(sendPacket);
									if (status != sf::TcpSocket::Done) {
										std::cout << "NotDone\n";
									}
									else {
										std::cout << "Sent\n";
									}

								}
								break;
							case Player::Turn::_WITCHTURN:
								break;
							}


							
						}
						else if (code == "VOTE_") {
							int votedPlayerId;
							int votingPlayerId;

							switch (currentTurn) {
							case PlayerServer::Turn::_DAY:

								receivedPacket >> votedPlayerId;
								receivedPacket >> votingPlayerId;

								if (!aPlayers[votingPlayerId]->voted) {
									aPlayers[votingPlayerId]->voted = true;
									aPlayers[votedPlayerId]->currentVotes++;
								}
								break;
							case PlayerServer::Turn::_WOLVES:

								receivedPacket >> votedPlayerId;
								receivedPacket >> votingPlayerId;

								if (aPlayers[votingPlayerId]->role == PlayerServer::ROLE::_WOLF) {
									if (!aPlayers[votingPlayerId]->voted) {
										aPlayers[votingPlayerId]->voted = true;
										aPlayers[votedPlayerId]->currentVotes++;
									}
								}
								break;
							case PlayerServer::Turn::_WITCHTURN:
								if (aPlayers[votingPlayerId]->role == PlayerServer::ROLE::_WITCH) {
									int toKillVotedPlayerId;
									int toHealVotedPlayerId;
									receivedPacket >> toKillVotedPlayerId;
									receivedPacket >> toHealVotedPlayerId;
									if (!witchVotes[0]) {
										if (votedPlayerId >= 0 && votedPlayerId < 13) {
											if (!witchPotions[0]) {
												witchPotions[0] = true;
												aPlayers[toKillVotedPlayerId]->alive = false;
											}
										}
										witchVotes[0] = true;
									}
									else if (!witchVotes[1]) {
										if (toKillVotedPlayerId >= 0 && toKillVotedPlayerId < 13) {
											if (aPlayers[toHealVotedPlayerId]->wasAlive && !aPlayers[toHealVotedPlayerId]->alive) {
												if (!witchPotions[1]) {
													witchPotions[1] = true;
													aPlayers[toHealVotedPlayerId]->alive = true;
													//aPlayers[toKillVotedPlayerId]->currentVotes++;
												}
											}
										}
										witchVotes[1];
									}

									//ENVIAR UN END_TURN Y RESULTADOS


								}
								break;
							}
						}
						aEventos.pop();
						bool allVoted = true;

						switch (currentTurn) {
						case PlayerServer::Turn::_DAY:
							//					bool allVoted = true;
							for (int i = 0; i < numPlayers; i++) {
								if (aPlayers[i]->alive) {
									if (!aPlayers[i]->voted) {
										allVoted = false;
										break;
									}
								}
							}

							if (allVoted) {
								//EndTurn(DAY)
								EndTurn(&currentTurn, &aPlayers);
							}


							break;
						case PlayerServer::Turn::_WOLVES:
							for (int i = 0; i < wolves.size(); i++) {
								if (wolves[i]->alive && !wolves[i]->voted) {
									allVoted = false;
									break;
								}
							}

							if (allVoted) {
								EndTurn(&currentTurn, &aPlayers);
							}

							break;
						case PlayerServer::Turn::_WITCHTURN:
							if (witch[0]!= nullptr) {
								if (witch[0]->alive) {
									if (witchVotes[0]&&witchVotes[1]) {
										EndTurn(&currentTurn, &aPlayers);
									}
								}
								else {
										EndTurn(&currentTurn, &aPlayers);
								}
							}
							break;
						}



						bool wins=true;
						for (int i = 0; i < wolves.size(); i++) {
							if (wolves[i]->alive)
								wins = false;
							break;
						}

						sf::Packet gameOverPacket;
						int whoWins = 0;

						if (wins) {

							gameOverPacket << "GAME_OVER_";
							gameOverPacket << whoWins;

							for (int i = 0; i < aPlayers.size(); i++) {
								aPlayers[i]->socket->send(gameOverPacket);
							}

						}else {
							wins = true;
							for (int i = 0; aPlayers.size(); i++) {
								if (aPlayers[i]->role != Player::ROLE::_WOLF&&aPlayers[i]->alive) {
									wins = false;
									break;
								}
							}
							if (wins) {
								whoWins = 1;
								gameOverPacket << "GAME_OVER_";
								gameOverPacket << wins;
							}

						}













						//Fin del While 
					}
				}

		
		receptionThread.join();
		////////////////////////GAMESTART///////////////////////
		////////////////////////GAMESTART///////////////////////
		////////////////////////GAMESTART///////////////////////
		////////////////////////GAMESTART///////////////////////





	}
}