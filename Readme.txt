# 📚 Afstudeer Documenten — Studiejaar 2024/2025 (Kwartiel 3 & 4)

Dit is de complete verzameling afstudeerdocumenten voor kwartiel 3 en 4 van het studiejaar **2024/2025**.  
Alle documenten zijn opgesteld in **LaTeX** met een focus op **herbruikbaarheid**, **consistentie** en **automatisering**.

---

## ⚙️ Bouwen van de documenten

Om alle PDF-documenten automatisch te genereren, kun je eenvoudig het batchscript uitvoeren:

### ✅ Stap-voor-stap:
1. Zorg dat de volgende programma's geïnstalleerd zijn op je computer:
   - `pdflatex`
   - `bibtex`
   - `makeglossaries`
2. Dubbelklik op het bestand:

```bash
CreateOutput.bat
```

Dit script doorloopt automatisch alle nodige stappen om de documenten te bouwen.

---

## 🧱 Structuur & Herbruikbaarheid

Er is bij de opzet van deze documenten veel aandacht besteed aan **herbruikbare componenten**.  
De map `GeneralTeX/` bevat generieke onderdelen zoals:

- Algemene opmaak en stijlen
- Instellingen voor lay-out en lettertypen
- Invoegbare componenten zoals titels, headers, voetnoten, etc.

Deze aanpak zorgt ervoor dat:

- 📄 Alle documenten een **uniforme opmaak** hebben  
- ♻️ Je eenvoudig updates of aanpassingen kunt doorvoeren op één centrale plek  
- 🔧 Aanpassingen aan stijl en structuur **automatisch doorwerken** in elk document

---

## 🚀 Snelstart

```text
1. Installeer LaTeX (bijvoorbeeld via MikTeX of TeX Live)
2. Controleer of pdflatex, bibtex en makeglossaries beschikbaar zijn
3. Voer CreateOutput.bat uit
4. Alle PDF's verschijnen automatisch in de outputmap
```

---

## 📁 Mappenoverzicht

| Map           | Inhoud                                    |
|----------------|--------------------------------------------|
| `GeneralTeX/`  | Herbruikbare stijlen, instellingen en templates |
| `Documents/`   | De daadwerkelijke afstudeerdocumenten         |
| `Output/`      | Hier worden de gegenereerde PDF's opgeslagen  |

---

💡 **Tip:** Als je LaTeX met een editor zoals [TeXstudio](https://www.texstudio.org/) of [Overleaf](https://www.overleaf.com/) gebruikt, kun je de afzonderlijke documenten ook handmatig bouwen.