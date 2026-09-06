# 3D Print Enclosure - Waveshare ESP32-S3-Touch-LCD-7 (Rev 1.2)
Designet til 3D-print på Bambu Lab (P1S, X1C, A1, H2D) med modulær 3-delt opbygning.

## De 3 Færdige STL-filer:

1. **`Waveshare7_Front_Bezel.stl` (Frontramme)**
   - 1.8 mm forsænket lomme foran, så displayets touch-glas ligger 100% plant med frontens overflade.
   - **Hul til metal chassis**: Nøjagtigt **170.0 mm x 100.0 mm**.
   - **4x massive indvendige monteringsbøsninger** (X = ±92 mm, Y = ±54 mm):
     - Vokser direkte ud af frontvæggen (har solidt fat i fronten, svæver IKKE).
     - Placeret godt væk fra kanten af 170x100 hullet ud i hjørnerne.
     - Blinde M3 skruehuller (2.8 mm diameter, 5.5 mm dybde) uden gennembrud på forsiden.
   - Snap-fit låseriller samt 2x åbningsriller (pry slots).

2. **`Waveshare7_Mellemstykke.stl` (Mellemstykke / Display Carrier)**
   - **4x indvendige huller**: Skrues fast direkte i displayets 4 fabriksmonterede M3 messingbøsninger (X = -64.63 / +61.57 mm, Y = +33.80 / -31.85 mm).
   - **4x udvendige hjørnehuller**: Skrues fast i frontrammens 4 massive hjørnebøsninger (X = ±92 mm, Y = ±54 mm).
   - **Stor center-åbning**: Giver 100% fri adgang til ESP32-S3 processoren, printkortet, JST-stik og knapper.
   - **Venstre udskæring**: Fri adgang til USB-C og MicroSD.
   - **Bund-udskæring**: God plads til batteriledninger fra 18650-batteriet.

3. **`Waveshare7_Back_Case.stl` (Bunden / Bagkasse)**
   - **De 4 stag i bunden er helt fjernet**: Bunden er helt ren og flad med masser af plads til ledningsføring.
   - Integreret 18650 snap-holder med fleksible C-klemmer i bunden.
   - Porte (2x USB-C + 1x MicroSD) udelukkende i VENSTRE side (højre side er 100% lukket).
   - Vægmontering via 2x nøglehulshuller (144 mm centerafstand).
   - Kabelgennemføring bagtil til vægdåse samt udbrudsrille (knockout) i bunden til ekstern ledning.
   - Snap-fit montering til frontrammen.

---

## Samlevejledning:

1. **Skru Mellemstykket på displayet**:
   - Læg `Waveshare7_Mellemstykke.stl` på bagsiden af Waveshare-displayet.
   - Skru 4x korte M3-skruer (f.eks. M3×6 mm eller M3×8 mm) gennem de 4 indvendige huller i mellemstykket og ind i displayets 4 messingbøsninger.
2. **Skru samlingen i Frontrammen**:
   - Læg display + mellemstykke ned i `Waveshare7_Front_Bezel.stl`.
   - Skru 4x M3-skruer gennem mellemstykkets 4 ydre hjørnehuller og ned i frontrammens massive bøsninger.
   - Displayet er nu låst 100% fast, sidder urokkeligt, og glasset flugter plant med fronten.
3. **Isæt batteri og klik bagkassen på**:
   - Tryk 18650-batteriet i bunden af bagkassen, forbind ledninger, og klik bagkassen fast på frontrammen.
