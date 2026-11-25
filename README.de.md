# PV-Übersicht

Die Kachel zeigt PV-Leistung, Netzbezug oder Einspeisung und Hausverbrauch. Mit den optionalen Akkusensoren siehst du auch Lade- und Entladeleistung sowie den Ladestand.

## Einrichtung

1. Öffne in Tessera **Plugins**, wähle im Katalog die **PV-Übersicht** und füge sie deinem Display hinzu.
2. Starte den Firmware-Build und installiere die Firmware. Warte, bis das Display wieder verbunden ist.
3. Platziere unter **Layout → Plugins** die **PV-Übersicht** und wähle die Kachelgröße.
4. Wähle die beiden Pflichtsensoren für die gesamte AC-Solarleistung und die vorzeichenbehaftete Netzleistung. Ergänze nach Bedarf Hausverbrauch, Akkuleistung, Akkuladestand und PV-Tagesertrag.
5. Positive Werte stehen für Netzbezug und Akkuladung. Meldet dein Sensor Einspeisung oder Entladung positiv, aktiviere die passende Option zum Invertieren des Vorzeichens.

Sensoren und Größe kannst du später direkt im Layout ändern. Nach einem Plugin-Update baust du die Display-Firmware über **Build again** neu.

## Welche Sensoren auswählen?

| Feld | Benötigter Sensor | Einheit |
|---|---|---|
| Solarleistung | Gesamte AC-Leistung deiner PV-Wechselrichter | W oder kW |
| Netzleistung | Nettoleistung mit Vorzeichen: Plus = Bezug, Minus = Einspeisung | W oder kW |
| Hausverbrauch | Gemessener aktueller Hausverbrauch, optional | W oder kW |
| Akkuleistung | Nettoleistung: Plus = Laden, Minus = Entladen, optional | W oder kW |
| Akkuladestand | Ladezustand des Akkus, optional | % |
| PV-Tagesertrag | Ertrag seit Mitternacht, optional | Wh oder kWh |

Die Einheit muss im Attribut `unit_of_measurement` stehen. Bei abweichender Vorzeichenkonvention **Netzvorzeichen invertieren** bzw. **Akkuvorzeichen invertieren** aktivieren. Netz und Akku zeigen zusätzlich zum Richtungstext ein Vorzeichen. Die Richtungsschwelle von standardmäßig 10 W verhindert Flackern um null; der Messwert selbst wird nicht auf null gesetzt.

Bei mehreren Wechselrichtern einen Summensensor auswählen. Für mehrere Akkus sind ein gemeinsamer Leistungssensor und ggf. ein nach Speicherkapazität gewichteter Ladestand nötig. Getrennte Sensoren für Bezug/Einspeisung oder Laden/Entladen müssen zuerst in Home Assistant zu einem vorzeichenbehafteten Nettosensor zusammengefasst werden. Ohne Haussensor gilt: Solar + Netzbezug − Einspeisung − Akkuladung + Akkuentladung. Bei 700 W Solar, 27 W Bezug und 197 W Akkuladung sind das 530 W Hausverbrauch. Verwende die reine Solarerzeugung und möglichst die AC-seitige Akkuleistung. Ohne Akkuleistungssensor entfällt der Speicherterm. Ein ausgewählter Haussensor hat Vorrang. Fehlende oder veraltete Eingangswerte sowie eine negative Summe werden als Status angezeigt. Der Tagesertrag muss in Home Assistant täglich zurückgesetzt werden; ein Gesamtzähler eignet sich nicht.

## Größenanpassung

Unter dem runden Haus zeigt ein zentriertes Akkufeld Ladestand, Laderichtung und Leistung. Das waagerechte Akkusymbol zeigt den Füllstand. Rechts daneben stehen Prozentwert und Leistung. Fläche und Rahmen heben die Speicherkarte in beiden Designs hervor. Der Tagesertrag steht bei ausreichender Höhe direkt unter der Solarleistung. Für beide Werte die Sensoren Akkuleistung und Akkuladestand zuordnen.

Für kleinere Kacheln ab 2×2 nutzt Automatisch ein platzsparendes Flussdiagramm. Auf höheren 2×3-Flächen können Solar und Netz oberhalb des Hauses stehen. Die Auswahl richtet sich nach Platz, Schriftgröße und den ausgewählten Sensoren.

Im Modus **Automatisch** zeigt eine große Kachel das Haus zwischen Solarmodul und Stromnetz. Pfeile zeigen die Richtung am Netzanschluss und am optionalen Akku. Bei Stillstand oder fehlenden Messwerten bleibt die Verbindung ohne Pfeil. Tagesertrag und Ladestand erscheinen als Zusatzwerte. Sehr kleine Kacheln und der Modus **Kompakt** verwenden die Zahlenansicht. Angezeigt wird in der Reihenfolge Solar, Netz, Haus, Akkuleistung, Ladestand, Tagesertrag. Sehr kleine Kacheln zeigen nur Solar. Größere Kacheln zeigen weitere Werte; nicht ausgewählte optionale Sensoren entfallen. **Automatisch** zeigt bis zu sechs Werte, **Kompakt** bis zu vier. Eine Fußzeile weist bei genügend Platz auf ausgelassene Werte hin.

Die Darstellung verwendet Tessera-Schriften und Hell-/Dunkelfarben. Der Layout-Editor zeigt einen Platzhalter; die individuelle Darstellung entsteht erst auf dem Display. Die Zollgröße muss im Plugin nicht festgelegt werden.

## Fehlende oder alte Werte

`unavailable`, `unknown`, ungültige Zahlen und falsche Einheiten werden ausdrücklich angezeigt, nicht als null. Eine optionale Altersprüfung ist standardmäßig ausgeschaltet. Nur einschalten, wenn alle ausgewählten Sensoren regelmäßig ihren Zeitstempel aktualisieren: Unveränderte Werte wie Ladestand oder nächtlicher Tagesertrag können sonst fälschlich als veraltet gelten. Ohne Uhrzeit oder Zeitstempel ist keine Altersprüfung möglich. Die Fußzeile „Home Assistant“ bezeichnet die Datenquelle, nicht den Verbindungszustand; dafür Tessera-Verbindungsanzeige beachten.

## Getestete Displays

Der Maintainer hat die Kachel auf Guition-Displays mit 4, 7 und 10 Zoll unter Tessera 0.62.0 erfolgreich getestet. Weitere Prüfungen sind in [VALIDATION.md](VALIDATION.md) dokumentiert.
