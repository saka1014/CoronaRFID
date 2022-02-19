# CoronaRFID


Dies ist die Quellcode Dokumentation für die Veranstalltung Energie-Effiziente Mikrocontroller.

Das Programm liest RFID-Tags und ordnet den User (Student) dem Raum zu an welcherm er sich Eingeloggt hat. Über ein Google-Sheet kann in einer Cloud eingesehen werden, welcher Student wann den Raum betreten hat. Sollte also einer der Studenten an Corona erkranken, kann über die Anwendung eine Nachverfolgung der Kontaktpersonen gestartet werden.

Zu Code Dokumentation:

Im Quellcode werden zuerst die Includes und die benötigten globalen Variablen angelegt.

1. RFID includes
2. Google und Wifi
3. deepsleep

RFID_Includes:

Die GIO Pins für den RFID Reader werden vereinbart und eine Registry angelegt in welcher gespeichert werden kann welcher Student im Raum aktuell eingeloogt ist.
Hervozuheben ist hierbei das die Regeisrty und der zwischenspeicher für die aktuell eingelesene ID im RTC Speicher "RTC_DATA_ATTR" gespeichert werden. 
Dieser behält bei einem Reboot aus dem Deepsleep seine Daten und wird erst bei einem Manuellen Reboot überschrieben.

Google und Wifi:

Die eigene Internetverbindung muss angegeben werden zusätzlich muss die ID des google Scripts angegenem werden in welche die Daten gespeichert werden sollen.
eine ausfühliche anleitung hierzu: https://esp32-server.de/google-sheets/

Deepsleep:

Um Energie zu sparen wird der Deepsleep verwendet.
Der Deepsleep hört auf ein Externes Event (Knopfdruck) deshalb wird in Hex angegeben welcher PIN im Deepsleep überwacht werden soll.


Das Programm besitzt verschiedene Funktionen:

1. Setup
2. Loop
3. printHex
4. printDec
5. searchTag
6. regTag
7. deleteTag
8. cloudfkt
9. buzzerlogin
10. buzzerlogoff


Setup:
In der Setup Funktion wird die Internet verbindung aufgebaut und überprüft, ob der Servo aus dem Deepsleep kommt.
Es wird vereibart das der ESP dann in den Deepsleep geht, wenn an PIN33 ein High-Signal anliegt.
Die SPI verbindung zum RFID-Reader wird aufgebaut. Wird die Setup zum erstenmal aufgerufen wird die Registry für die RFID Tags initialisiert. Kommt der ESP aus dem Deepsleep passiert das nicht, da diese ansonsten überschrieben würde.
Um den Buzzer anszusteuern wird eine PWM vereinbart. Mit einer niedrigen Frequenz um einen Tiefen Ton zu erzeugen.


Loop:
In der Loop wird geprüft, ob ein TAag zum Lesen vorhanden ist. Falls ja wird es gelesen und die ID zwischengespeichert.
Dann wird die Funktion search Tag aufgerufen.
  
  Search Tag:
  
  In der Search Tag Funktion wird in einer Schleife nach entsprechendem Tag gesucht ist es vorhanden wird true zurückgegeben.
  Falls nicht ein False.
  
Bei einem false wird die buzzerlogin Funktion aufgerufen

  Buzzerlogin:
  
  PWM an Buzzerpin um geräusch zu erzeugen.
  Der dutyccle von 300 erzeugt ein leises geräusch mit einer dauer von 50ms
  mit setzten des Dutycycles auf 0 ist der Buzzer wieder leise.
  
Danach wird die regTag Funktion aufgerufen.

  RegTag Funktion:
  
  Diese sucht nach einer freien Stelle in der Registry (dann frei wenn überlall initialisierungswert eingetragen ist) und schreibt die gelesene ID des Tags im zwischenspeicher     an die freie Stelle in der Registry.
  In der Registry wird auch die Cloudfkt aufgerufen.
  
  Clodfkt:
  
    In dieser Funktion wird die ID des Tags in einer URL gespeichert welche dann an den Google Server geschickt wird. Mit dieser Information trägt ein Script in dem Google Sheet     den Studenten im Raum ein.
  
Nach erfolgreicher Registrierung geht der ESP in den Deepsleep zurück.

Gibt die search Tag Funktion ein true zurück also der Student ist schon im Raum wird dieser Ausgeloggt.
Dafür wird die deleteTag Funktion aufgerufen.

  DeleteTag:
  
  In dieser wird der Tag in der Registry gesucht. An der Stelle an welcher die ID des Tags steht wird wieder der Initialisierungswert eingetragen.
  und die Buzzerlogoff Funktiona aufgerufen.
  
  Buzzerlogoff Funtion:
  
  Wie die Login Funktion, jedoch wird hier der Ton zwei mal erzeugt.
  
Nach dem Logoff vorgang geht der ESP wieder in den Deepsleep. 
Beim aufwecken über den Buzzer geht die Schleife wieder von vorne los.
  

  
