
Server:

    - Stocarea informatiilor:
        - am folosit map-uri, set-uri, string-uri si vectori pentru a retine
        detalii despre clientii conectati( map<socket, ID> si map<ID, socket>),
        topicurile cu SF = 1 (map<ID, unordered_set<TOPIC>>) pentru fiecare client,
        clientii care sunt abonati la un anumit topic( map<TOPIC, unordered_set<ID>>)
        si mesajele care trebuie trimise unui client offline(map<ID, vector<mesaje>>).
        - mesajele primite de server de la clientii UDP sunt parsate si stocate intr-o
        structura (tcp_message) care apoi este trimisa clientilor TCP abonati la res-
        pectivul topic( daca sunt).
        - mesajele de subscribe/ unsubscribe sunt stocate intr-o structura(msg_srv) si
        apoi trimise la server.
    
    - Implementare:
        	// retine pentru un socket clientul care e conectat pe el(ID-ul lui).
	        unordered_map<int, string> sock_ID_Map;

	        // retine pentru un client(ID), socketul pe care acesta e conectat.
	        unordered_map<string, int> id_sock_map;

	        // map intre topic si clientii abonati la el (ID-urile lor).
	        unordered_map<string, unordered_set<string>> topic_clients;

	        // map intre client(ID) si mesajele care trebuie trimise(daca e cazul)
	        // cat timp a fost offline.
	        unordered_map<string, vector<tcp_msg>> offline_msg; 

	        // map intre client(ID) si topicurile la care e abonat cu sf-ul 1.
	        unordered_map<string, unordered_set<string>> clients_sf;


            Conectare:

            - cand un client se conecteaza verificam daca este conectat deja un client 
            cu acelasi ID, caz in care il deconectam pe cel care vrea sa se conecteze.
            - daca nu exista un client cu acelasi ID deja conectat atunci il adaugam in
            map-ul sock_ID_Map si id_sock_map. Asta ne permite sa stim in permanenta ce
            clienti sunt conectati si sa stim pe ce socket e conectat un client( pentru
            trimitere mesaje, pentru afisare ID la deconectare, subscribe/ unsubscribe).
            - verificam daca exista un entry in map-ul offline_msg( daca trebuie sa primeasca
            mesaje cat timp a fost offline. Daca e prima conectare, sau daca nu s-a abonat 
            la topicuri cu SF = 1 nu i se va trimite nimic.)
            - trimitem mesajele care au fost trimise de UDP cat timp clientul TCP a fost offline
            si apoi stergem acel entry din map(sa nu trimita de mai multe ori mesajele).



            Primire si trimitere mesaje:

            - primim un mesaj de la un client UDP => parsare + construire mesaj => verificam daca
            exista clienti abonati la topicul din mesaj (find in map-ul topic_clients). Luam pe 
            rand fiecare client si daca acesta este conectat atunci ii trimitem masajul, altfel
            daca clientul este deconectat si este abonat cu SF = 1 la acest topic( find pe map-ul
             clients_sf), adaugam acel mesaj in map-ul offline_msg la clientul respectiv.

            

            Subscribe/ unsubscribe:

            - daca un client se aboneaza la un topic cu 2 SF-uri diferite, se va pastra ultima abonare
            Ex: daca se aboneaza cu SF = 0 si apoi cu SF = 1 atunci va ramane abonat cu SF = 1.
            - cand primim un mesaj verificam daca am primit exact cat ne asteptam(dimensiune structura
            srv_msg). Daca primim mai putin => apelam racv-uri pana cand primim tot mesajul.
            - primim un mesaj de la un client TCP.
            - daca SF = 1 atunci il adaugam in topic_clients si clients_sf.
            - daca SF = 0 atunci il adaugam in topic_clients si verificam daca acest client era abonat la
            acest topic cu SF = 1(verificam in map-ul clients_sf). Daca era abonat cu SF = 1 atunci il ster-
            gem din map-ul clients_sf. (stergem pentru client(ID) topicul).
            - daca primim un mesaj de unsubscribe verificam atat in map-ul topic_clients cat si clients_sf. In
            cazul in care exista topicul il stergem fie din topic_clients, fie din clients_sf, fie din
            ambele.



            Deconectare:

            - stergem clientul din sock_ID_Map, id_sock_map, socketul pe care e conectat clientul din set.
            - inchidem socket-ul pe care este conectat clientul.



Subscriber:

    Input subscriber:

    - exista 3 comenzi valide: exit, subscribe <topic> <sf> si unsubscribe <topic>.
    - daca primim exit se opreste clientul.
    - daca primim subscribe <topic> <sf>, verificam corectitudinea comenzii si construim un
    mesaj pe care sa-l trimitem pe server. Daca e valid il trimitem si apoi afisam pe client
    mesajul "Subscribed to topic".
    - daca primim unsubscribe <topic>, verificam corectitudinea comenzii si construim un
    mesaj pe care sa-l trimitem pe server. Daca e valid il trimitem si apoi afisam pe client
    mesajul "Unubscribed from topic".
    - daca nu primim niciuna dintre comenzile de mai sus, afisam "Comanda incorecta".



    Primire mesaje de la TCP:

    - verificam daca am primit cat ne asteptam(dimensiunea structurii tcp_msg). In cazul in care
    nu am primit suficienti octeti, reapelam recv pana cand suma octetilor primiti =  sizeof(tcp_msg)
    octeti.
    - afisam mesajul primit.






