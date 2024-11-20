#include <Servo.h>

// Configuración del servo
const int servoPin = 11; // Pin del servo
Servo myServo;

// Tiempo para movimientos del servo
const int dotDuration = 200;   // Duración de un punto en milisegundos
const int dashDuration = 600;  // Duración de una raya en milisegundos
const int pauseDuration = 200; // Pausa entre puntos y rayas

// Tabla de traducción Morse
String morseCode[] = {
  ".-",   "-...", "-.-.", "-..",  ".",    "..-.", "--.",  "....", "..",   ".---",
  "-.-",  ".-..", "--",   "-.",   "---",  ".--.", "--.-", ".-.",  "...",  "-",
  "..-",  "...-", ".--",  "-..-", "-.--", "--..",
  "-----", ".----", "..---", "...--", "....-", ".....", "-....", "--...", "---..", "----."
};
char characters[] = {
  'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J',
  'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T',
  'U', 'V', 'W', 'X', 'Y', 'Z',
  '0', '1', '2', '3', '4', '5', '6', '7', '8', '9'
};

void setup() {
  // Inicializar el servo
  myServo.attach(servoPin);
  myServo.write(0); // Servo en posición inicial
  delay(500);
  
  // Inicializar comunicación serial
  Serial.begin(9600);
  Serial.println("Traductor Morse iniciado. Escribe un mensaje:");
}

void loop() {
  // Si hay datos disponibles en el monitor serial
  if (Serial.available() > 0) {
    String input = Serial.readStringUntil('\n'); // Leer la línea ingresada
    input.toUpperCase(); // Convertir a mayúsculas
    Serial.println("Texto recibido: " + input);
    translateToMorse(input);
  }
}

// Función para traducir texto a Morse y control del servo
void translateToMorse(String text) {
  for (int i = 0; i < text.length(); i++) {
    char c = text.charAt(i);
    
    if (c == ' ') {
      // Pausa larga entre palabras
      delay(7 * dotDuration);
      continue;
    }
    
    // Buscar el índice del carácter en el alfabeto/números
    int index = -1;
    for (int j = 0; j < sizeof(characters); j++) {
      if (characters[j] == c) {
        index = j;
        break;
      }
    }

    // Si el carácter no está en el alfabeto/números, ignorarlo
    if (index == -1) continue;

    // Obtener el código Morse correspondiente
    String code = morseCode[index];
    Serial.println(String(c) + ": " + code);

    // Reproducir el código Morse con el servo
    for (int k = 0; k < code.length(); k++) {
      if (code.charAt(k) == '.') {
        moveServo(dotDuration); // Punto
      } else if (code.charAt(k) == '-') {
        moveServo(dashDuration); // Raya
      }
      delay(pauseDuration); // Pausa entre puntos y rayas
    }

    // Pausa entre letras/números
    delay(3 * dotDuration);
  }
}

// Función para mover el servo (toque)
void moveServo(int duration) {
  myServo.write(90); // Mover el servo hacia 90 grados (toque)
  delay(duration);
  myServo.write(0); // Regresar a la posición inicial
  delay(pauseDuration);
}
