 // Diese Zeile bindet die grundlegenden Funktionen und Definitionen der Arduino-Kernbibliothek ein,
 // die zur Programmierung des Arduino benötigt werden.
#include <Arduino.h>

// Diese Funktion wird beim Start des Arduino-Boards einmalig ausgeführt
// und initialisiert die serielle Kommunikation mit einer Baudrate von 9600,
// um Daten zwischen dem Arduino und einem angeschlossenen Computer zu senden und zu empfangen.
void setup() {
    Serial.begin(9600);
}
// Die loop-Funktion wird kontinuierlich ausgeführt, solange das Arduino-Board eingeschaltet ist.
// In diesem Abschnitt wird überprüft, ob Daten über die serielle Schnittstelle verfügbar sind.
// Wenn Daten vorhanden sind, liest der Arduino die eingehenden Zeichen bis zu einem Zeilenumbruch (\n)
// und speichert sie als String in der Variable input. Anschließend werden führende und nachfolgende Leerzeichen mit der trim-Methode entfernt.
void loop() {
    if (Serial.available() > 0) {
        String input = Serial.readStringUntil('\n');
        input.trim();

        // Ersetze Kommas durch Punkte in der Eingabe
        input.replace(',', '.');

        // Variablen für die Operanden und den Operator
        float operand1, operand2;
        char operatorChar;

        // In diesem Abschnitt wird nach dem Operator in der Eingabezeichenfolge gesucht.
        // Der operatorIndex wird zunächst auf -1 gesetzt, um anzuzeigen, dass der Operator noch nicht gefunden wurde.
        // Eine Schleife durchläuft jedes Zeichen der Eingabezeichenfolge input. Wenn eines der Zeichen ein Operator
        // ('+', '-', '*', '/') ist, wird operatorChar auf diesen Operator gesetzt und operatorIndex auf die Position dieses Operators in der Zeichenfolge.
        // Die Schleife wird mit break beendet, sobald der Operator gefunden wurde.
        int operatorIndex = -1;
        for (int i = 0; i < input.length(); i++) {
            if (input.charAt(i) == '+' || input.charAt(i) == '-' || input.charAt(i) == '*' || input.charAt(i) == '/') {
                operatorChar = input.charAt(i);
                operatorIndex = i;
                break;
            }
        }
        // Wenn kein Operator gefunden wurde, Fehler ausgeben und zurückkehren
        if (operatorIndex == -1) {
            Serial.println("Error: Operator not found");
            return;
        }
        // Dieser Code extrahiert die Operanden aus einer Eingabezeichenfolge, die einen Operator enthält.
        // Zuerst wird der Teil vor dem Operator extrahiert und in eine Gleitkommazahl umgewandelt, um den ersten Operanden zu erhalten.
        // Dann wird der Teil nach dem Operator extrahiert und ebenfalls in eine Gleitkommazahl umgewandelt, um den zweiten Operanden zu erhalten.
        // Diese Schritte ermöglichen es, die numerischen Werte für Berechnungen zu verwenden,
        // indem die Eingabe entsprechend aufgeteilt und in das richtige Format gebracht wird.
        operand1 = input.substring(0, operatorIndex).toFloat();
        operand2 = input.substring(operatorIndex + 1).toFloat();

        // Berechnungsfunktion
        // Dieser Code führt eine algebraische Operation basierend auf dem Operator zwischen zwei Operanden durch.
        // Dabei wird auf Division durch Null überprüft.
        // Das Ergebnis wird mit einer Genauigkeit von drei Dezimalstellen ausgegeben.
        float result;
        switch (operatorChar) {
            case '+':
                result = operand1 + operand2;
                break;
            case '-':
                result = operand1 - operand2;
                break;
            case '*':
                result = operand1 * operand2;
                break;
            case '/':
                if (operand2 != 0) {
                    result = operand1 / operand2;
                } else {
                    Serial.println("Error: Division by zero");
                    return;
                }
                break;
            default:
                Serial.println("Error: Invalid operator");
                return;
        }
        
        Serial.println(result, 3); // Ausgabe des Ergebnisses mit 3 Dezimalstellen
    }
}

