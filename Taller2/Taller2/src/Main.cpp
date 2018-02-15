#include "Server.h"
#include "Client.h"
#define MAX_MENSAJES 30

std::mutex mut;

void pushToMsjVector(std::vector<std::string>*aMsjs, std::string msj) {
	mut.lock();
	aMsjs->push_back(msj);
	while (aMsjs->size() > 25) {
		aMsjs->erase(aMsjs->begin(), aMsjs->begin() + 1);
	}
	mut.unlock();
}

void BeClient() {

}

void BeServer() {
	sf::IpAddress ip = sf::IpAddress::getLocalAddress();
	//sf::IpAddress ip = "192.168.122.247"; //ip a la que nos vamos a conectar

	sf::TcpSocket socket;
	sf::Socket::Status connectionStatus;
	char buffer[2000];
	std::size_t received;
	std::string brh = "Connected to: ";

		sf::TcpListener listener;
		connectionStatus = listener.listen(5000); //Escucha por el puerto especificado hasta que entra una peticion de conexion.
		if (connectionStatus != sf::Socket::Done) {
			std::cout << "Error al vincularse al puerto " + listener.getLocalPort() << std::endl;
		}
		connectionStatus = listener.accept(socket); //Acepta la conexion entrante
		if (connectionStatus != sf::Socket::Done) {
			std::cout << "Error al aceptar la conexión" << std::endl;
		}
		brh += "Server";
		listener.close(); //Cierra la escucha de más peticiones de conexión.
	

	//Se envia y se recive un mensaje de confirmacion de conexion.
	socket.send(brh.c_str(), brh.length() + 1);
	socket.receive(buffer, sizeof(buffer), received);

	std::cout << buffer << std::endl;

	//------------------------------------------------------------------------------------------
	//---------------------------------CONEXIÓNESTABLECIDA--------------------------------------
	//------------------------------------------------------------------------------------------
	//------------------------------------------------------------------------------------------

	std::vector<std::string> aMensajes; //Vector donde se guardan todos los mensajes que aparecen en la pantalla del chat.

	sf::Vector2i screenDimensions(800, 600);

	sf::RenderWindow window;
	window.create(sf::VideoMode(screenDimensions.x, screenDimensions.y), "Chat");

	sf::Font font;
	if (!font.loadFromFile("Barber_Street.ttf"))
	{
		std::cout << "Can't load the font file" << std::endl;
	}

	std::string mensaje = " >";

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

	//METER TODOS LOS SOCKETS EN EL VECTOR
	std::vector<sf::TcpSocket*>socketVector;
	socketVector.push_back(&socket);

	//Lanzamos el thread con el socket selector para recibir los mensajes.
	//std::thread receiveThread(&SocketSelectorMethod, &aMensajes, socketVector);

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
					socket.disconnect();
					window.close();
				}
				else if (evento.key.code == sf::Keyboard::Return) {
					pushToMsjVector(&aMensajes, mensaje);

					//Envio del mensaje
					socket.send(mensaje.c_str(), mensaje.length() + 1);
					mensaje = ">";
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
		for (size_t i = 0; i < aMensajes.size(); i++) {
			std::string chatting = aMensajes[i];
			chattingText.setPosition(sf::Vector2f(0, 20 * i));
			chattingText.setString(chatting);
			window.draw(chattingText);
		}
		std::string mensaje_ = mensaje + "_";
		text.setString(mensaje_);
		window.draw(text);


		window.display();
		window.clear();
	}
	//receiveThread.join();
}

int main() {
	char what;
	std::cout << "Que quieres hacer?\n1- Ser servidor\n2- ser cliente";
	std::cin >> what;

	switch (what) {
	case 1:
		break;
	case 2:
		break;
	default:
		std::cout << "Opcion no valida\n";
		break;
	}
	



}