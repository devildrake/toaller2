#include <Player.h>
#include <queue>
#include <SFML\Network.hpp>
#include <thread>
#include <time.h>
#define NUM_PLAYERS 6


void socketSelectorMethod(std::vector<PlayerServer*>*aPlayers, std::queue<sf::Packet>* aEventos) {
	sf::SocketSelector ss;
	sf::Socket::Status status;

	for (int i = 0; i < aPlayers->size(); i++) {
		ss.add(*aPlayers->at(i)->socket);
	}

	while (ss.wait()) {
		for (int i = 0; i < aPlayers->size(); i++) {
			if (ss.isReady(*aPlayers->at(i)->socket)) {
				sf::Packet evento;
				status = aPlayers->at(i)->socket->receive(evento);
				if (status == sf::Socket::Done) {
					aEventos->push(evento);
					std::cout << "Packet recibido y encolado\n";
				}
				else if (status == sf::Socket::Disconnected) {

					std::cout << "Desconexion de un socket\n";
					ss.remove(*aPlayers->at(i)->socket);
					delete aPlayers->at(i)->socket;
					aPlayers->at(i)->alive = false;

					sf::Packet forcedPacket;

					forcedPacket << "DISCONNECT_";
					forcedPacket << i;

					aEventos->push(forcedPacket);

				}
				else if (status == sf::Socket::Error) {
					std::cout << "Error al recibir de un socket\n";

				}


			}
		}
	}
}

//void SendPacketToClient(sf::Packet* packet,sf::TcpSocket* playerSocket) {
//	sf::Socket::Status status;
//
//	status = playerSocket->send(*packet);
//
//	if (status == sf::Socket::Done) {
//		std::cout << "Packet enviado correctamente\n";
//	}
//	else if (status == sf::Socket::Disconnected) {
//		std::cout << "El cliente esta desconectado\n";
//	}
//	else if (status == sf::Socket::Error) {
//		std::cout << "Error enviando packet a cliente\n";
//	}
//
//
//}

void EndTurn(Player::Turn* turn, std::vector<PlayerServer*>* aPlayers) {
	sf::Socket::Status status;
	switch (*turn) {
	case Player::Turn::_DAY:
		*turn = Player::Turn::_WOLVES;
		break;
	case Player::Turn::_WOLVES:
		*turn = Player::Turn::_DAY;
		break;
	}

	sf::Packet endTurnPacket;

	endTurnPacket << "ENDTURN_";

	endTurnPacket << (int)turn;

	if (*turn == Player::Turn::_DAY) {
		endTurnPacket << "Se hace de dia en CastroNegro";
	}
	else {
		endTurnPacket << "Se hace de noche en CastroNegro";

	}

	for (int i = 0; i < aPlayers->size(); i++) {

		if (aPlayers->at(i)->socket != nullptr) {
			status = aPlayers->at(i)->socket->send(endTurnPacket);
		}
		else {
			std::cout << "NULLSOCKET-> THIS IS NOT A POBLEM\n";
		}

		if (status == sf::Socket::Done) {
			std::cout << "Enviado packet de players muertos\n";
		}
		else if (status == sf::Socket::Disconnected) {
			std::cout << "Intentando enviar packet de players muertos a jugador desconectado\n";
			//aPlayers->at(i)->socket->disconnect();
			//delete aPlayers->at(i)->socket;
			//aPlayers->at(i)->socket = nullptr;


		}
		else if (status == sf::Socket::Error) {
			std::cout << "ERROR Intentando enviar packet de players muertos\n";
		}

	}

}


void ResetVotes(std::vector<PlayerServer*>*aPlayers) {
	for (int i = 0; i < aPlayers->size(); i++) {
		aPlayers->at(i)->voted = false;
		aPlayers->at(i)->currentVotes = 0;
	}
}

