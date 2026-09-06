# 3D Print Enclosure - Waveshare ESP32-S3-Touch-LCD-7 (Rev 1.2)
Bredde udvidet til **228 mm** (20 mm ekstra bredde for masser af fri plads til USB-stik og kabler indvendigt).

## De 3 Færdige STL-filer:

1. **`Waveshare7_Front_Bezel.stl` (Frontramme - 228 mm bred)**
   - **1.8 mm forsænket lomme (undersænkning)**: Måler 193.9 mm x 111.7 mm, hvor Waveshare touch-glasset lægges i forfra og flugter 100% plant med rammen.
   - **Solid 2.5 mm tyk støttehylde**: 11.9 mm bred i siderne og 5.8 mm i top/bund, som glasset hviler stabilt på.
   - **Hul til metal-displaykasse**: Nøjagtigt **170.0 mm x 100.0 mm**.
   - **4x massive indvendige hjørnebøsninger** (X = ±98 mm, Y = ±54 mm) til Mellemstykket: Vokser direkte ud af frontvæggen/hylden.
   - Orienteret med fronten opad, så undersænkningen ses direkte i Bambu Studio.

2. **`Waveshare7_Mellemstykke.stl` (Mellemstykke / Display Carrier)**
   - **4x indvendige huller**: Skrues direkte i Waveshares 4 fabriksmonterede M3 messingbøsninger (X = -64.63 / +61.57 mm, Y = +33.80 / -31.85 mm).
   - **4x ydre hjørnehuller**: Skrues fast i frontrammens 4 hjørnebøsninger (X = ±98 mm, Y = ±54 mm).
   - **Stor center-åbning**: Fuld adgang til ESP32-S3 printkort, stik og knapper.
   - **Udvidet venstre udskæring (44 mm bred)**: Giver masser af fri plads til indvendige USB-C stik og kabler.
   - **Bund-udskæring**: God plads til batteriledninger.

3. **`Waveshare7_Back_Case.stl` (Bunden / Bagkasse - 228 mm bred)**
   - **Helt ren og flad bund**: De 4 stag er helt fjernet, så der er masser af plads til ledningsføring.
   - **Integreret 18650 snap-holder**: Med fleksible C-klemmer i bunden.
   - **Porte kun i venstre side**: 2x USB-C + 1x MicroSD (højre side er 100% lukket).
   - **Vægmontering**: 2x nøglehuller med 160 mm afstand.
   - **Kabelgennemføringer**: Central åbning bagpå til vægdåse + knockout-rille i bunden.
   - **Snap-fit lukning** med åbningsriller.

---

## Samlevejledning:
1. Skru `Waveshare7_Mellemstykke.stl` fast bag på Waveshare-displayet med 4x korte M3-skruer ind i displayets egne gevind.
2. Læg samlingen i `Waveshare7_Front_Bezel.stl` og skru 4x M3-skruer gennem mellemstykkets ydre huller ned i frontrammens bøsninger. Glasset ligger nu plant i den 1.8 mm dybe undersænkning foran.
3. Isæt 18650-batteriet i bunden af bagkassen, forbind kablerne (nu med 20 mm ekstra bredde til USB-stik), og klik kassen sammen.
