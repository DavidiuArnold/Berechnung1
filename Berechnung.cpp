#include <iostream>  // Bibliothek für Ein- und Ausgabe
#include <fstream>   // Bibliothek für Dateioperationen
#include <windows.h> // Bibliothek für Windows-spezifische Funktionen
#include <string>    // Bibliothek für String-Operationen

// Funktion zur Konvertierung von std::string zu std::wstring
/*
Diese Konvertierung ist notwendig, weil std::string Zeichenketten im ASCII-Format darstellt,
diese wird von Arduino unterstützt.
Während std::wstring Zeichenketten im breiten Unicode-Format speichert.
Einige Funktionen in der Windows-API, zb. die mit seriellen Schnittstellen arbeiten
(ReadFile, WriteFile...), bevorzugen die Verwendung von breiten Zeichenketten (wchar_t) für die Pfadangaben.
*/
std::wstring stringToWString(const std::string& str) {
    // Bestimme die benötigte Größe für die Konvertierung
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
    // Erstelle eine wstring mit der benötigten Größe
    std::wstring wstrTo(size_needed, 0);
    // Führe die Konvertierung durch
    MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
    return wstrTo;
}

// Funktion zur Konfiguration des seriellen Ports
void configureSerialPort(HANDLE& hSerial) {
    // Erstellt eine DCB-Struktur zur Konfiguration des seriellen Ports
    /*Hier wird eine Variable initialisiert, die als Container für eine Struktur dient,
    welche verschiedene Einstellungen für die serielle Schnittstelle enthält. 
    Durch die Null-Initialisierung werden alle Felder dieser Struktur zurückgesetzt, 
    um eine saubere Ausgangsbasis für die Kommunikation mit der Schnittstelle zu schaffen.*/
    DCB dcbSerialParams = { 0 };
    dcbSerialParams.DCBlength = sizeof(dcbSerialParams);

    // Holt die aktuellen Einstellungen des seriellen Ports
    /*
    Diese Funktion wird verwendet, um die aktuellen Einstellungen
    der seriellen Schnittstelle zu erhalten und sie in einer Struktur zu speichern,
    die als dcbSerialParams bezeichnet wird. Wenn ein Fehler beim Abrufen der Einstellungen auftritt,
    wird eine Fehlermeldung ausgegeben und das Programm beendet.
    */
    if (!GetCommState(hSerial, &dcbSerialParams)) {
        std::cerr << "Error getting state: " << GetLastError() << std::endl;
        exit(1);
    }

    // Setzt die gewünschten Parameter für den seriellen Port
    /*
    In diesem Block werden die Einstellungen für die Kommunikation über den seriellen Port festgelegt.
    Es wird angegeben, wie schnell Daten gesendet werden sollen (BaudRate),
    wie viele Bits pro Zeichen verwendet werden (ByteSize),
    wie viele Stopbits jedes Zeichen haben soll (StopBits)
    und ob eine Paritätsprüfung durchgeführt werden soll (Parity).
    Hier werden Standardwerte verwendet, die für viele Anwendungen geeignet sind.
    */
    dcbSerialParams.BaudRate = CBR_9600;
    dcbSerialParams.ByteSize = 8;
    dcbSerialParams.StopBits = ONESTOPBIT;
    dcbSerialParams.Parity = NOPARITY;

    // Wendet die Konfiguration des seriellen Ports an und überprüft auf Fehler.
    if (!SetCommState(hSerial, &dcbSerialParams)) {
        std::cerr << "Error setting state: " << GetLastError() << std::endl;
        exit(1);
    }
}

// Funktion zum Senden von Daten an den seriellen Port
/*
Diese Funktion sendet Daten über den seriellen Port,
der durch das Handle hSerial dargestellt wird.
Die zu sendenden Daten sind im String data enthalten.
Die Funktion gibt die Anzahl der tatsächlich geschriebenen Bytes zurück.
Falls dieser Vorgang fehlschlägt, wird eine Fehlermeldung ausgegeben.
*/
void sendData(HANDLE& hSerial, const std::string& data) {
    DWORD bytesWritten;

    // Schreibe die Daten an den seriellen Port
    /*
    Hier werden Daten aus dem übergebenen data-String
    über die geöffnete serielle Schnittstelle gesendet. Falls dieser Vorgang fehlschlägt,
    wird eine Fehlermeldung mit dem entsprechenden Fehlercode ausgegeben.
    */
    if (!WriteFile(hSerial, data.c_str(), data.size(), &bytesWritten, NULL)) {
        std::cerr << "Error writing to serial port: " << GetLastError() << std::endl;
    }
}


