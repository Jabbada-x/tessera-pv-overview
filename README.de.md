# PV-Übersicht

> Entwicklungs-Beta für Firmware 0.60.0. Manifest und Messwertlogik sind geprüft. Ein vollständiger ESPHome-Build, die offizielle visuelle Vorschau und Tests auf deinen Displays stehen noch aus.

## Einrichtung

In Tessera unter **Plugins → Add with a link** den Link zu diesem GitHub-Repository einfügen. Ein Display auswählen, die Firmware bauen und anschließend unter **Layout → Plugins → PV-Übersicht** die Sensoren zuordnen. Alternativ lässt sich das Plugin lokal installieren.

1. ZIP entpacken und den kompletten Ordner `pv_overview` nach `/config/tessera-plugins/pv_overview/` in Home Assistant kopieren. Direkt darin muss `tessera-plugin.yaml` liegen. `tessera-plugins` liegt neben `esphome`.
2. In Tessera **Plugins** öffnen. Das lokale Plugin erscheint als **Test**. Zunächst einem Display hinzufügen und die Firmware bauen lassen. Dies ist zugleich der erste vollständige Firmware-Kompilierungstest dieser Beta. Bei einem Fehler das vollständige Build-Protokoll aufbewahren.
3. Unter **Layout → Plugins → PV-Übersicht** die Kachel hinzufügen und die Home-Assistant-Sensoren auswählen.
4. Kachelgröße durch Ziehen bzw. die Größenwahl anpassen. Auf einem 4-Zoll-Display zunächst eine ganze Seite verwenden. Auf 7 und 10,1 Zoll mit einer größeren Kachel beginnen.
5. Erst nach erfolgreichem Test auf dem ersten Display auf die anderen Displays übertragen.

Die Sensorauswahl und Kachelgröße lassen sich später ohne erneutes Flashen ändern. Änderungen am Plugin-Code benötigen dagegen **Build again**. Die ZIP ist kein direkt installierbares Firmware-Image und keine Lovelace-Karte.

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

Bei mehreren Wechselrichtern einen Summensensor auswählen. Für mehrere Akkus sind ein gemeinsamer Leistungssensor und ggf. ein nach Speicherkapazität gewichteter Ladestand nötig. Getrennte Sensoren für Bezug/Einspeisung oder Laden/Entladen müssen zuerst in Home Assistant zu einem vorzeichenbehafteten Nettosensor zusammengefasst werden. Hausverbrauch wird bewusst nicht aus möglicherweise gemischten AC-/DC-Werten berechnet. Der Tagesertrag muss in Home Assistant täglich zurückgesetzt werden; ein Gesamtzähler eignet sich nicht.

## Größenanpassung

Je nach freiem Platz entstehen eine, zwei oder drei Spalten. Angezeigt wird in der Reihenfolge Solar, Netz, Haus, Akkuleistung, Ladestand, Tagesertrag. Sehr kleine Kacheln zeigen nur Solar. Größere Kacheln zeigen weitere Werte; nicht ausgewählte optionale Sensoren entfallen. **Automatisch** zeigt bis zu sechs Werte, **Kompakt** bis zu vier. Eine Fußzeile weist bei genügend Platz auf ausgelassene Werte hin.

Die Darstellung verwendet Tessera-Schriften und Hell-/Dunkelfarben. Der Layout-Editor zeigt einen Platzhalter; die individuelle Darstellung entsteht erst auf dem Display. Die Zollgröße muss im Plugin nicht festgelegt werden.

## Fehlende oder alte Werte

`unavailable`, `unknown`, ungültige Zahlen und falsche Einheiten werden ausdrücklich angezeigt, nicht als null. Eine optionale Altersprüfung ist standardmäßig ausgeschaltet. Nur einschalten, wenn alle ausgewählten Sensoren regelmäßig ihren Zeitstempel aktualisieren: Unveränderte Werte wie Ladestand oder nächtlicher Tagesertrag können sonst fälschlich als veraltet gelten. Ohne Uhrzeit oder Zeitstempel ist keine Altersprüfung möglich. Die Fußzeile „Home Assistant“ bezeichnet die Datenquelle, nicht den Verbindungszustand; dafür Tessera-Verbindungsanzeige beachten.

## Entwicklungsstand

Version 0.1.0, lokal nutzbare Testfassung. Details und reproduzierbare Prüfkommandos stehen in `VALIDATION.md`. Es gibt noch keinen erfolgreichen vollständigen Firmware-Build und keine Prüfung auf realen 4-, 7- oder 10,1-Zoll-Displays. Ein Store-Vorschaubild fehlt, bis die offizielle Vorschau erfolgreich läuft.
