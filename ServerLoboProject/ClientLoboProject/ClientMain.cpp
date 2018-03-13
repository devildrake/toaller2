#include <Player.h>
#include <SFML\Network.hpp>
#include <SFML\Graphics.hpp>
#include <thread>

void receiveMethod(sf::TcpSocket*socket, std::vector<std::string>* aMsjs, int* ownId, std::vector<Player>*aPlayers,bool* start) {
	bool exit=false;
	while (!exit) {
		sf::Packet receptionPacket;
		std::string code;

		sf::TcpSocket::Status status;
		status = socket->receive(receptionPacket);

		if (status == sf::TcpSocket::Done) {
			receptionPacket >> code;

			if (code == "PLAYERS_") {
				receptionPacket >> *ownId;

				for (int i = 0; i < 6; i++) {
					std::string userName;
					int id;

					receptionPacket >> userName;
					receptionPacket >> id;


					Player player(userName,id);

					aPlayers->push_back(player);




					//delete aNewPlayer;
					std::cout<<"Receiving player with userName " << player.GetUserName() << " and id " << player.id << "\n";

				}
			}
			else if (code == "START_") {
				*start = true;
			}
			else if (code == "MSJ_") {
				std::string mensaje;
				receptionPacket >> mensaje;
				aMsjs->push_back(mensaje);
			}

		}
		else if(status==sf::TcpSocket::Status::Disconnected){
			std::cout << "Desconexion del servidor\n";
			socket->disconnect();
			exit = true;
		}
		else if (status == sf::TcpSocket::Status::Error) {
			std::cout << "ERROR CRITICO DE RECEPCIÓN\n";
		}
	}
}

void main() {
	int numPlayers = 6;
	std::string myUserName = "";
	int myId;

	std::cout << "Enter your username: \n";
	std::cin >> myUserName;


	std::string mensaje;
	std::vector<std::string> aMensajes;
	sf::TcpSocket* socket = new sf::TcpSocket();
	sf::Socket::Status status;
	std::vector<Player>aPlayers;
	bool start = false;
	status = socket->connect(sf::IpAddress::getLocalAddress(),50000);
	
	if (status == sf::TcpSocket::Done) {

		//Lanzamos el thread con el socket selector para recibir los mensajes.
		std::thread receiveThread(&receiveMethod, socket, &aMensajes, &myId, &aPlayers, &start);

		std::cout << "CONNECTED\n";



		sf::Packet nameSendPacket;

		nameSendPacket << "NAME_";
		nameSendPacket << myUserName;

		socket->send(nameSendPacket);

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
			sf::Event evento;
			//Gestión de eventos
			while (window.pollEvent(evento)) {
				switch (evento.type) {
				case sf::Event::Closed:
					window.close();
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
				std::string chatting = std::to_string(i) + "-> " + aPlayers.at(i).userName;
				chattingText.setPosition(sf::Vector2f(555, 20 * i));
				chattingText.setString(chatting);
				window.draw(chattingText);
			}

			std::string mensaje_ = mensaje + "_";
			text.setString(mensaje_);
			window.draw(text);

			window.display();
			window.clear();
		}

		////////////TODOS CONECTADOS//////////////////
		////////////TODOS CONECTADOS//////////////////
		////////////TODOS CONECTADOS//////////////////
		////////////TODOS CONECTADOS//////////////////
		////////////TODOS CONECTADOS//////////////////


		receiveThread.join();
	}
	else {
		std::cout << "ERROR\n";
	}
	

	



}