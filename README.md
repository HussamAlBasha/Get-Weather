# Get-Weather
The goal of the project is to display the real time weather and characteristics of any specific city. To do so, I and my friend Charbel Chdid have used the ESP 32 that will be running as a MQTT client and HTTP client.

Role of HTTP: The data we are pursuing is stored on the internet and the best way to retrieve it is to use the HTTP protocol by which we can access the data from their URLs. For that, we will be extracting the information from “open weather map” website. The ESP 32 will be acting as client and, using the get method, will be able to read the data and return it as a JSON file. Having the data on hand, the best way to represent it to the user is using the MQTT protocol.

Role of MQTT: In order to process the data to the users, MQTT is the best solution since it offers processing data to user interfaces and it allows filtering the data that are being displayed using subscription protocol. MQTT needs a broker that plays the role of the handler. The broker is responsible for allowing clients to subscribe to different topics and to publish data in these topics. The clients send a message to the broker to subscribe to a certain topic and the broker approves it. Then, when a client publishes some data under a certain topic, all the clients subscribed to that topic will receive the data. In the project we used Node-Red software for the user interface and Mosquitto as the broker. Both the ESP32 and Node-Red are clients to the broker and are subscribed to the same topics. The user will request to get the information by choosing the country, the city and pressing a button. After that, Node-Red will send a message under the country choice topic choosing the country, the city choice topic choosing the city and the weather topic requesting the data. The broker forwards the message to the subscribers (in our case we only have the ESP32), and once the esp receives the message, it will get the info using http, sends them back to the broker under the topic weather, and finally the info will arrive to the Node-Red client and will be displayed to the user.


I did a 1 min demo of the user interface: https://youtu.be/M_sP4r040yw






