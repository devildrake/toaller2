#pragma once
#include <iostream>
#include <SFML\Network.hpp>
class Player {
public:
	std::string userName;
	enum ROLE{_WOLF,_VILLAGER,_WITCH,_CHILD};
	ROLE role;
	int id;

	std::string GetUserName() {
		return userName;
	}

	Player() {

	}

	Player(std::string n, int id) {
		userName = n;
		this->id = id;
	}

};

class PlayerServer:public Player {
public:
	sf::TcpSocket* socket;

	PlayerServer(sf::TcpSocket* s, std::string name) {
		socket = s;
		userName = name;
	}

};