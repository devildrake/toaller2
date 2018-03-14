#pragma once
#include <iostream>
#include <SFML\Network.hpp>
class Player {
public:
	std::string userName;
	bool alive;
	//Bool para el turno anterior
	bool wasAlive;
	enum ROLE{_WOLF,_VILLAGER,_WITCH,_CHILD};
	ROLE role;
	int id;

	std::string GetUserName() {
		return userName;
	}

	Player() {
		wasAlive = true;
		alive = true;
	}

	Player(std::string n, int id) {
		userName = n;
		this->id = id;
		wasAlive = true;
		alive = true;
	}

};

class PlayerServer:public Player {
public:
	enum Turn { _DAY, _WOLVES, _WITCH };

	sf::TcpSocket* socket;
	int currentVotes = 0;
	bool voted = false;

	PlayerServer(sf::TcpSocket* s, std::string name) {
		socket = s;
		userName = name;
		currentVotes = 0;
		wasAlive = true;
		alive = true;
	}

	PlayerServer() {
		wasAlive = true;
		alive = true;
	}

};