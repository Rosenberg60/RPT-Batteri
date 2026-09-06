# 3D Print Enclosure - Waveshare ESP32-S3-Touch-LCD-7 (Rev 1.2)
Designet til vægmontering og 3D-print på Bambu Lab (P1S, X1C, A1, H2D).

## Filer i mappen:
1. **`Waveshare7_Front_Bezel.stl`**
   - Frontramme med 1.8 mm undersænket fordybning til touch-glasset (så displayet er 100% plant med fronten).
   - Indvendig 360° støttehylde (13.6 mm i siderne, 5.0 mm for oven/neden) som glasset hviler stabilt på.
   - 4 indvendige monteringsbøsninger til clamp-beslagene.
   - Snap-fit låseriller + 2x åbningsriller (pry slots) i bunden og siden.

2. **`Waveshare7_Clamp_Brackets.stl`**
   - 2 stk. spændebeslag (Top & Bund).
   - Skrues fast med 4x korte M3 skruer (f.eks. M3x6mm eller M3x8mm) bagfra direkte ind i Waveshares 4 fabriksmonterede messing-bøsninger og frontrammens bøsninger.
   - Låser displayet 100% urokkeligt fast i frontrammen uden synlige skruer på fronten.

3. **`Waveshare7_Back_Case.stl`**
   - Integreret 18650 snap-holder med fleksible C-klemmer i bunden.
   - 4x indvendige støttesøjler, der støtter mod displayets bagside ved tryk på touchskærmen.
   - Port-udskæringer (2x USB-C + 1x MicroSD) udelukkende i VENSTRE side (højre side er 100% lukket).
   - Vægmontering: 2x nøglehuls-huller (144 mm centerafstand).
   - Kabelgennemføring på bagsiden til vægdåse samt knockout i bunden til udvendig kabelføring.

4. **`Waveshare7_Full_Enclosure.scad`**
   - Fuld parametrisk kildekode i OpenSCAD.

## Samlevejledning (step-by-step):
1. **Læg displayet i fronten**: Displayet lægges i forfra i den undersænkede lomme. Frontglasset flugter nu 100% plant med rammen.
2. **Fastgør med beslagene**: Vend frontrammen om, læg de to `Waveshare7_Clamp_Brackets` over displayets 4 messingbøsninger, og spænd med 4x M3 skruer. Displayet sidder nu urokkeligt fast i fronten. *(Valgfrit: En stribe tyndt 3M display-tape på støttehylden for støvtæt forsegling).*
3. **Isæt 18650 batteri & ledninger**: Tryk batteriet ned i snap-holderen i bunden af bagkassen.
4. **Klik kassen sammen**: Bagkassen trykkes blot fast i frontrammen med snap-fit. Skal kassen åbnes igen, vippes den let op via åbningsrillen i bunden med en flad skruetrækker eller mønt.
