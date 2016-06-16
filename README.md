CALINA CRISTIAN FLORIN 333CA

In implementarea temei am folosit ca laborator de start laboratul 5.

Am construit scena incercand sa creez un loop astfel incat scena sa se poata
regenera doar la cresterea unei variabile.

Scena este simpla , nimic dificil , formata din un drum si o scara ce duce la un
alt drum cu o alta scara. Si astfel se repeta pe 5 nivele.

Pentru gravitatie am folosit o valoare mai mica decat jumpul , altfel n-ar fi fost
suficient de puternic pentru a sari.

Coliziunile le-am facut folosind formula abs. Am verificat pe x , y si z daca in absolut
camera - pozitia obiectlui (desenat de mai multe ori) este mai mica ca latura (2.0).
In caz ca se intamplau aceste coliziuni blocam move-ul la apasarea tastelor a respectiv d.
Faceam asta simuland mai intai miscarea si daca intra in obiect o anulam.

La trecerea pe obiectul verde fac si trecerea la urmatorul nivel (in care marim scena 
cu inca 2 scari si 2 drumuri).

Mai multe detalii se gasesc in cod.