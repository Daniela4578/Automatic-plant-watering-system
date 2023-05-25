#include <ESP8266WiFi.h>


const char* ssid = "YOUR_SSID";
const char* password = "YOUR_PASSWORD";

int ledPin = D5;
WiFiServer server(80);
unsigned long previousMillis = 0;
const unsigned long interval = 5000;  // 5 seconds


void setup() {
  Serial.begin(9600);
  delay(10);

  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);

  // Connect to WiFi network
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  // Start the server
  server.begin();
  Serial.println("Server started");

  // Print the IP address
  Serial.print("Use this URL : ");
  Serial.print("http://");
  Serial.print(WiFi.localIP());
  Serial.println("/");
}

void loop() {
  WiFiClient client = server.available();
  if (!client) {
    return;
  }

  // Check if it's time to read soil moisture
  waterSoil();

  String request = client.readStringUntil('\r');
  Serial.println(request);

  int value = LOW;
  if (request.indexOf("/LED=ON") != -1) {
    digitalWrite(ledPin, HIGH);
    value = HIGH;
  }
  if (request.indexOf("/LED=OFF") != -1) {
    digitalWrite(ledPin, LOW);
    value = LOW;
  }
  if (request.indexOf("/soil-moisture") != -1) {
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/plain");
    client.println("");
    client.println(String(checkSoil()));
    return;
  }

  newRootTest(client);

  Serial.println("Client disconnected");
  Serial.println("");
}


