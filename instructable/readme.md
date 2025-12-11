# Instructable

Een instructable is een stappenplan - zonder verdere uitleg - hoe je vertrekkend van de bill of materials en gebruik makend van de technische tekeningen de robot kan nabouwen. Ook de nodige stappen om de microcontroller te compileren en te uploaden staan beschreven.  

### stap 1
bestel alle componenten uit de bill of materials 

### stap 2
Controleer elk onderdeel afzonderlijk door de proefopstellingen (proof of concepts) uit te voeren. Raadpleeg zo nodig de datasheets om de werking of aansluitingen te verifiëren.

### stap 3
Soldeer alle componenten op de printplaat volgens het elektrische schema. Gebruik pinheaders waar nodig(bij de hc-06 module)

### stap 4
Monteer de 2 motors op het uit einde van de printplaat door 2 gaten te boren en de motors te bevestigen met bout en moer.

### stap 5
monteer de batterijhouder met de batterij op de printplaat met dubbelzijdige tape

### stap 6 
vijs nog 2 zelftappers vooraan in de printplaat zodat de sensor niet over de grondsleept en zo een goede meting kan maken

### stap 7
Zet het programma op de Arduino Nano. Koppel hiervoor eerst de TX- en RX-pinnen van de HC-05-module los om storingen tijdens het uploaden te vermijden. Na het uploaden kan je opnieuw verbinding maken met de HC-05 via een seriële communicatie-app.
