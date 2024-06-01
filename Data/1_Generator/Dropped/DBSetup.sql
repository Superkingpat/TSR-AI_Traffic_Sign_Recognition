--Pobrises vse tabele
DROP TABLE IF EXISTS EnvConfig;
DROP TABLE IF EXISTS EnvTags;
DROP TABLE IF EXISTS TagTable;
-- Sintaksa za SQLite

--Ustvarimo tabelo, ki sestoji iz stoplcev idAddress in address
CREATE TABLE 'EnvConfig' (	--Tu shranimo poti do datotek
	'idEnv' INTEGER PRIMARY KEY AUTOINCREMENT,
	'fileName' VARCHAR(100) NOT NULL UNIQUE,
    'color' BLOB
);

CREATE TABLE 'EnvTags' (	--To je tabela povezana s IndexTable. Povezuje datoteko s tagi
	'idEnv' INTEGER NOT NULL,
	'idTag' INTEGER NOT NULL,
	FOREIGN KEY ('idEnv') REFERENCES 'IndexTable'('idEnv') ON UPDATE CASCADE ON DELETE CASCADE,
	FOREIGN KEY ('idTag') REFERENCES 'TagTable'('idTag') ON UPDATE CASCADE ON DELETE CASCADE
);

CREATE TABLE 'TagTable' (
	'idTag' INTEGER PRIMARY KEY AUTOINCREMENT, 
	'tag' VARCHAR(100) NOT NULL UNIQUE
);