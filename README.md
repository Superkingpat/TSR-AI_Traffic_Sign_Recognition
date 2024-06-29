# Traffic Sign Recognition - TSR
[![Open in Visual Studio Code](https://open.vscode.dev/badges/open-in-vscode.svg)](https://open.vscode.dev/Superkingpat/TSR-AI_Traffic_Sign_Recognition) ![Contributors](https://img.shields.io/github/contributors/Superkingpat/TSR-AI_Traffic_Sign_Recognition) ![GitHub commit activity](https://img.shields.io/github/commit-activity/t/Superkingpat/TSR-AI_Traffic_Sign_Recognition) ![Last Commit](https://img.shields.io/github/last-commit/Superkingpat/TSR-AI_Traffic_Sign_Recognition) 

## Opis projekta

TSR je projekt za detekcijo, prepoznavo in beleženje prometnih znakov z mobilno napravo z pomočjo umetne inteligence na serverju. 

Naš namen je ustvariti varnejše okolje za vse udeležence cestnega prometa, bodisi vozniki, pešci ali kolesarji. TSR aplikacija omogoča telefonu spremljati promet in vas opozarjati na prehpde za pešce ali kolesarje, omejitve hitrosti, dela na cestišču itd.

Za prepoznavo in nadzor cestišca se uporablja dvodelna analiza slike z umetno inteligenco. Za lokalizacijo prometnih znakov na cestišču se uporablja [YOLOv8](https://github.com/ultralytics/ultralytics) od [Ultralytics](https://www.ultralytics.com), za treniranje katerega smo ročno anotirali več kot [1000 slik](https://app.roboflow.com/uni-qwrlr/driving-bfdzg/browse?queryText=&pageSize=50&startingIndex=0&browseQuery=true).

[![YOLOv8 logo](https://miro.medium.com/v2/resize:fit:1358/1*YQWYPi4uoT8RcG6BPbUoVw.png)](https://github.com/ultralytics/ultralytics)

Za klasifikacijo smo uporabljali Keras model DenseNet101, ki je dela [TensorFlow](https://www.tensorflow.org/tutorials/quickstart/beginner) knjižnice, za treniranje katerega smo uporabljali učno množico, ki je vsebovala okoli [90000 slik](https://univerzamb-my.sharepoint.com/:u:/g/personal/patrik_gobec_student_um_si/EYYisBJteiZJl5q1efYQeccBIVi3znCycz9KW-ghp77Njw?e=ll0q6K), dodatno smo te slike tudi augmentirali.

[![YOLOv8 logo](https://miro.medium.com/v2/resize:fit:2560/0*BrC7o-KTt54z948C.jpg)](https://keras.io)

## Razlaga programa

### comunicationHandler
todo
### Consumer
todo
### Mobile App
todo
### Image Geberation
todo
## Potrebne knjižnice in programska oprema

v tem predelu bomo predelali vso programsko opremo in Python knjižnice, ki so potrebne za uporabo našega projekta.

### Potrebni Python moduli

- [numpy](https://numpy.org)
- [Pillow](https://python-pillow.org)
- [io](https://docs.python.org/3/library/io.html)
- [time](https://docs.python.org/3/library/time.html)
- [ultralytics](https://pypi.org/project/ultralytics/)
- [prometheus_client](https://pypi.org/project/prometheus-client/)
- [tensorflow 2](https://www.tensorflow.org/tutorials/quickstart/beginner)
- [datetime](https://docs.python.org/3/library/datetime.html)
- [numba](https://numba.pydata.org)
- [sys](https://docs.python.org/3/library/sys.html)
- [confluent kafka](https://docs.confluent.io/platform/current/clients/confluent-kafka-python/html/index.html)

Vse potrebne knjižnice se lahko namestijo z ukazom:
```
pip install numpy pillow confluent-kafka ultralytics prometheus-client tensorflow numba
```

### Potrebna programska oprema - Docker

- Prenesite in namestite Docker iz [Docker spletne strani](https://desktop.docker.com/win/main/amd64/Docker%20Desktop%20Installer.exe). Prepričajte se, da imate nameščeno WSL 2 različico, če uporabljate Windows.

[![Docker logo](https://blog.codewithdan.com/wp-content/uploads/2023/06/Docker-Logo.png)](https://www.docker.com)

- Pred zagonom strežnika se prepričajte da so vsi IP naslovi definirani po vaši želji in da so vsa vrata, ki jih programi v [docker-compose.yaml](#todo) potrebujejo, odprta
- Odprite CMD ali terminal v direktoriju, kjer se nahaja [docker-compose.yaml](#todo), in zaženite ukaz ```docker compose up```
- S tem ukazom boste vzpostavili Kafka strežnik in Zookeeper z Prometheus in Grafana.

### Potrebna programska oprema - OpenVPN

- Za pravilno komunikacijo med strežnikom, consumerjem in mobilno aplikacijo je potrebno vspostaviti VPN povezavo med njimi.
- Na namiznem računalniku si naložite [OpenVPN](https://openvpn.net/client/client-connect-vpn-for-windows/) in iz [OneDriva](https://univerzamb-my.sharepoint.com/:f:/g/personal/patrik_gobec_student_um_si/Er2gpcJaEORMlxfCAjn7zBIB35p_Q3dOcjTnGAsXOKif0Q?e=94y0eR) prenesete potrebne VPN konfiguracije.

[![OpneVPN logo](https://static.100-downloads.com/media/programs/open-vpn.jpg)](https://openvpn.net/client/client-connect-vpn-for-windows/)

- Na Android mobitelu si naložite OpenVPN aplikacijo z Google Play Stora, in uporabite primerno VPN konfiguracijo z [OneDriva](https://univerzamb-my.sharepoint.com/:f:/g/personal/patrik_gobec_student_um_si/Er2gpcJaEORMlxfCAjn7zBIB35p_Q3dOcjTnGAsXOKif0Q?e=94y0eR)
- Prepričajte se, da sta IP naslov in vrata v consumer programu enaka kot ta, ki jih uporablja strežnik za uspešno komunikacijo med njima.

### Priprava AI modelov in IP naslovov

Prepričati se je treba da sta modela za YOLOv8 in Densenet101 v mapi za [modele](#todo)(ti modeli niso priloženi v reposetorju saj njihova velikost presega 100MB). Modeli se lahko pridobijo na [OneDrivu](#todo).