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

void ResetVotes(std::vector<PlayerServer*>*aPlayers) {
	for (int i = 0; i < aPlayers->size(); i++) {
		aPlayers->at(i)->voted = false;
		aPlayers->at(i)->currentVotes = 0;
	}
}

void EndTurn(PlayerServer::Turn* turnoACambiar,std::vector<PlayerServer*>* aPlayers) {
	sf::TcpSocket::Status status;

	std::vector<PlayerServer*> deadPlayers;
	PlayerServer* mostVotedPlayer = aPlayers->at(0);
	int mostVotedPlayerId = 0;
	bool draw = false;

	switch (*turnoACambiar) {
	case PlayerServer::Turn::_DAY:
		for (int i = 0; i < aPlayers->size(); i++) {
			if (aPlayers->at(i)->currentVotes > mostVotedPlayer->currentVotes) {
				mostVotedPlayer = aPlayers->at(i);
				mostVotedPlayerId = i;
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

				status = aPlayers->at(j)->socket->send(deathNotificationPacket);

				if (status != sf::TcpSocket::Status::Done) {
					std::cout << "Error de emisionA\n";
				}

			}
		}


		*turnoACambiar = PlayerServer::Turn::_WOLVES;
		break;

		//Acaba el turno de los lobos
	case PlayerServer::Turn::_WOLVES:


		for (int i = 0; i < aPlayers->size(); i++) {

			if (aPlayers->at(i)->currentVotes > mostVotedPlayer->currentVotes) {
				mostVotedPlayer = aPlayers->at(i);
				mostVotedPlayerId = i;
				draw = false;
			}
			else if (aPlayers->at(i)->currentVotes == mostVotedPlayer->currentVotes&&mostVotedPlayer->id != i) {
				draw = true;
				//mostVotedPlayer->wasAlive = true;
				//mostVotedPlayer->alive = false;



			}
		}

		aPlayers->at(mostVotedPlayerId)->wasAlive = true;
		aPlayers->at(mostVotedPlayerId)->alive = false;

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
				status = aPlayers->at(j)->socket->send(deathNotificationPacket);

				if (status != sf::TcpSocket::Status::Done) {
					std::cout << "Error de emisionB\n";
				}

			}

			std::cout << "DeadPlayerMeansEndTurn\n";

		}
		*turnoACambiar = PlayerServer::Turn::_WITCHTURN;
		//for (int j = 0; j < aPlayers->size(); j++) {
		//	if (aPlayers->at(j)->role == Player::ROLE::_WITCH) {
		//		if (!aPlayers->at(j)->alive) {
		//			EndTurn(turnoACambiar,aPlayers);
		//		}
		//	}
		//}
		break;
	case PlayerServer::Turn::_WITCHTURN:

		for (int i = 0; i < aPlayers->size(); i++) {
			if (aPlayers->at(i)->wasAlive && !aPlayers->at(i)->alive) {
				deadPlayers.push_back(aPlayers->at(i));
				aPlayers->at(i)->wasAlive = false;
			}
			aPlayers->at(i)->voted = false;
		}
		if (deadPlayers.size() > 0) {
			for (int i = 0; i < deadPlayers.size(); i++) {
				sf::Packet deathNotificationPacket;

				deathNotificationPacket << "DEATH_";
				deathNotificationPacket << deadPlayers.at(i)->id;
				deathNotificationPacket << (int)deadPlayers.at(i)->role;

				for (int j = 0; j < aPlayers->size(); j++) {
					status = aPlayers->at(j)->socket->send(deathNotificationPacket);

					if (status != sf::TcpSocket::Status::Done) {
						std::cout << "Error de emisionC\n";
					}

				}
			}
		}
		else {
			for (int i = 0; i < aPlayers->size(); i++) {
				sf::Packet packet;
				packet << "END_TURN_";
				status = aPlayers->at(i)->socket->send(packet);

				if (status != sf::TcpSocket::Status::Done) {
					std::cout << "Error de emisionF\n";
				}

				switch (*turnoACambiar) {
				case Player::Turn::_DAY:
					*turnoACambiar = Player::Turn::_WOLVES;
					break;
				case Player::Turn::_WOLVES:
					*turnoACambiar = Player::Turn::_WITCHTURN;
					break;
				case Player::Turn::_WITCHTURN:
					*turnoACambiar = Player::Turn::_DAY;
					break;
				}
				status = aPlayers->at(i)->socket->send(packet);

				if (status != sf::TcpSocket::Status::Done) {
					std::cout << "Error de emisionD\n";
				}

			}
		}

		*turnoACambiar = PlayerServer::Turn::_DAY;
		break;
	}


	
}

