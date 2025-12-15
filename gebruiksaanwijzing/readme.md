# Gebruiksaanwijzing

### opladen / vervangen batterijen
de 6 aa batterijen kunnen gewoon vervangen worden door de oude batterijen uit de houder te halen en nieuwe in te steken

### draadloze communicatie
#### verbinding maken
Schakel de batterijschakelaar op ON om het systeem onder spanning te zetten. Het ledje op de HC-05-module zal snel knipperen. Installeer een seriële communicatie-app, bijvoorbeeld Serial Bluetooth Terminal (beschikbaar in de Google Play Store). Maak hiermee verbinding met de bluetoothmodule. Zodra dit gelukt is, knippert het ledje trager. De microcontroller kan nu draadloos commando’s ontvangen.

#### commando's
debug [on/off] – Toont de ingestelde parameters en uitlezingen (wit- en zwartwaarden). run – Start de robot. stop – Stopt de robot. set cycle [µs] – Stelt de cycle time in. Deze moet hoger zijn dan de calculation time (aanbevolen: 2×). De calculation time is zichtbaar via het debug-commando. set power [0..255] – Regelt de motor­kracht. set diff [0..1] – Past de snelheid in bochten aan. Hoe hoger de waarde, hoe sterker de robot versnelt in bochten. set kp [0..] – Correctie op basis van fout. Hogere waarden zorgen voor snellere bijsturing, maar te hoog kan schokkerig gedrag veroorzaken. set ki [0..] – Corrigeert sterker naarmate een fout langer aanhoudt. set kd [0..] – Corrigeert op basis van de verandering van de fout ten opzichte van de vorige cyclus. calibrate black – Slaat de zwarte referentiewaarden van de sensor op. calibrate white – Slaat de witte referentiewaarden van de sensor op.  

### kalibratie
De sensoren meten waardes tussen 0 en 1000 om onderscheid te maken tussen wit en zwart. Deze waardes kunnen variëren door lichtinval, reflecties of andere storingen. Voor een betrouwbare werking moet de robot opnieuw gekalibreerd worden bij een verandering van omgeving of ondergrond.  

### settings
De robot rijdt stabiel met volgende parameters: power: 35 diff:0.47 kp: 4 

### start/stop button
uitleg locatie + werking start/stop button
