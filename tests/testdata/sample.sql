PRAGMA foreign_keys=OFF;
BEGIN TRANSACTION;
DROP TABLE IF EXISTS vendors;
CREATE TABLE vendors (
    id      INTEGER PRIMARY KEY,
    addr    TEXT NOT NULL,
    name    TEXT NOT NULL,
    private BOOLEAN,
    block   TEXT NOT NULL,
    updated TEXT NOT NULL
);
INSERT INTO vendors VALUES(12,'00:00:0C','Cisco Systems, Inc',0,'MA-L','2015/11/17');
INSERT INTO vendors VALUES(18516,'00:48:54','',1,'','');
INSERT INTO vendors VALUES(12876445,'0C:47:A9:D','DIG_LINK',0,'MA-M','2024/10/28');
COMMIT;