void main() {
	std::cout << "Servidor iniciado. Esperando conexiones..." << std::endl;
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
					std::cout << "Ha llegado algo" << std::endl;
					std::string receiveCode;
					namePacket >> receiveCode;

					if (receiveCode == "NAME_") {
						std::cout << "Receiving name\n";
						std::string aUserName;
						namePacket >> aUserName;
						PlayerServer* aPlayer = new PlayerServer(socket, aUserName);
						aPlayer->id = i;
						std::cout << "Player with id " << aPlayer->id << " and named " << aPlayer->GetUserName() << "\n";
						aPlayer->role = Player::ROLE::_VILLAGER;
						aPlayers.push_back(aPlayer);
						namePacket.clear();
					}
					else {
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

			std::cout << "Se asignan " << wolves.size() << " lobos\n";

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
				int roleInt = (int)aPlayers[i]->role;
				//roleInt = 1;
				infoPacket << roleInt;

				std::cout << "INTEGER " << i << "\n";

				for (int j = 0; j < numPlayers; j++) {
					infoPacket << aPlayers[j]->GetUserName();
					aPlayers[j]->id = j;
					infoPacket << aPlayers[j]->id;
				}
				status = aPlayers[i]->socket->send(infoPacket);
				if (status != sf::Socket::Done) {
					std::cout << "La informacion no se ha podido mandar correctamente" << std::endl;
				}
				else {
					std::cout << "Sending info to player " << aPlayers[i]->id << "\n";
				}
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
					int senderID;
					receivedPacket >> senderID;
					receivedPacket >> mensaje;
					sendPacket << "MSJ_";

					sendPacket << mensaje;

					sf::TcpSocket::Status status;
					if (aPlayers[senderID]->alive) {

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
							if (aPlayers[senderID]->role == PlayerServer::ROLE::_WOLF) {
								for (int i = 0; i < wolves.size(); i++) {
									status = wolves[i]->socket->send(sendPacket);
									if (status != sf::TcpSocket::Done) {
										std::cout << "NotDone\n";
									}
									else {
										std::cout << "Sent\n";
									}

								}
							}
							break;
						case Player::Turn::_WITCHTURN:




							break;
						}
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
							std::cout << "Player with id " << votingPlayerId << "voted for player with id " << votedPlayerId << "\n";

						}
						break;
					case PlayerServer::Turn::_WOLVES:

						receivedPacket >> votedPlayerId;
						receivedPacket >> votingPlayerId;

						if (aPlayers[votingPlayerId]->role == PlayerServer::ROLE::_WOLF) {
							if (votedPlayerId > 0 && votedPlayerId < numPlayers) {
								if (!aPlayers[votingPlayerId]->voted) {
									if (aPlayers[votingPlayerId]->alive) {
										aPlayers[votingPlayerId]->voted = true;
										aPlayers[votedPlayerId]->currentVotes++;
									}
									else {
										aPlayers[votingPlayerId]->voted = true;
									}
								}
							}
						}
						break;
					case PlayerServer::Turn::_WITCHTURN:
						receivedPacket >> votingPlayerId;
						if (aPlayers[votingPlayerId]->role == PlayerServer::ROLE::_WITCH) {
							int targetPlayerId;
							receivedPacket >> targetPlayerId;
							if (!witchVotes[0]) {
								if (votedPlayerId >= 0 && votedPlayerId < 13) {
									if (!witchPotions[0]) {
										witchPotions[0] = true;
										aPlayers[targetPlayerId]->alive = false;

										std::cout << "Kill player with id " << targetPlayerId << "\n";

									}
								}
								witchVotes[0] = true;
							}
							else if (!witchVotes[1]) {
								if (targetPlayerId >= 0 && targetPlayerId < 13) {
									if (aPlayers[targetPlayerId]->wasAlive && !aPlayers[targetPlayerId]->alive) {
										if (!witchPotions[1]) {
											witchPotions[1] = true;
											aPlayers[targetPlayerId]->alive = true;
											std::cout << "Revive player with id " << targetPlayerId << "\n";
											std::cout << " he is now alive is " << aPlayers[targetPlayerId]->alive;
											//aPlayers[toKillVotedPlayerId]->currentVotes++;
										}
									}
								}
								witchVotes[1] = true;
								EndTurn(&currentTurn, &aPlayers);

								witchVotes[0] = false;
								witchVotes[1] = true;
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

					for (int i = 0; i < aPlayers.size(); i++) {
						if (aPlayers[i]->alive) {
							if (!aPlayers[i]->voted) {
								allVoted = false;
								//break;
							}
						}

					}

					if (allVoted) {
						//EndTurn(DAY)
						EndTurn(&currentTurn, &aPlayers);
						ResetVotes(&aPlayers);
					}
					else {
						for (int i = 0; i < aPlayers.size(); i++) {
							if (!aPlayers[i]->voted) {
								std::cout << "El player con id " << aPlayers[i]->id << " falta por votar\n";

								if (aPlayers[i]->alive) {
									std::cout << "El player con id " << aPlayers[i]->id << "Esta vivo\n";

								}
							}

						}
					}


					break;
				case PlayerServer::Turn::_WOLVES:
					allVoted = true;
					for (int i = 0; i < wolves.size(); i++) {
						if (wolves[i]->alive && !wolves[i]->voted) {
							allVoted = false;
							break;
						}
					}

					if (allVoted) {
						EndTurn(&currentTurn, &aPlayers);
						ResetVotes(&aPlayers);
					}
					else {
						for (int i = 0; i < wolves.size(); i++) {
							if (!wolves[i]->voted&&wolves[i]->alive) {
								std::cout << "El lobo con id " << wolves[i]->id<<" falta por votar\n";
							}
						}
					}

					for (int j = 0; j < aPlayers.size(); j++) {
						if (aPlayers[j]->role == Player::ROLE::_WITCH) {
							if (!aPlayers[j]->alive) {

								EndTurn(&currentTurn,&aPlayers);

							}
						}
					}



					break;
				case PlayerServer::Turn::_WITCHTURN:
					if (witch[0]!= nullptr) {
						if (witch[0]->alive) {
							if (witchVotes[0]&&witchVotes[1]) {
								EndTurn(&currentTurn, &aPlayers);
								ResetVotes(&aPlayers);
							}
						}
						else {
								EndTurn(&currentTurn, &aPlayers);
								ResetVotes(&aPlayers);

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
						status = aPlayers[i]->socket->send(gameOverPacket);

						if (status != sf::TcpSocket::Status::Done) {
							std::cout << "Error de emisionE\n";
						}

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