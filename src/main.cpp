#include <WiFi.h>
#include "pb_encode.h"
#include "pb_decode.h"
#include "messages.pb.h" // Include the Nanopb-generated header


// ************* INPUT SSID, PW, HOST IP **************
const char* ssid = "SSID";
const char* password = "PASSWORD";
const char* host = "HOST IP";
const uint16_t port = 3000;

WiFiClient client;

float current_angle = 42.5; // Current position of the Octave

void handleIncomingData() {
    if (client.available()) {
        uint8_t buffer[128];
        size_t len = client.read(buffer, sizeof(buffer));

        // Decode the received message
        pb_istream_t stream = pb_istream_from_buffer(buffer, len);
        octave_Packet received_packet = octave_Packet_init_default;

        if (pb_decode(&stream, octave_Packet_fields, &received_packet)) {
            if (received_packet.has_angle) {
                current_angle = received_packet.angle;
                Serial.printf("Received new angle command: %.2f\n", current_angle);
            }
        } else {
            Serial.println("Failed to decode incoming message.");
        }
    }
}

void setup() {
    Serial.begin(115200);

    // Connect to Wi-Fi
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting to Wi-Fi...");
    }
    Serial.println("Connected to Wi-Fi");

    // Connect to the server
    if (client.connect(host, port)) {
        Serial.println("Connected to server");
    } else {
        Serial.println("Connection failed");
    }
}

void loop() {
    handleIncomingData(); // Process commands from the GUI

    uint8_t buffer[128];
    size_t message_length;

    // Initialize the Protobuf stream
    pb_ostream_t stream = pb_ostream_from_buffer(buffer, sizeof(buffer));

    // Populate the Packet message
    octave_Packet packet = octave_Packet_init_default;
    packet.type = 1;                   // Set type
    packet.has_angle = true;           // Indicate that angle is set
    packet.angle = current_angle;      // Set angle value

    // Encode the message
    if (pb_encode(&stream, octave_Packet_fields, &packet)) {
        message_length = stream.bytes_written;

        // Send the data
        client.write(buffer, message_length);
        Serial.printf("Broadcasting angle: %.2f\n", current_angle);
    } else {
        Serial.println("Failed to encode message.");
    }

    delay(1000); // Send data every second
}
