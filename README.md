# CoronaRFID

Dies ist die Quellcode Dokumentation für die Veranstaltung Energie-Effiziente Mikrocontroller.

Das Programm liest RFID-Tags und ordnet den User (Student) dem Raum zu, an welchem er sich eingeloggt hat. Über ein Google-Sheet kann in einer Cloud eingesehen werden, welcher Student wann den Raum betreten hat. Sollte also einer der Studenten an Corona erkranken, kann über die Anwendung eine Nachverfolgung der Kontaktpersonen gestartet werden.

Zu Code Dokumentation:

Im Quellcode werden zuerst die Includes und die benötigten globalen Variablen angelegt.

1. RFID includes
2. Google und Wi-Fi
3. deepsleep

RFID_Includes:

Die GIO Pins für den RFID Reader werden vereinbart und eine Registry angelegt in welcher gespeichert werden kann welcher Student im Raum aktuell eingeloogt ist.
Hervorzuheben ist hierbei, dass die Registry und der Zwischenspeicher für die aktuell eingelesene ID im RTC Speicher "RTC_DATA_ATTR" gespeichert werden.
Dieser behält bei einem Reboot aus dem Deepsleep seine Daten und wird erst bei einem manuellen Reboot überschrieben.

Google und Wi-Fi:

Die eigene Internetverbindung muss angegeben werden, zusätzlich muss die ID des google Scripts angegeben werden, in welchem die Daten gespeichert werden sollen.
Eine ausführliche Anleitung hierzu: https://esp32-server.de/google-sheets/

Deepsleep:

Um Energie zu sparen, wird der Deepsleep verwendet.
Der Deepsleep hört auf ein externes Event (Knopfdruck) deshalb wird in Hex angegeben welcher PIN im Deepsleep überwacht werden soll.

Das Programm besitzt verschiedene Funktionen:

1. Setup
2. Loop
3. printHex
4. printDec
5. searchTag
6. regTag
7. deleteTag
8. cloudfkt
9. buzzerlogin
10. buzzerlogoff

Setup:
In der Setup Funktion wird die Internetverbindung aufgebaut und überprüft, ob der ESP aus dem Deepsleep kommt.
Es wird vereinbart, dass der ESP dann in den Deepsleep geht, wenn an PIN33 ein High-Signal anliegt.
Die SPI Verbindung zum RFID-Reader wird aufgebaut. Wird die Setup zum ersten Mal aufgerufen, wird die Registry für die RFID-Tags initialisiert. Kommt der ESP aus dem Deepsleep passiert das nicht, da diese ansonsten überschrieben würde.
Um den Buzzer anzusteuern, wird eine PWM vereinbart. Mit einer niedrigen Frequenz, um einen tiefen Ton zu erzeugen.

Loop:
In der Loop wird geprüft, ob ein Tag zum Lesen vorhanden ist. Falls ja, wird es gelesen und die ID zwischengespeichert.
Dann wird die Funktion search Tag aufgerufen.

        Search Tag:

        In der Search Tag Funktion wird in einer Schleife nach entsprechendem Tag gesucht, ist es vorhanden wird true zurückgegeben.
        Falls nicht ein false.

Bei einem false wird die buzzerlogin Funktion aufgerufen

        Buzzerlogin:

        PWM an Buzzerpin, um Geräusch zu erzeugen.
        Der dutycycle von 300 erzeugt ein leises Geräusch mit einer Dauer von 50ms
        mit setzten des Dutycycles auf 0 ist der Buzzer wieder leise.

Danach wird die regTag Funktion aufgerufen.

        RegTag Funktion:

        Diese sucht nach einer freien Stelle in der Registry (dann frei, wenn überall Initialisierungswerte eingetragen sind) und schreibt die 
        gelesene ID des Tags im Zwischenspeicher an die freie Stelle in der Registry.
        In der Registry wird auch die Cloudfkt aufgerufen.

        Clodfkt:

        In dieser Funktion wird die ID des Tags in einer URL gespeichert, welche dann an den Google Server geschickt wird. Mit dieser     
        Information trägt ein Script in dem Google Sheet den Studenten im Raum ein.

Nach erfolgreicher Registrierung geht der ESP in den Deepsleep zurück.

Gibt die search Tag Funktion ein true zurück also der Student ist schon im Raum wird dieser Ausgeloggt.
Dafür wird die deleteTag Funktion aufgerufen.

        DeleteTag:

        In dieser wird der Tag in der Registry gesucht. An der Stelle, an welcher die ID des Tags steht, wird wieder der Initialisierungswert
        eingetragen und die Buzzerlogoff Funktion aufgerufen.

        Buzzerlogoff Funtion:

        Wie die Login-Funktion, jedoch wird hier der Ton zweimal erzeugt.

Nach dem Logoff Vorgang geht der ESP wieder in den Deepsleep.
Beim Aufwecken über den Buzzer geht die Schleife wieder von vorne los.


  