// Funktion zum Empfangen von Daten vom seriellen Port
    /*
    Hier werden Daten von der seriellen Schnittstelle gelesen. Wenn das Lesen erfolgreich ist,
    werden die gelesenen Daten in einem Puffer (buffer) gespeichert und als std::string zurückgegeben.
    Falls ein Fehler auftritt, wird eine entsprechende Fehlermeldung mit dem Fehlercode ausgegeben
    und ein leerer String wird zurückgegeben.
    */
std::string receiveData(HANDLE& hSerial) {
    char buffer[256] = { 0 }; 
    DWORD bytesRead;

    if (ReadFile(hSerial, buffer, sizeof(buffer) - 1, &bytesRead, NULL)) {
        buffer[bytesRead] = '\0';  // *buffer*
        return std::string(buffer);
    }
    else {
        std::cerr << "Error reading from serial port: " << GetLastError() << std::endl;
        return "";
    }
}
// Hauptfunktion
/*
Hauptfunktion des Programms. 
Sie ermöglicht die Eingabe von Ausdrücken, die über die serielle Schnittstelle an den Arduino gesendet werden.
Die Ergebnisse werden zurückgegeben und zusammen mit der Eingabe in einer Logdatei gespeichert. 
Das Programm läuft weiter, bis "exit" eingegeben wird, um es zu beenden.
*/
int main() {
    // Eingabe des COM-Ports durch Anwender
    std::string comPort;
    std::cout << "Bitte geben sie den COM-Port an, mit dem der arduino verbunden ist. zb. COM3:\n";
    std::getline(std::cin, comPort);  // Eingabe des COM-Ports

    // Konvertiere den COM-Port-String zu LPCWSTR damit die Schnittstelle korrekt mit dem Arduino kommunizieren kann.
    std::wstring fullComPort = stringToWString("\\\\.\\") + stringToWString(comPort);

    // Öffne den seriellen Port
    /*
    Funktion stellt eine Verbindung zum seriellen Port her,
    damit das Programm mit ihm kommunizieren kann. 
    Das Handle wird verwendet, um Daten zum Lesen und Schreiben über diesen Port zu senden und zu empfangen.
    */
    HANDLE hSerial = CreateFile(
        fullComPort.c_str(),
        GENERIC_READ | GENERIC_WRITE,
        0,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    // Überprüft, ob der serielle Port erfolgreich geöffnet wurde, Falls nicht erfolgt eine Fehlermeldung.
    if (hSerial == INVALID_HANDLE_VALUE) {
        std::cerr << "Error opening serial port: " << GetLastError() << std::endl;
        return 1;
    }

    // Konfiguriere den seriellen Port
    configureSerialPort(hSerial);

    std::ofstream logFile("Berechnung.txt");  // Erstellt eine Logdatei Berechnung.txt

    // Endlosschleife zum Senden und Empfangen von Daten
    while (true) {
        std::string input;
        std::cout << "Bitte geben sie die Berechnung ein. (oder schreiben sie 'beenden' um das Programm zu schliessen.):\n";
        std::cout << "Es stehen folgende Rechenoperatoren zur Auswahl. (+) (-) (*) (/) und nur Ganzzahlen:\n";
        std::getline(std::cin, input);  // Liest die Eingabe des Benutzers
        if (input == "beenden") break;  // Beendet die Schleife, wenn 'exit' eingegeben wird

        sendData(hSerial, input + "\n");  // Sendet die Eingabe an den seriellen Port
        
        std::string result = receiveData(hSerial);  // Empfängt die Antwort vom seriellen Port
        std::cout << "Ergebniss: " << result << std::endl;  // Gibt das Ergebnis aus

        logFile << "Benutzereingabe: " << input << " Ergebniss: " << result << std::endl;  // Schreibt die Kommunikation in die Logdatei
    }

    // Schließt den seriellen Port und die Logdatei
    CloseHandle(hSerial);
    logFile.close();
    return 0;
}
