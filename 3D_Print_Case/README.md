# 3D Print Enclosure - Waveshare ESP32-S3-Touch-LCD-7 (Rev 1.2)
Designet til 3D-print på Bambu Lab (P1S, X1C, A1, H2D) med modulær 3-delt opbygning.

## De 3 Færdige STL-filer:

1. **`Waveshare7_Front_Bezel.stl` (Frontramme)**
   - 1.8 mm forsænket lomme til displayet (så touchglasset flugter 100% plant med fronten).
   - Indvendig 360° støttehylde (13.6 mm i siderne, 5.0 mm i top/bund), hvor glasset hviler stabilt.
   - 4x kraftige indvendige monteringsbøsninger (X = ±80 mm, Y = ±52 mm) til fastskruning af mellemstykket.
   - Snap-fit låseriller samt 2x åbningsriller (pry slots) til værktøjsfri adskillelse.

2. **`Waveshare7_Mellemstykke.stl` (Mellemstykke / Display Carrier)**
   - **4x indvendige huller**: Skrues direkte ind i Waveshares 4 fabriksmonterede M3 messingbøsninger på bagsiden af displayet.
   - **4x udvendige hjørnehuller**: Skrues fast i frontrammens 4 indvendige bøsninger.
   - **Stor center-åbning**: Giver 100% fri adgang til ESP32-S3 processoren, printkortet, JST-stik og knapper.
   - **Venstre udskæring**: Fri adgang for USB-C kabler og MicroSD-kort.
   - **Bund-udskæring**: Ekstra plads til batteriledninger fra 18650-batteriet.
   - Forsænkede huller så M3 skruehoveder sidder plant i pladen.

3. **`Waveshare7_Back_Case.stl` (Bunden / Bagkasse)**
   - Integreret 18650 snap-holder med fleksible C-klemmer i bunden.
   - Udskæringer til porte (2x USB-C + 1x MicroSD) udelukkende i VENSTRE side (højre side er 100% lukket).
   - Vægmontering via 2x nøglehulshuller (144 mm centerafstand).
   - Kabelgennemføring bagtil til vægdåse samt udbrudsrille (knockout) i bunden til ekstern ledning.
   - 4x støttepuder i bunden, der yder direkte modhold mod mellemstykket, så skærmen ikke flekser ved tryk.
   - Snap-fit montering til frontrammen.

---

## Samlevejledning (step-by-step):

1. **Forbered display og mellemstykke**:
   - Tag Waveshare 7"-displayet (som udgør én samlet enhed af glas + LCD-chassis).
   - Læg `Waveshare7_Mellemstykke.stl` over bagsiden af displayet.
   - Skru de 4x M3 skruer (f.eks. M3×6 mm eller M3×8 mm) gennem mellemstykket og ind i displayets 4 messingbøsninger.
   *(Display og mellemstykke er nu én solid, håndterbar enhed).*

2. **Monter i Frontrammen**:
   - Læg enheden ned i `Waveshare7_Front_Bezel.stl`.
   - Skru 4x M3 skruer gennem mellemstykkets 4 ydre hjørnehuller og ned i frontrammens bøsninger.
   *(Displayet er nu låst 100% fast, sidder urokkeligt, og glasset flugter helt plant med fronten – helt uden skruer på forsiden).*

3. **Isæt batteri og ledninger**:
   - Klik 18650-batteriet ned i snap-holderen i bunden af bagkassen.
   - Forbind batteri- og forsyningsledninger.

4. **Klik kassen sammen**:
   - Tryk bagkassen og frontrammen sammen med snap-fit lukningen.
   - Kassen kan altid adskilles igen uden skruer via åbningsrillen i bunden med en mønt eller flad skruetrækker.