void ProcessVotes(std::vector<PlayerServer*>* aPlayer) {
	sf::Packet endTurnPacket;
	sf::Socket::Status status;
	//READ THIS SHIT 
	//estaba trabajando aqui y estaba a punto de gestionar quien es el mas votado y quizas establecer un metodo de cambio de turno
	//quizas es lo suyo mandar un packet PARA EL CAMBIO DE TURNO Y UNO PARA LAS MUERTES EL METODO SENDPACKETTOCLIENT ESTA ARRIBA DE RESETVOTES 
	//PERO DABA PROBLEMAS ASI QUE HE HECHO COPYPAST DEL CODIGO DEL METODO Y LO HE METIDO DONDE HAY ENVÍOS DE PACKETS
	int mostVotedID = 0;
	bool draw = false;

	for (int i = 0; i < aPlayer->size(); i++) {
		if (aPlayer->at(i)->currentVotes > aPlayer->at(mostVotedID)->currentVotes) {
			draw = false;
			mostVotedID = i;
		}
		else if (aPlayer->at(i)->currentVotes == aPlayer->at(mostVotedID)->currentVotes&&i != mostVotedID) {
			draw = true;
		}
	}

	aPlayer->at(mostVotedID)->alive = false;

	sf::Packet deadPlayersPacket;

	deadPlayersPacket << "DEAD_";

	for (int i = 0; i < aPlayer->size(); i++) {
		deadPlayersPacket << aPlayer->at(i)->alive;

		if (!aPlayer->at(i)->alive) {
			deadPlayersPacket << (int)aPlayer->at(i)->role;
		}

	}

	for (int i = 0; i < aPlayer->size(); i++) {
		
		if (aPlayer->at(i)->socket != nullptr) {
			status = aPlayer->at(i)->socket->send(deadPlayersPacket);
		}else {
			std::cout << "NULLSOCKET-> THIS IS NOT A POBLEM\n";
		}

		if (status == sf::Socket::Done) {
			std::cout << "Enviado packet de players muertos\n";
		}else if (status == sf::Socket::Disconnected) {
			std::cout << "Intentando enviar packet de players muertos a jugador desconectado\n";
		}
		else if (status == sf::Socket::Error) {
			std::cout << "ERROR Intentando enviar packet de players muertos\n";
		}

	}

}

void CheckEndGame(std::vector<PlayerServer*>*aPlayers,bool*end) {
	bool wolvesAlive=false;
	bool townsAlive=false;

	for (int i = 0; i < aPlayers->size(); i++) {
		if (aPlayers->at(i)->role == Player::ROLE::_WOLF) {
			if (aPlayers->at(i)->alive) {
				wolvesAlive = true;
			}
		}
		else {
			if (aPlayers->at(i)->alive) {
				townsAlive = true;
			}
		}
	}
	sf::Packet victoryPacket;
	if (!wolvesAlive) {
		victoryPacket << "GAMEOVER_";
		victoryPacket << "Ha ganado el pueblo";
	}
	else if (!townsAlive) {
		victoryPacket << "GAMEOVER_";
		victoryPacket << "Han ganado los lobos";
	}

	if (!wolvesAlive || !townsAlive) {
		sf::Socket::Status status;
		for (int i = 0; i < aPlayers->size(); i++) {
			if (aPlayers->at(i)->socket != nullptr) {
				status = aPlayers->at(i)->socket->send(victoryPacket);

				if (status == sf::Socket::Done) {
					std::cout << "Game over sent\n";
				}else if (status == sf::Socket::Disconnected) {
					std::cout << "Game over not sent (Disconnected client)\n";

				}else if (status == sf::Socket::Error) {
					std::cout << "Game over not sent (ERROR)\n";
				}
			}
			else {

			}
		}
		*end = true;
	}




}