void newRootTest(WiFiClient client) {
  client.println("<!DOCTYPE html>");
  client.println("<html lang=\"en\">");
  client.println("<head>");
  client.println("    <meta charset=\"UTF-8\">");
  client.println("    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">");
  client.println("    <meta http-equiv=\"X-UA-Compatible\" content=\"ie=edge\">");
  client.println("    <title>Влажност на почвата</title>");
  client.println("    <link rel=\"stylesheet\" href=\"https://stackpath.bootstrapcdn.com/bootstrap/4.5.0/css/bootstrap.min.css\">");
  client.println("    <link rel=\"shortcut icon\" type=\"image\" href=\"data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAEAAAABACAYAAACqaXHeAAAACXBIWXMAAAsTAAALEwEAmpwYAAAFeklEQVR4nO2aXUwUVxTHj1ZtoUqoTU1aNZXW9qGRXUDZoamNiV/QqH1oaF/aPrQ2JtpWRJ6aNG36Ul3Q+tnaajXRpjCACsuuAn5U2lSDuwuzivKlQ9OggnZnFnZFYMtymrsMFdkddnZ2hsVl/slJyHzce/6/c++de7MAaNKkSZMmTZo0adKkSdNE0dcV8WCk94ORdvkjn94HO0viYNLIWFwE+cX4SJBrk0LbS7IDzA/H9qJ3IKa1qywRjPQtUQBGugO2/foMxKyM9EFR8w+nwo8Qk9pG68FID4QGQJ4p1EHMKZ8+E9L8w/gNYkrb6cwwzAtRtBommt69hjM23fB8k8N6mnJZd09um+f+FvZ+48bWrq822HG66IvG4rPhAyiuGhdTG294Pv+s2ZUU6rkNrV0vb77pbslr82Cw2HzT0xi0nQJ6ERjpQRkAcKy1IINxzqUY/oOIAeSx7sHcNs+9T5pdy0XNN7tWbGE9TjHzw5HV4LqdccWV9sjLRnqPPPP+L8KuYPlQDidlcPC3KIbzRQ6gbSj5rW3ufzex7iJi9qNW93Prr3XNJlA+Zd2FW9vcA+SZbe09WMH14bftPQHms5u6kHLwSDk4r4Hh9qTXdS0x1DoToKCkWS6A6XvKncPt6BlXot84w/9A+hjqi0fFAOSFiKN3e7HXN4hE/3h9Q8Oe9eDHrW7MvOYSzAfG1J2l8qqfX4xTd5SKtqs6gC/+uo/77jxAC9eP7X0+HKm7Xh+ubBA3PTKm7TopG8D03WXRAXDe1Y9CsQN0z+vDD1vcksyTmPlTpWwAMw9WRQdA/2Cge+8gopnrw1USKz8cc0/YZQOYd8IevRHgHUS80+/Di91e3H27B98aY56PFamXO/xzWc78T63tiA6AVWNUOclyFWfsNeGTe02YdKpBEgRSybCrX1YnqW1VAFAinb12gUUoKHmYaEGJ/1rIRBkOZx+7INn8s0dr/O9MKADp9U6M+94SkOxT+82YXucMmajBweELx604pUB8OkzZUYpzS62SzY8rgHknxYcxuSc1Yf2lW/h8SS3GHziFT3x33B/kb3KN3JPSxpJL7Zha1YB6s115ADlsIIC0y53+RMUAkHtp1k5Zi2S4sbjmBupNtv9DcQDrWwO/8XMK/ww5b+cUXlTdPKn8SPOqAMga9bkjlZWynSWfLTJS1ATgH/ZqAnivqTusuS/30yU3yJxXHEAOOzTsR1d+OOIPnJYM4OkDleoCGGVeEQBUiE7DOc2RZ2MOwLQwTnNSTm+PHYDEI+ckA0g8cj72ACysapQMYGF102MJoHvMjhkOE34+G9J8wuFzYW1hw456ZxAAVrcSAK6H6nyx7S7OOlQtan7WoTP+Z8Z7E6Qvt7ZEDoDhD0tJgBxmkiwNfhDDe3jyN7lG7qlpnoyslMorAQB0ZvsvEQMwMM7VqiavwNBPPXM96PzXW5g1EQMAxCkUw/0RdaOjKm6wdeLi31lMsdQHNa+rsNlJ7pEDAADyY4aB4XvVNDXyCBu0mmGErtzapzczBlBS1BX+bYODGxiPI2xEUW716U/b3wc1lOHg1xkYjle68kqZ11XYXMnm+mxQU6/X8S9SDHdMqdEQ7AgbtnGTdSDFbCvWVTtC/oirmJZc7XqJcnB5lIOvphiONTj4B3IAyJnzOpOtV2+y/q032Wv0lrovU8z1r4ybcU2aNGnSpClA0wAgAQDmAMB8AHgVAJIBIB0AlgLAmwCwDADI/xmtBIBMAMgCgLVCZAnXVgjPLBPeWSq0kSy0OV/oI0HoM2qKB4AFZDsgJLw2SrFcyGGBkNO4SAcAa6JoWizWCCNFdSVPdgATdQrETbRFcJGwgL0xahEkC53URZC8S9ogbZH9vuKL4H+VBX+zZWTf5AAAAABJRU5ErkJggg==\">");
  client.println("    <style>");
  client.println("        body {");  // Change the selector from body1 to body
  client.println("                        background: rgb(170,255,249); background: linear-gradient(313deg, rgba(170,255,249,1) 100%, rgba(101,230,62,1) 100%);");
  client.println("        }");
  client.println("        .header-color-custom{ background: rgb(158,237,232); background: linear-gradient(313deg, rgba(158,237,232,1) 100%, rgba(101,230,62,1) 100%);}");
  client.println("");
  client.println("        .card-custom-color {");
  client.println("           background: rgb(0,223,247); background: linear-gradient(313deg, rgba(0,223,247,1) 100%, rgba(101,230,62,1) 100%);");
  client.println("        }");
  client.println("    </style>");
  client.println("</head>");
client.println("<body>");
client.println("    <header>");
client.println("        <nav class=\"navbar navbar-expand-lg navbar-light bg-light header-color-custom\">");
client.println("            <a class=\"navbar-brand mx-auto font-weight-bold h4\" href=\"#\">Следене на влажността на почвата</a>");
client.println("        </nav>");
client.println("    </header>");
client.println("    <div class=\"container\">");
client.println("        <main role=\"main\" class=\"pb-3\">");
client.println("            <div class=\"container-fluid h-100 pt-5\">");
client.println("                <div class=\"row h-100 justify-content-center align-items-center\">");
client.println("                    <div class=\"col-md-6 col-lg-4\">");
client.println("                        <div class=\"card h-100 d-flex flex-column justify-content-center align-items-center card-custom-color\">");
client.println("                            <div class=\"card-body text-center\">");
client.println("                                <h1 class=\"card-title mb-4\">Влажност: <br /> <span id=\"soil-moisture\">" + String(checkSoil()) + "</span> %</h1>");
client.println("                                <div class=\"row justify-content-center\">");
client.println("                                    <div class=\"col-md-12 mb-2\">");
client.println("                                        <button type=\"button\" class=\"btn btn-success btn-block text-light btn-lg\" id=\"turn-on-pump\">Включи помпата</button>");  // Replace anchor tags with buttons
client.println("                                    </div>");
client.println("                                </div>");
client.println("                                <div class=\"row justify-content-center\">");
client.println("                                    <div class=\"col-md-12\">");
client.println("                                        <button type=\"button\" class=\"btn btn-danger btn-block text-light btn-lg\" id=\"turn-off-pump\">Изключи помпата</button>");  // Replace anchor tags with buttons
client.println("                                    </div>");
client.println("                                </div>");
client.println("                            </div>");
client.println("                        </div>");
client.println("                    </div>");
client.println("                </div>");
client.println("            </div>");
  client.println("            <script>");
  client.println("                const soilMoistureSpan = document.getElementById(\"soil-moisture\");");
  client.println("                const turnOnButton = document.getElementById(\"turn-on-pump\");");
  client.println("                const turnOffButton = document.getElementById(\"turn-off-pump\");");
  client.println("");
  client.println("                function updateSoilMoistureLevel() {");
  client.println("                    var xhr = new XMLHttpRequest();");
  client.println("                    xhr.open('GET', '/soil-moisture', true);");
  client.println("                    xhr.onreadystatechange = function() {");
  client.println("                        if (xhr.readyState === 4 && xhr.status === 200) {");
  client.println("                            soilMoistureSpan.textContent = xhr.responseText;");
  client.println("                        }");
  client.println("                    };");
  client.println("                    xhr.send();");
  client.println("                }");
  client.println("");
  client.println("                function sendRequest(url) {");
  client.println("                    var xhr = new XMLHttpRequest();");
  client.println("                    xhr.open('GET', url, true);");
  client.println("                    xhr.send();");
  client.println("                }");
  client.println("");
  client.println("                turnOnButton.addEventListener(\"click\", function() {");
  client.println("                    sendRequest(\"/LED=ON\");");
  client.println("                });");
  client.println("");
  client.println("                turnOffButton.addEventListener(\"click\", function() {");
  client.println("                    sendRequest(\"/LED=OFF\");");
  client.println("                });");
  client.println("");
  client.println("                setInterval(updateSoilMoistureLevel, 1000);");
  client.println("");
  client.println("            </script>");
  client.println("");
  client.println("        </main>");
  client.println("    </div>");
  client.println("");
  client.println("    <script src=\"https://code.jquery.com/jquery-3.5.1.slim.min.js\"></script>");
  client.println("</body>");
  client.println("</html>");
}

int checkSoil()
{
  int soilMoisture = 0;
  // Read the analog value from the soil moisture sensor and convert it to a percentage
  soilMoisture = map(analogRead(A0), 0, 1024, 0, 100);

  soilMoisture = (soilMoisture - 100) * -1;
  
  // Return the soil moisture level as a percentage
  return soilMoisture;
}

void waterSoil(){
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    // Read soil moisture
    int moisture = checkSoil();

    // Check if moisture is under 15
    if (moisture <= 27) {
      // Turn on the LED
      digitalWrite(ledPin, HIGH);
      delay(3000);  // Keep the LED on for 3 seconds
      digitalWrite(ledPin, LOW);  // Turn off the LED
    }
  }
}