void main() {
	std::cout << "Servidor iniciado. Esperando conexiones..." << std::endl;
	srand(time(NULL));
	//std::vector<PlayerServer*> witch;
	std::vector<PlayerServer*> wolves;

	PlayerServer::Turn currentTurn = PlayerServer::Turn::_DAY;

	std::vector<std::string> aMensajes;

	std::vector<PlayerServer*>aPlayers;

	std::queue<sf::Packet> aEventos;

	bool end = false;
	sf::TcpListener listener;
	sf::Socket::Status status;

	status = listener.listen(50000);


	if (status == sf::Socket::Done) {
		std::cout << "Escuchando por el puerto\n";
		while (aPlayers.size() < NUM_PLAYERS) {
			sf::TcpSocket* socket = new sf::TcpSocket();
			status = listener.accept(*socket);

			if (status == sf::Socket::Done) {
				sf::Packet namePacket;
				PlayerServer* player = new PlayerServer();
				std::string name;
				status = socket->receive(namePacket);
				
				if (status == sf::Socket::Done) {
					namePacket >> name;
					player->userName = name;
					player->role = Player::ROLE::_VILLAGER;
					player->socket = socket;

					if (wolves.size() < 3) {
						if (rand() % 2 == 1) {
							player->role = Player::ROLE::_WOLF;
							wolves.push_back(player);
						}
					}

					aPlayers.push_back(player);



					sf::Packet remainingPlayersPacket;
					int numberOfRemainingPlayers = NUM_PLAYERS - aPlayers.size();

					remainingPlayersPacket << "MSJ_";
					std::string mensaje = "Esperando a ";
					mensaje += std::to_string(numberOfRemainingPlayers);
					mensaje += " jugadores\n";

					remainingPlayersPacket << mensaje;

					//player->socket->send(remainingPlayersPacket);
					sf::Socket::Status status;
					for (int i = 0; i < aPlayers.size(); i++) {
						if (aPlayers[i]->socket != nullptr) {
							status = aPlayers[i]->socket->send(remainingPlayersPacket);

							if (status == sf::Socket::Disconnected) {
								std::cout << "ERROR POR DESCONEXION DE EMISION DE REMAINING PLAYERS\n";

								//aPlayers[i]->alive = false;
								//aPlayers[i]->socket->disconnect();
								//delete aPlayers[i]->socket;
								//aPlayers[i]->socket = nullptr;

							}
							else if (status == sf::Socket::Error) {
								std::cout << "ERROR RANDOM DE EMISION DE REMAINING PLAYERS\n";
							}
						}
					}


				}
				else if (status == sf::Socket::Disconnected) {
					std::cout << "Desconexion inesperada de socket que tenia que mandar nombre\n";
					delete player;

				}else if (status == sf::Socket::Error) {
					std::cout << "Error inesperado de socket que tenia que mandar nombre\n";
					delete player;

				}
				
			}
			else if (status == sf::Socket::Disconnected) {
				std::cout << "Desconexion inesperada de socket entrante\n";
				delete socket;
			}else if (status == sf::Socket::Error) {
				std::cout << "Error inesperado de socket entrante\n";
				delete socket;
			}



		}

		if (wolves.size() == 0) {
			aPlayers[2]->role = Player::ROLE::_WOLF;
		}

		listener.close();
		////////////////////////ALLPLAYERSCONNECTED///////////////////
		////////////////////////ALLPLAYERSCONNECTED///////////////////
		////////////////////////ALLPLAYERSCONNECTED///////////////////
		////////////////////////ALLPLAYERSCONNECTED///////////////////
		////////////////////////ALLPLAYERSCONNECTED///////////////////
		////////////////////////ALLPLAYERSCONNECTED///////////////////
		////////////////////////ALLPLAYERSCONNECTED///////////////////

		for (int i = 0; i < aPlayers.size(); i++) {
			sf::Packet playersInfoPacket;
			playersInfoPacket << "PLAYERS_";
			playersInfoPacket << i;
			playersInfoPacket << (int)aPlayers.size();
			playersInfoPacket << (int)aPlayers[i]->role;

			for (int j = 0; j < aPlayers.size(); j++) {
				playersInfoPacket << aPlayers[j]->userName;
				playersInfoPacket << j;
				playersInfoPacket << (int)aPlayers[j]->role;
			}

			if (aPlayers[i]->socket != nullptr) {
				status = aPlayers[i]->socket->send(playersInfoPacket);
			}
			if(status==sf::Socket::Disconnected){
				std::cout << "Desconexion inesperada con un socket\n";
				end = true;

					aPlayers[i]->alive = false;
					delete aPlayers[i]->socket;
					delete aPlayers[i];
					aPlayers.erase(aPlayers.begin() + i);
					//delete aPlayers[i];
					//aPlayers.erase(aPlayers.begin() + i);
				

			}
			else if (status == sf::Socket::Error) {
				std::cout << "ERROR DE EMISION\n";
				end = true;

				for (int i = 0; i < aPlayers.size(); i++) {
					delete aPlayers[i]->socket;
					delete aPlayers[i];
					aPlayers.erase(aPlayers.begin() + i);
				}

			}
			else if(status==sf::Socket::Done) {
				std::cout << "Emision de datos correcta\n";
			}


		}



		std::thread receptionMethod(&socketSelectorMethod,&aPlayers, &aEventos);

		while (!end) {
			if (aEventos.size() > 0) {
				sf::Packet packet = aEventos.front();
				std::string code;

				packet >> code;

				if (code == "MSJ_") {
					std::string mensaje;
					sf::Packet reSendPacket;
					int senderId;
					packet >> mensaje;
					packet >> senderId;
					if (aPlayers[senderId]->alive) {
						switch (currentTurn) {
						case Player::Turn::_DAY:
							reSendPacket << "MSJ_";
							reSendPacket << mensaje;

							for (int i = 0; i < aPlayers.size(); i++) {
								if (aPlayers[i]->socket != nullptr) {
									status = aPlayers[i]->socket->send(reSendPacket);
									if (status == sf::Socket::Done) {
										std::cout << "Mensaje reenviado\n";
									}
									else {
										std::cout << "Ha habido algun problema reenviando el mensaje\n";
									}
								}
							}
							break;
						case Player::Turn::_WOLVES:
							if (aPlayers[senderId]->role == Player::ROLE::_WOLF) {
								reSendPacket << "MSJ_";
								reSendPacket << mensaje;

								for (int i = 0; i < wolves.size(); i++) {
									if (wolves[i]->socket != nullptr) {
										status = wolves[i]->socket->send(reSendPacket);
										if (status == sf::Socket::Done) {
											std::cout << "Mensaje reenviado\n";
										}
										else {
											std::cout << "Ha habido algun problema reenviando el mensaje\n";
										}
									}
								}

							}
							else {
								reSendPacket << "MSJ_";
								reSendPacket << "Solo los lobos pueden hablar de noche";

									if (aPlayers[senderId]->socket != nullptr) {
										status = aPlayers[senderId]->socket->send(reSendPacket);
										if (status == sf::Socket::Done) {
											std::cout << "Mensaje reenviado\n";
										}
										else if(status==sf::Socket::Disconnected){
											std::cout << "Ha habido algun problema reenviando el mensaje\n";
											aPlayers[senderId]->socket->disconnect();
											delete aPlayers[senderId]->socket;
											aPlayers[senderId]->alive = false;
											aPlayers[senderId]->socket = nullptr;

										}
									}
								

							}
							break;
						}
					}
					else {
						reSendPacket << "MSJ_";
						reSendPacket << "Los muertos no hablan";
						if (aPlayers[senderId]->socket != nullptr) {
							status = aPlayers[senderId]->socket->send(reSendPacket);
							if (status == sf::Socket::Done) {
								std::cout << "Mensaje reenviado\n";
							}
							else {
								std::cout << "Ha habido algun problema reenviando el mensaje\n";
							}
						}
					}
				}
				else if (code == "VOTE_") {
					///////////////////////AQUI HAY PROBLEM/////////////////////////////
					///////////////////////AQUI HAY PROBLEM/////////////////////////////
					///////////////////////AQUI HAY PROBLEM/////////////////////////////
					///////////////////////AQUI HAY PROBLEM/////////////////////////////
					///////////////////////AQUI HAY PROBLEM/////////////////////////////
					//errorPolla;
					///////////////////////AQUI HAY PROBLEM/////////////////////////////
					///////////////////////AQUI HAY PROBLEM/////////////////////////////
					///////////////////////AQUI HAY PROBLEM/////////////////////////////
					bool allVoted = true;
					sf::Packet respuestaPacket;
					respuestaPacket << "MSJ_";

					int votingPlayerId;
					int votedPlayerId;
					packet >> votingPlayerId;
					packet >> votedPlayerId;
					if (votedPlayerId != -1) {
						switch (currentTurn) {
						case Player::Turn::_DAY:
							if (aPlayers[votingPlayerId]->alive) {
								if (!aPlayers[votingPlayerId]->voted) {
									if (aPlayers[votedPlayerId]->alive) {
										//////////////////SE PROCESA EL VOTO
										respuestaPacket << "SERVIDOR: Voto procesado correctamente";
										aPlayers[votingPlayerId]->voted = true;
										aPlayers[votedPlayerId]->currentVotes++;
									}
									else {
										respuestaPacket << "SERVIDOR: No se puede votar por alguien muerto";
									}
								}
								else {
									respuestaPacket << "SERVIDOR: Ya has votado este turno";
								}
							}
							else {
								respuestaPacket << "SERVIDOR: No puedes votar estando muerto";
							}

							if (aPlayers[votingPlayerId]->socket != nullptr) {
								status = aPlayers[votingPlayerId]->socket->send(respuestaPacket);
							}
							if (status == sf::Socket::Done) {
								std::cout << "Packet enviado correctamente\n";
								for (int i = 0; i<aPlayers.size(); i++) {
									if (aPlayers[i]->alive && !aPlayers[i]->voted) {
										allVoted = false;
									}
								}

								if (allVoted) {
									EndTurn(&currentTurn,&aPlayers);
									ProcessVotes(&aPlayers);
									ResetVotes(&aPlayers);
									CheckEndGame(&aPlayers, &end);
								}

							}
							else if (status == sf::Socket::Disconnected) {
								std::cout << "El cliente esta desconectado\n";
								aPlayers[votingPlayerId]->socket->disconnect();
								delete aPlayers[votingPlayerId]->socket;
								aPlayers[votingPlayerId]->socket = nullptr;


							}
							else if (status == sf::Socket::Error) {
								std::cout << "Error enviando packet a cliente\n";
							}

							//SendPacketToClient(&respuestaPacket, aPlayers[votingPlayerId]->socket);
							break;
						case Player::Turn::_WOLVES:

							if (aPlayers[votingPlayerId]->role == Player::ROLE::_WOLF) {
								if (aPlayers[votingPlayerId]->alive) {
									if (!aPlayers[votingPlayerId]->voted) {
										if (aPlayers[votedPlayerId]->alive) {
											//////////////////SE PROCESA EL VOTO
											respuestaPacket << "SERVIDOR: Voto procesado correctamente";
											aPlayers[votingPlayerId]->voted = true;
											aPlayers[votedPlayerId]->currentVotes++;
										}
										else {
											respuestaPacket << "SERVIDOR: No se puede votar por alguien muerto";
										}
									}
									else {
										respuestaPacket << "SERVIDOR: Ya has votado este turno";
									}
								}
								else {
									respuestaPacket << "SERVIDOR: No puedes votar estando muerto";
								}
							}
							else {
								respuestaPacket << "SERVIDOR: Si no eres lobo no puedes votar de noche";
							}
							//SendPacketToClient(&respuestaPacket, aPlayers[votingPlayerId]->socket);
							if (aPlayers[votingPlayerId]->socket != nullptr) {
								status = aPlayers[votingPlayerId]->socket->send(respuestaPacket);
							}
							if (status == sf::Socket::Done) {
								std::cout << "Packet enviado correctamente\n";
								
								for (int i = 0; i<aPlayers.size(); i++) {
									if (aPlayers[i]->role == Player::ROLE::_WOLF) {
										if (aPlayers[i]->alive && !aPlayers[i]->voted) {
											allVoted = false;
										}
									}
								}

								if (allVoted) {
									ProcessVotes(&aPlayers);
									EndTurn(&currentTurn, &aPlayers);
									ResetVotes(&aPlayers);
									CheckEndGame(&aPlayers, &end);
								}

							}
							else if (status == sf::Socket::Disconnected) {
								std::cout << "El cliente esta desconectado\n";
								aPlayers[votingPlayerId]->socket->disconnect();
								delete aPlayers[votingPlayerId]->socket;
								aPlayers[votingPlayerId]->socket = nullptr;
							}
							else if (status == sf::Socket::Error) {
								std::cout << "Error enviando packet a cliente\n";
							}

							break;

						}
						




					}








				}
				else if (code == "DISCONNECT_") {
					int disconnectedId;
					packet >> disconnectedId;
					sf::Packet disconnectMessagePacket;
					disconnectMessagePacket << "DISCONNECT_";
					disconnectMessagePacket << disconnectedId;
					disconnectMessagePacket << (int)aPlayers[disconnectedId]->role;
					ResetVotes(&aPlayers);
					for (int i = 0; i < aPlayers.size(); i++) {
						if (aPlayers[i]->socket != nullptr) {
							status = aPlayers[i]->socket->send(disconnectMessagePacket);
							if (status == sf::Socket::Done) {
								std::cout << "Mensaje reenviado\n";
							}
							else {
								std::cout << "Ha habido algun problema reenviando el mensaje\n";
							}
						}
						else {
							aPlayers[i]->alive = false;
						}
					}


				}


				aEventos.pop();
			}
		}

		for (int i = 0; i < aPlayers.size();i++) {
			delete aPlayers[i];
		}


		receptionMethod.join();
		system("pause");
	}
	else {
		std::cout << "Error al vincular el puerto\n";
	}
	

}